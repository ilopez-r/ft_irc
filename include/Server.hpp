/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/10 16:35:55 by alirola-          #+#    #+#             */
/*   Updated: 2024/12/16 19:08:02 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <set>
#include <poll.h>
#include "Client.hpp"
#include "Channel.hpp"

class Server {
public:
    Server(int port, const std::string &password);
    ~Server();

    void run();
    void joinChannel(const std::string &channelName, Client *client);
    void leaveChannel(const std::string &channelName, Client *client);
    void showChannels(Client *client);
    void sendMessageToReceiver(const std::string &receiver, const std::string &message, Client *sender);
    void disconnectClient(Client *client);
    bool validatePassword(const std::string &password) const;
    bool isNicknameInUse(const std::string &nickname) const;
    void notifyChannelsOfNicknameChange(Client *client, const std::string &oldNickname, const std::string &newNickname);
    
    // Métodos para comandos específicos
    void kickUserFromChannel(const std::string &channelName, const std::string &userToKick, Client *sender);
    void inviteUserToChannel(const std::string &channelName, const std::string &userToInvite, Client *sender);
    void setChannelTopic(const std::string &channelName, const std::string &topic, Client *sender);
    void setChannelMode(const std::string &channelName, const std::string &mode, const std::string &param, Client *sender);
    std::string welcomeMessage; // Mensaje de bienvenida
    std::string design; // Mensaje de bienvenida


private:
    int port;
    std::string password;
    std::string _topic;
    int serverSocket;
    std::vector<pollfd> pollFds;
    std::map<int, Client*> clients;
    std::map<std::string, Channel> channels;

    void initializeServer();
    void acceptNewClient();
    void handleClientMessage(int clientFd);
    void removeClient(int clientFd);
    
};

#endif

