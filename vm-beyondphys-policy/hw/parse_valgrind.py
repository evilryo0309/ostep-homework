input_file = "trace.txt"
output_file = "vpn_trace.txt"

vpns = []
MAX_ACCESSES = 5000

with open(input_file, "r") as f:
    for line in f:
        line = line.strip()
        if line.startswith(("I", "L", "S", "M")):
            parts = line.split()
            if len(parts) >= 2:
                addr_str = parts[1].split(",")[0]
                try:
                    addr = int(addr_str, 16)
                    vpn = addr >> 12
                    vpns.append(str(vpn))
                except ValueError:
                    continue

        if len(vpns) >= MAX_ACCESSES:
            break

with open(output_file, "w") as out_f:
    out_f.write("\n".join(vpns))

print("successfully parsed VPNs and saved to", output_file)
