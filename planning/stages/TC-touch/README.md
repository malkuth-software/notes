# TC: touch, for embedded and industrial interfaces

Written by the coordinator, 2026-11-19, from the owner's direction of the
same day. Release: towards 1.0 (owner). Status: a stage brief; the owner's
rulings below are decided, the rest is a plan.

## 1. Why

GUI.Forms is meant for embedded hardware too: industrial interface controls
anywhere from a yacht to a machine in a plant. Many of those devices are
operated by touch, and many have no operating-system layer that handles
touch, so GUI.Forms (its appliance version) will have to handle it itself.
Where an OS does the hard work (Apple's, the owner suspects), GUI.Forms
uses it. Either way, the existing controls must adapt to touch.

## 2. The owner's direction (2026-11-19)

1. **Study how operating systems expose touch**, how they handle or offload
   false-touch suppression, and how they treat capacitive against resistive
   panels.
2. **Adapt the existing interface controls for touch**, typically:
   - larger controls;
   - a touch area larger than the visible button, where a touch is still
     valid;
   - contact treated as a press **when the finger is lifted**, unless the
     finger is very deliberately dragged off the button first.
3. **Dogfood it.**
4. **The appliance version handles touch itself** where industrial hardware
   has no OS layer for it.
5. **Hardware for dogfooding:** the Raspberry Pi 4 (the appliance's
   reference board) with two 10.1-inch Waveshare panels the owner has bought,
   new old stock, both of the newer kind that needs no manual display
   timing: one **capacitive**, one **resistive** (a Waveshare-based model
   11870). The owner also has an iPad mini (5th generation) and an iPhone SE.

**The industrial reference points** (owner):
- Allen-Bradley PanelView Plus 7 Standard: Windows CE and FactoryTalk View
  ME, analog resistive touch;
- Siemens Unified Comfort: embedded Linux with an HTML5-based runtime;
- Weintek cMT X: embedded RISC processors, typically 1 GB of RAM and 4 GB of
  flash in its standard range.

These set the class of target: modest processors and memory, resistive and
capacitive panels, often no desktop OS. A Pi 4 is a fair stand-in.

## 3. What to study (TC0)

### 3.1 How each platform exposes touch

| Platform | How touch arrives | False-touch suppression |
|---|---|---|
| iOS, iPadOS | UIKit touches with phases, contact radius and force; gesture recognisers | done by the OS (palm and accidental-touch rejection); the reference for good behaviour, testable on the owner's iPad mini and iPhone SE |
| Android | MotionEvent with pointer identities, size, pressure, tool type; the "touch slop" distance before a drag | largely in the OS and the panel's firmware |
| Windows | pointer input messages (`WM_POINTER*`) with contact area, pressure and the digitiser's confidence bit; touch injection for tests | palm rejection in digitiser firmware and the OS, reported through the confidence bit |
| macOS | no touch screens; trackpad touches only | not applicable |
| Linux | evdev multitouch protocol B (slots, tracking identities, contact width and height, pressure), read through libinput; XInput 2.2 touch events on X11; `wl_touch` on Wayland | libinput's palm detection (by contact size and edge zones); otherwise none |
| FreeBSD (the appliance) | evdev (`/dev/input/event*`, exclusive grab), with drivers for USB HID multitouch panels (`hmt`, `wmt`) and some resistive USB controllers; STXlib's appliance design already plans touch records (contact identity, position, down, move, up) | **none**: GUI.Forms must do it |

The study confirms each row on the platform's documentation and, where we
have the hardware, by recording real traces.

### 3.2 Capacitive against resistive

| | Capacitive | Resistive |
|---|---|---|
| Contacts | many at once (typically up to 10), with tracking identities | usually one; two fingers report a point between them |
| What it senses | the finger's capacitance; reports contact size | pressure joining two layers; reports position (sometimes pressure) |
| Activation | the lightest touch, so accidental touches and water drops register | needs a press, so accidental touches are rarer; works with gloves and styluses |
| Calibration | factory-calibrated | often needs a calibration matrix (three or more touch points) and can drift |
| Noise | water, electrical noise, gloves defeat it | jitter at light pressure; needs debouncing and a pressure threshold |
| Industrial use | modern panels, often with glove modes and water rejection in firmware | still common (PanelView Plus 7) for gloves, contamination and cost |

What GUI.Forms must do for each: for capacitive, reject palms, water and
grazes where the OS does not; for resistive, calibrate, debounce, smooth
jitter, apply a pressure or dwell threshold, and treat the single point
honestly (no multitouch gestures).

### 3.3 False-touch suppression where nothing does it for us

On the appliance (and on bare Linux evdev) GUI.Forms decides what counts as
a touch. To study and specify:
- contact-size limits (a palm is large);
- edge zones (a hand resting on the bezel);
- minimum contact time and pressure (grazes, resistive jitter);
- water: many simultaneous, stationary, irregular contacts;
- confirmation for critical actions: dwell-to-activate, hold-to-confirm,
  two-handed operation (the industrial practice; the usability standard for
  such equipment is IEC 62366);
- probabilistic targeting: a touch as a likelihood over nearby controls
  (contact shape, control importance), refusing when two targets are too
  close to call.

## 4. Adapting the controls

**The press rule** (owner): contact becomes a press when the finger is
lifted inside the control's touch area. A finger that slides within that
area still presses on release; one dragged deliberately beyond it cancels
(the control returns to normal, nothing fires). As in iOS's
"touch up inside", the area that keeps a press alive is larger than the
area that starts one (hysteresis), so a slightly wandering finger still
presses and only a deliberate drag-off cancels.

**Touch areas larger than the visible control.** Each control's anatomy
gains a touch inset: the hit area extends beyond the drawn bounds by a
themeable amount (for example to a minimum of 9 mm square, the common
guidance for fingers), resolved against neighbours so areas do not overlap
(nearest control wins, or a refusal when too close; section 3.3).

**Larger controls: a touch variant of the theme.** Like the dark and light
variants of one theme (owner, 2026-11-04), a theme gains a touch variant:
larger minimum sizes, padding, row heights, thumb and handle sizes, spacing,
and fonts where needed. A program can choose it, or GUI.Forms chooses it
when the primary pointer is touch.

**No hover.** Touch has no hover, so everything that relies on it needs a
touch path: tooltips on a long press, hover plates and highlights shown
while pressed instead, menus opened by tap, the window capsule's hover
title on a press-and-hold.

**Scrolling and dragging.** A movement threshold ("slop") separates a tap
from a pan; panning scrolls with momentum (the existing smooth scrolling);
the drag engine takes touch drags; pinch-zoom where a control supports
zoom (plots, canvases, Plan Paint).

**Industrial extras**, as properties on the stock controls: hold-to-confirm
buttons, dwell-to-activate, large steppers and jog wheels, guarded actions
(lift a cover, then press).

## 5. The input path

- `PointerEvent` gains a **pointer kind** (mouse, pen, touch) and, for
  touch, contact identity, contact size, pressure and the OS's confidence
  where reported. Touch keeps going through the one portable door every
  host shares (`HostSession::dispatch`), so the press rule, the touch areas
  and the suppression are core behaviour, identical on every host.
- The single-surface window manager's input records
  (`src/host/surface/input/surface_input.hpp`) gain a touch kind, for the
  appliance and any other surface host.
- Hosts: X11 through XInput 2.2 touch; Windows through `WM_POINTER`; the
  appliance through STXlib's touch records over evdev; macOS needs none.
- Suppression (section 3.3) runs in the core when the host reports that it
  has none (a host capability), and is skipped where the OS already does it.
- The MCP `pointer` tool (MCP study) gains a touch kind, so models and
  scripts can drive touch on any host, including ones without a panel.

## 6. Testing and dogfooding

- **Without a panel:** synthetic touch through the portable input records
  and the MCP tool; recorded traces from the two panels replayed in
  `gui_forms_tests` (taps, wandering presses, deliberate drag-offs, palms,
  water, resistive jitter), each with its expected outcome.
- **On the panels:** the Pi 4 with the capacitive and the resistive panel,
  running the Gallery and Plan Paint in the touch variant, and a small
  industrial demonstration panel (indicator lamps, hold-to-confirm buttons,
  steppers, a trend plot) built from stock controls.
- **The reference:** the same interactions on the iPad mini and iPhone SE,
  to compare against Apple's handling (by observation; GUI.Forms has no iOS
  host).
- Confirm each panel's touch link when they arrive (USB HID touch, which is
  the common case for HDMI panels of this kind, or SPI or I2C) and its
  controller, since that decides the driver on Linux and FreeBSD.

## 7. Stages

| Stage | Contents |
|---|---|
| TC0 | The study of section 3, with traces recorded from both panels on the Pi 4 |
| TC1 | Pointer kind and touch fields in the portable events and the surface records; X11 and Windows hosts report touch |
| TC2 | The press rule with hysteresis, touch areas in the anatomy, no-hover paths, slop and panning |
| TC3 | The touch variant of the default and familiar themes |
| TC4 | Suppression and calibration in the core for hosts without them (capacitive and resistive), with the trace tests |
| TC5 | Industrial extras on the stock controls; the demonstration panel |
| TC6 | Dogfood on the Pi 4 with both panels; the appliance host when STXlib's display and input capabilities (its K8) are ready |

## 8. Open questions

1. **Which OS on the Pi 4 for the 1.0 dogfooding.** STXlib's appliance
   design places the GUI.Forms appliance host after GUI.Forms 1.0 (its
   `C-appliance-design.md` calls the host adapter GUI.Forms 2.0 work), while
   touch is wanted towards 1.0. Proposal: dogfood touch for 1.0 on the Pi 4
   under Linux with the existing X11 host (XInput 2.2 touch), which tests the
   core's press rule, touch areas, themes and, through raw evdev mode,
   suppression; bring the FreeBSD appliance host in when STXlib's K8 lands.
2. **Touch on the iPad and iPhone:** observation only, or should GUI.Forms
   gain an iOS surface host (the 2026-10-27 direction was "the right shaped
   surface" for Android and iOS, without the Java side)?


## Owner additions, 2026-11-21 (later, towards 1.0)

- **Touch demonstrators**, one or more, for tablet and touch work. The owner
  sees a Windows 8 style start screen (a scrolling field of live tiles) as
  the ideal one: it exercises telling a flick from a tap, so a flick pans
  and a tap selects, with no accidental activation. An original design in
  that style, not a copy of Windows.
- **An e-ink theme** that uses dithering intelligently for good looks on a
  few grey levels (ordered and error-diffusion dithering chosen per surface
  kind: flat fills, gradients, images, text kept undithered), with e-ink's
  slow, ghosting refresh in mind. The owner has no e-ink device, so the
  work is checked through a simulated panel (a grey-level and refresh model
  applied to captures) rather than hardware.
