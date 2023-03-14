
#define _GNU_SOURCE 
#include <stdio.h>
#include <sys/mman.h>   
#include <string.h>
#include <stdlib.h>


#include "md5.h"

// gcc -Wl,--omagic 
// for writable page

#define MAX_OBF_SIZE 0x1000*0x3
#define PAGE_MASK 0xFFFFFFFFFFFFF000


//#define VERBOSE_PRINT

typedef unsigned long long u64;

u64 last_rip = 0;
u64 last_start = 0;
u64 last_end_pos = 0;


__attribute__((always_inline))
static inline void start_obf_marker()
{
    asm("pushq %%rax\n\t pushq $0xDEAD\n\t popq %%rax\n\t popq %%rax\n\t"  
        : 
    );
    
}


__attribute__((always_inline))
static inline void end_obf_marker()
{
    asm("pushq %%rax\n\t pushq $0x13371337\n\t popq %%rax\n\t popq %%rax\n\t"   // \x50\x68\x37\x13\x37\x13\x58\x58
        :
    );
}

__attribute__((always_inline))
static inline u64  get_rip()
{
    u64 res;
    asm("call .+5\npop %0"
        :"=r" (res)
    );
    return (void*) res;
}

__attribute__((always_inline))
static inline void start_obf()
{
    u64 rip = get_rip();
    //last_rip = rip;


    unsigned char marker_end[8];
    unsigned char marker_start[8];
    //"\x50\x68\x37\x13\x37\x13\x58\x58" ^ 0x42
    //"\x50\x68\xAD\xDE\x00\x00\x58\x58" ^ 0x42  start
    // we use that instead of the actual marker so that we don't accidentally find one
    memcpy(marker_end,   "\x12*uQuQ\x1a\x1a", sizeof(marker_end));  
    memcpy(marker_start, "\x12\x2a\xef\x9c\x42\x42\x1a\x1a", sizeof(marker_start));
    for (int i=0 ; i < 8; i++)
    {
        marker_end[i] ^= 0x42;
        marker_start[i] ^= 0x42;
    }
    u64 end_pos = memmem(rip, MAX_OBF_SIZE, marker_end, sizeof(marker_end));
    u64 start_pos = memmem(rip, MAX_OBF_SIZE, marker_start, sizeof(marker_start)) + sizeof(marker_start);

    if ((end_pos == 0) || (start_pos == sizeof(marker_start)))
    {
        puts("error");
        exit(-2);
    }


#ifdef VERBOSE_PRINT
    printf("star: %p\n", start_pos);
    printf("end: %p\n", end_pos);
#endif
    last_end_pos = end_pos;
    last_start = start_pos;



    u64 sz = end_pos - start_pos;
    u64 target_rip = rip&PAGE_MASK;
    last_rip = target_rip;
#ifdef VERBOSE_PRINT
    printf("Target rip: %p\n", target_rip);
#endif
    int res = mprotect(target_rip, MAX_OBF_SIZE, PROT_EXEC | PROT_READ | PROT_WRITE );
    if (res)
    {
        perror("Error while mprotect\n");
        exit(-1);
    }


    for (u64 i=0; i < last_end_pos - last_start; i++)
    {
       (*(unsigned char*) (last_start + i)) ^= (i % 0x100);
       //(*(char*) (last_start + i)) ^= (i % 0x100);
    }


#ifdef VERBOSE_PRINT
    printf("deobf ok\n");
#endif

    start_obf_marker();
    


}



__attribute__((always_inline))
static inline void end_obf()
{
    end_obf_marker();

    for (u64 i=0; i < last_end_pos - last_start; i++)
    {
       (*(unsigned char*) (last_start + i)) ^=  (i % 0x100);
    }
    last_end_pos = 0;
    last_start = 0;
    int res = mprotect(last_rip, MAX_OBF_SIZE, PROT_EXEC | PROT_READ  );

}




//Flag : ARC{Obfus4ti0n_4_lyfe}
#define FLAG_LEN 23

//check length
//maybe add more checks like valid charset ?
int check1(char* flag)
{
    start_obf();
    if(strlen(flag) != FLAG_LEN)
    {
        puts("Nope.");
        exit(-2);
    }
    end_obf();

    return 1;

}

//Check format ARC{...}
int check2(char* flag)
{
    start_obf();
    int res = 1; 
    res &= (flag[0] ^ 'A') == 0;
    res &= (flag[1] ^ 'R') == 0;
    res &= (flag[2] ^ 'C') == 0;
    res &= (flag[3] ^ '{') == 0;
    res &= (flag[FLAG_LEN-1] ^ '}') == 0;
    end_obf();

    return res;

}

//check Obfus4ti0n_4_
int check3(char* flag)
{
    start_obf();
    int res = 1; 

    int off_start = 4;

    char* input = flag + off_start;

    char* magic_blob = "\x20\x2f\x7c\xe6\x9f\xc7\x85\xf0\x3d\x36\xa5\xbe\xd0\x64\x2a\x4a\x55\x14\x71\xbf\x36\x38\x91\xce\x8d\xa7\x07\x58";
    int sz = strlen(magic_blob)/2;

    for (int i=0; i < sz; i ++)
    {
        unsigned char val = magic_blob[2*i] + magic_blob[2*i+1];
        res &= (input[i] == val) ; 
        
        #ifdef VERBOSE_PRINT
        printf("input[i] %c, val: %c\n", val );
        #endif

    }


    end_obf();

    return res;

}


//check lyfe
//>>> hashlib.md5(b"lyfe").hexdigest()
//'855eaad768495f4066f26e7e625113fa'
int check4(char* flag)
{
    start_obf();
    int off_start = 18;
    int res = 1;

    uint8_t* input1  = flag + off_start;
    int input1_size = strlen(input1) -1; // -1 to remove the } 


    MD5Context ctx;
	md5Init(&ctx);
    md5Update(&ctx, input1, input1_size);
    md5Finalize(&ctx);

    res = memcmp(ctx.digest, "\x85\x5e\xaa\xd7\x68\x49\x5f\x40\x66\xf2\x6e\x7e\x62\x51\x13\xfa", 16) == 0;

    end_obf();

    return res;

}


int main(int argc, char** argv)
{
    if(argc < 2)
    {
        puts("Usage: chal password\n");
        exit(-1);
    }

    char* flag = argv[1];
    //int res1 = 1;
    int res1 = check1(flag);
    int res2 = check2(flag);
    int res3 = check3(flag);
    int res4 = check4(flag);

    if (res1 && res2 && res3 && res4)
    {
        printf("Woot! The flag is %s\n", flag);
        char buffer[256];
        snprintf(buffer, 256, "openssl enc -d -aes-256-cbc -pbkdf2 -in secret_encrypted.txt -k \"%s\"", flag);
        system(buffer);
    }
    else
    {
        puts("Try again!");
    }




}