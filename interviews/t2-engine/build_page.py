import base64
import pathlib
import re
import sys

here = pathlib.Path(__file__).resolve().parent
fonts = pathlib.Path("C:/Users/Shadow/gui_forms/assets/fonts")
faces = [
    ("Portsmouth Rapids", 400, "normal", "PortsmouthRapids.ttf"),
    ("Portsmouth Rapids", 700, "normal", "PortsmouthRapids-Bold.ttf"),
    ("Portsmouth Mono", 400, "normal", "PortsmouthMono.ttf"),
]
rules = []
for family, weight, style, name in faces:
    data = base64.b64encode((fonts / name).read_bytes()).decode("ascii")
    rules.append(
        f'@font-face {{ font-family: "{family}"; font-weight: {weight}; font-style: {style}; '
        f'font-display: swap; src: url(data:font/ttf;base64,{data}) format("truetype"); }}')
# Portsmouth Rapids: the repository owner's typeface, MIT (assets/fonts/LICENSE-PortsmouthRapids.txt).
source = (here / "page_source.html").read_text(encoding="utf-8")
assert source.count("/*FONTS*/") == 1
page = source.replace("/*FONTS*/", "/* Portsmouth Rapids and Portsmouth Mono, MIT, the repository owner's typefaces. */\n" + "\n".join(rules))
(here / "t2-engine-interview.html").write_text(page, encoding="utf-8", newline="\n")
script = re.search(r"<script>(.*)</script>", source, re.S).group(1)
(here / "page_script.js").write_text(script, encoding="utf-8", newline="\n")
print("bytes", len(page.encode("utf-8")))
