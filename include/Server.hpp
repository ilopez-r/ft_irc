/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/10 16:35:55 by alirola-          #+#    #+#             */
/*   Updated: 2024/12/11 20:15:53 by ilopez-r         ###   ########.fr       */
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
    void sendMessageToReceiver(const std::string &receiver, const std::string &message, Client *sender);
    void disconnectClient(Client *client);

private:
    int port;
    std::string password;
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

