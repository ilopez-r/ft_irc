#include "../include/Client.hpp"
#include "../include/Server.hpp"
#include "../include/Utils.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>

Client::Client(int fd, const std::string &ip)
    : fd(fd), ip(ip), nickname(""), username (""), authenticated(false), passwordSent(false) {}

Client::~Client() {
    close(fd);
}

int Client::getFd() const { return fd; }

const std::string &Client::getIp() const { return ip; }

const std::string &Client::getNickname() const { return nickname; }

void Client::setNickname(const std::string &nickname) {
    this->nickname = nickname;
}

const std::string &Client::getUsername() const { return username; }

void Client::setUsername(const std::string &username) {
    this->username = username;
}

bool Client::isAuthenticated() const { return authenticated; }

void Client::authenticate() { authenticated = true; }

bool Client::hasSentPassword() const { return passwordSent; }

void Client::setPasswordSent(bool status) { passwordSent = status; }

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

    if (cmd == "QUIT" || cmd == "quit")
    {
        server.disconnectClient(this);
        return; // Termina la conexión del cliente.
    }
    if (cmd == "VIVA" || cmd == "viva")
    {
        if (cmd == "VIVA")
            sendMessage("~ ESPAÑA\n");
        else
            sendMessage("~ ¡PONLO EN MAYÚSCULAS HOMBRE!\n");
        return;
    }
    if (cmd == "HELP" || cmd == "help") {
        sendMessage(server.welcomeMessage);
        return;
    }
    if (cmd == "PASS" || cmd == "pass")
    {
        if (passwordSent)
        {
            sendMessage("~ ERROR: 'PASS' command already sent.\r\n");
            return;
        }
        if (server.validatePassword(param, this))
        {
            setPasswordSent(true);
            sendMessage("~ Password accepted.\r\n");
        }
        else
            sendMessage("~ ERROR: Incorrect password.\r\n");
    }
    else if (!passwordSent)
    {
        sendMessage("~ ERROR: You must authenticate with 'PASS' before any other command.\r\n");
        return;

    }
    else if (cmd == "USER" || cmd == "user")
    {
        if (param.empty())
        {
            sendMessage("~ ERROR: No username provided.\r\n");
            return;
        }
        // Asignar el username al cliente.
        setUsername(param);
        sendMessage("~ You setted your username to '" + param + "'\r\n");
    }
    else if (cmd == "NICK" || cmd == "nick")
    {
        if (username == "")
        {
            sendMessage("~ ERROR: First you have to use command 'USER' and specify your username\r\n");
            return;
        }
        if (param.empty())
        {
            sendMessage("~ ERROR: No nickname provided\r\n");
            return;
        }
        // Validar el nickname (esto puede expandirse según las reglas de IRC).
        if (param.find(' ') != std::string::npos || param.length() > 9)
        {
            sendMessage("~ ERROR: Invalid nickname\r\n");
            return;
        }
        // Verificar si el nickname ya está en uso.
        if (server.isNicknameInUse(param))
        {
            sendMessage("ERROR: Nickname '" + param + "' is already in use\r\n");
            return;
        }
        // Asignar el nickname al cliente.
        std::string oldNickname = getNickname();
        setNickname(param);
        if (oldNickname != "")
        {
            sendMessage("~ You changed your nickname from '" + oldNickname + "' to '" + param + "'\r\n");
            std::cout << "Client (" << getFd() << ") '" << oldNickname << "' changed his nickname to '" << getNickname() << "'" << std::endl;
        }
        else
        {
            sendMessage("~ You setted your nickname to '" + param + "'\r\n");
            std::cout << "Client (" << getFd() << ") setted his nickname to '" << getNickname() << "'" << std::endl;
        }
        server.notifyChannelsOfNicknameChange(this, oldNickname, param);
    }
    else if (cmd == "JOIN" || cmd == "join") {
        if (nickname == "") {
            sendMessage("~ ERROR: First you have to use command 'NICK' and specify your nickname\r\n");
            return;
        }
        if (param.empty()) {
            sendMessage("~ ERROR: No channel name provided\r\n");
            return;
        }
        if (param[0] != '#') {
            sendMessage("~ ERROR: Channel name must start with '#'\r\n");
            return;
        }

        server.joinChannel(param, this);

    } else if (cmd == "MSG" || cmd == "msg") {
        if (nickname == "") {
            sendMessage("~ ERROR: First you have to use command 'NICK' and specify your nickname\r\n");
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
            sendMessage("~ ERROR: Receiver or message is empty\r\n");
            return;
        }
        // Enviar el mensaje al receptor.
        if (receiver == getNickname())  {
            sendMessage("~ ERROR: You cannot send a message to yourself (" + getNickname() + ").\r\n");
            return;
        }
        server.sendMessageToReceiver(receiver, message, this);

    } else if (cmd == "KICK" || cmd == "kick") {
        if (nickname == "") {
            sendMessage("~ ERROR: First you have to use command 'NICK' and specify your nickname.\r\n");
            return;
        }
        // Sintaxis: KICK <channel> <user> :<reason>
        size_t spacePos2 = param.find(' ');
        std::string channelName = param.substr(0, spacePos2);
        std::string userToKick = param.substr(spacePos2 + 1);

        if (channelName.empty() || userToKick.empty()) {
            sendMessage("ERROR: Invalid KICK command syntax.\r\n");
            return;
        }

        server.kickUserFromChannel(channelName, userToKick, this);

    } else if (cmd == "INVITE" || cmd == "invite") {
        if (nickname == "") {
            sendMessage("~ ERROR: First you have to use command 'NICK' and specify your nickname.\r\n");
            return;
        }
        // Sintaxis: INVITE <user> <channel>
        size_t spacePos2 = param.find(' ');
        std::string userToInvite = param.substr(0, spacePos2);
        std::string channelName = param.substr(spacePos2 + 1);

        if (userToInvite.empty() || channelName.empty()) {
            sendMessage("ERROR: Invalid INVITE command syntax.\r\n");
            return;
        }

        server.inviteUserToChannel(channelName, userToInvite, this);

    } else if (cmd == "TOPIC" || cmd == "topic") {
        if (nickname == "") {
            sendMessage("~ ERROR: First you have to use command 'NICK' and specify your nickname.\r\n");
            return;
        }

        // Sintaxis: TOPIC <channel> <new topic>
        size_t colonPos = param.find(' ');
        std::string channelName = param.substr(0, colonPos);
        std::string topic = param.substr(colonPos + 1);
        if (colonPos == std::string::npos)
           topic = "";

        server.setChannelTopic(channelName, topic, this);

    } else if (cmd == "MODE" || cmd == "mode") {
        if (nickname == "") {
            sendMessage("~ ERROR: First you have to use command 'NICK' and specify your nickname.\r\n");
            return;
        }
        // Sintaxis esperada: MODE <channel> <+|-mode> [param]
        size_t firstSpace = param.find(' ');
        std::string channelName = param.substr(0, firstSpace);
        std::string remaining = (firstSpace != std::string::npos) ? param.substr(firstSpace + 1) : "";

        size_t secondSpace = remaining.find(' ');
        std::string mode = remaining.substr(0, secondSpace);
        std::string modeParam = (secondSpace != std::string::npos) ? remaining.substr(secondSpace + 1) : "";

        if (channelName.empty() || mode.empty()) {
            sendMessage("ERROR: Invalid MODE command syntax. Use MODE <channel> <+|-mode> [param]\r\n");
            return;
        }

        // Enviar la solicitud de configuración del modo al servidor
        server.setChannelMode(channelName, mode, modeParam, this);
        return;
    }
    else
        sendMessage("~ Unknown command: " + cmd + "\r\n");
}
