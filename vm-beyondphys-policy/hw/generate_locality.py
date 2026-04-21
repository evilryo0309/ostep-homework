import random

num_accesses = 200
trace = []

for _ in range(num_accesses):
    # 產生 0.0 到 1.0 之間的亂數
    if random.random() < 0.8:
        # 80% 的機率，存取 Hot Pages (假設編號 0 ~ 19)
        trace.append(str(random.randint(0, 19)))
    else:
        # 20% 的機率，存取 Cold Pages (假設編號 20 ~ 99)
        trace.append(str(random.randint(20, 99)))

print(",".join(trace))
