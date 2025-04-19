import pandas as pd
import matplotlib.pyplot as plt
import os

csv_path = "performance_data.csv"
df = pd.read_csv(csv_path)

output_dir = "plots"
os.makedirs(output_dir, exist_ok=True)

serial_df = df[df['Method'] == 'Serial']

plt.figure(figsize=(8, 6))
plt.plot(serial_df['Input Size'], serial_df['Time (s)'], marker='o', linestyle='None', color='teal')
plt.title("Serial Method Performance")
plt.xlabel("Input Size")
plt.ylabel("Time (s)")
plt.grid(True)
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "Serial_Method_Performance.png"))
plt.close()

other_methods = df[df['Method'] != 'Serial']['Method'].unique()

for method in other_methods:
    method_df = df[df['Method'] == method]
    
    plt.figure(figsize=(8, 6))
    
    for input_size in method_df['Input Size'].unique():
        input_df = method_df[method_df['Input Size'] == input_size]
        x = input_df['Threads/Processes/Block Size']
        y = input_df['Time (s)']
        plt.plot(x, y, marker='o', linestyle='None', label=f"Input: {input_size}")
    
    plt.title(f"{method} Performance")
    plt.xlabel("Threads / Processes / Block Size")
    plt.ylabel("Time (s)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    
    filename = f"{method.replace(' ', '_')}_Performance.png"
    plt.savefig(os.path.join(output_dir, filename))
    plt.close()
