#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <linux/serial.h>
#include <asm/ioctls.h>
#include <arpa/inet.h>
#include <string.h>

#define SERVER_PORT 55556
#define MAX_CLIENT_NUM 1024
#define MAX_BUF_LEN 1024


char vol_plus[] = {
	0x55, 0x2C, 0x01, 0x02, 0x10, 0x01, 0x00, 0x12, 0x6F, 0x72, 0x69, 0x5F, 0x76, 0x6F, 0x6C, 0x5F, 
	0x2B, 0x00, 0x33, 0x05, 0x00, 0x00, 0x00, 0x00, 0xAF, 0x79, 0x7C, 0x00, 0x54, 0xD2, 0x28, 0x00, 
	0x54, 0xD2, 0x28, 0x00, 0x00, 0x01, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x3F
};
char vol_minus[] = {
	0x55, 0x2C, 0x11, 0x02, 0x10, 0x01, 0x00, 0x13, 0x6F, 0x72, 0x69, 0x5F, 0x76, 0x6F, 0x6C, 0x5F, 
	0x2D, 0x00, 0x4A, 0x06, 0x69, 0x6F, 0x6E, 0x00, 0xAF, 0x79, 0xA3, 0x00, 0x54, 0xD2, 0x28, 0x00, 
	0x54, 0xD2, 0x28, 0x00, 0x00, 0x01, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0xF7
};
char power_on_off[] = {
	0x55, 0x2C, 0x02, 0x02, 0x10, 0x01, 0x00, 0x15, 0x6F, 0x72, 0x69, 0x5F, 0x70, 0x6F, 0x77, 0x65, 
	0x72, 0x00, 0x3D, 0x05, 0x69, 0x6F, 0x6E, 0x00, 0xAF, 0x79, 0xB3, 0x00, 0x54, 0xD2, 0x28, 0x00, 
	0x54, 0xD2, 0x28, 0x00, 0x00, 0x01, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x37
};

#define debug

#ifdef debug
    #define log(msg...) printf(msg)
#else
    #define log(msg...)
#endif

typedef struct _cfd_t{
    int cfd;
	struct sockaddr_in client_addr;
}cfd_t;

int create_tcp_server(void)
{
	int fd;
	int reuse_addr_flag = 1;
	struct sockaddr_in server_addr;

	memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY),
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
    {
		exit(-1);
    }

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, 
					&reuse_addr_flag, sizeof(reuse_addr_flag)) < 0)
	{
        close(fd);
		exit(-2);
	}

	if (bind(fd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
        close(fd);
		exit(-3);
	}

	if (listen(fd, 3) < 0)
	{
        close(fd);
		exit(-4);
	}

    log("create tcp server OK\n");

	return fd;
}

char is_local_ip_addr(struct sockaddr_in *client_addr)
{
	in_addr_t netaddr = inet_netof(client_addr->sin_addr);
	in_addr_t localaddr = inet_lnaof(client_addr->sin_addr);
	log("%d, %d\n", netaddr, localaddr);

	if ((netaddr == 127 && localaddr == 1) || netaddr == 10 || 
			netaddr == 0xC0A8 ||
			(netaddr >= 0xAC10 && netaddr < 0xAC20))
	{
		return 1;
	}	

	return 0;
}

int main()
{
	int i, j;
    int tcp_fd;
   	cfd_t cfd[MAX_CLIENT_NUM];
    int tmp_fd;
	struct sockaddr_in tmp_addr;
    socklen_t tmp_socklen;
	int max_fd;
	int ready_num;
	int read_len;
	char read_buf[MAX_BUF_LEN];
	fd_set read_set;

	//daemon(1, 1);

	tcp_fd = create_tcp_server();

	for(i = 0; i < MAX_CLIENT_NUM; i++)
	{
		cfd[i].cfd = -1;
	}

	while(1)
	{
		FD_ZERO(&read_set);

        FD_SET(tcp_fd, &read_set);
		max_fd = tcp_fd;

		for(i = 0; i < MAX_CLIENT_NUM; i++)
		{
			if (cfd[i].cfd >= 0)
			{
				FD_SET(cfd[i].cfd, &read_set);
				if (max_fd < cfd[i].cfd)
				{
					max_fd = cfd[i].cfd;
				}
			}
		}
		
		ready_num = select(max_fd + 1, &read_set, NULL, NULL, NULL);
		if (ready_num > 0)
		{
            if (FD_ISSET(tcp_fd, &read_set))
            {
                tmp_socklen = sizeof(tmp_addr);
                tmp_fd = accept(tcp_fd, (struct sockaddr*)&tmp_addr, &tmp_socklen);
                if (tmp_fd >= 0)
                {
					for(i = 0; i < MAX_CLIENT_NUM; i++)
					{
						if (cfd[i].cfd < 0)
						{
							log("new connect %s, %d\n", inet_ntoa(tmp_addr.sin_addr), tmp_addr.sin_port);
							cfd[i].cfd = tmp_fd;
							cfd[i].client_addr = tmp_addr;
							break;
						}
					}

					if (i >= MAX_CLIENT_NUM)
					{
						close(tmp_fd);
						log("too much connect, close incoming client: %s, %d\n", inet_ntoa(tmp_addr.sin_addr), tmp_addr.sin_port);
					}
                }
            }

			for(i = 0; i < MAX_CLIENT_NUM; i++)
			{
				if (cfd[i].cfd >= 0 && FD_ISSET(cfd[i].cfd, &read_set))
				{
                    read_len = read(cfd[i].cfd, read_buf, MAX_BUF_LEN);
                    if (read_len > 0)
                    {
						char * respon;
						read_buf[read_len] = 0;
						log("%s\n", read_buf);

						if (is_local_ip_addr(&cfd[i].client_addr))
						{
							if (strstr(read_buf, "turn_on") || strstr(read_buf, "turn_off"))
							{
								respon = power_on_off;
								log("%s\n", read_buf);
							}
							else if (strstr(read_buf, "volume_up"))
							{
								respon = vol_plus;
								log("%s\n", read_buf);

							}
							else if (strstr(read_buf, "volume_down"))
							{
								respon = vol_minus;
								log("%s\n", read_buf);
							}

							for(j = 0; j < MAX_CLIENT_NUM; j++)
							{
								if (cfd[j].cfd >= 0 && !is_local_ip_addr(&cfd[j].client_addr))
								{
									write(cfd[j].cfd, respon, sizeof(power_on_off));
								}
							}
						}
                    }
                    else
                    {
						log("client leave connect %s, %d\n", inet_ntoa(cfd[i].client_addr.sin_addr),
								cfd[i].client_addr.sin_port);
                        close(cfd[i].cfd);
                        cfd[i].cfd = -1;
                    }
				}
			}
		}
		else
		{
            log("select ret:%d\n", ready_num);
		}
	}
	
	return 0;
}

