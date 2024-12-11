#include "../include/Client.hpp"
#include "../include/Server.hpp"
#include "../include/Utils.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>

Client::Client(int fd, const std::string &ip)
    : fd(fd), ip(ip), nickname(""), username (""), authenticated(false) {}

Client::~Client() {
    close(fd);
}

int Client::getFd() const { return fd; }

const std::string &Client::getIp() const { return ip; }

const std::string &Client::getNickname() const { return nickname; }

void Client::setNickname(const std::string &nickname) {
    this->nickname = nickname;
    std::cout << "Client (" << getFd() << ") changed nickname to '" << getNickname() << "'" << std::endl;
}

const std::string &Client::getUsername() const { return username; }

void Client::setUsername(const std::string &username) {
    this->username = username;
}

bool Client::isAuthenticated() const { return authenticated; }

void Client::authenticate() { authenticated = true; }

void Client::sendMessage(const std::string &message) {
    if (::send(fd, message.c_str(), message.size(), 0) < 0) {
        std::cerr << "Failed to send message to client: " << fd << std::endl;
    }
}

void Client::processMessage(const std::string &message, Server &server) {
    handleCommand(message, server);
}

void Client::handleCommand(const std::string &command, Server &server) {
    // Ignorar comandos vacíos o que solo contengan espacios.
    std::string trimmedCommand = Utils::trim(command);
    if (trimmedCommand.empty()) {
        return; // No hacer nada si la línea está vacía.
    }

    // Separar el comando en partes.
    size_t spacePos = trimmedCommand.find(' ');
    std::string cmd = trimmedCommand.substr(0, spacePos);
    std::string param = (spacePos != std::string::npos) ? trimmedCommand.substr(spacePos + 1) : "";

    if (cmd == "NICK") {
        if (username == "") {
            sendMessage("~ ERROR: First you have to use command 'USER' and specify a username.\r\n");
            return;
        }

        if (param.empty()) {
            sendMessage("~ ERROR: No nickname provided.\r\n");
            return;
        }

        // Validar el nickname (esto puede expandirse según las reglas de IRC).
        if (param.find(' ') != std::string::npos || param.length() > 9) {
            sendMessage("~ ERROR: Invalid nickname.\r\n");
            return;
        }

        // Asignar el nickname al cliente.
        setNickname(param);
        sendMessage("~ Nickname set to: " + param + "\r\n");

    } else if (cmd == "USER") {
        if (param.empty()) {
            sendMessage("~ ERROR: No username provided.\r\n");
            return;
        }

        // Asignar el username al cliente.
        setUsername(param);
        sendMessage("~ User registered as: " + param + "\r\n");

    } else if (cmd == "JOIN") {
        if (nickname == "") {
            sendMessage("~ ERROR: First you have to use command 'NICK' and specify your nickname.\r\n");
            return;
        }
        if (param.empty()) {
            sendMessage("~ ERROR: No channel name provided.\r\n");
            return;
        }
        if (param[0] != '#') {
            sendMessage("~ ERROR: Channel name must start with '#'.\r\n");
            return;
        }

        server.joinChannel(param, this);

    } else if (cmd == "MSG") {
        if (nickname == "") {
            sendMessage("~ ERROR: First you have to use command 'NICK' and specify your nickname.\r\n");
            return;
        }
        size_t colonPos = param.find(' ');
        if (colonPos == std::string::npos) {
            sendMessage("~ ERROR: Invalid MSG format. Use MSG <receiver> <message>\r\n");
            return;
        }

        std::string receiver = param.substr(0, colonPos);
        std::string message = param.substr(colonPos + 1);

        if (receiver.empty() || message.empty()) {
            sendMessage("~ ERROR: Receiver or message is empty.\r\n");
            return;
        }
        // Enviar el mensaje al receptor.
        if (receiver == getNickname())  {
            sendMessage("~ ERROR: You cannot send a message to yourself (" + getNickname() + ").\r\n");
            return;
        }
        server.sendMessageToReceiver(receiver, message, this);
    } else if (cmd == "QUIT") {
        server.disconnectClient(this);
        return; // Termina la conexión del cliente.

    } else if (cmd == "VIVA") {
        sendMessage("~ ESPAÑA\r\n");

    } else {
        sendMessage("~ Unknown command: " + cmd + "\r\n");
    }
}
