#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <poll.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <fcntl.h>
#include<stdlib.h>
#include <iostream>
using namespace std;
FILE *f;
bool stop;
int cscnt;
unsigned int *msg_cnt;
//#define N 5535
#define N 20000
struct client
{
    unsigned int ip;
    unsigned short int port;

    int idx[50];
};

client *clients;
int set_non_block_mode(int ls)
{
    int fl = fcntl(ls, F_GETFL, 0);
    return fcntl(ls, F_SETFL, fl | O_NONBLOCK);
}
int sock_err(const char *function, int ls)
{
    int err;
    err = errno;
    fprintf(stderr, "%ls: socket error: %d\n", function, err);
    return -1;
}
int recv_msg(int,struct sockaddr_in);
/* A utility function to reverse a string  */
void reverse(char str[], int length)
{
    int start = 0;
    int end = length -1;
    while (start < end)
    {
        swap(*(str+start), *(str+end));
        start++;
        end--;
    }
}

// Implementation of itoa()
char* itoa(int num, char* str, int base)
{
    int i = 0;
    bool isNegative = false;

    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // In standard itoa(), negative numbers are handled only with
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = true;
        num = -num;
    }

    // Process individual digits
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }

    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';

    str[i] = '\0'; // Append string terminator

    // Reverse the string
    reverse(str, i);

    return str;
}
int fill_in(struct sockaddr_in *addr)
{
    unsigned int ip = ntohl(addr->sin_addr.s_addr);
    unsigned short int port = ntohs(addr->sin_port);
    for (int i = 0; i < cscnt; i++)
    {
        if (clients[i].ip == ip && clients[i].port == port)
            return i;
    }
    cscnt++;
    clients = (client *)realloc(clients, (cscnt+1) * sizeof(client));
    clients[cscnt - 1].ip = ip;
    clients[cscnt - 1].port = port;
    for (int i = 0; i < 50; i++)
        clients[cscnt - 1].idx[i] = -1;

    printf("New client detected: %u.%u.%u.%u:%u\n",
           (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip)&0xFF, port);
    return cscnt - 1;
}
bool add_idx(int num, int idx)
{
    int j = -1;
    for (int i = 0; i < 50; i++)
    {
        if (clients[num].idx[i] == idx)
        {
            return false;
        }
        if (clients[num].idx[i] == -1 && j == -1)
            j = i;
    }
    if (j >= 0 && j < 50)
        clients[num].idx[j] = idx;

    return true;
}
int send_notice(int num, int s, sockaddr_in *addr)
{
    char str[80] = {'\0'};
    char *ptr = str;
    // формирвоание ответа
    for (int i = 0; i < 20; i++)
    {
        if (clients[num].idx[i] != -1)
        {
            int idx = htonl(clients[num].idx[i]);
            //int id = clients[num].idx[i];
            memcpy(ptr, &idx, sizeof(int));
            ptr += 4;
        }
    }
    // Отправка ответа
    int res = sendto(s, str, ptr - str,
                     0, (struct sockaddr *)addr, sizeof(struct sockaddr_in));
    if (res <= 0)
        sock_err("sendto", s);
}
int main(int argc, char* argv[])
{
    short int port1, port2;
    if (argc == 2)
    {
        port1 = (short int)atoi(argv[1]);
        port2 = port1;
    }
    else if (argc == 3)
    {
        port1 = (short int)atoi(argv[1]);
        port2 = (short int)atoi(argv[2]);
        if(port2<port1){
            printf("No port to listen\n");
            return 0;
        }
    }
    else
    {
        port1 = 8700;
        port2 = 8700;
    }
    const int port_num = port2 - port1 + 1;
    int *ls = (int *)calloc(port_num, sizeof(int)); // Создание UDP-сокета
    msg_cnt = (unsigned int *)calloc(port_num, sizeof(int));
    clients = (client *)calloc(1, sizeof(client)); // Массив инфорации
    struct sockaddr_in addr[N];

    int flags = MSG_NOSIGNAL;
    int nfds = 0;
    f = fopen("msg.txt", "a+");


     struct pollfd *pfd;
    pfd=(pollfd*)malloc(sizeof(pollfd)*port_num);

// Привязка сокетов
    printf("Listening on:");
    for (int i = 0; i < port_num; i++)
    {
        ls[i] = socket(AF_INET, SOCK_DGRAM, 0);
        if (ls < 0)
            return sock_err("socket", ls[i]);
        set_non_block_mode(ls[i]);

        // Заполнение структуры с адресом прослушивания узла
        memset(&addr, 0, sizeof(addr));
        addr[i].sin_family = AF_INET;
        addr[i].sin_port = htons(i + port1); // Будет прослушиваться порт 8000
        addr[i].sin_addr.s_addr = htonl(INADDR_ANY);
        // Связь адреса и сокета, чтобы он мог принимать входящие дейтаграммы
        if (bind(ls[i], (struct sockaddr *)&addr[i], sizeof(addr[i])) < 0)
            return sock_err("bind", ls[i]);

          pfd[i].fd = ls[i];
            pfd[i].events = POLLIN | POLLOUT ;

    }


    while (1)
	{
		int ev_cnt = poll(pfd, port_num, 1000); // Waiting events for 1 sec
		if (ev_cnt > 0)
		{

			for (int i = 0; i < port_num; i++)
			{
				if (pfd[i].revents & POLLHUP)
				{
				 // Client disconnected, closing socket
                    for(int j=0;j<port_num;j++)
                    {
                    close(ls[j]);
                    return 0;
                    }
				}
				if (pfd[i].revents & POLLERR) // Error, closing socket
                {
                   for(int j=0;j<port_num;j++)
                    {
                    close(ls[j]);
                   return 0;
                    }
				}
				if (pfd[i].revents & POLLIN) // Sock_arr[i] avaible for reading, call recv
				{
					recv_msg(ls[i],addr[i]);
					if (stop)
                    {
                        printf("'stop' message arrived. Terminating...\n");
                        //fclose(f);

                        // Закрытие сокета
                        for (int i = 0; i < port_num; i++)
                            close(ls[i]);
                       free(ls);
                       free(clients);
                       free(msg_cnt);
                       free(pfd);
                       fclose(f);
                        return 0;
                    }

				}
			}
		}
	}







}









int recv_msg(int cs,struct sockaddr_in addr)
{
    int flags = MSG_NOSIGNAL;
    //struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    char buffer[N] = {'\0'};
    //char str[N] = {'\0'};


    char *buf = buffer;
    int rcv = recvfrom(cs, (char *)buffer, sizeof(buffer) - 1, flags, (struct sockaddr *)&addr, &addrlen);

    if (rcv < 0)
        return sock_err("recv", cs);
    if (rcv == 0)
        return 0;
   int num= fill_in(&addr);// number of client has right come

    int idx;
    memcpy(&idx, buf, 4);
    if (!add_idx(num, ntohl(idx)))
    {
        return 0;
    }
    buf += 4;

unsigned int ip = clients[num].ip;
    fprintf(f, "%u.%u.%u.%u:%u ",
            (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip)&0xFF, clients[num].port);
    char day;
    char month;
    char year_buf[2]="\0";
    unsigned short year=0;

    for(int i=0;i<2;i++)
    {
    memcpy(&day,buf,1);// got the first day
     buf++;
    if ((int)day < 10) fprintf(f, "0");
		fprintf(f, "%i.", (int)day);





    memcpy(&month,buf,1);// got the first day
     buf++;
    if ((int)month < 10) fprintf(f, "0");
		fprintf(f, "%i.", (int)month);



    ///////


    memcpy(year_buf,buf,2);
    buf=buf+2;
    memcpy(&year, year_buf, 2);
		fprintf(f, "%u ", htons(year));
    }

    char dma;

    memcpy(&dma,buf,1);// got the first day
     buf++;
    if ((int)dma < 10) fprintf(f, "0");
	fprintf(f, "%i:", (int)dma);

    memcpy(&dma,buf,1);// got the first day
     buf++;
    if ((int)dma < 10) fprintf(f, "0");
	fprintf(f, "%i:", (int)dma);


    memcpy(&dma,buf,1);// got the first day
     buf++;
    if ((int)dma<10) fprintf(f, "0");
	fprintf(f, "%i ", (int)dma);


    char message[N]="\0";

    memcpy(message,buf,strlen(buf));
    fprintf(f,"%s",message);
    if(!strcmp(message,"stop"))
    {
    stop=1;
    }

   if(strcmp(message,"stop"))
    {
        fputs("\n", f);

    }


    msg_cnt[num]++;


    send_notice(num, cs, &addr);
memset(message,0,N);
buf=NULL;
memset(buffer,0,N);
return 0;
}



















