#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
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
#include<stdlib.h>
#include<string.h>
#include <time.h>
struct tmmy
{
	int tm_mday;
	int tm_mon;
	unsigned short tm_year;
	int tm_hour;
	int tm_min;
	int tm_sec;
};
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
void format_message_string(char* buffer, struct tmmy *t, struct tmmy* t1,char* message);
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
int recv_response(int s, FILE* f)
{
	char buffer[256];
	int res;
	// Принятие очередного блока данных.
	// Если соединение будет разорвано удаленным узлом recv вернет 0
	while ((res = recv(s, buffer, sizeof(buffer), 0)) > 0)
	{
		fwrite(buffer, 1, res, f);
		printf(" %d bytes received\n", res);
	}
	if (res < 0)
		return sock_err("recv", s);
	return 0;
}
int send_request(int s,const char* name_of_file)
{
	//const char* request = "GET / HTTP/1.0\r\nServer: " WEBHOST "\r\n\r\n";
	//int size = strlen(request);
	int sent = 0;
#ifdef _WIN32
	int flags = 0;
#else
	int flags = MSG_NOSIGNAL;
#endif
	/*while (sent < size)
	{
		// Отправка очередного блока данных
		int res = send(s, request + sent, size - sent, flags);
		if (res < 0)
			return sock_err("send", s);
		sent += res;
		printf(" %d bytes sent.\n", sent);
	}*/
	if (send(s, "put", 3, 0) == -1) {
		perror("send");
		exit(1);
	}
	FILE* f;
	//printf("%s", name_of_file);
	//f=fopen("C:\\Users\\Elvin\\source\\repos\\tcpclient\\cli20.txt","r");
	f=fopen(name_of_file,"r");

	if (f == NULL)
	printf("cannot open file");
	int num_of_mess = 0;
	char buffer[560];
	char tempbuf[100];
	memset(tempbuf, 0, 100);
	char response[100]="\0";
	//memset(response, 0, 100);
	char c = 0;
	//fprintf(stdout, "%c", c);
	struct tmmy t;
	struct tmmy t1;
	char message[256] = "\0";
	memset(message, 0, 256);
	char num_of_mess1[10] = "\0";
	for (;;)
	{
		while (!(feof(f)))
		{
			/*num_of_mess = htons(num_of_mess);
			send(s, _itoa(num_of_mess, num_of_mess1, 10), strlen(num_of_mess1), flags);
			num_of_mess++;*/
			fgets(buffer, 560, f);
			 
			format_message_string(buffer, &t, &t1, message);
			if ((t1.tm_mday < 1 || t1.tm_mday>31) || (t1.tm_mon < 1 || t1.tm_mon>12) || (t1.tm_year < 0 || t1.tm_year>9999) || (t.tm_mday < 1 || t.tm_mday>31) || (t.tm_mon < 1 || t.tm_mon>12) || (t.tm_year < 0 || t.tm_year>9999) || (t.tm_hour < 0 || t.tm_hour>23) || (t.tm_min < 0 || t.tm_min>59) || (t.tm_sec < 0 || t.tm_sec>59))
			{
				continue;
				memset(message, 0, 256);

			}
			if (message[0] == '\0' || message[0] == ' ' || (message[0]<'0' &&message[0]>'9') || message[0] == '\n')
			{
				continue;
				memset(message, 0, 256);
			}
			if (!strcmp(message, "\n"))
			//if (message[0], "\n")
			{
				continue;
				memset(message, 0, 256);
			}
			if (!strcmp(message, " "))
			//if (message[0], " ")
			{
				continue;
				memset(message, 0, 256);
			}
			//if (!strcmp(message, "stop"))
			if(message[0]=='s' && message[1] == 't' && message[2] == 'o' && message[3]=='p')
			{
				send(s, "stop", 4, flags);
				break;
			}
			
			num_of_mess = htonl(num_of_mess);
			_itoa(num_of_mess, num_of_mess1, 10);
			send(s, num_of_mess1, strlen(num_of_mess1), flags);

			_itoa(t1.tm_mday, tempbuf, 10);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);
			_itoa(t1.tm_mon, tempbuf, 10);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);
			t1.tm_year= htons(t1.tm_year);
			_itoa(t1.tm_year, tempbuf, 10);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);

			_itoa(t.tm_mday, tempbuf, 10);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);
			_itoa(t.tm_mon, tempbuf, 10);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);
			t.tm_year = htons(t.tm_year);
			_itoa(t.tm_year, tempbuf, 10);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);

			_itoa(t.tm_hour, tempbuf, 10);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);
			_itoa(t.tm_min, tempbuf, 10);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);
			//t.tm_sec = htons(t1.tm_year);
			_itoa(t.tm_sec, tempbuf, 10);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);

			//send(s, _itoa(t1.tm_year, tempbuf, 10), strlen(tempbuf), flags);
			//send(s, "31.12.2029 18:00:59 +70001234567 stopfghjhgfd", 50, 0);
			send(s, message, strlen(message)+1, flags);
			recv(s, response, sizeof(response), 0);
			//printf("%s\n", response);
			response[strlen(response)] = '\r';
			//fprintf(stdout,response);
			fprintf(stdout, "%s",response);
			memset(response, 0, strlen(response));
			//fflush(stdout);
			num_of_mess++;
			memset(message, 0, 256);
			memset(buffer, 0, 560);
			

		}
		break;
	}
	fclose(f);
	return 0;
}

void split(char* IP, char* port,char* argv)
{
	int i = 0;
	while (argv[i] != ':')
	{
		IP[i] = argv[i];
		i++;
	}
	i++;
	memcpy(port, argv + i, sizeof(strlen(argv + i)));

}

int main(int argc, char **argv)
{
	int i;
	/*for (i = 0; i < argc; i++)
	{
		printf("%s\n", argv[i]);
	}*/
	//char words_out[20];
	//добавь проверку на неверно введенные аргументы 
	init();




	char name_of_file[10]="\0";
	char IP_Adress[50]="\0";
	char number_of_port[50]="\0";
	//memset(name_of_file, 0, sizeof(char)*strlen(name_of_file));
	//memset(IP_Adress, 0, sizeof(char)*strlen(IP_Adress));
	//memset(number_of_port, 0, sizeof(char)*strlen(number_of_port));
	memcpy(name_of_file, argv[2], sizeof(char)*(strlen(argv[2])));
	split(IP_Adress, number_of_port,argv[1]);



	//struct hostent *he=gethostbyname(IP_Adress);


	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		printf("socket not opened");
		//puts(words_out, stdout); обработать !!
		return -1;
	}
	struct sockaddr_in addr;
	int nomer_porta = atoi(number_of_port);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nomer_porta);
	//addr.sin_addr.s_addr = inet_aton((IP_Adress));
	addr.sin_addr.s_addr = inet_addr(IP_Adress);
	
	int ret=connect(sockfd, (struct sockaddr*) &addr, sizeof(addr));
	int counter = 0;
	while (counter != 10)
	{
		if (counter == 10)
		{
			s_close(sockfd);
			return sock_err("connect", sockfd);
		}
		if (ret != 0)
		{
			ret = connect(sockfd, (struct sockaddr*) &addr, sizeof(addr));
#ifdef _WIN32
			Sleep(100);
#else
			usleep(100000); //задержка в мкс (100 мс)
#endif
		}
		if (ret == 0)
			break;
		counter++;

	}
	send_request(sockfd,name_of_file);
	//recv_response
	
	closesocket(sockfd);
	deinit();
	return 0;
}
 
void format_message_string(char *buffer, struct tmmy *t, struct tmmy *t1,char* message)
{
	if (buffer[0] == '\0' || buffer[0] == ' ' || (buffer[0]<'0' &&buffer[0]>'9') || buffer[0]=='\n')
		return;
	char* forstr; // тут будет лежать адрес с начала (отдельная строка) , 
	char* forstr1;
	char buf[10] = "\0";
	char buf1[10] = "\0";
	char buf2[10] = "\0";

	forstr = strtok(buffer, " ");
	int i = 0;
	int j = 0;
	while (forstr[i]!='\0')
	{
		while (forstr[i] != '.')
		{
			buf[j] = forstr[i];
			j++;
			i++;
		}
		t1->tm_mday = atoi(buf);
		j = 0;
		i++;
		memset(buf, 0, 10);
		//buf [0]= '\0';
		while (forstr[i] != '.')
		{
			buf[j] = forstr[i];
			j++;
			i++;
		}
		i++;
		t1->tm_mon = atoi(buf);
		j = 0;
		//buf[0] = '\0';
		memset(buf, 0, 10);
		while (forstr[i] != '\0')
		{
			buf[j] = forstr[i];
			j++;
			i++;
		}
		
		t1->tm_year = atoi(buf);
		//buf[0] = '\0';
		memset(buf, 0, 10);

	}
	forstr = strtok(NULL, " ");
	i = 0;
	j = 0;
	//buf[0] = '\0';
	memset(buf, 0, 10);

	while (forstr[i] != '\0')
	{
		while (forstr[i] != '.')
		{
			buf[j] = forstr[i];
			j++;
			i++;
		}
		t->tm_mday = atoi(buf);
		j = 0;
		i++;
		buf[0] = '\0';
		while (forstr[i] != '.')
		{
			buf[j] = forstr[i];
			j++;
			i++;
		}
		i++;
		t->tm_mon = atoi(buf);
		j = 0;
		buf[0] = '\0';

		while (forstr[i] != '\0')
		{
			buf[j] = forstr[i];
			j++;
			i++;
		}

		t->tm_year = atoi(buf);
		buf[0] = '\0';

	}
	forstr = strtok(NULL, " ");

	i = 0;
	j = 0;
	//buf[0] = '\0';
	memset(buf, 0, 10);

	while (forstr[i] != '\0')
	{
		while (forstr[i] != ':')
		{
			buf[j] = forstr[i];
			j++;
			i++;
		}
		t->tm_hour = atoi(buf);
		j = 0;
		i++;
		//buf[0] = '\0';
		memset(buf, 0, 10);

		while (forstr[i] != ':')
		{
			buf[j] = forstr[i];
			j++;
			i++;
		}
		i++;
		t->tm_min = atoi(buf);
		j = 0;
		//buf[0] = '\0';
		memset(buf, 0, 10);

		while (forstr[i] != '\0')
		{
			buf[j] = forstr[i];
			j++;
			i++;
		}

		t->tm_sec = atoi(buf);
		//buf[0] = '\0';
		memset(buf, 0, 10);

	}
	forstr = strtok(NULL, "\0");
	/*if (strcmp(forstr, "\n"))
		return;
	if (strcmp(forstr, " "))
		return;*/
	memcpy(message, forstr, strlen(forstr));
	
}

