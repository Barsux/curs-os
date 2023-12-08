#include "server2.h"


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
        printf("Сервер №2 запущен! Ожидание клиентов.\n");
        while(1){
            sock = accept(listener, NULL, NULL);
            if(sock < 0){
                perror("Сокет не создался.");
                return -1;
            }
            /*

                Здесь суета с многопоточностью.

            */

        }
    }
};

int main(){
    Server * server = new Server();
    if(server->try_open() < 0){
        printf("Ошибка инициализации, выход...\n");
        exit(EXIT_FAILURE);
    }
    if(server->run() < 0){
        printf("Ошибка исполнения, выход...\n");
        exit(EXIT_FAILURE);
    }
}