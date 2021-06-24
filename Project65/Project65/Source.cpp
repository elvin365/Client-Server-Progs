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
#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#define NCS 10 //max кол-во клиентских соединений
#define N 1000 // размер буффера для принемаемых сообщений

int nomer_message_of_client = 0;

int max_num = 0;
int counter = 0;
bool flag_put = 0;
bool flag_stop = 0;
short port = 0;
bool one_message = 0;
struct data_and_message
{
	int index;
	char day1;
	char month1;
	unsigned short year1;

	char day2;
	char month2;
	unsigned short year2;

	char hour;
	char min;
	char sec;

	char* message;
};
struct info_sock
{
	int socket;
	unsigned int ip;
	struct data_and_message soobchenie[N];
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

bool catch_put(struct info_sock* cs, int i)
{
	int rec = 0;
	char go_get_that_put[4] = "\0";
	rec = recv(cs[i].socket, go_get_that_put, 3, 0);
	if (rec > 0)
		return 1;
	else 0;
}
void give_number(struct info_sock* cs, int i)
{
	char num4[5] = "\0";
	int rec = 0;
	//while((rec=recv(cs[i].socket, num4, 4, 0))<0);
	rec=recv(cs[i].socket, num4, 4, 0);
	memcpy(&nomer_message_of_client, num4, 4);
	nomer_message_of_client = ntohl(atoi(num4));
	cs[i].soobchenie[nomer_message_of_client].index = ntohl(atoi(num4));
	
}
void day_mes_year(struct info_sock* cs, int i)
{
	int rec = 0;
	rec=recv(cs[i].socket, &(cs[i].soobchenie[nomer_message_of_client].day1), 1, 0);
	recv(cs[i].socket, &(cs[i].soobchenie[nomer_message_of_client].month1), 1, 0);
	char god[3] = "\0";
	recv(cs[i].socket, god, 2, 0);
	memcpy(&cs[i].soobchenie[nomer_message_of_client].year1, god, 2);
	cs[i].soobchenie[nomer_message_of_client].year1 = ntohs(cs[i].soobchenie[nomer_message_of_client].year1);

	recv(cs[i].socket, &(cs[i].soobchenie[nomer_message_of_client].day2), 1, 0);
	recv(cs[i].socket, &(cs[i].soobchenie[nomer_message_of_client].month2), 1, 0);
	memset(god, 0, 3);
	recv(cs[i].socket, god, 2, 0);
	memcpy(&cs[i].soobchenie[nomer_message_of_client].year2, god, 2);
	cs[i].soobchenie[nomer_message_of_client].year2 = ntohs(cs[i].soobchenie[nomer_message_of_client].year2);
}
void sh_time(struct info_sock* cs, int i)
{
	recv(cs[i].socket, &(cs[i].soobchenie[nomer_message_of_client].hour), 1, 0);
	recv(cs[i].socket, &(cs[i].soobchenie[nomer_message_of_client].min), 1, 0);
	recv(cs[i].socket, &(cs[i].soobchenie[nomer_message_of_client].sec), 1, 0);
}
void read_message(struct info_sock* cs, int i)
{
	cs[i].soobchenie->message= (char*)calloc(N, sizeof(char));
	int rec=0;
	while ((rec = recv(cs[i].socket, cs[i].soobchenie[nomer_message_of_client].message, N+1, 0)) >= 0);
	
	if (!strcmp(cs[i].soobchenie[nomer_message_of_client].message, "stop"))
		flag_stop = 1;
	return;
}
void write_in(struct info_sock *cs)
{
	int i = 0;
	FILE* fp = NULL;
	fp = fopen("msg.txt", "a+");
	while (i < counter)
	{
		fprintf(fp, "%d.%d.%d.%d:%i ", (cs[i].ip >> 24) & 0xFF, (cs[i].ip >> 16) & 0xFF, (cs[i].ip >> 8) & 0xFF, (cs[i].ip) & 0xFF, port);

		if (cs[i].soobchenie[nomer_message_of_client].day1<10)
		{
			fprintf(fp, "0");
		}
		fprintf(fp, "%d." ,(int)cs[i].soobchenie[nomer_message_of_client].day1);
		if (cs[i].soobchenie[nomer_message_of_client].month1 < 10)
		{
			fprintf(fp, "0");
		}
		fprintf(fp, "%d.", (int)cs[i].soobchenie[nomer_message_of_client].month1);
		fprintf(fp, "%u ", cs[i].soobchenie[nomer_message_of_client].year1);
		
		if (cs[i].soobchenie[nomer_message_of_client].day2<10)
		{
			fprintf(fp, "0");
		}
		fprintf(fp, "%d.", (int)cs[i].soobchenie[nomer_message_of_client].day2);
		if (cs[i].soobchenie[nomer_message_of_client].month2 < 10)
		{
			fprintf(fp, "0");
		}
		fprintf(fp, "%d.", (int)cs[i].soobchenie[nomer_message_of_client].month2);
		fprintf(fp, "%u ", cs[i].soobchenie[nomer_message_of_client].year2);



		if (cs[i].soobchenie[nomer_message_of_client].hour<10)
		{
			fprintf(fp, "0");
		}
		fprintf(fp, "%d:", (int)cs[i].soobchenie[nomer_message_of_client].hour);
		if (cs[i].soobchenie[nomer_message_of_client].min < 10)
		{
			fprintf(fp, "0");
		}
		fprintf(fp, "%d:", (int)cs[i].soobchenie[nomer_message_of_client].min);

		if (cs[i].soobchenie[nomer_message_of_client].sec<10)
		{
			fprintf(fp, "0");
		}
		fprintf(fp, "%d ", (int)cs[i].soobchenie[nomer_message_of_client].sec);
		

		fprintf(fp, "%s", cs[i].soobchenie[nomer_message_of_client].message);
		fprintf(fp, "\n");
		i++;
	}
	fclose(fp);
	one_message = 1;
}

int main(int argc,char* argv[])
{
	init();
	
	 port = (unsigned short int)atoi((const char*)argv[1]);
	int ls;	// Прослушивающий сокет
	ls = socket(AF_INET, SOCK_STREAM, 0);
	//int cs[N];
	//
	//Перевод сокета в неблокирующий режим
	set_non_block_mode(ls);
	if (ls < 0)
		return sock_err("socket", ls);
	struct info_sock cs[NCS];
	struct sockaddr_in addr;
	fd_set rfd;
	fd_set wfd;
	int nfds = ls;
	int i;
	struct timeval tv = { 2, 500000 };

	// Заполнение адреса прослушивания
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);			  // Сервер по умолчанию прослушивает порт 9000
	addr.sin_addr.s_addr = htonl(INADDR_ANY); // Все адреса

											  // Связывание сокета и адреса прослушивания
	if (bind(ls, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return sock_err("bind", ls);

	//Перевод сокета в состояние прослушивания
	if (listen(ls, NCS) < 0)
		return sock_err("listen", ls);

	while (1)
	{
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		FD_SET(ls, &rfd);
		for (i = 0; i < counter; i++)
		{
			FD_SET(cs[i].socket, &rfd);
			FD_SET(cs[i].socket, &wfd);
			if (nfds < cs[i].socket)
				nfds = cs[i].socket;
		}
		if (select(nfds + 1, &rfd, &wfd, 0, &tv) > 0)
		{
			// Есть события
			if (FD_ISSET(ls, &rfd))
			{
				// Есть события на прослушивающем сокете, можно вызвать accept,принять
					// подключение и добавить сокет подключившегося клиента в массив cs
				int addrlen = sizeof(struct sockaddr_in);
				cs[counter].socket = accept(ls, (struct sockaddr*)&addr, &addrlen);
				if (cs[counter].socket < 0)
				{
					return sock_err("accept", ls);
				}
				set_non_block_mode(cs[counter].socket);
				cs[counter].ip = ntohl(addr.sin_addr.s_addr);

				printf("Peer %d connected: %u.%u.%u.%u:%u\n",counter+1, (cs[counter].ip >> 24) & 0xFF, (cs[counter].ip >> 16) & 0xFF,
					(cs[counter].ip >> 8) & 0xFF, (cs[counter].ip) & 0xFF,port);
				counter++;

			}
			for (i = 0; i < counter; i++)
			{
				if (FD_ISSET(cs[i].socket, &rfd))
				{
					// Сокет cs[i] доступен для чтения. Функция recv вернет данные,recvfrom - дейтаграмму
					//Sleep(1200);

					do
					{
						
						if (flag_put == 0)
							flag_put = catch_put(cs, i);
						give_number(cs, i);
						day_mes_year(cs, i);
						sh_time(cs, i);
						read_message(cs, i);
						write_in(cs);
						Sleep(1200);
						
					} while (strcmp(cs[i].soobchenie[nomer_message_of_client].message, "stop"));
					//nomer_message_of_client = 0;
					//Sleep(1200);

				}
				if (FD_ISSET(cs[i].socket, &wfd))
				{
					// Сокет cs[i] доступен для записи. Функция send и sendto будетуспешно завершена
					if (i >0 && (i-1)!=0)
					{
						closesocket(cs[i-1].socket);
					}
				/*	int k = counter;
					
					if(flag_stop==0)
					send(cs[i].socket, "ok", 2, 0);

					if (flag_stop == 1)
					{
						fprintf(stdout, "'stop' arrived.Terminating");
						flag_stop = 0;
						//for (int jk = 0; jk < max_num; jk++)
						//{
							send(cs[i].socket, "ok", 2, 0);
							//closesocket(cs[jk].socket);
						//}
					}*/

					while (nomer_message_of_client != 0)
					{
						send(cs[i].socket, "ok", 2, 0);
						nomer_message_of_client--;
					}

				}
			}
			if(i==0)
				closesocket(cs[0].socket);
			if (i == 1)
				closesocket(cs[1].socket);
			
		}
		else
		{
			// Произошел таймаут или ошибка
			//break;
		}
	}
	

	deinit();

	return 0;
}