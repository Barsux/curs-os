#include "client.h"
#include "base.h"
#include <cstdlib>
#include <netinet/in.h>
#include <sys/socket.h>

class Client{
    public:
    int sock, port;
    struct sockaddr_in addr;
    Client(int port){
        this->port = port;
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
        str2ip4("127.0.0.1", addr.sin_addr.s_addr);

        status = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        if(status < 0){
            perror("Не удалось подключиться к серверу.");
            return -1;
        }

    }
};

int main(){
    // TODO! Добавить выбор порта, а следственно и сервера.
    Client * client = new Client(PORT1);
    if(client->try_open() < 0){
        printf("Ошибка подключения, выход...\n");
        exit(EXIT_FAILURE);
    }
    
}