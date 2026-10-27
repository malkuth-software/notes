"""Build a self-contained interview page from the shared template.

Usage: python build_interview.py <spec.json>
The spec names: groups (a .js file defining GROUPS), out, title, eyebrow,
lede, context_html, footer_html, key (localStorage key), download (answer
file name), record (the record's first line).
"""
import base64
import json
import pathlib
import re
import sys

spec_path = pathlib.Path(sys.argv[1]).resolve()
spec = json.loads(spec_path.read_text(encoding="utf-8"))
here = spec_path.parent
base = pathlib.Path("D:/gui_forms_workers/a_interview/page_source.html").read_text(encoding="utf-8")
groups = (here / spec["groups"]).read_text(encoding="utf-8")


def swap(text, old, new):
    assert text.count(old) == 1, old[:70]
    return text.replace(old, new)


page = base
page = swap(page, "<title>Accessibility Vocabulary Interview</title>", "<title>" + spec["title"] + "</title>")
page = swap(page, '<div class="eyebrow">GUI.Forms · stage A · accessibility · vocabulary round 1</div>',
            '<div class="eyebrow">' + spec["eyebrow"] + '</div>')
page = swap(page, "<h1>Accessibility Vocabulary Interview</h1>", "<h1>" + spec["title"] + "</h1>")
lede = re.search(r'<p class="lede">Stage A \(release 0\.2\).*?</p>', page, flags=re.S)
page = page[:lede.start()] + '<p class="lede">' + spec["lede"] + '</p>' + page[lede.end():]
start = page.index('<div class="context">')
end = page.index('<main id="groups"></main>')
page = page[:start] + '<div class="context">\n    <div class="panel">\n' + spec["context_html"] + '\n    </div>\n  </div>\n\n  ' + page[end:]
gstart = page.index("const GROUPS = [")
gend = page.index("const VISUALS = {};")
page = page[:gstart] + groups + "\n" + page[gend:]
page = swap(page, 'function localKey() { return "a-vocabulary-round-1"; }', 'function localKey() { return "' + spec["key"] + '"; }')
page = swap(page, 'interview: "Stage A accessibility vocabulary, round 1"', 'interview: "' + spec["record"] + '"')
page = swap(page, 'link.download = "a-vocabulary-answers.json";', 'link.download = "' + spec["download"] + '";')
page = swap(page, 'textContent = "Saved a-vocabulary-answers.json to your downloads.";', 'textContent = "Saved ' + spec["download"] + ' to your downloads.";')
page = swap(page, 'state.textContent = "That file is not a saved stage A interview; nothing was loaded.";', 'state.textContent = "That file is not a saved ' + spec["record"] + ' file; nothing was loaded.";')
page = swap(page, 'const lines = ["Stage A accessibility vocabulary, round 1"];', 'const lines = ["' + spec["record"] + '"];')
foot = re.search(r'<footer class="next panel">.*?</footer>', page, flags=re.S)
page = page[:foot.start()] + '<footer class="next panel">\n    <h2>What happens with your answers</h2>\n    <p>' + spec["footer_html"] + '</p>\n  </footer>' + page[foot.end():]

fonts = pathlib.Path("C:/Users/Shadow/gui_forms/assets/fonts")
faces = [("Portsmouth Rapids", 400, "PortsmouthRapids.ttf"), ("Portsmouth Rapids", 700, "PortsmouthRapids-Bold.ttf"), ("Portsmouth Mono", 400, "PortsmouthMono.ttf")]
rules = []
for family, weight, name in faces:
    data = base64.b64encode((fonts / name).read_bytes()).decode("ascii")
    rules.append(f'@font-face {{ font-family: "{family}"; font-weight: {weight}; font-style: normal; font-display: swap; src: url(data:font/ttf;base64,{data}) format("truetype"); }}')
built = page.replace("/*FONTS*/", "/* Portsmouth Rapids and Portsmouth Mono, MIT, the repository owner's typefaces. */\n" + "\n".join(rules))
(here / spec["out"]).write_text(built, encoding="utf-8", newline="\n")
print("bytes", len(built.encode("utf-8")), "questions", groups.count('{ id: "'))
