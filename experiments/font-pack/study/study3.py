import pickle,sys,math,collections,lzma,subprocess
P='/private/tmp/claude-501/-Users-ultimussecundai--claude-spares-spare-e9tPAG/bb9ebc2c-6ee1-4261-bbb1-72e670b7053c/scratchpad/b/'
items,_=pickle.load(open(P+'toks.pkl','rb'))
groups=pickle.load(open(P+'groups.pkl','rb'))
def xz(b): return len(lzma.compress(b,preset=9|lzma.PRESET_EXTREME))
def zs(b): return len(subprocess.run(['zstd','-q','--ultra','-22','--long=27','-c'],input=b,capture_output=True).stdout)
def rep(name,streams):
    x=[xz(s) for s in streams]; z=[zs(s) for s in streams]
    print(f'{name:28} raw {sum(map(len,streams)):9} xz {sum(x):9} zstd {sum(z):9}  xz parts {x}',flush=True)
raw=b''.join(c for _,c in items)
rep('L0 raw',[raw])
l1=bytearray(); ops=bytearray(); na=bytearray(); fb=bytearray(); sb=bytearray(); mk=bytearray(); ab=bytearray(); opn=bytearray()
fx=bytearray(); fy=bytearray()
for g in groups:
    for op,args,m in g:
        if op==-2: mk+=m; continue
        o=bytes([op&255]) if op>=0 else b'\x0c'
        l1+=o+bytes([len(args)])
        ops+=o; na.append(len(args)); opn+=o+bytes([len(args)])
        for j,r in enumerate(args):
            l1+=r; ab+=r; fb.append(r[0]); sb+=r[1:]
            (fx if j%2==0 else fy).append(r[0])
        l1+=m; mk+=m
    ops.append(255)
rep('L1 opfirst single',[bytes(l1)])
rep('L2 opn|args|mask',[bytes(opn),bytes(ab),bytes(mk)])
rep('L3 op|n|first|second|mask',[bytes(ops),bytes(na),bytes(fb),bytes(sb),bytes(mk)])
rep('L4 first split parity',[bytes(fx),bytes(fy)])
