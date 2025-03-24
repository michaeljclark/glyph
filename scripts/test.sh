#!/bin/sh

tmp="glyph-temp"
test -d ${tmp} || mkdir ${tmp}
for test in $(ls tests/[0-9][0-9]_*.py | sort | sed 's#tests/\(.*\)\.py#\1#'); do
	echo ${test}
	./build/test_${test} > ${tmp}/${test}_native
	./tests/${test}.py > ${tmp}/${test}_python
	diff -u ${tmp}/${test}_native ${tmp}/${test}_python
done
test -d ${tmp} && rm -fr ${tmp}
