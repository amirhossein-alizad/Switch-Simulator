#include "Manager.hpp"
#include "System.hpp"
#include "Switch.hpp"

using namespace std;

constexpr char ADD_SYSTEM[] = "MySystem";
constexpr char ADD_SWITCH[] = "MySwitch";
constexpr char CONNECT_SYSTEM_SWITCH[] = "connect_sy_sw";
constexpr char CONNECT_SWITCH_SWITCH[] = "connect_sw_sw";
constexpr char PING[] = "ping";
constexpr char REQUEST[] = "req";
constexpr char SPANNING_TREE[] = "sptree";

void Manager::handleCommand() {
    string command;
    const int command_index = 0;
    while (getline(cin, command)) {
        vector<string> arguments = tokenizeInput(command);

        if(arguments[command_index] == ADD_SYSTEM) {
            // MySystem <id>
            if(arguments.size() != 2) {
                cout << "too few or too many arguments!\n";
                continue;
            }
            addSystem(stoi(arguments[1]));
        } else if(arguments[command_index] == ADD_SWITCH) {
            // MySwitch <portnum> <id>
            if(arguments.size() != 3) {
                cout << "too few or too many arguments!\n";
                continue;
            }
            addSwitch(stoi(arguments[1]), stoi(arguments[2]));
        } else if(arguments[command_index] == CONNECT_SYSTEM_SWITCH) {
            // connect_sy_sw <system_id> <switch_id> <port>
            if(arguments.size() != 4) {
                cout << "too few or too many arguments!\n";
                continue;
            }
            connect_sy_sw(stoi(arguments[1]), stoi(arguments[2]), stoi(arguments[3]));
        } else if(arguments[command_index] == CONNECT_SWITCH_SWITCH) {
            // connect_sw_sw <switch_id1> <port_id1> <switch_id2> <port_id2>
            if(arguments.size() != 5) {
                cout << "too few or too many arguments!\n";
                continue;
            }
            connect_sw_sw(stoi(arguments[1]), stoi(arguments[2]), stoi(arguments[3]), stoi(arguments[4]));   
        } else if(arguments[command_index] == PING) {
            // ping <system_id1> <system_id2>
            if(arguments.size() != 3) {
                cout << "too few or too many arguments!\n";
                continue;
            }
            ping(stoi(arguments[1]), stoi(arguments[2]));
        } else if(arguments[command_index] == REQUEST) {
            // request <from_systemid> <to_systemid> <filename>
            if(arguments.size() != 4) {
                cout << "too few or too many arguments!\n";
                continue;
            }
            requestFile(stoi(arguments[1]), stoi(arguments[2]), arguments[3]);
        } else if(arguments[command_index] == SPANNING_TREE) {
            // sptree
            if(arguments.size() != 1) {
                cout << "too few or too many arguments!\n";
                continue;
            }
            spanningTree();
        } else {
            cout << "Unknown command!" << endl;
        }

        sleep(2);
    }
    sleep(5);
}

void Manager::addSwitch(int numOfPorts, int id) {
    if(find_switch_index(id) != -1) {
        cout << "switch with this id already exists!\n";
        return;
    }
    
    SwitchInfo* new_si = new SwitchInfo;
    Switch* new_switch = new Switch(numOfPorts, id);
    if(pipe(new_si->pipes) < 0)
        cout << "can't make unnamed pipes for switch " << id << endl;
    new_si->sw = new_switch;
    switches.push_back(new_si);
    cout << "Switch created!\n";
    if(fork() == 0)
        new_switch->run(new_si->pipes[0]);
}

void Manager::addSystem(int id) {
    if(find_system_index(id) != -1) {
        cout << "system with this id already exists!\n";
        return;
    }

    SystemInfo* new_sy = new SystemInfo;
    System* new_system = new System(id);
    if(pipe(new_sy->pipes) < 0)
        cout << "can't make unnamed pipes for system " << id << endl;
    new_sy->sy = new_system;
    systems.push_back(new_sy);
    cout << "System created!\n";
    if(fork() == 0)
        new_system->run(new_sy->pipes[0]);
}

int Manager::find_system_index(int id) {
    int i;
    for(i = 0; i < systems.size(); i++)
        if(systems[i]->sy->getID() == id)
            return i;
    return -1;
}

int Manager::find_switch_index(int id) {
    int i;
    for(i = 0; i < switches.size(); i++)
        if(switches[i]->sw->getID() == id)
            return i;
    return -1;
}

void Manager::connect_sy_sw(int system_id, int switch_id, int port) {
    if(find_system_index(system_id) == -1) {
        cout << "system doesn't exist!\n";
        return;
    }
    if(find_switch_index(switch_id) == -1) {
        cout << "switch doesn't exist!\n";
        return;
    }
    if(switches[find_switch_index(switch_id)]->sw->getNumOfPorts() <= port) {
        cout << "this switch doesn't have a port with this number\n";
        return;
    }
    cout << "connecting system " << system_id << " to switch " << switch_id << " from port " << port << endl;

    string switch_pipe = "switch" + to_string(switch_id) + "/port" + to_string(port);
    string system_pipe = "system" + to_string(system_id) + "/input";
    string msg_to_system = "connect " + switch_pipe;
    string msg_to_switch = "connect " + to_string(port) + " " + system_pipe;

    write(systems[find_system_index(system_id)]->pipes[1], msg_to_system.c_str(), msg_to_switch.length()+1);
    write(switches[find_switch_index(switch_id)]->pipes[1], msg_to_switch.c_str(), msg_to_switch.length()+1);
}

void Manager::connect_sw_sw(int switch_id1, int port_id1, int switch_id2, int port_id2) {
    if( (find_switch_index(switch_id1) == -1) || (find_switch_index(switch_id2) == -1) ) {
        cout << "switch doesn't exist!\n";
        return;
    }
    if( (switches[find_switch_index(switch_id1)]->sw->getNumOfPorts() <= port_id1) || (switches[find_switch_index(switch_id2)]->sw->getNumOfPorts() <= port_id2) ) {
        cout << "this switch doesn't have a port with this number\n";
        return;
    }
    cout << "connecting switch " << switch_id1 << " from prot " << port_id1 << " to swtich " << switch_id2 << " from port " << port_id2 << endl;

    string switch1_pipe = "switch" + to_string(switch_id1) + "/port" + to_string(port_id1);
    string switch2_pipe = "switch" + to_string(switch_id2) + "/port" + to_string(port_id2);
    string msg_to_switch1 = "connect " + to_string(port_id1) + " " + switch2_pipe;
    string msg_to_switch2 = "connect " + to_string(port_id2) + " " + switch1_pipe;

    write(switches[find_switch_index(switch_id1)]->pipes[1], msg_to_switch1.c_str(), msg_to_switch1.length()+1);
    write(switches[find_switch_index(switch_id2)]->pipes[1], msg_to_switch2.c_str(), msg_to_switch2.length()+1);
}

void Manager::ping(int from, int to) {
    if((find_system_index(from) == -1) || (find_system_index(to) == -1)) {
        cout << "system with this id doesn't exists!\n";
        return;
    }
    cout << "pinging!" << endl;

    string command = "ping " + to_string(to);
    write(systems[find_system_index(from)]->pipes[1], command.c_str(), command.length()+1);
}

void Manager::requestFile(int from, int to, string filename) {
    if((find_system_index(from) == -1) || (find_system_index(to) == -1)) {
        cout << "system with this id doesn't exists!\n";
        return;
    }
    cout << "requesting!" << endl;

    string command = "request " + to_string(to) + " " + filename;
    write(systems[find_system_index(from)]->pipes[1], command.c_str(), command.length()+1);
}

void Manager::spanningTree() {
    cout << "sending signal to all the swithces!" << endl;
    for(int i = 0; i < switches.size(); i++) {
        string command = "spanning_tree";
        write(switches[i]->pipes[1], command.c_str(), command.length()+1);
    }
}

vector<string> Manager::tokenizeInput(string input) {
    stringstream inputStringStream(input);
    return vector<string>(istream_iterator<string>(inputStringStream),
                          istream_iterator<string>());
}