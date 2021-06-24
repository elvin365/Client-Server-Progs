#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Äèðåêòèâà ëèíêîâùèêó: èñïîëüçîâàòü áèáëèîòåêó ñîêåòîâ
#pragma comment(lib, "ws2_32.lib")
#else // LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
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

void reverse(char s[])
{
	int i, j;
	char c;

	for (i = 0, j = strlen(s) - 1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}
void itoa(int n, char s[])
{
	int i, sign;

	if ((sign = n) < 0)  /* записываем знак */
		n = -n;          /* делаем n положительным числом */
	i = 0;
	do {       /* генерируем цифры в обратном порядке */
		s[i++] = n % 10 + '0';   /* берем следующую цифру */
	} while ((n /= 10) > 0);     /* удаляем */
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
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
void format_message_string(char* buffer, struct tmmy *t, struct tmmy* t1, char* message);
void deinit()
{
#ifdef _WIN32
	// Äëÿ Windows ñëåäóåò âûçâàòü WSACleanup â êîíöå ðàáîòû
	WSACleanup();
#else
	// Äëÿ äðóãèõ ÎÑ äåéñòâèé íå òðåáóåòñÿ
#endif
}

int init()
{
#ifdef _WIN32
	// Äëÿ Windows ñëåäóåò âûçâàòü WSAStartup ïåðåä íà÷àëîì èñïîëüçîâàíèÿ ñîêåòîâ
	WSADATA wsa_data;
	return (0 == WSAStartup(MAKEWORD(2, 2), &wsa_data));
#else
	return 1; // Äëÿ äðóãèõ ÎÑ äåéñòâèé íå òðåáóåòñÿ
#endif
}
int recv_response(int s)
{
	char buffer[256];
	int res;
	// Принятие очередного блока данных.
	// Если соединение будет разорвано удаленным узлом recv вернет 0
	while ((res = recv(s, buffer, sizeof(buffer), 0)) > 0)
	{
		//fwrite(buffer, 1, res, f);
		printf(" %d bytes received\n", res);
	}
	if (res < 0)
		return sock_err("recv", s);
	return 0;
}
int send_request(int s, const char* name_of_file)
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
	// Îòïðàâêà î÷åðåäíîãî áëîêà äàííûõ
	int res = send(s, request + sent, size - sent, flags);
	if (res < 0)
	return sock_err("send", s);
	sent += res;
	printf(" %d bytes sent.\n", sent);
	}*/
	

	if (send(s, "put", 3, 0) == -1) {
		//perror("send");
		fprintf(stdout, "nosend");
		exit(1);
	}
	//send(s, "stop", 4, flags);
	FILE* f;
	//printf("%s", name_of_file);
	f = fopen(name_of_file, "r");
	if (f == NULL)
		printf("cannot open file");
	int num_of_mess = 0;
	char buffer[512];
	char tempbuf[100];
	memset(tempbuf, 0, 100);
	char response[2] = "\0";
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
			send(s, itoa(num_of_mess, num_of_mess1, 10), strlen(num_of_mess1), flags);
			num_of_mess++;*/
			//fgets(buffer, 50000, f);
			fgets(buffer, 512, f);
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
			{
				continue;
				memset(message, 0, 256);
			}
			if (!strcmp(message, " "))
			{
				continue;
				memset(message, 0, 256);
			}
			if (message[0] == 's' && message[1] == 't' && message[2] == 'o' && message[3] == 'p')
			{
				send(s, "stop", 4, flags);
				break;
			}


			num_of_mess = htons(num_of_mess);
			itoa(num_of_mess, num_of_mess1);
			send(s, num_of_mess1, strlen(num_of_mess1), flags);

			itoa(t1.tm_mday, tempbuf);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);
			itoa(t1.tm_mon, tempbuf);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);
			t1.tm_year = htons(t1.tm_year);
			itoa(t1.tm_year, tempbuf);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);

			itoa(t.tm_mday, tempbuf);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);
			itoa(t.tm_mon, tempbuf);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);
			t.tm_year = htons(t.tm_year);
			itoa(t.tm_year, tempbuf);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);

			itoa(t.tm_hour, tempbuf);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);
			itoa(t.tm_min, tempbuf);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);
			//t.tm_sec = htons(t1.tm_year);
			itoa(t.tm_sec, tempbuf);
			send(s, tempbuf, strlen(tempbuf), flags);
			memset(tempbuf, 0, 100);

			//send(s, _itoa(t1.tm_year, tempbuf, 10), strlen(tempbuf), flags);
			//send(s, "31.12.2029 18:00:59 +70001234567 stopfghjhgfd", 50, 0);
			send(s, message, strlen(message) + 1, flags);



			char buffer[2];
			int res1 = 0;
			// Принимаем подтверждение от сервера
			while (1)
			{
				buffer[0] = '\0';
				buffer[1] = '\0';
				res1 = recv(s, buffer, 1, 0);
				printf("%d  ", res1);
				if (res1 == 0)
				{
					printf("\nThe connection was terminated by the remote party\n");
					return 0;
				}
				else if (res1 < 0)
					return sock_err("recv", s);
				if (buffer[0] == 'o')
					break;
			}
			while (1)
			{
				buffer[0] = '\0';
				buffer[1] = '\0';
				res1 = recv(s, buffer, 1, 0);
				printf("%d  ", res1);
				if (res1 == 0)
				{
					printf("\nThe connection was terminated by the remote party\n");
					return 0;
				}
				else if (res1 < 0)
					return sock_err("recv", s);
				if (buffer[0] == 'k')
					break;
			}

			buffer[0] = '\0';
			buffer[1] = '\0';





			num_of_mess++;
			memset(message, 0, 256);
			memset(buffer, 0, 512);


		}
		break;
	}
	fclose(f);
	return 0;
}

void split(char* IP, char* port, char* argv)
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
unsigned int get_host_ipn(const char* name)
{
	struct addrinfo* addr = 0;
	unsigned int ip4addr = 0;
	// Функция возвращает все адреса указанного хоста
	// в виде динамического однонаправленного списка
	if (0 == getaddrinfo(name, 0, 0, &addr))
	{
		struct addrinfo* cur = addr;
		while (cur)
		{
			// Интересует только IPv4 адрес, если их несколько - то первый
			if (cur->ai_family == AF_INET)
			{
				ip4addr = ((struct sockaddr_in*) cur->ai_addr)->sin_addr.s_addr;
				break;
			}
			cur = cur->ai_next;
		}
		freeaddrinfo(addr);
	}
	return ip4addr;
}

int main(int argc, char **argv)
{
	//fprintf(stdout,"%d",1);
	int i;

	/*for (i = 0; i < argc; i++)
	{
	printf("%s\n", argv[i]);
	}*/
	//char words_out[20];
	//äîáàâü ïðîâåðêó íà íåâåðíî ââåäåííûå àðãóìåíòû 
	init();




	char name_of_file[50] = "\0";
	char IP_Adress[50] = "\0";
	char number_of_port[50] = "\0";
	//memset(name_of_file, 0, sizeof(char)*strlen(name_of_file));
	//memset(IP_Adress, 0, sizeof(char)*strlen(IP_Adress));
	//memset(number_of_port, 0, sizeof(char)*strlen(number_of_port));
	memcpy(name_of_file, argv[2], sizeof(char)*(strlen(argv[2])));
	split(IP_Adress, number_of_port, argv[1]);



	//struct hostent *he=gethostbyname(IP_Adress);


	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		//printf("socket not opened");
		fprintf(stdout, "socket not opened");
		//puts(words_out, stdout); îáðàáîòàòü !!
		return -1;
	}
	struct sockaddr_in addr;
	unsigned short int nomer_porta = (unsigned short int )atoi(number_of_port);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nomer_porta);
	//addr.sin_addr.s_addr = inet_aton((IP_Adress));
	//addr.sin_addr.s_addr = inet_addr(IP_Adress);
	addr.sin_addr.s_addr = get_host_ipn(IP_Adress);
	int ret = connect(sockfd, (struct sockaddr*) &addr, sizeof(addr));
	int counter = 0;
	if (ret == -1)
	{
		while (counter != 10)
		{
			if (counter == 10)
			{
				s_close(sockfd);
				return sock_err("connect", sockfd);
			}
			if (ret == -1)
			{
				ret = connect(sockfd, (struct sockaddr*) &addr, sizeof(addr));
				if (ret == 0)
					break;
#ifdef _WIN32
				Sleep(100);
#else
				usleep(100000); //çàäåðæêà â ìêñ (100 ìñ)
#endif
			}
			if (ret == 0)
				break;
			counter++;
		}
	}
	send_request(sockfd, name_of_file);
	//recv_response

	closesocket(sockfd);
	//close(sockfd);
	deinit();
	return 0;
}

void format_message_string(char *buffer, struct tmmy *t, struct tmmy *t1, char* message)
{
	if (buffer[0] == '\0' || buffer[0] == ' ' || (buffer[0]<'0' &&buffer[0]>'9') || buffer[0] == '\n')
		return;
	char* forstr; // òóò áóäåò ëåæàòü àäðåñ ñ íà÷àëà (îòäåëüíàÿ ñòðîêà) , 
	char* forstr1;
	char buf[10] = "\0";
	char buf1[10] = "\0";
	char buf2[10] = "\0";

	forstr = strtok(buffer, " ");
	int i = 0;
	int j = 0;
	while (forstr[i] != '\0')
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

