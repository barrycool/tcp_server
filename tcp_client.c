#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/msg.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 55556
#define MAX_BUF_LEN 1024

//#define debug

#ifdef debug
    #define log(msg...) printf(msg)
#else
    #define log(msg...)
#endif

int main(int argc, char **argv)
{
	int fd;
	int ret;
    struct sockaddr_in server_addr;

	if (argc !=2)
	{
		log("Usage: %s msg\n", argv[0]);
		return 1;
	}

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		return fd;

    log("create tcp client OK\n");

	memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    ret = connect(fd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0)
    {
		log("create tcp client OK\n");
		return 1;
	}

	write(fd, argv[1], strlen(argv[1]));

	close(fd);

	return 0;
}


