/* ************************************************************************** */
/*																			*/
/*														:::	  ::::::::   */
/*   Server.cpp										 :+:	  :+:	:+:   */
/*													+:+ +:+		 +:+	 */
/*   By: ilopez-r <ilopez-r@student.42malaga.com	+#+  +:+	   +#+		*/
/*												+#+#+#+#+#+   +#+		   */
/*   Created: 2025/01/10 14:36:53 by ilopez-r		  #+#	#+#			 */
/*   Updated: 2025/01/10 16:10:36 by ilopez-r		 ###   ########.fr	   */
/*																			*/
/* ************************************************************************** */

#include "../include/Server.hpp"

Server::Server(int port, const std::string &password): port(port), _password(password)
{
	initializeServer();
	design =
		"  ______ _	   __		  __	  _____   __		   ___\n"
		" |  ____| |	  \\ \\		/ /\\	/ ____| /_/		  |__ \\ \n"
		" | |__  | |	   \\ \\  /\\  / /  \\  | (___   / \\			 ) |\n"
		" |  __| | |		\\ \\/  \\/ / /\\ \\  \\___ \\ / _ \\		   / / \n"
		" | |____| |____	 \\  /\\  / ____ \\ ____) / ___ \\		 / /_ \n"
		" |______|______|	 \\/  \\/_/	\\_\\_____/_/   \\_\\	   |____|\n"
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
		"\n";
}

Server::~Server()
{
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
		delete (it->second);
	close(serverSocket);
}

void Server::initializeServer()
{
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0)
		throw std::runtime_error("Failed to create socket");
	if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) == -1)// Configurar el socket como no bloqueante
		throw std::runtime_error("Failed to set non-blocking mode on server socket");
	sockaddr_in serverAddr;
	std::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);
	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		throw std::runtime_error("Failed to bind socket");
	if (listen(serverSocket, SOMAXCONN) < 0)
		throw std::runtime_error("Failed to listen on socket");
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

void Server::acceptNewClient()
{
	sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int clientFd = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
	if (clientFd < 0)
	{
		std::cerr << "Failed to accept new client\n";
		return;
	}
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)// Configurar el cliente como no bloqueante
	{
		std::cerr << "Failed to set non-blocking mode on client socket\n";
		close(clientFd);
		return;
	}
	struct pollfd pfd;
	pfd.fd = clientFd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	pollFds.push_back(pfd);

	clients[clientFd] = new Client(clientFd, inet_ntoa(clientAddr.sin_addr));
	std::cout << "New client connected: (" << clientFd << ")\n";
	clients[clientFd]->messageToMyself(design + "Welcome to the IRC Server!\nType HELP to show the instructions\n");
}

void Server::handleClientMessage(int clientFd)
{
	char buffer[512];
	std::memset(buffer, 0, sizeof(buffer));
	int bytesReceived = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
	
	if (bytesReceived <= 0)//Si se cierra la ventana sin hacer QUIT
		return(commandQUIT(clients[clientFd], ""));// Llamar a commandQUIT para eliminar del contenedor de clientes del servidor al cliente y su fd 
	// Acumular datos recibidos en el búfer del cliente
	std::string& clientBuffer = clients[clientFd]->getBuffer(); // Obtener el búfer del cliente
	clientBuffer += std::string(buffer, bytesReceived);

	// Procesar mensajes completos en el búfer
	size_t pos;
	while ((pos = clientBuffer.find('\n')) != std::string::npos)
	{
		std::string fullMessage = clientBuffer.substr(0, pos); // Extraer un mensaje completo
		clientBuffer.erase(0, pos + 1); // Eliminar el mensaje procesado del búfer

		// Procesar el mensaje completo
		clients[clientFd]->processLine(fullMessage, *this);
	}
}

std::string Server::trim(const std::string &str)
{
	size_t start = str.find_first_not_of(" \t\r\n");
	size_t end = str.find_last_not_of(" \t\r\n");
	if (start == std::string::npos)
		return ("");
	else
		return (str.substr(start, end - start + 1));
}
void Server::commandQUIT(Client *client, const std::string &param)
{
	if (!param.empty())// Verificar ningun otra palabara detras de QUIT
			return(client->messageToMyself("~ ERROR: Command 'QUIT' does not accept any parameters\n"));
	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end();)// Eliminar al cliente de los canales en los que está.
	{
		Channel &channel = it->second;
		std::string channelName = it->first;
		if (channel.isInvited(client))//Eliminar al cliente de la lista de invitados si lo estaba (esto se hace ya que si entra otro con un fd invitado, sigue invitado)
			channel.invitedClients.erase(client);
		if (channel.isBanned(client))//Eliminar al cliente de la lista de baneados si lo estaba (esto se hace ya que si entra otro con un fd baneado, sigue baneado)
			channel.unbanClient(client);
		if (channel.hasClient(client))
		{
			commandLEAVE(client, channelName, "");
			if (channels.find(channelName) == channels.end()) // Si `commandLEAVE` eliminó el canal, actualizar el iterador.
				it = channels.begin(); // Reinicia iteración en caso de eliminación.
			else
				++it; // Avanza al siguiente canal si no fue eliminado.
		}
		else
			++it; // Avanza si el cliente no está en este canal.
	}
	int clientFd = client->getFd();
	if (!client->getNickname().empty())//Si el cliente tiene nickname
		std::cout << "Client (" << clientFd << ") '" << client->getNickname() << "' disconnected\n";
	else//si el cliente no tiene nickname
		std::cout << "Client (" << clientFd << ") disconnected\n";
	close(clientFd); // Cerrar el fd del cliente
	delete clients[clientFd];//Eliminar el puntero de la memoria
	clients.erase(clientFd);// Eliminar de la lista de clientes del servidor
	for (std::vector<pollfd>::iterator it = pollFds.begin(); it != pollFds.end(); ++it)// Recorrer pollFds para eliminar al cliente
	{
		if (it->fd == clientFd)
		{
			pollFds.erase(it);
			break;
		}
	}
}

void Server::commandHELP(Client *client, const std::string &cmd, const std::string &other)
{
	std::string cmdUpper = cmd;
	for (std::size_t i = 0; i < cmd.size(); i++)
		cmdUpper[i] = toupper(cmd[i]);
	if (!other.empty())
		return(client->messageToMyself("~ ERROR: Command 'HELP' does not accept any more parameters than an existing command. Use: HELP [cmd]\n"));
	std::string QUIT = "~ QUIT: Disconnect from the server\n";
	if (cmdUpper == "QUIT")
		return(client->messageToMyself(QUIT));
	std::string PASS = "~ PASS <password>: Necessary to start using the chat\n";
	if (cmdUpper == "PASS")
		return(client->messageToMyself(PASS));
	std::string USER = "~ USER <username>: Register your username\n";
	if (cmdUpper == "USER")
		return(client->messageToMyself(USER));
	std::string NICK = "~ NICK <nickname>: Set your nickname\n";
	if (cmdUpper == "NICK")
		return(client->messageToMyself(NICK));
	std::string PROFILE = "~ PROFILE: Show your username and nickname\n";
	if (cmdUpper == "PROFILE")
		return(client->messageToMyself(PROFILE));
	std::string CHANNELS = "~ CHANNELS [all]: Show in which channels you are active. Type <all> to see all channels in server\n";
	if (cmdUpper == "CHANNELS")
		return(client->messageToMyself(CHANNELS));
	std::string MSG = "~ MSG <user/#channel> <message>: Send a message\n";
	if (cmdUpper == "MSG")
		return(client->messageToMyself(MSG));
	std::string JOIN = "~ JOIN <#channel> [key]: Join a channel\n";
	if (cmdUpper == "JOIN")
		return(client->messageToMyself(JOIN));
	std::string LEAVE = "~ LEAVE <#channel>: Disconnect from a channel\n";
	if (cmdUpper == "LEAVE")
		return(client->messageToMyself(LEAVE));
	std::string KICK = "~ KICK <#channel> <user> <reason>: For operators. Kick a user from a channel\n";
	if (cmdUpper == "KICK")
		return(client->messageToMyself(KICK));
	std::string BAN = "~ BAN <#channel> <user> <reason>: For operators. BAN a user from a channel\n";
	if (cmdUpper == "BAN")
		return(client->messageToMyself(BAN));
	std::string UNBAN = "~ UNBAN <#channel> <user>: For operators. UNBAN a user from a channel\n";
	if (cmdUpper == "UNBAN")
		return(client->messageToMyself(UNBAN));
	std::string INVITE = "~ INVITE <#channel> <user>: For operators. Invite a user to a channel\n";
	if (cmdUpper == "INVITE")
		return(client->messageToMyself(INVITE));
	std::string TOPIC = "~ TOPIC <#channel> [new topic]: Show or set (if you add <new topic> and you are operator) a topic for a channel\n";
	if (cmdUpper == "TOPIC")
		return(client->messageToMyself(TOPIC));
	std::string KEY = "~ KEY <#channel>: For operators. Show the key from a channel\n";
	if (cmdUpper == "KEY")
		return(client->messageToMyself(KEY));
	std::string MODE = "~ MODE <#channel> [+|-mode] [param]: For operators. Change modes in a channel\n"
						"	* +|- i: Set/remove invite-only channel\n"
						"	* +|- t: Set/remove the restrictions of the topic command to channel operators\n"
						"	* +|- k: Set/remove the channel key (password)\n"
						"	* +|- o: Give/remove channel operator privileges\n"
						"	* +|- l: Set/remove the user limit to a channel\n";
	if (cmdUpper == "MODE")
		return(client->messageToMyself(MODE));
	std::string REMOVE = "~ REMOVE <#channel> <topic/modes/invited>: For operators. Remove topic, modes or the clients invited list from a channel\n";
	if (cmdUpper == "REMOVE")
		return(client->messageToMyself(REMOVE));
	std::string HELP = "~ HELP [cmd]: Show instructions. Type a command <cmd> to see only its instructions\n";
	if (cmdUpper == "")
		return(client->messageToMyself(QUIT + PASS + USER + NICK + PROFILE + CHANNELS + MSG + JOIN + LEAVE + KICK + BAN + UNBAN + INVITE + TOPIC + KEY + MODE + REMOVE + HELP));
	return(client->messageToMyself("~ ERROR: Command '" + cmdUpper + "' is not an existing command. Use: HELP [cmd]\n"));
}

void Server::commandPASS(Client *client, const std::string &pass, const std::string &other)
{
	if (client->getPasswordSent() == true)//Verificar que no se mande 2 veces la misma pass
			return(client->messageToMyself("~ ERROR: 'PASS' command already sent\n"));
	if (other != "")// Verificar ninguna otra palabara detras de la password
		return(client->messageToMyself("~ ERROR: Command 'PASS' does not accept any more parameters than the PASSWORD. Use: PASS <password>\n"));
	if (pass == _password)// Verificar si la pass es correcta
	{
		std::cout << "Client (" << client->getFd() << ") accepted in server\n";
		client->setPasswordSent(true);
		client->messageToMyself("~ Password accepted.\n");
	}
	else //password incorrecta
		client->messageToMyself("~ ERROR: Incorrect password. Use: PASS <password>\n");
}

void Server::commandUSER(Client *client, const std::string &username, const std::string &other)
{
	if (username.empty())// Validar username no esta vacio
		return(client->messageToMyself("~ ERROR: No username provided. Use: USER <username>\n"));
	if (!other.empty()) // Verificar ningun otra palabara detras del username
		return(client->messageToMyself("~ ERROR: Command 'USER' does not accept any more parameters than the USERNAME. Use: USER <username>\n"));
	if (username.length() > 9)// Verificar que no sea mas largo de 9 caracteres	
		return(client->messageToMyself("~ ERROR: Username cannot be longer than 9 characters\n"));
	client->setUsername(username);
	client->messageToMyself("~ You setted your username to '" + username + "'\n");
}

void Server::commandNICK(Client *client, const std::string &nickname, const std::string &other)
{
	if (client->getUsername() == "")// Verificar que se ha introducido un username
		return(client->messageToMyself("~ ERROR: First you have to use command 'USER' and specify your username. Use: USER <username>\n"));
	if (nickname.empty())// Validar nickname no esta vacio
		return(client->messageToMyself("~ ERROR: No nickname provided. Use: NICK <nickname>\n"));
	if (!other.empty()) // Verificar ningun otra palabara detras del nickname
		return(client->messageToMyself("~ ERROR: Command 'NICK' does not accept any more parameters than the NICKNAME. Use: NICK <nickname>\n"));
	if (nickname.length() > 9)// Verificar que no sea mas largo de 9 caracteres	
		return(client->messageToMyself("~ ERROR: Nickname cannot be longer than 9 characters\n"));
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		Client *clients = it->second;
		if (clients->getNickname() == nickname)// Verificar si el nickname ya está en uso
			return(client->messageToMyself("~ ERROR: Nickname '" + nickname + "' is already in use\n")); // El nickname ya está en uso.
	}
	std::string oldNickname = client->getNickname(); //Guardar el nickname anterior
	client->setNickname(nickname);// Asignar el nickname al cliente.
	if (oldNickname.empty()) //Si es el primer nickname que se pone
	{
		std::cout << "Client (" << client->getFd() << ") setted his nickname to '" << client->getNickname() << "'\n";
		return(client->messageToMyself("~ You setted your nickname to '" + nickname + "'\n"));
	}
	client->messageToMyself("~ You changed your nickname from '" + oldNickname + "' to '" + nickname + "'\n");
	std::cout << "Client (" << client->getFd() << ") '" << oldNickname << "' changed his nickname to '" << client->getNickname() << "'\n";
	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end();it++)
	{
		Channel &channel = it->second;
		std::string channelName = it->first;
		if (it->second.hasClient(client))
			channel.messageToGroupNoSender("~ [" + channelName + "]: '" + oldNickname + "' changed his nickname to '" + nickname + "'\n", client);
	}
}

void Server::commandPROFILE(Client *client, const std::string &param)
{
	if (!param.empty())// Verificar ningun otra palabara detras de PROFILE
			return(client->messageToMyself("~ ERROR: Command 'PROFILE' does not accept any parameters\n"));
	client->messageToMyself("~ Your profile information:\n");
	client->messageToMyself("	- Username: " + client->getUsername() + "\n");
	client->messageToMyself("	- Nickname: " + client->getNickname() + "\n");
}

void Server::commandCHANNELS(Client *client, const std::string &param, const std::string &other)
{
	if ((param != "all" && param != "") || !other.empty())//Verificar ningun otra palabara detras de CHANNELS
		return(client->messageToMyself("~ ERROR: Command 'CHANNELS' does not accept any more parameters than ALL. Use: CHANNELS [all]\n"));
	if (channels.empty())//Si no hay canales creados
		return(client->messageToMyself("~ No channels are currently available\n"));
	if (param == "all")//Si se quieren ver todos los canales que existen
	{
		client->messageToMyself("~ List of all channels:\n");
		for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); it++)//Recorrer todos los acanales que hay ya creados
		{
			const std::string &channelName = it->first;
			const Channel &channel = it->second;
			client->messageToMyself("	* " + channelName);
			if (!channel.getModes().empty())// Mostrar los modos activos solo si hay alguno
				client->messageToMyself("[" + channel.getModes() + "]");
			if (!channel._topic.empty()) //Mostrar el topic si no está vacío
				client->messageToMyself("[Topic: " + channel._topic + "]");
			client->messageToMyself("(" + channel.getChannelSize(channel.clients.size()) + " member(s)): ");
			for (std::set<Client *>::iterator clientIt = channel.clients.begin(); clientIt != channel.clients.end(); ++clientIt)//Recorrer todos los clientes que hay en ese canal
			{
				std::set<Client *>::iterator nextIt = clientIt; 
				++nextIt;//Nos guardamos el siguiente del cliente al que estamos comprobando, para poder ver si es el ultimo que queda del canal por mostrar
				if (nextIt == channel.clients.end())//Si es el ultimo, salto de linea
					client->messageToMyself((*clientIt)->getNickname() + "\n");
				else//Si no es el ultimo, ponemos una coma
					client->messageToMyself((*clientIt)->getNickname() + ", ");
			}
		}
	}
	else //Si se quiere ver solo los canales en los que yo estoy
	{
		int flag = 0;
		for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
		{
			const std::string &channelName = it->first;
			const Channel &channel = it->second;
			if (channel.hasClient(client))
			{
				if (flag == 0)
				{
					flag = 1;
					client->messageToMyself("~ You are currently active in channel(s):\n");
				}
				client->messageToMyself("	* " + channelName);
				if (!channel.getModes().empty())// Mostrar los modos activos solo si hay alguno
					client->messageToMyself("[" + channel.getModes() + "]");
				if (!channel._topic.empty()) //Mostrar el topic si no está vacío
					client->messageToMyself("[Topic: " + channel._topic + "]");
				client->messageToMyself("(" + channel.getChannelSize(channel.clients.size()) + " member(s)): ");
				if (channel.clients.size() == 1)
					client->messageToMyself(client->getNickname() + "\n");
				else
				{
					for (std::set<Client *>::iterator clientIt = channel.clients.begin(); clientIt != channel.clients.end(); ++clientIt)
					{
						if ((*clientIt)->getNickname() != client->getNickname())
						{
							std::set<Client *>::iterator nextIt = clientIt;
							++nextIt;
							std::set<Client *>::iterator nextnextIt = nextIt;
							++nextnextIt;
							if (nextIt == channel.clients.end() || ((*nextIt)->getNickname() == client->getNickname() && nextnextIt == channel.clients.end()))
								client->messageToMyself((*clientIt)->getNickname() + " and " + client->getNickname() + "\n");
							else if (nextIt != channel.clients.end())
								client->messageToMyself((*clientIt)->getNickname() + ", ");
						}
					}
				}
			}
		}
		if (flag == 0)
			client->messageToMyself("~ You are not active in any channels\n");
	}
}

void Server::commandMSG(Client *sender, const std::string &receiver, const std::string &message)
{
	if (receiver.empty() || message.empty())//Verificar que el destinatario y el mensaje no esten vacios
		return(sender->messageToMyself("~ ERROR: Invalid MSG format. Use MSG <receiver> <message>\n"));
	if (receiver == sender->getNickname())//Verificar que no me este mandando un mensaje a mi mismo
		return(sender->messageToMyself("~ ERROR: You cannot send a message to yourself ('" + sender->getNickname() + "')\n"));
	if (receiver[0] == '#')// Si el receptor es un canal
	{
		std::map<std::string, Channel>::iterator it = channels.find(receiver);
		Channel &channel = it->second;
		if (it == channels.end())//Comprobar si el canal existe
			return(sender->messageToMyself("~ ERROR: Channel " + receiver + " does not exist\n"));
		if (!channel.hasClient(sender))// Comprobar si el que envia no está en el canal
			return(sender->messageToMyself("~ ERROR: To send a message in channel: " + channel.getName() + " you have to JOIN it first\n"));
		if (channel.isOperator(sender))//Comprobar si soy operador
			return(channel.messageToGroup("~ [" + channel.getName() + "][OP] " + sender->getNickname() + ": " + message + "\n"));
		else// Enviar mensaje al canal
			return(channel.messageToGroup("~ [" + channel.getName() + "] " + sender->getNickname() + ": " + message + "\n"));
	}
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)//Recorrer todos los clientes que hay en el servidor
	{
		Client *destinatary = it->second;
		if (destinatary->getNickname() == receiver)//Si encuentra al destinatario en el servidor, le manda el mensaje
			return(sender->messageToSomeone("~ [PRV] " + sender->getNickname() + ": " + message + "\n", destinatary));
	}
	sender->messageToMyself("~ ERROR: User '" + receiver + "' does not exist\n");// Si no se encuentra el receptor
}

void Server::commandJOIN(Client *client, const std::string &channelName, const std::string &key, const std::string &other)
{
	if (channelName.empty())// Verificar que se ha especificado un canal
		return(client->messageToMyself("~ ERROR: No channel name provided. Use: JOIN <#channel> [key]\n"));
	if (channelName[0] != '#')//Verificar que el canal empieze por #
		return(client->messageToMyself("~ ERROR: Channel name must start with '#'\n"));
	if (channelName.size() < 2)
		return(client->messageToMyself("~ ERROR: Channel name cannot be empty\n"));
	if (!other.empty())
		return(client->messageToMyself("~ ERROR: Command 'JOIN' does not accept any more parameters than #CHANNEL and KEY. Use: JOIN <#channel> [key]\n"));
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())// Si el canal no existe, se crea y se asigna al cliente como operador.
	{
		channels[channelName] = Channel(channelName);
		Channel &channel = channels[channelName];
		channel.addClient(client);//Añadir al cliente al canal
		channel.addOperator(client); // Cliente se convierte en operador inicial.
		std::cout << "Client (" << client->getFd() << ") '" << client->getNickname() << "' created and joined channel: " << channelName << "\n";//Mensaje en servidor
		client->messageToMyself("~ You have created and joined channel: " + channelName + "\n");//Mensaje al cliente
		if (key != "")//Si se puso algo detrás de #channel
			client->messageToMyself("~ Channel: " + channelName + " does not need a key to enter\n");//Mensaje al cliente
		return;
	}
	// Si el canal ya existe
	if (channel.hasClient(client))// Comprobar si el cliente ya está en el canal
		return(client->messageToMyself("~ ERROR: You are already in channel: " + channelName + "\n"));
	if (channel.isBanned(client))// Verificar si el cliente está baneado
		return(client->messageToMyself("~ ERROR: You are banned in channel: " + channelName + "\n"));
	if (channel.isInviteOnly() && !channel.isInvited(client))// Si el canal está en modo invite-only, verificar si el cliente ha sido invitado
		return(client->messageToMyself("~ ERROR: Channel: " + channelName + " is on INVITE-ONLY mode\n"));
	if (channel.getUserLimit() > 0 && channel.getUserLimit() <= channel.clients.size())// Comprobar si hay límite de usuarios y si ya se ha alcanzado el máximo
		return(client->messageToMyself("~ ERROR: Channel: " + channelName + " is full\n"));
	if (channel.getKey() == "" && key != "")//Si no esta en modo key y yo he puesto una contraseña
		client->messageToMyself("~ Channel: " + channelName + " does not need a key to enter\n");
	if (channel.getKey() != "" && key != "" && channel.isInvited(client))//Si esta en modo key y yo he puesto una key, pero estoy invitado
		client->messageToMyself("~ You do not need a key to enter in channel: " + channelName + " beacause you are invited\n");
	if (channel.getKey() != "" && !channel.isInvited(client))// Comprobar si el canal está en modo key y no estoy invitado
	{
		if (key == "")//Si no he puesto ninguna key
			return(client->messageToMyself("~ ERROR: Channel: " + channelName + " is in KEY mode. Use: JOIN <#channel> <key>\n"));
		if (key != channel.getKey())//Si la key es incorrecta
			return(client->messageToMyself("~ ERROR: Incorrect key\n"));
	}
	channel.addClient(client);//Añade al cliente.
	std::cout << "Client (" << client->getFd() << ") '" << client->getNickname() << "' joined channel: " << channelName << "\n";//Mensaje en servidor
	client->messageToMyself("~ You joined channel: " + channelName + "\n");//Mensaje al cliente
	channel.messageToGroupNoSender("~ '" + client->getNickname() + "' joined channel: " + channelName + "\n", client);//Mensaje al grupo
}

void Server::commandLEAVE(Client *client, const std::string &channelName, const std::string &other)
{
	if (channelName.empty())// Verificar que se ha especificado un canal
		return(client->messageToMyself("~ ERROR: No channel name provided. Use: LEAVE <#channel>\n"));
	if (channelName[0] != '#')//Verificar que el canal empieze por #
		return(client->messageToMyself("~ ERROR: Channel name must start with '#'\n"));
	if (!other.empty())// Verificar ningun otra palabara detras del canal
		return(client->messageToMyself("~ ERROR: Command 'LEAVE' does not accept any more parameters than #CHANNEL. Use: LEAVE <#channel>\n"));
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(client->messageToMyself("~ ERROR: ERROR: Channel " + channelName + " does not exist\n"));
	if (!channel.hasClient(client))//Comprobar si el cliente está dentro del canal
		return(client->messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	channel.removeClientChannnel(client);
	std::cout << "Client (" << client->getFd() << ") '" << client->getNickname() << "' left channel: " << channelName << "\n";
	client->messageToMyself("~ You left channel: " + channelName + "\n");
	channel.messageToGroupNoSender("~ '" + client->getNickname() + "' left channel: " + channelName + "\n", client);
	if (channel.operators.size() == 0 && channel.clients.size() > 0)// Si no hay operadores pero quedan clientes dentro del canal, asignar el operador al cliente más antiguo
	{
		Client *firstClient = *channel.clients.begin();//Guardamos el cliente que hay en el principio del contenedor del canal
		channel.operators.insert(*channel.clients.begin());//Lo insertamos en el contenedor de operadores
		std::cout << "Client (" << firstClient->getFd() << ") '" << firstClient->getNickname() << "' is now an operator in channel: " << channelName << "\n";
		firstClient->messageToMyself("~ You are now an operator in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + firstClient->getNickname() + "' is now an operator in channel: " + channelName + "\n", firstClient);
	}
	if (channel.clients.size() == 0)//Si no hay nadie más en el canal, eliminar canal
	{
		std::cout << "Channel: " << channelName << " was removed\n";
		channels.erase(it);
	}
}

void Server::commandKICK(Client *sender, const std::string &channelName, const std::string &user, const std::string &reason)
{
	if (channelName.empty() || user.empty() || reason.empty())//Verificar que ninguno de los parametros este vacio
		return(sender->messageToMyself("~ ERROR: Invalid KICK command syntax. Use: KICK <#channel> <user> <reason>\n"));
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(sender))//Comprobar si estoy está dentro del canal
		return(sender->messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(sender))//Comprobar si soy operador
		return(sender->messageToMyself("~ ERROR: You are not an operator of channel: " + channelName + ".\n"));
	if (user == sender->getNickname())//Verificar que no me este kickeando a mi mismo
		return(sender->messageToMyself("~ ERROR: You cannot kick yourself ('" + sender->getNickname() + "') from a channel\n"));
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a kickear
		{
			if (channel.hasClient(client))//Si el usuario al que se kickea está dentro del canal
			{
				channel.removeClientChannnel(client);//Eliminar al usuario del canal
				if (channel.isInvited(client))//Eliminar al usuario de la lista de invitados del canal si lo estaba
					channel.invitedClients.erase(client);
				std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' has kicked Client (" << client->getFd() << ") '" << client->getNickname() << "' from channel " << channelName << "\n";
				channel.messageToGroupNoSender("~ '" + user + "' has been kicked by '" + sender->getNickname() + "' from channel " + channelName + "\n", sender);
				sender->messageToMyself("~ You have kicked '" + user + "' from " + channelName + "\n");
				return(client->messageToMyself("~ You have been kicked from " + channelName + " by '" + sender->getNickname() + "'. Reason: " + reason + "\n"));
			}
			else//Si no se cumple, ese usuario ya estaba fuera del canal
				return(sender->messageToMyself("~ ERROR: User '" + user + "' is not in channel: " + channelName + "\n"));
		}
	}
	sender->messageToMyself("~ ERROR: User '" + user + "' does not exist\n"); //El usuario no esta en el servidor
}

void Server::commandBAN(Client *sender, const std::string &channelName, const std::string &user, const std::string &reason)
{
	if (channelName.empty() || user.empty() || reason.empty())//Verificar que ninguno de los parametros este vacio
		return(sender->messageToMyself("~ ERROR: Invalid BAN command syntax. Use: BAN <#channel> <user> <reason>\n"));
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(sender))//Comprobar si estoy está dentro del canal
		return(sender->messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(sender))//Comprobar si soy operador
		return(sender->messageToMyself("~ ERROR: You are not an operator of channel: " + channelName + ".\n"));
	if (user == sender->getNickname())//Verificar que no me este baneando a mi mismo
		return(sender->messageToMyself("~ ERROR: You cannot ban yourself ('" + sender->getNickname() + "') from a channel\n"));
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a banear
		{
			if (channel.isBanned(client))// Verificar si ya está baneado
				return(sender->messageToMyself("~ ERROR: User '" + user + "' is already banned in " + channelName + ".\n"));
			if (channel.hasClient(client))//Eliminar al usuario del canal si está dentro
				channel.removeClientChannnel(client);
			if (channel.isInvited(client))//Eliminar al usuario de la lista de invitados del canal si lo estaba
				channel.invitedClients.erase(client);
			channel.banClient(client);// Banear al usuario
			std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' has banned Client (" << client->getFd() << ") '" << client->getNickname() << "' from channel " << channelName << "\n";
			channel.messageToGroupNoSender("~ '" + user + "' has been banned by '" + sender->getNickname() + "' from channel " + channelName + "\n", sender);
			sender->messageToMyself("~ You have banned '" + user + "' from " + channelName + "\n");
			return(client->messageToMyself("~ You have been banned from " + channelName + " by '" + sender->getNickname() + "'. Reason: " + reason + "\n"));
		}
	}
	sender->messageToMyself("~ ERROR: User '" + user + "' does not exist\n"); //El usuario no esta en el servidor
}

void Server::commandUNBAN(Client *sender, const std::string &channelName, const std::string &user, const std::string &other)
{
	if (channelName.empty() || user.empty())//Verificar que ninguno de los parametros este vacio
		return(sender->messageToMyself("~ ERROR: Invalid UNBAN command syntax. Use: UNBAN <#channel> <user>\n"));
	if (!other.empty())//Verificar ningun otra palabara detras de <user>
		return(sender->messageToMyself("~ ERROR: Command 'UNBAN' does not accept any more parameters than CHANNEL and USER. Use: UNBAN <#channel> <user>\n"));
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(sender))//Comprobar si estoy está dentro del canal
		return(sender->messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(sender))//Comprobar si soy operador
		return(sender->messageToMyself("~ ERROR: You are not an operator of channel: " + channelName + ".\n"));
	if (user == sender->getNickname())//Verificar que no me este unbaneando a mi mismo
		return(sender->messageToMyself("~ ERROR: You cannot unban yourself ('" + sender->getNickname() + "') from a channel\n"));
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a banear
		{
			if (!channel.isBanned(client))// Verificar que no está baneado
				return(sender->messageToMyself("~ ERROR: User '" + user + "' is not banned in " + channelName + ".\n"));
			channel.unbanClient(client);// Desbanear al usuario
			std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' has unbanned Client (" << client->getFd() << ") '" << client->getNickname() << "' from channel " << channelName << "\n";
			channel.messageToGroupNoSender("~ '" + user + "' has been unbanned by '" + sender->getNickname() + "' from channel " + channelName + "\n", sender);
			sender->messageToMyself("~ You have unbanned '" + user + "' from " + channelName + "\n");
			return(client->messageToMyself("~ You have been unbanned from " + channelName + " by '" + sender->getNickname() + "\n"));
		}
	}
	sender->messageToMyself("~ ERROR: User '" + user + "' does not exist\n"); //El usuario no esta en el servidor
}

void Server::commandINVITE(Client *sender, const std::string &channelName, const std::string &user, const std::string &other)
{
	if (channelName.empty() || user.empty())//Verificar que ninguno de los parametros este vacio
		return(sender->messageToMyself("~ ERROR: Invalid INVITE command syntax. Use: INVITE <user> <#channel>\n"));
	if (!other.empty())//Verificar ningun otra palabara detras de <user>
		return(sender->messageToMyself("~ ERROR: Command 'INVITE' does not accept any more parameters than CHANNEL and USER. Use: INVITE <#channel> <user>\n"));
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(sender))//Comprobar si estoy está dentro del canal
		return(sender->messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(sender))//Comprobar si soy operador
		return(sender->messageToMyself("~ ERROR: You are not an operator of channel: " + channelName + ".\n"));
	if (user == sender->getNickname())//Verificar que no me este invitando a mi mismo
		return(sender->messageToMyself("~ ERROR: You cannot invite yourself ('" + sender->getNickname() + "') to a channel\n"));
	if (channel.getUserLimit() > 0 && channel.getUserLimit() <= channel.clients.size())//Verificar si el canal esta lleno
		return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " is already full\n"));
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a invitar/kickear
		{
			if (!channel.hasClient(client))//Si el usuario al que se invita no esta ya dentro del canal
			{
				channel.inviteClient(client);//Invitar al usuario al canal
				std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' invited Client (" << client->getFd() << ") '" << client->getNickname() << "' to channel: " << channelName << "\n";
				sender->messageToMyself("~ You invited '" + user + "' to channel: " + channelName + ".\n");
				return(client->messageToMyself("~ '" + sender->getNickname() + "' invited you to channel: " + channelName + ". Use: JOIN <#channel> [key]\n"));
			}
			else//Si no se cumple ninguna de las dos, ese usuario ya estaba dentro del canal (si es invite) o ya estaba fuera del canal (si es kick)
				return(sender->messageToMyself("~ ERROR: User '" + user + "' is already in channel: " + channelName + "\n"));
		}
	}
	sender->messageToMyself("~ ERROR: User '" + user + "' does not exist\n"); //El usuario no esta en el servidor
}

void Server::commandTOPIC(Client *sender, const std::string &channelName, const std::string &topic)
{
	if (channelName.empty())//Comprobar si solo se ha escrito topic
		return(sender->messageToMyself("~ ERROR: Invalid TOPIC command syntax. Use: TOPIC <#channel> [new topic]\n"));
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())//Comprobar si el canal existe
		return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " does not exist\n"));
	Channel &channel = it->second;
	if (topic.empty())//Comprobar si solamente quiero ver el topic
	{
		if (channel._topic.empty())//Comprobar si el canal no tiene topic
			return(sender->messageToMyself("~ Channel: " + channelName + " has no TOPIC yet\n"));
		return(sender->messageToMyself("~ Current TOPIC for channel: " + channelName + " is '" + channel._topic + "'\n"));
	}
	if (!channel.hasClient(sender))//Comprobar si estoy está dentro del canal
		return(sender->messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if ((channel.isTopicRestricted() && !channel.isOperator(sender)))//Comprobar si el canal esta +t y yo no soy operador
		return(sender->messageToMyself("~ ERROR: You are not allowed to change the topic in channel: " + channelName + ". Channel is in MODE +t and you are not an operator\n"));
	if (topic == "REMOVE" || topic == "remove")// Eliminar el tema del canal
	{
		if (channel._topic == "")//Comprobar si estoy intentando eliminar el topic en un canal que no tiene topic
			return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " has no TOPIC\n"));
		channel._topic = "";
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' removed TOPIC in channel: " << channelName << "\n";
		sender->messageToMyself("~ You removed TOPIC in channel: " + channelName + "\n");
		return(channel.messageToGroupNoSender("~ '" + sender->getNickname() + "' removed TOPIC in channel: " + channelName + "\n", sender));
	}
	if (channel._topic == topic)//Comprobar si estoy intentando poner un topic en un canal que ya tiene ese mismo topic
			return(sender->messageToMyself("~ ERROR: TOPIC in channel: " + channelName + " is already '" + topic + "'\n"));
	channel._topic = topic;
	std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' changed TOPIC to '" << topic << "' in channel: " << channelName << "\n";
	sender->messageToMyself("~ You changed TOPIC to '" + topic + "' in channel: " + channelName + "\n");
	channel.messageToGroupNoSender("~ '" + sender->getNickname() + "' changed TOPIC to '" + topic + "' in channel: " + channelName + "\n", sender);
}

void Server::commandKEY(Client *sender, const std::string &channelName, const std::string &other)
{
	if (channelName.empty())// Verificar que se ha especificado un canal
		return(sender->messageToMyself("~ ERROR: Invalid KEY command syntax. Use: KEY <#channel>\n"));
	if (!other.empty())
		return(sender->messageToMyself("~ ERROR: Command 'KEY' does not accept any more parameters than CHANNEL. Use: KEY <#channel>\n"));
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender->messageToMyself("~ ERROR: Channel " + channelName + " does not exist\n"));
	if (!channel.hasClient(sender))//Comprobar si estoy está dentro del canal
		return(sender->messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(sender))//Comprobar si soy operador
		return(sender->messageToMyself("~ ERROR: You are not an operator in channel: " + channelName + "\n"));
	if(channel.getKey() == "")//Comprobar si el acanal tiene key
		return(sender->messageToMyself("~ Channel: " + channelName + " has no key\n"));
	sender->messageToMyself("~ Channel: " + channelName + " has '" + channel.getKey() + "' as a key\n");
}

void Server::commandMODE(Client *sender, const std::string &channelName, const std::string &mode, const std::string &param)
{
	if (channelName.empty())//Verificar que canal no esté vacío
		return(sender->messageToMyself("~ ERROR: Invalid MODE command syntax. Use: MODE <channel> [+|-mode] [param]\n"));
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender->messageToMyself("~ ERROR: Channel " + channelName + " does not exist\n"));
	if (mode.empty())// Si no se especifica modo, mostrar los modos activos del canal
	{
		if (!channel.getModes().empty())
			return(sender->messageToMyself("~ Active mode(s) for channel " + channelName + ": " + channel.getModes() + "\n"));
		return(sender->messageToMyself("~ No active modes for channel: " + channelName + "\n"));
	}
	if (!param.empty() && (mode == "+i" || mode == "-i" || mode == "+t" || mode == "-t" || mode  == "-l" || mode  == "-k"))
		return(sender->messageToMyself("~ ERROR: Command 'MODE " + mode +"' does not accept any more parameters than CHANNEL and <+|-mode>. Use: MODE <channel> <+|-mode>\n"));
	size_t spacePos = param.find(' ');
	if(spacePos != std::string::npos)
		return(sender->messageToMyself("~ ERROR: Command 'MODE " + mode +"' does not accept any more parameters than CHANNEL, <+|-mode> and [param]. Use: MODE <channel> <+|-mode> [param]\n"));
	if (!channel.hasClient(sender))//Comprobar si estoy está dentro del canal
		return(sender->messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(sender))//Comprobar si soy operador
		return(sender->messageToMyself("~ ERROR: You are not an operator in channel: " + channelName + "\n"));
	if (mode == "+o")// Asignar privilegio de operador
	{
		if (param == "")
			return(sender->messageToMyself("~ ERROR: You need to specify a user for MODE +o\n"));
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			Client *client = clientIt->second;
			if (client->getNickname() == param)
			{
				if (channel.hasClient(client))
				{
					if (channel.isOperator(client))
						return(sender->messageToMyself("~ ERROR: '" + param + "' is already an operator in channel: " + channelName + "\n"));
					channel.addOperator(clientIt->second);
					std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' gave operator privileges to '" << param << "' in channel: " << channelName << "\n";
					sender->messageToMyself("~ You gave OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\n");
					channel.messageToGroupNoSenderNoReceiver("~ '" + sender->getNickname() + "' gave OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\n", sender, client);
					return(client->messageToMyself("~ '" + sender->getNickname() + "' gave you OPERATOR PRIVILEGES in channel: " + channelName + "\n"));
				}
				else
					return(sender->messageToMyself("~ ERROR: User '" + param + "' is not in channel: " + channelName + "\n"));
			}
		}
		sender->messageToMyself("~ ERROR: User '" + param + "' does not exist\n");

	}
	else if (mode == "-o")// Eliminar privilegio de operador
	{
		if (param == "")
			return(sender->messageToMyself("~ ERROR: You need to specify a user for MODE -o\n"));
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			Client *client = clientIt->second;
			if (client->getNickname() == param)
			{
				if (channel.hasClient(client))
				{
					if (!channel.isOperator(client))
						return(sender->messageToMyself("~ ERROR: '" + param + "' is not an operator in channel: " + channelName + "\n"));
					channel.removeOperator(clientIt->second);
					std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' removed operator privileges to '" << param << "' in channel: " << channelName << "\n";
					sender->messageToMyself("~ You removed OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\n");
					channel.messageToGroupNoSenderNoReceiver("~ '" + sender->getNickname() + "' removed OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\n", sender, client);
					return(client->messageToMyself("~ '" + sender->getNickname() + "' removed your OPERATOR PRIVILEGES in channel: " + channelName + "\n"));
				}
				else
					return(sender->messageToMyself("~ ERROR: User '" + param + "' is not in channel: " + channelName + "\n"));
			}
		}
		sender->messageToMyself("~ ERROR: User '" + param + "' does not exist\n");

	}
	else if (mode == "+i") //Establecer solo invitados pueden unirse al canal
	{
		if (channel.inviteOnly == true)
			return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " is already on INVITE-ONLY mode\n"));
		channel.setInviteOnly(true);
		sender->messageToMyself("~ You enabled INVITE-ONLY mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender->getNickname() + "' enabled INVITE-ONLY mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' enabled invite-only mode in channel: " << channelName << "\n";
	}
	else if (mode == "-i")//Eliminar solo invitados pueden unirse al canal
	{
		if (channel.inviteOnly == false)
			return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " is not on INVITE-ONLY mode\n"));
		channel.setInviteOnly(false);
		sender->messageToMyself("~ You disabled INVITE-ONLY mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender->getNickname() + "' disabled INVITE-ONLY mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' disabled invite-only mode in channel: " << channelName << "\n";
	}
	else if (mode == "+t")//Establecer topic solo lo pueden cambiar operadores
	{
		if (channel.topicRestricted == true)
			return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " is already on TOPIC-RESTRICTED mode\n"));
		channel.setTopicRestricted(true);
		sender->messageToMyself("~ You enabled TOPIC-RESTRICTED mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender->getNickname() + "' enabled TOPIC-RESTRICTED mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' enabled topic-restricted mode in channel: " << channelName << "\n";
	}
	else if (mode == "-t")//Eliminar topic solo lo pueden cambiar operadores
	{
		if (channel.topicRestricted == false)
			return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " is not on TOPIC-RESTRICTED mode\n"));
		channel.setTopicRestricted(false);
		sender->messageToMyself("~ You disabled TOPIC-RESTRICTED mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender->getNickname() + "' disabled TOPIC-RESTRICTED mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' disabled topic-restricted mode in channel: " << channelName << "\n";
	}
	else if (mode == "+k")//Establecer contraseña al canal
	{
		if (param == "")
			return(sender->messageToMyself("~ ERROR: You need to specify a key for MODE +k\n"));
		if (param.size() > 9)
			return(sender->messageToMyself("~ ERROR: Key cannot be longer than 9 characters\n"));
		if(channel.getKey() == param)
			return(sender->messageToMyself("~ ERROR: KEY is already '" + param + "' in channel: " + channelName + "\n"));
		channel.setKey(param);
		sender->messageToMyself("~ You enabled KEY (" + param + ") mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender->getNickname() + "' enabled KEY (" + param + ") mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' enabled key (" << param << ") mode in channel: " << channelName << "\n";
	}
	else if (mode == "-k")//Eliminar contraseña del canal
	{
		if(channel.getKey() == "")
			return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " is not on KEY mode\n"));
		channel.setKey("");
		sender->messageToMyself("~ You disabled KEY mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender->getNickname() + "' disabled KEY mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' disabled key mode in channel: " << channelName << "\n";
	}
	else if (mode == "+l")//Establecer numero maximo de usuarios en el canal
	{
		if (param == "")
			return(sender->messageToMyself("~ ERROR: You need to specify a number for MODE +l\n"));
		for (std::size_t i = 0; i < param.size(); i++)
			if (!std::isdigit(param[i]))
				return(sender->messageToMyself("~ ERROR: Limit cannot be something different from a number\n"));
		size_t limit = std::atoi(param.c_str());
		if (limit == 0 || limit == 1)
			return(sender->messageToMyself("~ ERROR: Limit cannot be less than 2\n"));
		if (limit < channel.clients.size())
			return(sender->messageToMyself("~ ERROR: Limit cannot be less than the channel's users actual number (" + channel.getChannelSize(channel.clients.size()) + ")\n"));
		if (limit == channel.getUserLimit())
			return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " is already limited to " + param + "\n"));
		channel.setUserLimit(limit);
		sender->messageToMyself("~ You enabled USER-LIMIT (" + param + ") mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender->getNickname() + "' enabled USER-LIMIT (" + param + ") mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' enabled user-limit (" << param << ") mode in channel: " << channelName << "\n";
	}
	else if (mode == "-l")//Eliminar numero maximo de usuarios en el canal
	{
		if (channel.getUserLimit() == 0)
			return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " already has no users limit\n"));
		channel.clearUserLimit();
		sender->messageToMyself("~ You disabled USER-LIMIT mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender->getNickname() + "' disabled USER-LIMIT mode in channel: " + channelName + "\n", sender);
		std::cout << "Client (" << sender->getFd() << ") '" << sender->getNickname() << "' disabled user-limit mode in channel: " << channelName << "\n";
	}
	else
		sender->messageToMyself("~ ERROR: Invalid mode command. Modes are: +|- i, t, k, o, l\n");
}

void Server::commandREMOVE(Client *sender, const std::string &channelName, const std::string &param, const std::string &other)
{
	if (channelName.empty() || param.empty())//Verificar que ninguno de los parametros este vacio
		return(sender->messageToMyself("~ ERROR: Invalid REMOVE command syntax. Use: REMOVE <#channel> <topic/modes/invited>\n"));
	if (!other.empty())//Verificar ningun otra palabara detras de <param>
		return(sender->messageToMyself("~ ERROR: Command 'REMOVE' does not accept any more parameters than CHANNEL and a parameter. Use: REMOVE <#channel> <topic/modes/invited>\n"));
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender->messageToMyself("~ ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(sender))//Comprobar si estoy está dentro del canal
		return(sender->messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(sender))//Comprobar si soy operador
		return(sender->messageToMyself("~ ERROR: You are not an operator of channel: " + channelName + ".\n"));
	if (param == "topic")// Si el param es topic. Eliminar el topic
	{
		if (channel._topic.empty())// Verificar si el topic ya está vacío
			return(sender->messageToMyself("~ ERROR: Channel " + channelName + " has no topic to remove.\n"));
		channel._topic.clear();
		sender->messageToMyself("~ Topic removed from channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender->getNickname() + "' has removed the topic from channel: " + channelName + "\n", sender);
	}
	else if (param == "modes")// Si el param es modes. Desactivar todos los modos activos
	{
		if (!channel.inviteOnly && !channel.topicRestricted && channel.getKey().empty() && channel.getUserLimit() == 0)// Verificar si todos los modos están ya desactivados
			return(sender->messageToMyself("~ ERROR: All modes are already disabled in channel: " + channelName + "\n"));
		channel.setInviteOnly(false);
		channel.setTopicRestricted(false);
		channel.setKey("");
		channel.clearUserLimit();
		sender->messageToMyself("~ All modes have been cleared in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender->getNickname() + "' has cleared all modes in channel: " + channelName + "\n", sender);
	}
	else if (param == "invited")// Si el param es invited. Eliminar lista de inivitados
	{
		if (channel.invitedClients.empty())// Verificar si la lista de invitados ya está vacía
			return(sender->messageToMyself("~ ERROR: No invited users to remove in channel: " + channelName + "\n"));
		channel.invitedClients.clear();
		sender->messageToMyself("~ All invited users have been removed from channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender->getNickname() + "' has cleared the invited list in channel: " + channelName + "\n", sender);
	}
	else
		sender->messageToMyself("~ ERROR: Invalid parameter for REMOVE. Use: topic, modes, or invited.\n");
}
