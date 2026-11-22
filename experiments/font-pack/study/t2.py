# Type 2 charstring tokenizer with stem counting through subroutine calls.
# A token is (kind, value, raw_bytes): kind 'n' number, 'o' operator, 'm' mask bytes.
def bias(n): return 107 if n<1240 else (1131 if n<33900 else 32768)
class Ctx:
    def __init__(s,gs,ls): s.gs=gs; s.ls=ls; s.gb=bias(len(gs)); s.lb=bias(len(ls)); s.masklen={}
def run(ctx,code,key,st):
    # st: [stack_depth_list, nstems, done]; records mask length per (key,offset)
    i=0; n=len(code); stack=st[0]
    while i<n:
        b=code[i]
        if b>=32 or b==28:
            if b==28: v=int.from_bytes(code[i+1:i+3],'big',signed=True); i+=3
            elif b<247: v=b-139; i+=1
            elif b<251: v=(b-247)*256+code[i+1]+108; i+=2
            elif b<255: v=-(b-251)*256-code[i+1]-108; i+=2
            else: v=None; i+=5
            stack.append(v)
        elif b in (1,3,18,23):
            st[1]+=len(stack)//2; stack.clear(); i+=1
        elif b in (19,20):
            st[1]+=len(stack)//2; stack.clear()
            ml=(st[1]+7)//8
            ctx.masklen[(key,i)]=ml
            i+=1+ml
        elif b==10:
            k=stack.pop()+ctx.lb; i+=1
            run(ctx,ctx.ls[k],('l',ctx.fd,k),st)
            if st[2]: return
        elif b==29:
            k=stack.pop()+ctx.gb; i+=1
            run(ctx,ctx.gs[k],('g',k),st)
            if st[2]: return
        elif b==11: return
        elif b==14: st[2]=True; return
        elif b==12: stack.clear(); i+=2
        else: stack.clear(); i+=1
def tokens(code,masklen,key):
    i=0;n=len(code);out=[]
    while i<n:
        b=code[i]
        if b>=32 or b==28:
            if b==28: l=3
            elif b<247: l=1
            elif b<255: l=2
            else: l=5
            out.append(('n',code[i:i+l])); i+=l
        elif b in (19,20):
            ml=masklen.get((key,i))
            if ml is None: out.append(('x',code[i:])); return out   # unreachable tail
            out.append(('o',b)); out.append(('m',code[i+1:i+1+ml])); i+=1+ml
        elif b==12: out.append(('o',256+code[i+1])); i+=2
        else: out.append(('o',b)); i+=1
    return out
