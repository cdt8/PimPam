#!/bin/bash

# ================================
# Usage:
#   ./show_cycle.sh GRAPH PATTERN TAG
# Example:
#   ./show_cycle.sh FE CLIQUE3 clique3_flickrEdges_adj
# ================================

# 参数解析
GRAPH="${1:-PA}"             # 默认 GRAPH=PA
PATTERN="${2:-CLIQUE3}"      # 默认 PATTERN=CLIQUE3
TAG="${3:-clique3_roadNet-PA_adj}"  # 默认 TAG

# 构造日志文件路径
LOG_FILE1="./result/${TAG}.txt"
LOG_FILE2="./result/${TAG}_NO_RUN.txt"

# 输出信息
echo "Running: GRAPH=$GRAPH PATTERN=$PATTERN"
echo "Log File 1: $LOG_FILE1"
echo "Log File 2: $LOG_FILE2"


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