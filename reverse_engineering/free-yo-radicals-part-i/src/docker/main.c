/* simple-server.c
 *
 * Copyright (c) 2000 Sean Walton and Macmillan Publishers.  Use may be in
 * whole or in part in accordance to the General Public License (GPL).
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*****************************************************************************/
/*** simple-server.c                                                       ***/
/***                                                                       ***/
/*****************************************************************************/

/**************************************************************************
 *	This is a simple echo server.  This demonstrates the steps to set up
 *	a streaming server.
 **************************************************************************/
//branch leak
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <byteswap.h>
#include <stdbool.h>

#define MY_PORT		9999
#define MAXBUF		46
#define MAX_ENTRIES 6


unsigned char deliminator = '\x0e';
unsigned char terminator = '\x0c';

typedef struct packet {
    int clientfd;
    int payload_ip;
    int payload_port;
    int timestamp;
    int action;
    char datafield_1[16];
    char datafield_2[8]; //long
}packet;

struct string {
    char buffer[17];
    char* (* print)(char *,char*);
    struct string *next;
};

struct number {
    unsigned long num;
    char* (* print)(unsigned long, char*);
    struct number *next;
    char reserved[9];
};

struct string *string_head = NULL;
struct string *string_current = NULL;
struct number *number_head = NULL;
struct number *number_current = NULL;

char* send_num(unsigned long a_num, char* buffer)
{
    sprintf(buffer,"mSv: %lu ", a_num);
    return buffer;
}

char* send_str(char * a_str,char* buffer)
{
    sprintf(buffer,"Location: %s\t", a_str);
    return buffer;
}

void print_flag(int client) {
    FILE *flag;
    char flag_txt[50];
    flag = fopen("./flag", "r");
    fgets(flag_txt, 50, flag);
    send(client, flag_txt, strlen(flag_txt), 0);
}

int validate(unsigned char* input)
{
    int expected_size = 46;
    int deliminator_locations[9] = {4, 7, 12, 15, 20, 25, 30, 35, 44};
        /*
           fields_desc = [ IPField("ipaddress", "127.0.0.1"),
           XByteField("seperator",delim ),
           ShortField("port",9999),
           XByteField("seperator",delim ),
           UTCTimeField("time",1589852355),
           XByteField("seperator",delim ),
           ShortEnumField("action",1,{1:"create",2:"delete",3:"print"}),
           XByteField("seperator",delim ),
           delimField("data","aaaabbbbccccdddd",chr(delim)), #MAX 20 CHARS DELIM EVERY 4
           XByteField("seperator",delim ),
           LongField("value",8),
           XByteField("seperator",delim),
           XByteField("terminator",term)
           ]
        */

    for(int i=0; i < 9; i++) {
        //Check for IPaddress
        if(memcmp(&deliminator,(char*)input+deliminator_locations[i],sizeof(char))) {
            printf("Error: Found bad deliminator, Invalid packet\n");
            return 0;
        }
    }
    //test the last character is the terminating char
    if(memcmp(&terminator,(char*)input+expected_size-1,sizeof(char)))
    {
        printf("Error: Incorrect termination, Invalid packet\n");
        return 0;
    }


    printf("Valid packet\n");
    return 1;
}

void parse_packet(struct packet *p, unsigned char* payload)
{
    int temp = 0;
    if(validate(payload))
    {
        memcpy(&p->payload_ip, (int*)payload,4);        // 4
        memcpy(&p->payload_port, (char*)payload+5,2);   // 2
        memcpy(&p->timestamp, (char*)payload+8,4);      // 4
        memcpy(&p->action, (char*)payload+13,2);        // 2
        temp = htons(p->action);
        p->action = temp;
        memcpy(p->datafield_1, (char*)payload+16,4);    // 4
        memcpy(p->datafield_1+4, (char*)payload+21,4);
        memcpy(p->datafield_1+8, (char*)payload+26,4);
        memcpy(p->datafield_1+12, (char*)payload+31,4);
        memcpy(p->datafield_2, (char*)payload+36,8);
    }
}

int checkForTermination(char* str, int size)
{
    bool found = false;
    for(int i=0; i < size; i++) {
        if(str[i] == terminator)
            found = true;
    }
    return found;
}

void zechallenge(int clientfd)
{
    struct string *curstr = NULL;
    struct number *curnum = NULL;
    const char *hellomessage = "Connected\n\0";
    const char *optionsmessage = "\nWaiting for data... \n\0";
    const char *byemessage = "\n[+] Bye bye!\n\0";
    bool ops_completed[4] = { false, false, false, false };

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    int loopout = 0;
    int numentries = 0;
    unsigned int choice = 0;
    unsigned int index = 0;
    char buffer[MAXBUF];
    char message[MAXBUF];
    int poll_ret;
    int recv_ret;

    // Set Timeouts
    setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    setsockopt(clientfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);
    // Send hello message
    send(clientfd, hellomessage, strlen(hellomessage), 0);

    while (!loopout)
    {
        if (send(clientfd, optionsmessage, strlen(optionsmessage), 0) < 0) break;

        if (recv(clientfd, buffer, MAXBUF, 0))
        {
            packet p = (struct packet) {0,0,0,0,0};
            parse_packet(&p,buffer);
            printf("action: %d\n",p.action);
            switch (p.action)
            {
                case 1:
                    // Create

                    // For level00
                    if(!ops_completed[0])
                        ops_completed[0] = true;

                    if(numentries < MAX_ENTRIES)
                    {
                        if(!checkForTermination(p.datafield_1, 16))
                        {
                            curstr = malloc(sizeof(struct string));
                            memcpy(curstr->buffer, (char*)p.datafield_1, 16);
                            curstr->buffer[16] = '\0';
                            /* add a print function */
                            curstr->print = send_str;
                            /* store the string to our master list */
                            curstr->next = string_head;
                            string_head = curstr;
                            printf("Created new coordinate\n");
                        }
                        else
                            printf("Error: Termination char found in coordinate\n");

                        if(!checkForTermination(p.datafield_2, 8))
                        {
                            curnum = malloc(sizeof(struct number));

                            memcpy(&curnum->num, (char*)p.datafield_2, sizeof(long));
                            curnum->num = bswap_64(curnum->num);
                            /* pick a print function */
                            curnum->print = send_num;

                            /* store the number to our master list */
                            curnum->next = number_head;
                            number_head = curnum;
                            printf("Created new radiation value!\n");
                        }
                        else
                            printf("Send error message. Termination char found in radiation value\n\n");

                        numentries++;
                        break;
                    }
                    else {
                        printf("Error: Max Entries reached\n");
                        break;
                    }


                case 2:
                    //delete string

                    // For level00
                    if(!ops_completed[1])
                        ops_completed[1] = true;

                    if(numentries && string_head != NULL && !checkForTermination(p.datafield_1, 16))
                    {
                        struct string *current = string_head;
                        struct string *previous = NULL;
                        while(current != NULL) {
                            if(!strcmp(p.datafield_1,current->buffer)){
                                // delete the node
                                printf("Deleted requested coordinate\n");
                                // Check if head
                                if(current == string_head) {
                                    string_head = string_head->next;
                                } else {
                                    previous->next = current->next;
                                }
                                free(current);
                                break;
                            }
                            previous = current;
                            current = current->next;
                        }
                    }
                    else{
                        printf("Warning: There are no coordinates left to delete\n");
                    }

                    if(numentries && number_head != NULL && !checkForTermination(p.datafield_2, 8))
                    {
                        struct number *current = number_head;
                        struct number *previous = NULL;
                        unsigned long del_value = 0;
                        memcpy(&del_value, (char*)p.datafield_2, sizeof(long));
                        del_value = bswap_64(del_value);
                        while(current != NULL) {
                            if(del_value == current->num)
                            {
                                printf("Deleted requested radiation value\n");
                                // Check if head
                                if(current == number_head) {
                                    number_head = number_head->next;
                                } else {
                                    previous->next = current->next;
                                }
                                free(current);
                                break;
                            }
                            previous = current;
                            current = current->next;
                        }
                    }
                    else{
                        printf("Warning: There are no radiation values left to delete\n");
                    }
                    numentries--;
                    break;
                case 3:
                    // Print

                    // For level00
                    if(!ops_completed[2])
                        ops_completed[2] = true;

                    if (numentries)
                    {
                        unsigned long print_value = 0;
                        int count = 0;
                        struct string *tempstr = NULL;
                        struct number *tempnum = NULL;

                        memcpy(&print_value, (char*)p.datafield_2, sizeof(long));
                        print_value = bswap_64(print_value);
                        if(print_value == 1) {
                            tempstr = curstr;
                            tempnum = curnum;
                            count = MAX_ENTRIES -1;

                        } else {
                            tempstr = string_head;
                            tempnum = number_head;
                        }
                        while(tempnum != NULL && tempstr != NULL && count < MAX_ENTRIES)
                        {
                            tempnum->print(tempnum->num,message);
                            send(clientfd, message, strlen(message), 0);
                            tempstr->print(tempstr->buffer,message);
                            send(clientfd, message, strlen(message), 0);
                            tempnum = tempnum->next;
                            tempstr = tempstr->next;
                            count++;
                        }
                    }
                    else
                        printf("Warning: There is no entries to print\n");
                    break;
                case 5:
                    // exit

                    // For level00
                    if(!ops_completed[3])
                        ops_completed[3] = true;

                    if (ops_completed[0] && ops_completed[1] && ops_completed[2] && ops_completed[3])
                        print_flag(clientfd);

                    loopout = 1;
                    send(clientfd, byemessage, strlen(byemessage), 0);
                    close(clientfd);
                    exit(0);
                    break;
            }
        } else break;
    }
    close(clientfd);
}

int main(int argc, char *argv[])
{
    errno = 0;
    char *p;
    struct sockaddr_in self;
    int port = MY_PORT;
    int clientfd;
    int option = 1;
    struct sockaddr_in client_addr;
    int addrlen=sizeof(client_addr);
    pid_t parent = getpid();
    pid_t child;

    if (argc == 2)
        port = strtol(argv[1], &p, 10);
    printf("[+] Listening port: %d\n", port);

    int sockfd;
    /*---Create streaming socket---*/
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror("Socket");
        exit(errno);
    }
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    /*---Initialize address/port structure---*/
    bzero(&self, sizeof(self));
    self.sin_family = AF_INET;
    self.sin_port = htons(port);
    self.sin_addr.s_addr = INADDR_ANY;

    /*---Assign a port number to the socket---*/
    if (bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0 )
    {
        perror("socket--bind");
        exit(errno);
    }

    /*---Make it a "listening socket"---*/
    if ( listen(sockfd, 20) != 0 )
    {
        perror("socket--listen");
        exit(errno);
    }

    /*---Forever... ---*/
    /*---accept a connection (creating a data pipe)---*/
    while (1)
    {
        //struct packet *client; //is this going to cause a problem of overwriting the same struct for each connection?
        clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
        client_addr=client_addr;
        printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        child = fork();
        switch (child) {
            case -1:
                perror("Failed to fork");
                return 1;
            case 0:
                // Child Process
                zechallenge(clientfd);
            default:
                // Parent Process
                close(clientfd);
        }
    }

    /*---Clean up (should never get here!)---*/
    close(sockfd);
    return 0;
}
