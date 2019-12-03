# kv is a list of dicts with key reg & value = value of reg

for i in range(1, len(kv)):
    index = 0
    while index < 17:
            if kv[i][index] != kv[i-1][index]:
                    break
            index += 1
    print(f"Difference of line {i} from {i-1}: r{index} is different ({kv[i][index]}, Previously: {kv[i-1][index]})")