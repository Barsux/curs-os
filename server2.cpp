#include "server2.h"
#include "base.h"
#include <pthread.h>
#define CHECKINTERVAL 1
#define data1(str) getMemoryUsagePercentage(str)
#define data2(str) getSwapMemoryUsagePercentage(str)

bool message_handler(U8 * msg, U8 * request, U8 * res_1, U8 * res_2){
    bool first_changed = false;
    bool second_changed = false;
    struct packet *pack = (struct packet*)msg;
    if(!pack->is_request || pack->timestamp != 0) return false;
    data1(pack->data_first);
    data2(pack->data_second);
    if(memcmp(res_1, pack->data_first, DATA_SIZE) != 0){
        if(pack->flag == RECV_FIRST_DATA || RECV_DATA) memcpy(res_1, pack->data_first, DATA_SIZE);
        first_changed = true;
    }
    if(memcmp(res_2, pack->data_second, DATA_SIZE) != 0){
        if(pack->flag == RECV_SECOND_DATA || RECV_DATA) memcpy(res_2, pack->data_second, DATA_SIZE);
        second_changed = true;
    }
    print("%d %d", first_changed, second_changed);
    if(pack->flag == RECV_FIRST_DATA && !first_changed) return false;
    else if(pack->flag == RECV_SECOND_DATA && !second_changed) return false;
    else if(pack->flag == RECV_DATA && !first_changed && !second_changed) return false;
    else if(pack->flag == RECV_DATA && !(first_changed && second_changed)){
        if(first_changed){
            pack->flag = RECV_FIRST_DATA;
            memset(pack->data_second, 0, DATA_SIZE);
        } else {
            pack->flag = RECV_SECOND_DATA;
            memset(pack->data_first, 0, DATA_SIZE);
        }
    }

    pack->timestamp = nanotime();
    pack->is_request = false;
    memcpy(request, msg, sizeof(struct packet));
    return true;
}

void perform_packet(U8 * buff, data_flags flag){
    struct packet * pck = (struct packet*)buff;
    pck->is_request = false;
    pck->flag = flag;
    memset(pck->data_first, 0, DATA_SIZE);
    memset(pck->data_second, 0, DATA_SIZE);
    data1(pck->data_first);
    data2(pck->data_second);
    if(flag == RECV_FIRST_DATA)memset(pck->data_second, 0, DATA_SIZE);
    if(flag == RECV_SECOND_DATA)memset(pck->data_first, 0, DATA_SIZE);
    pck->timestamp = nanotime();
}

void * client_listener(void * arg){
    U8 res_1[DATA_SIZE];
    U8 res_2[DATA_SIZE];
    U8 databuff[DATA_SIZE];
    memset(res_1, 0, DATA_SIZE);
    memset(res_2, 0, DATA_SIZE);
    memset(databuff, 0, DATA_SIZE);
    int sock = *(int*)arg;
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    U8 rx[BUFF_SIZE];
    U8 tx[BUFF_SIZE];
    bool needRequest = false;
    bool first_changed = true;
    bool second_changed = true;
    U64 interval = CHECKINTERVAL * 1000000000;
    U64 event = nanotime() + interval;
    while(1){
        fd_set read_fd_set, write_fd_set;
        FD_ZERO(&read_fd_set);
        FD_ZERO(&write_fd_set);
        FD_SET((unsigned int)sock, &read_fd_set);
        FD_SET((unsigned int)sock, &write_fd_set);
        int status = select(sock+1, &read_fd_set, &write_fd_set, NULL, &timeout);
        if(status > 0 && FD_ISSET(sock, &read_fd_set)){
            int byteSize = recv(sock, rx, BUFF_SIZE, 0);
            if (!byteSize) continue;
            needRequest = message_handler(rx, tx, res_1, res_2);
        }
        if(needRequest && status > 0 && FD_ISSET(sock, &write_fd_set)){
            int byteSize = send(sock, tx, BUFF_SIZE, 0);
            if (!byteSize) continue;
            
            needRequest = false;
            print("Отправил пакет...");
            memset(tx, 0, BUFF_SIZE);
        }
        if(nanotime() < event) continue;
        event = nanotime() + interval;
        memset(databuff, 0, DATA_SIZE);
        data1(databuff);
        if(memcmp(res_1, databuff, DATA_SIZE) != 0){
            memcpy(res_1, databuff, DATA_SIZE);
            first_changed = true;
        }
        memset(databuff, 0, DATA_SIZE);
        data2(databuff);
        if(memcmp(res_2, databuff, DATA_SIZE) != 0){
            memcpy(res_2, databuff, DATA_SIZE);
            second_changed = true;
        }
        if(first_changed || second_changed){
            if(first_changed && second_changed){
                perform_packet(tx, RECV_DATA);
            } else if(first_changed){
                perform_packet(tx, RECV_FIRST_DATA);
            } else {
                perform_packet(tx, RECV_SECOND_DATA);
            }
            first_changed = false;
            second_changed = false;
            needRequest = true;
        }
    }
}

class Server{
    public:
    int sock, listener;
    struct sockaddr_in addr;
    Server(){

    }
    int try_open(){
        int status = 0;
        // Сокет для входных подключений
        listener = socket(AF_INET, SOCK_STREAM, 0);
        if(listener < 0){
            perror("Сокет не открылся, расходимся.");
            return -1;
        }

        //Заполняем структуру
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT2);
        addr.sin_addr.s_addr = INADDR_ANY;

        status = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
        if(status < 0){
            perror("Нет подключения к интерфейсу.");
            return -1;
        }

        listen(listener, DEBUG_LISTENERS);
        return 1;
    }

    

    int run(){
        pthread_t tid[MAX_LISTENERS];
        int threadptr = 0;
        print("Сервер №2 запущен! Ожидание клиентов.");
        while(1){
            sock = accept(listener, NULL, NULL);
            if(sock < 0){
                perror("Сокет не создался.");
                return -1;
            }
            print("Клиент подключился.");
            pthread_create(&tid[threadptr], NULL, client_listener, (void *)&sock);
            threadptr++;
            if(threadptr > MAX_LISTENERS){
                threadptr = 0;
                while(threadptr < MAX_LISTENERS){
                    pthread_join(tid[threadptr], NULL);
                    threadptr++;
                }
                threadptr = 0;
            }

        }
    }
};


int main(){
    Server * server = new Server();
    if(server->try_open() < 0){
        print("Ошибка инициализации, выход...");
        exit(EXIT_FAILURE);
    }
    if(server->run() < 0){
        print("Ошибка исполнения, выход...");
        exit(EXIT_FAILURE);
    }
}