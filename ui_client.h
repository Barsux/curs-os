#ifndef UICLI
#define UICLI

#include "base.h"
#include <gtkmm.h>

class ServerApp : public Gtk::Window
{
public:
    ServerApp();
    virtual ~ServerApp();
    sigc::signal<void, data_flags> tx;
    sigc::signal<void, int> interrupt;
    void on_rx1_signal(std::string data);
    void on_rx2_signal(std::string data);
    void on_status_signal(int data);

protected:
    int currServer;
    // Signal handlers
    void on_connect_button_clicked();
    void on_button_clicked(int numButton);
    void on_toggle_server(int server);
    void notConnectedState();
    void connectedState();

    // Member widgets
    Gtk::CheckButton server1_checkbutton;
    Gtk::CheckButton server2_checkbutton;
    Gtk::Button connect_button;
    Gtk::Button get1_button;
    Gtk::Button get2_button;
    Gtk::Button get_button;
    Gtk::Label data_entry1;
    Gtk::Label data_entry2;
    Gtk::Label status_entry;
    Gtk::Label server1Label;
    Gtk::Label server2Label;
    Gtk::Entry intervalEntry;
};

typedef struct{
    int port;
    int interval;
    ServerApp *app;
} threadInfo;

#endif