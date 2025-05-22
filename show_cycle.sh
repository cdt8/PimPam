#!/bin/bash

# 设置默认值（可选）
GRAPH="${GRAPH:-PA}"          # 如果未设置 GRAPH，则默认使用 "TH"
PATTERN="${PATTERN:-CLIQUE3}" # 如果未设置 PATTERN，则默认使用 "CLIQUE3"
LOG_FILE1="./result/clique3_roadNet-PA_adj.txt"
LOG_FILE2="./result/clique3_roadNet-PA_adj_NO_RUN.txt"

# 执行 make test 并传入环境变量
echo "Running: GRAPH=$GRAPH PATTERN=$PATTERN make test"
GRAPH="$GRAPH" PATTERN="$PATTERN"  make test 

# 检查 make 是否成功
if [ $? -ne 0 ]; then
    echo "Error: 'make test' failed!"
    exit 1
fi

# 检查日志文件是否存在
if [ ! -f "$LOG_FILE1" ]; then
    echo "Error: Expected log file not found: $LOG_FILE1"
    exit 1
fi


# 检查 Python 脚本是否成功
if [ $? -ne 0 ]; then
    echo "Error: Python script failed!"
    exit 1
fi


# 执行 make test 并传入环境变量
echo "Running: GRAPH=$GRAPH PATTERN=$PATTERN make test"
GRAPH="$GRAPH" PATTERN="$PATTERN"  make test EXTRA_FLAGS="-DNO_RUN"

# 检查 make 是否成功
if [ $? -ne 0 ]; then
    echo "Error: 'make test' failed!"
    exit 1
fi

# 检查日志文件是否存在
if [ ! -f "$LOG_FILE2" ]; then
    echo "Error: Expected log file not found: $LOG_FILE2"
    exit 1
fi

# 用 Python 脚本解析日志
echo "Running: python3 ./python_tool/show_run_cycle.py  $LOG_FILE1 $LOG_FILE2"
python3 ./python_tool/show_run_cycle.py "$LOG_FILE1" "$LOG_FILE2"

# 检查 Python 脚本是否成功
if [ $? -ne 0 ]; then
    echo "Error: Python script failed!"
    exit 1
fi

echo "Done!"