#ifndef HEADER_FRAME
#define HEADER_FRAME

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>
#include <fstream>

constexpr int MASSAGE = 1;
constexpr int MASSAGE_CNF = 2;
constexpr int FILE_C = 3;
constexpr int FILE_E = 4;
constexpr int REQ = 5;
constexpr int STP = 6;

class Frame {
    private:
        int from;
        int to;
        int type;
        std::string content;
        static int max_sizeof_content;
        std::vector<std::string> tokenizeInput(std::string input);
        static std::vector<std::string> splitString(std::string s);
    public:
        Frame(int from, int to, int type, std::string cotent);
        Frame(std::string massage);
        static std::vector<Frame> makeFramesFromMsg(std::string msg, int from, int to);
        int getTo();
        int getFrom();
        int getType();
        std::string getContent();
        std::string toString();
};

#endif