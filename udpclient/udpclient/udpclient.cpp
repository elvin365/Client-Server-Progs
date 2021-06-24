#define _WINSOCK_DEPRECATED_NO_WARNINGS
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
#include <sys/time.h>
#include <sys/select.h>
#include <netdb.h>
#include <errno.h>
#endif

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
unsigned int number_of_message;
char* datagrams[20];
#define N 20000
int already_recieved = 0;
unsigned int datagram[20];

FILE* f=NULL;
void parsing_file(char*  name_of_file, struct my_time *data_and_message);
int send_your_mess(int s, struct sockaddr_in *addr, struct my_time* data_and_message);
unsigned int recv_response(int s, struct sockaddr_in *addr, struct my_time* data_and_message);
void resend_msg(int s, struct sockaddr_in *addr, struct my_time* data_and_message);


struct my_time
{
	unsigned int inx;
	unsigned int tm_myday1;
	unsigned int tm_mymonth1;
	unsigned short tm_myyear1;
	unsigned int tm_myhour1;
	unsigned int tm_myminutes1;
	unsigned int tm_mysec1;
	unsigned int tm_myday2;
	unsigned int tm_mymonth2;
	unsigned short tm_myyear2;

	char* message;
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
void s_close(int s)
{
#ifdef _WIN32
	closesocket(s);
#else
	close(s);
#endif
}
int main(int argc,char* argv[])
{
	init();
	int s;
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		return sock_err("socket", s);
	short port;
	char *ip=NULL;
	struct sockaddr_in addr;
	//if (argc == 3)
	//{
		strtok(argv[1], ":");
		ip = argv[1];
		port = (short int)atoi(strtok(NULL, "\0"));
		f = fopen(argv[2], "r");

	//}
	if (f == NULL)
	{
		printf("Error: cannot open file!\n");
		return 0;
	}
	// Заполнение структуры с адресом удаленного узла
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port); // Порт DNS - 53
	addr.sin_addr.s_addr = inet_addr(ip);

	printf(" Server address: %u.%u.%u.%u:%u\n",
(ntohl(inet_addr(ip)) >> 24) & 0xFF, (ntohl(inet_addr(ip)) >> 16) & 0xFF, (ntohl(inet_addr(ip)) >> 8) & 0xFF, (ntohl(inet_addr(ip))) & 0xFF, htons(port));
	//send_your_mess(s, &addr);
	struct my_time data_and_message[N];

	parsing_file(argv[2], data_and_message);

	send_your_mess(s, &addr, data_and_message);

	while (already_recieved <= 20  && already_recieved!=number_of_message )
	{
		recv_response(s, &addr, data_and_message);
		//resend_msg(s, &addr, data_and_message);
	}
	deinit();
	return 0;
}

void parsing_file(char*  name_of_file, struct my_time* data_and_message)
{
	int i = 0;
	char datas[512] = "\0";
	char message[50000] = "\0";
	char* data1 = NULL;
	char* data2 = NULL;
	char* data3 = NULL;
	int trh_space = 0;
	FILE* f;
	f = fopen(name_of_file, "r");
	for (number_of_message = 0; number_of_message < 20 && !(feof(f)); number_of_message++)
	{
		//while (!(feof(f)))
		//{
			i = 0;
			char c = '1';
			while (!isalpha(c))
			{
				if (trh_space == 3)
					break;
				c = getc(f);
				if (c == ' ')
					trh_space++;
				datas[i] = c;
				i++;
			}
			trh_space = 0;
			datas[strlen(datas) - 1] = '\0';
			i = 0;
			//fseek(f, -1, SEEK_CUR);
			//c=getc(f);
			data1 = strtok(datas, " ");
			data2 = strtok(NULL, " ");
			data3 = strtok(NULL, " ");
			data_and_message[number_of_message].tm_myday1 = atoi(strtok(data1, "."));
			data_and_message[number_of_message].tm_mymonth1 = atoi(strtok(NULL, "."));
			data_and_message[number_of_message].tm_myyear1 = atoi(strtok(NULL, "."));


			data_and_message[number_of_message].tm_myday2 = atoi(strtok(data2, "."));
			data_and_message[number_of_message].tm_mymonth2 = atoi(strtok(NULL, "."));
			data_and_message[number_of_message].tm_myyear2 = atoi(strtok(NULL, "."));

			data_and_message[number_of_message].tm_myhour1 = atoi(strtok(data3, ":"));
			data_and_message[number_of_message].tm_myminutes1 = atoi(strtok(NULL, ":"));
			data_and_message[number_of_message].tm_mysec1 = atoi(strtok(NULL, ":"));

			memset(datas, 0, strlen(datas));
			memset(data1, 0, strlen(data1));
			memset(data2, 0, strlen(data2));
			memset(data3, 0, strlen(data3));
			fgets(message, 512, f);
			if (!strcmp(message, "\n"))
				continue;
			if (!strcmp(message, "stop"))
			{
				message[strlen(message)] = '\0';
				data_and_message[number_of_message].message = (char*)calloc(strlen(message) + 1, sizeof(char));
				memcpy(data_and_message[number_of_message].message, message, strlen(message));
			}
			else
			{
				char* mes1;
				mes1 = strtok(message, "\n");
				data_and_message[number_of_message].message = (char*)calloc(strlen(message) + 1, sizeof(char));
				memcpy(data_and_message[number_of_message].message, mes1, strlen(mes1));
			}
				
			//data_and_message[number_of_message].message = (char*)calloc(strlen(message) + 1, sizeof(char));
			//memcpy(data_and_message[number_of_message].message, message, strlen(message));
		//}
	}

/*			c = getc(f);
			int flag = 0;
			i = 0;
			while (message[i] = getc(f))
			{

				if (isdigit(message[i]))
				{
					flag++;
					i++;
					message[i] = getc(f);
				}

				if (isdigit(message[i]))
				{
					flag++;
					i++;
					message[i] = getc(f);

				}

				if (message[i] == '.')
				{
					flag++;
					i++;
					// message[i]=getc(f);

					break;
				}
				if (feof(f))
					break;
				flag = 0;
				i++;
			}
			data_and_message[number_of_message].message = (char*)calloc(strlen(message) + 1, sizeof(char));

			if (flag == 3)
			{

				for (int i = strlen(message); i>0; i--)
				{
					if (message[i] == '\n' && message[i - 1] == '\n')
					{
						memset(message + i - 1, 0, 50000 - i - 1);
						break;
					}
					if (message[i] == '\n')
					{
						memset(message + i, 0, 50000 - i);
						break;
					}
				}
				fseek(f, -(int)sizeof(char), SEEK_CUR);
				fseek(f, -(int)sizeof(char), SEEK_CUR);
				fseek(f, -(int)sizeof(char), SEEK_CUR);
				


				memcpy(data_and_message[number_of_message].message, message, strlen(message) + 1);
				memset(message, 0, 50000);
				number_of_message++;
				continue;
			}
			if (flag == 2)
			{
				if (message[i] == '\377' || -1 == int(message[i]))
					message[i] = '\0';
				memcpy(data_and_message[number_of_message].message, message, strlen(message) + 1);
				memset(message, 0, 50000);
				number_of_message++;
				continue;
			}
			if (flag == 1)
			{
				//fprintf(stdout,"%s",message);
				if (message[i] == '\377' || -1 == int(message[i]))
					message[i] = '\0';
				memcpy(data_and_message[number_of_message].message, message, strlen(message) + 1);
				memset(message, 0, 50000);
				number_of_message++;
				continue;
			}
			if (flag == 0)
			{
				message[strlen(message) - 1] = '\0';
				memcpy(data_and_message[number_of_message].message, message, strlen(message) + 1);
				memset(message, 0, 50000);
			}
			number_of_message++;

			memset(message, 0, 50000);



		}
		break;
	}

*/
}

int send_your_mess(int s, struct sockaddr_in *addr, struct my_time* data_and_message)
{
	//char *str = (char*)calloc(N, sizeof(char));
	char str[50000] = "\0";
	int size = 0;
	 int i;
	 int NETWORK_num_to_send = 0;
	 int len = 0;
	for (i = 0; i < number_of_message; i++)
	{
		data_and_message[i].inx = i;
		NETWORK_num_to_send = htonl(i);
		
		memcpy(str, &NETWORK_num_to_send, sizeof(int));
		len = 4;
		strncat(str+sizeof(NETWORK_num_to_send), (const char*)&data_and_message[i].tm_myday1, 1);
		len++;
		strncat(str+ 5, (const char*)&data_and_message[i].tm_mymonth1, 1);
		len++;
		unsigned short year = htons(data_and_message[i].tm_myyear1);
		len += 2;
		char god1[3] = "\0";
		memcpy(god1, &year, 2);
		strncat(str+ 6, god1, 2);

		strncat(str+ 8, (const char*)&data_and_message[i].tm_myday2, 1);
		len++;
		strncat(str+ 9, (const char*)&data_and_message[i].tm_mymonth2, 1);
		len++;
		  year = htons(data_and_message[i].tm_myyear2);
		memset(god1, 0, 3);
		memcpy(god1, &year, 2);
		strncat(str+ 10, god1, 2);
		len += 2;


		strncat(str+ 12, (const char*)&data_and_message[i].tm_myhour1, 1);
		len++;
		strncat(str+ 13, (const char*)&data_and_message[i].tm_myminutes1, 1);
		len++;
		strncat(str+ 14, (const char*)&data_and_message[i].tm_mysec1, 1);
		len++;
		int oldlen = strlen(data_and_message[i].message);
		len = len + oldlen;
		strncat(str + 15, data_and_message[i].message, strlen(data_and_message[i].message));
		
			int res = sendto(s, str, len + 1, 0, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
			memset(str, 0, len + 1);
		
		

	}
	return 0;
}
void resend_msg(int s, sockaddr_in *addr,struct  my_time* data_and_message)
{
	char str[50000] = "\0";
	int size = 0;
	int i;
	int NETWORK_num_to_send = 0;
	int len = 0;
	for (i = 0; i < number_of_message && data_and_message[i].tm_myday1!=0; i++)
	{
		//data_and_message[i].inx = i;
		NETWORK_num_to_send = htonl(data_and_message[i].inx);

		memcpy(str, &NETWORK_num_to_send, sizeof(int));
		len = 4;
		strncat(str + sizeof(NETWORK_num_to_send), (const char*)&data_and_message[i].tm_myday1, 1);
		len++;
		strncat(str + 5, (const char*)&data_and_message[i].tm_mymonth1, 1);
		len++;
		unsigned short year = htons(data_and_message[i].tm_myyear1);
		len += 2;
		char god1[3] = "\0";
		memcpy(god1, &year, 2);
		strncat(str + 6, god1, 2);

		strncat(str + 8, (const char*)&data_and_message[i].tm_myday2, 1);
		len++;
		strncat(str + 9, (const char*)&data_and_message[i].tm_mymonth2, 1);
		len++;
		year = htons(data_and_message[i].tm_myyear2);
		memset(god1, 0, 3);
		memcpy(god1, &year, 2);
		strncat(str + 10, god1, 2);
		len += 2;


		strncat(str + 12, (const char*)&data_and_message[i].tm_myhour1, 1);
		len++;
		strncat(str + 13, (const char*)&data_and_message[i].tm_myminutes1, 1);
		len++;
		strncat(str + 14, (const char*)&data_and_message[i].tm_mysec1, 1);
		len++;
		int oldlen = strlen(data_and_message[i].message);
		len = len + oldlen;
		strncat(str + 15, data_and_message[i].message, strlen(data_and_message[i].message));

		int res = sendto(s, str, len + 1, 0, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
		memset(str, 0, len + 1);

	}

}
unsigned int recv_response(int s, sockaddr_in *send_addr, struct my_time* data_and_message)
{
	
	struct timeval tv = { 0, 100 * 1000 }; // 100 msec
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(s, &fds);
	int received = 0;
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	int res = 0;
	do
	{
		FD_ZERO(&fds);
		FD_SET(s, &fds);
		// Проверка - если в сокете входящие дейтаграммы
		// (ожидание в течение tv)
		 res = select(s + 1, &fds, 0, 0, &tv);
		
		if (res > 0)
		{ // Данные есть, считывание их

			received = recvfrom(s, (char*)datagram, sizeof(datagram), 0, (struct sockaddr *)&addr, &addrlen);
			if (received <= 0)
			{
				// Ошибка считывания полученной дейтаграммы
				sock_err("recvfrom", s);
				return 0;
			}
			received /= sizeof(int);
			// Удаляем информацию о датаграммах,полученных сервером
			for (int i = 0; i < received; i++)
			{
				datagram[i] = ntohl(datagram[i]);
				if (datagram[i] <= 20 && data_and_message[datagram[i]].tm_myday1 != 0)
				{
					already_recieved++;
					
					data_and_message[datagram[i]].tm_myday1 = 0;// зануляются те сообщения,что были отправлены
				}

			}
			// Повторно отправлеям неполученные датаграммы
			resend_msg(s, &addr, data_and_message);
		}

	}  while (res>0);
	

	return 0;
}