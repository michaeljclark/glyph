#!/bin/sh

for test in $(ls tests/[0-9][0-9]_*.py | sort | sed 's#tests/\(.*\)\.py#\1#'); do
	echo ${test}
	A=$(mktemp test_${test}_XXXXXX)
	B=$(mktemp test_${test}_XXXXXX)
	./build/test_${test} > ${A}
	./tests/${test}.py > ${B}
	diff -u ${A} ${B}
	rm -f ${A} ${B}
done
