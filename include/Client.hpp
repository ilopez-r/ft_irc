#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <queue>

class Server;

class Client {
public:
    Client(int fd, const std::string &ip);
    ~Client();

    int getFd() const;
    const std::string &getIp() const;
    const std::string &getNickname() const;
    void setNickname(const std::string &nickname);
    const std::string &getUsername() const;
    void setUsername(const std::string &username);
    bool isAuthenticated() const;
    void authenticate();
    bool hasSentPassword() const;
    void setPasswordSent(bool status);

    void sendMessage(const std::string &message);
    void processMessage(const std::string &message, Server &server);

private:
    int fd;
    std::string ip;
    std::string nickname;
    std::string username;
    bool authenticated;
    bool passwordSent;
    std::queue<std::string> messageQueue;

    void handleCommand(const std::string &command, Server &server);
};

#endif
