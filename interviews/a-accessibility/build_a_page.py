import base64
import pathlib
import re

here = pathlib.Path(__file__).resolve().parent
t2 = pathlib.Path("D:/gui_forms_workers/t2_interview/page_source.html").read_text(encoding="utf-8")
groups = (here / "groups.js").read_text(encoding="utf-8")


def swap(text, old, new):
    assert text.count(old) == 1, old[:70]
    return text.replace(old, new)


page = t2
page = swap(page, "<title>T2 Engine Interview</title>", "<title>Accessibility Vocabulary Interview</title>")
page = swap(page, '<div class="eyebrow">GUI.Forms · stage T2 · engine core · interview round 1</div>',
            '<div class="eyebrow">GUI.Forms · stage A · accessibility · vocabulary round 1</div>')
page = swap(page, "<h1>T2 Engine Interview</h1>", "<h1>Accessibility Vocabulary Interview</h1>")
page = re.sub(r'<p class="lede">The theme engine\'s first stage waits on these decisions\..*?</p>',
              '<p class="lede">Stage A (release 0.2) now verifies every stock control through MSAA, UI Automation and AT-SPI on CI, '
              'with zero mismatches. What remains needs vocabulary, which is yours to admit: the roles, fields and text '
              'support the workers found missing on every host. Each question explains its options and my recommendation. '
              'Your answers are kept in this browser; save them to a file or copy the record and give it to the coordinating '
              'session, which records them in the stage A brief and hands them to the workers.</p>', page, count=1, flags=re.S)
start = page.index('<div class="context">')
end = page.index('<main id="groups"></main>')
page = page[:start] + '''<div class="context">
    <div class="panel">
      <h2>Where stage A stands</h2>
      <p>Merged: one routing core for every host (by runtime ID, so a disposed control is refused); the MSAA and UI Automation clients over the whole Gallery on Windows (2,792 elements, 0 mismatches, 96 of 96 default actions through their patterns); the AT-SPI client over the whole Gallery on Linux (2,743 nodes, 0 mismatches); focus for composite controls on every host. Found on the way and fixed: eight memory-safety defects of the same kind in controls.</p>
      <p>Already decided (DK-51): a live-region role for the docking move mode's announcements, built in this stage. Not asked here.</p>
    </div>
  </div>

  ''' + page[end:]
gstart = page.index("const GROUPS = [")
gend = page.index("// ------------------------------------------------------------- rendering")
page = page[:gstart] + groups + "\n" + page[gend:]
rstart = page.index("// ------------------------------------------------------------- rendering")
rend = page.index("// ---------------------------------------------------------------- answers")
page = page[:rstart] + "const VISUALS = {};\n\n" + page[rend:]
page = swap(page, 'function localKey() { return "t2-interview-round-1"; }', 'function localKey() { return "a-vocabulary-round-1"; }')
page = swap(page, 'interview: "T2 engine core, round 1"', 'interview: "Stage A accessibility vocabulary, round 1"')
page = swap(page, 'link.download = "t2-interview-answers.json";', 'link.download = "a-vocabulary-answers.json";')
page = swap(page, 'textContent = "Saved t2-interview-answers.json to your downloads.";', 'textContent = "Saved a-vocabulary-answers.json to your downloads.";')
page = swap(page, 'state.textContent = "That file is not a saved T2 interview; nothing was loaded.";', 'state.textContent = "That file is not a saved stage A interview; nothing was loaded.";')
page = swap(page, 'const lines = ["T2 interview, round 1"];', 'const lines = ["Stage A accessibility vocabulary, round 1"];')
page = re.sub(r'<footer class="next panel">.*?</footer>',
              '''<footer class="next panel">
    <h2>What happens with your answers</h2>
    <p>They become GIVEN decisions in <span class="mono">planning/stages/A-accessibility/README.md</span> and the design language's §12, and the vocabulary joins the schema. Then the workers build them on every host: the roles and fields through MSAA, UI Automation, AT-SPI and macOS AX, and editable text through AT-SPI Text, UI Automation TextPattern and macOS text attributes, each proven by the Gallery-wide client tests on CI.</p>
  </footer>''', page, count=1, flags=re.S)
(here / "page_source.html").write_text(page, encoding="utf-8", newline="\n")

fonts = pathlib.Path("C:/Users/Shadow/gui_forms/assets/fonts")
faces = [("Portsmouth Rapids", 400, "PortsmouthRapids.ttf"), ("Portsmouth Rapids", 700, "PortsmouthRapids-Bold.ttf"), ("Portsmouth Mono", 400, "PortsmouthMono.ttf")]
rules = []
for family, weight, name in faces:
    data = base64.b64encode((fonts / name).read_bytes()).decode("ascii")
    rules.append(f'@font-face {{ font-family: "{family}"; font-weight: {weight}; font-style: normal; font-display: swap; src: url(data:font/ttf;base64,{data}) format("truetype"); }}')
built = page.replace("/*FONTS*/", "/* Portsmouth Rapids and Portsmouth Mono, MIT, the repository owner's typefaces. */\n" + "\n".join(rules))
(here / "a-vocabulary-interview.html").write_text(built, encoding="utf-8", newline="\n")
print("bytes", len(built.encode("utf-8")), "questions", groups.count('{ id: "'))
