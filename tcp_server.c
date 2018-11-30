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
#include <time.h>
#include <errno.h>

#define COMMAND_PORT 55557
#define DEVICE_PORT 55556
#define MAX_CLIENT_NUM 102
#define MAX_BUF_LEN 2048


/*char vol_plus[] = {
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
  };*/

char TV_vol_plus[] = {
	0x55, 0x34, 0x07, 0x02, 0x10, 0x01, 0x00, 0x12, 0x6F, 0x72, 0x69, 0x5F, 0x76, 0x6F, 0x6C, 0x5F,
	0x2B, 0x00, 0xE9, 0x26, 0xA8, 0xCE, 0x28, 0x00, 0x24, 0x05, 0xD0, 0x26, 0x00, 0x00, 0x00, 0x00,
	0x2C, 0xCE, 0x28, 0x00, 0xE8, 0xA8, 0x4A, 0x23, 0x45, 0x9D, 0x68, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x64, 0x00, 0x00, 0x00, 0x2D
};

char TV_vol_minus[] = {
	0x55, 0x34, 0x08, 0x02, 0x10, 0x01, 0x00, 0x13, 0x6F, 0x72, 0x69, 0x5F, 0x76, 0x6F, 0x6C, 0x5F,
	0x2D, 0x00, 0xE9, 0x26, 0xA8, 0xCE, 0x28, 0x00, 0x24, 0x05, 0xD0, 0x26, 0x00, 0x00, 0x00, 0x00,
	0x2C, 0xCE, 0x28, 0x00, 0xE8, 0xA8, 0x4A, 0x23, 0x45, 0x9D, 0x68, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x64, 0x00, 0x00, 0x00, 0x2E
};

char TV_power_on_off[] = {
	0x55, 0x34, 0x06, 0x02, 0x10, 0x01, 0x00, 0x15, 0x6F, 0x72, 0x69, 0x5F, 0x70, 0x6F, 0x77, 0x65,
	0x72, 0x00, 0xE9, 0x26, 0xA8, 0xCE, 0x28, 0x00, 0x24, 0x05, 0xD0, 0x26, 0x00, 0x00, 0x00, 0x00,
	0x2C, 0xCE, 0x28, 0x00, 0xE8, 0xA8, 0x4A, 0x23, 0x45, 0x9D, 0x68, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x64, 0x00, 0x00, 0x00, 0xE5
};

char fan_mute[] = {
	0x55, 0x34, 0x01, 0x02, 0x80, 0x02, 0x82, 0xA8, 0x2A, 0x20, 0x80, 0x8A, 0x00, 0x00, 0xAA, 0xAA,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x01, 0x64, 0x5A, 0x2D, 0x06, 0x10, 0xFF,
	0x4D, 0x75, 0x74, 0x65, 0x00, 0x00, 0x00, 0x00, 0x07, 0x12, 0x2A, 0x2E, 0x05, 0x01, 0x00, 0x00,
	0x64, 0x00, 0x00, 0x00, 0x70
};

char fan_vol_plus[] = {
	0x55, 0x34, 0x02, 0x02, 0x80, 0x02, 0x82, 0xA8, 0x80, 0x20, 0x2A, 0x8A, 0x00, 0x00, 0xAA, 0xAA,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x01, 0x64, 0x5A, 0x2D, 0x06, 0x11, 0xFF,
	0x56, 0x6F, 0x6C, 0x2B, 0x00, 0x00, 0x00, 0x00, 0x27, 0x12, 0x2A, 0x2E, 0x05, 0x01, 0x00, 0x00,
	0x64, 0x00, 0x00, 0x00, 0xAE
};

char fan_vol_minux[] = {
	0x55, 0x34, 0x03, 0x02, 0x80, 0x02, 0x82, 0xA8, 0x82, 0x20, 0x28, 0x8A, 0x00, 0x00, 0xAA, 0xAA,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x01, 0x64, 0x5A, 0x2C, 0x06, 0x11, 0xFF,
	0x56, 0x6F, 0x6C, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x23, 0x04, 0x2A, 0x2E, 0x05, 0x01, 0x00, 0x00,
	0x64, 0x00, 0x00, 0x00, 0x44
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
	time_t last_heart_beat_time;
}cfd_t;

int create_tcp_server(unsigned short port)
{
	int fd;
	int reuse_addr_flag = 1;
	struct sockaddr_in server_addr;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
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

int main()
{
	int i, j;
	int command_fd;
	int device_fd;
	cfd_t command_cfd[MAX_CLIENT_NUM];
	cfd_t device_cfd[MAX_CLIENT_NUM];
	int tmp_fd;
	struct sockaddr_in tmp_addr;
	socklen_t tmp_socklen;
	int max_fd;
	int ready_num;
	int read_len;
	char read_buf[MAX_BUF_LEN];
	fd_set read_set;
	struct timeval select_timeout;

	daemon(1, 1);

	command_fd = create_tcp_server(COMMAND_PORT);
	device_fd = create_tcp_server(DEVICE_PORT);

	for(i = 0; i < MAX_CLIENT_NUM; i++)
	{
		command_cfd[i].cfd = -1;
		device_cfd[i].cfd = -1;
	}

	while(1)
	{
		select_timeout.tv_sec = 60;
		select_timeout.tv_usec = 0;

		FD_ZERO(&read_set);

		FD_SET(command_fd, &read_set);
		FD_SET(device_fd, &read_set);
		max_fd = command_fd > device_fd ? command_fd : device_fd;

		for(i = 0; i < MAX_CLIENT_NUM; i++)
		{
			if (command_cfd[i].cfd >= 0)
			{
				FD_SET(command_cfd[i].cfd, &read_set);
				if (max_fd < command_cfd[i].cfd)
				{
					max_fd = command_cfd[i].cfd;
				}
			}

			if (device_cfd[i].cfd >= 0)
			{
				FD_SET(device_cfd[i].cfd, &read_set);
				if (max_fd < device_cfd[i].cfd)
				{
					max_fd = device_cfd[i].cfd;
				}
			}
		}

		ready_num = select(max_fd + 1, &read_set, NULL, NULL, &select_timeout);
		if (ready_num > 0)
		{
			if (FD_ISSET(command_fd, &read_set))
			{
				tmp_socklen = sizeof(tmp_addr);
				tmp_fd = accept(command_fd, (struct sockaddr*)&tmp_addr, &tmp_socklen);
				if (tmp_fd >= 0)
				{
					for(i = 0; i < MAX_CLIENT_NUM; i++)
					{
						if (command_cfd[i].cfd < 0)
						{
							log("new connect %s, %d\n", inet_ntoa(tmp_addr.sin_addr), tmp_addr.sin_port);
							command_cfd[i].cfd = tmp_fd;
							command_cfd[i].client_addr = tmp_addr;
							command_cfd[i].last_heart_beat_time = time(NULL);
							break;
						}
					}

					if (i >= MAX_CLIENT_NUM)
					{
						close(tmp_fd);
						log("too much command connect, close incoming client: %s, %d\n", inet_ntoa(tmp_addr.sin_addr), tmp_addr.sin_port);
					}
				}
			}

			if (FD_ISSET(device_fd, &read_set))
			{
				tmp_socklen = sizeof(tmp_addr);
				tmp_fd = accept(device_fd, (struct sockaddr*)&tmp_addr, &tmp_socklen);
				if (tmp_fd >= 0)
				{
					for(i = 0; i < MAX_CLIENT_NUM; i++)
					{
						if (device_cfd[i].cfd < 0)
						{
							log("new connect %s, %d\n", inet_ntoa(tmp_addr.sin_addr), tmp_addr.sin_port);
							device_cfd[i].cfd = tmp_fd;
							device_cfd[i].client_addr = tmp_addr;
							device_cfd[i].last_heart_beat_time = time(NULL);
							break;
						}
					}

					if (i >= MAX_CLIENT_NUM)
					{
						close(tmp_fd);
						log("too much devices connect, close incoming client: %s, %d\n", inet_ntoa(tmp_addr.sin_addr), tmp_addr.sin_port);
					}
				}
			}

			for(i = 0; i < ready_num; i++)
			{
				if (command_cfd[i].cfd >= 0 && FD_ISSET(command_cfd[i].cfd, &read_set))
				{
					read_len = read(command_cfd[i].cfd, read_buf, MAX_BUF_LEN);
					if (read_len > 0)
					{
						char * respon;
						read_buf[read_len] = 0;
						//log("%s\n", read_buf);

						command_cfd[i].last_heart_beat_time = time(NULL);

						if (strstr(read_buf, "TurnOn") || strstr(read_buf, "TurnOff"))
						{
							if (strstr(read_buf, "tv"))
							{
								respon = TV_power_on_off;
							}
							else if (strstr(read_buf, "fan"))
							{
								respon = fan_mute;
							}
							else if (strstr(read_buf, "light"))
							{
								respon = NULL;	
							}

							log("%s\n", read_buf);
						}
						else if (strstr(read_buf, "AdjustUpVolume"))
						{
							if (strstr(read_buf, "tv"))
							{
								respon = TV_vol_plus;
							}
							else if (strstr(read_buf, "fan"))
							{
								respon = fan_vol_plus;
							}

							log("%s\n", read_buf);
						}
						else if (strstr(read_buf, "AdjustDownVolume"))
						{
							if (strstr(read_buf, "tv"))
							{
								respon = TV_vol_minus;
							}
							else if (strstr(read_buf, "fan"))
							{
								respon = fan_vol_minux;
							}

							log("%s\n", read_buf);
						}

						for(j = 0; j < MAX_CLIENT_NUM; j++)
						{
							if (device_cfd[j].cfd >= 0)
							{
								if (respon != NULL)
								{
									write(device_cfd[j].cfd, respon, sizeof(TV_power_on_off));
								}
								else
								{
									log("%s\n", read_buf);
									write(device_cfd[j].cfd, read_buf, strlen(read_buf));
								}
							}
						}
					}
					else
					{
						log("command client leave : %s, %d\n", inet_ntoa(command_cfd[i].client_addr.sin_addr),
								command_cfd[i].client_addr.sin_port);
						close(command_cfd[i].cfd);
						command_cfd[i].cfd = -1;
					}
				}
			}

			for(i = 0; i < ready_num; i++)
			{
				if (device_cfd[i].cfd >= 0 && FD_ISSET(device_cfd[i].cfd, &read_set))
				{
					read_len = read(device_cfd[i].cfd, read_buf, MAX_BUF_LEN);
					if (read_len > 0)
					{
						read_buf[read_len] = 0;
						log("device: %s\n", read_buf);

						device_cfd[i].last_heart_beat_time = time(NULL);
					}
					else
					{
						log("device client leave : %s, %d\n", inet_ntoa(device_cfd[i].client_addr.sin_addr),
								device_cfd[i].client_addr.sin_port);
						close(device_cfd[i].cfd);
						device_cfd[i].cfd = -1;
					}
				}
			}
		}
		else if (ready_num == 0)
		{
			for(i = 0; i < MAX_CLIENT_NUM; i++)
			{
				if (command_cfd[i].cfd > 0 && time(NULL) - command_cfd[i].last_heart_beat_time > 180)
				{
					log("command client time out, close : %s, %d\n", inet_ntoa(command_cfd[i].client_addr.sin_addr),
							command_cfd[i].client_addr.sin_port);
					close(command_cfd[i].cfd);
					command_cfd[i].cfd = -1;
				}

				if (device_cfd[i].cfd > 0 && time(NULL) - device_cfd[i].last_heart_beat_time > 180)
				{
					log("device client time out, close : %s, %d\n", inet_ntoa(device_cfd[i].client_addr.sin_addr),
							device_cfd[i].client_addr.sin_port);
					close(device_cfd[i].cfd);
					device_cfd[i].cfd = -1;
				}
			}
		}
		else
		{
			log("select ret:%d, %s\n", ready_num, strerror(errno));
		}
	}

	return 0;
}

