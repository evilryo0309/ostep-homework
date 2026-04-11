import matplotlib.pyplot as plt

# data from run.sh, where each line is "number_of_pages time_in_ns"
pages = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192]
time_ns = [
    1.16,
    1.30,
    1.09,
    1.25,
    2.37,
    4.10,
    4.71,
    4.72,
    4.62,
    5.70,
    8.13,
    8.81,
    9.40,
    11.30,
]

# Use Matplotlib to create a line plot of the data
plt.plot(pages, time_ns, marker="o", linestyle="-", color="#FF8C00")

# Key step: set X-axis to logarithmic scale with base 2, and only show the tested page numbers as ticks
plt.xscale("log", base=2)
plt.xticks(pages, pages)  # 讓 X 軸只顯示我們測試的分頁數標籤

# Set title and labels for axes
plt.title("TLB Size Measurement")
plt.xlabel("Number Of Pages")
plt.ylabel("Time Per Access (ns)")

# Show grid for better readability
plt.grid(True, which="both", ls="--", alpha=0.5)

plt.savefig("tlb_plot.png")
# plt.savefig('tlb_plot.pdf', bbox_inches='tight') # 儲存為 PDF，並移除多餘白邊

print("圖片已儲存為 tlb_plot.png！")
