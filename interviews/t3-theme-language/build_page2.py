import base64
import pathlib
import re

here = pathlib.Path(__file__).resolve().parent
base = pathlib.Path("D:/gui_forms_workers/a_interview/page_source.html").read_text(encoding="utf-8")
groups = (here / "groups2.js").read_text(encoding="utf-8")


def swap(text, old, new):
    assert text.count(old) == 1, old[:70]
    return text.replace(old, new)


page = base
page = swap(page, "<title>Accessibility Vocabulary Interview</title>", "<title>T3 Theme Language, Round 2</title>")
page = swap(page, '<div class="eyebrow">GUI.Forms · stage A · accessibility · vocabulary round 1</div>',
            '<div class="eyebrow">GUI.Forms · stage T3 · theme language · interview round 2</div>')
page = swap(page, "<h1>Accessibility Vocabulary Interview</h1>", "<h1>T3 Theme Language, Round 2</h1>")
page = re.sub(r'<p class="lede">Stage A \(release 0\.2\).*?</p>',
              '<p class="lede">Round 2 settles the questions the T3 plan raised (PLAN.md, PR #316). Section A holds the decisions only you can make; section B holds smaller technical calls where I would accept the plan’s recommendations. Your answers are kept in this browser; save them to a file or copy the record back to the coordinating session.</p>',
              page, count=1, flags=re.S)
start = page.index('<div class="context">')
end = page.index('<main id="groups"></main>')
page = page[:start] + '''<div class="context">
    <div class="panel">
      <h2>Where T3 stands</h2>
      <p>Round 1 is recorded in the T3 brief: CSS property names lowered onto engine parameters plus engine names for what CSS lacks; shorthands; px, percentages and em; colour roles first; theme variables with engine inputs read-only; :is(); themes compiled, where unknown properties warn and bad values refuse the theme; transitions and keyframes on published states; one sheet with @media; @import by name; a JSON manifest; inline styles from C++ and form documents with your !important rule, high contrast still winning on colour; the hostile-input limits; the parser internal until stage V.</p>
      <p>The plan found six engine changes T3 needs, each its own phase. The biggest: the material primitive takes only a recipe and a shape, so CSS backgrounds, borders and shadows have nothing to land on yet. Colour roles don't exist in code yet either. Plan size: 11 phases, about 20 to 24 PRs.</p>
    </div>
  </div>

  ''' + page[end:]
gstart = page.index("const GROUPS = [")
gend = page.index("const VISUALS = {};")
page = page[:gstart] + groups + "\n" + page[gend:]
page = swap(page, 'function localKey() { return "a-vocabulary-round-1"; }', 'function localKey() { return "t3-theme-language-round-2"; }')
page = swap(page, 'interview: "Stage A accessibility vocabulary, round 1"', 'interview: "T3 theme language, round 2"')
page = swap(page, 'link.download = "a-vocabulary-answers.json";', 'link.download = "t3-round2-answers.json";')
page = swap(page, 'textContent = "Saved a-vocabulary-answers.json to your downloads.";', 'textContent = "Saved t3-round2-answers.json to your downloads.";')
page = swap(page, 'state.textContent = "That file is not a saved stage A interview; nothing was loaded.";', 'state.textContent = "That file is not a saved T3 round 2; nothing was loaded.";')
page = swap(page, 'const lines = ["Stage A accessibility vocabulary, round 1"];', 'const lines = ["T3 theme language, round 2"];')
page = re.sub(r'<footer class="next panel">.*?</footer>',
              '''<footer class="next panel">
    <h2>What happens with your answers</h2>
    <p>They become owner decisions in <span class="mono">planning/stages/T3-theme-language/README.md</span>. Then a worker writes the T3 plan (grammar, phases and the gate) for your review before the parser is built: conformance fixtures for every selector and property form, hostile-input tests and a fuzz target, and the byte-for-byte round trip.</p>
  </footer>''', page, count=1, flags=re.S)
(here / "page_source2.html").write_text(page, encoding="utf-8", newline="\n")

fonts = pathlib.Path("C:/Users/Shadow/gui_forms/assets/fonts")
faces = [("Portsmouth Rapids", 400, "PortsmouthRapids.ttf"), ("Portsmouth Rapids", 700, "PortsmouthRapids-Bold.ttf"), ("Portsmouth Mono", 400, "PortsmouthMono.ttf")]
rules = []
for family, weight, name in faces:
    data = base64.b64encode((fonts / name).read_bytes()).decode("ascii")
    rules.append(f'@font-face {{ font-family: "{family}"; font-weight: {weight}; font-style: normal; font-display: swap; src: url(data:font/ttf;base64,{data}) format("truetype"); }}')
built = page.replace("/*FONTS*/", "/* Portsmouth Rapids and Portsmouth Mono, MIT, the repository owner's typefaces. */\n" + "\n".join(rules))
(here / "t3-round2-interview.html").write_text(built, encoding="utf-8", newline="\n")
print("bytes", len(built.encode("utf-8")), "questions", groups.count('{ id: "'))
