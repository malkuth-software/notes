# Extract raw CFF pieces (charstrings, global and local subrs, fd select) to a pickle for study.
import sys,pickle
from fontTools.ttLib import TTFont
f=TTFont(sys.argv[1],lazy=True)
top=f['CFF '].cff[0]
cs=top.CharStrings
order=f.getGlyphOrder()
chars=[];fds=[]
for g in order:
    c=cs[g]  # T2CharString not decompiled
    chars.append(bytes(c.bytecode))
    fds.append(cs.getItemAndSelector(g)[1] if hasattr(cs,'getItemAndSelector') else 0)
gs=[bytes(s.bytecode) for s in top.GlobalSubrs]
ls=[]
for fd in top.FDArray:
    p=fd.Private
    ls.append([bytes(s.bytecode) for s in p.Subrs] if hasattr(p,'Subrs') else [])
pickle.dump(dict(chars=chars,fds=fds,gs=gs,ls=ls),open(sys.argv[2],'wb'))
print(len(chars),'glyphs',sum(map(len,chars)),'bytes; gsubrs',len(gs),sum(map(len,gs)),'; local',[(len(l),sum(map(len,l))) for l in ls])
