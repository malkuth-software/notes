import base64
import pathlib
import re

here = pathlib.Path(__file__).resolve().parent
base = pathlib.Path("D:/gui_forms_workers/a_interview/page_source.html").read_text(encoding="utf-8")
groups = (here / "groups.js").read_text(encoding="utf-8")


def swap(text, old, new):
    assert text.count(old) == 1, old[:70]
    return text.replace(old, new)


page = base
page = swap(page, "<title>Accessibility Vocabulary Interview</title>", "<title>T3 Theme Language Interview</title>")
page = swap(page, '<div class="eyebrow">GUI.Forms · stage A · accessibility · vocabulary round 1</div>',
            '<div class="eyebrow">GUI.Forms · stage T3 · theme language · interview round 1</div>')
page = swap(page, "<h1>Accessibility Vocabulary Interview</h1>", "<h1>T3 Theme Language Interview</h1>")
page = re.sub(r'<p class="lede">Stage A \(release 0\.2\).*?</p>',
              '<p class="lede">T3 is the theme language: the text people write to style GUI.Forms, and the parser that turns it into the '
              'engine\'s rules. Much is already decided (below). These questions settle the rest before any parser code is written. '
              'Each explains its options and my recommendation. Your answers are kept in this browser; save them to a file or copy '
              'the record and give it to the coordinating session, which records them in the T3 brief.</p>', page, count=1, flags=re.S)
start = page.index('<div class="context">')
end = page.index('<main id="groups"></main>')
page = page[:start] + '''<div class="context">
    <div class="panel">
      <h2>Already decided</h2>
      <p>Themes are a CSS-syntax subset addressed to the named anatomy (::part(), :state(), stable IDs), not a document tree. No combinators. No layout properties. Inheritance only for text and colour roles. Engine inputs are custom properties only the engine sets. Expressions are T2's bounded grammar. New effects are C++ engine primitives, never theme code. Shape is an engine parameter; themes never set geometry or hit regions. Images are named resources (CNG native, PNG for interchange). Themes are data only, may be swapped live, and need a content hash, not a signature. Names are kebab-case. Form documents share this syntax and parser. An untouched file round-trips byte for byte.</p>
      <p>The engine side is built: T2's rules, cascade, primitives and CSS coverage table (planning/stages/T2-engine-core/CSS_COVERAGE.md). Its blueprint is sealed; its optimisation and full population continue alongside T3.</p>
    </div>
  </div>

  ''' + page[end:]
gstart = page.index("const GROUPS = [")
gend = page.index("const VISUALS = {};")
page = page[:gstart] + groups + "\n" + page[gend:]
page = swap(page, 'function localKey() { return "a-vocabulary-round-1"; }', 'function localKey() { return "t3-theme-language-round-1"; }')
page = swap(page, 'interview: "Stage A accessibility vocabulary, round 1"', 'interview: "T3 theme language, round 1"')
page = swap(page, 'link.download = "a-vocabulary-answers.json";', 'link.download = "t3-interview-answers.json";')
page = swap(page, 'textContent = "Saved a-vocabulary-answers.json to your downloads.";', 'textContent = "Saved t3-interview-answers.json to your downloads.";')
page = swap(page, 'state.textContent = "That file is not a saved stage A interview; nothing was loaded.";', 'state.textContent = "That file is not a saved T3 interview; nothing was loaded.";')
page = swap(page, 'const lines = ["Stage A accessibility vocabulary, round 1"];', 'const lines = ["T3 theme language, round 1"];')
page = re.sub(r'<footer class="next panel">.*?</footer>',
              '''<footer class="next panel">
    <h2>What happens with your answers</h2>
    <p>They become owner decisions in <span class="mono">planning/stages/T3-theme-language/README.md</span>. Then a worker writes the T3 plan (grammar, phases and the gate) for your review before the parser is built: conformance fixtures for every selector and property form, hostile-input tests and a fuzz target, and the byte-for-byte round trip.</p>
  </footer>''', page, count=1, flags=re.S)
(here / "page_source.html").write_text(page, encoding="utf-8", newline="\n")

fonts = pathlib.Path("C:/Users/Shadow/gui_forms/assets/fonts")
faces = [("Portsmouth Rapids", 400, "PortsmouthRapids.ttf"), ("Portsmouth Rapids", 700, "PortsmouthRapids-Bold.ttf"), ("Portsmouth Mono", 400, "PortsmouthMono.ttf")]
rules = []
for family, weight, name in faces:
    data = base64.b64encode((fonts / name).read_bytes()).decode("ascii")
    rules.append(f'@font-face {{ font-family: "{family}"; font-weight: {weight}; font-style: normal; font-display: swap; src: url(data:font/ttf;base64,{data}) format("truetype"); }}')
built = page.replace("/*FONTS*/", "/* Portsmouth Rapids and Portsmouth Mono, MIT, the repository owner's typefaces. */\n" + "\n".join(rules))
(here / "t3-theme-language-interview.html").write_text(built, encoding="utf-8", newline="\n")
print("bytes", len(built.encode("utf-8")), "questions", groups.count('{ id: "'))
