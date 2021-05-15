#ifndef HEADER_SYSTEM
#define HEADER_SYSTEM

#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <fstream>
#include <sstream>
#include <iterator>
#include <vector>

#define FILE_NAME "recv_file.txt";

class System {
    private:
        int id;
        std::string directory;
        int input_pipe;
        int write_to_switch;
        int MASSAGE_SIZE;
        int getting_file_from_id;
        std::ofstream recv_file;
        std::ofstream log;
        void handleManagerCommand(int read_fd_pipe);
        void handleInputFrame(int input_pipe);
        void initiatePipes();
        std::vector<std::string> tokenizeInput(std::string input);
    public:
        int getID();
        System(int id);
        void run(int read_fd_pipe);
};

#endif