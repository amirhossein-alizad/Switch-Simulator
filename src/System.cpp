#include "System.hpp"
#include "Frame.hpp"

using namespace std;

constexpr char CONNECT[] = "connect";
constexpr char PING[] = "ping";
constexpr char REQUEST[] = "request";


System::System(int id) {
    this->MASSAGE_SIZE = 59;
    this->id = id;
    this->directory = "system" + to_string(id);
    this->write_to_switch = 0;
    this->getting_file_from_id = -1;
}

int System::getID() { return id; }

void System::initiatePipes() {
    if(mkdir(directory.c_str(), 0777) == -1)
        cout << "System " << id << "can't make its directory!\n";
    
    this->log.open( directory+"/log.txt", std::ios_base::app);
    
    string pipe_name = directory + "/input";
    if(mkfifo(pipe_name.c_str(), 0666) != 0) 
        cout << "failed to make pipe for system " << id << endl;
    int temp = open(pipe_name.c_str(), O_RDONLY | O_NONBLOCK);
    if( temp < 0 )
        cout << "faild to open the pipe for system " << id << endl;
    this->input_pipe = temp;
    this->log << "system " << id << " opend " << this->input_pipe << endl;
}

void System::run(int read_fd_pipe) {
    initiatePipes();

    fd_set inputs;
    int max_fd = 0; // felan estefadeii nadare in
    while (1) {
        FD_ZERO(&inputs);
        FD_SET(read_fd_pipe, &inputs);
        max_fd = read_fd_pipe;
        FD_SET(input_pipe, &inputs);
        if(max_fd < input_pipe)
            max_fd = input_pipe;

        if(select(1000, &inputs, NULL, NULL, NULL) < 0)
            cout << "select error in system " << id << endl;

        if(FD_ISSET(read_fd_pipe, &inputs)) {
            handleManagerCommand(read_fd_pipe);
        } else if(FD_ISSET(input_pipe, &inputs)) {
            handleInputFrame(input_pipe);
        }
    }
}

void System::handleManagerCommand(int read_fd_pipe) {
    char massage[MASSAGE_SIZE];
    read(read_fd_pipe, massage, MASSAGE_SIZE);
    this->log << "incoming massage from the manager: " << massage << endl << endl;

    vector<string> arguments = tokenizeInput(string(massage));
    if( (this->write_to_switch == 0) && (arguments[0] != CONNECT ) ) {
        cout << "system " << id << " is not connected to a swtich!\n";
        return;
    }

    
    if(arguments[0] == CONNECT) {
        if(this->write_to_switch != 0) {
            cout << "system " << id << " already connected to a switch: connection failed" << endl;
            return;
        }

        int write_fd = open(arguments[1].c_str(), O_WRONLY | O_NONBLOCK);
        if(write_fd < 0) {
            cout << "system " << id << " can't open the pipe to write to!\n";
            return;
        }
        this->write_to_switch = write_fd;
        this->log << "system connect to: " << this->write_to_switch << endl;

    } else if(arguments[0] == PING) {
        int to_id = stoi(arguments[1]);
        Frame f = Frame(id, to_id, MASSAGE, "pinging");
        write(this->write_to_switch, f.toString().c_str(), f.toString().length()+1);
    } else if(arguments[0] == REQUEST) {
        int to_id = stoi(arguments[1]);
        string filename = arguments[2];
        Frame f = Frame(id, to_id, REQ, filename);
        write(this->write_to_switch, f.toString().c_str(), f.toString().length()+1);
    }

}

void System::handleInputFrame(int input_pipe) {
    char massage[MASSAGE_SIZE];
    read(input_pipe, massage, MASSAGE_SIZE);
    Frame incomming_frame = Frame(string(massage));
    this->log << "incoming frame: " << massage << endl << endl;

    if(incomming_frame.getTo() != id || incomming_frame.getType() == STP)
        return;

    if(incomming_frame.getType() == MASSAGE) {
        Frame response = Frame(id, incomming_frame.getFrom(), MASSAGE_CNF, "pinging back");
        write(this->write_to_switch, response.toString().c_str(), response.toString().length()+1);

    } else if(incomming_frame.getType() == REQ) {
        string filename = incomming_frame.getContent();
        ifstream file_content(filename);
        if(file_content.is_open()) {
            string content( (std::istreambuf_iterator<char>(file_content) ), (std::istreambuf_iterator<char>() )  );
            vector<Frame> response = Frame::makeFramesFromMsg(content, id, incomming_frame.getFrom());
            for(int i = 0; i < response.size(); i++)
                write(this->write_to_switch, response[i].toString().c_str(), response[i].toString().length()+1);

        } else {
            Frame response = Frame(id, incomming_frame.getFrom(), FILE_E, "there is no such file!");
            write(this->write_to_switch, response.toString().c_str(), response.toString().length()+1);
        }
        file_content.close();

    } else if(incomming_frame.getType() == FILE_C) {
        if(getting_file_from_id == -1) {
            getting_file_from_id = incomming_frame.getFrom();
            string recv_file_name = directory + "/" + FILE_NAME;
            recv_file.open(recv_file_name, std::ios::out);
        }

        if(incomming_frame.getFrom() != getting_file_from_id) {
            this->log << "incoming file frame from a different source!" << endl << endl;
            return;
        } else {
            recv_file << incomming_frame.getContent();
        }
        

    } else if(incomming_frame.getType() == FILE_E) {
        getting_file_from_id = -1;
        if(recv_file.is_open()) {
            recv_file.close();
            this->log << "end of file transfer!" << endl << endl;
        }
    }

}

vector<string> System::tokenizeInput(string input) {
    stringstream inputStringStream(input);
    return vector<string>(istream_iterator<string>(inputStringStream),
                          istream_iterator<string>());
}