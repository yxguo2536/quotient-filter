import math
print("\n")

# generate data of Bloom Filter
def fpr(n, m):
    k = math.ceil(math.log(2) * (m/n))
    rate =   (1-math.exp(-k*n/m))**k
    return rate
n=1
for m in range(4,33):
    # Bits/element(m/n) | -log(false positive rate)
    print("%d,%.5f" % ( m/n , -math.log(fpr(n,m), 10 )))
print("\n")

# generate data of Quotient Filter with load factor 0.75
hash_length=64
alpha=0.75
for q in range(40, hash_length):
    r=hash_length-q
    fpr=1-math.exp(-alpha*(2**q)/(2**hash_length))
    bit_per_element=(r+3)/alpha
    # q | r | fpr | Bits/element | -log(false positive rate)
    print("%d,%d,%.8f,%.1f,%.5f" % (
        q, r, fpr, bit_per_element, -math.log(fpr, 10)
    ))
print("\n")

# generate data of Quotient Filter with load factor 0.9
hash_length=64
alpha=0.9
for q in range(40, hash_length):
    r=hash_length-q
    fpr=1-math.exp(-alpha*(2**q)/(2**hash_length))
    bit_per_element=(r+3)/alpha
    # q | r | fpr | Bits/element | -log(false positive rate)
    print("%d,%d,%.8f,%.1f,%.5f" % (
        q, r, fpr, bit_per_element, -math.log(fpr, 10)
    ))