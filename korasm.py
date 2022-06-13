#!/usr/bin/env python3
import sys

if len(sys.argv) != 2:
    print("usage: korasm file.kor")
    sys.exit(1)

with open(sys.argv[1]) as f:
    words = f.read().split()

here = 0
i = 0
opcodes = ["nop", "lit", "dup", "drop", "swap", "over", "rot", "nip",
           "call", "ccall", "ret", "cret", "jmp", "cjmp", "wtr", "rtw",
           "eq", "neq", "lt", "gt", "and", "or", "xor", "shift",
           "add", "sub", "mul", "divmod", "fetch", "store", "sext", "trap"]

refs = {}
labels = {}
image = bytearray(65536)
image_size = 0

def is_hex(s):
    try:
        v = int(s, 16)
        return True
    except ValueError:
        return False

def write_word(a, w):
    for i in range(4):
        image[a] = w & 0xff
        a += 1
        w >>= 8
    

while i < len(words):
    print(f"i={i} here={here}")
    if words[i] == 'org':
        here = int(words[i+1], 16)
        i+=2
    elif words[i] == '(':
        while words[i] != ')':
            i += 1
        i += 1
    elif words[i] == ':':
        labels[words[i+1]] = here
        i += 2
    elif words[i] == '@':
        image[here] = opcodes.index("lit")
        here += 1
        refs.setdefault(words[i+1], []).append(here)
        here += 4
        i += 2
    elif words[i] in opcodes:
        image[here] = opcodes.index(words[i])
        here += 1
        i+= 1
    elif words[i] == '\'':
        image[here-1] |= 1 << 5
        i += 1
    elif words[i] == '\'\'':
        image[here-1] |= 1 << 6
        i += 1
    elif words[i] == '"':
        for b in bytearray(words[i+1], 'ascii'):
            image[here] = b
            here += 1
        i += 2
    elif words[i] == ".x":
        image[here] = int(words[i+1], 16)
        here += 1
        i += 2
    elif words[i] == '#':
        v = int(words[i+1], 16)
        if v < 256:
            image[here] = opcodes.index("lit") | 1 << 5
            here += 1
            image[here] = v
            here += 1
        else:
            print("Unimplemented literals > 255")
        i += 2
    else:
        print(f"unknown word: {words[i]}")
        sys.exit(1)
    image_size = max(image_size, here)


print(f"labels: {labels}")
print(f"refs: {refs}")

for r in refs.keys():
    for a in refs[r]:
        write_word(a, labels[r])

with open('output.img', 'wb') as f:
    f.write(image[0:image_size])
