#define _CRT_SECURE_NO_WARNINGS
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
// Директива линковщику: использовать библиотеку сокетов
#pragma comment(lib, "ws2_32.lib")
#else // LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#endif
#include <stdio.h>
#include<stdlib.h>
#include <string.h>
unsigned short int port;
#define queue 10
#define N 100
bool connectedSockets_put[N];
int counter = 0;
bool flag_for_stop = 0;
int howmany = 0;
struct socket_ip
{
	int s;
	unsigned int ip;
};
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
void get_msg(socket_ip* cs, int i,FILE* fp);
int main(int argc,char *argv[])
{
	init();
	port = (unsigned short int)atoi((const char*)argv[1]);
	int ls;//прослушивающий сокет
	ls = socket(AF_INET, SOCK_STREAM, 0);
	set_non_block_mode(ls);
	if (ls < 0)
		return sock_err("socket", ls);
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);			  // Сервер по умолчанию прослушивает порт 9000
	addr.sin_addr.s_addr = htonl(INADDR_ANY); // Все адреса

	if (bind(ls, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return sock_err("bind", ls);

	//Перевод сокета в состояние прослушивания
	if (listen(ls, queue) < 0)
		return sock_err("listen", ls);

	socket_ip cs[N]; // Сокеты с подключенными клиентами
	fd_set rfd;
	fd_set wfd;
	int nfds = ls;
	int i;
	struct timeval tv = { 2, 50000 };
	FILE* fp=0;
	fp=fopen("msg.txt", "w+");

	while (1)
	{
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		FD_SET(ls, &rfd);
		for (i = 0; i < counter; i++)
		{
			FD_SET(cs[i].s, &rfd);
			FD_SET(cs[i].s, &wfd);
			if (nfds < cs[i].s)
				nfds = cs[i].s;
		}
		if (select(nfds + 1, &rfd, &wfd, 0, &tv) > 0)
		{
			// Есть события
			if (FD_ISSET(ls, &rfd))
			{
				// Есть события на прослушивающем сокете, можно вызвать accept,принять
					// подключение и добавить сокет подключившегося клиента в массив cs
				int addrlen = sizeof(struct sockaddr_in);
				cs[counter].s = accept(ls, (struct sockaddr*)&addr, &addrlen);
				if (cs[counter].s < 0)
				{
					return sock_err("accept", ls);
				}
				set_non_block_mode(cs[counter].s);
				cs[counter].ip = ntohl(addr.sin_addr.s_addr);

				printf("Peer %d connected: %u.%u.%u.%u:%u\n", counter + 1, (cs[counter].ip >> 24) & 0xFF, (cs[counter].ip >> 16) & 0xFF,
					(cs[counter].ip >> 8) & 0xFF, (cs[counter].ip) & 0xFF, port);
				counter++;
			}
			for (i = 0; i < counter; i++)
			{
				if (FD_ISSET(cs[i].s, &rfd))
				{
					get_msg(&cs[i], i,fp);
					// Сокет cs[i] доступен для чтения. Функция recv вернет данные,recvfrom - дейтаграмму
				}
				if (FD_ISSET(cs[i].s, &wfd))
				{
					send(cs[i].s, "ok", 2, 0);
					if (flag_for_stop == 1)
						goto mekka;
					// Сокет cs[i] доступен для записи. Функция send и sendto будет успешно завершена
				}
			}
		}
		else
		{
			// Произошел таймаут или ошибка
		}
	}

mekka:
	fclose(fp);
	while (counter != 0)
	{
		closesocket(cs[counter].s);
		counter--;
	}
	closesocket(ls);
	return 0;
}




void get_msg(socket_ip *cs,int i,FILE* fp)
{
	char big_fat_mess[25000] = "\0";
	char get_put[3] = "\0";
	if (connectedSockets_put[i] != 1)
	{
		recv(cs[i].s, get_put, 3, 0);
		connectedSockets_put[i] = 1;
	}//getting put
	char get_num[4] = "\0";
	recv(cs[i].s, get_num, 4, 0);

	fprintf(fp,"%u.%u.%u.%u:%u", (cs[i].ip >> 24) & 0xFF, (cs[i].ip >> 16) & 0xFF,
		(cs[i].ip >> 8) & 0xFF, (cs[i].ip) & 0xFF, port);
	//gettin' days,monthes, years, times
	char day;
	char month;
	char year_buf[3] = "\0";
	short year;
	for (int i = 0; i < 2; i++)
	{
		 recv(cs[i].s, &day, 1, 0); // 
			if ((int)day < 10) fprintf(fp, "0");
			fprintf(fp, "%i.", (int)day);

		recv(cs[i].s, &month, 1, 0); // 
			if ((int)month < 10) fprintf(fp, "0");
			fprintf(fp, "%i.", (int)month);
		recv(cs[i].s, year_buf, 2, 0); // 
		
			memcpy(&year, year_buf, 2);
			fprintf(fp, "%d ", htons(year));
	}
	char dma;
	 recv(cs[i].s, &dma, 1, 0);
	if ((int)dma < 10) fprintf(fp, "0");
	fprintf(fp, "%i:", (int)dma);
	
	recv(cs[i].s, &dma, 1, 0);
	if ((int)dma < 10) fprintf(fp, "0");
	fprintf(fp, "%i:", (int)dma);

	recv(cs[i].s, &dma, 1, 0);
	if ((int)dma<10) fprintf(fp, "0");
	fprintf(fp, "%i ", (int)dma);

	int rec2 = 0;

	while(rec2 = recv(cs[i].s, big_fat_mess, 25000, 0)>0);
	
	fprintf(fp, "%s", big_fat_mess);
	howmany++;
	if (!strcmp(big_fat_mess, "stop"))
	{
		flag_for_stop = 1;
	}



}