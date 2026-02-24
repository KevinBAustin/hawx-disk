#!/bin/bash
expected=6

.test/run-hawx > .test/hawx.out
passed=$(grep PASSED .test/hawx.out | wc -l)

cat .test/hawx.out

echo
echo
echo "Passed $passed out of $expected tests."

killall qemu-system-riscv64 2>&1 > /dev/null

if [ $passed -ne $expected ]; then exit 1; fi
