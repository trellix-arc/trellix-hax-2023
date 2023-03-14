.PHONY: all
all: chal distrib


chal: main.c
	gcc -m64 -o chal_clear main.c md5.c
	python3 obfuscate.py 
	chmod +x ./chal
	strip -s ./chal

distrib: chal
	mkdir -p distrib
	openssl enc -aes-256-cbc -pbkdf2 -in secret_clear.txt  -out ../challenge/secret_encrypted.txt -k "ARC{Obfusc4ti0n_4_lyfe}"
	mv chal ../challenge/chal
	rm ./chal_clear

clean:
	rm chal chal_clear *.o 