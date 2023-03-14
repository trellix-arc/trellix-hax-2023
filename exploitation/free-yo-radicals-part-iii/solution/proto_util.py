import time
from scapy.all import *

def str2seperatedstr(s, delim):
    s=s.ljust(16,"\x00")
    #every 4 chars insert the delimter into the str.
    return delim.join(s[i:i+4] for i in range(0, len(s), 4))

def byte2seperatedstr(b, delim):
    b=b.ljust(16,b"\x00")
    #every 4 chars insert the delimter into the str.
    return delim.join(b[i:i+4] for i in range(0, len(b), 4))

class delimField(StrFixedLenField):
    __slots__ = ["delim"]

    def __init__(self,name,default,delim):
        self.delim =delim

        if default == None:
            StrFixedLenField.__init__(self,name,str2seperatedstr("",delim),20)

        else:
            x = str2seperatedstr(default,delim)
            StrFixedLenField.__init__(self,name,x,len(x))

    #internal means BYTE ARRAY
    def i2h(self,pkt,val):
        # val is a byte array
        val = val.replace(b'\x00',b'')
        return val.replace(bytes(self.delim,"utf-8"),b"")

    def addfield(self,pkt,s,val):
        if val == None:
            return StrFixedLenField.addfield(self,pkt,s,val)
        return StrFixedLenField.addfield(self,pkt,s,byte2seperatedstr(val,bytes(self.delim,"utf-8")))

class Transaction(Packet):
    name = "Transaction packet"
    delim = 0x0E
    term = 0x0C
    fields_desc = [ IPField("ipaddress", "127.0.0.1"), #consider using SourceIPField type?
                    XByteField("seperator",delim ),
                    ShortField("port",9999),
                    XByteField("seperator",delim ),
                    UTCTimeField("time", int(time.time())),
                    XByteField("seperator",delim ),
                   ShortEnumField("action",1,{1:"create",2:"delete",3:"print", 5:"bye"}),
                    XByteField("seperator",delim ),
                    delimField("data","aaaabbbbccccdddd",chr(delim)), #BUG IN DEFAULT CASE, NEED TO ADDRESS
                    XByteField("seperator",delim ),
                    LongField("value",8),
                    XByteField("seperator",delim),
                    XByteField("terminator",term)
    ]

def proto_packet(action='create', data='', value=0):
    p=Transaction()
    p.action = action
    p.data = data
    p.value = value
    return bytes(p)

if __name__ == '__main__':
    print(proto_packet())
