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
    #include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <fcntl.h>
#define LEN 400000
#define MX 20


struct addr_str
{
    struct sockaddr_in addr;
    char yetRecv[30];
    char count;
};
struct addr_str my_ar[10];



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

char yetRecv[MX] = {0};
int fl_stop = 1;

int count = 0;

int parse_and_print(struct addr_str my_ar_ind, unsigned short port, int cs, FILE* f, struct sockaddr_in addr)
{

    char buf[LEN];
    
    //memset(buf, 0, LEN);
    int len;
    socklen_t addr_len = sizeof(my_ar_ind.addr);
    #ifdef _WIN32
    int flags = 0;
    #else
    int flags = MSG_NOSIGNAL;
    #endif
    len = recvfrom(cs, buf, LEN, flags, (struct sockaddr*)&my_ar_ind.addr, &addr_len);
    if (len == -1)
        return 0;
    unsigned int ip = (unsigned int)ntohl(my_ar_ind.addr.sin_addr.s_addr);
    buf[len] = 0;
    int n_mes = (unsigned char)buf[3];
    n_mes = (unsigned char)buf[2] + (n_mes << 8);
    n_mes = (unsigned char)buf[1] + (n_mes << 8);
    n_mes = (unsigned char)buf[0] + (n_mes << 8);
    n_mes = ntohl(n_mes);
    if(my_ar_ind.yetRecv[n_mes%20]==1)
    {
        my_ar_ind.count++;
        return 0;
    }
    //printf("%d\n", n_mes);
    my_ar_ind.yetRecv[n_mes%20] = 1;    
    fprintf(f,"%d.%d.%d.%d:", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip) & 0xFF);
    //printf("%d.%d.%d.%d\n", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip) & 0xFF);
    fprintf(f, "%d ", port);
    int ii = 4;
    //AA
    /*
    len = recv(cs, buf, 2, 0);
    if (len == -1)
        return 0;
    buf[len] = 0;
    */
    uint16_t AA = (unsigned char)buf[0+ii];
    AA += (buf[1+ii] << 8);
    AA = ntohs(AA);
    fprintf(f, "%d ", AA);
    //printf("AA = %d\n", AA);

    //BBB
    //printf("BBB start\n");

    ii+=2;
    /*
    len = recv(cs, buf, 4, 0);
    if (len == -1)
        return 0;
    buf[len] = 0;
    //printf("BBB end\n");
    */
    int BBB = (unsigned char)buf[ii+3];
    BBB = (unsigned char)buf[ii+2] + (BBB << 8);
    BBB = (unsigned char)buf[ii+1] + (BBB << 8);
    BBB = (unsigned char)buf[ii+0] + (BBB << 8);
    BBB = ntohl(BBB);
    fprintf(f, "%d ", BBB);
    //printf("BBB = %d\n", BBB);

    ii+=4;
    //hh
    /*
    len = recv(cs, buf, 1, 0);
    if (len == -1)
        return 0;
    buf[len] = 0;
    */
    //char hh = atoi(buf[++ii]);// = buf[0];
    fprintf(f, "%.2d:", buf[ii++]);
    //printf("hh = %d\n", buf[0]);

    //mm
    /*
    len = recv(cs, buf, 1, 0);
    if (len == -1)
        return 0;
    buf[len] = 0;
    */
    //char mm = atoi(++buf);// = buf[0];
    fprintf(f, "%.2d:", buf[ii++]);
    //printf("mm = %d\n", buf[0]);

    //ss
    /*
    len = recv(cs, buf, 1, 0);
    if (len == -1)
        return 0;
    buf[len] = 0;
    */
    //char ss = atoi(buf[++ii]);// = buf[0];
    fprintf(f, "%.2d ", buf[ii++]);
    //printf("ss = %d\n", buf[0]);

    //N
    /*
    len = recv(cs, buf, 4, 0);
    if (len == -1)
        return 0;
    */
    int N = (unsigned char)buf[ii+3];
    N = (unsigned char)buf[ii+2] + (N << 8);
    N = (unsigned char)buf[ii+1] + (N << 8);
    N = (unsigned char)buf[ii+0] + (N << 8);
    N = ntohl(N);
    //printf("BBBstr = %s\n", buf);
    //printf("N = %d\n", N);
    ii+=4;
    //message
    int i = 0;

    while (i < N)
    {
        fprintf(f, "%c", buf[ii]);
        //printf("%c", buf[ii]);
        if(buf[ii] == 's' && i < N)
        {
            i++;
            ii++;
            fprintf(f, "%c", buf[ii]);
            //printf("%c", buf[ii]);
            if(buf[ii] == 't'&& i < N)
            {
                i++;
                ii++;
                fprintf(f, "%c", buf[ii]);
                //printf("%c", buf[ii]);
                if(buf[ii] == 'o'&& i < N)
                {
                    i++;
                    ii++;
                    fprintf(f, "%c", buf[ii]);
                    //printf("%c", buf[ii]);
                    if(buf[ii] == 'p'&& i < N)
                    {
                        
                        i++;
                        ii++;
                        fprintf(f, "\n");
                        if(i==N)
                        {
                            sendto(cs,buf, 4, flags,(struct sockaddr*)&my_ar_ind, addr_len);
                            fl_stop = 0;
                            return 0;
                        }
                    }
                }
            }
        }
        i++;
        ii++;
        
    }
    fprintf(f, "\n");
    //send(cs, "ok", 2, 0);
    my_ar_ind.count++;
    sendto(cs,buf, 4, flags,(struct sockaddr*)&my_ar_ind, addr_len);
    return 1;
}


int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        return sock_err("argument", 0);
    }


    int port1 = atoi(argv[1]);
    int port2 = atoi(argv[2]);
    int s[10];
    struct sockaddr_in addr_arr[10];
    //int* s = (int *)malloc( (port2 - port1)*sizeof(int));
    //struct sockaddr_in* addr_arr = (struct sockaddr_in*)malloc( (port2 - port1)*sizeof(struct sockaddr_in));
    struct sockaddr_in addr;
    int i;
    #ifdef _WIN32
    int flags = 0;
    #else
    int flags = MSG_NOSIGNAL;
    #endif
    // Инициалиазация сетевой библиотеки
    init();
    for(i = port1; i <= port2; i++)
    {
        // Создание UDP-сокета
        s[i - port1] = socket(AF_INET, SOCK_DGRAM, 0);
        if (s[i-port1] < 0)
            return sock_err("socket", s[i-port1]);
        // Заполнение структуры с адресом прослушивания узла
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(i); // Будет прослушиваться порт 8000
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(s[i - port1], (struct sockaddr*) &addr, sizeof(addr)) < 0)
            return sock_err("bind", *s);
        my_ar[i - port1].addr = addr;
        my_ar[i - port1].count = 0;
    }
    
   
    // Связь адреса и сокета, чтобы он мог принимать входящие дейтаграммы 
    int ls; // Сокет, прослушивающий соединения
    fd_set wfd, rfd;
    int nfds = s[0];
    i = 0;
    struct timeval tv = { 0, 100*1000 };

    FILE* f = fopen("msg.txt", "a+");
    if(f== NULL)
    {
        sock_err("file", s[0] );
    }
    do
    {
        FD_ZERO(&rfd);
        FD_ZERO(&wfd);
        
        for (i = 0; i <= port2 - port1 ; i++)
        {
            FD_SET(s[i], &rfd);
            FD_SET(s[i], &wfd);
            if (nfds < s[i])
                nfds = s[i];
        }
        if (select(nfds + 1, &rfd, &wfd, 0, &tv) > 0)
        {
            
            for (i = 0; i <= port2 - port1 ; i++)
            {
                if (FD_ISSET(s[i], &rfd))
                {
                    if(count%80==0)
                    {
                        my_ar[i].count = 0;
                        for(int j = 0; j<30;j++)
                        {
                            my_ar[i].yetRecv[j]=0;
                        }
                    }
                    if(parse_and_print(my_ar[i], i+port1,s[i], f, addr) == 0)
                    {
                       break; 
                    }
                    // Сокет cs[i] доступен для чтения. Функция recv вернет данные, recvfrom - дейтаграмму
                }
                if (FD_ISSET(s[i], &wfd))
                {
                    // Сокет cs[i] доступен для записи. Функция send и sendto будет успешно завершена
                }
            }
        }
        else
        {
            // Произошел таймаут или ошибка
        }
    } while (fl_stop);
    // Закрытие сокета
    for (i = 0; i <= port2 - port1 ; i++)
    {
        s_close(s[i]);
    }
    deinit();
    return 0;
}
