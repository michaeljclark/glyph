#!/usr/bin/env python3

import os
import pathlib
import subprocess

def exec_test(name, temp, exec, suffix):
    path = os.path.join(temp, '%s_%s' % (name, suffix))
    with open(path, 'w') as out:
        subprocess.run([exec], stdout=out)
        return path

tests = pathlib.Path('tests')
build = pathlib.Path('build')

temp = build / 'temp'
if not os.path.isdir(temp):
    os.mkdir(temp)

for script in tests.glob('*.py'):
    name = os.path.splitext(os.path.basename(script))[0]
    native = os.path.join(build, 'test_%s' % name)
    if os.access(script, os.X_OK) and  os.access(native, os.X_OK):
        native_txt = exec_test(name, temp, native, 'native')
        script_txt =  exec_test(name, temp, script, 'script')
        diff_result = subprocess.run(['diff', '-u',
            native_txt, script_txt], capture_output=True, text=True)
        os.unlink(native_txt)
        os.unlink(script_txt)
        print('%s\n%s' % (name, diff_result.stdout), end='')

os.rmdir(temp)
