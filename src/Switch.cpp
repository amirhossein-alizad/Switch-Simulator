#include "Switch.hpp"
#include "Frame.hpp"

using namespace std;

constexpr char CONNECT[] = "connect";
constexpr char SPANNING_TREE[] = "spanning_tree";

Switch::Switch(int numOfPorts, int id) {
    this->MASSAGE_SIZE = 59;
    this->id = id;
    this->numOfPorts = numOfPorts;
    this->directory = "switch" + to_string(id);
    for(int i = 0; i < numOfPorts; i++) {
        Port* p = new Port;
        p->status = DISCONNECTED;
        p->input_pipe_fd = -1;
        p->output_pipe_fd = -1;
        this->ports[i] = p;
    }
    this->stp_info = new STPinfo;
    this->stp_info->root_id = id;
    this->stp_info->distance_to_root = 0;
    this->stp_info->port_to_root = -1;
    this->stp_info->next_switch_to_root = -1;
}

int Switch::getID() { return id; }

int Switch::getNumOfPorts() { return numOfPorts; }

void Switch::initiatePipes() {
    if(mkdir(directory.c_str(), 0777) == -1)
        cout << "Switch " << id << " can't make its directory!\n";
    
    this->log.open(directory+"/log.txt", std::ios_base::app);
    this->STPlog.open("STPlog.txt", std::ios::app);
    if(this->STPlog.is_open() == false) {
        cout << "switch " << id << " can't open STPlog file!\n";
    }

    for(int i = 0; i < numOfPorts; i++) {
        string pipe_name = directory + "/port" + to_string(i);
        if(mkfifo(pipe_name.c_str(), 0666) != 0)
            cout << "failed to make pipe for switch " << id << endl;
        int temp = open(pipe_name.c_str(), O_RDONLY | O_NONBLOCK);
        if (temp < 0)
            cout << "faild to open the pipe for switch " << id << endl; 
        this->ports[i]->input_pipe_fd = temp;

        this->log << "switch " << id << " opened: " << this->ports[i]->input_pipe_fd << endl;
    }
}

void Switch::run(int read_fd_pipe) {
    initiatePipes();
    
    fd_set inputs;
    int max_fd = 0;
    while (1) {
        FD_ZERO(&inputs);
        FD_SET(read_fd_pipe, &inputs);
        for(int i = 0; i < numOfPorts; i++){
            if(this->ports[i]->status != ACTIVE)
                continue;
            FD_SET(this->ports[i]->input_pipe_fd, &inputs);
            if(this->ports[i]->input_pipe_fd > max_fd)
                max_fd = this->ports[i]->input_pipe_fd;
        }
        if(select(1000, &inputs, NULL, NULL, NULL) < 0)
            cout<<"select error in switch " << id <<endl;

        if(FD_ISSET(read_fd_pipe, &inputs)) {
            handleManagerCommand(read_fd_pipe);
        } else {
            for(int i = 0; i < numOfPorts; i++) {
                if(FD_ISSET(this->ports[i]->input_pipe_fd, &inputs)) {
                    handleInputFrame(i, this->ports[i]->input_pipe_fd);
                }
            }
        }
    }
}

void Switch::printPortStatus(int port) {
    if(port >= numOfPorts) {
        this->log << "port " << port << " doesn't exists!\n";
        return;
    }

    this->log << "Port " << port << " status " << endl;
    this->log <<"\tstatus: " << STATUS[this->ports[port]->status] << endl;
    this->log <<"\tinput: " << this->ports[port]->input_pipe_fd << endl;
    this->log <<"\toutput: " << this->ports[port]->output_pipe_fd << endl;
    this->log << endl;
}

void Switch::printLookuptable() {
    this->log << "Lookup Table:" << endl;
    for(auto it = lookup_table.begin(); it != lookup_table.end(); it++)
        this->log << "\t" << it->first << "\t" << it->second << endl;
    this->log << endl;
}

void Switch::printSTPinfo() {
    this->log << "STP current info: " << endl;
    this->log << "\tself_id: " << id << endl;
    this->log << "\troot_id: " << stp_info->root_id << endl; 
    this->log << "\tdistance_to_root: " << stp_info->distance_to_root << endl;
    this->log << "\tnext_switch_to_root: " << stp_info->next_switch_to_root << endl; 
    this->log << "\tport_to_root: " << stp_info->port_to_root << endl;
    this->log << endl;
}

void Switch::handleManagerCommand(int read_fd_pipe) {
    char massage[MASSAGE_SIZE];
    read(read_fd_pipe, massage, MASSAGE_SIZE);
    this->log << "incomming massage from manager: " << massage << endl << endl;

    vector<string> arguments = tokenizeInput(string(massage));
    if(arguments[0] == CONNECT) {
        int port = stoi(arguments[1]);
        if(this->ports[port]->status == ACTIVE) {
            cout << "port " << port << " of switch " << id << " is already connected: connection failed!" << endl;
            return;
        }

        int write_fd = open(arguments[2].c_str(), O_WRONLY | O_NONBLOCK);
        if(write_fd < 0) {
            cout << "switch " << id << " can't open pipe to write to!\n";
            return;
        }
        this->ports[port]->output_pipe_fd = write_fd;
        this->ports[port]->status = ACTIVE;

        printPortStatus(port);

    } else if(arguments[0] == SPANNING_TREE) {
        this->STPlog << "initiating the spannig tree algorithem for swtich " << id << endl;
        string content = getSTPimpression();
        Frame f = Frame(0, 0, STP, content);
        printSTPinfo();
        for(int i = 0; i < numOfPorts; i++) {
            if(this->ports[i]->status == ACTIVE)
                write(this->ports[i]->output_pipe_fd, f.toString().c_str(), f.toString().length()+1);
        }
    }
}

string Switch::getSTPimpression() {
    string content = to_string(this->id) + " " + to_string(this->stp_info->root_id) + " " + to_string(this->stp_info->distance_to_root);
    return content;
}

void Switch::handleInputFrame(int port_num, int pipe_fd) {
    char massage[MASSAGE_SIZE];
    read(pipe_fd, massage, MASSAGE_SIZE);
    Frame incomming_frame = Frame(string(massage));
    this->log << "incoming frame from port " << port_num << " : " << massage << endl << endl;

    if(incomming_frame.getType() == STP) {
        handleSTPframe(incomming_frame, port_num);
        return;
    }

    int from_id = incomming_frame.getFrom();
    if(lookup_table.find(from_id) == lookup_table.end()) {  // the from_id not in lookup table
        lookup_table[from_id] = port_num;
        printLookuptable();
    }

    int to_id = incomming_frame.getTo();
    if(lookup_table.find(to_id) == lookup_table.end()) {    // the to_id is not in lookup table
        for(int i = 0; i < numOfPorts; i++) {
            if(this->ports[i]->status == ACTIVE && i != port_num)
                write(this->ports[i]->output_pipe_fd, massage, MASSAGE_SIZE);
        }

    } else {
        int port = lookup_table.find(to_id)->second;
        if(this->ports[port]->status == ACTIVE)
            write(this->ports[port]->output_pipe_fd, massage, MASSAGE_SIZE);
    }
}

void Switch::handleSTPframe(Frame frame, int port_num) {
    vector<string> args = tokenizeInput(frame.getContent());
    int source_id = stoi(args[0]);
    int source_root_id = stoi(args[1]);
    int source_distance_from_root = stoi(args[2]);
    int new_distance = source_distance_from_root + 1;

    if(source_root_id > stp_info->root_id)
        return;

    bool impression_changed = false;
    if(source_root_id < stp_info->root_id || 
       ( (source_root_id == stp_info->root_id) && (new_distance < stp_info->distance_to_root) ) ) {
        impression_changed = true;
        stp_info->root_id = source_root_id;
        stp_info->distance_to_root = new_distance;
        stp_info->next_switch_to_root = source_id;
        stp_info->port_to_root = port_num;

    } else if( (source_distance_from_root < stp_info->distance_to_root) || 
               ( (source_distance_from_root == stp_info->distance_to_root) && (source_id < id) ) ) {
        this->ports[port_num]->status = INACTIVE;
        printPortStatus(port_num);
        this->STPlog << "switch " << id << " diactivate port " << port_num << endl;
    }

    if(impression_changed) {
        string content = getSTPimpression();
        Frame f = Frame(0, 0, STP, content);
        printSTPinfo();
        for(int i = 0; i < numOfPorts; i++) {
            if(this->ports[i]->status == ACTIVE && i != port_num)
                write(this->ports[i]->output_pipe_fd, f.toString().c_str(), f.toString().length()+1);
        }
    }
}

vector<string> Switch::tokenizeInput(string input) {
    stringstream inputStringStream(input);
    return vector<string>(istream_iterator<string>(inputStringStream),
                          istream_iterator<string>());
}