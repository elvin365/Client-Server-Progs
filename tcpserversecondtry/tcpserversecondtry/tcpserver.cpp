#define _CRT_SECURE_NO_WARNINGS
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else // LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netdb.h>
#include <errno.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<iostream>
using namespace std;
#define N 20
int i;
int k = 0;//count
int rcv;
int flag_for_put=0;
int flag_to_stop = 0;

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
void deinit()
{
#ifdef _WIN32
	// Для Windows следует вызвать WSACleanup в конце работы
	WSACleanup();
#else
	// Для других ОС действий не требуется
#endif
}
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

struct connected_cli
{
	unsigned int cs_ip;
	int myday1;
	int mymonth1;
	unsigned short year1;
	
	int myday2;
	int mymonth2;
	unsigned short year2;

	int hour;
	int min;
	int sec;

	char incoming_mess[50000];

};

int main(int argc,char* argv[])
{
	char buffer[2000] = "\0";

	if (argc != 2)
	{
		fprintf(stdout,"wrong");
		exit(1);
	}

	int port = atoi(argv[1]);

	FILE* fp = NULL;

	struct connected_cli  connected_sockets[N];// Сокеты с подключенными клиентами
	int ls; // Сокет, прослушивающий соединения
	int sockets[N]; // Сокеты с подключенными клиентами
	fd_set fds;
	fd_set wfd;
	struct timeval tv = { 1, 0 };
	struct sockaddr_in addr;


	// Инициалиазация сетевой библиотеки
	init();
	// Создание TCP-сокета
	ls = socket(AF_INET, SOCK_STREAM, 0);
	set_non_block_mode(ls);
	if (ls < 0)
		return sock_err("socket", ls);


	// Заполнение адреса прослушивания
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port); // Сервер прослушивает порт 9000
	addr.sin_addr.s_addr = htonl(INADDR_ANY); // Все адреса
											  // Связывание сокета и адреса прослушивания
	if (bind(ls, (struct sockaddr*) &addr, sizeof(addr)) < 0)
		return sock_err("bind", ls);
	// Начало прослушивания
	if (listen(ls, 10) < 0)
		return sock_err("listen",ls);
	int nfds = ls;

	while (1)
	{
		FD_ZERO(&fds);
		FD_ZERO(&wfd);
		FD_SET(ls, &fds);
		for (i = 0; i < k; i++)
		{
			FD_SET((sockets[i]), &fds);
			FD_SET(sockets[i], &wfd);
			if (nfds < sockets[i])
				nfds = sockets[i];
		}
		if (select(nfds + 1, &fds, &wfd, 0, &tv) > 0)
		{
			// Есть события
			if (FD_ISSET(ls, &fds))
			{
				// Есть события на прослушивающем сокете, можно вызвать accept,принять
				//подключение и добавить сокет подключившегося клиента в массив cs
				int addrlen = sizeof(addr);
				sockets[k]= accept(ls, (struct sockaddr*)&addr, &addrlen);
					if (sockets[k]< 0)
					{
						fprintf(stdout, "error1");
						break;
					}
					

					if (set_non_block_mode(sockets[k]) != 0)
						return sock_err("set_non_block_mode", sockets[k]);
					//FD_SET(sockets[k],& fds);
					connected_sockets[k].cs_ip = ntohl(addr.sin_addr.s_addr);


					/*cout << "Client connected: " << (connected_sockets[k].addr_of_this_socket.sin_addr.s_addr & 0xff) << "."
						<< (connected_sockets[k].addr_of_this_socket.sin_addr.s_addr >> 8 & 0xff) << "."
						<< (connected_sockets[k].addr_of_this_socket.sin_addr.s_addr >> 16 & 0xff) << "."
						<< (connected_sockets[k].addr_of_this_socket.sin_addr.s_addr >> 24 & 0xff) << ":"
						<< htons(connected_sockets[k].addr_of_this_socket.sin_port) << endl;*/

					printf("Peer connected: %u.%u.%u.%u \n",(connected_sockets[k].cs_ip>>24)&0xFF,(connected_sockets[k].cs_ip)>>16 & 0xFF,
						(connected_sockets[k].cs_ip >> 8) & 0xFF, (connected_sockets[k].cs_ip) & 0xFF);
					k++;
			}
			for (i = 0; i < k; i++)
			{
				mekka:
					if (FD_ISSET(sockets[i], &fds))
					{
						int temp = 0;
						// Сокет cs[i] доступен для чтения. Функция recv вернет данные,recvfrom - дейтаграмму // все обработки тут
						fp = fopen("msg.txt", "w+");
						char buf_for_put[4] = "\0";
						memset(buf_for_put, 0, sizeof(char) * 4);
						char tocopy_put[2] = "\0";
						char tocopy_year[3] = "\0";
						unsigned short year;

						int qwert = 0;

						if (flag_for_put != 1)
						{
							rcv = recv(sockets[i], buf_for_put, 3, 0);// ловим put
							flag_for_put = 1;
						}
						rcv = 0;
						char nomer_msg[4] = "\0";

						//номер сообщения
						unsigned int num_msg;
						int len_msg = recv(sockets[i], nomer_msg, 4, 0);
						memcpy(&num_msg, nomer_msg, 4);
						num_msg = ntohl(num_msg);



						printf("%s\n", buf_for_put);
						printf("nomer %s\n", nomer_msg);


						char myday[2] = "\0";
						rcv = recv(sockets[i], myday, sizeof(char), 0);
						connected_sockets[i].myday1 = myday[0];
						// printf("%d:")
						printf("myday %d\n", myday);

						char mymonth[2] = "\0";
						rcv = recv(sockets[i], mymonth, sizeof(char), 0);
						connected_sockets[i].mymonth1 = (int)(mymonth[0]);

						char god[3] = "\0";
						rcv = recv(sockets[i], god, 2, 0);

						memcpy(&connected_sockets[i].year1, god, 2);
						connected_sockets[i].year1 = ntohs(connected_sockets[i].year1);
						///////////////////////////////////////////////////////////////////////принятие вторых дат

						rcv = recv(sockets[i], myday, sizeof(char), 0);
						connected_sockets[i].myday2 = int(myday[0]);

						rcv = recv(sockets[i], mymonth, sizeof(char), 0);
						connected_sockets[i].mymonth2 = int(mymonth[0]);

						rcv = recv(sockets[i], god, 2, 0);

						memset(god, 0, 3);
						memcpy(&connected_sockets[i].year2, god, 2);
						connected_sockets[i].year2 = ntohs(connected_sockets[i].year2);

						////////////////////////////////////////////////  TIME
						char hour[2] = "\0";
						rcv = recv(sockets[i], hour, sizeof(char), 0);
						connected_sockets[i].hour = int(hour[0]);

						char min[2] = "\0";
						rcv = recv(sockets[i], min, sizeof(char), 0);
						connected_sockets[i].min = int(min[0]);

						char sec[2] = "\0";
						rcv = recv(sockets[i], sec, sizeof(char), 0);
						connected_sockets[i].sec = int(sec[0]);

						while ((rcv = recv(sockets[i], connected_sockets[i].incoming_mess, sizeof(connected_sockets[i].incoming_mess), 0)) < 0)
						{

						}


						if (!strcmp(connected_sockets[i].incoming_mess, "stop"))
						{
							flag_to_stop = 1;
						}
						//FD_CLR(sockets[i], &fds);
						
						//goto metka;


					}
					if (FD_ISSET(sockets[i], &wfd))
					{
						// Сокет cs[i] доступен для записи. Функция send и sendto будет успешно завершена 
						//отослать ок
						//metka:
						//send(sockets[i], "ok", 2, 0);
						/*printf("Peer disconnected: %u.%u.%u.%u \n", (connected_sockets[i].cs_ip >> 24) & 0xFF, (connected_sockets[i].cs_ip) >> 16 & 0xFF,
							(connected_sockets[i].cs_ip >> 8) & 0xFF, (connected_sockets[i].cs_ip) & 0xFF);
						FD_CLR(sockets[i], &fds);
						closesocket(sockets[i]);*/
						send(sockets[i], "ok", 2, 0);
						for (int j = 0; j < abs(-1); j++)
						{
							if (-1 < 0) {
								for (i = 0; i < strlen(connected_sockets[i].incoming_mess) - 1; i++)
									connected_sockets[i].incoming_mess[i] = connected_sockets[i].incoming_mess[i + 1];
								connected_sockets[i].incoming_mess[strlen(connected_sockets[i].incoming_mess)- 1] = 0;
							}
							else {
								for (i = strlen(connected_sockets[i].incoming_mess) - 1; i > 0; i--)
									connected_sockets[i].incoming_mess[i] = connected_sockets[i].incoming_mess[i - 1];
								connected_sockets[i].incoming_mess[0] = '\0';
							}
						}

						if (!strcmp(connected_sockets[i].incoming_mess, "stop"))
						{
							FD_CLR(sockets[i], &fds);
							closesocket(sockets[i]);
						}
						else
							i++;
						goto mekka;

					}
				
			}
		}
		else
		{
			// Произошел таймаут или ошибка
		}
	}




	closesocket(ls);

	return 0;




}