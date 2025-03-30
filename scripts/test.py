#!/usr/bin/env python3

import os
import pathlib
import subprocess

def exec_test(temp, name, prog):
    path = os.path.join(temp, name)
    with open(path, 'w') as out:
        subprocess.run(prog, stdout=out)
        return path

tests = pathlib.Path('tests')
build = pathlib.Path('build')

temp = build / 'temp'
if not os.path.isdir(temp):
    os.mkdir(temp)

for script in sorted(tests.glob('*.py')):
    name = os.path.splitext(os.path.basename(script))[0]
    native = os.path.join(build, 'test_%s' % name)
    golang = os.path.join(tests, '%s.go' % name)
    if os.access(script, os.X_OK) and  os.access(native, os.X_OK) and os.access(golang, os.F_OK):
        native_txt = exec_test(temp, '%s_%s' % (name, 'native'), [native])
        script_txt =  exec_test(temp, '%s_%s' % (name, 'script'), [script])
        golang_txt =  exec_test(temp, '%s_%s' % (name, 'golang'), ['go','run',golang])
        diff_result = subprocess.run(['diff3',
            native_txt, script_txt, golang_txt], capture_output=True, text=True)
        os.unlink(native_txt)
        os.unlink(script_txt)
        os.unlink(golang_txt)
        print('%s\n%s' % (name, diff_result.stdout), end='')

os.rmdir(temp)
