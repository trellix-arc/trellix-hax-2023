import struct

with open("chal_clear", "rb") as f:
    data = f.read()

print(len(data))

marker_start = b"\x50\x68\xAD\xDE\x00\x00\x58\x58"
marker_end = b"\x50\x68\x37\x13\x37\x13\x58\x58"

pos_start = 0

def patch(data, pos, patch_bytes):
    return data[:pos] + patch_bytes + data[pos+len(patch_bytes):]



while True:
    pos_start = data.find(marker_start, pos_start)
    if pos_start == -1:
        break
    pos_start += len(marker_start)

    pos_end = data.find(marker_end, pos_start)
    if pos_end == -1:
        raise "error missing end"


    print("function found")
    print("Start: {:x} end: {:x}".format(pos_start, pos_end))
    i = 0;
    while True:
        pos = pos_start + i
        val = i%0x100
        #val = 0x27
        data = patch(data, pos, struct.pack("B", data[pos] ^ val))
        i += 1
        if pos_start + i == pos_end:
            break

with open("chal", "wb") as f:
    f.write(data)

print(len(data))
        
