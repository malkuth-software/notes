import pickle,math,collections
from axes import axes
P='/private/tmp/claude-501/-Users-ultimussecundai--claude-spares-spare-e9tPAG/bb9ebc2c-6ee1-4261-bbb1-72e670b7053c/scratchpad/b/'
groups=pickle.load(open(P+'groups.pkl','rb'))
def val(r):
    b=r[0]
    if b==28: return int.from_bytes(r[1:3],'big',signed=True)
    if b<247: return b-139
    if b<251: return (b-247)*256+r[1]+108
    if b<255: return -(b-251)*256-r[1]-108
    return 99999
def bk(v):
    a=abs(v); s=0 if a==0 else (1 if a<4 else 2 if a<11 else 3 if a<26 else 4 if a<61 else 5 if a<108 else 6)
    return s if v>=0 else -s
def bk2(v):
    a=abs(v); s=0 if a==0 else (1 if a<3 else 2 if a<6 else 3 if a<11 else 4 if a<18 else 5 if a<26 else 6 if a<40 else 7 if a<61 else 8 if a<108 else 9 if a<200 else 10)
    return s if v>=0 else -s
# rows: (base ctx, list of prev values in item) streamed; evaluate adaptive cost for several context functions
models={
 'base':lambda b,p:(b,),
 'p1':lambda b,p:(b,bk(p[-1])),
 'p2':lambda b,p:(b,bk(p[-2])),
 'p1p2':lambda b,p:(b,bk(p[-1]),bk(p[-2])),
 'p1f p2f':lambda b,p:(b,bk2(p[-1]),bk2(p[-2])),
 'p2 p6':lambda b,p:(b,bk(p[-2]),bk(p[-6])),
 'p1 p2 p3':lambda b,p:(b,bk(p[-1]),bk(p[-2]),bk(p[-3])),
 'p1 p2 p6':lambda b,p:(b,bk(p[-1]),bk(p[-2]),bk(p[-6])),
 'exact p2':lambda b,p:(b,p[-2]),
 'exact p1':lambda b,p:(b,p[-1]),
}
cnt={k:collections.defaultdict(dict) for k in models}; tot={k:collections.defaultdict(int) for k in models}
cost={k:0.0 for k in models}
bc=collections.defaultdict(dict); bt=collections.defaultdict(int)
log2=math.log2
for gi,g in enumerate(groups):
    p=[0]*6
    for op,args,m in g:
        n=len(args)
        for j,r in enumerate(args):
            v=val(r); b=(op,min(n,13),min(j,13))
            # base adaptive cost (escape to uniform 2^-11)
            d=bc[b]; t=bt[b]; c=d.get(v,0)
            basecost = -log2(c/(t+1)) if c else (log2(t+1)+11)
            d[v]=c+1; bt[b]=t+1
            for k,f in models.items():
                if k=='base': cost[k]+=basecost; continue
                x=f(b,p); d=cnt[k][x]; t=tot[k][x]; c=d.get(v,0)
                cost[k]+= -log2(c/(t+1)) if c else (log2(t+1)+basecost)
                d[v]=c+1; tot[k][x]=t+1
            p.append(v)
        if op not in (10,29): pass
for k in models: print(f'{k:12} {cost[k]/8:12.0f} ctxs {len(cnt[k]) if k!="base" else len(bc)}')
