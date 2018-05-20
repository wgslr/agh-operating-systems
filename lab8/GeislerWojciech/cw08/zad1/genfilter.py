#!/usr/bin/env python3

import sys
import random

dim = int(sys.argv[1])
numbers = [random.uniform(0.0, 1.0) for _ in range(dim**2)]
summed = sum(numbers)
numbers2 = [x/summed for x in numbers]

print(dim)
print(" ".join(map(str, numbers2)))


