#!/bin/bash

WEB_HASH=`./apps/webget tomahawk.postech.ac.kr /hasher/xyzzy | tee /dev/stderr | tail -n 1`
CORRECT_HASH="184858a00fd7971f810848266ebcecee5e8b69972c5ffaed622f5ee078671aed"

if [ "${WEB_HASH}" != "${CORRECT_HASH}" ]; then
    echo ERROR: webget returned output that did not match the test\'s expectations
    exit 1
fi
exit 0
