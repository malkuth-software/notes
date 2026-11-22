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
def H(counter):
    t=sum(counter.values()); return -sum(v*math.log2(v/t) for v in counter.values())
hit=collections.Counter(); tot=collections.Counter()
sym=collections.defaultdict(collections.Counter)  # context -> symbol counts under candidate model
base=collections.defaultdict(collections.Counter)
for g in groups:
    hx=[];hy=[]
    for op,args,m in g:
        n=len(args); ax=axes(op,n)
        for j,r in enumerate(args):
            v=val(r); a=ax[j]
            ctx=(op,min(n,13),min(j,13))
            base[ctx][v]+=1
            if a=='?': sym[ctx][('L',v)]+=1; tot['?']+=1; continue
            h=hx if a=='x' else hy
            tot[a]+=1
            c=None
            if len(h)>=1 and v==-h[-1]: c='n1'
            elif len(h)>=2 and v==-(h[-1]+h[-2]): c='n2'
            elif len(h)>=1 and v==h[-1]: c='p1'
            elif len(h)>=2 and v==-h[-2]: c='m2'
            elif len(h)>=2 and v==h[-2]: c='p2'
            elif len(h)>=3 and v==-(h[-1]+h[-2]+h[-3]): c='n3'
            if c: hit[c]+=1; sym[ctx][c]+=1
            else: sym[ctx][('L',v)]+=1
            h.append(v)
print(tot,hit)
print('base value entropy bytes',sum(H(c) for c in base.values())/8,' with candidates',sum(H(c) for c in sym.values())/8)
