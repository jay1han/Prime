#!/usr/bin/python3

def read_int(infile):
    data = infile.read(4)
    if len(data) == 0:
        return None
    return data[0] + 256 * (data[1] + 256 * (data[2] + 256 * (data[3])))

def read_multi(infile, n):
    result = []
    for i in range(n):
        factor = read_int(infile)
        exponent = read_int(infile)
        result.append((factor, exponent))
    return result

with open("primes", "rb") as primes:
    n = 0
    while n is not None:
        n = read_int(primes)
        if n is not None:
            divisors = read_int(primes)
            if divisors > 0:
                factors = read_multi(primes, divisors)
                print(f"{n:8} =", end="")
                for divisor in factors:
                    print(f" {divisor[0]}^{divisor[1]}", end="")
                print()
            else:
                print(f"{n:8} is Prime")
        
