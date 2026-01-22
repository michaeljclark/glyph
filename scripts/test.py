#!/usr/bin/env python3

import os
import pathlib
import hashlib
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
        python_txt =  exec_test(temp, '%s_%s' % (name, 'script'), [script])
        golang_txt =  exec_test(temp, '%s_%s' % (name, 'golang'), ['go','run',golang])
        diff_result = subprocess.run(['diff3',
            native_txt, python_txt, golang_txt], capture_output=True, text=True)
        native_hash = hashlib.sha256(open(native_txt, "rb").read()).hexdigest()
        python_hash = hashlib.sha256(open(python_txt, "rb").read()).hexdigest()
        golang_hash = hashlib.sha256(open(golang_txt, "rb").read()).hexdigest()
        os.unlink(native_txt)
        os.unlink(python_txt)
        os.unlink(golang_txt)
        print('%-32s %s %s %s' % (name, native_hash[0:15], python_hash[0:15], golang_hash[0:15]))
        if diff_result.stdout:
            print(diff_result.stdout, end='')

os.rmdir(temp)
