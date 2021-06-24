#define _CRT_SECURE_NO_WARNINGS
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
// Директива линковщику: использовать библиотеку сокетов
#pragma comment(lib, "ws2_32.lib")
#else // LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#endif
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else // LINUX
#include <fcntl.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define N 10
struct clients
{
	int s;
	unsigned int ip;
};
int set_non_block_mode(int s)
{
#ifdef _WIN32
	unsigned long mode = 1;
	return ioctlsocket(s, FIONBIO, &mode);
#else
	int fl = fcntl(s, F_GETFL, 0);
	return fcntl(s, F_SETFL, fl | O_NONBLOCK);
#endif
}
int counter = 0;
int connectedSocketsPut[50] = { 0 };
short port;
FILE* fp;
bool stop;
int init()
{
#ifdef _WIN32
	// Для Windows следует вызвать WSAStartup перед началом использования сокетов
	WSADATA wsa_data;
	return (0 == WSAStartup(MAKEWORD(2, 2), &wsa_data));
#else
	return 1; // Для других ОС действий не требуется
#endif
}
void deinit()
{
#ifdef _WIN32
	// Для Windows следует вызвать WSACleanup в конце работы
	WSACleanup();
#else
	// Для других ОС действий не требуется
#endif
}
int sock_err(const char* function, int s)
{
	int err;
#ifdef _WIN32
	err = WSAGetLastError();
#else
	err = errno;
#endif
	fprintf(stderr, "%s: socket error: %d\n", function, err);
	return -1;
}
void s_close(int s)
{
#ifdef _WIN32
	closesocket(s);
#else
	close(s);
#endif
}
bool is_check = true;

void resv_msg(int, struct clients*);
int main(int argc,char* argv[])
{
	init();
	 port = atoi(argv[1]);
	 fp = fopen("msg.txt", "w+");
	struct sockaddr_in addr;
	int ls;
	ls = socket(AF_INET, SOCK_STREAM, 0);
	if (ls < 0)
		return sock_err("soket", ls);
	set_non_block_mode(ls);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(ls, (struct sockaddr*) &addr, sizeof(addr)) < 0)
		return sock_err("bind", ls);
	if (listen(ls, 100) < 0)
		return sock_err("listen", ls);

	struct clients cs[N];
	fd_set rfd;
	int nfds = ls;
	int i;
	struct timeval tv = { 1, 0 };
	while (1)
	{
		FD_ZERO(&rfd);
		FD_SET(ls, &rfd);
		for (i = 0; i < counter; i++)
		{
			FD_SET(cs[i].s, &rfd);
			if (nfds < cs[i].s)
				nfds = cs[i].s;
		}
		if (select(nfds + 1, &rfd, 0, 0, &tv) > 0)
		{
			// Есть события
			if (FD_ISSET(ls, &rfd))
			{
				// Есть события на прослушивающем сокете, можно вызвать accept,принять
					// подключение и добавить сокет подключившегося клиента в массив cs

				
				int addrlen = sizeof(struct sockaddr_in);
				cs[counter].s = accept(ls, (struct sockaddr*) &addr, &addrlen);
				if (cs[counter].s < 0)
				{
					return sock_err("accept", ls);
				}
				set_non_block_mode(cs[counter].s);
				cs[counter].ip = ntohl(addr.sin_addr.s_addr);


				fprintf(stdout, "Peer connected : %d.%d.%d.%d:%i\n", (cs[counter].ip >> 24) & 0xFF, (cs[counter].ip >> 16) & 0xFF, (cs[counter].ip >> 8) & 0xFF, (cs[counter].ip) & 0xFF, port);
				counter++;

			}
			for (i = 0; i < counter; i++)
			{
				if (FD_ISSET(cs[i].s, &rfd))
				{
					// Сокет cs[i] доступен для чтения. Функция recv вернет данные,recvfrom - дейтаграмму
					resv_msg(i,cs);
					if (stop)
					{
						printf("'stop' arrived.");
						fclose(fp);
						for (int i = 0; i < counter; i++)
						{
							closesocket(cs[i].s);
							goto metka;
						}
					}
				}
				
			}
		}
		else
		{
			// Произошел таймаут или ошибка
		}
	}

metka:
	closesocket(ls);
	return 0;

}




void resv_msg(int num, struct clients *cs)
{
	Sleep(50);
	char get_put[4] = "\0";
	int rec = 0;
	if (connectedSocketsPut[num] != 1)
	{
		rec=recv(cs[num].s, get_put, 3, 0);
		connectedSocketsPut[num] = 1;
	}
	/*char nmsg[4] = "\0";
	recv(cs[num].s, nmsg, 3, 0);*/

	char get_num[5] = "\0";
	recv(cs[num].s, get_num, 4, 0);
	fprintf(fp, "%u.%u.%u.%u:%u ", (cs[num].ip >> 24) & 0xFF, (cs[num].ip >> 16) & 0xFF, (cs[num].ip >> 8) & 0xFF, (cs[num].ip) & 0xFF, port);

	char day;
	char month;
	unsigned short year;
	char year_buf[2] = "\0";
	for (int i = 0; i < 2; i++)
	{
		recv(cs[num].s, &day, 1, 0); // 
		if ((int)day < 10) fprintf(fp, "0");
		fprintf(fp, "%i.", (int)day);

		recv(cs[num].s, &month, 1, 0); // 
		if ((int)month < 10) fprintf(fp, "0");
		fprintf(fp, "%i.", (int)month);

		recv(cs[num].s, year_buf, 2, 0); // 
		memcpy(&year, year_buf, 2);
		fprintf(fp, "%u ", htons(year));
	}
	char dma;
	recv(cs[num].s, &dma, 1, 0);
	if ((int)dma < 10) fprintf(fp, "0");
	fprintf(fp, "%i:", (int)dma);

	recv(cs[num].s, &dma, 1, 0);
	if ((int)dma < 10) fprintf(fp, "0");
	fprintf(fp, "%i:", (int)dma);

	recv(cs[num].s, &dma, 1, 0);
	if ((int)dma<10) fprintf(fp, "0");
	fprintf(fp, "%i ", (int)dma);


	char message[100000] = "\0";
	memset(message, 0, 100000);
	while ((rec = recv(cs[num].s, message, 100000, 0)) > 0);
	//fputs(message, fp);
	fprintf(fp, "%s", message);
	if (strcmp(message, "stop"))
	{
		fputs("\n", fp);
	}
	//recv(cs[num].s, message, 35000, 0);
	send(cs[num].s, "ok", 2, 0);
	if (!strcmp(message, "stop"))
	{
		stop = 1;
	}
	


}

