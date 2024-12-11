/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/10 17:01:49 by alirola-          #+#    #+#             */
/*   Updated: 2024/12/11 20:16:49 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include "../include/Utils.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <algorithm> // Para std::remove_if

// Functor para std::remove_if
struct RemovePollFd {
    int clientFd;

    RemovePollFd(int fd) : clientFd(fd) {}

    bool operator()(const pollfd &p) const {
        return p.fd == clientFd;
    }
};

Server::Server(int port, const std::string &password)
    : port(port), password(password) {
    initializeServer();
}

Server::~Server() {
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        delete it->second;
    }
    close(serverSocket);
}

void Server::initializeServer() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(serverSocket, SOMAXCONN) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }

    struct pollfd pfd;
    pfd.fd = serverSocket;
    pfd.events = POLLIN;
    pfd.revents = 0;
    pollFds.push_back(pfd);
}

void Server::run() {
    while (true) {
        int pollCount = poll(&pollFds[0], pollFds.size(), -1);
        if (pollCount < 0) {
            throw std::runtime_error("Poll failed");
        }

        for (size_t i = 0; i < pollFds.size(); ++i) {
            if (pollFds[i].revents & POLLIN) {
                if (pollFds[i].fd == serverSocket) {
                    acceptNewClient();
                } else {
                    handleClientMessage(pollFds[i].fd);
                }
            }
        }
    }
}

void Server::acceptNewClient() {
    sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int clientFd = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientFd < 0) {
        std::cerr << "Failed to accept new client" << std::endl;
        return;
    }

    struct pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    pollFds.push_back(pfd);

    clients[clientFd] = new Client(clientFd, inet_ntoa(clientAddr.sin_addr));
    std::cout << "New client connected: " << clientFd << std::endl;
}

void Server::handleClientMessage(int clientFd) {
    char buffer[512];
    std::memset(buffer, 0, sizeof(buffer));
    int bytesReceived = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived <= 0) {
        removeClient(clientFd);
        return;
    }

    std::string message(buffer);
    clients[clientFd]->processMessage(message, *this);
}

void Server::removeClient(int clientFd) {
    close(clientFd);
    delete clients[clientFd];
    clients.erase(clientFd);

    pollFds.erase(std::remove_if(pollFds.begin(), pollFds.end(), RemovePollFd(clientFd)),
                  pollFds.end());

    std::cout << "Client disconnected: " << clientFd << std::endl;
}

void Server::joinChannel(const std::string &channelName, Client *client) {
    if (channels.find(channelName) == channels.end()) {
        // Si el canal no existe, créalo
        channels[channelName] = Channel(channelName);
        std::cout << "Client (" << client->getFd() << ") '" << client->getNickname() << "' created channel: " << channelName << std::endl;
        client->sendMessage("~ Channel created: " + channelName + "\r\n");
    }

    // Añadir al cliente al canal
    channels[channelName].addClient(client);
    client->sendMessage("~ Joined channel: " + channelName + "\r\n");
    std::cout << "Client (" << client->getFd() << ") '" << client->getNickname() << "' joined channel: " << channelName << std::endl;
}

void Server::sendMessageToReceiver(const std::string &receiver, const std::string &message, Client *sender) {
    std::string formattedMessage = "~ " + sender->getNickname() + ": " + message + "\r\n";
    std::string formattedMessageWhisp = "~ [WHISP] " + sender->getNickname() + ": " + message + "\r\n";

    // Verificar si el receptor es un canal.
    if (!receiver.empty() && receiver[0] == '#') {
        std::map<std::string, Channel>::iterator it = channels.find(receiver);
        if (it == channels.end()) {
            sender->sendMessage("~ ERROR: Channel " + receiver + " does not exist.\r\n");
            return;
        }

        // Enviar mensaje al remitente y difundir en el canal.
        sender->sendMessage(formattedMessage);
        it->second.broadcastMessage(formattedMessage, sender);
        return;
    }

    // Verificar si el receptor es un usuario.
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second->getNickname() == receiver) {
            // Enviar mensaje al receptor y al remitente.
            sender->sendMessage(formattedMessageWhisp);
            it->second->sendMessage(formattedMessageWhisp);
            return;
        }
    }

    // Si no se encuentra el receptor.
    sender->sendMessage("~ ERROR: User " + receiver + " does not exist.\r\n");
}

void Server::disconnectClient(Client *client) {
    // Difundir el mensaje de despedida a los canales del cliente.
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
        it->second.removeClient(client);
    }

    // Cerrar la conexión del cliente.
    int clientFd = client->getFd();
    std::string message = "~ You have disconnected from server\r\n";
    send(client->getFd(), message.c_str(), message.size(), 0);
    close(clientFd);
    clients.erase(clientFd);

    // Eliminar el pollfd asociado al cliente.
    for (std::vector<pollfd>::iterator it = pollFds.begin(); it != pollFds.end(); ++it) {
        if (it->fd == clientFd) {
            pollFds.erase(it);
            break;
        }
    }
    if (client->getNickname() != "")
        std::cout << "Client (" << client->getFd() << ") '" << client->getNickname() << "' disconnected from server" << std::endl;
    else
        std::cout << "Client (" << client->getFd() << ") disconnected from server" << std::endl;
    delete client;
}

