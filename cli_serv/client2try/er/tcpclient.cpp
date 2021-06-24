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
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#endif
#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include<cctype>
#include <iostream>
using namespace std;
#define N 30000
int number_of_message=0;
struct my_time
{
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
int sock_err(const char* function, int s)
{
int err;
#ifdef _WIN32
err = WSAGetLastError();
    #elseerr = errno;
    #endif
    fprintf(stderr, "%s: socket error: %d\n", function, err);
    return -1;
}
void parsing_file(char*  name_of_file,struct my_time *data_and_message);
void send_messages(struct my_time* data_and_message,int s);
int check(char* buf)
{
    int points=4;
    int i=0;
    int doublepoints=1;
    if(!strcmp(buf,"\n") || !strcmp(buf," "))
    return 0;
    while(buf[i]!='\0')
    {
    if(buf[i]=='.')
        points--;
    if(buf[i]==':')
        doublepoints--;
    if(points==0 && doublepoints==0)
    return 1;
    i++;
    }

}
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

int main(int argc ,char *argv[])
{

    if(argc!=3)
    {
    printf("Wrong");
    return 0;
    }

    const char* ip=strtok(argv[1],":");
    int port=atoi(strtok(NULL," "));
    struct my_time data_and_message[N];

    parsing_file(argv[2],data_and_message);

    struct sockaddr_in addr;
    int s=socket(AF_INET,SOCK_STREAM,0);
    if(s<0)
    return sock_err("socket",s);

    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=get_host_ipn(ip);

    for(int i=0;i<10;i++)
    {
        if(connect(s,(struct sockaddr*)&addr,sizeof(addr))!=0)
        {
        usleep(100000);
        }
        if(i==9 && connect(s,(struct sockaddr*)&addr,sizeof(addr)!=0))
        {
            close(s);
            return sock_err("connect",s);
        }
        else
        {
       /* char put='p';
        int res=send(s,&put,1,0);
        put='u';
         res=send(s,&put,1,0);
        put='t';
         res=send(s,&put,1,0);
        break;*/
        char put[4]="put";
        send(s,put,3,0);
        break;
        }
    }
    send_messages(data_and_message,s);

    close(s);

    return 0;


}

void parsing_file(char*  name_of_file,struct my_time* data_and_message)
{
    int i=0;
    char datas[512]="\0";
    char message[50000]="\0";
    char* data1="\0";
    char* data2="\0";
    char* data3="\0";
    FILE* f;
    f=fopen(name_of_file,"r");
    for(number_of_message=0;number_of_message<N;number_of_message++)
    {
    while(!(feof(f)))
    {
        i=0;
            char c='1';
           while(!isalpha(c))
            {
                c=getc(f);
                datas[i]=c;
                i++;
            }
        datas[strlen(datas)-1]='\0';
        i=0;
        fseek(f,-sizeof(char),SEEK_CUR);
        //c=getc(f);
        data1=strtok(datas," ");
        data2=strtok(NULL," ");
        data3=strtok(NULL," ");
        data_and_message[number_of_message].tm_myday1=atoi(strtok(data1,"."));
        data_and_message[number_of_message].tm_mymonth1=atoi(strtok(NULL,"."));
        data_and_message[number_of_message].tm_myyear1=atoi(strtok(NULL,"."));


        data_and_message[number_of_message].tm_myday2=atoi(strtok(data2,"."));
        data_and_message[number_of_message].tm_mymonth2=atoi(strtok(NULL,"."));
        data_and_message[number_of_message].tm_myyear2=atoi(strtok(NULL,"."));

        data_and_message[number_of_message].tm_myhour1=atoi(strtok(data3,":"));
        data_and_message[number_of_message].tm_myminutes1=atoi(strtok(NULL,":"));
        data_and_message[number_of_message].tm_mysec1=atoi(strtok(NULL,":"));

        memset(datas,0,strlen(datas));
        memset(data1,0,strlen(data1));
        memset(data2,0,strlen(data2));
        memset(data3,0,strlen(data3));

        int flag=0;
        i=0;
        while(message[i]=getc(f))
        {

            if(isdigit(message[i]))
            {
            flag++;
            i++;
            message[i]=getc(f);
            }

            if(isdigit(message[i]))
            {
                flag++;
                i++;
                message[i]=getc(f);

            }

            if(message[i]=='.')
            {
                flag++;
                i++;
               // message[i]=getc(f);

                break;
            }
            if (feof(f))
                break;
            flag=0;
        i++;
        }
        data_and_message[number_of_message].message=(char*)calloc(strlen(message)+1,sizeof(char));

        if(flag==3)
        {

            for(int i=strlen(message);i>0;i--)
            {
            if(message[i]=='\n' && message[i-1]=='\n')
            {
            memset(message+i-1,0,50000-i-1);
            break;
            }
            if(message[i]=='\n')
            {
            memset(message+i,0,50000-i);
            break;
            }
            }
        fseek(f,-sizeof(char),SEEK_CUR);
        fseek(f,-sizeof(char),SEEK_CUR);
        fseek(f,-sizeof(char),SEEK_CUR);

        memcpy(data_and_message[number_of_message].message,message,strlen(message)+1);
        memset(message,0,50000);
        number_of_message++;
        continue;
        }
        if(flag==2)
        {
        if(message[i]=='\377' || -1 ==int(message[i]) )
        message[i]='\0';
        memcpy(data_and_message[number_of_message].message,message,strlen(message)+1);
        memset(message,0,50000);
        number_of_message++;
        continue;
        }
        if(flag==1)
        {
        //fprintf(stdout,"%s",message);
        if(message[i]=='\377' || -1 ==int(message[i]) )
        message[i]='\0';
        memcpy(data_and_message[number_of_message].message,message,strlen(message)+1);
        memset(message,0,50000);
        number_of_message++;
        continue;
        }
        if(flag==0)
        {
        message[strlen(message)-1]='\0';
        memcpy(data_and_message[number_of_message].message,message,strlen(message)+1);
        memset(message,0,50000);
        }
            number_of_message++;

memset(message,0,50000);



    }
    break;
}


}



void send_messages(struct my_time* data_and_message, int s)
{
        int number_of_message2=0;
        int number_of_message2_to_send=0;
        char the_number_to_send;
        char nmsg[5]="\0";
        char one_byte[2]="\0";
        char ok[3]="\0";

        for(number_of_message2=0;number_of_message2<number_of_message;number_of_message2++)
        {

            number_of_message2_to_send=htonl(number_of_message2);
            //number_of_message2_to_send=7;
            memcpy(nmsg,&number_of_message2_to_send,4);
            send(s,nmsg,4,MSG_NOSIGNAL); // we are sinding the number of message

            // now we are seinding the chislo of month
            one_byte[0]=char(data_and_message[number_of_message2].tm_myday1);
            //one_byte[1]="\0";
            send(s,&one_byte[0],1,MSG_NOSIGNAL);

            one_byte[0]=char(data_and_message[number_of_message2].tm_mymonth1);
            //one_byte[1]="\0";
            send(s,&one_byte[0],1,MSG_NOSIGNAL);

          unsigned short year=htons(data_and_message[number_of_message2].tm_myyear1);
         send(s,&year,2,MSG_NOSIGNAL);


            one_byte[0]=char(data_and_message[number_of_message2].tm_myday2);
            //one_byte[1]="\0";
            send(s,&one_byte[0],1,MSG_NOSIGNAL);

            one_byte[0]=char(data_and_message[number_of_message2].tm_mymonth2);
            //one_byte[1]="\0";
            send(s,&one_byte[0],1,MSG_NOSIGNAL);

          year=htons(data_and_message[number_of_message2].tm_myyear2);
         send(s,&year,2,MSG_NOSIGNAL);


            one_byte[0]=char(data_and_message[number_of_message2].tm_myhour1);

            send(s,&one_byte[0],1,MSG_NOSIGNAL);

            one_byte[0]=char(data_and_message[number_of_message2].tm_myminutes1);

            send(s,&one_byte[0],1,MSG_NOSIGNAL);

            one_byte[0]=char(data_and_message[number_of_message2].tm_mysec1);

            send(s,&one_byte[0],1,MSG_NOSIGNAL);


            send(s,data_and_message[number_of_message2].message,strlen(data_and_message[number_of_message2].message)+1,MSG_NOSIGNAL);

            recv(s,ok,2,MSG_NOSIGNAL);
            fprintf(stdout,"%s\n",ok);

        }
}






