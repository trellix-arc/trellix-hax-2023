# Solution Write Up for Classic Game Theory
The challenge was written in Turbo Pascal, for MS-DOS 16-bit.
It basically takes each 2-bytes from input, calculate its modified-MD5 hash and compare with a hash table. The md5 hash is with a minor modification (couple of bytes in initialized value).

1. GAME.EXE can be executed under any DOS emulator. I use DOSBox.
2. Reverse engineer using IDA, spot the algorithm how it calculate with md5 and xor to decide to finish at 'good' or 'bad' branch.
3. The modified init hash value can be found at 0x011D, and the hash table at 0x01F6
4. Rewrite the binary and bruteforce by generating a rainbow table of 2 characters input.

See solution/solve.py.

# Flag
[//]: <> (Add the flag below)
**ARC{y0u_g0tt4_pl4y_the_l0ng_g4me}**
