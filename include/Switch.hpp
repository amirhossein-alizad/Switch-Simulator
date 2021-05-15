#ifndef HEADER_SWITCH
#define HEADER_SWITCH

#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <fstream>
#include <sstream>
#include <iterator>

class Frame;

constexpr int DISCONNECTED = 0;
constexpr int INACTIVE = 1;
constexpr int ACTIVE  = 2;

const std::string STATUS[] = {"disconnected",   // nothing is connected to this port
                              "inactive",       // sth is connected to this port but it isn't active becuase of spaning tree algorithem
                              "active"};        // sth is connected to this port and comminucation is active

typedef struct port {
    int status;
    int input_pipe_fd;
    int output_pipe_fd;
} Port;

typedef struct STPinfo {
    int root_id;
    int distance_to_root;
    int next_switch_to_root;
    int port_to_root;
} STPinfo;

class Switch {
    private:
        int id;
        int numOfPorts;
        int MASSAGE_SIZE;
        std::ofstream log;
        std::ofstream STPlog;
        std::string directory;
        STPinfo* stp_info;
        std::map<int, Port*> ports;
        std::map<int, int> lookup_table;
        // fd_set readFDs;
        void handleManagerCommand(int read_fd_pipe);
        void handleInputFrame(int port_num, int pipe_fd);
        void initiatePipes();
        std::vector<std::string> tokenizeInput(std::string input);
        void printPortStatus(int port);
        void printLookuptable();
        void handleSTPframe(Frame frame, int port_num);
        std::string getSTPimpression();
        void printSTPinfo();
    public:
        int getID();
        int getNumOfPorts();
        Switch(int numOfPorts, int id);
        void run(int read_fd_pipe);
};

#endif