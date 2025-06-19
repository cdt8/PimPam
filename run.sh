#!/bin/bash

GRAPH=${1:-AM0312}
PATTERN=${2:-CLIQUE3}

make clean
GRAPH=$GRAPH PATTERN=$PATTERN make test

# 从 common.h 里提取 DATA_NAME 和 PATTERN_NAME
DATA_NAME=$(grep -A1 "#elif defined($GRAPH)" include/common.h | grep DATA_NAME | awk '{print $3}' | tr -d '"')
PATTERN_NAME=$(grep -A1 "#elif defined($PATTERN)" include/common.h | grep PATTERN_NAME | awk '{print $3}' | tr -d '"')

# 兼容大小写
DATA_NAME_LOWER=$(echo $DATA_NAME | tr 'A-Z' 'a-z')
PATTERN_NAME_LOWER=$(echo $PATTERN_NAME | tr 'A-Z' 'a-z')

# LOG_FILE="./result/${PATTERN_NAME_LOWER}_${DATA_NAME_LOWER}.txt"
LOG_FILE="./result/$(echo $PATTERN | tr 'A-Z' 'a-z')_$(echo $DATA_NAME ).txt"
python3 ./python_tool/cycle2runtime.py "$LOG_FILE"