import random

num_accesses = 100
max_page_num = 99

trace = [str(random.randint(0, max_page_num)) for _ in range(num_accesses)]
# Only print the trace, without any additional formatting or comments
print(",".join(trace))
