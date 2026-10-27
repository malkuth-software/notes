// Experiment 001: which route puts an application-composed page on the window
// within the owner's bar (50 ms per page turn at device resolution, zero work
// at rest). See README.md beside this file for the question, the procedure
// and the results.
//
// One run measures one route. The probe opens the book-sized window, makes two
// synthetic opaque pages at device resolution (boxes of ink on paper; no text
// of any kind), turns between them a set number of times from rest, then sits
// idle for a sample and prints one JSON line. The pages are made before the
// first turn: only the route is timed, not composition.

#include "wake_counter.hpp"

#include "core/process_counters.hpp"
#include "forms/book_view.hpp"

#include "gui_forms/application.hpp"
#include "gui_forms/canvas.hpp"
#include "gui_forms/control.hpp"
#include "gui_forms/live_surface.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

enum class Route : std::uint8_t {
    // Control: fill_rect in two colours. The cost of a turn with no image.
    fill,
    // Two registered images; a turn draws the other with draw_image.
    image_swap,
    // The same, drawn with draw_image_region_sampled(nearest).
    image_swap_nearest,
    // One registered image; a turn replaces its pixels with
    // update_bgra32_premultiplied, then draw_image.
    image_update,
    // One registered image; a turn patches its whole area with
    // patch_bgra32_premultiplied, then draw_image.
    image_patch,
    // The page as eight registered strips; a turn updates each, and the
    // paint draws each with draw_image.
    image_strips,
    // Two registered images; a turn draws the other with fill_image_pattern,
    // one tile the size of the page.
    image_pattern,
    // A LiveSurface that is never registered with the window: a turn
    // publishes the page into it and on_paint draws it with
    // Painter::draw_live_surface.
    surface_painter,
    // A LiveSurface registered once for the life of the view: a turn only
    // publishes; the host's live clock presents it.
    surface_held,
    // A LiveSurface registered for the turn through a child control that is
    // added at the turn and removed once the turn is presented.
    surface_turn,
    // A RasterCanvas child showing a GUI.Drawing bitmap at one bitmap pixel
    // per device pixel; a turn rewrites the bitmap and synchronizes.
    raster_canvas,
    // The book itself on the chosen route: the real view as a child, turned
    // through the proving slice in reading form. Unlike the routes above,
    // each turn composes its spread, so this is the whole cost of a page
    // turn after the key arrives.
    book_reading,
    // The same in study form: the original on the left page.
    book_study,
};

struct RouteEntry final {
    Route route{Route::fill};
    std::string_view name{};
};

constexpr std::array<RouteEntry, 13U> route_table{{
    {Route::fill, "fill"},
    {Route::image_swap, "image-swap"},
    {Route::image_swap_nearest, "image-swap-nearest"},
    {Route::image_update, "image-update"},
    {Route::image_patch, "image-patch"},
    {Route::image_strips, "image-strips"},
    {Route::image_pattern, "image-pattern"},
    {Route::surface_painter, "surface-painter"},
    {Route::surface_held, "surface-held"},
    {Route::surface_turn, "surface-turn"},
    {Route::raster_canvas, "raster-canvas"},
    {Route::book_reading, "book-reading"},
    {Route::book_study, "book-study"},
}};

constexpr std::uint32_t strip_count = 8U;
constexpr std::uint32_t page_count = 2U;
// The toolkit's per-image limit (image_resource_types.hpp:22-32).
constexpr std::uint32_t image_maximum_edge_px = 4096U;

constexpr std::chrono::milliseconds settle_before_first_turn{1000};
constexpr std::chrono::milliseconds turn_interval{250};
constexpr std::chrono::milliseconds idle_warm_up{2000};
constexpr std::chrono::milliseconds observer_time_limit{1000};

struct ProbeOptions final {
    Route route{Route::fill};
    std::string_view route_name{"fill"};
    std::uint32_t turns{30U};
    std::uint32_t idle_seconds{10U};
    double logical_width{1060.0};
    double logical_height{680.0};
};

// Times of one turn, all measured from the request: the first statement of
// the timer callback that stands in for the key handler.
struct TurnRecord final {
    // To the return of the route's own update call (update, patch, publish).
    double update_ms{0.0};
    // To the return of the on_paint that followed. The toolkit records
    // on_paint and replays it into its raster afterwards, so this is before
    // the pixels are drawn. Zero when the route presents without a paint.
    double recorded_ms{0.0};
    // To the first dispatcher callback that saw the turn presented. This is
    // the figure held against the bar.
    double observed_ms{0.0};
    // The host's own paint-and-commit duration for the frames it presented
    // during the turn (MetricsSnapshot::present_duration_nanoseconds).
    double host_present_ms{0.0};
    bool presented{false};
};

double milliseconds_between(Clock::time_point from, Clock::time_point to) {
    const std::chrono::duration<double, std::milli> elapsed = to - from;
    const double milliseconds = elapsed.count();
    return milliseconds;
}

// ---- Synthetic pages ---------------------------------------------------------

struct Bgra final {
    std::byte blue{};
    std::byte green{};
    std::byte red{};
    std::byte alpha{};
};

constexpr Bgra paper_color{std::byte{0xe0}, std::byte{0xec}, std::byte{0xf0}, std::byte{0xff}};
constexpr Bgra ink_color{std::byte{0x1c}, std::byte{0x22}, std::byte{0x2a}, std::byte{0xff}};

void fill_box(std::vector<std::byte>& pixels, std::uint32_t width_px, std::uint32_t left, std::uint32_t top,
              std::uint32_t right, std::uint32_t bottom, Bgra color) {
    for (std::uint32_t y = top; y < bottom; ++y) {
        const std::size_t row = static_cast<std::size_t>(y) * width_px * 4U;
        for (std::uint32_t x = left; x < right; ++x) {
            const std::size_t offset = row + static_cast<std::size_t>(x) * 4U;
            pixels[offset] = color.blue;
            pixels[offset + 1U] = color.green;
            pixels[offset + 2U] = color.red;
            pixels[offset + 3U] = color.alpha;
        }
    }
}

// Paper with rows of ink boxes whose widths come from a linear congruential
// sequence seeded by the page number, so the two pages differ in most rows.
// Opaque, premultiplied BGRA, rows top to bottom without padding.
void render_synthetic_page(std::uint32_t width_px, std::uint32_t height_px, std::uint32_t seed,
                           std::vector<std::byte>& destination) {
    const std::size_t byte_count = static_cast<std::size_t>(width_px) * height_px * 4U;
    std::vector<std::byte> pixels(byte_count);
    fill_box(pixels, width_px, 0U, 0U, width_px, height_px, paper_color);

    const std::uint32_t margin = width_px / 12U;
    const std::uint32_t line_pitch = 44U;
    const std::uint32_t box_height = 18U;
    const std::uint32_t gap = 14U;
    std::uint32_t state = seed * 2654435761U + 12345U;
    for (std::uint32_t top = margin; top + box_height + margin <= height_px; top += line_pitch) {
        std::uint32_t left = margin;
        while (left < width_px - margin) {
            state = state * 1664525U + 1013904223U;
            const std::uint32_t box_width = 30U + (state >> 16U) % 120U;
            const std::uint32_t right = std::min(left + box_width, width_px - margin);
            fill_box(pixels, width_px, left, top, right, top + box_height, ink_color);
            left = right + gap;
        }
    }
    destination = std::move(pixels);
}

// ---- The view ----------------------------------------------------------------

// A RasterCanvas that reports the end of each of its paints.
class ProbeCanvas final : public gui_forms::RasterCanvas {
  public:
    explicit ProbeCanvas(gui_forms::StableId id) : gui_forms::RasterCanvas(std::move(id)) {}

    // Called at the end of every on_paint. Owned here; the target holds its
    // run weakly.
    std::function<void()> painted{};

    void on_paint(gui_forms::Painter& painter, gui_forms::Rect local_damage) override {
        gui_forms::RasterCanvas::on_paint(painter, local_damage);
        if (painted) {
            painted();
        }
    }
};

// Exists only so a live surface can be registered against a control that can
// be detached again; it paints nothing.
class SurfaceRegistrant final : public gui_forms::Control {
  public:
    explicit SurfaceRegistrant(gui_forms::StableId id) : gui_forms::Control(std::move(id)) {}
};

class ProbeView final : public gui_forms::Control {
  public:
    explicit ProbeView(gui_forms::StableId id) : gui_forms::Control(std::move(id)) {}

    // Called at the end of every on_paint of this view or its canvas.
    std::function<void()> painted{};

    // Makes the two pages and whatever the route registers. `self` is this
    // view's own owning pointer; the window must be attached. Returns false
    // with failure() set when a resource could not be made.
    [[nodiscard]] bool prepare(gui_forms::Window& window, const std::shared_ptr<ProbeView>& self,
                               Route route);

    // Shows the other page by the route. Returns false with failure() set
    // when the toolkit refused the update.
    [[nodiscard]] bool turn();

    // surface_turn only: detaches the registrant once the turn is presented.
    void release_turn_registration();

    void on_paint(gui_forms::Painter& painter, gui_forms::Rect local_damage) override;

    [[nodiscard]] std::uint64_t paints() const noexcept {
        return paints_;
    }
    [[nodiscard]] Clock::time_point last_paint_end() const noexcept {
        return last_paint_end_;
    }
    [[nodiscard]] std::uint32_t device_width() const noexcept {
        return width_px_;
    }
    [[nodiscard]] std::uint32_t device_height() const noexcept {
        return height_px_;
    }
    [[nodiscard]] const std::string& failure() const noexcept {
        return failure_;
    }
    // True when the host has sampled the generation published by the last
    // turn. Meaningful for the surface routes only.
    [[nodiscard]] bool surface_sampled() const;
    void note_paint_end();

  private:
    [[nodiscard]] std::span<const std::byte> page_pixels(std::uint32_t page) const {
        const std::span<const std::byte> pixels(pages_[page].data(), pages_[page].size());
        return pixels;
    }
    [[nodiscard]] bool register_whole_pages(std::uint32_t count);
    [[nodiscard]] bool register_strips();
    [[nodiscard]] bool publish_surface(std::uint32_t page);
    [[nodiscard]] bool write_bitmap(std::uint32_t page);
    [[nodiscard]] bool fail(std::string_view what, gui_forms::ImageResourceError error);

    // Borrowed; the toolkit detaches controls before the window dies.
    gui_forms::Window* window_{nullptr};
    std::weak_ptr<ProbeView> self_{};
    Route route_{Route::fill};
    std::uint32_t width_px_{0U};
    std::uint32_t height_px_{0U};
    std::uint64_t row_bytes_{0U};
    std::array<std::vector<std::byte>, page_count> pages_{};
    std::uint32_t shown_page_{0U};

    // Whole-page images: two for the swap routes, one for update and patch.
    std::array<gui_forms::ImageId, page_count> images_{};
    // image_strips: strip s covers device rows [strip_top_[s], strip_top_[s + 1]).
    std::array<gui_forms::ImageId, strip_count> strips_{};
    std::array<std::uint32_t, strip_count + 1U> strip_top_{};

    std::shared_ptr<gui_forms::LiveSurface> surface_{};
    std::uint64_t published_generation_{0U};
    std::shared_ptr<SurfaceRegistrant> registrant_{};
    bool registrant_attached_{false};

    std::shared_ptr<gui_drawing::Bitmap> bitmap_{};
    std::shared_ptr<ProbeCanvas> canvas_{};
    std::shared_ptr<bible::forms::BookView> book_{};

    std::uint64_t paints_{0U};
    Clock::time_point last_paint_end_{};
    std::string failure_{};
};

bool ProbeView::fail(std::string_view what, gui_forms::ImageResourceError error) {
    failure_ = std::string(what) + " (image resource error " + std::to_string(static_cast<int>(error)) + ")";
    return false;
}

bool ProbeView::register_whole_pages(std::uint32_t count) {
    for (std::uint32_t page = 0U; page < count; ++page) {
        const gui_forms::ImageLoadResult loaded =
            (*window_).load_bgra32_premultiplied(width_px_, height_px_, row_bytes_, page_pixels(page));
        if (loaded.error != gui_forms::ImageResourceError::none) {
            return fail("load_bgra32_premultiplied", loaded.error);
        }
        images_[page] = loaded.image;
    }
    return true;
}

bool ProbeView::register_strips() {
    for (std::uint32_t strip = 0U; strip <= strip_count; ++strip) {
        const std::uint64_t top = static_cast<std::uint64_t>(height_px_) * strip / strip_count;
        strip_top_[strip] = static_cast<std::uint32_t>(top);
    }
    for (std::uint32_t strip = 0U; strip < strip_count; ++strip) {
        const std::uint32_t rows = strip_top_[strip + 1U] - strip_top_[strip];
        const std::size_t offset = static_cast<std::size_t>(strip_top_[strip]) * row_bytes_;
        const std::size_t bytes = static_cast<std::size_t>(rows) * row_bytes_;
        const gui_forms::ImageLoadResult loaded = (*window_).load_bgra32_premultiplied(
            width_px_, rows, row_bytes_, page_pixels(0U).subspan(offset, bytes));
        if (loaded.error != gui_forms::ImageResourceError::none) {
            return fail("load_bgra32_premultiplied (strip)", loaded.error);
        }
        strips_[strip] = loaded.image;
    }
    return true;
}

bool ProbeView::publish_surface(std::uint32_t page) {
    gui_forms::LiveSurfaceWriteLease lease = (*surface_).try_acquire_write(false);
    if (!lease) {
        failure_ = "LiveSurface::try_acquire_write returned no lease";
        return false;
    }
    const std::span<std::byte> target = lease.pixels();
    const std::span<const std::byte> source = page_pixels(page);
    if (target.size() != source.size() || lease.row_bytes() != row_bytes_) {
        failure_ = "LiveSurface buffer does not have the page's layout";
        return false;
    }
    std::memcpy(target.data(), source.data(), source.size());
    published_generation_ = lease.publish();
    return true;
}

bool ProbeView::write_bitmap(std::uint32_t page) {
    const gui_drawing::BitmapLockView view = (*bitmap_).lock(gui_drawing::BitmapLockMode::write);
    const std::span<const std::byte> source = page_pixels(page);
    const std::size_t tight_row = static_cast<std::size_t>(row_bytes_);
    for (std::uint32_t row = 0U; row < height_px_; ++row) {
        std::memcpy(view.writable_data + static_cast<std::size_t>(row) * view.row_bytes,
                    source.data() + static_cast<std::size_t>(row) * tight_row, tight_row);
    }
    (*bitmap_).unlock(view.token);
    return true;
}

bool ProbeView::prepare(gui_forms::Window& window, const std::shared_ptr<ProbeView>& self, Route route) {
    window_ = &window;
    self_ = self;
    route_ = route;
    const gui_forms::Size client = window.client_size();
    const double scale = window.scale();
    const long width = std::lround(client.width * scale);
    const long height = std::lround(client.height * scale);
    if (width < 1 || height < 1 || width > static_cast<long>(image_maximum_edge_px) ||
        height > static_cast<long>(image_maximum_edge_px)) {
        failure_ = "the client area in device pixels is outside 1..4096 on an edge";
        return false;
    }
    width_px_ = static_cast<std::uint32_t>(width);
    height_px_ = static_cast<std::uint32_t>(height);
    row_bytes_ = static_cast<std::uint64_t>(width_px_) * 4U;
    const bool book_route = route_ == Route::book_reading || route_ == Route::book_study;
    if (!book_route) {
        for (std::uint32_t page = 0U; page < page_count; ++page) {
            render_synthetic_page(width_px_, height_px_, page + 1U, pages_[page]);
        }
    }

    bool prepared = true;
    switch (route_) {
    case Route::fill:
        break;
    case Route::image_swap:
    case Route::image_swap_nearest:
    case Route::image_pattern:
        prepared = register_whole_pages(page_count);
        break;
    case Route::image_update:
    case Route::image_patch:
        prepared = register_whole_pages(1U);
        break;
    case Route::image_strips:
        prepared = register_strips();
        break;
    case Route::surface_painter:
    case Route::surface_held:
    case Route::surface_turn: {
        gui_forms::LiveSurfaceDescription description;
        description.width = width_px_;
        description.height = height_px_;
        // The smallest pool the toolkit allows: the page shown and the page
        // being written.
        description.buffer_count = gui_forms::minimum_live_surface_buffer_count;
        surface_ = gui_forms::LiveSurface::create(description);
        if (!surface_) {
            failure_ = "LiveSurface::create returned nothing";
            return false;
        }
        prepared = publish_surface(0U);
        if (prepared && route_ == Route::surface_held) {
            prepared = window.queue_live_surface_presentation(self, surface_);
            if (!prepared) {
                failure_ = "queue_live_surface_presentation refused the view";
            }
        }
        if (prepared && route_ == Route::surface_turn) {
            registrant_ = gui_forms::make_control<SurfaceRegistrant>(gui_forms::StableId("probe.registrant"));
            (*registrant_).set_dock(gui_forms::DockStyle::fill);
        }
        break;
    }
    case Route::raster_canvas: {
        bitmap_ = std::make_shared<gui_drawing::Bitmap>(width_px_, height_px_);
        prepared = write_bitmap(0U);
        canvas_ = gui_forms::make_control<ProbeCanvas>(gui_forms::StableId("probe.canvas"));
        (*canvas_).painted = painted;
        (*canvas_).set_dock(gui_forms::DockStyle::fill);
        (*canvas_).set_transparency_grid(false);
        (*canvas_).set_sampling(gui_forms::ImageSampling::nearest);
        // One bitmap pixel per device pixel.
        (*canvas_).set_zoom(1.0 / scale);
        (*canvas_).set_bitmap(bitmap_);
        add_child(canvas_);
        break;
    }
    case Route::book_reading:
    case Route::book_study: {
        book_ = gui_forms::make_control<bible::forms::BookView>(gui_forms::StableId("probe.book"));
        (*book_).painted = painted;
        (*book_).set_dock(gui_forms::DockStyle::fill);
        add_child(book_);
        prepared = (*book_).load_book(window);
        if (!prepared) {
            failure_ = (*book_).load_failure();
        }
        bible::page::BookSettings settings;
        settings.form = route_ == Route::book_study ? bible::page::BookForm::study : bible::page::BookForm::reading;
        (*book_).set_settings(settings);
        break;
    }
    }
    if (prepared) {
        invalidate(gui_forms::Dirty::paint);
    }
    return prepared;
}

bool ProbeView::turn() {
    const std::uint32_t next_page = (shown_page_ + 1U) % page_count;
    bool turned = true;
    switch (route_) {
    case Route::fill:
    case Route::image_swap:
    case Route::image_swap_nearest:
    case Route::image_pattern:
        invalidate(gui_forms::Dirty::paint);
        break;
    case Route::image_update: {
        const gui_forms::ImageLoadResult updated = (*window_).update_bgra32_premultiplied(
            images_[0U], width_px_, height_px_, row_bytes_, page_pixels(next_page), *this);
        if (updated.error != gui_forms::ImageResourceError::none) {
            return fail("update_bgra32_premultiplied", updated.error);
        }
        images_[0U] = updated.image;
        invalidate(gui_forms::Dirty::paint);
        break;
    }
    case Route::image_patch: {
        const gui_forms::ImageLoadResult patched = (*window_).patch_bgra32_premultiplied(
            images_[0U], 0U, 0U, width_px_, height_px_, row_bytes_, page_pixels(next_page), *this,
            client_rectangle());
        if (patched.error != gui_forms::ImageResourceError::none) {
            return fail("patch_bgra32_premultiplied", patched.error);
        }
        images_[0U] = patched.image;
        invalidate(gui_forms::Dirty::paint);
        break;
    }
    case Route::image_strips:
        for (std::uint32_t strip = 0U; strip < strip_count; ++strip) {
            const std::uint32_t rows = strip_top_[strip + 1U] - strip_top_[strip];
            const std::size_t offset = static_cast<std::size_t>(strip_top_[strip]) * row_bytes_;
            const std::size_t bytes = static_cast<std::size_t>(rows) * row_bytes_;
            const gui_forms::ImageLoadResult updated = (*window_).update_bgra32_premultiplied(
                strips_[strip], width_px_, rows, row_bytes_, page_pixels(next_page).subspan(offset, bytes),
                *this);
            if (updated.error != gui_forms::ImageResourceError::none) {
                return fail("update_bgra32_premultiplied (strip)", updated.error);
            }
            strips_[strip] = updated.image;
        }
        invalidate(gui_forms::Dirty::paint);
        break;
    case Route::surface_painter:
        turned = publish_surface(next_page);
        invalidate(gui_forms::Dirty::paint);
        break;
    case Route::surface_held:
        turned = publish_surface(next_page);
        break;
    case Route::surface_turn: {
        turned = publish_surface(next_page);
        add_child(registrant_);
        registrant_attached_ = true;
        const bool queued = (*window_).queue_live_surface_presentation(registrant_, surface_);
        if (!queued) {
            failure_ = "queue_live_surface_presentation refused the registrant";
            turned = false;
        }
        invalidate(gui_forms::Dirty::paint);
        break;
    }
    case Route::raster_canvas: {
        turned = write_bitmap(next_page);
        const bool synchronized = (*canvas_).synchronize_bitmap();
        if (!synchronized) {
            return fail("RasterCanvas::synchronize_bitmap", (*canvas_).last_resource_error());
        }
        (*canvas_).invalidate(gui_forms::Dirty::paint);
        break;
    }
    case Route::book_reading:
    case Route::book_study:
        // Forward a spread at a time, and from the last back to the first.
        if (!(*book_).turn(1)) {
            (*book_).show_page(0U);
        }
        break;
    }
    shown_page_ = next_page;
    return turned;
}

void ProbeView::release_turn_registration() {
    if (!registrant_attached_) {
        return;
    }
    static_cast<void>(remove_child((*registrant_).runtime_id()));
    registrant_attached_ = false;
}

bool ProbeView::surface_sampled() const {
    if (!surface_) {
        return false;
    }
    const gui_forms::LiveSurfaceSnapshot snapshot = (*surface_).snapshot();
    const bool sampled = snapshot.last_read_generation >= published_generation_;
    return sampled;
}

void ProbeView::note_paint_end() {
    last_paint_end_ = Clock::now();
    ++paints_;
    if (painted) {
        painted();
    }
}

void ProbeView::on_paint(gui_forms::Painter& painter, gui_forms::Rect local_damage) {
    static_cast<void>(local_damage);
    const gui_forms::Rect client = client_rectangle();
    switch (route_) {
    case Route::fill: {
        const gui_forms::Color first = gui_forms::Color::rgba(0xf0U, 0xecU, 0xe0U);
        const gui_forms::Color second = gui_forms::Color::rgba(0xe6U, 0xe0U, 0xd0U);
        painter.fill_rect(client, shown_page_ == 0U ? first : second);
        break;
    }
    case Route::image_swap:
        painter.draw_image(images_[shown_page_], client);
        break;
    case Route::image_swap_nearest: {
        const gui_forms::Rect source{0.0, 0.0, static_cast<double>(width_px_), static_cast<double>(height_px_)};
        painter.draw_image_region_sampled(images_[shown_page_], source, client, gui_forms::ImageSampling::nearest);
        break;
    }
    case Route::image_pattern: {
        const gui_forms::Size source_size{static_cast<double>(width_px_), static_cast<double>(height_px_)};
        const gui_forms::Size tile_size{client.width, client.height};
        painter.fill_image_pattern(images_[shown_page_], source_size, client, tile_size);
        break;
    }
    case Route::image_update:
    case Route::image_patch:
        painter.draw_image(images_[0U], client);
        break;
    case Route::image_strips:
        for (std::uint32_t strip = 0U; strip < strip_count; ++strip) {
            const double top = client.height * static_cast<double>(strip_top_[strip]) / height_px_;
            const double bottom = client.height * static_cast<double>(strip_top_[strip + 1U]) / height_px_;
            const gui_forms::Rect destination{client.x, client.y + top, client.width, bottom - top};
            painter.draw_image(strips_[strip], destination);
        }
        break;
    case Route::surface_painter:
    case Route::surface_held:
    case Route::surface_turn:
        painter.draw_live_surface(surface_, client);
        break;
    case Route::raster_canvas:
    case Route::book_reading:
    case Route::book_study:
        // The child paints; this view has nothing of its own.
        return;
    }
    note_paint_end();
}

// ---- The run -----------------------------------------------------------------

// State of one probe run, owned by run_probe and held weakly by the
// callbacks the toolkit retains.
struct ProbeRun final {
    ProbeOptions options{};
    // Borrowed; the toolkit stops window callbacks before the window dies.
    gui_forms::Window* window{nullptr};
    std::weak_ptr<ProbeView> view{};
    gui_forms::ApplicationWindowHandle handle{};
    gui_forms::FrameRequestToken timer{};

    std::vector<TurnRecord> turns{};
    bool turn_in_flight{false};
    bool observer_pending{false};
    Clock::time_point request_time{};
    double update_ms{0.0};
    std::uint64_t frames_before{0U};
    std::uint64_t present_ns_before{0U};
    std::uint64_t paints_before{0U};
    // Ticks that found the previous turn still unpresented.
    std::uint32_t skipped_ticks{0U};
    std::uint64_t paint_passes_at_first_turn{0U};
    std::uint64_t paint_passes_after_last_turn{0U};

    bool prepared{false};
    std::string failure{};

    bible::core::ProcessCounters idle_begin_counters{};
    WakeCounters idle_begin_wakes{};
    Clock::time_point idle_begin_time{};
    bool completed{false};
};

void post_observer(const std::shared_ptr<ProbeRun>& run);

// Decides whether the turn in flight has been presented; records it if so,
// otherwise posts itself again until observer_time_limit has passed.
struct PresentObserver final {
    std::weak_ptr<ProbeRun> run;

    void operator()() const {
        const std::shared_ptr<ProbeRun> live = run.lock();
        if (!live) {
            return;
        }
        (*live).observer_pending = false;
        const std::shared_ptr<ProbeView> view = (*live).view.lock();
        if (!view || !(*live).turn_in_flight) {
            return;
        }
        const Clock::time_point now = Clock::now();
        const gui_forms::MetricsSnapshot snapshot = (*(*live).window).metrics().snapshot();
        bool presented = false;
        if ((*live).options.route == Route::surface_held) {
            presented = (*view).surface_sampled();
        } else {
            presented = snapshot.frames_presented > (*live).frames_before;
        }
        const double elapsed_ms = milliseconds_between((*live).request_time, now);
        const std::chrono::duration<double, std::milli> limit = observer_time_limit;
        if (!presented && elapsed_ms < limit.count()) {
            post_observer(live);
            return;
        }

        TurnRecord record;
        record.presented = presented;
        record.update_ms = (*live).update_ms;
        record.observed_ms = elapsed_ms;
        if ((*view).paints() > (*live).paints_before) {
            record.recorded_ms = milliseconds_between((*live).request_time, (*view).last_paint_end());
        }
        const std::uint64_t present_ns = snapshot.present_duration_nanoseconds - (*live).present_ns_before;
        record.host_present_ms = static_cast<double>(present_ns) / 1'000'000.0;
        (*live).turns.push_back(record);
        (*live).turn_in_flight = false;
        (*view).release_turn_registration();
    }
};

void post_observer(const std::shared_ptr<ProbeRun>& run) {
    if ((*run).observer_pending) {
        return;
    }
    (*run).observer_pending = true;
    static_cast<void>((*(*run).window).begin_invoke(PresentObserver{run}));
}

// The view's and the canvas's end-of-paint notice. A posted callback runs
// after the host has finished the paint that called on_paint, so the observer
// it posts sees that frame's presentation.
struct PaintedNotice final {
    std::weak_ptr<ProbeRun> run;

    void operator()() const {
        const std::shared_ptr<ProbeRun> live = run.lock();
        if (!live || !(*live).turn_in_flight) {
            return;
        }
        post_observer(live);
    }
};

double percentile(const std::vector<double>& sorted, double fraction) {
    // Nearest rank: the smallest value with at least `fraction` of the
    // sample at or below it.
    const double rank = std::ceil(fraction * static_cast<double>(sorted.size()));
    const std::size_t index = rank < 1.0 ? 0U : static_cast<std::size_t>(rank) - 1U;
    const double value = sorted[std::min(index, sorted.size() - 1U)];
    return value;
}

enum class TurnField : std::uint8_t { update, recorded, observed, host_present };

double turn_field(const TurnRecord& record, TurnField field) {
    switch (field) {
    case TurnField::update:
        return record.update_ms;
    case TurnField::recorded:
        return record.recorded_ms;
    case TurnField::observed:
        return record.observed_ms;
    case TurnField::host_present:
        return record.host_present_ms;
    }
    return 0.0;
}

void print_distribution(std::string_view name, const std::vector<TurnRecord>& turns, TurnField field) {
    std::vector<double> values;
    values.reserve(turns.size());
    for (const TurnRecord& record : turns) {
        values.push_back(turn_field(record, field));
    }
    std::sort(values.begin(), values.end());
    std::cout << "\"" << name << "\":{";
    if (values.empty()) {
        std::cout << "\"count\":0}";
        return;
    }
    std::cout << "\"count\":" << values.size() << ",\"min\":" << values.front()
              << ",\"p50\":" << percentile(values, 0.50) << ",\"p95\":" << percentile(values, 0.95)
              << ",\"max\":" << values.back() << "}";
}

// Ends the idle sample, prints the one result line and closes the window.
struct IdleEnd final {
    std::weak_ptr<ProbeRun> run;

    void operator()(gui_forms::FrameTime) const {
        const std::shared_ptr<ProbeRun> live = run.lock();
        if (!live) {
            return;
        }
        (*live).timer.disconnect();
        const std::shared_ptr<ProbeView> view = (*live).view.lock();
        if (!view) {
            return;
        }
        gui_forms::Window& window = *(*live).window;
        const Clock::time_point end_time = Clock::now();
        const bible::core::ProcessCounters end_counters = bible::core::read_process_counters();
        const WakeCounters end_wakes = read_wake_counters();
        const gui_forms::MetricsSnapshot snapshot = window.metrics().snapshot();
        const bible::core::ProcessCounters& begin_counters = (*live).idle_begin_counters;
        const WakeCounters& begin_wakes = (*live).idle_begin_wakes;
        const gui_forms::Size client = window.client_size();
        const std::uint64_t page_bytes =
            static_cast<std::uint64_t>((*view).device_width()) * (*view).device_height() * 4U;
        std::uint32_t presented_turns = 0U;
        for (const TurnRecord& record : (*live).turns) {
            if (record.presented) {
                ++presented_turns;
            }
        }
        const std::uint64_t turn_paint_passes =
            (*live).paint_passes_after_last_turn - (*live).paint_passes_at_first_turn;

        std::cout << "{\"page_route_probe\":{\"route\":\"" << (*live).options.route_name << "\""
                  << ",\"renderer\":\"" << snapshot.renderer_name << "\""
                  << ",\"window_scale\":" << window.scale() << ",\"client_width\":" << client.width
                  << ",\"client_height\":" << client.height << ",\"device_width\":" << (*view).device_width()
                  << ",\"device_height\":" << (*view).device_height() << ",\"page_bytes\":" << page_bytes
                  << ",\"turns\":" << (*live).turns.size() << ",\"presented_turns\":" << presented_turns
                  << ",\"skipped_ticks\":" << (*live).skipped_ticks
                  << ",\"paint_passes_during_turns\":" << turn_paint_passes << ",";
        print_distribution("observed_ms", (*live).turns, TurnField::observed);
        std::cout << ",";
        print_distribution("update_ms", (*live).turns, TurnField::update);
        std::cout << ",";
        print_distribution("recorded_ms", (*live).turns, TurnField::recorded);
        std::cout << ",";
        print_distribution("host_present_ms", (*live).turns, TurnField::host_present);
        std::cout << ",\"observed_all_ms\":[";
        for (std::size_t index = 0U; index < (*live).turns.size(); ++index) {
            if (index > 0U) {
                std::cout << ",";
            }
            std::cout << (*live).turns[index].observed_ms;
        }
        // frame_deadlines_fired and scheduler_wakes include the one tick that
        // delivers this callback and so ends the sample.
        std::cout << "],\"idle\":{\"sample_ms\":" << milliseconds_between((*live).idle_begin_time, end_time)
                  << ",\"cpu_user_ns\":" << (end_counters.cpu_user_ns - begin_counters.cpu_user_ns)
                  << ",\"cpu_kernel_ns\":" << (end_counters.cpu_kernel_ns - begin_counters.cpu_kernel_ns)
                  << ",\"cycles\":" << (end_wakes.cycles - begin_wakes.cycles)
                  << ",\"context_switches\":" << (end_wakes.context_switches - begin_wakes.context_switches)
                  << ",\"paint_passes\":" << snapshot.paint_passes
                  << ",\"frames_presented\":" << snapshot.frames_presented
                  << ",\"frame_deadlines_fired\":" << snapshot.frame_deadlines_fired
                  << ",\"scheduler_wakes\":" << snapshot.scheduler_wakes
                  << ",\"active_surface_ticks\":" << snapshot.active_surface_ticks
                  << ",\"live_surface_registered\":"
                  << (window.has_live_surface_presentations() ? "true" : "false") << "}"
                  << ",\"private_bytes\":" << end_counters.private_bytes
                  << ",\"resident_bytes\":" << end_counters.resident_bytes << "}}" << std::endl;

        const bool counters_valid = begin_counters.valid && end_counters.valid;
        (*live).completed = counters_valid && presented_turns == (*live).turns.size();
        static_cast<void>((*live).handle.request_close());
    }
};

// Opens the idle sample after the warm-up.
struct IdleBegin final {
    std::weak_ptr<ProbeRun> run;

    void operator()(gui_forms::FrameTime) const {
        const std::shared_ptr<ProbeRun> live = run.lock();
        const std::shared_ptr<ProbeView> view = live ? (*live).view.lock() : nullptr;
        if (!live || !view) {
            return;
        }
        gui_forms::Window& window = *(*live).window;
        window.reset_activity_metrics();
        (*live).idle_begin_counters = bible::core::read_process_counters();
        (*live).idle_begin_wakes = read_wake_counters();
        (*live).idle_begin_time = Clock::now();
        const std::chrono::seconds sample((*live).options.idle_seconds);
        (*live).timer =
            window.schedule_ui_timer(*view, sample, gui_forms::FrameClock::now() + sample, IdleEnd{live});
    }
};

// One tick is one page turn, started from rest.
struct TurnTick final {
    std::weak_ptr<ProbeRun> run;

    void operator()(gui_forms::FrameTime) const {
        const Clock::time_point request_time = Clock::now();
        const std::shared_ptr<ProbeRun> live = run.lock();
        const std::shared_ptr<ProbeView> view = live ? (*live).view.lock() : nullptr;
        if (!live || !view) {
            return;
        }
        gui_forms::Window& window = *(*live).window;
        if ((*live).turn_in_flight) {
            ++(*live).skipped_ticks;
            return;
        }
        const gui_forms::MetricsSnapshot snapshot = window.metrics().snapshot();
        if ((*live).turns.size() >= (*live).options.turns) {
            (*live).paint_passes_after_last_turn = snapshot.paint_passes;
            (*live).timer = window.schedule_ui_timer(*view, idle_warm_up, gui_forms::FrameClock::now() + idle_warm_up,
                                                     IdleBegin{live});
            return;
        }
        if ((*live).turns.empty()) {
            (*live).paint_passes_at_first_turn = snapshot.paint_passes;
        }
        (*live).request_time = request_time;
        (*live).frames_before = snapshot.frames_presented;
        (*live).present_ns_before = snapshot.present_duration_nanoseconds;
        (*live).paints_before = (*view).paints();
        (*live).turn_in_flight = true;
        const bool turned = (*view).turn();
        (*live).update_ms = milliseconds_between(request_time, Clock::now());
        if (!turned) {
            (*live).failure = (*view).failure();
            static_cast<void>((*live).handle.request_close());
            return;
        }
        if ((*live).options.route == Route::surface_held) {
            // No paint follows a publish on this route; the observer polls.
            post_observer(live);
        }
    }
};

struct ProbeReady final {
    std::weak_ptr<ProbeView> view;
    std::shared_ptr<ProbeRun> run;

    void operator()(gui_forms::Window& window, gui_forms::ApplicationWindowHandle handle) const {
        const std::shared_ptr<ProbeView> live_view = view.lock();
        if (!live_view) {
            return;
        }
        (*run).window = &window;
        (*run).handle = handle;
        (*live_view).painted = PaintedNotice{run};
        (*run).prepared = (*live_view).prepare(window, live_view, (*run).options.route);
        if (!(*run).prepared) {
            (*run).failure = (*live_view).failure();
            static_cast<void>(handle.request_close());
            return;
        }
        (*run).timer = window.schedule_ui_timer(*live_view, turn_interval,
                                                gui_forms::FrameClock::now() + settle_before_first_turn,
                                                TurnTick{run});
    }
};

int run_probe(const ProbeOptions& options) {
    const std::shared_ptr<ProbeRun> run = std::make_shared<ProbeRun>();
    (*run).options = options;
    (*run).turns.reserve(options.turns);

    const std::shared_ptr<ProbeView> view = gui_forms::make_control<ProbeView>(gui_forms::StableId("probe.view"));
    (*run).view = view;
    const gui_forms::Size window_size{options.logical_width, options.logical_height};
    std::unique_ptr<gui_forms::Window> window = std::make_unique<gui_forms::Window>(view, window_size);

    gui_forms::ApplicationWindowOptions window_options;
    window_options.title = "Pocket Bible page route probe";
    window_options.initial_size = window_size;
    window_options.minimum_size = gui_forms::Size{600.0, 420.0};
    window_options.ready = ProbeReady{view, run};

    const gui_forms::ApplicationResult result =
        gui_forms::Application::run(std::move(window), std::move(window_options));
    if (result.callback_exception) {
        std::rethrow_exception(result.callback_exception);
    }
    if (!result.accepted()) {
        std::cerr << "The native window could not run (application error " << static_cast<int>(result.error)
                  << ")." << std::endl;
        return 1;
    }
    if (!(*run).failure.empty()) {
        std::cerr << "Route " << options.route_name << " failed: " << (*run).failure << std::endl;
        return 1;
    }
    if (!(*run).completed) {
        std::cerr << "Route " << options.route_name << " closed before every turn was presented." << std::endl;
        return 1;
    }
    return 0;
}

// ---- Arguments ---------------------------------------------------------------

void print_usage() {
    std::cerr << "usage: page_route_probe --route NAME [--turns N] [--idle SECONDS] [--width W] [--height H]\n"
              << "  routes:";
    for (const RouteEntry& entry : route_table) {
        std::cerr << " " << entry.name;
    }
    std::cerr << "\n  --turns N    page turns to time, 1..1000 (default 30)\n"
              << "  --idle S     idle sample in seconds, 1..600 (default 10)\n"
              << "  --width W --height H   logical window size (default 1060 x 680)\n";
}

bool parse_number(std::string_view text, std::uint32_t minimum, std::uint32_t maximum,
                  std::uint32_t& destination) {
    std::uint32_t value = 0U;
    const char* first = text.data();
    const char* last = text.data() + text.size();
    const std::from_chars_result parsed = std::from_chars(first, last, value);
    if (parsed.ec != std::errc{} || parsed.ptr != last || value < minimum || value > maximum) {
        return false;
    }
    destination = value;
    return true;
}

bool parse_route(std::string_view text, ProbeOptions& options) {
    for (const RouteEntry& entry : route_table) {
        if (entry.name == text) {
            options.route = entry.route;
            options.route_name = entry.name;
            return true;
        }
    }
    return false;
}

bool parse_arguments(int argc, char** argv, ProbeOptions& options) {
    bool route_given = false;
    for (int index = 1; index + 1 < argc; index += 2) {
        const std::string_view key(argv[index]);
        const std::string_view value(argv[index + 1]);
        std::uint32_t number = 0U;
        if (key == "--route") {
            route_given = parse_route(value, options);
            if (!route_given) {
                return false;
            }
        } else if (key == "--turns" && parse_number(value, 1U, 1000U, number)) {
            options.turns = number;
        } else if (key == "--idle" && parse_number(value, 1U, 600U, number)) {
            options.idle_seconds = number;
        } else if (key == "--width" && parse_number(value, 600U, 4096U, number)) {
            options.logical_width = static_cast<double>(number);
        } else if (key == "--height" && parse_number(value, 420U, 4096U, number)) {
            options.logical_height = static_cast<double>(number);
        } else {
            return false;
        }
    }
    const bool complete = route_given && argc % 2 == 1;
    return complete;
}

} // namespace

int main(int argc, char** argv) {
    try {
        ProbeOptions options;
        if (!parse_arguments(argc, argv, options)) {
            print_usage();
            return 2;
        }
        const int exit_code = run_probe(options);
        return exit_code;
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
}
