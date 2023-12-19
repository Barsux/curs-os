#include "ui_client.h"
#include "base.h"
#include <iso646.h>

#define RECV_INTERVAL 30

pthread_t client_thread;
bool serverRunning = false;

class Client{
    public:
    int sock, port, interval;
    bool running;
    ServerApp *app;
    sigc::signal<void, std::string> rx1;
    sigc::signal<void, std::string> rx2;
    sigc::signal<void, int> status;
    struct sockaddr_in addr;
    U8 rx[BUFF_SIZE];
    U8 tx[BUFF_SIZE];
    bool txReady = false;
    Client(int port, int interval, ServerApp *app){
        this->port = port;
        this->interval = interval;
        this->app = app;
        running = true;
        app->interrupt.connect(sigc::mem_fun(*this, &Client::interrupt));
        app->tx.connect(sigc::mem_fun(*this, &Client::perform_packet));
    
        this->rx1.connect(sigc::mem_fun(*app, &ServerApp::on_rx1_signal)); 
        this->rx2.connect(sigc::mem_fun(*app, &ServerApp::on_rx2_signal));
        this->status.connect(sigc::mem_fun(*app, &ServerApp::on_status_signal));
    }

    void interrupt(int signal){
        if(signal)this->running = false;
        print("Приказ выключится");
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
            std::string str = std::string(pack->data_first, pack->data_first + DATA_SIZE);
            rx1.emit(str);
        }
        else if(pack->flag == RECV_SECOND_DATA){
            printf("Пришёл пакет от %s\nЗапрос №2: %s\n", time, pack->data_second);
            std::string str = std::string(pack->data_second, pack->data_second + DATA_SIZE);
            rx2.emit(str);
        }
        else if(pack->flag == RECV_DATA){
            printf("Пришёл пакет от %s\nЗапрос №1: %s\nЗапрос №2: %s\n", time, pack->data_first, pack->data_second);
            std::string str = std::string(pack->data_first, pack->data_first + DATA_SIZE);
            rx1.emit(str);
            std::string str1 = std::string(pack->data_second, pack->data_second + DATA_SIZE);
            rx2.emit(str1);
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
        U16 server_stuck = 0;
        status.emit(0);
        while(1){
            if(!running) {
                status.emit(2);
                return 0;
            }
            if(server_stuck > 100000000){
                status.emit(1);
                return -1;
            }
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
            int st = select(sock+1, &read_fd_set, &write_fd_set, NULL, &timeout);
            if(st > 0 && FD_ISSET(sock, &read_fd_set)){
                int byteSize = recv(sock, rx, BUFF_SIZE, 0);
                if(byteSize == 0) continue;
                print("Пришёл пакет");
                message_handler(rx);
                server_stuck = 0;
            } else {
                server_stuck++;
            }
            if(txReady && st > 0 && FD_ISSET(sock, &write_fd_set)){
                int byteSize = send(sock, tx, BUFF_SIZE, 0);
                if (!byteSize) continue;
                print("Отправил");
                txReady = false;
                memset(tx, 0, BUFF_SIZE);
            }
        }
    }
};

void * client_thread_func(void * arg){
    threadInfo * ti = (threadInfo*)arg;
    print("Тут");
    Client client = Client(ti->port, ti->interval, ti->app);
    while(1){
        if(client.try_open() > 0){
            break;
        }
        client.status.emit(-1);
        usleep(5000000);
    }
    
    client.run();
    return NULL;
}



ServerApp::ServerApp()
{
    set_default_size(600, 300);

    // Connect button
    connect_button.set_label("Подключиться");
    connect_button.signal_clicked().connect(sigc::mem_fun(*this, &ServerApp::on_connect_button_clicked));

    // Get 1 button
    get1_button.set_label("Получить первый параметр.");
    get1_button.signal_clicked().connect([=](){ on_button_clicked(1); });

    // Get 2 button
    get2_button.set_label("Получить второй параметр.");
    get2_button.signal_clicked().connect([=](){ on_button_clicked(2); });
    get_button.set_label("Получить оба параметра.");
    get_button.signal_clicked().connect([=](){ on_button_clicked(3); });

    server1_checkbutton.signal_toggled().connect([=](){ on_toggle_server(1); });
    server2_checkbutton.signal_toggled().connect([=](){ on_toggle_server(2); });

    status_entry.set_text("Статус сервера.");
    status_entry.set_xalign(0);
    intervalEntry.set_placeholder_text("Интервал опроса.");
    // Text Entry 1

    server1Label.set_text("Сервер №1");
    server1Label.set_xalign(0);
    server2Label.set_text("Сервер №2");
    server2Label.set_xalign(0);
    notConnectedState();

    // Vertical Box Layout
    Gtk::Box* vBox = Gtk::manage(new Gtk::VBox());
    vBox->add(server1Label);
    vBox->add(server1_checkbutton);
    vBox->add(server2Label);
    vBox->add(server2_checkbutton);
    vBox->add(status_entry);
    vBox->add(connect_button);
    vBox->add(intervalEntry);
    vBox->add(get1_button);
    vBox->add(get2_button);
    vBox->add(get_button);
    vBox->add(data_entry1);
    vBox->add(data_entry2);

    add(*vBox);

    show_all_children();
}

void ServerApp::notConnectedState(){
    connect_button.set_label("Подключиться");
    get1_button.set_sensitive(false);
    get2_button.set_sensitive(false);
    get_button.set_sensitive(false);
    intervalEntry.set_sensitive(false);
    server1_checkbutton.set_sensitive(true);
    server2_checkbutton.set_sensitive(true);
    data_entry1.set_text("");
    data_entry2.set_text("");
}

void ServerApp::connectedState(){
    connect_button.set_label("Отключиться");
    get1_button.set_sensitive(true);
    get2_button.set_sensitive(true);
    get_button.set_sensitive(true);
    intervalEntry.set_sensitive(true);
    server1_checkbutton.set_sensitive(false);
    server2_checkbutton.set_sensitive(false);
}

ServerApp::~ServerApp()
{
    currServer = 1;
    serverRunning = false;
}

void ServerApp::on_toggle_server(int server){
    if(server == currServer) return;
    if(server == 1){
        if(server2_checkbutton.get_active()) server2_checkbutton.set_active(false);
    } else {
        if(server1_checkbutton.get_active()) server1_checkbutton.set_active(false);
    }
    currServer = server;
}

void ServerApp::on_connect_button_clicked()
{
    if(serverRunning){
        interrupt.emit(1);
        notConnectedState();
        return;
    }
    threadInfo ti;
    ti.port = PORT1;
    if(currServer == 2)ti.port = PORT2;
    std::string ival = intervalEntry.get_text();
    if(!ival.empty()){
        ti.interval = std::stoi(ival);
    }
    ti.app = this;
    pthread_create(&client_thread, NULL, client_thread_func, (void*)&ti);
}

void ServerApp::on_button_clicked(int numButton)
{
   if(numButton == 1)tx.emit(RECV_FIRST_DATA);
   if(numButton == 2)tx.emit(RECV_SECOND_DATA);
   if(numButton == 3)tx.emit(RECV_DATA);
}


void ServerApp::on_rx1_signal(std::string data){
    data_entry1.set_text(data);
}

void ServerApp::on_rx2_signal(std::string data){
    data_entry2.set_text(data);
}

void ServerApp::on_status_signal(int data){
    serverRunning = false;
    status_entry.set_text("");
    notConnectedState();
    if(data == -1){
        status_entry.set_text("Нет соединения. Попытка переподключится через 5 сек.");
    }
    else if(data == 0){
        char buff[128];
        snprintf(buff, 128, "Подключено к серверу: %d", currServer);
        status_entry.set_text(buff);
        serverRunning = true;
        connectedState();
    }
    else if(data == 1){
        status_entry.set_text("Потеря соединения... Переподключитесь.");
    }
    else{
        status_entry.set_text("Отключено.");
    }
}

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");
    ServerApp server_app;
    return app->run(server_app);
}
