import sys
import subprocess
import matplotlib.pyplot as plt

# --- Parameter Configuration ---
MAX_LIMIT = 1024
TEST_AS_SIZE = "1k"
STEP = 32
NUM_ADDR = 1000
SEEDS = [1, 2, 3]

plt.figure(figsize=(10, 6))
print(f"Using environment lock: {sys.executable}")
print("Starting simulation. Errors will be displayed immediately...\n")

for seed in SEEDS:
    x_limits = []
    y_valid_fractions = []
    
    for limit in range(0, MAX_LIMIT + 1, STEP):
        # Key fix 1: Use sys.executable to lock the current virtual environment Python
        cmd = [sys.executable, "../relocation.py", "-s", str(seed), "-n", str(NUM_ADDR), "-a", TEST_AS_SIZE, "-l", str(limit), "-c"]
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode != 0:
            print(f"[Crash Warning] Simulator error when Limit={limit}, Seed={seed}!")
            print(f"Error code: {result.returncode}")
            print(f"Error message:\n{result.stderr}")
            sys.exit(1) # Exit immediately for debugging
            
        valid_count = result.stdout.count("VALID")
        valid_fraction = valid_count / NUM_ADDR
        
        x_limits.append(limit)
        y_valid_fractions.append(valid_fraction)
        
    plt.plot(x_limits, y_valid_fractions, marker='.', linestyle='--', label=f'Seed {seed}')

# --- Configure chart appearance ---
plt.title(f'Fraction of Valid VAs vs. Bounds (with AS={TEST_AS_SIZE})', fontsize=14)
plt.xlabel('Bounds Register (Limit Value)', fontsize=12)
plt.ylabel('Fraction of Valid Addresses (0.0 to 1.0)', fontsize=12)
plt.xlim(0, MAX_LIMIT)
plt.ylim(0, 1.05)
plt.grid(True, linestyle='--', alpha=0.7)
plt.legend()

output_filename = 'q5.png'
plt.savefig(output_filename, dpi=300, bbox_inches='tight')
print(f"Done! Chart saved as: {output_filename}")