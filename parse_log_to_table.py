import re
import argparse

def parse_log(log_file, output_file):
    pattern_data = re.compile(r'DATA-NAME:\s*(\S+)')
    pattern_time = re.compile(r'DPU Time \(ms\):\s*([\d.]+)')
    pattern_ans  = re.compile(r'DPU ans:\s*(\d+)')

    records = []
    name, time, ans = None, None, None

    with open(log_file, 'r') as f:
        for line in f:
            if match := pattern_data.search(line):
                name = match.group(1)
            elif match := pattern_time.search(line):
                time = match.group(1)
            elif match := pattern_ans.search(line):
                ans = match.group(1)

            if name and time and ans:
                records.append((name, time, ans))
                name, time, ans = None, None, None

    with open(output_file, 'w') as out:
        out.write(f"{'Dataset':<20} {'Time(ms)':<15} {'Answer':<10}\n")
        for name, time, ans in records:
            out.write(f"{name:<20} {time:<15} {ans:<10}\n")

    print(f"[INFO] 提取完成，共 {len(records)} 条记录，输出文件: {output_file}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="分析 DPU 日志并生成表格")
    parser.add_argument("--input", "-i", default="dataset_run.log", help="输入日志文件名（默认: dataset_run.log）")
    parser.add_argument("--output", "-o", default="summary_table.txt", help="输出表格文件名（默认: summary_table.txt）")
    args = parser.parse_args()

    parse_log(args.input, args.output)
