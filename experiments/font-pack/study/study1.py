import pickle,sys,math,collections
from t2 import *
P='/private/tmp/claude-501/-Users-ultimussecundai--claude-spares-spare-e9tPAG/bb9ebc2c-6ee1-4261-bbb1-72e670b7053c/scratchpad/b/'
d=pickle.load(open(P+'cff.pkl','rb'))
masklen={}
for gi,(c,fd) in enumerate(zip(d['chars'],d['fds'])):
    ctx=Ctx(d['gs'],d['ls'][fd]); ctx.fd=fd; ctx.masklen=masklen
    run(ctx,c,('c',gi),[[],0,False])
items=[(('c',i),c) for i,c in enumerate(d['chars'])]+[(('g',i),c) for i,c in enumerate(d['gs'])]
for fd,l in enumerate(d['ls']): items+=[(('l',fd,i),c) for i,c in enumerate(l)]
toks=[tokens(c,masklen,k) for k,c in items]
pickle.dump((items,toks),open(P+'toks.pkl','wb'))
def H(counter):
    t=sum(counter.values()); return -sum(v*math.log2(v/t) for v in counter.values())
def ctxH(pairs):
    cs=collections.defaultdict(collections.Counter)
    for c,s in pairs: cs[c][s]+=1
    return sum(H(v) for v in cs.values())/8, sum(len(v) for v in cs.values())
allb=b''.join(c for _,c in items)
print('bytes',len(allb),'o0',H(collections.Counter(allb))/8,'o1',ctxH(zip(allb,allb[1:]))[0])
kinds=collections.Counter(); 
for t in toks:
    for k,v in t: kinds[k]+=1 if k!='m' and k!='x' else len(v)
print(kinds)
ops=collections.Counter(v for t in toks for k,v in t if k=='o'); print(ops.most_common())
