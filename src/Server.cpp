/* ************************************************************************** */
/*																			*/
/*														:::	  ::::::::   */
/*   Server.cpp										 :+:	  :+:	:+:   */
/*													+:+ +:+		 +:+	 */
/*   By: ilopez-r <ilopez-r@student.42malaga.com	+#+  +:+	   +#+		*/
/*												+#+#+#+#+#+   +#+		   */
/*   Created: 2024/12/10 17:01:49 by alirola-		  #+#	#+#			 */
/*   Updated: 2024/12/12 14:12:37 by ilopez-r		 ###   ########.fr	   */
/*																			*/
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
	: port(port), password(password), _topic("") {
	initializeServer();
	welcomeMessage =
		"Here are all the commands that you can use:\r\n"
		"- PASS <password>: Necessary to start using the chat\r\n"
		"- USER <username>: Register your username\r\n"
		"- NICK <nickname>: Set your nickname\r\n"
		"- JOIN <#channel>: Join a channel\r\n"
		"- MSG <user/#channel> <message>: Send a private message\r\n"
		"- INVITE <user> <#channel>: Invite a user to a channel\r\n"
		"- TOPIC <#channel> <new topic>: Show or set (if you add <new topic>) a topic for a channel\r\n"
		"- ¡¡NO ESTÁ TEMINADO!! KICK <channel> <user> :<reason>: For operators. Kick a user from a channel\r\n"
		"- MODE <#channel> <+|-mode> [param]: For operators. Change modes in a channel\r\n"
		"		* i: Set/remove Invite-only channel\r\n"
		"		* t: Set/remove the restrictions of the TOPIC command to channel operators\r\n"
		"		* ¡¡NO ESTÁ TEMINADO!! k: Set/remove the channel key (password)\r\n"
		"		* ¡¡NO ESTÁ TEMINADO!! o: Give/take channel operator privilege\r\n"
		"		* l: Set/remove the user limit to channel\r\n"
		"- ¡¡NO ESTÁ TEMINADO!! LEAVE <#channel>: Disconnect from a channel\r\n"
		"- QUIT: Disconnect from the server\r\n"
		"- HELP: Show instructions\r\n"
		"Enjoy your chat!\r\n";
	design =
		"\n"
		"  ______ _       __          __      _____   __           ___\r\n"
    	" |  ____| |      \\ \\        / /\\    / ____| /_/          |__ \\ \r\n"
    	" | |__  | |       \\ \\  /\\  / /  \\  | (___   / \\             ) |\r\n"
		" |  __| | |        \\ \\/  \\/ / /\\ \\  \\___ \\ / _ \\           / / \r\n"
		" | |____| |____     \\  /\\  / ____ \\ ____) / ___ \\         / /_ \r\n"
		" |______|______|     \\/  \\/_/    \\_\\_____/_/   \\_\\       |____|\r\n"
		"\n"
		"		⠀⠀⠀⠀⠀⠀⠀⢀⣠⣤⣤⣶⣶⣶⣶⣤⣤⣄⡀⠀⠀⠀⠀⠀⠀⠀\r\n"
		"		⠀⠀⠀⠀⢀⣤⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣤⡀⠀⠀⠀⠀\r\n"
		"		⠀⠀⠀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⠀⠀⠀\r\n"
		"		⠀⢀⣾⣿⣿⣿⣿⡿⠟⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡀⠀\r\n"
		"		⠀⣾⣿⣿⣿⣿⡟⠀⠀⠀⢹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⠀\r\n"
		"		⢠⣿⣿⣿⣿⣿⣧⠀⠀⠀⣠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡄\r\n"
		"		⢸⣿⣿⣿⣿⣿⣿⣦⠀⠀⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇\r\n"
		"		⠘⣿⣿⣿⣿⣿⣿⣿⣷⣄⠀⠈⠻⢿⣿⠟⠉⠛⠿⣿⣿⣿⣿⣿⣿⠃\r\n"
		"		⠀⢿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣄⡀⠀⠀⠀⠀⠀⠀⣼⣿⣿⣿⣿⡿⠀\r\n"
		"		⠀⠈⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣶⣤⣤⣴⣾⣿⣿⣿⣿⡿⠁⠀\r\n"
		"		⠀⢠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⠀⠀⠀\r\n"
		"		⠀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠛⠁⠀⠀⠀⠀\r\n"
		"		⠠⠛⠛⠛⠉⠁⠀⠈⠙⠛⠛⠿⠿⠿⠿⠛⠛⠋⠁⠀⠀⠀⠀⠀⠀⠀\r\n"
		"\n"
		"Welcome to the IRC Server!\r\n"
		"Type HELP to show the instructions\r\n";
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
	std::cout << "New client connected: (" << clientFd << ")\n";
	clients[clientFd]->sendMessage(design);
	/* clients[clientFd]->sendMessage(welcomeMessage); */
}

bool Server::isNicknameInUse(const std::string &nickname) const {
    for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second->getNickname() == nickname) {
            return true; // El nickname ya está en uso.
        }
    }
    return false; // El nickname está disponible.
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
	std::map<std::string, Channel>::iterator it = channels.find(channelName);

	if (it == channels.end()) {
		// Si el canal no existe, créalo y asigna al cliente como operador.
		channels[channelName] = Channel(channelName);
		Channel &channel = channels[channelName];
		channel.addClient(client);
		channel.addOperator(client); // Cliente se convierte en operador inicial.
		std::cout << "Client (" << client->getFd() << ") '" << client->getNickname() << "' created and joined channel: " << channelName << std::endl;
		client->sendMessage("~ You have created and joined channel: " + channelName + "\r\n");
		channel.broadcastMessage(client->getNickname() + " has created the channel " + channelName + "\r\n", client);
		return;
	}

	// Si el canal ya existe, simplemente añade al cliente.
	Channel &channel = it->second;

	// Comprobar si el cliente ya está en el canal.
	if (channel.hasClient(client)){
		client->sendMessage("ERROR: You are already in the channel: " + channelName + "\r\n");
		return;
	}

	// Si el canal está en modo invite-only, verificar si el cliente ha sido invitado.
	if (channel.isInviteOnly() && !channel.isInvited(client)) {
		client->sendMessage("ERROR: Channel " + channelName + " is invite-only\r\n");
		return;
	}

	// Comprobar límite de usuarios.
	if (channel.getUserLimit() > 0 && channel.getUserLimit() <= channel.clients.size()) {
		client->sendMessage("ERROR: Channel " + channelName + " is full\r\n");
		return;
	}

	channel.addClient(client);
	client->sendMessage("~ You joined channel: " + channelName + "\r\n");
	channel.broadcastMessage("~ '" + client->getNickname() + "' joined channel: " + channelName + "\r\n", client);
	std::cout << "Client (" << client->getFd() << ") '" << client->getNickname() << "' joined channel: " << channelName << std::endl;
}


void Server::sendMessageToReceiver(const std::string &receiver, const std::string &message, Client *sender) {
	// Verificar si el receptor es un canal.
	if (!receiver.empty() && receiver[0] == '#') {
		std::map<std::string, Channel>::iterator it = channels.find(receiver);
		if (it == channels.end()) {
			sender->sendMessage("~ ERROR: Channel " + receiver + " does not exist\r\n");
			return;
		}
		Channel &channel = it->second;
		if (!channel.hasClient(sender))
		{
			sender->sendMessage("~ ERROR: To send a message in channel: " + channel.getName() + " you have to JOIN it first\r\n");
			return;
		}
		// Enviar mensaje al remitente y difundir en el canal.
		std::string formattedMessage = "~ [" + channel.getName() + "] " + sender->getNickname() + ": " + message + "\r\n";
		sender->sendMessage(formattedMessage);
		it->second.broadcastMessage(formattedMessage, sender);
		return;
	}

	// Verificar si el receptor es un usuario.
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->getNickname() == receiver) {
			// Enviar mensaje al receptor y al remitente.
			std::string formattedMessageWhisp = "~ [PRV] " + sender->getNickname() + ": " + message + "\r\n";
			sender->sendMessage(formattedMessageWhisp);
			it->second->sendMessage(formattedMessageWhisp);
			return;
		}
	}

	// Si no se encuentra el receptor.
	sender->sendMessage("~ ERROR: User " + receiver + " does not exist\r\n");
}

void Server::disconnectClient(Client *client) {
	// Difundir el mensaje de despedida a los canales del cliente.
	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
		it->second.removeClient(client);
	}

	// Cerrar la conexión del cliente.
	int clientFd = client->getFd();
	std::string message = "~ You have disconnected from the server\r\n";
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
		std::cout << "Client (" << client->getFd() << ") '" << client->getNickname() << "' disconnected" << std::endl;
	else
		std::cout << "Client (" << client->getFd() << ") disconnected" << std::endl;
	delete client;
}

void Server::notifyChannelsOfNicknameChange(Client *client, const std::string &oldNickname, const std::string &newNickname) {
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
        Channel &channel = it->second;
        if (channel.hasClient(client)) {
            channel.notifyNicknameChange(oldNickname, newNickname, client);
        }
    }
}

bool Server::validatePassword(const std::string &password, Client *client) const {
	std::cout << "Client (" << client->getFd() << ") accepted in server" << std::endl;
	return this->password == password;
}

void Server::kickUserFromChannel(const std::string &channelName, const std::string &userToKick, Client *sender) {
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end()) {
		sender->sendMessage("ERROR: Channel " + channelName + " does not exist\r\n");
		return;
	}

	Channel &channel = it->second;
	if (!channel.isOperator(sender)) {
		sender->sendMessage("ERROR: You are not an operator of " + channelName + "\r\n");
		return;
	}

	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt) {
		Client *client = clientIt->second;
		if (client->getNickname() == userToKick) {
			channel.removeClient(client);
			channel.broadcastMessage(userToKick + " has been kicked by " + sender->getNickname() + "\r\n", sender);
			sender->sendMessage("You have kicked " + userToKick + " from " + channelName + ".\r\n");
			client->sendMessage("You have been kicked from " + channelName + " by " + sender->getNickname() + "\r\n");
			std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' has kicked Client (" << client->getFd() << ") '" << client->getNickname() << "' from channel" << channelName << std::endl;
			return;
		}
	}

	sender->sendMessage("ERROR: User '" + userToKick + "' not found in channel: " + channelName + "\r\n");
}

void Server::inviteUserToChannel(const std::string &channelName, const std::string &userToInvite, Client *sender) {
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end()) {
		sender->sendMessage("ERROR: Channel: " + channelName + " does not exist\r\n");
		return;
	}

	Channel &channel = it->second;
	if (!channel.isOperator(sender)) {
		sender->sendMessage("ERROR: You are not an operator of channel: " + channelName + ".\r\n");
		return;
	}

	if (channel.getUserLimit() > 0 && channel.getUserLimit() <= channel.clients.size()) {
		sender->sendMessage("ERROR: Channel: " + channelName + " is already full\r\n");
		return;
	}

	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt) {
		Client *client = clientIt->second;
		if (client->getNickname() == userToInvite) {
			channel.inviteClient(client);
			client->sendMessage("'" + sender->getNickname() + "' invited you to channel: " + channelName + "\r\n");
			sender->sendMessage("You invited '" + userToInvite + "' to channel: " + channelName + ".\r\n");
			std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' invited Client (" << client->getFd() << ") '" << client->getNickname() << "' to channel: " << channelName << std::endl;
			return;
		}
	}

	sender->sendMessage("ERROR: User '" + userToInvite + "' does not exist\r\n");
}

void Server::setChannelTopic(const std::string &channelName, const std::string &topic, Client *sender) {
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end()) {
		sender->sendMessage("ERROR: Channel: " + channelName + " does not exist\r\n");
		return;
	}

	Channel &channel = it->second;
	
	// Establecer o mostrar el tema del canal
	if (!topic.empty())
	{
		if ((channel.isTopicRestricted() && !channel.isOperator(sender)) || !channel.hasClient(sender))
		{
			sender->sendMessage("ERROR: You are not allowed to change the topic of channel: " + channelName + "\r\n");
			return;
		}
		if (_topic == topic)
		{
			sender->sendMessage("ERROR: TOPIC in channel: " + channelName + " is already '" + topic + "'\r\n");
			return;
		}
		_topic = topic;
		sender->sendMessage("You changed TOPIC to '" + topic + "' in channel: " + channelName + "\r\n");
		channel.broadcastMessage("'" + sender->getNickname() + "' changed TOPIC to '" + topic + "' in channel: " + channelName + "\r\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' changed TOPIC to '" << topic << "' in channel: " << channelName << std::endl;
	}
	else
	{
		if (_topic != "")
		{
			sender->sendMessage("Current TOPIC for channel: " + channelName + " is '" + _topic + "'\r\n");
			return;
		}
		sender->sendMessage("Channel: " + channelName + " has no TOPIC yet\r\n");
	}
}

void Server::setChannelMode(const std::string &channelName, const std::string &mode, const std::string &param, Client *sender) {
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end()) {
		sender->sendMessage("ERROR: Channel " + channelName + " does not exist.\r\n");
		return;
	}

	Channel &channel = it->second;
	if (!channel.isOperator(sender)) {
		sender->sendMessage("ERROR: You are not an operator of " + channelName + ".\r\n");
		return;
	}

	if (mode == "+o") {
		// Asignar privilegio de operador
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt) {
			if (clientIt->second->getNickname() == param) {
				channel.addOperator(clientIt->second);
				sender->sendMessage("You gave OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\r\n");
				channel.broadcastMessage("'" + sender->getNickname() + "' gave OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\r\n", sender);
				std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' gave operator privileges to '" << param << "' in channel: " << channelName << std::endl;
				return;
			}
		}
		sender->sendMessage("ERROR: User " + param + " does not exist.\r\n");

	} else if (mode == "-o") {
		// Revocar privilegio de operador
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt) {
			if (clientIt->second->getNickname() == param) {
				channel.removeOperator(clientIt->second);
				sender->sendMessage("You removed OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\r\n");
				channel.broadcastMessage("'" + sender->getNickname() + "' removed OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\r\n", sender);
				std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' removed operator privileges to '" << param << "' in channel: " << channelName << std::endl;
				return;
			}
		}
		sender->sendMessage("ERROR: User " + param + " does not exist.\r\n");

	} else if (mode == "+i") {
		channel.setInviteOnly(true);
		sender->sendMessage("You enabled INVITE-ONLY mode in channel: " + channelName + "\r\n");
		channel.broadcastMessage("'" + sender->getNickname() + "' enabled INVITE-ONLY mode in channel: " + channelName + "\r\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' enabled invite-only mode in channel: " << channelName << std::endl;
	} else if (mode == "-i") {
		channel.setInviteOnly(false);
		sender->sendMessage("You disabled INVITE-ONLY mode in channel: " + channelName + "\r\n");
		channel.broadcastMessage("'" + sender->getNickname() + "' disabled INVITE-ONLY mode in channel: " + channelName + "\r\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' disabled invite-only mode in channel: " << channelName << std::endl;
	} else if (mode == "+t") {
		channel.setTopicRestricted(true);
		sender->sendMessage("You enabled TOPIC-RESTRICTED mode in channel: " + channelName + "\r\n");
		channel.broadcastMessage("'" + sender->getNickname() + "' enabled TOPIC-RESTRICTED mode in channel: " + channelName + "\r\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' enabled topic-restricted mode in channel: " << channelName << std::endl;
	} else if (mode == "-t") {
		channel.setTopicRestricted(false);
		sender->sendMessage("You disabled TOPIC-RESTRICTED mode in channel: " + channelName + "\r\n");
		channel.broadcastMessage("'" + sender->getNickname() + "' disabled TOPIC-RESTRICTED mode in channel: " + channelName + "\r\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' disabled topic-restricted mode in channel: " << channelName << std::endl;
	} else if (mode == "+k") {
		channel.setKey(param);
		sender->sendMessage("You enabled KEY (" + param + ") mode in channel: " + channelName + "\r\n");
		channel.broadcastMessage("'" + sender->getNickname() + "' enabled KEY (" + param + ") mode in channel: " + channelName + "\r\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' enabled key (" << param << ") mode in channel: " << channelName << std::endl;
	} else if (mode == "-k") {
		channel.setKey("");
		sender->sendMessage("You disabled KEY mode in channel: " + channelName + "\r\n");
		channel.broadcastMessage("'" + sender->getNickname() + "' disabled KEY mode in channel: " + channelName + "\r\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' disabled key mode in channel: " << channelName << std::endl;
	} else if (mode == "+l") {
		size_t limit = std::atoi(param.c_str());
		channel.setUserLimit(limit);
		sender->sendMessage("You enabled USER-LIMIT (" + param + ") mode in channel: " + channelName + "\r\n");
		channel.broadcastMessage("'" + sender->getNickname() + "' enabled USER-LIMIT (" + param + ") mode in channel: " + channelName + "\r\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' enabled user-limit (" << param << ") mode in channel: " << channelName << std::endl;
	} else if (mode == "-l") {
		channel.clearUserLimit();
		sender->sendMessage("You disabled USER-LIMIT mode in channel: " + channelName + "\r\n");
		channel.broadcastMessage("'" + sender->getNickname() + "' disabled USER-LIMIT mode in channel: " + channelName + "\r\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' disabled user-limit mode in channel: " << channelName << std::endl;
	} else {
		sender->sendMessage("ERROR: Invalid mode command. Modes are: +|- i, t, k, o, l\r\n");
	}
}
