#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

#define HALT        '\x00'
#define STORE       '\x01'
#define POP         '\x02'
#define POWER       '\x03'
#define MULTIPLY    '\x04'
#define DIVIDE      '\x05'
#define MODULO      '\x06'
#define ADD         '\x07'
#define SUBTRACT    '\x08'
#define LSHIFT      '\x09'
#define RSHIFT      '\x0a'
#define AND         '\x0b'
#define XOR         '\x0c'
#define OR          '\x0d'
#define MOV         '\x0e'

void error(const char * msg)
{
    printf("%s\n", msg);
    exit(1);
}

typedef struct Header {
    unsigned int magic;
    unsigned char stacks;
} Header;



class Stack {

    public:
        std::vector<unsigned char>* s;
        Stack()
        {
            s = new std::vector<unsigned char>();
        }
};

typedef struct Stacks {
    unsigned char length;
    std::vector<Stack*>* stacks;
} Stacks;


typedef struct Program {
    Stacks * stacks;
    char * bytes;
} Program;

Stacks* initStacks(unsigned char stack_length)
{
    Stacks * stacks = (Stacks *)malloc(sizeof(Stacks));

    stacks->length = stack_length;
    stacks->stacks = new std::vector<Stack*>();

    for (int i = 0; i < stack_length; i++)
    {
        stacks->stacks->push_back(new Stack());
    }

    return stacks;
}

Program * initProgram(unsigned char stacks, char * bytecode) 
{
    Program * program = (Program*)malloc(sizeof(Program));

    program->stacks = initStacks(stacks);

    program->bytes = bytecode;

    return program;
}

Program * init(char * bytes) 
{
    printf("Initializing VM:\n");

    Header * header = (Header*)bytes;

    if (header->magic != 0x41425853) {
        error("Incorrect Magic Number");
    }

    printf("\tMagic Number: \t%c%c%c%c\n", (char)((header->magic & 0xFF000000) >> 24), (char)((header->magic & 0xFF0000) >> 16), 
        (char)((header->magic & 0xFF00) >> 8), (char)(header->magic & 0xFF));

    if (header->stacks > 6) {
        error("Too Many Stacks");
    }

    printf("\tStacks: \t%d\n", header->stacks);

    return initProgram(header->stacks, bytes + sizeof(header));
}

void run(Program * program)
{
    printf("Running Bytecode:\n");
    unsigned int ip = 0;
    unsigned char op = (unsigned char)program->bytes[ip];

    bool store_check = false;
    bool start_check = false;
    bool g8_check = false;
    bool g7_check = false;
    bool __check = false;
    bool stack0_check = false;
    bool stack1_check = false;
    bool stack2_check = false;
    bool stack3_check = false;

    int xor_count = 0;

    unsigned char vals[4];

    while (op != HALT)
    {

        if (op != STORE && !store_check)
        {
            store_check = true;

            start_check = (program->stacks->stacks->at(0)->s->at(0) == 0x41) & (program->stacks->stacks->at(1)->s->at(0) == 0x52)
                & (program->stacks->stacks->at(2)->s->at(0) == 0x43) & (program->stacks->stacks->at(3)->s->at(0) == 0x7b);

            g8_check = (program->stacks->stacks->at(4)->s->at(1) == 0x20) & (program->stacks->stacks->at(4)->s->at(3) == 0x21)
                & (program->stacks->stacks->at(4)->s->at(5) == 0x22) & (program->stacks->stacks->at(4)->s->at(7) == 0x23);

            g7_check = (program->stacks->stacks->at(4)->s->at(9) == program->stacks->stacks->at(4)->s->at(10)) & 
                    (program->stacks->stacks->at(4)->s->at(11) == program->stacks->stacks->at(4)->s->at(12)) & 
                    (program->stacks->stacks->at(4)->s->at(13) == program->stacks->stacks->at(4)->s->at(14)) & 
                    (program->stacks->stacks->at(4)->s->at(15) == program->stacks->stacks->at(4)->s->at(16));

            __check = (program->stacks->stacks->at(4)->s->at(20) == 95) & (program->stacks->stacks->at(4)->s->at(21) == 95);

            stack0_check = (program->stacks->stacks->at(0)->s->at(1) == 'B') & (program->stacks->stacks->at(0)->s->at(2) == '_') & (program->stacks->stacks->at(0)->s->at(3) == 'r');
            stack1_check = (program->stacks->stacks->at(1)->s->at(1) == '0') & (program->stacks->stacks->at(1)->s->at(2) == 't') & (program->stacks->stacks->at(1)->s->at(3) == 'e');
            stack2_check = (program->stacks->stacks->at(2)->s->at(1) == '0') & (program->stacks->stacks->at(2)->s->at(2) == 'h') & (program->stacks->stacks->at(2)->s->at(3) == '_');
            stack3_check = (program->stacks->stacks->at(3)->s->at(1) == 'm') & (program->stacks->stacks->at(3)->s->at(2) == '3') & (program->stacks->stacks->at(3)->s->at(3) == 'g');

        }

        if (op == STORE) 
        {

            if (store_check) exit(-1);

            ip++;
            unsigned char s0 = (unsigned char)program->bytes[ip];
            ip++;
            unsigned char val = (unsigned char)program->bytes[ip];

            program->stacks->stacks->at(s0)->s->push_back(val);

        }
        else if (op == POP)
        {
            ip++;
            unsigned char s0 = (unsigned char)program->bytes[ip];

            program->stacks->stacks->at(s0)->s->pop_back();

        }
        else if (op == POWER)
        {
            ip++;
            unsigned char s0 = (unsigned char)program->bytes[ip];
            ip++;
            unsigned char s1 = (unsigned char)program->bytes[ip];

            unsigned char val0 = program->stacks->stacks->at(s0)->s->back();
            program->stacks->stacks->at(s0)->s->pop_back();
            unsigned char val1 = program->stacks->stacks->at(s1)->s->back();
            program->stacks->stacks->at(s1)->s->pop_back();

            program->stacks->stacks->at(s0)->s->push_back(pow(val1, val0));
            
        }
        else if (op == MULTIPLY)
        {
            ip++;
            unsigned char s0 = (unsigned char)program->bytes[ip];
            ip++;
            unsigned char s1 = (unsigned char)program->bytes[ip];

            unsigned char val0 = program->stacks->stacks->at(s0)->s->back();
            program->stacks->stacks->at(s0)->s->pop_back();
            unsigned char val1 = program->stacks->stacks->at(s1)->s->back();
            program->stacks->stacks->at(s1)->s->pop_back();

            program->stacks->stacks->at(s0)->s->push_back(val1 * val0);
            
        }
        else if (op == DIVIDE)
        {
            ip++;
            unsigned char s0 = (unsigned char)program->bytes[ip];
            ip++;
            unsigned char s1 = (unsigned char)program->bytes[ip];

            unsigned char val0 = program->stacks->stacks->at(s0)->s->back();
            program->stacks->stacks->at(s0)->s->pop_back();
            unsigned char val1 = program->stacks->stacks->at(s1)->s->back();
            program->stacks->stacks->at(s1)->s->pop_back();

            program->stacks->stacks->at(s0)->s->push_back(val1 / val0);
            
        }
        else if (op == MODULO)
        {
            ip++;
            unsigned char s0 = (unsigned char)program->bytes[ip];
            ip++;
            unsigned char s1 = (unsigned char)program->bytes[ip];

            unsigned char val0 = program->stacks->stacks->at(s0)->s->back();
            program->stacks->stacks->at(s0)->s->pop_back();
            unsigned char val1 = program->stacks->stacks->at(s1)->s->back();
            program->stacks->stacks->at(s1)->s->pop_back();

            program->stacks->stacks->at(s0)->s->push_back(val1 % val0);
            
        }
        else if (op == ADD)
        {

            if (__check && (program->stacks->stacks->at(4)->s->size() == 25))
            {
                program->stacks->stacks->at(0)->s->push_back(program->stacks->stacks->at(4)->s->back());
                program->stacks->stacks->at(4)->s->pop_back();

                program->stacks->stacks->at(1)->s->push_back(program->stacks->stacks->at(4)->s->back());
                program->stacks->stacks->at(4)->s->pop_back();

                program->stacks->stacks->at(2)->s->push_back(program->stacks->stacks->at(4)->s->back());
                program->stacks->stacks->at(4)->s->pop_back();

                program->stacks->stacks->at(3)->s->push_back(program->stacks->stacks->at(4)->s->back());
                program->stacks->stacks->at(4)->s->pop_back();

                if ((program->stacks->stacks->at(0)->s->back() ^ program->stacks->stacks->at(3)->s->back()) != 48)
                {
                    exit(-1);
                }
                if ((program->stacks->stacks->at(1)->s->back() ^ program->stacks->stacks->at(3)->s->back()) != 108)
                {
                    exit(-1);
                }
                if ((program->stacks->stacks->at(2)->s->back() ^ program->stacks->stacks->at(3)->s->back()) != 106)
                {
                    exit(-1);
                }
            
            }
            else {

                ip++;
                unsigned char s0 = (unsigned char)program->bytes[ip];
                ip++;
                unsigned char s1 = (unsigned char)program->bytes[ip];

                unsigned char val0 = program->stacks->stacks->at(s0)->s->back();
                program->stacks->stacks->at(s0)->s->pop_back();
                unsigned char val1 = program->stacks->stacks->at(s1)->s->back();
                program->stacks->stacks->at(s1)->s->pop_back();

                program->stacks->stacks->at(s0)->s->push_back(val1 + val0);
            }
            
        }
        else if (op == SUBTRACT)
        {

            if (__check && (program->stacks->stacks->at(4)->s->size() == 21))
            {
                program->stacks->stacks->at(3)->s->push_back(program->stacks->stacks->at(4)->s->back());
                program->stacks->stacks->at(4)->s->pop_back();

                program->stacks->stacks->at(2)->s->push_back(program->stacks->stacks->at(4)->s->back());
                program->stacks->stacks->at(4)->s->pop_back();

                program->stacks->stacks->at(1)->s->push_back(program->stacks->stacks->at(4)->s->back());
                program->stacks->stacks->at(4)->s->pop_back();

                program->stacks->stacks->at(0)->s->push_back(program->stacks->stacks->at(4)->s->back());
                program->stacks->stacks->at(4)->s->pop_back();

                if ((program->stacks->stacks->at(0)->s->back() ^ program->stacks->stacks->at(3)->s->back()) != 104)
                {
                    exit(-1);
                }
                if ((program->stacks->stacks->at(1)->s->back() ^ program->stacks->stacks->at(3)->s->back()) != 55)
                {
                    exit(-1);
                }
                if ((program->stacks->stacks->at(2)->s->back() ^ program->stacks->stacks->at(3)->s->back()) != 108)
                {
                    exit(-1);
                }
            
            }
            else {
                ip++;
                unsigned char s0 = (unsigned char)program->bytes[ip];
                ip++;
                unsigned char s1 = (unsigned char)program->bytes[ip];

                unsigned char val0 = program->stacks->stacks->at(s0)->s->back();
                program->stacks->stacks->at(s0)->s->pop_back();
                unsigned char val1 = program->stacks->stacks->at(s1)->s->back();
                program->stacks->stacks->at(s1)->s->pop_back();

                program->stacks->stacks->at(s0)->s->push_back(val1 - val0);
            }

            
            
        }
        else if (op == LSHIFT)
        {
            ip++;
            unsigned char s0 = (unsigned char)program->bytes[ip];
            ip++;
            unsigned char s1 = (unsigned char)program->bytes[ip];

            unsigned char val0 = program->stacks->stacks->at(s0)->s->back();
            program->stacks->stacks->at(s0)->s->pop_back();
            unsigned char val1 = program->stacks->stacks->at(s1)->s->back();
            program->stacks->stacks->at(s1)->s->pop_back();

            program->stacks->stacks->at(s0)->s->push_back(val1 << val0);
            
        }
        else if (op == RSHIFT)
        {
            ip++;
            unsigned char s0 = (unsigned char)program->bytes[ip];
            ip++;
            unsigned char s1 = (unsigned char)program->bytes[ip];

            unsigned char val0 = program->stacks->stacks->at(s0)->s->back();
            program->stacks->stacks->at(s0)->s->pop_back();
            unsigned char val1 = program->stacks->stacks->at(s1)->s->back();
            program->stacks->stacks->at(s1)->s->pop_back();

            program->stacks->stacks->at(s0)->s->push_back(val1 >> val0);
            
        }
        else if (op == AND)
        {
            ip++;
            unsigned char s0 = (unsigned char)program->bytes[ip];
            ip++;
            unsigned char s1 = (unsigned char)program->bytes[ip];

            unsigned char val0 = program->stacks->stacks->at(s0)->s->back();
            program->stacks->stacks->at(s0)->s->pop_back();
            unsigned char val1 = program->stacks->stacks->at(s1)->s->back();
            program->stacks->stacks->at(s1)->s->pop_back();

            program->stacks->stacks->at(s0)->s->push_back(val0 & val1);

            if (val0 == val1) {

                vals[xor_count] = val0 & val1;

                xor_count++;

                if (xor_count >= 4)
                {

                    bool test = true;

                    if ((vals[0] & vals[1]) != 36) test = false;
                    if ((vals[0] & vals[2]) != 100) test = false;
                    if ((vals[1] & vals[2]) != 38) test = false;
                    if ((vals[1] & vals[3]) != 52) test = false;
                    if ((vals[0] + vals[1] + vals[2] + vals[3]) != 317) test = false;
                    if ((vals[0] + vals[1] + vals[3]) != 207) test = false;
                    if ((vals[0] + vals[1] + vals[2]) != 265) test = false;
                    if ((vals[3] + vals[1] + vals[2]) != 217) test = false;

                    if (!test) exit(-1);

                } 

            }
            
        }
        else if (op == XOR)
        {
            ip++;
            unsigned char s0 = (unsigned char)program->bytes[ip];
            ip++;
            unsigned char s1 = (unsigned char)program->bytes[ip];

            unsigned char val0 = program->stacks->stacks->at(s0)->s->back();
            program->stacks->stacks->at(s0)->s->pop_back();
            unsigned char val1 = program->stacks->stacks->at(s1)->s->back();
            program->stacks->stacks->at(s1)->s->pop_back();

            program->stacks->stacks->at(s0)->s->push_back(val0 ^ val1);

            if (val1 == 0x23)
            {
                vals[3] = val0 ^ val1;
            }
            else if (val1 == 0x22)
            {
                vals[2] = val0 ^ val1;
            }
            else if (val1 == 0x21)
            {
                vals[1] = val0 ^ val1;
            }
            else if (val1 == 0x20)
            {
                vals[0] = val0 ^ val1;

                bool test = true;


                if ((vals[0] ^ vals[1]) != 92)
                {
                    test = false;
                }

                if ((vals[0] ^ vals[2]) != 25)
                {
                    test = false;
                }

                if ((vals[0] ^ vals[3]) != 94)
                {
                    test = false;
                }

                if ((vals[1] ^ vals[2]) != 69)
                {
                    test = false;
                }

                if ((vals[1] ^ vals[3]) != 2)
                {
                    test = false;
                }

                if ((vals[2] ^ vals[3]) != 71)
                {
                    test = false;
                }

                if ((vals[0] ^ vals[1] ^ vals[2] ^ vals[3]) != 27)
                {
                    test = false;
                }

                if ((vals[0] ^ vals[1] ^ vals[3]) != 111)
                {
                    test = false;
                }


                if (!test) exit(-1);
            }
            
        }
        else if (op == OR)
        {
            ip++;
            unsigned char s0 = (unsigned char)program->bytes[ip];
            ip++;
            unsigned char s1 = (unsigned char)program->bytes[ip];

            unsigned char val0 = program->stacks->stacks->at(s0)->s->back();
            program->stacks->stacks->at(s0)->s->pop_back();
            unsigned char val1 = program->stacks->stacks->at(s1)->s->back();
            program->stacks->stacks->at(s1)->s->pop_back();

            program->stacks->stacks->at(s0)->s->push_back(val0 | val1);
            
        }
        else if (op == MOV)
        {
            ip++;
            unsigned char s0 = (unsigned char)program->bytes[ip];
            ip++;
            unsigned char s1 = (unsigned char)program->bytes[ip];

            unsigned char val1 = program->stacks->stacks->at(s1)->s->back();

            program->stacks->stacks->at(s1)->s->pop_back();

            program->stacks->stacks->at(s0)->s->push_back(val1);
            
        }

        ip++;
        op = program->bytes[ip];
    }

    start_check = start_check  & (program->stacks->stacks->at(0)->s->back() == 0x7d);

    if (start_check & g8_check & g7_check & __check & stack0_check & stack1_check & stack2_check & stack3_check)
    {

        printf("Flag: ");

        int j = 8;

        for (int i = 0; i < j; i++)
        {
            printf("%c", program->stacks->stacks->at(0)->s->at(i));
            printf("%c", program->stacks->stacks->at(1)->s->at(i));
            printf("%c", program->stacks->stacks->at(2)->s->at(i));
            printf("%c", program->stacks->stacks->at(3)->s->at(i));
        }
        printf("%c\n", program->stacks->stacks->at(0)->s->at(j));
    }

}

int main(int argc, char * argv[])
{

    if (argc != 2 || !strstr(argv[1], ".arc")) {
        error("Please run the VM with a valid file");
    }

    std::ifstream ifs(argv[1], std::ios::binary|std::ios::ate);

    int length = ifs.tellg();

    char *bytes = new char[length];

    ifs.seekg(0, std::ios::beg);

    ifs.read(bytes, length);

    ifs.close();

    Program * program = init(bytes);

    run(program);

    printf("Closing VM...\n");
    
    delete[] bytes;

    return 0;
}