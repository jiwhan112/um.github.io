"""Build and test the portable C++20 examples; expected failures use -c (no link)."""
from pathlib import Path
import subprocess, json, re, platform, sys
root=Path(__file__).resolve().parent
logs=root/'logs';logs.mkdir(exist_ok=True)
def execute(args, path):
    result=subprocess.run(args, cwd=root, capture_output=True, text=True, timeout=120)
    path.write_text('$ '+ ' '.join(map(str,args))+'\n'+result.stdout+result.stderr,encoding='utf-8')
    if result.returncode: print(path.name, 'returned', result.returncode)
    return result
report={'system':platform.platform(),'learning_standard':'C++20','builds':{},'negative':{},'limitations':['Windows/MSVC not executed','No DirectX/GPU integration test','Representative tests, not complete correctness proof']}
for tag, compiler, sanitizer in [('gcc','g++',False),('clang','clang++',False),('clang-sanitized','clang++',True)]:
    build=root/f'build-{tag}'
    report['builds'][tag]={'compiler':subprocess.check_output([compiler,'--version'],text=True).splitlines()[0]}
    args=['cmake','-S',str(root),'-B',str(build),f'-DCMAKE_CXX_COMPILER={compiler}','-DCMAKE_BUILD_TYPE=Debug']
    if sanitizer: args+=['-DENABLE_SANITIZERS=ON']
    for step, cmd in [('configure',args),('build',['cmake','--build',str(build),'-j4']),('tests',['ctest','--test-dir',str(build),'-V'])]:
        result=execute(cmd,logs/f'{tag}-{step}.txt')
        report['builds'][tag][step]=result.returncode
        if result.returncode: break
        if step=='tests':
            entries=re.findall(r'PASS (\w+): (\d+) checks',result.stdout)
            report['builds'][tag]['checks']={n:int(c) for n,c in entries}
            report['builds'][tag]['total_checks']=sum(int(c) for n,c in entries)
for compiler,tag in [('g++','gcc'),('clang++','clang')]:
    report['negative'][tag]={}
    for file in sorted((root/'negative').glob('*.cpp')):
        result=execute([compiler,'-std=c++20','-Wall','-Wextra','-pedantic-errors','-c',str(file),'-o',str(logs/f'{tag}-{file.stem}.o')],logs/f'{tag}-negative-{file.stem}.txt')
        report['negative'][tag][file.stem]={'rejected': result.returncode!=0,'exit_code':result.returncode}
(logs/'report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(report,ensure_ascii=False,indent=2))
valid=all(v.get('tests')==0 for v in report['builds'].values()) and all(i['rejected'] for group in report['negative'].values() for i in group.values())
sys.exit(0 if valid else 1)
