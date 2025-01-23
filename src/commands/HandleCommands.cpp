#include "../../include/Server.hpp"

std::string to_string (int number)
{
	std::ostringstream oss;
	oss << number;
	return (oss.str());
}

void commandPART(Client &client, Server &server, const std::string &channelName, const std::string &other)
{
	if (channelName.empty())// Verificar que se ha especificado un canal
		return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: No channel name provided. Use: PART <#channel>\r\n"));
	if (channelName[0] != '#')//Verificar que el canal empieze por #
		return(client.messageToMyself(":ircserver 403 " + client.getNickname() + " ERROR: Channel name must start with '#'\r\n"));
	std::string parsedOther = other;
	if (other[0] == ':')
		parsedOther = other.substr(1);
	if (!parsedOther.empty() && other != ":Leaving")// Verificar ningun otra palabara detras del canal
		return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: Command 'PART' does not accept any more parameters than #CHANNEL. Use: PART <#channel>\r\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(client.messageToMyself(":ircserver 403 " + client.getNickname() + " ERROR: Channel " + channelName + " does not exist\r\n"));
	if (!channel.hasClient(&client))//Comprobar si el cliente está dentro del canal
		return(client.messageToMyself(":ircserver 442 " + client.getNickname() + " ERROR: You are not in channel: " + channelName + "\r\n"));
	if (channel.isOperator(&client))//Elimiar al usuario de la lista de operadores del canal si lo estaba
		channel.removeOperator(&client);
	client.messageToMyself(":" + client.getNickname() + " PART " + channelName + " ~ You left channel ~\r\n");
	channel.removeClientChannnel(&client);
	std::cout << "Client (" << client.getFd() << ") '" << client.getNickname() << "' left channel: " << channelName << "\r\n";
	channel.messageToGroupNoSender(":" + client.getNickname() + " PRIVMSG " + channelName + " ~ Left channel ~\r\n", &client);
	if (channel.getOperatorsNumber() == 0 && channel.getClientsNumber() > 0)// Si no hay operadores pero quedan clientes dentro del canal, asignar el operador al cliente más antiguo
	{
		Client *firstClient = *channel.getClients().begin();//Guardamos el cliente que hay en el principio del contenedor del canal
		channel.addOperator(firstClient);//Lo insertamos en el contenedor de operadores
		std::cout << "Client (" << firstClient->getFd() << ") '" << firstClient->getNickname() << "' is now an operator in channel: " << channelName << "\r\n";
		firstClient->messageToMyself(":" + firstClient->getNickname() + " PRIVMSG " + channelName + " ~ You are now an operator in channel ~\r\n");
		channel.messageToGroupNoSender(":" + firstClient->getNickname() + " PRIVMSG " + channelName + " ~ Is now an operator in channel ~\r\n", firstClient);
	}
	std::string userList = channelName + " :";
	for (std::set<Client*>::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)//Recorrer lista de clientes del canal
	{
		if (channel.isOperator(*it))//Si el cliente es operador, ponerle un @ delante
			userList += "@";
		userList += (*it)->getNickname() + " ";//Añadir cliente a la lista
	}
	channel.messageToGroup(":ircserver 353 " + client.getNickname() + " " + channelName + " " + userList + "\r\n");// Enviar la lista de usuarios (RPL_NAMREPLY 353)
	channel.messageToGroup(":ircserver 366 " + channelName + " " + channelName + "\r\n");// Fin de la lista de usuarios (RPL_ENDOFNAMES 366)
	if (channel.getClientsNumber() == 0)//Si no hay nadie más en el canal, eliminar canal
	{
		std::cout << "Channel: " << channelName << " was removed\n";
		channels.erase(it);
	}
}

void commandQUIT(Client &client, Server &server)
{
	std::map<std::string, Channel>& channels = server.getChannels();
	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end();)// Eliminar al cliente de los canales en los que está.
	{
		Channel &channel = it->second;
		std::string channelName = it->first;
		if (channel.isInvited(&client))//Eliminar al cliente de la lista de invitados si lo estaba (esto se hace ya que si entra otro con un fd invitado, sigue invitado)
			channel.removeInvitedClient(&client);
		if (channel.isBanned(&client))//Eliminar al cliente de la lista de baneados si lo estaba (esto se hace ya que si entra otro con un fd baneado, sigue baneado)
			channel.unbanClient(&client);
		if (channel.hasClient(&client))
		{
			commandPART(client, server, channelName, "");
			if (channels.find(channelName) == channels.end()) // Si `commandPART` eliminó el canal, actualizar el iterador.
				it = channels.begin(); // Reinicia iteración en caso de eliminación.
			else
				++it; // Avanza al siguiente canal si no fue eliminado.
		}
		else
			++it; // Avanza si el cliente no está en este canal.
	}
	int clientFd = client.getFd();
	if (!client.getNickname().empty())//Si el cliente tiene nickname
		std::cout << "Client (" << clientFd << ") '" << client.getNickname() << "' disconnected\n";
	else//si el cliente no tiene nickname
		std::cout << "Client (" << clientFd << ") disconnected\n";
	for (std::vector<pollfd>::iterator it = server.getPollFds().begin(); it != server.getPollFds().end(); ++it)// Recorrer pollFds para eliminar al cliente
	{
		if (it->fd == clientFd)
		{
			server.getPollFds().erase(it);
			delete (server.getClients()[clientFd]);//Eliminar el puntero de la memoria
			server.getClients().erase(clientFd);// Eliminar de la lista de clientes del servidor
			close(clientFd); // Cerrar el fd del cliente
			break;
		}
	}
}

void commandCOMMANDS(Client &client, const std::string &cmd, const std::string &other)
{
	std::string cmdUpper = cmd;
	for (std::size_t i = 0; i < cmd.size(); i++)
		cmdUpper[i] = toupper(cmd[i]);
	if (!other.empty())
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: Command 'COMMANDS' does not accept any more parameters than an existing command. Use: COMMANDS [cmd]\r\n"));
		return(client.messageToMyself(":ircserver 461 ERROR: Command 'COMMANDS' does not accept any more parameters than an existing command. Use: COMMANDS [cmd]\r\n"));
	}
	std::string QUIT = "~ QUIT: Disconnect from the server\n";
	if (cmdUpper == "QUIT")
		return(client.messageToMyself(QUIT));
	std::string PASS = "~ PASS <password>: Necessary to start using the chat\n";
	if (cmdUpper == "PASS")
		return(client.messageToMyself(PASS));
	std::string USER = "~ USER <username>: Register your username\n";
	if (cmdUpper == "USER")
		return(client.messageToMyself(USER));
	std::string NICK = "~ NICK <nickname>: Set your nickname\n";
	if (cmdUpper == "NICK")
		return(client.messageToMyself(NICK));
	std::string PROFILE = "~ PROFILE: Show your username and nickname\n";
	if (cmdUpper == "PROFILE")
		return(client.messageToMyself(PROFILE));
	std::string CHANNELS = "~ CHANNELS [all]: Show in which channels you are active. Type <all> to see all channels in server\n";
	if (cmdUpper == "CHANNELS")
		return(client.messageToMyself(CHANNELS));
	std::string MSG = "~ MSG <user/#channel> <message>: Send a message\n";
	if (cmdUpper == "MSG")
		return(client.messageToMyself(MSG));
	std::string JOIN = "~ JOIN <#channel> [key]: Join a channel\n";
	if (cmdUpper == "JOIN")
		return(client.messageToMyself(JOIN));
	std::string PART = "~ PART <#channel>: Disconnect from a channel\n";
	if (cmdUpper == "PART")
		return(client.messageToMyself(PART));
	std::string KICK = "~ KICK <#channel> <user> <reason>: For operators. Kick a user from a channel\n";
	if (cmdUpper == "KICK")
		return(client.messageToMyself(KICK));
	std::string INVITE = "~ INVITE <#channel> <user>: For operators. Invite a user to a channel\n";
	if (cmdUpper == "INVITE")
		return(client.messageToMyself(INVITE));
	std::string UNINVITE = "~ UNINVITE <#channel> <user>: For operators. Uninvite a user to a channel\n";
	if (cmdUpper == "UNINVITE")
		return(client.messageToMyself(UNINVITE));
	std::string TOPIC = "~ TOPIC <#channel> [new topic]: Show or set (if you add <new topic> and you are operator) a topic for a channel\n";
	if (cmdUpper == "TOPIC")
		return(client.messageToMyself(TOPIC));
	std::string KEY = "~ KEY <#channel>: For operators. Show the key from a channel\n";
	if (cmdUpper == "KEY")
		return(client.messageToMyself(KEY));
	std::string MODE = "~ MODE <#channel> [+|-mode] [param]: For operators. Change modes in a channel\n"
						"   * +|- i: Set/remove invite-only channel\n"
						"   * +|- t: Set/remove the restrictions of the topic command to channel operators\n"
						"   * +|- k: Set/remove the channel key (password)\n"
						"   * +|- o: Give/remove channel operator privileges\n"
						"   * +|- l: Set/remove the user limit to a channel\n"
						"   * +|- b: Ban/unban a user from a channel\n";
	if (cmdUpper == "MODE")
		return(client.messageToMyself(MODE));
	std::string REMOVE = "~ REMOVE <#channel> <topic/modes/invited/banned>: For operators. Remove topic, modes or the clients invited/banned list from a channel\n";
	if (cmdUpper == "REMOVE")
		return(client.messageToMyself(REMOVE));
	std::string COMMANDS = "~ COMMANDS [cmd]: Show instructions. Type a command <cmd> to see only its instructions\n";
	if (cmdUpper == "")
		return(client.messageToMyself(QUIT + PASS + USER + NICK + PROFILE + CHANNELS + MSG + JOIN + PART + KICK + INVITE + UNINVITE + TOPIC + KEY + MODE + REMOVE + COMMANDS));
	if (!client.getNickname().empty())
		return(client.messageToMyself(":ircserver 421 " + client.getNickname() + " ERROR: Command '" + cmdUpper + "' is not an existing command. Use: COMMANDS [cmd]\n"));
	return(client.messageToMyself(":ircserver 421 ERROR: Command '" + cmdUpper + "' is not an existing command. Use: COMMANDS [cmd]\n"));
}

void commandPASS(Client &client, Server &server, const std::string &pass, const std::string &other)
{
	if (client.getPasswordSent() == true)//Verificar que no se mande 2 veces la misma pass
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 462 " + client.getNickname() + " ERROR: 'PASS' command already sent\r\n"));
		return(client.messageToMyself(":ircserver 462 ERROR: 'PASS' command already sent\r\n"));
	}
	if (other != "")// Verificar ninguna otra palabara detras de la password
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: Command 'PASS' does not accept any more parameters than the PASSWORD. Use: PASS <password>\r\n"));
		return(client.messageToMyself(":ircserver 461 ERROR: Command 'PASS' does not accept any more parameters than the PASSWORD. Use: PASS <password>\r\n"));
	}
	if (pass == server.getPassword())// Verificar si la pass es correcta
	{
		std::cout << "Client (" << client.getFd() << ") accepted in server\n";
		client.setPasswordSent(true);
		client.messageToMyself("~ Correct password! You must now use commands USER <username> and NICK <nickname> to start using the server\n");
	}
	else //password incorrecta
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 464 " + client.getNickname() + " ERROR: Incorrect password. Use: PASS <password>\r\n"));
		return(client.messageToMyself(":ircserver 464 ERROR: Incorrect password. Use: PASS <password>\r\n"));
	}
}

void commandUSER(Client &client, const std::string &username, const std::string &other, const std::string &other2)
{
	if (username.empty())// Validar username no esta vacio
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: No username provided. Use: USER <username>\r\n"));
		return(client.messageToMyself(":ircserver 461 ERROR: No username provided. Use: USER <username>\r\n"));
	}
	if (other2 == "0 * :realname")
	{
		client.setUsername(username);
		return(client.messageToMyself("~ You setted your username to '" + username + "'\n"));
	}
	if (!other.empty()) // Verificar ningun otra palabara detras del username
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: Command 'USER' does not accept any more parameters than the USERNAME. Use: USER <username>\r\n"));
		return(client.messageToMyself(":ircserver 461 ERROR: Command 'USER' does not accept any more parameters than the USERNAME. Use: USER <username>\r\n"));
	}
	if (username.length() > 9)// Verificar que no sea mas largo de 9 caracteres
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 436 " + client.getNickname() + " ERROR: Username cannot be longer than 9 characters\r\n"));
		return(client.messageToMyself(":ircserver 436 ERROR: Username cannot be longer than 9 characters\r\n"));
	}
	client.setUsername(username);
	client.messageToMyself("~ You setted your username to '" + username + "'\n");
}

void commandNICK(Client &client, Server &server, const std::string &nickname, const std::string &other)
{
	if (nickname.empty())// Validar nickname no esta vacio
		return(client.messageToMyself(":ircserver 431 ERROR: No nickname provided. Use: NICK <nickname>\r\n"));
	if (!other.empty()) // Verificar ningun otra palabara detras del nickname
		return(client.messageToMyself(":ircserver 461 ERROR: Command 'NICK' does not accept any more parameters than the NICKNAME. Use: NICK <nickname>\n"));
	for (int i = 0; nickname[i]; i++)
	{
		if (nickname[i] == '#' || nickname[i] == '@' || nickname[i] == '!' || nickname[i] == '?')
		{
			if (!client.getNickname().empty())
				return(client.messageToMyself(":ircserver 999 " + client.getNickname() + " ERROR: Nickname cannot include especial characters like: #, @, !, ?\r\n"));
			return(client.messageToMyself(":ircserver 999 ERROR: Nickname cannot include especial characters like: #, @, !, ?\r\n"));
		}
	}
	if (nickname.length() > 9)// Verificar que no sea mas largo de 9 caracteres
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 436 " + client.getNickname() + " ERROR: Nickname cannot be longer than 9 characters\r\n"));
		return(client.messageToMyself(":ircserver 436 ERROR: Nickname cannot be longer than 9 characters\r\n"));
	}
	if (nickname == "bot" || nickname == "BOT")
	{
		if (!client.getNickname().empty())
			return(client.messageToMyself(":ircserver 436 " + client.getNickname() + " ERROR: Nickname cannot be '" + nickname + "'\r\n"));
		return(client.messageToMyself(":ircserver 436 ERROR: Nickname cannot be '" + nickname + "'r\n"));
	}
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		Client *clients = it->second;
		if (clients->getNickname() == nickname)// Verificar si el nickname ya está en uso
		{
			if (!client.getNickname().empty())
				return(client.messageToMyself(":ircserver 433 " + client.getNickname() + " ERROR: Nickname '" + nickname + "' is already in use\r\n"));// El nickname ya está en uso.
			return(client.messageToMyself(":ircserver 433 ERROR: Nickname '" + nickname + "' is already in user\n"));
		}
	}
	std::string oldNickname = client.getNickname(); //Guardar el nickname anterior
	client.setNickname(nickname);// Asignar el nickname al cliente.
	if (oldNickname.empty()) //Si es el primer nickname que se pone
	{
		std::cout << "Client (" << client.getFd() << ") setted his nickname to '" << client.getNickname() << "'\n";
		return(client.messageToMyself("~ You setted your nickname to '" + nickname + "'\n"));
	}
	client.messageToMyself("~ You changed your nickname from '" + oldNickname + "' to '" + nickname + "'\n");
	std::cout << "Client (" << client.getFd() << ") '" << oldNickname << "' changed his nickname to '" << client.getNickname() << "'\n";
	std::map<std::string, Channel>& channels = server.getChannels();
	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end();it++)
	{
		Channel &channel = it->second;
		std::string channelName = it->first;
		if (channel.hasClient(&client))
			channel.messageToGroupNoSender(":" + client.getNickname() + " PRIVMSG " + channelName + " ~ Changed his nickname to '" + nickname + "' ~\r\n", &client);
	}
}

void commandPROFILE(Client &client, const std::string &param)
{
	if (!param.empty())// Verificar ningun otra palabara detras de PROFILE
			return(client.messageToMyself("~ ERROR: Command 'PROFILE' does not accept any parameters\n"));
	client.messageToMyself("~ Your profile information:\n");
	client.messageToMyself("	- Username: " + client.getUsername() + "\n");
	client.messageToMyself("	- Nickname: " + client.getNickname() + "\n");
}

void commandCHANNELS(Client &client, Server &server, const std::string &param, const std::string &other)
{
	if ((param != "all" && param != "") || !other.empty())//Verificar ningun otra palabara detras de CHANNELS
		return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: Command 'CHANNELS' does not accept any more parameters than ALL. Use: CHANNELS [all]\r\n"));
	if (server.getChannels().empty())//Si no hay canales creados
		return(client.messageToMyself("~ No channels are currently available\n"));
	if (param == "all")//Si se quieren ver todos los canales que existen
	{
		client.messageToMyself("~ List of all channels:\n");
		std::map<std::string, Channel>& channels = server.getChannels();
		for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); it++)//Recorrer todos los canales que hay ya creados
		{
			const std::string &channelName = it->first;
			const Channel &channel = it->second;
			client.messageToMyself("	* " + channelName);
			if (!channel.getModes().empty())// Mostrar los modos activos solo si hay alguno
				client.messageToMyself("[" + channel.getModes() + "]");
			if (!channel.isTopicEmpty()) //Mostrar el topic si no está vacío
				client.messageToMyself("[Topic: " + channel.getTopic() + "]");
			client.messageToMyself("(" + to_string(channel.getClientsNumber()) + " member(s)): ");
			for (std::set<Client *>::iterator clientIt = channel.getClients().begin(); clientIt != channel.getClients().end(); ++clientIt)//Recorrer todos los clientes que hay en ese canal
			{
				std::set<Client *>::iterator nextIt = clientIt; 
				++nextIt;//Nos guardamos el siguiente del cliente al que estamos comprobando, para poder ver si es el ultimo que queda del canal por mostrar
				if (nextIt == channel.getClients().end())//Si es el ultimo, salto de linea
					client.messageToMyself((*clientIt)->getNickname() + "\n");
				else//Si no es el ultimo, ponemos una coma
					client.messageToMyself((*clientIt)->getNickname() + ", ");
			}
		}
	}
	else //Si se quiere ver solo los canales en los que yo estoy
	{
		int flag = 0;
		std::map<std::string, Channel>& channels = server.getChannels();
		for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
		{
			const std::string &channelName = it->first;
			const Channel &channel = it->second;
			if (channel.hasClient(&client))
			{
				if (flag == 0)
				{
					flag = 1;
					client.messageToMyself("~ You are currently active in channel(s):\n");
				}
				client.messageToMyself("	* " + channelName);
				if (!channel.getModes().empty())// Mostrar los modos activos solo si hay alguno
					client.messageToMyself("[" + channel.getModes() + "]");
				if (!channel.isTopicEmpty()) //Mostrar el topic si no está vacío
					client.messageToMyself("[Topic: " + channel.getTopic() + "]");
				client.messageToMyself("(" + to_string(channel.getClientsNumber()) + " member(s)): ");
				if (channel.getClientsNumber() == 1)
					client.messageToMyself(client.getNickname() + "\n");
				else
				{
					for (std::set<Client *>::iterator clientIt = channel.getClients().begin(); clientIt != channel.getClients().end(); ++clientIt)
					{
						if ((*clientIt)->getNickname() != client.getNickname())
						{
							std::set<Client *>::iterator nextIt = clientIt;
							++nextIt;
							std::set<Client *>::iterator nextnextIt = nextIt;
							++nextnextIt;
							if (nextIt == channel.getClients().end() || ((*nextIt)->getNickname() == client.getNickname() && nextnextIt == channel.getClients().end()))
								client.messageToMyself((*clientIt)->getNickname() + " and " + client.getNickname() + "\n");
							else if (nextIt != channel.getClients().end())
								client.messageToMyself((*clientIt)->getNickname() + ", ");
						}
					}
				}
			}
		}
		if (flag == 0)
			client.messageToMyself("~ You are not active in any channels\n");
	}
}

void commandPRIVMSG(Client &sender, Server &server, const std::string &receiver, const std::string &message, const std::string &other)
{
	std::cout << "1\n";
	if (receiver.empty() || message.empty())//Verificar que el destinatario y el mensaje no esten vacios
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid MSG format. Use MSG <receiver> <message>\n"));
	if (receiver == sender.getNickname())//Verificar que no me este mandando un mensaje a mi mismo
		return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + "  ERROR: You cannot send a message to yourself ('" + sender.getNickname() + "')\n"));
	if (receiver == "bot" || receiver == "BOT")
	{
		if (!other.empty())
			return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Command 'MSG BOT' does not accept any more parameters than help/joke/play. Use: MSG bot help/joke/play\n"));
		std::string newMessage = message;
		if (message[0] == ':')
			newMessage = message.substr(1);
		sender.messageToMyself(":bot PRIVMSG ");
		return(server.getHandleBotCommand(sender, newMessage));
	}
	if (receiver[0] == '#')// Si el receptor es un canal
	{
		std::map<std::string, Channel>& channels = server.getChannels();
		std::map<std::string, Channel>::iterator it = channels.find(receiver);
		Channel &channel = it->second;
		if (it == channels.end())//Comprobar si el canal existe
			return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel " + receiver + " does not exist\n"));
		if (!channel.hasClient(&sender))// Comprobar si el que envia no está en el canal
			return(sender.messageToMyself(":ircserver 404 " + sender.getNickname() + " ERROR: To send a message in channel: " + channel.getName() + " you have to JOIN it first\n"));
		if (message[0] == ':')
		{
			std::string newMessage = message.substr(1);
			return(channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + receiver + " " + newMessage + "\r\n", &sender));
		}
		return(channel.messageToGroup(":" + sender.getNickname() + " PRIVMSG " + receiver + " " + message + "\r\n"));
	}
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)//Recorrer todos los clientes que hay en el servidor
	{
		Client *destinatary = it->second;
		std::cout << "2\n";
		if (destinatary && destinatary->getNickname() == receiver)//Si encuentra al destinatario en el servidor, le manda el mensaje
		{
			std::cout << "3\n";
			if (message[0] == ':')
			{
				std::string newMessage = message.substr(1);
				return(destinatary->messageToMyself(":" + sender.getNickname() + " PRIVMSG " + receiver + " " + newMessage + "\r\n"));
			}
			else
				return(destinatary->messageToMyself(":" + sender.getNickname() + " PRIVMSG " + receiver + " " + message + "\r\n"));
		}
	}
	sender.messageToMyself(":ircserver 401 " + sender.getNickname() + " ERROR: User '" + receiver + "' does not exist\n");// Si no se encuentra el receptor
}

void commandJOIN(Client &client, Server &server, const std::string &channelName, const std::string &key, const std::string &other)
{
	if (channelName.empty())// Verificar que se ha especificado un canal
		return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: No channel name provided. Use: JOIN <#channel> [key]\n"));
	if (channelName[0] != '#')//Verificar que el canal empieze por #
		return(client.messageToMyself(":ircserver 421 " + client.getNickname() + " ERROR: Channel name must start with '#'\n"));
	if (channelName.size() < 2)
		return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: Channel name cannot be empty\n"));
	if (!other.empty())
		return(client.messageToMyself(":ircserver 461 " + client.getNickname() + " ERROR: Command 'JOIN' does not accept any more parameters than #CHANNEL and KEY. Use: JOIN <#channel> [key]\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	if (channels.find(channelName) == channels.end())// Comprueba si el canal no existe en el contenedor channels
	{
		channels.insert(std::make_pair(channelName, Channel(channelName)));// Crea el canal con su nombre y un objeto de la clase channel (utilizando el constructor), y lo inserta en el contendor channels
        Channel &channel = channels.find(channelName)->second;// Accede al canal (->second es el objeto de la clase channel)
		channel.addClient(&client);//Añadir al cliente al canal
		channel.addOperator(&client); // Cliente se convierte en operador inicial.
		std::cout << "Client (" << client.getFd() << ") '" << client.getNickname() << "' created and joined channel: " << channelName << "\n";//Mensaje en servidor
		client.messageToMyself(":" + client.getNickname() + " JOIN " + channelName + " ~ You have created and joined channel ~\r\n");//Mensaje al cliente
		if (!key.empty())//Si se puso algo detrás de #channel
			client.messageToMyself(":ircserver 0 " + client.getNickname() + " " + channelName + " ~ Does not need a key to enter ~\r\n");//Mensaje al cliente
	}
	else if(channels.find(channelName) != channels.end())// Si el canal ya existe
	{
		Channel &channel = channels.find(channelName)->second;// Accede al canal (->second es el objeto de la clase channel)
		if (channel.hasClient(&client))// Comprobar si el cliente ya está en el canal
			return(client.messageToMyself(":ircserver 0 " + client.getNickname() + " " + channelName + " ~ You are already in channel~ \n"));
		if (channel.isBanned(&client))// Verificar si el cliente está baneado
			return(client.messageToMyself(":ircserver 465 " + client.getNickname() + " ERROR: You are banned in channel: " + channelName + "\n"));
		if (channel.isInviteOnly() && !channel.isInvited(&client))// Si el canal está en modo invite-only, verificar si el cliente ha sido invitado
			return(client.messageToMyself(":ircserver 473 " + client.getNickname() + " ERROR: Channel: " + channelName + " is on INVITE-ONLY mode\n"));
		if (channel.getUserLimit() > 0 && channel.getUserLimit() <= channel.getClientsNumber())// Comprobar si hay límite de usuarios y si ya se ha alcanzado el máximo
			return(client.messageToMyself(":ircserver 471 " + client.getNickname() + " ERROR: Channel: " + channelName + " is full\n"));
		if (!channel.getKey().empty() && !channel.isInvited(&client))// Comprobar si el canal está en modo key y no estoy invitado
		{
			if (key.empty())//Si no he puesto ninguna key
				return(client.messageToMyself(":ircserver 475 " + client.getNickname() + " ERROR: Channel: " + channelName + " is in KEY mode. Use: JOIN <#channel> <key>\n"));
			if (key != channel.getKey())//Si la key es incorrecta
				return(client.messageToMyself(":ircserver 475 " + client.getNickname() + " ERROR: Incorrect key\n"));
		}
		channel.addClient(&client);//Añade al cliente.
		std::cout << "Client (" << client.getFd() << ") '" << client.getNickname() << "' joined channel: " << channelName << "\n";//Mensaje en servidor
		client.messageToMyself(":" + client.getNickname() + " JOIN " + channelName + " ~ You joined channel ~\r\n");//Mensaje al cliente
		channel.messageToGroupNoSender(":" + client.getNickname() + " PRIVMSG " + channelName + " ~ Joined channel ~\r\n", &client);//Mensaje al grupo
		if (channel.getKey().empty() && !key.empty())//Si no esta en modo key y yo he puesto una contraseña
			client.messageToMyself(":ircserver 0 " + client.getNickname() + " " + channelName + " ~ Does not need a key to enter ~\r\n");
		if (!channel.getKey().empty() && !key.empty() && channel.isInvited(&client))//Si esta en modo key y yo he puesto una key, pero estoy invitado
			client.messageToMyself(":ircserver 0 " + client.getNickname() + " " + channelName + " ~ You do not need a key to enter in channel beacause you are invited ~\r\n");
	}
	Channel &channel = channels.find(channelName)->second;// Accede al canal (->second es el objeto de la clase channel)
	if (!channel.isTopicEmpty())// Si el canal tiene topic
		client.messageToMyself(":ircserver 332 " + client.getNickname() + " " + channelName + " :" + channel.getTopic() + "\r\n");// Enviar el TEMA actual del canal (RPL_TOPIC 332)
	std::string userList = channelName + " :";
	for (std::set<Client*>::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)//Recorrer lista de clientes del canal
	{
		if (channel.isOperator(*it))//Si el cliente es operador, ponerle un @ delante
			userList += "@";
		userList += (*it)->getNickname() + " ";//Añadir cliente a la lista
	}
	channel.messageToGroup(":ircserver 353 " + client.getNickname() + " " + channelName + " " + userList + "\r\n");// Enviar la lista de usuarios (RPL_NAMREPLY 353)
	channel.messageToGroup(":ircserver 366 " + channelName + " " + channelName + "\r\n");// Fin de la lista de usuarios (RPL_ENDOFNAMES 366)
}

void commandKICK(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &reason)
{
	if (channelName.empty() || user.empty())//Verificar que ninguno de los parametros este vacio
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid KICK command syntax. Use: KICK <#channel> <user> <reason>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself(":ircserver 442 " + sender.getNickname() + " ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself(":ircserver 482 " + sender.getNickname() + " ERROR: You are not an operator of channel: " + channelName + "\n"));
	if (user == sender.getNickname())//Verificar que no me este kickeando a mi mismo
		return(sender.messageToMyself(":ircserver 400 " + sender.getNickname() + " ERROR: You cannot kick yourself ('" + sender.getNickname() + "') from a channel\n"));
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a kickear
		{
			if (channel.hasClient(client))//Si el usuario al que se kickea está dentro del canal
			{
				if (channel.isInvited(client))//Eliminar al usuario de la lista de invitados del canal si lo estaba
					channel.removeInvitedClient(client);
				if (channel.isOperator(client))//Elimiar al usuario de la lista de operadores del canal si lo estaba
					channel.removeOperator(client);
				std::string reasonParsed = reason;
				if (!reason.empty() && reason[0] == ':')
					reasonParsed = reason.substr(1);
				if (!reason.empty() && reasonParsed != user)
					client->messageToMyself(":ircserver 0 " + client->getNickname() + " " + channelName + " ~ You have been kicked from channel by '" + sender.getNickname() + "'. Reason: " + reasonParsed + " ~\r\n");
				else
					client->messageToMyself(":ircserver 0 " + client->getNickname() + " " + channelName + " ~ You have been kicked from channel by '" + sender.getNickname() + "' ~\r\n");
				if (reasonParsed != user)
					client->messageToMyself(":" + sender.getNickname() + " KICK " + channelName + " " + user + " :" + reasonParsed + "\r\n");
				else
					client->messageToMyself(":" + sender.getNickname() + " KICK " + channelName + " " + user + "\r\n");
				channel.removeClientChannnel(client);//Eliminar al usuario del canal
				std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' has kicked Client (" << client->getFd() << ") '" << client->getNickname() << "' from channel " << channelName << "\n";
				channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Has kicked '" + client->getNickname() + "' from channel ~\r\n", &sender);//Mensaje al grupo
				sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You have kicked '" + user + "' from channel ~\n");
				std::string userList = channelName + " :";
				for (std::set<Client*>::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)//Recorrer lista de clientes del canal
				{
					if (channel.isOperator(*it))//Si el cliente es operador, ponerle un @ delante
						userList += "@";
					userList += (*it)->getNickname() + " ";//Añadir cliente a la lista
				}
				channel.messageToGroup(":ircserver 353 " + sender.getNickname() + " " + channelName + " " + userList + "\r\n");// Enviar la lista de usuarios (RPL_NAMREPLY 353)
				return(channel.messageToGroup(":ircserver 366 " + channelName + " " + channelName + "\r\n"));// Fin de la lista de usuarios (RPL_ENDOFNAMES 366)
				
			}
			else//Si no se cumple, ese usuario ya estaba fuera del canal
				return(sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + user + "' is not in channel: " + channelName + "\n"));
		}
	}
	sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + user + "' does not exist\n");//El usuario no esta en el servidor
}

void commandINVITE(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &other)
{
	if (channelName.empty() || user.empty())//Verificar que ninguno de los parametros este vacio
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid INVITE command syntax. Use: INVITE <user> <#channel>\n"));
	if (!other.empty())//Verificar ningun otra palabara detras de <user>
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Command 'INVITE' does not accept any more parameters than CHANNEL and USER. Use: INVITE <#channel> <user>\r\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself(":ircserver 442 " + sender.getNickname() + " ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself(":ircserver 482 " + sender.getNickname() + " ERROR: You are not an operator of channel: " + channelName + "\n"));
	if (user == sender.getNickname())//Verificar que no me este invitando a mi mismo
		return(sender.messageToMyself(":ircserver 400 " + sender.getNickname() + " ERROR: You cannot invite yourself ('" + sender.getNickname() + "') to a channel\n"));
	if (channel.getUserLimit() > 0 && channel.getUserLimit() <= channel.getClientsNumber())//Verificar si el canal esta lleno
		return(sender.messageToMyself(":ircserver 471 " + sender.getNickname() + " ERROR: Channel: " + channelName + " is full\n"));
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a invitar
		{
			if (channel.isInvited(client))//Verificar si ya está en la lista de invitados
				return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: User '" + user + "' is already in the clients invited list in channel: " + channelName + "\n"));
			if (!channel.hasClient(client))//Si el usuario al que se invita no esta ya dentro del canal
			{
				channel.inviteClient(client);//Invitar al usuario al canal
				std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' invited Client (" << client->getFd() << ") '" << client->getNickname() << "' to channel: " << channelName << "\n";
				sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You invited '" + user + "' ~\n");
				return(client->messageToMyself("~ '" + sender.getNickname() + "' invited you to channel: " + channelName + ". Use: JOIN <#channel> [key]\n"));
			}
			else//Si no se cumple, ese usuario ya estaba dentro del canal
				return(sender.messageToMyself(":ircserver 443 " + sender.getNickname() + " ERROR: User '" + user + "' is already in channel: " + channelName + "\n"));
		}
	}
	sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + user + "' does not exist\n");//El usuario no esta en el servidor
}

void commandUNINVITE(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &other)
{
	if (channelName.empty() || user.empty())//Verificar que ninguno de los parametros este vacio
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid UNINVITE command syntax. Use: UNINVITE <user> <#channel>\n"));
	if (!other.empty())//Verificar ningun otra palabara detras de <user>
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Command 'UNINVITE' does not accept any more parameters than CHANNEL and USER. Use: UNINVITE <#channel> <user>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself(":ircserver 442 " + sender.getNickname() + " ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself(":ircserver 482 " + sender.getNickname() + " ERROR: You are not an operator of channel: " + channelName + "\n"));
	if (user == sender.getNickname())//Verificar que no me este desinvitando a mi mismo
		return(sender.messageToMyself(":ircserver 400 " + sender.getNickname() + " ERROR: You cannot uninvite yourself ('" + sender.getNickname() + "') to a channel\n"));
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a invitar
		{
			if (!channel.isInvited(client))//Verificar si no está en la lista de invitados
				return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: User '" + user + "' is not in the clients invited list in channel: " + channelName + "\n"));
			channel.removeInvitedClient(client);//Eliminar al usuario de la lista de invitados del canal
			std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' uninvited Client (" << client->getFd() << ") '" << client->getNickname() << "' to channel: " << channelName << "\n";
			sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You uninvited '" + user + "' ~\n");
			return(client->messageToMyself("~ '" + sender.getNickname() + "' uninvited you to channel: " + channelName + ". You are not in the invited list anymore\n"));
		}
	}
	sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + user + "' does not exist\n");//El usuario no esta en el servidor
}

void commandTOPIC(Client &sender, Server &server, const std::string &channelName, const std::string &topic)
{
	if (channelName.empty())//Comprobar si solo se ha escrito topic
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid TOPIC command syntax. Use: TOPIC <#channel> [new topic]\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel: " + channelName + " does not exist\n"));
	Channel &channel = it->second;
	if (topic.empty())//Comprobar si solamente quiero ver el topic
	{
		if (channel.isTopicEmpty())//Comprobar si el canal no tiene topic
			return(sender.messageToMyself(":ircserver 331 " + sender.getNickname() + " " + channelName + " ~ Has no TOPIC yet ~\r\n"));
		return(sender.messageToMyself(":ircserver 332 " + sender.getNickname() + " " + channelName + " '" + channel.getTopic() + "'\r\n"));
	}
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself(":ircserver 442 " + sender.getNickname() + " ERROR: You are not in channel: " + channelName + "\n"));
	if ((channel.isTopicRestricted() && !channel.isOperator(&sender)))//Comprobar si el canal esta +t y yo no soy operador
		return(sender.messageToMyself(":ircserver 482 " + sender.getNickname() + " ERROR: You are not allowed to change the topic in channel: " + channelName + ". Channel is in MODE +t and you are not an operator\n"));
	std::string topicParsed = topic;
	if (topic[0] == ':')
		topicParsed = topic.substr(1);
	if (topicParsed == "REMOVE" || topicParsed == "remove")// Eliminar el tema del canal
	{
		if (channel.getTopic() == "")//Comprobar si estoy intentando eliminar el topic en un canal que no tiene topic
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " has no TOPIC\n"));
		channel.setTopic("");
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' removed TOPIC in channel: " << channelName << "\n";
		sender.messageToMyself(":ircserver 332 " + sender.getNickname() + " " + channelName + " " + channel.getTopic() + "\r\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Removed TOPIC in channel ~\r\n", &sender);
		return(channel.messageToGroupNoSender(":ircserver 332 " + sender.getNickname() + " " + channelName + " " + channel.getTopic() + "\r\n", &sender));
	}
	if (channel.getTopic() == topicParsed)//Comprobar si estoy intentando poner un topic en un canal que ya tiene ese mismo topic
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: TOPIC in channel: " + channelName + " is already '" + topicParsed + "'\n"));
	channel.setTopic(topicParsed);
	std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' changed TOPIC to '" << topicParsed << "' in channel: " << channelName << "\n";
	sender.messageToMyself(":ircserver 332 " + sender.getNickname() + " " + channelName + " '" + channel.getTopic() + "'\r\n");
	channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Changed TOPIC to '" + topicParsed + "' in channel ~\r\n", &sender);
	channel.messageToGroupNoSender(":ircserver 332 " + sender.getNickname() + " " + channelName + " '" + channel.getTopic() + "'\r\n", &sender);
}

void commandKEY(Client &sender, Server &server, const std::string &channelName, const std::string &other)
{
	if (channelName.empty())// Verificar que se ha especificado un canal
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid KEY command syntax. Use: KEY <#channel>\n"));
	if (!other.empty())
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Command 'KEY' does not accept any more parameters than CHANNEL. Use: KEY <#channel>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself(":ircserver 442 " + sender.getNickname() + " ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself(":ircserver 482 " + sender.getNickname() + " ERROR: You are not an operator of channel: " + channelName + "\n"));
	if(channel.getKey() == "")//Comprobar si el acanal tiene key
		return(sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ Channel has no key ~\n"));
	sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ Channel has '" + channel.getKey() + "' as a key ~\n");
}

void commandMODE(Client &sender, Server &server, const std::string &channelName, const std::string &mode, const std::string &param)
{
	if (channelName.empty())//Verificar que canal no esté vacío
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid MODE command syntax. Use: MODE <channel> [+|-mode] [param]\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel: " + channelName + " does not exist\n"));
	if (mode.empty())// Si no se especifica modo, mostrar los modos activos del canal
	{
		if (!channel.getModes().empty())
			return(sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ Active mode(s) for channel: " + channel.getModes() + " ~\r\n"));
		return(sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ No active modes for channel ~\r\n"));
	}
	if (!param.empty() && (mode == "+i" || mode == "-i" || mode == "+t" || mode == "-t" || mode  == "-l" || mode  == "-k"))
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Command 'MODE " + mode +"' does not accept any more parameters than CHANNEL and <+|-mode>. Use: MODE <channel> <+|-mode>\n"));
	size_t spacePos = param.find(' ');
	if (spacePos != std::string::npos)
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Command 'MODE " + mode +"' does not accept any more parameters than CHANNEL, <+|-mode> and [param]. Use: MODE <channel> <+|-mode> [param]\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself(":ircserver 442 " + sender.getNickname() + " ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself(":ircserver 482 " + sender.getNickname() + " ERROR: You are not an operator of channel: " + channelName + "\n"));
	if (mode == "+o")// Asignar privilegio de operador
	{
		if (param == "")
			return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: You need to specify a user for MODE +o\n"));
		std::map<int, Client*>& clients = server.getClients();
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			Client *client = clientIt->second;
			if (client->getNickname() == param)
			{
				if (channel.hasClient(client))
				{
					if (channel.isOperator(client))
						return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: '" + param + "' is already an operator in channel: " + channelName + "\n"));
					channel.addOperator(client);
					std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' gave operator privileges to '" << param << "' in channel: " << channelName << "\n";
					sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You gave OPERATOR PRIVILEGES to '" + param + "' in channel ~\n");
					channel.messageToGroupNoSenderNoReceiver(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Gave OPERATOR PRIVILEGES to '" + param + "' in channel ~\r\n", &sender, client);//Mensaje al grupo
					client->messageToMyself(":ircserver 0 " + client->getNickname() + " " + channelName + " ~ '" + sender.getNickname() + "' gave you OPERATOR PRIVILEGES in channel ~\n");
					std::string userList = channelName + " :";
					for (std::set<Client*>::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)//Recorrer lista de clientes del canal
					{
						if (channel.isOperator(*it))//Si el cliente es operador, ponerle un @ delante
							userList += "@";
						userList += (*it)->getNickname() + " ";//Añadir cliente a la lista
					}
					channel.messageToGroup(":ircserver 353 " + sender.getNickname() + " " + channelName + " " + userList + "\r\n");// Enviar la lista de usuarios (RPL_NAMREPLY 353)
					return(channel.messageToGroup(":ircserver 366 " + channelName + " " + channelName + "\r\n"));// Fin de la lista de usuarios (RPL_ENDOFNAMES 366)
				}
				else
					return(sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + param + "' is not in channel: " + channelName + "\n"));
			}
		}
		sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + param + "' does not exist\n");//El usuario no esta en el servidor

	}
	else if (mode == "-o")// Eliminar privilegio de operador
	{
		if (param == "")
			return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: You need to specify a user for MODE -o\n"));
		std::map<int, Client*>& clients = server.getClients();
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			Client *client = clientIt->second;
			if (client->getNickname() == param)
			{
				if (channel.hasClient(client))
				{
					if (!channel.isOperator(client))
						return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: '" + param + "' is not an operator in channel: " + channelName + "\n"));
					if (channel.getOperatorsNumber() == 1)// Si soy el unico operador, que no me deje quitarmelo
						return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: You are the only operator in channel: " + channelName + "\n"));
					channel.removeOperator(client);
					std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' removed operator privileges to '" << param << "' in channel: " << channelName << "\n";
					sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You remove OPERATOR PRIVILEGES to '" + param + "' in channel ~\n");
					channel.messageToGroupNoSenderNoReceiver(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Removed OPERATOR PRIVILEGES to '" + param + "' in channel ~\r\n", &sender, client);//Mensaje al grupo
					client->messageToMyself(":ircserver 0 " + client->getNickname() + " " + channelName + " ~ '" + sender.getNickname() + "' removed your OPERATOR PRIVILEGES in channel ~\n");
					std::string userList = channelName + " :";
					for (std::set<Client*>::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)//Recorrer lista de clientes del canal
					{
						if (channel.isOperator(*it))//Si el cliente es operador, ponerle un @ delante
							userList += "@";
						userList += (*it)->getNickname() + " ";//Añadir cliente a la lista
					}
					channel.messageToGroup(":ircserver 353 " + sender.getNickname() + " " + channelName + " " + userList + "\r\n");// Enviar la lista de usuarios (RPL_NAMREPLY 353)
					return(channel.messageToGroup(":ircserver 366 " + channelName + " " + channelName + "\r\n"));// Fin de la lista de usuarios (RPL_ENDOFNAMES 366)
				}
				else
					return(sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + param + "' is not in channel: " + channelName + "\n"));
			}
		}
		sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + param + "' does not exist\n");//El usuario no esta en el servidor

	}
	else if (mode == "+i")//Establecer solo invitados pueden unirse al canal
	{
		if (channel.inviteOnly == true)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " is already on INVITE-ONLY mode\n"));
		channel.setInviteOnly(true);
		for (std::set<Client *>::iterator itClient = channel.getClients().begin(); itClient != channel.getClients().end(); ++itClient)// Agregar a todos los clientes actuales a la lista de invitados
			if (!channel.isInvited(*itClient))// Verifica si ya está invitado
				channel.inviteClient(*itClient);// Agrega a la lista de invitados
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You enabled INVITE-ONLY mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Enabled INVITE-ONLY mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' enabled invite-only mode in channel: " << channelName << "\n";
	}
	else if (mode == "-i")//Eliminar solo invitados pueden unirse al canal
	{
		if (channel.inviteOnly == false)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " is not on INVITE-ONLY mode\n"));
		channel.setInviteOnly(false);
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You disabled INVITE-ONLY mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Disabled INVITE-ONLY mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' disabled invite-only mode in channel: " << channelName << "\n";
	}
	else if (mode == "+t")//Establecer topic solo lo pueden cambiar operadores
	{
		if (channel.topicRestricted == true)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " is already on TOPIC-RESTRICTED mode\n"));
		channel.setTopicRestricted(true);
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You enabled TOPIC-RESTRICTED mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Enabled TOPIC-RESTRICTED mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' enabled topic-restricted mode in channel: " << channelName << "\n";
	}
	else if (mode == "-t")//Eliminar topic solo lo pueden cambiar operadores
	{
		if (channel.topicRestricted == false)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " is not on TOPIC-RESTRICTED mode\n"));
		channel.setTopicRestricted(false);
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You disabled TOPIC-RESTRICTED mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Disabled TOPIC-RESTRICTED mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' disabled topic-restricted mode in channel: " << channelName << "\n";
	}
	else if (mode == "+k")//Establecer contraseña al canal
	{
		if (param == "")
			return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: You need to specify a key for MODE +k\n"));
		if (param.size() > 9)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Key cannot be longer than 9 characters\n"));
		if(channel.getKey() == param)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: KEY is already '" + param + "' in channel\n"));
		channel.setKey(param);
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You enabled KEY (" + param + ") mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Enabled KEY (" + param + ") mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' enabled key (" << param << ") mode in channel: " << channelName << "\n";
	}
	else if (mode == "-k")//Eliminar contraseña del canal
	{
		if(channel.isKeyEmpty())
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " is not on KEY mode\n"));
		channel.clearKey();
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You disabled KEY mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Disabled KEY mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' disabled key mode in channel: " << channelName << "\n";
	}
	else if (mode == "+l")//Establecer numero maximo de usuarios en el canal
	{
		if (param == "")
			return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: You need to specify a number for MODE +l\n"));
		for (std::size_t i = 0; i < param.size(); i++)
			if (!std::isdigit(param[i]))
				return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Limit cannot be something different from a positive number\n"));
		size_t limit = std::atoi(param.c_str());
		if (limit == 0 || limit == 1)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Limit cannot be less than 2\n"));
		if (limit < channel.getClientsNumber())
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Limit cannot be less than the channel's actual number of members (" + to_string(channel.getClientsNumber()) + ")\n"));
		if (limit == channel.getUserLimit())
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " is already limited to " + param + "\n"));
		channel.setUserLimit(limit);
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You enabled USER-LIMIT (" + param + ") mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Enabled USER-LIMIT (" + param + ") mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' enabled user-limit (" << param << ") mode in channel: " << channelName << "\n";
	}
	else if (mode == "-l")//Eliminar numero maximo de usuarios en el canal
	{
		if (channel.getUserLimit() == 0)
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel: " + channelName + " already has no users limit\n"));
		channel.clearUserLimit();
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You disabled USER-LIMIT mode in channel ~\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Disabled USER-LIMIT mode in channel ~\r\n", &sender);//Mensaje al grupo
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' disabled user-limit mode in channel: " << channelName << "\n";
	}
	else if (mode == "+b")
	{
		if (param == "")
		{
			if (channel.getBannedClients().empty()) // Verificar si no hay baneados
				return;
			for (std::set<Client*>::const_iterator it = channel.getBannedClients().begin(); it != channel.getBannedClients().end(); ++it)// Recorrer la lista de usuarios baneados y enviar un mensaje por cada uno
				sender.messageToMyself(":ircserver 367 " + sender.getNickname() + " " + channelName + " " + (*it)->getNickname() + " " + sender.getNickname() + "\r\n");
			return;
		}
		if (param == sender.getNickname())//Verificar que no me este baneando a mi mismo
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: You cannot ban yourself ('" + sender.getNickname() + "') from a channel\n"));
		std::map<int, Client*>& clients = server.getClients();
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			Client *client = clientIt->second;
			if (client->getNickname() == param)
			{
				if (channel.isBanned(client))// Verificar si ya está baneado
					return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: '" + param + "' is already banned in channel: " + channelName + "\n"));
				if (channel.isInvited(client))//Eliminar al usuario de la lista de invitados del canal si lo estaba
					channel.removeInvitedClient(client);
				channel.banClient(client);// Banear al usuario
				std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' has banned Client (" << client->getFd() << ") '" << client->getNickname() << "' from channel " << channelName << "\n";
				sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You have BANNED '" + param + "' from channel ~\n");
				channel.messageToGroupNoSenderNoReceiver(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Has BANNED '" + param + "' from channel ~\r\n", &sender, client);//Mensaje al grupo
				if (!channel.hasClient(client))
					client->messageToMyself(":ircserver 0 " + client->getNickname() + " " + channelName + " ~ '" + sender.getNickname() + "' has BANNED you from channel ~\n");
				if (channel.hasClient(client))//Eliminar al usuario del canal si está dentro
				{
					channel.removeClientChannnel(client);
					client->messageToMyself(":" + client->getNickname() + " PART " + channelName + " ~ '" + sender.getNickname() + "' has BANNED you from channel ~\r\n");
					if (channel.isOperator(client))//Elimiar al usuario de la lista de operadores del canal si lo estaba
						channel.removeOperator(client);
					std::string userList = channelName + " :";
					for (std::set<Client*>::const_iterator it = channel.getClients().begin(); it != channel.getClients().end(); ++it)//Recorrer lista de clientes del canal
					{
						if (channel.isOperator(*it))//Si el cliente es operador, ponerle un @ delante
							userList += "@";
						userList += (*it)->getNickname() + " ";//Añadir cliente a la lista
					}
					channel.messageToGroup(":ircserver 353 " + sender.getNickname() + " " + channelName + " " + userList + "\r\n");// Enviar la lista de usuarios (RPL_NAMREPLY 353)
					channel.messageToGroup(":ircserver 366 " + channelName + " " + channelName + "\r\n");// Fin de la lista de usuarios (RPL_ENDOFNAMES 366)
				}
				return;
			}
		}
		sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + param + "' does not exist\n");//El usuario no esta en el servidor
	}
	else if (mode == "-b")
	{
		if (param == "")
			return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: You need to specify a user for MODE -b\n"));
		if (param == sender.getNickname())//Verificar que no me este desbaneando a mi mismo
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: You cannot unban yourself ('" + sender.getNickname() + "') from a channel\n"));
		std::map<int, Client*>& clients = server.getClients();
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			Client *client = clientIt->second;
			if (client->getNickname() == param)
			{
				if (!channel.isBanned(client))// Verificar que no está baneado ya
					return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: User '" + param + "' is not banned in " + channelName + ".\n"));
				channel.unbanClient(client);// Desbanear al usuario
				std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' has unbanned Client (" << client->getFd() << ") '" << client->getNickname() << "' from channel " << channelName << "\n";
				sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You have UNBANNED '" + param + "' from channel ~\n");
				channel.messageToGroupNoSenderNoReceiver(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Has UNBANNED '" + param + "' from channel ~\r\n", &sender, client);//Mensaje al grupo
				return(client->messageToMyself(":ircserver 0 " + client->getNickname() + " " + channelName + " ~ '" + sender.getNickname() + "' has UNBANNED you from channel ~\n"));
			}
		}
		sender.messageToMyself(":ircserver 441 " + sender.getNickname() + " ERROR: User '" + param + "' does not exist\n");//El usuario no esta en el servidor
	}
	else
		sender.messageToMyself(":ircserver 472 " + sender.getNickname() + " ERROR: Invalid mode command. Modes are: +|- i, t, k, o, l, b\r\n");
}

void commandREMOVE(Client &sender, Server &server, const std::string &channelName, const std::string &param, const std::string &other)
{
	if (channelName.empty() || param.empty())//Verificar que ninguno de los parametros este vacio
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Invalid REMOVE command syntax. Use: REMOVE <#channel> <topic/modes/invited/banned>\n"));
	if (!other.empty())//Verificar ningun otra palabara detras de <param>
		return(sender.messageToMyself(":ircserver 461 " + sender.getNickname() + " ERROR: Command 'REMOVE' does not accept any more parameters than CHANNEL and a parameter. Use: REMOVE <#channel> <topic/modes/invited/banned>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself(":ircserver 403 " + sender.getNickname() + " ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself(":ircserver 442 " + sender.getNickname() + " ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself(":ircserver 482 " + sender.getNickname() + " ERROR: You are not an operator of channel: " + channelName + "\n"));
	if (param == "topic")// Si el param es topic. Eliminar el topic
	{
		if (channel.isTopicEmpty())// Verificar si el topic ya está vacío
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel " + channelName + " has no topic to remove\n"));
		channel.clearTopic();
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You removed topic from channel ~\r\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Has removed the topic from channel ~\r\n", &sender);//Mensaje al grupo
	}
	else if (param == "modes")// Si el param es modes. Desactivar todos los modos activos
	{
		if (!channel.inviteOnly && !channel.topicRestricted && channel.getKey().empty() && channel.getUserLimit() == 0)// Verificar si todos los modos están ya desactivados
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel " + channelName + " has no modes to remove\n"));
		channel.setInviteOnly(false);
		channel.setTopicRestricted(false);
		channel.setKey("");
		channel.clearUserLimit();
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You removed all modes from channel ~\r\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Has removed all modes from channel ~\r\n", &sender);//Mensaje al grupo
	}
	else if (param == "invited")// Si el param es invited. Eliminar lista de inivitados
	{
		if (channel.isInvitedListEmpty())// Verificar si la lista de invitados ya está vacía
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel " + channelName + " has no invited users list to remove\n"));
		channel.clearInvitedClients();
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You removed the invited users list from channel ~\r\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Has removed the invited users list from channel ~\r\n", &sender);//Mensaje al grupo
	}
	else if (param == "banned")// Si el param es banned. Eliminar lista de baneados
	{
		if (channel.isBannedListEmpty())// Verificar si la lista de baneados ya está vacía
			return(sender.messageToMyself(":ircserver 999 " + sender.getNickname() + " ERROR: Channel " + channelName + " has no banned users list to remove\n"));
		channel.clearBannedClients();
		sender.messageToMyself(":ircserver 0 " + sender.getNickname() + " " + channelName + " ~ You removed the banned users list from channel ~\r\n");
		channel.messageToGroupNoSender(":" + sender.getNickname() + " PRIVMSG " + channelName + " ~ Has removed the banned users list from channel ~\r\n", &sender);//Mensaje al grupo
	}
	else
		sender.messageToMyself(":ircserver 421 " + sender.getNickname() + " ERROR: Invalid parameter for REMOVE. Use: topic, modes, invited or banned\r\n");
}

void Server::handleCommand(Client &client, Server &server, const std::string &cmd, const std::string &param, const std::string &paramraw2, const std::string &param2, const std::string &param3)
{
	if (cmd == "CAP")
		return(client.messageToMyself("CAP * LS\r\n"));
	if (cmd == "QUIT")// Sintaxis: QUIT
		commandQUIT(client, server);
	else if (cmd == "VIVA")
		client.messageToMyself("~ ESPAÑA\n");
	else if (cmd == "COMMANDS")// Sintaxis: COMMANDS [cmd]
		commandCOMMANDS(client, param, param2);
	else if (cmd == "PASS")// Sintaxis: PASS <password>
		commandPASS(client, server, param, param2);
	else if (!client.getPasswordSent()) //Obligar a que el primer comando que se debe introducir es pass
		client.messageToMyself(":ircserver 999 " + client.getNickname() + " ERROR: You must authenticate with 'PASS' before any other command. Use: PASS <password>\r\n");
	else if (cmd == "USER")// Sintaxis: USER <username> 
		commandUSER(client, param, param2, paramraw2);
	else if (cmd == "NICK")// Sintaxis: NICK <nickname>
		commandNICK(client, server, param, param2);
	else if (client.getNickname().empty() || client.getUsername().empty())// Obligar a que haya un nickname para poder hacer el resto de comandos
		client.messageToMyself(":ircserver 444 " + client.getNickname() + " ERROR: First you have to use commands 'NICK' and 'USER' and specify your nickname and username. Use: NICK <nickname> and USER <username>\r\n");
	else if (cmd == "PROFILE")
		commandPROFILE(client, param);
	else if (cmd == "CHANNELS")// Sintaxis: CHANNELS [all]
		commandCHANNELS(client, server, param, param2);
	else if (cmd == "MSG" ||  cmd == "PRIVMSG")// Sintaxis: MSG <user/#channel> <message>
		commandPRIVMSG(client, server, param, paramraw2, param3);
	else if (cmd == "JOIN")// Sintaxis: JOIN <#channel> [key]. 
		commandJOIN(client, server, param, param2, param3);
	else if (cmd == "WHO")
		return;
	else if (cmd == "PART")// Sintaxis: PART <#channel>
		commandPART(client, server, param, param2);
	else if (cmd == "KICK")// Sintaxis: KICK <#channel> <user> <reason>
		commandKICK(client, server, param, param2, param3);
	else if (cmd == "INVITE")// Sintaxis: INVITE <#channel> <user> 
		commandINVITE(client, server, param, param2, param3);
	else if (cmd == "UNINVITE")// Sintaxis: UNINVITE <#channel> <user> 
		commandUNINVITE(client, server, param, param2, param3);
	else if (cmd == "TOPIC")// Sintaxis: TOPIC <#channel> [new topic]
		commandTOPIC(client, server, param, paramraw2);
	else if (cmd == "KEY")// Sintaxis: KEY <#channel>
		commandKEY(client, server, param, param2);
	else if (cmd == "MODE")// Sintaxis: MODE <#channel> [+|-mode] [arg]
		commandMODE(client, server, param, param2, param3);
	else if (cmd == "REMOVE")// Sintaxis: REMOVE <#channel> <topic/modes/invited/banned> 
		commandREMOVE(client, server, param, param2, param3);
	else
		client.messageToMyself(":ircserver 421 " + client.getNickname() + " ERROR: Unknown command: " + cmd + "\r\n");
}