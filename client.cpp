#include "client.h"

#define RECV_INTERVAL 30

class Client{
    public:
    int sock, port;
    struct sockaddr_in addr;
    U8 rx[BUFF_SIZE];
    U8 tx[BUFF_SIZE];
    bool txReady = false;
    Client(int port){
        this->port = port;
    }
    
    bool message_handler(U8 * msg){
        struct packet *pack = (struct packet*)msg;
        if(pack->is_request || pack->timestamp == 0){
            print("%s", pack->data_first);
            return false;
        } 
        char time[64];
        memset(time, 0, 64);
        utc2str(time, 64, pack->timestamp);
        print("Обрабатываю");
        if(pack->flag == RECV_FIRST_DATA){
            printf("Пришёл пакет от %s\nЗапрос №1: %s\n", time, pack->data_first);
        }
        else if(pack->flag == RECV_SECOND_DATA){
            printf("Пришёл пакет от %s\nЗапрос №2: %s\n", time, pack->data_second);
        }
        else if(pack->flag == RECV_DATA){
            printf("Пришёл пакет от %s\nЗапрос №1: %s\nЗапрос №2: %s\n", time, pack->data_first, pack->data_second);
        }

    return true;
    }

    int try_open(){
        int status = 0;
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock < 0){
            perror("Сокет не создался, расходимся: ");
            return -1;
        }
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(IP);
        print("Инициализация прошла успешно, подключаюсь к серверу...\n");
        status = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        if(status < 0){
            perror("Не удалось подключиться к серверу.NULL");
            return -1;
        }
        return 1;
    }

    void perform_packet(data_flags flag){
        struct packet * pck = (struct packet*)tx;
        pck->timestamp = 0;
        pck->is_request = true;
        pck->flag = flag;
        memset(pck->data_first, 0, DATA_SIZE);
        memset(pck->data_second, 0, DATA_SIZE);
        txReady = true;
    }

    int run(){
        struct timeval timeout;
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        memset(tx, 0, BUFF_SIZE);
        memset(rx, 0, BUFF_SIZE);
        U64 interval = (U64)RECV_INTERVAL * 1000000000;
        U64 trigger = nanotime() + interval;
        while(1){
            U64 ts = nanotime();
            if(ts >= trigger){
                perform_packet(RECV_DATA);
                trigger = ts + interval;
            }
            fd_set read_fd_set, write_fd_set;
            FD_ZERO(&read_fd_set);
            FD_ZERO(&write_fd_set);
            FD_SET((unsigned int)sock, &read_fd_set);
            FD_SET((unsigned int)sock, &write_fd_set);
            int status = select(sock+1, &read_fd_set, &write_fd_set, NULL, &timeout);
            if(status > 0 && FD_ISSET(sock, &read_fd_set)){
                int byteSize = recv(sock, rx, BUFF_SIZE, 0);
                if (!byteSize) continue;
                print("Пришёл пакет");
                message_handler(rx);
            }
            if(txReady && status > 0 && FD_ISSET(sock, &write_fd_set)){
                int byteSize = send(sock, tx, BUFF_SIZE, 0);
                if (!byteSize) continue;
                print("Отправил");
                txReady = false;
                memset(tx, 0, BUFF_SIZE);
            }
        }
    }
};

int main(){
    // TODO! Добавить выбор порта, а следственно и сервера.
    Client client = Client(PORT2);
    if(client.try_open() < 0){
        print("Ошибка подключения, выход...\n");
        exit(EXIT_FAILURE);
    }
    print("Подключено!\n");
    client.run();
    
}