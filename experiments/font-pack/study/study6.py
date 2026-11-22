# Near-repeat test: predict each number from the number that followed the most
# recent earlier occurrence of the same k preceding numbers, and look at the
# residual. Tests whether CJK component reuse with small variations is exploitable.
import pickle,math,collections,sys
P='/private/tmp/claude-501/-Users-ultimussecundai--claude-spares-spare-e9tPAG/bb9ebc2c-6ee1-4261-bbb1-72e670b7053c/scratchpad/b/'
groups=pickle.load(open(P+'groups.pkl','rb'))
def val(r):
    b=r[0]
    if b==28: return int.from_bytes(r[1:3],'big',signed=True)
    if b<247: return b-139
    if b<251: return (b-247)*256+r[1]+108
    if b<255: return -(b-251)*256-r[1]-108
    return 99999
vals=[];ctx=[]
for g in groups:
    for op,args,m in g:
        n=len(args)
        for j,r in enumerate(args):
            vals.append(val(r)); ctx.append((op,min(n,13),min(j,13)))
N=len(vals); print('numbers',N)
def H(c):
    t=sum(c.values()); return -sum(v*math.log2(v/t) for v in c.values())
base=collections.defaultdict(collections.Counter)
for v,c in zip(vals,ctx): base[c][v]+=1
print('baseline bits/number (op,n,j ctx, static)',sum(H(c) for c in base.values())/N)
for k in (2,3,4):
    last={}; res=collections.Counter(); hit=0; exact=0; near=0
    resctx=collections.defaultdict(collections.Counter); miss=collections.defaultdict(collections.Counter)
    for i in range(N):
        if i>=k:
            key=tuple(vals[i-k:i])
            p=last.get(key)
            if p is not None:
                d=vals[i]-vals[p]; hit+=1
                if d==0: exact+=1
                if abs(d)<=3: near+=1
                resctx[ctx[i]][d]+=1
            else: miss[ctx[i]][vals[i]]+=1
            last[key]=i
        else: miss[ctx[i]][vals[i]]+=1
    hb=sum(H(c) for c in resctx.values()); mb=sum(H(c) for c in miss.values())
    print(f'k={k}: predicted {hit/N:.3f} of numbers; exact {exact/hit:.3f}, within 3: {near/hit:.3f}; residual {hb/hit:.2f} bits each; unpredicted {mb/(N-hit):.2f} bits each; total {(hb+mb)/8/1e6:.2f} MB')
