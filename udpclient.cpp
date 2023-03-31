#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>



#define LEN 400000
#define MAXX 20


// Функция извлекает IPv4-адрес из DNS-дейтаграммы.
// Задание л/р не требует детального изучения кода этой функции 



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

void s_close(int s)
{
#ifdef _WIN32
	closesocket(s);
#else
	close(s);
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

struct mes
{
    unsigned short AA;
    int BBB;
    unsigned char hh;
    unsigned char mm;
    unsigned char ss;
    unsigned int N;
    char* text;
    unsigned int numb;
};

struct mes messages[MAXX];
char sendMesYet[MAXX] = { 0 };
union num
{
    int i;
    char arr[4];
};
int count2=0;
int recv_response(int s)
{
    char datagram[80];
    struct timeval tv = { 0,100 * 1000 };
    fd_set fds;
    FD_ZERO(&fds);  
    FD_SET(s, &fds); 
    int res = select(s + 1, &fds, 0, 0, &tv);
    int I, j;
    if (res > 0)
    {
        struct sockaddr_in addr;
        int addrlen = sizeof(addr);
        int received = recvfrom(s, datagram, 
            sizeof(datagram), 0, (struct sockaddr*)&addr, &addrlen);
        if (received <= 0)
        {
            sock_err("recvfrom", s);
            return 0;
        }
        
            memcpy(&I, datagram, 4);
            sendMesYet[ntohl(I)] = 1;
            count2++;
        
        
    }
    else if (res == 0)
    {
        return 0;
    }
    else
    {
        sock_err("select", s);
        return 0;
    }
}


#ifdef _WIN32
int flags = 0;
#else
int flags = MSG_NOSIGNAL;
#endif


int my_sent(int s, sockaddr_in* addr, int I)
{
    int j = strlen(messages[I].text);
    char* ar = (char*)malloc(sizeof(messages[I].AA) + sizeof(messages[I].BBB) + sizeof(messages[I].hh) + sizeof(messages[I].mm) +
        sizeof(messages[I].ss) + sizeof(messages[I].N) + j + sizeof(messages[I].numb));
    int ii = 0;
    //numb

    ar[ii + 3] = messages[I].numb & 0xFF;
    ar[ii + 2] = messages[I].numb >> 8;
    ar[ii + 1] = messages[I].numb >> 16;
    ar[ii + 0] = messages[I].numb >> 24;

    ii = 4;
    //AA
    
    ar[ii + 1] = messages[I].AA & 0xFF;
    ar[ii + 0] = (messages[I].AA >> 8) & 0xFF;
    //printf("ar[1] = %d ", (ar[ii + 1]));
    //printf("ar[0] = %d\n", (ar[ii + 0]));
    //printf("a = %d\n", ((ar[ii + 1])<<8)+ ar[ii + 0]);
    ii += 2;
    //BBB
    ar[ii + 3] = messages[I].BBB & 0xFF;
    ar[ii + 2] = messages[I].BBB >> 8;
    ar[ii + 1] = messages[I].BBB >> 16;
    ar[ii + 0] = messages[I].BBB >> 24;
    ii += 4;
    //hh mm ss
    ar[ii + 0] = messages[I].hh;
    ar[ii + 1] = messages[I].mm;
    ar[ii + 2] = messages[I].ss;
    ii += 3;
    //N
    ar[ii + 3] = messages[I].N & 0xFF;
    ar[ii + 2] = messages[I].N >> 8;
    ar[ii + 1] = messages[I].N >> 16;
    ar[ii + 0] = messages[I].N >> 24;

    ii += 4;
    //text
    int k = 0;
    for (k = 0; k < j; k++)
    {
        ar[ii++] = messages[I].text[k];
    }
    
    int len = sendto(s, ar, ii, flags, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
    if (len == -1)
    {
        sock_err("sendto", s);
        return 0;
    }
    
    
    
}

void slh_n(int s, union num numb, int flags, sockaddr_in* addr)// string number
{
    numb.i = htonl(numb.i);
    sendto(s, numb.arr, 4, flags, (sockaddr*)addr, sizeof(sockaddr_in));
}

int I = 0;
int KOL = 0;
int send_request(FILE* f, int s, char* filename, sockaddr_in* addr, bool FL, int kkk)
{
    int i = 0;
    if (FL)
    {
        for (i = 0; i < I; i++)
        {
            if (sendMesYet[i] == 0)
            {
                my_sent(s, addr, i);
            }
        }
        return 1;
    }

    union num numb;
    union num BBB, N;
    numb.i = 0;


    
    
    
    char message[LEN];
    char count = 0;
    char buf[20];
    unsigned int tmp;
    N.i = 0;
    int j = 0;
    unsigned int len;
    //slh_n(s, numb, flags, addr);
    while ((i = fgetc(f)) != EOF)
    {
        if ((i != ' ' && count < 5) && ((count < 2 && count>4) || i != ':') && i != '\n')
        {
            buf[j] = i;
            j++;

        }
        else
        {
            //printf("count = %d\n", count);
            if (i == '\n')
                continue;            
            if (count == 0)// AA
            {
                
                buf[j] = 0;

                tmp = atoi(buf);
                
                messages[I].AA = tmp;
                count++;
                j = 0;
            }
            else if (count == 1)// BBB
            {
                buf[j] = 0;
                messages[I].BBB = atoi(buf);
                //slh_n(s, BBB, flags, addr);
                j = 0;
                count++;
            }
            else if (count >= 2 && count <= 4)
            {
                buf[j] = 0;
                buf[0] = atoi(buf);
                if (count == 2)
                {
                    messages[I].hh = buf[0];
                }
                else if (count == 3)
                {
                    messages[I].mm = buf[0];
                }
                else 
                {
                    messages[I].ss = buf[0];
                }
                //sendto(s, &buf[0], 1, flags, (sockaddr*)addr, sizeof(sockaddr_in));
                j = 0;
                count++;
            }
            else if (count == 5)
            {
                N.i = 0;
                j = 0;
                message[j++] = i;
                
                while ((i = fgetc(f)) != '\n' && i != EOF)
                {
                    message[j++] = i;
                }
                message[j] = 0;
                char* str = (char*)malloc(j);
                memcpy(str, message, j);
                str[j] = 0;
                N.i = j;
                messages[I].N = j;
                messages[I].text = str;
                messages[I].numb = numb.i+(kkk*20);
                my_sent(s, addr, I);
                I++;
                printf("message sent\n");
                KOL = numb.i + 1;
                if (!strcmp(message, "stop"))
                {
                    fclose(f);
                    return 0;
                }

                count = 0;
                j = 0;
                numb.i++;
                if (numb.i % 20 == 0)
                {
                    return 2; 
                }
                //slh_n(s, numb, flags, addr);
            }
        }

    }
    fclose(f);
    return 0;
}










int main(int argc, char** argv)
{
    char str_ipaddr[20];
    char str_port[10];
    char* str_filename = NULL;

    if (argc != 3)
    {
        return sock_err("arguments", 0);
    }
    int i = 0;
    while ((argv[1][i] != ':') && (argv[1][i] != ' ') && (argv[1][i] != 0))
    {
        str_ipaddr[i] = argv[1][i];
        i++;
    }
    str_ipaddr[i] = 0;
    i++;
    int j = 0;
    while (argv[1][i] != ':' && argv[1][i] != ' ' && argv[1][i] != 0)
    {
        str_port[j] = argv[1][i];
        i++;
        j++;
    }
    str_port[j] = 0;
    str_filename = argv[2];


    int s;
    struct sockaddr_in addr;
    // Инициалиазация сетевой библиотеки
    init();
    // Создание UDP-сокета
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
        return sock_err("socket", s);
    // Заполнение структуры с адресом удаленного узла
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(str_port)); // Порт DNS - 53
    addr.sin_addr.s_addr = inet_addr(str_ipaddr);
    int ret = 0;

    FILE* f = fopen(str_filename, "r");
    if (f == NULL)
    {
        return sock_err("file", s);
    }
    int kkk = 0;
    while (1)
    {
        ret = send_request(f, s, str_filename, &addr, 0, kkk);
        recv_response(s);
        while (count2 != I && I!=0)
        {
            send_request(f, s, str_filename, &addr, 1, kkk);
            recv_response(s);
        }
        if (ret == 2)
        {
            I = 0;
            count2 = 0;
            kkk++;
        }
        if (ret == 0)
        {
            
            break;
        }
        for (i = 0; i < MAXX; i++)
        {
            sendMesYet[i] = 0;
        }
    }

	// Закрытие сокета
	s_close(s);
	deinit();
	return 0;
}
