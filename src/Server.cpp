/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/19 20:21:10 by ilopez-r          #+#    #+#             */
/*   Updated: 2024/12/20 18:17:42 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

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
	help =
		"Here are all the commands that you can use:\n"
		"- PASS <password>: Necessary to start using the chat\n"
		"- USER <username>: Register your username\n"
		"- NICK <nickname>: Set your nickname\n"
		"- CHANNELS: Show available channels and which people are in them\n"
		"- JOIN <#channel>: Join a channel\n"
		"- MSG <user/#channel> <message>: Send a private message\n"
		"- INVITE <user> <#channel>: Invite a user to a channel\n"
		"- TOPIC <#channel> [new topic]: Show or set (if you add <new topic>) a topic for a channel\n"
		"- KICK <channel> <user> :<reason>: For operators. Kick a user from a channel\n"
		"- MODE <#channel> <+|-mode> [param]: For operators. Change modes in a channel\n"
		"		* i: Set/remove Invite-only channel\n"
		"		* t: Set/remove the restrictions of the TOPIC command to channel operators\n"
		"		* k: Set/remove the channel key (password)\n"
		"		* o: Give/remove channel operator privilege\n"
		"		* l: Set/remove the user limit to channel\n"
		"- LEAVE <#channel>: Disconnect from a channel\n"
		"- QUIT: Disconnect from the server\n"
		"- HELP: Show instructions\n"
		"Enjoy your chat!\n";
	design =
		"\n"
		"  ______ _       __          __      _____   __           ___\n"
    	" |  ____| |      \\ \\        / /\\    / ____| /_/          |__ \\ \n"
    	" | |__  | |       \\ \\  /\\  / /  \\  | (___   / \\             ) |\n"
		" |  __| | |        \\ \\/  \\/ / /\\ \\  \\___ \\ / _ \\           / / \n"
		" | |____| |____     \\  /\\  / ____ \\ ____) / ___ \\         / /_ \n"
		" |______|______|     \\/  \\/_/    \\_\\_____/_/   \\_\\       |____|\n"
		"\n"
		"		⠀⠀⠀⠀⠀⠀⠀⢀⣠⣤⣤⣶⣶⣶⣶⣤⣤⣄⡀⠀⠀⠀⠀⠀⠀⠀\n"
		"		⠀⠀⠀⠀⢀⣤⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣤⡀⠀⠀⠀⠀\n"
		"		⠀⠀⠀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⠀⠀⠀\n"
		"		⠀⢀⣾⣿⣿⣿⣿⡿⠟⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡀⠀\n"
		"		⠀⣾⣿⣿⣿⣿⡟⠀⠀⠀⢹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⠀\n"
		"		⢠⣿⣿⣿⣿⣿⣧⠀⠀⠀⣠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡄\n"
		"		⢸⣿⣿⣿⣿⣿⣿⣦⠀⠀⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇\n"
		"		⠘⣿⣿⣿⣿⣿⣿⣿⣷⣄⠀⠈⠻⢿⣿⠟⠉⠛⠿⣿⣿⣿⣿⣿⣿⠃\n"
		"		⠀⢿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣄⡀⠀⠀⠀⠀⠀⠀⣼⣿⣿⣿⣿⡿⠀\n"
		"		⠀⠈⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣶⣤⣤⣴⣾⣿⣿⣿⣿⡿⠁⠀\n"
		"		⠀⢠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⠀⠀⠀\n"
		"		⠀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠛⠁⠀⠀⠀⠀\n"
		"		⠠⠛⠛⠛⠉⠁⠀⠈⠙⠛⠛⠿⠿⠿⠿⠛⠛⠋⠁⠀⠀⠀⠀⠀⠀⠀\n"
		"\n"
		"Welcome to the IRC Server!\n"
		"Type HELP to show the instructions\n";
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

void Server::run()
{
	while (true)
	{
		int pollCount = poll(&pollFds[0], pollFds.size(), -1);
		if (pollCount < 0)
			throw std::runtime_error("Poll failed");
		for (size_t i = 0; i < pollFds.size(); ++i)
		{
			if (pollFds[i].revents & POLLIN)
			{
				if (pollFds[i].fd == serverSocket)
					acceptNewClient();
				else
					handleClientMessage(pollFds[i].fd);
			}
		}
	}
}

void Server::acceptNewClient() {
	sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int clientFd = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
	if (clientFd < 0) {
		std::cerr << "Failed to accept new client\n";
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

std::string Server::trim(const std::string &str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}


void Server::handleClientMessage(int clientFd) {
    char buffer[512];
    std::memset(buffer, 0, sizeof(buffer));
    int bytesReceived = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived <= 0) {
        removeClient(clientFd);
        return;
    }

    // Procesar el mensaje
    std::string rawInput(buffer);
    std::string trimmedCommand = trim(rawInput);
	if (trimmedCommand.empty()) {
        return; // No hacer nada si la línea está vacía.
    }
	
    // Procesar el comando
    clients[clientFd]->processMessage(trimmedCommand, *this);
}

void Server::removeClient(int clientFd) {
	close(clientFd);
	delete clients[clientFd];
	clients.erase(clientFd);

	pollFds.erase(std::remove_if(pollFds.begin(), pollFds.end(), RemovePollFd(clientFd)),
				  pollFds.end());

	std::cout << "Client disconnected: " << clientFd << "\n";
}

void Server::joinChannel(const std::string &channelName, Client *client, const std::string &key) {
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;

	// Si el canal no existe, créalo y asigna al cliente como operador.
	if (it == channels.end())
	{
		channels[channelName] = Channel(channelName);
		Channel &channel = channels[channelName];
		channel.addClient(client);
		channel.addOperator(client); // Cliente se convierte en operador inicial.
		std::cout << "Client (" << client->getFd() << ") '" << client->getNickname() << "' created and joined channel: " << channelName << "\n";
		client->sendMessage("~ You have created and joined channel: " + channelName + "\n");
		if (key != "")
            client->sendMessage("~ Channel '" + channelName + "' does not need a key to enter\n");
		return;
	}

	// Si el canal ya existe
	// Comprobar si el cliente ya está en el canal.
	if (channel.hasClient(client)){
		client->sendMessage("~ ERROR: You are already in the channel: " + channelName + "\n");
		return;
	}

	// Si el canal está en modo invite-only, verificar si el cliente ha sido invitado.
	if (channel.isInviteOnly() && !channel.isInvited(client)) {
		client->sendMessage("~ ERROR: Channel " + channelName + " is invite-only\n");
		return;
	}

	// Comprobar límite de usuarios.
	if (channel.getUserLimit() > 0 && channel.getUserLimit() <= channel.clients.size()) {
		client->sendMessage("~ ERROR: Channel " + channelName + " is full\n");
		return;
	}
	//Si no esta en modo key y yo he puesto una contraseña
	if (channel.getKey() == "" && key != "")
        client->sendMessage("~ Channel '" + channelName + "' does not need a key to enter\n");
	if (channel.getKey() != "" && key != "" && channel.isInvited(client))
		client->sendMessage("~ You do not need a key to enter in channel: " + channelName + " beacause you are invited\n");
	// Comprobar si el canal está en modo key
	if (channel.getKey() != "")
	{
		if (key == channel.getKey() || channel.isInvited(client))
		{
			channel.addClient(client);
			client->sendMessage("~ You joined channel: " + channelName + "\n");
			channel.broadcastMessage("~ '" + client->getNickname() + "' joined channel: " + channelName + "\n", client);
			std::cout << "Client (" << client->getFd() << ") '" << client->getNickname() << "' joined channel: " << channelName << "\n";
			return;
		}
		else if (key == "")
		{
			client->sendMessage("~ ERROR: Channel is in KEY mode. Use: JOIN <#channel> <key>\n");
			return;
		}
		else
		{
			client->sendMessage("~ ERROR: Incorrect key\n");
			return;
		}
	}

	//Añade al cliente.
	channel.addClient(client);
	client->sendMessage("~ You joined channel: " + channelName + "\n");
	channel.broadcastMessage("~ '" + client->getNickname() + "' joined channel: " + channelName + "\n", client);
	std::cout << "Client (" << client->getFd() << ") '" << client->getNickname() << "' joined channel: " << channelName << "\n";
}
void Server::leaveChannel(const std::string &channelName, Client *client) {
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())//Comprobar si el canal existe
	{
		client->sendMessage("~ ERROR: No channel: "  + channelName + " found\n");
		return;
	}
	Channel &channel = it->second;
	if (!channel.hasClient(client))//Comprobar si el cliente está dentro del canal
	{
		client->sendMessage("~ ERROR: You are not in channel: " + channelName + "\n");
		return;
	}

	client->sendMessage("~ You left channel: " + channelName + "\n");
	channel.broadcastMessage("~ '" + client->getNickname() + "' left channel: " + channelName + "\n", client);
	std::cout << "Client (" << client->getFd() << ") '" << client->getNickname() << "' left channel: " << channelName << "\n";
	channel.removeClient(client);

	// Si no hay operadores, asignar el operador al cliente más antiguo
    if (channel.operators.size() == 0 && channel.clients.size() > 0)
    {
        Client *firstClient = *channel.clients.begin();
        channel.operators.insert(*channel.clients.begin());
		firstClient->sendMessage("~ You are now an operator in channel: " + channelName + "\n");
        channel.broadcastMessage("~ '" + firstClient->getNickname() + "' is now an operator in channel: " + channelName + "\n", firstClient);
	    std::cout << "Client (" << firstClient->getFd() << ") '" << firstClient->getNickname() << "' is now an operator in channel: " << channelName << "\n";
    }

    //Si no hay nadie más en el canal, eliminar canal
    if (channel.clients.size() == 0)
    {
        std::cout << "Channel: " << channelName << " was removed\n";
        channels.erase(it);
    }
}

void Server::showChannels(Client *client)
{
	if (channels.empty())
	{
        client->sendMessage("~ No channels are currently available\n");
        return;
    }

	client->sendMessage("~ List of active channels:\n");

	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); it++)
	{
		const std::string &channelName = it->first;
        const Channel &channel = it->second;

		client->sendMessage("	- " + channelName + "(" + channel.getChannelSize(channel.clients.size()) + "): ");
		for (std::set<Client *>::iterator clientIt = channel.clients.begin(); clientIt != channel.clients.end(); ++clientIt)
		{
			std::set<Client *>::iterator nextIt = clientIt;
    		++nextIt;
			if (nextIt == channel.clients.end())
				client->sendMessage((*clientIt)->getNickname() + "\n");
			else
            	client->sendMessage((*clientIt)->getNickname() + ", ");
        }
	}
}

void Server::sendMessageToReceiver(const std::string &receiver, const std::string &message, Client *sender) {
	// Verificar si el receptor es un canal.
	if (!receiver.empty() && receiver[0] == '#') {
		std::map<std::string, Channel>::iterator it = channels.find(receiver);
		if (it == channels.end()) {
			sender->sendMessage("~ ERROR: Channel " + receiver + " does not exist\n");
			return;
		}
		Channel &channel = it->second;
		if (!channel.hasClient(sender))
		{
			sender->sendMessage("~ ERROR: To send a message in channel: " + channel.getName() + " you have to JOIN it first\n");
			return;
		}
		// Enviar mensaje al remitente y difundir en el canal.
		std::string formattedMessage = "~ [" + channel.getName() + "] " + sender->getNickname() + ": " + message + "\n";
		sender->sendMessage(formattedMessage);
		it->second.broadcastMessage(formattedMessage, sender);
		return;
	}

	// Verificar si el receptor es un usuario.
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->getNickname() == receiver) {
			// Enviar mensaje al receptor y al remitente.
			std::string formattedMessageWhisp = "~ [PRV] " + sender->getNickname() + ": " + message + "\n";
			sender->sendMessage(formattedMessageWhisp);
			it->second->sendMessage(formattedMessageWhisp);
			return;
		}
	}

	// Si no se encuentra el receptor.
	sender->sendMessage("~ ERROR: User " + receiver + " does not exist\n");
}

void Server::disconnectClient(Client *client)
{
	// Eliminar al cliente de los canales en los que está.
	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end();)
	{
		if (it->second.hasClient(client))
		{
			leaveChannel(it->first, client);
			if (it->second.isInvited(client))
				it->second.invitedClients.erase(client);
			if (channels.find(it->first) == channels.end()) // Si `leaveChannel` eliminó el canal, actualizar el iterador.
				it = channels.begin(); // Reinicia iteración en caso de eliminación.
			else
				++it; // Avanza al siguiente canal si no fue eliminado.
		}
		else
			++it; // Avanza si el cliente no está en este canal.
	}
	int clientFd = client->getFd();
	
	// Eliminar el pollfd asociado al cliente.
	for (std::vector<pollfd>::iterator it = pollFds.begin(); it != pollFds.end(); ++it)
	{
		if (it->fd == clientFd)
		{
			pollFds.erase(it);
			break;
		}
	}
	
	if (client->getNickname() != "")
		std::cout << "Client (" << client->getFd() << ") '" << client->getNickname() << "' disconnected\n";
	else
		std::cout << "Client (" << client->getFd() << ") disconnected\n";
	client->sendMessage("~ You have disconnected from the server\n");
	close(clientFd);
	clients.erase(clientFd);
	delete (client);
}

void Server::notifyChannelsOfNicknameChange(Client *client, const std::string &oldNickname, const std::string &newNickname) {
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
        Channel &channel = it->second;
        if (channel.hasClient(client)) {
            channel.notifyNicknameChange(oldNickname, newNickname, client);
        }
    }
}

bool Server::validatePassword(const std::string &password) const
{
	return this->password == password;
}

void Server::invite_and_kick(const std::string &channelName, const std::string &user, Client *sender, const std::string &reason, int invite)
{
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())//Comprobar si el canal existe
	{
		sender->sendMessage("~ ERROR: Channel: " + channelName + " does not exist\n");
		return;
	}
	Channel &channel = it->second;
	if (!channel.hasClient(sender))//Comprobar si estoy está dentro del canal
	{
		sender->sendMessage("~ ERROR: You are not in channel: " + channelName + "\n");
		return;
	}
	if (!channel.isOperator(sender))//Comprobar si soy operador
	{
		sender->sendMessage("~ ERROR: You are not an operator of channel: " + channelName + ".\n");
		return;
	}
	if (user == sender->getNickname())//Verificar que no me este invitando/kickeando a mi mismo
	{
		if (invite == 1)
			sender->sendMessage("~ ERROR: You cannot invite yourself ('" + sender->getNickname() + "') to a channel\n");
		else
			sender->sendMessage("~ ERROR: You cannot kick yourself ('" + sender->getNickname() + "') from a channel\n");
		return;
	}
	if (invite == 1 && channel.getUserLimit() > 0 && channel.getUserLimit() <= channel.clients.size())//Verificar si el canal esta lleno
	{
		sender->sendMessage("~ ERROR: Channel: " + channelName + " is already full\n");
		return;
	}
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a invitar/kickear
		{
			if (invite == 1 && !channel.hasClient(client))//Si es el comando invite y el usuario al que se invita no esta ya dentro del canal
			{
				channel.inviteClient(client);//Invitar al usuario al canal
				client->sendMessage("~ '" + sender->getNickname() + "' invited you to channel: " + channelName + ". Use: JOIN <#channel> [key]\n");
				sender->sendMessage("~ You invited '" + user + "' to channel: " + channelName + ".\n");
				std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' invited Client (" << client->getFd() << ") '" << client->getNickname() << "' to channel: " << channelName << "\n";
				return;
			}
			else if (invite == 0 && channel.hasClient(client))//Si es el comando kick y el usuario al que se kickea esta dentro del canal
			{
				channel.removeClient(client);//Eliminar al usuario del canal
				if (channel.isInvited(client))//Eliminar al usuario de la lista de invitados del canal si lo estaba
					channel.invitedClients.erase(client);
				channel.broadcastMessage("~ '" + user + "' has been kicked by '" + sender->getNickname() + "' from channel " + channelName + "\n", sender);
				sender->sendMessage("~ You have kicked '" + user + "' from " + channelName + "\n");
				client->sendMessage("~ You have been kicked from " + channelName + " by '" + sender->getNickname() + "'. Reason: " + reason + "\n");
				std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' has kicked Client (" << client->getFd() << ") '" << client->getNickname() << "' from channel " << channelName << "\n";
				return;
			}
			else//Si no se cumple ninguna de las dos, ese usuario ya estaba dentro del canal (si es invite) o ya estaba fuera del canal (si es kick)
			{
				if (invite == 1)
					sender->sendMessage("~ ERROR: User '" + user + "' is already in channel: " + channelName + "\n");
				else
					sender->sendMessage("~ ERROR: User '" + user + "' is not in channel: " + channelName + "\n");
				return;
			}
		}
	}
	sender->sendMessage("~ ERROR: User '" + user + "' does not exist\n"); //El usuario no esta en el servidor
}

void Server::setChannelTopic(const std::string &channelName, const std::string &topic, Client *sender) {
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())//Comprobar si el canal existe
	{
		sender->sendMessage("~ ERROR: Channel: " + channelName + " does not exist\n");
		return;
	}
	Channel &channel = it->second;
	if (!channel.hasClient(sender))//Comprobar si estoy está dentro del canal
	{
		sender->sendMessage("~ ERROR: You are not in channel: " + channelName + "\n");
		return;
	}
	// Establecer o mostrar el tema del canal
	if (topic == "REMOVE" || topic == "remove")
	{
		if ((channel.isTopicRestricted() && !channel.isOperator(sender)))
		{
			sender->sendMessage("~ ERROR: You are not allowed to remove the topic in channel: " + channelName + ". Channel is in MODE +t and you are not an operator\n");
			return;
		}
		if (_topic == "")
		{
			sender->sendMessage("~ ERROR: Channel: " + channelName + " has no TOPIC\n");
			return;
		}
		_topic = "";
		sender->sendMessage("~ You removed TOPIC in channel: " + channelName + "\n");
		channel.broadcastMessage("~ '" + sender->getNickname() + "' removed TOPIC in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' removed TOPIC in channel: " << channelName << "\n";
	}
	else if (topic != "REMOVE" && topic != "remove" && !topic.empty())
	{
		if ((channel.isTopicRestricted() && !channel.isOperator(sender)))
		{
			sender->sendMessage("~ ERROR: You are not allowed to change the topic in channel: " + channelName + ". Channel is in MODE +t and you are not an operator\n");
			return;
		}
		if (_topic == topic)
		{
			sender->sendMessage("~ ERROR: TOPIC in channel: " + channelName + " is already '" + topic + "'\n");
			return;
		}
		_topic = topic;
		sender->sendMessage("~ You changed TOPIC to '" + topic + "' in channel: " + channelName + "\n");
		channel.broadcastMessage("~ '" + sender->getNickname() + "' changed TOPIC to '" + topic + "' in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' changed TOPIC to '" << topic << "' in channel: " << channelName << "\n";
	}
	else
	{
		if (_topic != "")
		{
			sender->sendMessage("~ Current TOPIC for channel: " + channelName + " is '" + _topic + "'\n");
			return;
		}
		sender->sendMessage("~ Channel: " + channelName + " has no TOPIC yet\n");
	}
}

void Server::setChannelMode(const std::string &channelName, const std::string &mode, const std::string &param, Client *sender)
{
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())//Comprobar si el canal existe
	{
		sender->sendMessage("~ ERROR: Channel " + channelName + " does not exist.\n");
		return;
	}
	Channel &channel = it->second;
	if (!channel.hasClient(sender))//Comprobar si estoy está dentro del canal
	{
		sender->sendMessage("~ ERROR: You are not in channel: " + channelName + "\n");
		return;
	}
	if (!channel.isOperator(sender))//Comprobar si soy operador
	{
		sender->sendMessage("~ ERROR: You are not an operator of " + channelName + "\n");
		return;
	}
	if (mode == "+o")// Asignar privilegio de operador
	{
		if (param == "")
		{
			sender->sendMessage("~ ERROR: You need to specify a user for MODE +o\n");
			return;
		}
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			Client *client = clientIt->second;
			if (client->getNickname() == param)
			{
				if (channel.hasClient(client))
				{
					channel.addOperator(clientIt->second);
					sender->sendMessage("~ You gave OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\n");
					channel.broadcastMessage2("~ '" + sender->getNickname() + "' gave OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\n", sender, client);
					client->sendMessage("~ '" + sender->getNickname() + "' gave you OPERATOR PRIVILEGES in channel: " + channelName + "\n");
					std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' gave operator privileges to '" << param << "' in channel: " << channelName << "\n";
					return;
				}
				else
				{
					sender->sendMessage("~ ERROR: User '" + param + "' is not in channel: " + channelName + "\n");
					return;
				}
			}
		}
		sender->sendMessage("~ ERROR: User '" + param + "' does not exist\n");

	}
	else if (mode == "-o")// Eliminar privilegio de operador
	{
		if (param == "")
		{
			sender->sendMessage("~ ERROR: You need to specify a user for MODE -o\n");
			return;
		}
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			Client *client = clientIt->second;
			if (client->getNickname() == param)
			{
				if (channel.hasClient(client))
				{
					channel.removeOperator(clientIt->second);
					sender->sendMessage("~ You removed OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\n");
					channel.broadcastMessage2("~ '" + sender->getNickname() + "' removed OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\n", sender, client);
					client->sendMessage("~ '" + sender->getNickname() + "' removed your OPERATOR PRIVILEGES in channel: " + channelName + "\n");
					std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' removed operator privileges to '" << param << "' in channel: " << channelName << "\n";
					return;
				}
				else
				{
					sender->sendMessage("~ ERROR: User '" + param + "' is not in channel: " + channelName + "\n");
					return;
				}
			}
		}
		sender->sendMessage("~ ERROR: User '" + param + "' does not exist\n");

	}
	else if (mode == "+i") //Establecer solo invitados pueden unirse al canal
	{
		channel.setInviteOnly(true);
		sender->sendMessage("~ You enabled INVITE-ONLY mode in channel: " + channelName + "\n");
		channel.broadcastMessage("~ '" + sender->getNickname() + "' enabled INVITE-ONLY mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' enabled invite-only mode in channel: " << channelName << "\n";
	}
	else if (mode == "-i")//Eliminar solo invitados pueden unirse al canal
	{
		channel.setInviteOnly(false);
		sender->sendMessage("~ You disabled INVITE-ONLY mode in channel: " + channelName + "\n");
		channel.broadcastMessage("~ '" + sender->getNickname() + "' disabled INVITE-ONLY mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' disabled invite-only mode in channel: " << channelName << "\n";
	}
	else if (mode == "+t")//Establecer topic solo lo pueden cambiar operadores
	{
		channel.setTopicRestricted(true);
		sender->sendMessage("~ You enabled TOPIC-RESTRICTED mode in channel: " + channelName + "\n");
		channel.broadcastMessage("~ '" + sender->getNickname() + "' enabled TOPIC-RESTRICTED mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' enabled topic-restricted mode in channel: " << channelName << "\n";
	}
	else if (mode == "-t")//Eliminar topic solo lo pueden cambiar operadores
	{
		channel.setTopicRestricted(false);
		sender->sendMessage("~ You disabled TOPIC-RESTRICTED mode in channel: " + channelName + "\n");
		channel.broadcastMessage("~ '" + sender->getNickname() + "' disabled TOPIC-RESTRICTED mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' disabled topic-restricted mode in channel: " << channelName << "\n";
	}
	else if (mode == "+k")//Establecer contraseña al canal
	{
		if (param == "")
		{
			sender->sendMessage("~ ERROR: You need to specify a key for MODE +k\n");
			return;
		}
		channel.setKey(param);
		sender->sendMessage("~ You enabled KEY (" + param + ") mode in channel: " + channelName + "\n");
		channel.broadcastMessage("~ '" + sender->getNickname() + "' enabled KEY (" + param + ") mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' enabled key (" << param << ") mode in channel: " << channelName << "\n";
	}
	else if (mode == "-k")//Eliminar contraseña del canal
	{
		channel.setKey("");
		sender->sendMessage("~ You disabled KEY mode in channel: " + channelName + "\n");
		channel.broadcastMessage("~ '" + sender->getNickname() + "' disabled KEY mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' disabled key mode in channel: " << channelName << "\n";
	}
	else if (mode == "+l")//Establecer numero maximo de usuarios en el canal
	{
		if (param == "")
		{
			sender->sendMessage("~ ERROR: You need to specify a number for MODE +l\n");
			return;
		}
		size_t limit = std::atoi(param.c_str());
		if (limit == 0 || limit == 1)
		{
			sender->sendMessage("~ ERROR: Limit cannot be less than 2\n");
			return;
		}
		if (limit < channel.clients.size())
		{
			sender->sendMessage("~ ERROR: Limit cannot be less than the channel's users actual number (" + channel.getChannelSize(channel.clients.size()) + ")\n");
			return;
		}
		if (limit == channel.getUserLimit())
		{
			sender->sendMessage("~ ERROR: Channel: " + channelName + " is already limited to " + param + "\n");
			return;
		}
		channel.setUserLimit(limit);
		sender->sendMessage("~ You enabled USER-LIMIT (" + param + ") mode in channel: " + channelName + "\n");
		channel.broadcastMessage("~ '" + sender->getNickname() + "' enabled USER-LIMIT (" + param + ") mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' enabled user-limit (" << param << ") mode in channel: " << channelName << "\n";
	}
	else if (mode == "-l")//Establecer numero maximo de usuarios en el canal
	{
		if (channel.getUserLimit() == 0)
		{
			sender->sendMessage("~ ERROR: Channel: " + channelName + " already has no users limit\n");
			return;
		}
		channel.clearUserLimit();
		sender->sendMessage("~ You disabled USER-LIMIT mode in channel: " + channelName + "\n");
		channel.broadcastMessage("~ '" + sender->getNickname() + "' disabled USER-LIMIT mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' disabled user-limit mode in channel: " << channelName << "\n";
	}
	else
		sender->sendMessage("~ ERROR: Invalid mode command. Modes are: +|- i, t, k, o, l\n");
}
