import hashlib
import binascii 
import struct
import random 


chunk1 = b"ARC{"
chunk2 = b"Obfusc4ti0n_4_"
chunk3 = b"lyfe"
chunk4 = b"}"

flag = [chunk1, chunk2, chunk3, chunk4]


print("Flag: ")
print(b"".join(flag))

chunk2_start = len(chunk1) 
chunk3_start = len(chunk1) + len(chunk2)


def to_c_hex(data):
    val = b"-" + binascii.hexlify(data, b"-")
    return val.replace(b"-", b"\\x").decode("ascii")


def generate_chunk2_encrypted(data):
    res = b""
    for d in data:
        v = d
        while v == d:
            v = random.randint(1,255) # don't want v==d otherwise v-d == 0 and it would be null byte
        res += struct.pack("BB", (d-v) % 0x100, v)
    return res



print("chunk2 start offset: ", chunk2_start )
print("chunk2 md5: ", to_c_hex(generate_chunk2_encrypted(chunk2)))

print("chunk3 start offset: ", chunk3_start )
print("chunk3 md5: ", to_c_hex(hashlib.md5(chunk3).digest()))