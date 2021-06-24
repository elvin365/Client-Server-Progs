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
int numrecords[200];
short port;
int NumberMessages = -1;
int conectedSocets_put[50] = { 0 };
int flag_put = 0;
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

int recv_string(int cs, int i, FILE* f, struct sockaddr_in addr)
{
	
	Sleep(50);// один он был
	//Sleep(50);
	//Sleep(50);
	//Sleep(50);
	//Sleep(50);
	char message[350000];
	unsigned long getElem;
	int LetsCheck;
	char HHMMSS;
	char day;
	char month;
	char year_buf[2] = "\0";
	unsigned short year;
	//char ip[INET_ADDRSTRLEN];
	//inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
	unsigned int ip = ntohl(addr.sin_addr.s_addr);
	if (flag_put == 0)
	{
		LetsCheck = recv(cs, (char*)&getElem, 3, 0);
		flag_put = 1;
	}
	/*if (conectedSocets_put[i] != 1)
	{
		LetsCheck = recv(cs, (char*)&getElem, 3, 0);
		if (LetsCheck<0)
			return sock_err("receive2", cs);
		conectedSocets_put[i] = 1;
	}*/
	LetsCheck = recv(cs, (char*)&getElem, 4, 0);
	if (LetsCheck<0)
		return sock_err("receive2", cs);
	if (LetsCheck == 0)
		return 0;

	//fprintf(f, "%s:%i ", ip, port); //ip:port   ntohs(addr.sin_port)-получение порта.
	fprintf(f, "%d.%d.%d.%d:%i ", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip) & 0xFF, port);
	for (int i = 0; i < 2; i++)
	{
		LetsCheck= recv(cs, &day, 1, 0); // 
		if (LetsCheck < 0)
			return sock_err("day", cs);
		else
		{
			if ((int)day < 10) fprintf(f, "0");
			fprintf(f, "%i.", (int)day);
		}
		LetsCheck = recv(cs, &month, 1, 0); // 
		if (LetsCheck < 0)
			return sock_err("day", cs);
		else
		{
			if ((int)month < 10) fprintf(f, "0");
			fprintf(f, "%i.", (int)month);
		}
		LetsCheck = recv(cs, year_buf, 2, 0); // 
		if (LetsCheck < 0)
			return sock_err("year", cs);
		else
		{
			//if ((int)day < 10) fprintf(f, "0");
			memcpy(&year, year_buf, 2);
			fprintf(f, "%d ", htons(year));
		}
	}
	LetsCheck = recv(cs, &HHMMSS, 1, 0);//hh mm
	if ((int)HHMMSS < 10) fprintf(f, "0");
	fprintf(f, "%i:", (int)HHMMSS);

	LetsCheck = recv(cs, &HHMMSS, 1, 0);//hh mm
	if ((int)HHMMSS < 10) fprintf(f, "0");
	fprintf(f, "%i:", (int)HHMMSS);

	LetsCheck = recv(cs, &HHMMSS, 1, 0);//hh mm
	if ((int)HHMMSS<10) fprintf(f, "0");
	fprintf(f, "%i ", (int)HHMMSS);
	/*for (int i = 0; i < 6; i++)
	{
		LetsCheck = recv(cs, &HHMMSS, 1, 0);//hh mm
		if (i < 2) {
			if (LetsCheck < 0)
				return sock_err("HHMMSS", cs);
			else
			{
				if ((int)HHMMSS < 10) fprintf(f, "0");
				fprintf(f, "%i:", (int)HHMMSS);
			}
		}
		else if (i == 2) {
			if (LetsCheck < 0) //ss
				return sock_err("HHMMSS", cs);
			else
			{
				if ((int)HHMMSS < 10) fprintf(f, "0");
				fprintf(f, "%i ", (int)HHMMSS);
			}
		}
		else if (i > 2) {

			if (i < 5) {
				if (LetsCheck <= 0)
					return sock_err("HHMMSS", cs);
				else
				{
					if ((int)HHMMSS<10) fprintf(f, "0");
					fprintf(f, "%i:", (int)HHMMSS);
				}

			}
			else {

				if (LetsCheck <= 0)
					return sock_err("HHMMSS", cs);
				else
				{
					if ((int)HHMMSS<10) fprintf(f, "0");
					fprintf(f, "%i ", (int)HHMMSS);
				}
			}
		}
	}*/


	/*LetsCheck = recv(cs, (char*)&getElem, 4, 0);
	if (LetsCheck <= 0)
		return sock_err("NUM", cs);
	getElem = ntohl(getElem);
	fprintf(f, "%lu ", getElem);*/

	LetsCheck = recv(cs, message, 350000, 0);
	if (LetsCheck <= 0)
		return sock_err("MESSAGE", cs);
	/*if (isalpha(message[0]) || message[0]==':' || message[0]==)
	{
		int qty = -1;
		int j1;
		int i1;
		for (j1 = 0; j1<abs(qty); j1++) {
			if (qty < 0) {
				for (i1 = 0; i1<strlen(message) - 1; i1++)
					message[i1] = message[i1 + 1];
				message[strlen(message) - 1] = 0;
			}
			else {
				for (i1 = strlen(message) - 1; i1>0; i1--)
					message[i1] = message[i1 - 1];
				message[0] = '\0';
			}
			
		}
	}*/

	fprintf(f, "%s", message);
	if (strcmp(message, "stop") == 0 && message[4] == '\0')
		return 1;
	fprintf(f, "\n");
	numrecords[i]++;
	return 0;
}
int send_notice(int cs, int i, int numrecords[])
{
	while (numrecords[i] != 0)
	{
		int ret = send(cs, "ok", 2, 0);
		if (ret < 0)
			return sock_err("send", cs);
		if (ret == 0)
			return 0;
		numrecords[i]--;
	}
	return 0;
}
int doit(int s, int *conectedSocets) {
	FILE* file = fopen("msg.txt", "w+");
	int i;
	for (i = 0; i<200; i++)
		numrecords[i] = 0;
	struct sockaddr_in conectedSocets_addr[50];
	struct timeval timeVal = { 1, 0 };
	int nfds = s;
	struct sockaddr_in addr;
	// Инициалиазация сетевой библиотеки
	init();
	// Создание TCP-сокета
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return sock_err("socket", s);
	set_non_block_mode(s);
	// Заполнение адреса прослушивания
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port); // Сервер прослушивает порт 9000
	addr.sin_addr.s_addr = htonl(INADDR_ANY); // Все адреса
											  // Связывание сокета и адреса прослушивания
	if (bind(s, (struct sockaddr*) &addr, sizeof(addr)) < 0)
		return sock_err("bind", s);
	// Начало прослушивания
	if (listen(s, 10) < 0)
		return sock_err("listen", s);

	fd_set rfd;
	fd_set wfd;
	while (1)
	{
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		FD_SET(s, &rfd);
		nfds = s;
		for (i = 0; i < (NumberMessages + 1); i++)
		{
			FD_SET(conectedSocets[i], &rfd);
			FD_SET(conectedSocets[i], &wfd);
			if (nfds < conectedSocets[i])
				nfds = conectedSocets[i];
		}
		if (select(nfds + 1, &rfd, &wfd, 0, &timeVal) >= 0)
		{ // Есть события 

			if (FD_ISSET(s, &rfd))
			{ // Есть события на прослушивающем сокете, можно вызвать accept, принять 
			  // подключение и добавить сокет подключившегося клиента в массив cs 

				NumberMessages++;
				int addrlen = sizeof(struct sockaddr_in);
				conectedSocets[NumberMessages] = accept(s, (struct sockaddr*) &conectedSocets_addr[NumberMessages], &addrlen);
				if (conectedSocets[NumberMessages] < 0)
				{
					return sock_err("accept", s);
				}
				set_non_block_mode(conectedSocets[NumberMessages]);
				unsigned int buf = ntohl(conectedSocets_addr[NumberMessages].sin_addr.s_addr);
				//char buf[INET_ADDRSTRLEN];
				//inet_ntop(AF_INET, &conectedSocets_addr[NumberMessages].sin_addr, buf, INET_ADDRSTRLEN);

				fprintf(stdout, "%d.%d.%d.%d:%i connect to server\n", (buf>>24)& 0xFF,(buf>>16)& 0xFF, (buf >> 8) & 0xFF, (buf) & 0xFF, port);
			}
			for (int i = 0; i < (NumberMessages + 1); i++)
			{
				if (FD_ISSET(conectedSocets[i], &rfd))
				{ // Сокет cs[i] доступен для чтения. Функция recv вернет данные, recvfrom - дейтаграмму 
					//metka:
					int rcv = recv_string(conectedSocets[i], i, file, conectedSocets_addr[i]);
					//int ierr = WSAGetLastError();

				//	if (ierr == WSAEWOULDBLOCK) {  // currently no data available
				//		Sleep(50);  // wait and try again
				//		goto metka;
				//	}
					if (rcv == 1)
					{
						fprintf(stdout, "the connection is closed, the server will be disconnected\n");
						fclose(file);
						s_close(s);
						while (NumberMessages != 0) {
							s_close(conectedSocets[NumberMessages]);
							NumberMessages--;
						}
						return 0;
					}
				}
				if (FD_ISSET(conectedSocets[i], &wfd))
				{ // Сокет cs[i] доступен для записи. Функция send и sendto будет успешно завершена 
					if (send_notice(conectedSocets[i], i, numrecords) != 0)
						return sock_err("send", conectedSocets[i]);
				}
			}
		}
		else
		{ // Произошел таймаут или ошибка 
			return sock_err("select", s);
		}
	}


}
int main(int argc, char* argv[])
{

	int s=0;
	int conectedSocets[100];
	port = atoi(argv[1]);
	printf("listening %i\n", port);
	doit(s, conectedSocets);

	s_close(s);
	while (NumberMessages != 0) {
		s_close(conectedSocets[NumberMessages]);
		NumberMessages--;
	}
	deinit();
	return 0;


}