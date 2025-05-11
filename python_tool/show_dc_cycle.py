import re
import matplotlib
matplotlib.use('Agg')  # 无图形界面兼容
import matplotlib.pyplot as plt
import os
import sys

def parse_dpu_cycles(file_path):
    # 模式1：tasklet 级别的时间信息
    pattern1 = re.compile(
        r'DPU:\s*(\d+),\s*tasklet:\s*(\d+),\s*I_cycle:\s*(\d+),\s*total_cycle:\s*(\d+),\s*root_num:\s*(\d+)')
    # 模式2：DPU 汇总信息（含比率）
    pattern2 = re.compile(
        r'DPU:\s*(\d+),\s*large_degree_num:\s*(\d+),\s*root_num:\s*(\d+),\s*ratio:\s*([\d.]+)')

    dpu_data = {}

    with open(file_path, 'r') as f:
        for line in f:
            m1 = pattern1.search(line)
            m2 = pattern2.search(line)

            if m1:
                dpu, tasklet, i_cycle, total_cycle, root_num = map(int, m1.groups())
                if dpu not in dpu_data or total_cycle > dpu_data[dpu].get('total_cycle', 0):
                    dpu_data.setdefault(dpu, {})
                    dpu_data[dpu]['I_cycle'] = i_cycle
                    dpu_data[dpu]['Other_cycle'] = total_cycle - i_cycle
                    dpu_data[dpu]['total_cycle'] = total_cycle
                    dpu_data[dpu]['root_num'] = root_num
            elif m2:
                dpu, large_degree_num, root_num, ratio = m2.groups()
                dpu = int(dpu)
                dpu_data.setdefault(dpu, {})
                dpu_data[dpu]['large_degree_num'] = int(large_degree_num)
                dpu_data[dpu]['root_num'] = int(root_num)
                dpu_data[dpu]['ratio'] = float(ratio)

    return dpu_data

def plot_dpu_metrics_bar(dpu_data, output_path):
    if not dpu_data:
        print("警告：未提取到任何 DPU 数据，图像未生成。")
        return

    dpus = sorted(dpu_data.keys())
    i_cycles = [dpu_data[dpu].get('I_cycle', 0) for dpu in dpus]
    other_cycles = [dpu_data[dpu].get('Other_cycle', 0) for dpu in dpus]
    root_nums = [dpu_data[dpu].get('root_num', 0) for dpu in dpus]
    ratios = [dpu_data[dpu].get('ratio', 0.0) for dpu in dpus]
    total_cycles = [dpu_data[dpu].get('total_cycle', 0) for dpu in dpus]

    max_total = max(total_cycles)
    max_dpu = dpus[total_cycles.index(max_total)]
    runtime_ms = max_total * 2.85e-6

    plt.figure(figsize=(14, 6))

    # 子图1：堆叠柱状图
    plt.subplot(1, 2, 1)
    plt.bar(dpus, i_cycles, label='I_cycle', color='skyblue')
    plt.bar(dpus, other_cycles, bottom=i_cycles, label='Other', color='salmon')
    plt.title("Max Cycle per DPU (Stacked)", fontsize=14)
    plt.xlabel("DPU ID", fontsize=12)
    plt.ylabel("Cycle Count", fontsize=12)
    plt.xticks(rotation=45, fontsize=8)
    plt.legend(fontsize=10)
    label_text = f"Max Total Cycle = {max_total} (DPU {max_dpu})\nEst. Runtime ≈ {runtime_ms:.2f} ms"
    plt.text(0.05, 0.95, label_text, transform=plt.gca().transAxes,
             fontsize=10, verticalalignment='top', bbox=dict(facecolor='white', alpha=0.8))

    # 子图2：Root Num 柱状图 + 比率折线图
    plt.subplot(1, 2, 2)
    plt.bar(dpus, root_nums, color='orange', label='root_num')
    plt.plot(dpus, ratios, color='blue', marker='o', label='ratio')
    plt.title("Root Num & Ratio per DPU", fontsize=14)
    plt.xlabel("DPU ID", fontsize=12)
    plt.ylabel("Root Num", fontsize=12)
    plt.xticks(rotation=45, fontsize=8)
    plt.legend(fontsize=10)

    plt.tight_layout()
    plt.savefig(output_path, dpi=300)
    plt.close()

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("用法: python show_dc_cycle.py input_file.txt")
        sys.exit(1)

    input_file = sys.argv[1]
    if not os.path.isfile(input_file):
        print(f"错误：文件 '{input_file}' 不存在。")
        sys.exit(1)

    output_file = os.path.splitext(input_file)[0] + "_detailed.png"
    dpu_data = parse_dpu_cycles(input_file)
    plot_dpu_metrics_bar(dpu_data, output_file)
    print(f"图像已保存至: {output_file}")
