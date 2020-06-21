#!/bin/bash

# Script for internal debugging/startup

CONF=release

cd ${APROJECTS}/build-littlefgconnect-${CONF}

export LD_LIBRARY_PATH=~/Qt/5.12.5/gcc_64/lib:${APROJECTS}/build-littlefgconnect-${CONF}

${APROJECTS}/build-littlefgconnect-${CONF}/littlenfgconnect $@
