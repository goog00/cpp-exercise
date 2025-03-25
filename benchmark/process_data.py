import xml.etree.ElementTree as ET
import json
import pandas as pd

# 处理 Google Test 的 XML 输出
def parse_gtest_xml(xml_file):
    tree = ET.parse(xml_file)
    root = tree.getroot()
    data = []
    for testcase in root.findall(".//testcase"):
        name = testcase.get("name")
        time = float(testcase.get("time")) * 1000  # 转换为毫秒
        status = "Passed" if testcase.find("failure") is None else "Failed"
        data.append({"Test Name": name, "Time (ms)": time, "Status": status})
    return pd.DataFrame(data)

# 处理 Google Benchmark 的 JSON 输出
def parse_benchmark_json(json_file):
    with open(json_file, 'r') as f:
        data = json.load(f)
    benchmarks = data["benchmarks"]
    result = []
    for bm in benchmarks:
        name = bm["name"]
        time = bm["real_time"]  # 时间单位由 JSON 指定，这里假设是 ns
        iterations = bm["iterations"]
        result.append({"Benchmark Name": name, "Time (ns)": time, "Iterations": iterations})
    return pd.DataFrame(result)

# 执行解析并保存为 CSV
gtest_df = parse_gtest_xml("test_results.xml")
benchmark_df = parse_benchmark_json("benchmark_results.json")

gtest_df.to_csv("gtest_processed.csv", index=False)
benchmark_df.to_csv("benchmark_processed.csv", index=False)

print("Google Test Data:\n", gtest_df)
print("\nGoogle Benchmark Data:\n", benchmark_df)