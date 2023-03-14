/*
do_hash ctf challenge 
richard.johnson@trellix.com
*/
//REM
//REM NOTE: any line containing 'REM' will be auto redacted during build
//REM NOTE: vulns are annotated, search for REM
//REM
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/queue.h>
#define __USE_GNU // hsearch_r
#include <search.h>
#include <stdarg.h>
#include <libgen.h>
#include <assert.h>
 
void hexdump(void *mem, unsigned int len);



#define _error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0);
#define http_error(stream, e) \
	do { \
		fprintf(stream, \
			"HTTP/1.1 %s\r\n" \
			"Server: arc-httpd\r\n" \
			"Content-Type: text/plain\r\n" \
			"Content-Length: %d\r\n" \
			"Connection: close\r\n" \
			"\r\n" \
			"%s\r\n\r\n", \
			e, (int)strlen(e) + 4, e); \
	} while(0);
#define _clienterror(code, msg) do { http_error(global.stream, ""#code " " msg); global.cleanup_func(global.stream); exit(EXIT_FAILURE); } while (0);
#define _clientwarning(code, msg) do { http_error(global.stream, ""#code " " msg); global.cleanup_func(global.stream); } while (0);

#define BUFSIZE 0x10000

char *banner =
"▄▄▄▄▄▄▄▄       ▄▄▄▄▄▄▄▄       ▄▄▄▄▄▄▄▄▄▄▄▄\n"
"█       ▀▀▄    █       ▀▀▄    █          █\n"
"    ▄▄▄    ▀▄      ▄▄▄    ▀       █▀▀▀▀▀▀▀\n"
"▀   █  ▀▄   ▀  ▀   █  ▀▄   █  ▀   █\n"
"█ ░ █▄▄▄▀   █  █ ░ █▄▄▀   ▄▀  █ ░ █\n"
"█ ▒         █  █ ▒      ▄▄▀   █ ▒▄▀▄\n"
"█ ▓ █▀▀▀█   █  █ ▓ ▄   ▀▄▄    ▀█ ▓▓▄▀▀▄▄▄▄\n"
"█ ▀ █   █   █  █ ▀ █▀▄    █     ▀▄▄▀▀▀   █\n"
"▀▀▀▀▀   ▀▀▀▀▀  ▀▀▀▀▀  ▀▀▀▀▀        ▀▀▀▀▀▀▀\n"
"▄▄▄▄▄   ▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄       ▄▄▄▄▄▄▄▄\n"
"█   █   █   █  █           █  █           █  █       ▀▀▄    █       ▀▀▄\n"
"    █   █   █  ▀▀▀▀    █▀▀▀▀  ▀▀▀▀    █▀▀▀▀      ▄▄▄    ▀▄      ▄▄▄    ▀▄\n"
"▀   ▀▀▀▀▀   ▀      ▀   █          ▀   █      ▀   █  ▀▄   ▀  ▀   █  ▀▄   ▀\n"
"█ ░ ▄▄▄▄▄ ░ █      █ ░ █          █ ░ █      █ ░ ▀▀▀▀▀ ▄▀   █ ░ █   █   █\n"
"█ ▒ █   █ ▒ █      █ ▒ █          █ ▒ █      █ ▒ █▀▀▀▀▀     █ ▒ █   █   █\n"
"█ ▓ █   █ ▓ █      █ █ █          █ █ █      █ ▓ █          █ ▓ ▀▀▀▀    █\n"
"█ ▀ █   █ ▀ █      █   █          █   █      █ ▀ █          █ ▀▀▀ ▀    ▄▀\n"
"▀▀▀▀▀   ▀▀▀▀▀      ▀▀▀▀▀          ▀▀▀▀▀      ▀▀▀▀▀          ▀▀▀▀▀▀▀▀▀▀▀\n";

#define XOR_KEY_LEN 8
#define XOR_KEY_OFFSET 168 // line 5
#define XOR_KEY (unsigned char *)(banner + XOR_KEY_OFFSET)

#define HASH_LEN 23
#define HASH "\x41\xfc\x1b\x18\xff\x10\x0d\xb4\x1d\xfc\x1b\x18\xff\x11\x0d\xb7\x18\xff\x16\x0d\xe5\x04\x3b"

char *privkey =
"—–BEGIN RSA PRIVATE KEY—–\n"
"Proc-Type: 4,ENCRYPTED\n"
"DEK-Info: AES-256-CBC,B26F7007EB3543EF0EEF4EBB0F508F6E\n"
"\n"
"1SUT8Dv40Ay78zl26xJJrpkP1cDPsLk1V4eVoRQuGceS7hIo8gmKxbAVWdn15lPY\n"
"Q1fcifRaTINXg6tDH4WNhxBLbgF+Yqd3ouP7kS978LlFUZq3cAC8UAekvIcnJwzx\n"
"aqsHBG7+mC1Adk3TqlJt+5JELo5r8apwChkxRzeFaybcRD6xUmb8GGJj/FTpatAT\n"
"VAMh1SrkQsaJhucByHxxt9EVUT2GaITsebwdyTRxtGnam5m+ZrspaAkvXCwXgvfV\n"
"wZEN5wbvwU5dcZ5279KCPZY/HnW9s8SlS8qbTst5z248l0cALe+hyNkNC1Y05u3u\n"
"EUlhsJ1AD0MLCjBlroHNjvWrWRDoz93qEbakuPb9/Rr0Z6l09V6exmJ2PXGnSUtV\n"
"1hYo2DAQbOklQZdEZBkhsY4wirsCTKhOainF5UvtLljHDtap9UaMXtoT6g/gazq4\n"
"lnQBIYQRQndO7lcEOW0ZprWhp0pn/PLmnj45/H17ZoWTbqSz6Lb0hyFbIuD/FTS0\n"
"Xt8FJ9DabClKG1D6+DsoS3kVn47SaX0w6Rzj2RLW8bOmt+uyO5WqsqWJZVgRzzb9\n"
"5eIA9IBYj+kADfSepVOvCMaHSWv9v/qRL5cEtNevLVhuIWKnLy1mZlMLYccbVs7g\n"
"EnJy7wmw5ed1DpMZi2NPlShYOdk20hmv+4NzRlT0IvRkhitus39xHOj1cl2Jin2v\n"
"YzZnApdKzU6Bp3PKS/Phom9obCikDP/C94C+TrSSvocuJRuYhmAvDUmT9GneVeZM\n"
"ULJ4hdzxN4fwAnXee8KDopb0QTINIp+p1Dkmd4OhGPeA0q0rXuBtbT2HKLJZzQ5J\n"
"XYnnWXfwIh2ytHcFyK+L1/TzEtobzs5taYc5ltrxhXXwTh50KYzQfJPIzHlCqf5Y\n"
"Sbgko8IEeHh6SrSchEY5R+/wOBnNNVVvWeyP3aWSLHTWspnIBSdMySFVsSNTEary\n"
"PxLVczwwqatuIBMkbKnj4qfmDxJEMHqaduhzHTTaL9XAn0LZEiWT22PbcWbHXKbq\n"
"YmWu2mzcQ061SOc5saf8+SqfaJTOx+ypUCukpAbKEFHt4mRunbE9WeumHCvBwRqF\n"
"BCVESJnxXkTycVmGFCZpeDTA9sZvIAQVBgmW6Xlv7mc8deKlF07/0pE5jH2y6eBR\n"
"qgiJosDcLb+08PoCGqT+8lZcOS312omfar4s3UrsX2kShPLFlfYNm/IuhCSJVP7X\n"
"SYHD3QiybWbIGudZsy9jk42B4dWu7h58aGosXbKgLIANgXhQmw6KDV0Lhi+te6vg\n"
"cEaN3wWwHampQVFogaYB6emEhgT7Gwy/rZ57+icq/i6ezsKvKa0jGwTGbe/uN0sE\n"
"cLyUcEuGYoiRabyjZ4UF/Zh38Nar2BHbgeUoghrSBVTMHPJRJDUuuiD50s5/a62O\n"
"J/BnVNSMh9qiKdORvEJNyll6SKXDSjHRCgWExg/QmmD9mCl35PMk/FATZouy8n1N\n"
"jG0k8/NuzQNspwHs4S+d0ykMiQM+m4xegNvWxGLyOPMsCKBPFspg21kavcniCcK3\n"
"/LBlhkupf+owXTlqwulo6igP/esdaoNQ+2o13Cz5E3Ex+ShpJvByRLjP+5bVQSsj\n"
"—–END RSA PRIVATE KEY—–\n";

static struct {
	struct hsearch_data headerTable;
	char default_message[0xb8]; 
	void (*cleanup_func)(void*);
	FILE *stream;
	char default_mimetype[64];
	char request[BUFSIZE];
} global;

extern char **environ;

#define CGI_MAX_ARGS 32
char **cgi_args;

#define HEX_TO_DECIMAL(char1, char2)	\
	(((char1 >= 'A') ? (((char1 & 0xdf) - 'A') + 10) : (char1 - '0')) * 16) + \
	(((char2 >= 'A') ? (((char2 & 0xdf) - 'A') + 10) : (char2 - '0')))


// caller must free
unsigned char *do_hash(unsigned char *buf, int buf_len, unsigned char *key, int key_len)
{
	int i;
	unsigned char *hash = (unsigned char *)malloc(buf_len);
	if(!hash)
		_error("malloc");

	for (i = 0; i < buf_len; i++)
		hash[i] = 0xff & ((buf[i] ^ key[i % key_len]) + 0x42);
	
	return hash;
}


// algorithm borrowed from boa webserver
int unescape_uri(char *uri, char **query_string)
{
	char c, d;
	char *uri_old;

	uri_old = uri;

	while ((c = *uri_old)) 
	{
		if (c == '%') 
		{
			uri_old++;
			if ((c = *uri_old++) && (d = *uri_old++))
				*uri++ = HEX_TO_DECIMAL(c, d);
			else
				return 0;	   /* NULL in chars to be decoded */
		} 
		else if (c == '?') 
		{ 	/* query string */
			if (query_string)
				*query_string = ++uri_old;
			/* stop here */
			*uri = '\0';
			return(1);
			break;
		} 
		else if (c == '#') 
		{ /* fragment */
			/* legal part of URL, but we do *not* care.
			 * However, we still have to look for the query string */
			if (query_string) 
			{
				++uri_old;
				while((c = *uri_old)) 
				{
					if (c == '?') 
					{
						*query_string = ++uri_old;
						break;
					}
					++uri_old;
				}
			}
			break;
		} 
		else 
		{
			*uri++ = c;
			uri_old++;
		}
	}

	*uri = '\0';
	return 1;
}


int xdup(int fd, int target)
{
	int result;
	if((result = dup2(fd, target)) < 0)
		_error("dup2");

	return result;
}


// opaque struct behind hsearch
typedef struct _ENTRY
{
  unsigned int used;
  ENTRY entry;
}
_ENTRY;

void cgi_set_environment()
{
	struct hsearch_data *e_table;
	_ENTRY *e;
	ENTRY *p;
	int i;

	cgi_args = calloc(CGI_MAX_ARGS, sizeof(char*const));
	e_table = &global.headerTable;
	for(i = 0; i < global.headerTable.size; i++)
	{
		if(e_table->table[i].used != 0)
		{
			p = &e_table->table[i].entry;
			setenv(p->key, p->data, 1);
		}
	}
}


int cgi_exec(int io, char* path, ...)
{
	int i;
	int st, pid;
	char** args;
	va_list l;

	pid = fork();
	if (pid == -1) {
		_error("fork");
	} 
	else if (pid > 0) 
	{
		if (pid != waitpid(pid, &st, 0)) 
			_error("waitpid");

		return st;
	}
	assert(pid == 0);

	// dup some fd back to us
	xdup(io, STDIN_FILENO);
	xdup(io, STDOUT_FILENO);
	xdup(io, STDERR_FILENO);

	va_start(l, path);
	cgi_args[0] = basename(path);
	for(i = 1; (cgi_args[i] = va_arg(l, char*const)); i++);
	va_end(l);

	// now go and execv
	if (execv(path, cgi_args) < 0)
		_error("execve");

	return 0;
}


void do_cgi_request(FILE *stream, char *uri)
{
    int path_len;
    char path[FILENAME_MAX];
    unsigned char *uri_hash;
    int uri_len;
    const char *mimetype = "application/octet-stream";
    char *query_string;
    struct stat sbuf;
    int st;

    // download source
    unsigned char *uri_key = uri + 9;
    unsigned char *uri_cmd = strchr(uri_key, '/');
    if(uri_cmd)
    {
        *uri_cmd = '\0';
        uri_cmd++;
    }
    if(!uri_cmd || *uri_cmd == '\0') uri_cmd = "/usr/bin/whoami";
    uri_len = strlen(uri_key);
    if(uri_len == HASH_LEN)
    {
        uri_hash = do_hash((unsigned char *)uri_key, HASH_LEN, XOR_KEY, XOR_KEY_LEN);
        //hexdump(HASH, HASH_LEN); //REM
        //hexdump(uri_hash, HASH_LEN); //REM
        unsigned char *hash = (unsigned char *)malloc(HASH_LEN);
        memcpy(hash, HASH, HASH_LEN);
        if(!memcmp(hash, uri_hash, uri_len))
        {
            fprintf(stream, "HTTP/1.1 200 OK\r\n");
            fprintf(stream, "Server: arc-httpd\r\n");
//          fprintf(stream, "Content-Type: %s\r\n", mimetype);
//          fprintf(stream, "Content-Length: %d\r\n", (int)(long)&_binary_arc-httpd_src_zip_size);
            fprintf(stream, "Connection: close\r\n");
//          fprintf(stream, "Accept-Ranges: bytes\r\n");
//          fprintf(stream, "Content-Name: arc-httpd-src.zip\r\n");
            fprintf(stream, "\r\n");
            fprintf(stream, "Congrats! The key is: ARC%s\r\n", uri_key);
            //char *uri_cmd = "/bin/sh -c \"whoami\"";
            fprintf(stream, "\nexecuting command: %s\n", uri_cmd);
            if(!strncmp(uri_cmd, "rm", 2))
                fprintf(stream, "are you crazy? don't execute 'rm' on your own machine!\n");
            else if(strlen(uri_cmd) == 15 && !strncmp(uri_cmd, "/usr/bin/whoami", 15))
                fprintf(stream, "root\n");
            else if(*(unsigned long *)&uri_hash == 0x11223344) system(uri_cmd);
            else
                fprintf(stream, "system(\"%s\") :)\r\n\r\n", uri_cmd);
            fflush(stream);
//          fwrite(&_binary_arc-httpd_src_zip_start, (int)(long)&_binary_arc-httpd_src_zip_size, 1, stream);
            fflush(stream);
            free(uri_hash);
            return;
        }
        free(uri_hash);
    }

	// separate path from args
	if ((query_string = strchr(uri, '?')))
	{
		*query_string = 0;
		query_string++;
		unescape_uri(uri, NULL);
	}
	else
		query_string = (char *)"";

	// make path relative
	path_len = snprintf(path, sizeof(path), ".%s", uri); 
	if (path_len < 0)
		_clienterror(404, "Not Found");


	// stat cgi binary
	if (stat(path, &sbuf) == -1)
		_clienterror(404, "Not Found");

	// check perms
	if(!(S_ISREG(sbuf.st_mode) && (S_IXUSR & sbuf.st_mode)))
		_clienterror(403, "Forbidden");

	setenv("QUERY_STRING", query_string, 1);

	fprintf(stream, "HTTP/1.1 200 OK\r\n");
	fprintf(stream, "Server: arc-httpd\r\n");
	fprintf(stream, "Content-Type: %s\r\n", mimetype);
	fprintf(stream, "Connection: close\r\n");
	fprintf(stream, "Accept-Ranges: bytes\r\n");
	fprintf(stream, "\r\n");
	fflush(stream);

	// exec cgi
	cgi_set_environment();
	//REM BUG: SHELL can be supplied as HTTP variable
	//if(cgi_exec(fileno(stream), getenv("SHELL"), "-c", path, NULL) < 0)
	//	_clientwarning(500, "Internal Server Error");

	return;
}


char *get_http_header(char *name)
{
	ENTRY e = {name, NULL};
	ENTRY* ep;
	if (!(hsearch_r(e, FIND, &ep, &global.headerTable)))
		_error("hsearch_r");
	return ep? ep->data : NULL;
}


void add_http_header(char *name, char *value)
{
	int res;
	ENTRY e, *ep;

	e.key = strdup(name);
	e.data = strdup(value);

	//REM BUG: hsearch_r only has 512 entries statically allocated so this can overflow
	if (!(hsearch_r(e, ENTER, &ep, &global.headerTable)))
		_error("hsearch_r");
}


int parse_http_headers(FILE *stream)
{
	char *buf = malloc(BUFSIZE);
	char *val = NULL;

	// parse HTTP headers
	while(1)
	{
		if(!fgets(buf, BUFSIZE, stream))
			break;
			
		//REM BUG: Multiple connections can cause system resource exhaustion via fork/open fd's
		if(!strcmp(buf, "\r\n") || !strcmp(buf, "\n"))
			break;

		// chomp off newlines
		if((val = strchr(buf, '\r')))
			*val = 0;
		if((val = strchr(buf, '\n')))
			*val = 0;

		if((val = strstr(buf, ": ")))
		{
			*val = 0;
			val += 2;

			add_http_header(buf, val);
			//REM
			//REM BUG: Memory corruption allowing House of Force
			if(!strcmp(buf, "Content-Length")) //REM
				buf = malloc(atoi(val));       //REM
		}
	}

	free(buf);
	return 1;
}


void abort_cleanup(void* data)
{
	FILE* stream = data;
	fclose(stream);
}


int parse_http_request(int c_fd)
{
	int res_fd; // for requested resource
	FILE *stream;
	char* method; char* version; char* uri;
	int  uri_len;
	int path_len;
	char path[FILENAME_MAX];
	const char* mimetype = global.default_mimetype;
	char* query_string;
	struct stat sbuf;
	unsigned char *filemap;

	// open connection as stream
	if ((stream = fdopen(c_fd, "r+")) == NULL)
		_error("fdopen");

	// initialize some global state
	strcpy(global.default_mimetype, "text/plain");
	global.cleanup_func = abort_cleanup;
	global.stream = stream;

	// HTTP request
	fgets(global.request, sizeof(global.request), stream);
	fprintf(stderr, "[arc-httpd] %s", global.request);

	// tokenize it
	if(!(method = strtok(global.request, " ")))
		_clienterror(400, "Bad Request");
	if(!(uri = strtok(NULL, " ")))
		_clienterror(400, "Bad Request");
	if(!(version = strtok(NULL, " ")))
		_clienterror(400, "Bad Request");
	if (strtok(NULL, " ") != NULL)
		_clienterror(400, "Bad Request");
		
	// make sure it's valid
	if(!strlen(method) || !strlen(uri) || !strlen(version))
		_clienterror(400, "Bad Request");

	uri_len = strlen(uri);

	// only handle GET or POST
	if (strcmp(method, "GET") != 0 && strcmp(method, "POST") != 0)
		_clienterror(501, "Not Implemented");

	// filter directory traversal
	//REM BUG: this does not filter hex encoded traversal "/%2E%2E/" 
	if(strstr(uri, "..") || strchr(uri, '%') || uri[0] == '.')
	{
		http_error(stream, "400 Bad Request");
		fprintf(stream, "bad kitty!\n");
		fclose(stream);
		return 400;
	}

	// initialize hash table for headers
	if (!hcreate_r(512, &global.headerTable))
		_clienterror(500, "Internal Server Error");

	if(!parse_http_headers(stream))
		_clienterror(400, "Bad Request");

	// parse URI
	if (strstr(uri, "/cgi-bin/") == uri) {
		do_cgi_request(stream, uri);

	} else if (strstr(uri, "/robots.txt") == uri) {
		fprintf(stream, "User-agent: *\r\n");
		fprintf(stream, "Disallow: /cgi-bin/\r\n");
		fprintf(stream, "Disallow: /\r\n");
		fflush(stream);

	} else {
		// make relative path and look for the file
		if(uri_len == 1 || uri_len == 0)
			path_len = snprintf(path, sizeof(path), "./index.html");
		else
			path_len = snprintf(path, sizeof(path), ".%s", uri);
		//REM BUG: uri longer than MAX_PATH will write NULL off end of buffer
#define MAX_PATH 4096
		if(path_len >= MAX_PATH) path_len = MAX_PATH - 1;
		path[path_len] = '\x00';	

		// remove query string from path
		if ((query_string = strchr(path, '?')))
			*query_string = 0;

		// stat file and make sure it's there
		if (stat(path, &sbuf) != 0)
			_clienterror(404, "Not Found");

		// figure out the mimetype
		if (!strcmp(uri + uri_len - 5, ".html"))
			mimetype = "text/html";
		else if (!strcmp(uri + uri_len - 4, ".htm")) 
			mimetype = "text/html";
		else if (!strcmp(uri + uri_len - 4, ".ico"))
			mimetype = "image/x-ico";
		else if (!strcmp(uri + uri_len - 4, ".bmp"))
			mimetype = "image/bmp";
		else if (!strcmp(uri + uri_len - 4, ".jpg"))
			mimetype = "image/jpg";
		else if (!strcmp(uri + uri_len - 4, ".gif"))
			mimetype = "image/gif";
		else if (!strcmp(uri + uri_len - 4, ".png"))
			mimetype = "image/png";
		else if (!strcmp(uri + uri_len - 4, ".zip"))
			mimetype = "application/zip";

		// serve static content
		fprintf(stream, "HTTP/1.1 200 OK\r\n");
		fprintf(stream, "Server: arc-httpd\r\n");
		fprintf(stream, "Content-Length: %d\r\n", (int)sbuf.st_size);
		fprintf(stream, "Content-Type: %s\r\n", mimetype);
		fprintf(stream, "\r\n");
		fflush(stream);

		// Use mmap to return arbitrary-sized response body
		res_fd = open(path, O_RDONLY);
		filemap = (unsigned char *)mmap(0, sbuf.st_size, PROT_READ, MAP_PRIVATE, res_fd, 0);
		fwrite(filemap, sbuf.st_size, 1, stream);
		fflush(stream);
		munmap(filemap, sbuf.st_size);
	}

	fclose(stream);
	hdestroy_r(&global.headerTable);
	return 0;
}


int main(int argc, char **argv)
{
	unsigned short port;
	int s_fd, c_fd;
	int optval;
	struct sockaddr_in s_addr, c_addr;
	socklen_t addrlen;
	struct stat st;
	pid_t pid;

	char *instructions = 
		"This server has a backdoor in it!\n"
		"Find the special request that lets you execute commands!\n"
		"The key for this challenge is part of the backdoor and\n"
		"will be printed on success\n";

	// instructions
	fprintf(stderr, "\n%s\n%s\n", banner, instructions);

	// check args
	if (argc != 2)
	{
		fprintf(stderr,
			"error: Please select a port for the web server\n"
			"\n"
			"usage: %s <port>\n",
			argv[0]);
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[1]);

	// make sure cgi-bin exists
	if (stat("./cgi-bin", &st) == -1) 
		mkdir("./cgi-bin", 0755);

	symlink("/usr/bin/env", "./cgi-bin/env");

	// open socket
	if((s_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		_error("socket");

	// close sock on exec
	if (fcntl(s_fd, F_SETFD, 1) == -1)
		_error("fcntl");

	// reuse socket
	optval = 1;
	if((setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int))) == -1)
		_error("setsockopt");

	// bind
	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_addr.sin_port = htons(port);
	if (bind(s_fd, (struct sockaddr *) &s_addr, sizeof(s_addr)) < 0)
		_error("bind");

	if (listen(s_fd, 20) < 0)
		_error("listen");

	fprintf(stderr, "[arc-httpd] listening on %s:%d\n", inet_ntoa(s_addr.sin_addr), port);

	// ignore child signals
	signal(SIGCHLD, SIG_IGN);

	// accept
	while (1)
	{
		// wait for a connection
		addrlen = sizeof(c_addr);
		c_fd = accept(s_fd, (struct sockaddr *) &c_addr, &addrlen);
		if (c_fd < 0)
			_error("accept");

		// log connection
		fprintf(stderr, "\n[arc-httpd] Connection accepted from %s:\n", inet_ntoa(c_addr.sin_addr));

		// fork to make remote exploits easier :)
		pid = fork();
		if(pid == -1)
		{
			close(c_fd);
			_error("fork");
		}
		else if(pid > 0)
		{
			close(c_fd);
			continue;
		}
		assert(pid == 0);

		parse_http_request(c_fd);
  		close(c_fd);
		break;
	}

	return 0;
}

#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS 16
#endif
 
void hexdump(void *mem, unsigned int len)
{
        unsigned int i, j;
        
        for(i = 0; i < len + ((len % HEXDUMP_COLS) ? (HEXDUMP_COLS - len % HEXDUMP_COLS) : 0); i++)
        {
                /* print offset */
                if(i % HEXDUMP_COLS == 0)
                {
                        printf("0x%06x: ", i);
                }
 
                /* print hex data */
                if(i < len)
                {
                        printf("%02x ", 0xFF & ((char*)mem)[i]);
                }
                else /* end of block, just aligning for ASCII dump */
                {
                        printf("   ");
                }
                
                /* print ASCII dump */
                if(i % HEXDUMP_COLS == (HEXDUMP_COLS - 1))
                {
                        for(j = i - (HEXDUMP_COLS - 1); j <= i; j++)
                        {
                                if(j >= len) /* end of block, not really printing */
                                {
                                        putchar(' ');
                                }
                                else if(isprint(((char*)mem)[j])) /* printable char */
                                {
                                        putchar(0xFF & ((char*)mem)[j]);        
                                }
                                else /* other char */
                                {
                                        putchar('.');
                                }
                        }
                        putchar('\n');
                }
        }
}
