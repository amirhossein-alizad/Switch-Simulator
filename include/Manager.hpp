#ifndef HEADER_MANAGER
#define HEADER_MANAGER

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iterator>
#include <unistd.h>
#include <map>

class Switch;
class System;

typedef struct SwtichInfo {
    Switch* sw;
    int pipes[2];
} SwitchInfo;

typedef struct SystemInfo {
    System* sy;
    int pipes[2];
} SystemInfo;

class Manager {
    private:
        std::vector<SwitchInfo*> switches;
        std::vector<SystemInfo*> systems;
        std::vector<std::string> tokenizeInput(std::string input);
        void addSwitch(int numOfPorts, int id);
        void addSystem(int id);
        void connect_sy_sw(int system_id, int switch_id, int port);
        void connect_sw_sw(int switch_id1, int port_id1, int switch_id2, int port_id2);
        int find_system_index(int id);
        int find_switch_index(int id);
        void ping(int from, int to);
        void requestFile(int from, int to, std::string filename);
        void spanningTree();
    public:
        void handleCommand();
};

#endif