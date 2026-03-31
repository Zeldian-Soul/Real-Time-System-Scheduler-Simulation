import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results.csv")

# ── Plot 1: Average Waiting Time vs Dispatcher Overhead ──────────────────────
fig, ax = plt.subplots(figsize=(8, 5))

ax.plot(df["disp"], df["avg_wt"],   "b-o", linewidth=2, markersize=7, label="Overall")
ax.plot(df["disp"], df["hard_wt"],  "r-s", linewidth=2, markersize=7, label="Hard RT")
ax.plot(df["disp"], df["soft_wt"],  "g-^", linewidth=2, markersize=7, label="Soft RT")

ax.set_xlabel("Dispatcher Overhead (ms)", fontsize=12)
ax.set_ylabel("Average Waiting Time (ms)", fontsize=12)
ax.set_title("Average Waiting Time vs Dispatcher Overhead", fontsize=13, fontweight="bold")
ax.legend(fontsize=11)
ax.grid(True, linestyle="--", alpha=0.5)
ax.set_xticks(df["disp"])

plt.tight_layout()
plt.savefig("plot_waiting_time.png", dpi=200)
print("Saved: plot_waiting_time.png")
plt.show()

# ── Plot 2: Average Turnaround Time vs Dispatcher Overhead ───────────────────
fig, ax = plt.subplots(figsize=(8, 5))

ax.plot(df["disp"], df["avg_tat"],   "b-o", linewidth=2, markersize=7, label="Overall")
ax.plot(df["disp"], df["hard_tat"],  "r-s", linewidth=2, markersize=7, label="Hard RT")
ax.plot(df["disp"], df["soft_tat"],  "g-^", linewidth=2, markersize=7, label="Soft RT")

ax.set_xlabel("Dispatcher Overhead (ms)", fontsize=12)
ax.set_ylabel("Average Turnaround Time (ms)", fontsize=12)
ax.set_title("Average Turnaround Time vs Dispatcher Overhead", fontsize=13, fontweight="bold")
ax.legend(fontsize=11)
ax.grid(True, linestyle="--", alpha=0.5)
ax.set_xticks(df["disp"])

plt.tight_layout()
plt.savefig("plot_turnaround_time.png", dpi=200)
print("Saved: plot_turnaround_time.png")
plt.show()

# ── Bonus: CPU Utilization vs Dispatcher Overhead ────────────────────────────
fig, ax = plt.subplots(figsize=(8, 5))

ax.plot(df["disp"], df["cpu_util"], "m-D", linewidth=2, markersize=7)

ax.set_xlabel("Dispatcher Overhead (ms)", fontsize=12)
ax.set_ylabel("CPU Utilization (%)", fontsize=12)
ax.set_title("CPU Utilization vs Dispatcher Overhead", fontsize=13, fontweight="bold")
ax.set_ylim(0, 110)
ax.grid(True, linestyle="--", alpha=0.5)
ax.set_xticks(df["disp"])

plt.tight_layout()
plt.savefig("plot_cpu_util.png", dpi=200)
print("Saved: plot_cpu_util.png")
plt.show()
