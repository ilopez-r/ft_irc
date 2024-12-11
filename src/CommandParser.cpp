#include "../include/CommandParser.hpp"
#include <sstream>

std::vector<std::string> CommandParser::splitCommand(const std::string &command) {
    std::vector<std::string> parts;
    std::istringstream stream(command);
    std::string part;
    while (std::getline(stream, part, ' ')) {
        parts.push_back(part);
    }
    return parts;
}
