import re
import sys
import os

def parse_dpu_cycles(file_path):
    pattern = re.compile(r'DPU:\s*(\d+),\s*tasklet:\s*(\d+),\s*cycle:\s*(\d+),\s*root_num:\s*(\d+)')
    max_cycle = 0

    with open(file_path, 'r') as f:
        for line in f:
            match = pattern.search(line)
            if match:
                cycle = int(match.group(3))
                if cycle > max_cycle:
                    max_cycle = cycle
    return max_cycle

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("用法: python calc_runtime.py input_file.txt")
        sys.exit(1)

    input_file = sys.argv[1]
    if not os.path.isfile(input_file):
        print(f"错误：文件 '{input_file}' 不存在。")
        sys.exit(1)

    max_cycle = parse_dpu_cycles(input_file)
    runtime_ms = max_cycle * 2.85e-6  # 1 cycle = 2.85ns = 2.85e-6 ms
    print(f"最大cycle数: {max_cycle}")
    print(f"估算运行时间: {runtime_ms:.2f} ms")