# Solution Write Up for 'Spying through the webdoor'
# Expected Behavior
```
rjohnson@shiv:/arc-httpd$ ./arc-httpd

▄▄▄▄▄▄▄▄       ▄▄▄▄▄▄▄▄       ▄▄▄▄▄▄▄▄▄▄▄▄
█       ▀▀▄    █       ▀▀▄    █          █
    ▄▄▄    ▀▄      ▄▄▄    ▀       █▀▀▀▀▀▀▀
▀   █  ▀▄   ▀  ▀   █  ▀▄   █  ▀   █
█ ░ █▄▄▄▀   █  █ ░ █▄▄▀   ▄▀  █ ░ █
█ ▒         █  █ ▒      ▄▄▀   █ ▒▄▀▄
█ ▓ █▀▀▀█   █  █ ▓ ▄   ▀▄▄    ▀█ ▓▓▄▀▀▄▄▄▄
█ ▀ █   █   █  █ ▀ █▀▄    █     ▀▄▄▀▀▀   █
▀▀▀▀▀   ▀▀▀▀▀  ▀▀▀▀▀  ▀▀▀▀▀        ▀▀▀▀▀▀▀
▄▄▄▄▄   ▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄       ▄▄▄▄▄▄▄▄
█   █   █   █  █           █  █           █  █       ▀▀▄    █       ▀▀▄
    █   █   █  ▀▀▀▀    █▀▀▀▀  ▀▀▀▀    █▀▀▀▀      ▄▄▄    ▀▄      ▄▄▄    ▀▄
▀   ▀▀▀▀▀   ▀      ▀   █          ▀   █      ▀   █  ▀▄   ▀  ▀   █  ▀▄   ▀
█ ░ ▄▄▄▄▄ ░ █      █ ░ █          █ ░ █      █ ░ ▀▀▀▀▀ ▄▀   █ ░ █   █   █
█ ▒ █   █ ▒ █      █ ▒ █          █ ▒ █      █ ▒ █▀▀▀▀▀     █ ▒ █   █   █
█ ▓ █   █ ▓ █      █ █ █          █ █ █      █ ▓ █          █ ▓ ▀▀▀▀    █
█ ▀ █   █ ▀ █      █   █          █   █      █ ▀ █          █ ▀▀▀ ▀    ▄▀
▀▀▀▀▀   ▀▀▀▀▀      ▀▀▀▀▀          ▀▀▀▀▀      ▀▀▀▀▀          ▀▀▀▀▀▀▀▀▀▀▀

This server has a backdoor in it!
Find the special request that lets you execute commands!
The key for this challenge is part of the backdoor and
will be printed on success

error: Please select a port for the web server

usage: ./arc-httpd <port>
```
# Solution

1. The description gives a clue that this is a backdoor in the request processing of this HTTP server. The player is expected to either reverse engineer the recv() call path or do a bottom-up analysis starting from the call to system(). In either case they will find the backdoor is in a function that handles cgi-bin related requests. The function checks for an unknown substring that is encoded with a hash which must be reversed or "cracked" to reach the call to system().

2. The hash function is a simple multibyte xor and add and the key is bytes from within the binary. This is meant to be an easy 101 exercise.

The encode/decode functions are included here
```
// caller must free
unsigned char *xor_multi_dec(unsigned char *buf, int buf_len, unsigned char *key, int key_len)
{
        int i;
        unsigned char *hash = (unsigned char *)malloc(buf_len);
        if(!hash)
                perror("malloc");

        for (i = 0; i < buf_len; i++)
    	{
                hash[i] = ((buf[i] - 0x42) ^ key[i % key_len]);
    	}
        return hash;
}

// caller must free
unsigned char *xor_multi_enc(unsigned char *buf, int buf_len, unsigned char *key, int key_len)
{
        int i;
        unsigned char *hash = (unsigned char *)malloc(buf_len);
        if(!hash)
                perror("malloc");

        for (i = 0; i < buf_len; i++)
                hash[i] = (buf[i] ^ key[i % key_len]) + 0x42;

        return hash;
}
```
3. The player eventually recovers the key for the level through reversing the hash and sends the winning request. 
```
$ nc -vv localhost 8080
Connection to localhost 8080 port [tcp/http-alt] succeeded!
GET /cgi-bin/{XOR_XOR_XOR_YOUR_BOAT} HTTP/1.0

HTTP/1.1 200 OK
Server: arc-httpd
Connection: close

Congrats! The key is: ARC{XOR_XOR_XOR_YOUR_BOAT}

executing command: /usr/bin/whoami
root
```
While there is technically a code path that would really call system(), any path with the key included will pass the substring to one of two paths .. the actual system() call is intentionally guarded by an unsolvable path for safety reasons, other paths will just print the command.
```
$ nc -vv localhost 8080
Connection to localhost 8080 port [tcp/http-alt] succeeded!
GET /cgi-bin/{XOR_XOR_XOR_YOUR_BOAT}//this/path/is/fake HTTP/1.0

HTTP/1.1 200 OK
Server: arc-httpd
Connection: close

Congrats! The key is: ARC{XOR_XOR_XOR_YOUR_BOAT}

executing command: /this/path/is/fake
system("/this/path/is/fake") :)
```

4. Last step Hack the planet!


# Flag
[//]: <> (Add the flag below)
**ARC{XOR_XOR_XOR_YOUR_BOAT}**
