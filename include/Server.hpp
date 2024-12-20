/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/10 16:35:55 by alirola-          #+#    #+#             */
/*   Updated: 2024/12/20 17:39:39 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <algorithm> // Para std::remove_if
#include <string>
#include <map>
#include <vector>
#include <set>
#include <poll.h>
#include <cctype>
#include "Client.hpp"
#include "Channel.hpp"

class Client;

class Channel;

class Server {
public:
    Server(int port, const std::string &password);
    ~Server();

    void run();
    void joinChannel(const std::string &channelName, Client *client, const std::string &key);
    void leaveChannel(const std::string &channelName, Client *client);
    void showChannels(Client *client);
    void sendMessageToReceiver(const std::string &receiver, const std::string &message, Client *sender);
    void disconnectClient(Client *client);
    bool validatePassword(const std::string &password) const;
    bool isNicknameInUse(const std::string &nickname) const;
    void notifyChannelsOfNicknameChange(Client *client, const std::string &oldNickname, const std::string &newNickname);
    static std::string trim(const std::string &str);
    
    // Métodos para comandos específicos
    void invite_and_kick(const std::string &channelName, const std::string &user, Client *sender, const std::string &reason, int invite);
    void setChannelTopic(const std::string &channelName, const std::string &topic, Client *sender);
    void setChannelMode(const std::string &channelName, const std::string &mode, const std::string &param, Client *sender);
    std::string help; // Mensaje de ayuda
    std::string design; // Mensaje de bienvenida
    std::map<std::string, Channel> channels;

private:
    int port;
    std::string password;
    std::string _topic;
    int serverSocket;
    std::vector<pollfd> pollFds;
    std::map<int, Client*> clients;

    void initializeServer();
    void acceptNewClient();
    void handleClientMessage(int clientFd);
    void removeClient(int clientFd);
    
};

#endif

