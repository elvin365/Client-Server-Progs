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
int ecrec[400];
short port;
int TheAmountOfMess = -1;
int JustGetThePut[50] = { 0 };
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
char* itoa(int num, char* str, int base)
{
	int i = 0;
	bool isNegative = false;

	// If number is negative, append '-' 
	if (isNegative)
		str[i++] = '-';

	str[i] = '\0'; // Append string terminator 

				   // Reverse the string 

	return str;
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

	Sleep(1200);// один он был
			  //Sleep(50);
			  //Sleep(50);
			  //Sleep(50);
			  //Sleep(50);
	char message[350000];
	memset(message, 0, 350000);
	unsigned long go_get;
	int list;
	char dma;
	char day;
	char month;
	char year_buf[2] = "\0";
	unsigned short year;
	unsigned int ip = ntohl(addr.sin_addr.s_addr);
	
	if (flag_put == 0)
	{
		
		list = recv(cs, (char*)&go_get, 3, 0);
		flag_put = 1;
	}
	list = recv(cs, (char*)&go_get, 4, 0);
	if (list<0)
		return sock_err("receive2", cs);
	if (list == 0)
		return 0;

	fprintf(f, "%d.%d.%d.%d:%i ", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip) & 0xFF, port);
	for (int i = 0; i < 2; i++)
	{
		list = recv(cs, &day, 1, 0); // 
		if (list < 0)
			return sock_err("day", cs);
		else
		{
			if ((int)day < 10) fprintf(f, "0");
			fprintf(f, "%i.", (int)day);
		}
		list = recv(cs, &month, 1, 0); // 
		if (list < 0)
			return sock_err("day", cs);
		else
		{
			if ((int)month < 10) fprintf(f, "0");
			fprintf(f, "%i.", (int)month);
		}
		list = recv(cs, year_buf, 2, 0); // 
		if (list < 0)
			return sock_err("year", cs);
		else
		{
			memcpy(&year, year_buf, 2);
			fprintf(f, "%d ", htons(year));
		}
	}
	list = recv(cs, &dma, 1, 0);
	if ((int)dma < 10) fprintf(f, "0");
	fprintf(f, "%i:", (int)dma);
	list = recv(cs, &dma, 1, 0);
	if ((int)dma < 10) fprintf(f, "0");
	fprintf(f, "%i:", (int)dma);
	list = recv(cs, &dma, 1, 0);
	if ((int)dma<10) fprintf(f, "0");
	fprintf(f, "%i ", (int)dma);
	list = recv(cs, message, 350000, 0);
	
	if (list <= 0)
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
	ecrec[i]++;
	return 0;
}
int send_notice(int cs, int i, int ecrec[])
{
	while (ecrec[i] != 0)
	{
		int ret = send(cs, "ok", 2, 0);
		if (ret < 0)
			return sock_err("send", cs);
		if (ret == 0)
			return 0;
		ecrec[i]--;
	}
	return 0;
}
int HandleAlready(int s, int *Me_Sock_Con) {
	FILE* file = fopen("msg.txt", "w+");
	int i;
	for (i = 0; i<200; i++)
		ecrec[i] = 0;
	struct sockaddr_in Me_Sock_Con_addr[50];
	struct timeval value_of_time = { 2, 500000 };
	int nfds = s;
	struct sockaddr_in addr;
	init();
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return sock_err("socket", s);
	set_non_block_mode(s);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port); 
	addr.sin_addr.s_addr = htonl(INADDR_ANY); 
											  
	if (bind(s, (struct sockaddr*) &addr, sizeof(addr)) < 0)
		return sock_err("bind", s);
	if (listen(s, 10) < 0)
		return sock_err("listen", s);

	fd_set rfd;
	fd_set wfd;
	unsigned int buf;
	while (1)
	{
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		FD_SET(s, &rfd);
		nfds = s;
		for (i = 0; i < (TheAmountOfMess + 1); i++)
		{
			FD_SET(Me_Sock_Con[i], &rfd);
			FD_SET(Me_Sock_Con[i], &wfd);
			if (nfds < Me_Sock_Con[i])
				nfds = Me_Sock_Con[i];
		}
		if (select(nfds + 1, &rfd, &wfd, 0, &value_of_time) >= 0)
		{ // Есть события 

			if (FD_ISSET(s, &rfd))
			{ // Есть события на прослушивающем сокете, можно вызвать accept, принять 
			  // подключение и добавить сокет подключившегося клиента в массив cs 

				TheAmountOfMess++;
				int addrlen = sizeof(struct sockaddr_in);
				Me_Sock_Con[TheAmountOfMess] = accept(s, (struct sockaddr*) &Me_Sock_Con_addr[TheAmountOfMess], &addrlen);
				if (Me_Sock_Con[TheAmountOfMess] < 0)
				{
					return sock_err("accept", s);
				}
				set_non_block_mode(Me_Sock_Con[TheAmountOfMess]);
				buf = ntohl(Me_Sock_Con_addr[TheAmountOfMess].sin_addr.s_addr);
				

				fprintf(stdout, "Peer connected : %d.%d.%d.%d:%i\n", (buf >> 24) & 0xFF, (buf >> 16) & 0xFF, (buf >> 8) & 0xFF, (buf) & 0xFF, port);
			}
			for (int i = 0; i < (TheAmountOfMess + 1); i++)
			{
				if (FD_ISSET(Me_Sock_Con[i], &rfd))
				{ 
				  //metka:
					int rcv = recv_string(Me_Sock_Con[i], i, file, Me_Sock_Con_addr[i]);
					//int ierr = WSAGetLastError();

					//	if (ierr == WSAEWOULDBLOCK) {  // currently no data available
					//		Sleep(50);  // wait and try again
					//		goto metka;
					//	}
					if (rcv == 1)
					{
						fprintf(stdout, "'stop' message arrived.  Terminating...\n");
						fprintf(stdout, "Peer disconnected : %d.%d.%d.%d:%i\n", (buf >> 24) & 0xFF, (buf >> 16) & 0xFF, (buf >> 8) & 0xFF, (buf) & 0xFF, port);
						fclose(file);
						s_close(s);
						while (TheAmountOfMess != 0) {
							s_close(Me_Sock_Con[TheAmountOfMess]);
							TheAmountOfMess--;
						}
						return 0;
					}
				}
				if (FD_ISSET(Me_Sock_Con[i], &wfd))
				{ 
					if (send_notice(Me_Sock_Con[i], i, ecrec) != 0)
						return sock_err("send", Me_Sock_Con[i]);
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

	int Me_Sock_Con[100];
	port = atoi(argv[1]);
	int socket=0;
	printf("listening %i\n", port);
	HandleAlready(socket, Me_Sock_Con);

	s_close(socket);
	while (TheAmountOfMess != 0) {
		s_close(Me_Sock_Con[TheAmountOfMess]);
		TheAmountOfMess--;
	}
	deinit();
	return 0;


}