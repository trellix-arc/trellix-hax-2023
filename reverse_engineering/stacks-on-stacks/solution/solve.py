from z3 import *

# instantiate solver
s = Solver()

# flag length
FLAG_LEN = 33

# create arguments
args = [BitVec(f'args[{i}]', 32) for i in range(0,FLAG_LEN)]

# **********************************************************

# add constraints for ascii
for i in range(0, FLAG_LEN):
    s.add(args[i] < 127)
    s.add(args[i] >= 32)

# G1 0, 1, 2, 3
s.add(args[0] == 0x41)
s.add(args[1] == 0x52)
s.add(args[2] == 0x43)
s.add(args[3] == 0x7b)

#G2 4, 5, 6, 7
s.add(args[4] == ord('B'))
s.add(args[5] == ord('0'))
s.add(args[6] == ord('0'))
s.add(args[7] == ord('m'))

#G3 8, 9, 10, 11
s.add(args[8] == ord('_'))
s.add(args[9] == ord('t'))
s.add(args[10] == ord('h'))
s.add(args[11] == ord('3'))

#G4 12, 13, 14, 15
s.add(args[12] == ord('r'))
s.add(args[13] == ord('e'))
s.add(args[14] == ord('_'))
s.add(args[15] == ord('g'))

#G5 16, 17, 18, 19
s.add(args[19] == 95)
s.add(args[16] ^ args[19] == 48)
s.add(args[17] ^ args[19] == 108)
s.add(args[18] ^ args[19] == 106)

#G6 20, 21, 22, 23 
s.add(args[23] == 95)
s.add(args[20] ^ args[23] == 104)
s.add(args[21] ^ args[23] == 55)
s.add(args[22] ^ args[23] == 108)

#G7 24, 25, 26, 27
s.add(args[24] & args[25] == 36)
s.add(args[24] & args[26] == 100)
s.add(args[25] & args[26] == 38)
s.add(args[25] & args[27] == 52)
s.add(args[24] + args[25] + args[26] + args[27] == 317)
s.add(args[24] + args[25] + args[27] == 207)
s.add(args[24] + args[25] + args[26] == 265)
s.add(args[27] + args[25] + args[26] == 217)

#G8 28, 29, 30, 31
s.add(args[28] ^ args[29] == 92)
s.add(args[28] ^ args[30] == 25)
s.add(args[28] ^ args[31] == 94)
s.add(args[29] ^ args[30] == 69)
s.add(args[29] ^ args[31] == 2)
s.add(args[30] ^ args[31] == 71)
s.add(args[28] ^ args[29] ^ args[30] ^ args[31] == 27)
s.add(args[28] ^ args[29] ^ args[31] == 111)


#G9 32
s.add(args[-1] == 0x7d)

# **********************************************************

# check our contraints
s.check()

# get the model
m = s.model()

# print flag
flag = ''
for i in range(0, FLAG_LEN):
    flag += chr(int(str(m[args[i]])))
print(flag)