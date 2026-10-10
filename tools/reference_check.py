"""Independent NumPy least-squares and SciPy nonlinear prediction checks."""
import ctypes as C
import json
import pathlib
import subprocess
import numpy as np
from scipy.optimize import least_squares
ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = ROOT / 'build'
BUILD.mkdir(exist_ok=True)
sources = ['parser', 'model', 'eval', 'regression']
subprocess.run(['cc','-std=c11','-O2','-shared','-fPIC','-Iinclude','tests/reference_bridge.c', *[f'src/{s}.c' for s in sources],'-lm','-o',str(BUILD/'reference.so')], cwd=ROOT, check=True)
lib=C.CDLL(str(BUILD/'reference.so'))
D=C.c_double
lib.reference_fit.argtypes=[C.c_char_p,C.POINTER(D),C.POINTER(D),C.c_uint,C.c_void_p,C.POINTER(D),C.POINTER(D)]
lib.reference_fit.restype=C.c_int

def run(expression, x, y):
    x=np.ascontiguousarray(x,dtype=np.float64);y=np.ascontiguousarray(y,dtype=np.float64)
    names=C.create_string_buffer(72);p=(D*6)();metrics=(D*4)()
    status=lib.reference_fit(expression.encode(),x.ctypes.data_as(C.POINTER(D)),y.ctypes.data_as(C.POINTER(D)),len(x),names,p,metrics)
    result={names.raw[i*12:(i+1)*12].split(b'\0')[0].decode():p[i] for i in range(6) if names.raw[i*12]}
    return status,result,list(metrics)

rng=np.random.default_rng(2026)
checks=[]
for degree in range(1,5):
    for noisy in (False,True):
        x=np.linspace(-2,3,17);coef=np.arange(degree+1,dtype=float)*.4+1
        y=np.vander(x,degree+1)@coef+(rng.normal(0,.1,len(x)) if noisy else 0)
        letters='abcdh'[:degree+1]
        expression='y_1~'+'+'.join(f'{c}*x_1^{degree-i}' for i,c in enumerate(letters))
        status,p,metrics=run(expression,x,y);reference=np.linalg.lstsq(np.vander(x,degree+1),y,rcond=None)[0]
        assert status==0,(expression,status,p)
        np.testing.assert_allclose([p[c] for c in letters],reference,rtol=2e-6,atol=2e-6)
        assert metrics[3]==1,('linear fast path',metrics)
        checks.append({'case':f'polynomial degree {degree}, noisy={noisy}','sse':metrics[0],'iterations':metrics[3]})
models=[
 ('y_1~a*exp(b*x_1)',lambda p,x:p[0]*np.exp(p[1]*x),[2,.35]),
 ('y_1~a*b^(x_1)+c',lambda p,x:p[0]*p[1]**x+p[2],[1.7,1.3,.4]),
 ('y_1~a*x_1^b',lambda p,x:p[0]*x**p[1],[2.4,1.5]),
 ('y_1~a*ln(x_1)+b',lambda p,x:p[0]*np.log(x)+p[1],[2.3,.5]),
 ('y_1~a/x_1+b',lambda p,x:p[0]/x+p[1],[3.5,1.2]),
 ('y_1~a*sqrt(x_1)+b',lambda p,x:p[0]*np.sqrt(x)+p[1],[1.8,.6]),
 ('y_1~a*sin(b*x_1+c)+d',lambda p,x:p[0]*np.sin(p[1]*x+p[2])+p[3],[2,1.1,.3,.5]),
]
for expr,fn,true in models:
    x=np.linspace(.4,4,24);y=fn(np.array(true),x)+rng.normal(0,.0001,len(x))
    status,p,metrics=run(expr,x,y)
    assert status==0,(expr,status,p)
    estimate=np.array([p[c] for c in 'abcd'[:len(true)]])
    reference=least_squares(lambda z:fn(z,x)-y,np.array(true),xtol=1e-12,ftol=1e-12,gtol=1e-12)
    np.testing.assert_allclose(fn(estimate,x),fn(reference.x,x),rtol=1e-4,atol=1e-4)
    checks.append({'case':expr,'sse':metrics[0],'iterations':metrics[3]})
# These must report errors, not plausible-looking invented coefficients.
for expr,x,y,expected in [('y_1~a*x_1+b',[1,1,1],[2,3,4],3),('y_1~a*x_1+b',[1],[2],2),('y_1~a*ln(x_1)+b',[-1,0,1],[1,2,3],4)]:
    status,_,_=run(expr,x,y);assert status==expected,(expr,status,expected)
    checks.append({'case':expr,'expected_error':status})
print(f'reference: {len(checks)} NumPy/SciPy model and error cases passed')
(BUILD/'reference-results.json').write_text(json.dumps(checks,indent=2)+'\n')
