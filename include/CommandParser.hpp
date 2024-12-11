#ifndef COMMANDPARSER_HPP
#define COMMANDPARSER_HPP

#include <string>
#include <vector>

class CommandParser {
public:
    static std::vector<std::string> splitCommand(const std::string &command);
};

#endif
