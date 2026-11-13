# Axis assignment for Type 2 operator arguments: returns a string of 'x','y','?' per argument.
def axes(op,n):
    if op==21: return ('?'*(n-2)+'xy') if n>=2 else '?'*n
    if op==22: return '?'*(n-1)+'x' if n>=1 else ''
    if op==4: return '?'*(n-1)+'y' if n>=1 else ''
    if op==5: return 'xy'*(n//2) if n%2==0 else '?'*n
    if op==6: return ''.join('xy'[j&1] for j in range(n))
    if op==7: return ''.join('yx'[j&1] for j in range(n))
    if op==8: return 'xy'*(n//2) if n%6==0 else '?'*n
    if op==24: return 'xy'*(n//2) if n%6==2 else '?'*n
    if op==25: return 'xy'*(n//2) if n>=6 and (n-6)%2==0 else '?'*n
    if op==27:
        s='y' if n%4==1 else ''
        return s+'xxyx'*(n//4) if n%4 in(0,1) else '?'*n
    if op==26:
        s='x' if n%4==1 else ''
        return s+'yxyy'*(n//4) if n%4 in(0,1) else '?'*n
    if op in(31,30):
        h = op==31; s=''; m=n
        tail = (n%8 in (1,5))
        k=n-1 if tail else n
        if k%4!=0: return '?'*n
        for q in range(k//4):
            s+= 'xxyy' if h else 'yxyx'
            h=not h
        if tail: s+= 'x' if not h else 'y'   # last curve's extra arg: for h-start last curve ends with dyf then dxf
        return s
    return '?'*n
