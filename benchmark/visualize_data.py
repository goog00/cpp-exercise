import pandas as pd
import plotly.express as px

# 读取处理后的 CSV 文件
gtest_df = pd.read_csv("gtest_processed.csv")
benchmark_df = pd.read_csv("benchmark_processed.csv")

# 可视化 Google Test 结果
fig_gtest = px.bar(
    gtest_df,
    x="Test Name",
    y="Time (ms)",
    color="Status",
    title="Google Test Execution Times",
    labels={"Time (ms)": "Execution Time (milliseconds)", "Test Name": "Test Case"},
    text=gtest_df["Time (ms)"].round(2),  # 在柱子上显示时间值
)
fig_gtest.update_traces(textposition="auto")  # 调整文字位置
fig_gtest.update_layout(
    xaxis_title="Test Case",
    yaxis_title="Execution Time (ms)",
    bargap=0.2,  # 柱子间距
    showlegend=True
)
fig_gtest.write_html("gtest_visualization.html")
print("Google Test visualization saved as 'gtest_visualization.html'")

# 可视化 Google Benchmark 结果
fig_benchmark = px.line(
    benchmark_df,
    x="Benchmark Name",
    y="Time (ns)",
    title="Google Benchmark Performance",
    labels={"Time (ns)": "Time per Iteration (nanoseconds)", "Benchmark Name": "Benchmark"},
    markers=True  # 显示数据点
)
fig_benchmark.update_layout(
    xaxis_title="Benchmark",
    yaxis_title="Time per Iteration (ns)",
    showlegend=False
)
fig_benchmark.write_html("benchmark_visualization.html")
print("Google Benchmark visualization saved as 'benchmark_visualization.html'")