import pickle,sys,math,collections,lzma,subprocess
P='/private/tmp/claude-501/-Users-ultimussecundai--claude-spares-spare-e9tPAG/bb9ebc2c-6ee1-4261-bbb1-72e670b7053c/scratchpad/b/'
items,toks=pickle.load(open(P+'toks.pkl','rb'))
def H(counter):
    t=sum(counter.values()); return -sum(v*math.log2(v/t) for v in counter.values())
def ctxH(pairs):
    cs=collections.defaultdict(collections.Counter)
    for c,s in pairs: cs[c][s]+=1
    return sum(H(v) for v in cs.values())/8
def val(r):
    b=r[0]
    if b==28: return int.from_bytes(r[1:3],'big',signed=True)
    if b<247: return b-139
    if b<251: return (b-247)*256+r[1]+108
    if b<255: return -(b-251)*256-r[1]-108
    return 99999
# group into (op, args) ; trailing args w/o op get op=-1
groups=[]  # per item list of (op,[raw args],mask)
for t in toks:
    g=[];args=[];i=0
    while i<len(t):
        k,v=t[i]
        if k=='n': args.append(v)
        elif k=='o':
            m=b''
            if v in(19,20): m=t[i+1][1]; i+=1
            g.append((v,args,m)); args=[]
        else: g.append((-2,args,v)); args=[]
        i+=1
    if args: g.append((-1,args,b''))
    groups.append(g)
pickle.dump(groups,open(P+'groups.pkl','wb'))
# model entropies
opp=[];nargp=[];f0=[];f1=[];f2=[];s1=[];mk=[]
A=collections.Counter()
for g in groups:
    prev=-9
    for op,args,m in g:
        opp.append((prev,op)); prev=op
        n=len(args); nargp.append((op,min(n,40)))
        A[(op,n)]+=1
        pv=[0,0]
        for j,r in enumerate(args):
            b=r[0]
            f0.append((op,b))
            f1.append(((op,j%2 if op not in(8,) else j%6),b))
            f2.append(((op,min(n,13),min(j,13)),b))
            if len(r)>1: s1.append((b,r[1:]))
        for j,x in enumerate(m): mk.append((j,x))
print('ops o1',ctxH(opp),'nargs',ctxH(nargp),'mask',ctxH(mk), 'second bytes',ctxH(s1),len(s1))
print('first byte: ctx op',ctxH(f0),' op+par',ctxH(f1),' op+n+j',ctxH(f2), len(f0))
print(A.most_common(60))
