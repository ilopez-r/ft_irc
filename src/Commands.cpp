/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 00:44:46 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/13 00:44:49 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Commands.hpp"

Commands::Commands()
{

}

Commands::~Commands()
{

}

void Commands::handleCommand(Client &client, Server &server, const std::string &cmd, const std::string &param, const std::string &paramraw2, const std::string &param2, const std::string &param3)
{
	if (cmd == "QUIT")// Sintaxis: QUIT
		commandQUIT(client, server, param);
	else if (cmd == "VIVA")
		client.messageToMyself("~ ESPAÑA\n");
	else if (cmd == "HELP")// Sintaxis: HELP [cmd]
		commandHELP(client, param, param2);
	else if (cmd == "PASS")// Sintaxis: PASS <password>
		commandPASS(client, server, param, param2);
	else if (!client.getPasswordSent()) //Obligar a que el primer comando que se debe introducir es pass
		client.messageToMyself("~ ERROR: You must authenticate with 'PASS' before any other command. Use: PASS <password>\n");
	else if (cmd == "USER")// Sintaxis: USER <username> 
		commandUSER(client, param, param2);
	else if (cmd == "NICK")// Sintaxis: NICK <nickname>
		commandNICK(client, server, param, param2);
	else if (client.getNickname().empty())// Obligar a que haya un nickname para poder hacer el resto de comandos
		client.messageToMyself("~ ERROR: First you have to use command 'NICK' and specify your nickname. Use: NICK <nickname>\n");
	else if (cmd == "PROFILE")
		commandPROFILE(client, param);
	else if (cmd == "CHANNELS")// Sintaxis: CHANNELS [all]
		commandCHANNELS(client, server, param, param2);
	else if (cmd == "MSG")// Sintaxis: MSG <user/#channel> <message>
		commandMSG(client, server, param, paramraw2);
	else if (cmd == "JOIN")// Sintaxis: JOIN <#channel> [key]. 
		commandJOIN(client, server, param, param2, param3);
	else if (cmd == "LEAVE")// Sintaxis: LEAVE <#channel>
		commandLEAVE(client, server, param, param2);
	else if (cmd == "KICK")// Sintaxis: KICK <#channel> <user> <reason>
		commandKICK(client, server, param, param2, param3);
	else if (cmd == "BAN")// Sintaxis: BAN <#channel> <user> <reason>
		commandBAN(client, server, param, param2, param3);
	else if (cmd == "UNBAN")// Sintaxis: UNBAN <#channel> <user>
		commandUNBAN(client, server, param, param2, param3);
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
		client.messageToMyself("~ Unknown command: " + cmd + "\n");
}

std::string Commands::to_string (int number) const
{
	std::ostringstream oss;
	oss << number;
	return (oss.str());
}

void Commands::commandQUIT(Client &client, Server &server, const std::string &param)
{
	if (!param.empty())// Verificar ningun otra palabara detras de QUIT
			return(client.messageToMyself("~ ERROR: Command 'QUIT' does not accept any parameters\n"));
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
			commandLEAVE(client, server, channelName, "");
			if (channels.find(channelName) == channels.end()) // Si `commandLEAVE` eliminó el canal, actualizar el iterador.
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
	close(clientFd); // Cerrar el fd del cliente
	delete server.getClients()[clientFd];//Eliminar el puntero de la memoria
	server.getClients().erase(clientFd);// Eliminar de la lista de clientes del servidor
	for (std::vector<pollfd>::iterator it = server.getPollFds().begin(); it != server.getPollFds().end(); ++it)// Recorrer pollFds para eliminar al cliente
	{
		if (it->fd == clientFd)
		{
			server.getPollFds().erase(it);
			break;
		}
	}
}

void Commands::commandHELP(Client &client, const std::string &cmd, const std::string &other)
{
	std::string cmdUpper = cmd;
	for (std::size_t i = 0; i < cmd.size(); i++)
		cmdUpper[i] = toupper(cmd[i]);
	if (!other.empty())
		return(client.messageToMyself("~ ERROR: Command 'HELP' does not accept any more parameters than an existing command. Use: HELP [cmd]\n"));
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
	std::string LEAVE = "~ LEAVE <#channel>: Disconnect from a channel\n";
	if (cmdUpper == "LEAVE")
		return(client.messageToMyself(LEAVE));
	std::string KICK = "~ KICK <#channel> <user> <reason>: For operators. Kick a user from a channel\n";
	if (cmdUpper == "KICK")
		return(client.messageToMyself(KICK));
	std::string BAN = "~ BAN <#channel> <user> <reason>: For operators. BAN a user from a channel\n";
	if (cmdUpper == "BAN")
		return(client.messageToMyself(BAN));
	std::string UNBAN = "~ UNBAN <#channel> <user>: For operators. UNBAN a user from a channel\n";
	if (cmdUpper == "UNBAN")
		return(client.messageToMyself(UNBAN));
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
						"	* +|- i: Set/remove invite-only channel\n"
						"	* +|- t: Set/remove the restrictions of the topic command to channel operators\n"
						"	* +|- k: Set/remove the channel key (password)\n"
						"	* +|- o: Give/remove channel operator privileges\n"
						"	* +|- l: Set/remove the user limit to a channel\n";
	if (cmdUpper == "MODE")
		return(client.messageToMyself(MODE));
	std::string REMOVE = "~ REMOVE <#channel> <topic/modes/invited/banned>: For operators. Remove topic, modes or the clients invited/banned list from a channel\n";
	if (cmdUpper == "REMOVE")
		return(client.messageToMyself(REMOVE));
	std::string HELP = "~ HELP [cmd]: Show instructions. Type a command <cmd> to see only its instructions\n";
	if (cmdUpper == "")
		return(client.messageToMyself(QUIT + PASS + USER + NICK + PROFILE + CHANNELS + MSG + JOIN + LEAVE + KICK + BAN + UNBAN + INVITE + UNINVITE + TOPIC + KEY + MODE + REMOVE + HELP));
	return(client.messageToMyself("~ ERROR: Command '" + cmdUpper + "' is not an existing command. Use: HELP [cmd]\n"));
}

void Commands::commandPASS(Client &client, Server &server, const std::string &pass, const std::string &other)
{
	if (client.getPasswordSent() == true)//Verificar que no se mande 2 veces la misma pass
			return(client.messageToMyself("~ ERROR: 'PASS' command already sent\n"));
	if (other != "")// Verificar ninguna otra palabara detras de la password
		return(client.messageToMyself("~ ERROR: Command 'PASS' does not accept any more parameters than the PASSWORD. Use: PASS <password>\n"));
	if (pass == server.getPassword())// Verificar si la pass es correcta
	{
		std::cout << "Client (" << client.getFd() << ") accepted in server\n";
		client.setPasswordSent(true);
		client.messageToMyself("~ Correct password! You must now use commands USER <username> and NICK <nickname> to start using the server\n");
	}
	else //password incorrecta
		client.messageToMyself("~ ERROR: Incorrect password. Use: PASS <password>\n");
}

void Commands::commandUSER(Client &client, const std::string &username, const std::string &other)
{
	if (username.empty())// Validar username no esta vacio
		return(client.messageToMyself("~ ERROR: No username provided. Use: USER <username>\n"));
	if (!other.empty()) // Verificar ningun otra palabara detras del username
		return(client.messageToMyself("~ ERROR: Command 'USER' does not accept any more parameters than the USERNAME. Use: USER <username>\n"));
	if (username.length() > 9)// Verificar que no sea mas largo de 9 caracteres	
		return(client.messageToMyself("~ ERROR: Username cannot be longer than 9 characters\n"));
	client.setUsername(username);
	client.messageToMyself("~ You setted your username to '" + username + "'\n");
}

void Commands::commandNICK(Client &client, Server &server, const std::string &nickname, const std::string &other)
{
	if (client.getUsername() == "")// Verificar que se ha introducido un username
		return(client.messageToMyself("~ ERROR: First you have to use command 'USER' and specify your username. Use: USER <username>\n"));
	if (nickname.empty())// Validar nickname no esta vacio
		return(client.messageToMyself("~ ERROR: No nickname provided. Use: NICK <nickname>\n"));
	if (!other.empty()) // Verificar ningun otra palabara detras del nickname
		return(client.messageToMyself("~ ERROR: Command 'NICK' does not accept any more parameters than the NICKNAME. Use: NICK <nickname>\n"));
	if (nickname.length() > 9)// Verificar que no sea mas largo de 9 caracteres	
		return(client.messageToMyself("~ ERROR: Nickname cannot be longer than 9 characters\n"));
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		Client *clients = it->second;
		if (clients->getNickname() == nickname)// Verificar si el nickname ya está en uso
			return(client.messageToMyself("~ ERROR: Nickname '" + nickname + "' is already in use\n")); // El nickname ya está en uso.
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
			channel.messageToGroupNoSender("~ [" + channelName + "]: '" + oldNickname + "' changed his nickname to '" + nickname + "'\n", &client);
	}
}

void Commands::commandPROFILE(Client &client, const std::string &param)
{
	if (!param.empty())// Verificar ningun otra palabara detras de PROFILE
			return(client.messageToMyself("~ ERROR: Command 'PROFILE' does not accept any parameters\n"));
	client.messageToMyself("~ Your profile information:\n");
	client.messageToMyself("	- Username: " + client.getUsername() + "\n");
	client.messageToMyself("	- Nickname: " + client.getNickname() + "\n");
}

void Commands::commandCHANNELS(Client &client, Server &server, const std::string &param, const std::string &other)
{
	if ((param != "all" && param != "") || !other.empty())//Verificar ningun otra palabara detras de CHANNELS
		return(client.messageToMyself("~ ERROR: Command 'CHANNELS' does not accept any more parameters than ALL. Use: CHANNELS [all]\n"));
	if (server.getChannels().empty())//Si no hay canales creados
		return(client.messageToMyself("~ No channels are currently available\n"));
	if (param == "all")//Si se quieren ver todos los canales que existen
	{
		client.messageToMyself("~ List of all channels:\n");
		std::map<std::string, Channel>& channels = server.getChannels();
		for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); it++)//Recorrer todos los acanales que hay ya creados
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

void Commands::commandMSG(Client &sender, Server &server, const std::string &receiver, const std::string &message)
{
	if (receiver.empty() || message.empty())//Verificar que el destinatario y el mensaje no esten vacios
		return(sender.messageToMyself("~ ERROR: Invalid MSG format. Use MSG <receiver> <message>\n"));
	if (receiver == sender.getNickname())//Verificar que no me este mandando un mensaje a mi mismo
		return(sender.messageToMyself("~ ERROR: You cannot send a message to yourself ('" + sender.getNickname() + "')\n"));
	if (receiver[0] == '#')// Si el receptor es un canal
	{
		std::map<std::string, Channel>& channels = server.getChannels();
		std::map<std::string, Channel>::iterator it = channels.find(receiver);
		Channel &channel = it->second;
		if (it == channels.end())//Comprobar si el canal existe
			return(sender.messageToMyself("~ ERROR: Channel " + receiver + " does not exist\n"));
		if (!channel.hasClient(&sender))// Comprobar si el que envia no está en el canal
			return(sender.messageToMyself("~ ERROR: To send a message in channel: " + channel.getName() + " you have to JOIN it first\n"));
		if (channel.isOperator(&sender))//Comprobar si soy operador
			return(channel.messageToGroup("~ [" + channel.getName() + "][OP] " + sender.getNickname() + ": " + message + "\n"));
		else// Enviar mensaje al canal
			return(channel.messageToGroup("~ [" + channel.getName() + "] " + sender.getNickname() + ": " + message + "\n"));
	}
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)//Recorrer todos los clientes que hay en el servidor
	{
		Client *destinatary = it->second;
		if (destinatary->getNickname() == receiver)//Si encuentra al destinatario en el servidor, le manda el mensaje
			return(sender.messageToSomeone("~ [PRV] " + sender.getNickname() + ": " + message + "\n", destinatary));
	}
	sender.messageToMyself("~ ERROR: User '" + receiver + "' does not exist\n");// Si no se encuentra el receptor
}

void Commands::commandJOIN(Client &client, Server &server, const std::string &channelName, const std::string &key, const std::string &other)
{
	if (channelName.empty())// Verificar que se ha especificado un canal
		return(client.messageToMyself("~ ERROR: No channel name provided. Use: JOIN <#channel> [key]\n"));
	if (channelName[0] != '#')//Verificar que el canal empieze por #
		return(client.messageToMyself("~ ERROR: Channel name must start with '#'\n"));
	if (channelName.size() < 2)
		return(client.messageToMyself("~ ERROR: Channel name cannot be empty\n"));
	if (!other.empty())
		return(client.messageToMyself("~ ERROR: Command 'JOIN' does not accept any more parameters than #CHANNEL and KEY. Use: JOIN <#channel> [key]\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	if (channels.find(channelName) == channels.end())// Comprueba si el canal no existe en el contenedor channels
	{
		channels.insert(std::make_pair(channelName, Channel(channelName)));// Crea el canal con su nombre y un objeto de la clase channel (utilizando el constructor), y lo inserta en el contendor channels
        Channel &channel = channels.find(channelName)->second;// Accede al canal (->second es el objeto de la clase channel)
		channel.addClient(&client);//Añadir al cliente al canal
		channel.addOperator(&client); // Cliente se convierte en operador inicial.
		std::cout << "Client (" << client.getFd() << ") '" << client.getNickname() << "' created and joined channel: " << channelName << "\n";//Mensaje en servidor
		client.messageToMyself("~ You have created and joined channel: " + channelName + "\n");//Mensaje al cliente
		if (!key.empty())//Si se puso algo detrás de #channel
			client.messageToMyself("~ Channel: " + channelName + " does not need a key to enter\n");//Mensaje al cliente
		return;
	}
	// Si el canal ya existe
	Channel &channel = channels.find(channelName)->second;// Accede al canal (->second es el objeto de la clase channel)
	if (channel.hasClient(&client))// Comprobar si el cliente ya está en el canal
		return(client.messageToMyself("~ ERROR: You are already in channel: " + channelName + "\n"));
	if (channel.isBanned(&client))// Verificar si el cliente está baneado
		return(client.messageToMyself("~ ERROR: You are banned in channel: " + channelName + "\n"));
	if (channel.isInviteOnly() && !channel.isInvited(&client))// Si el canal está en modo invite-only, verificar si el cliente ha sido invitado
		return(client.messageToMyself("~ ERROR: Channel: " + channelName + " is on INVITE-ONLY mode\n"));
	if (channel.getUserLimit() > 0 && channel.getUserLimit() <= channel.getClientsNumber())// Comprobar si hay límite de usuarios y si ya se ha alcanzado el máximo
		return(client.messageToMyself("~ ERROR: Channel: " + channelName + " is full\n"));
	if (channel.getKey().empty() && !key.empty())//Si no esta en modo key y yo he puesto una contraseña
		client.messageToMyself("~ Channel: " + channelName + " does not need a key to enter\n");
	if (!channel.getKey().empty() && !key.empty() && channel.isInvited(&client))//Si esta en modo key y yo he puesto una key, pero estoy invitado
		client.messageToMyself("~ You do not need a key to enter in channel: " + channelName + " beacause you are invited\n");
	if (!channel.getKey().empty() && !channel.isInvited(&client))// Comprobar si el canal está en modo key y no estoy invitado
	{
		if (key.empty())//Si no he puesto ninguna key
			return(client.messageToMyself("~ ERROR: Channel: " + channelName + " is in KEY mode. Use: JOIN <#channel> <key>\n"));
		if (key != channel.getKey())//Si la key es incorrecta
			return(client.messageToMyself("~ ERROR: Incorrect key\n"));
	}
	channel.addClient(&client);//Añade al cliente.
	std::cout << "Client (" << client.getFd() << ") '" << client.getNickname() << "' joined channel: " << channelName << "\n";//Mensaje en servidor
	client.messageToMyself("~ You joined channel: " + channelName + "\n");//Mensaje al cliente
	channel.messageToGroupNoSender("~ '" + client.getNickname() + "' joined channel: " + channelName + "\n", &client);//Mensaje al grupo
}

void Commands::commandLEAVE(Client &client, Server &server, const std::string &channelName, const std::string &other)
{
	if (channelName.empty())// Verificar que se ha especificado un canal
		return(client.messageToMyself("~ ERROR: No channel name provided. Use: LEAVE <#channel>\n"));
	if (channelName[0] != '#')//Verificar que el canal empieze por #
		return(client.messageToMyself("~ ERROR: Channel name must start with '#'\n"));
	if (!other.empty())// Verificar ningun otra palabara detras del canal
		return(client.messageToMyself("~ ERROR: Command 'LEAVE' does not accept any more parameters than #CHANNEL. Use: LEAVE <#channel>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(client.messageToMyself("~ ERROR: ERROR: Channel " + channelName + " does not exist\n"));
	if (!channel.hasClient(&client))//Comprobar si el cliente está dentro del canal
		return(client.messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (channel.isOperator(&client))//Elimiar al usuario de la lista de operadores del canal si lo estaba
		channel.removeOperator(&client);
	channel.removeClientChannnel(&client);
	std::cout << "Client (" << client.getFd() << ") '" << client.getNickname() << "' left channel: " << channelName << "\n";
	client.messageToMyself("~ You left channel: " + channelName + "\n");
	channel.messageToGroupNoSender("~ '" + client.getNickname() + "' left channel: " + channelName + "\n", &client);
	if (channel.getOperatorsNumber() == 0 && channel.getClientsNumber() > 0)// Si no hay operadores pero quedan clientes dentro del canal, asignar el operador al cliente más antiguo
	{
		Client *firstClient = *channel.getClients().begin();//Guardamos el cliente que hay en el principio del contenedor del canal
		channel.addOperator(*channel.getClients().begin());//Lo insertamos en el contenedor de operadores
		std::cout << "Client (" << firstClient->getFd() << ") '" << firstClient->getNickname() << "' is now an operator in channel: " << channelName << "\n";
		firstClient->messageToMyself("~ You are now an operator in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + firstClient->getNickname() + "' is now an operator in channel: " + channelName + "\n", firstClient);
	}
	if (channel.getClientsNumber() == 0)//Si no hay nadie más en el canal, eliminar canal
	{
		std::cout << "Channel: " << channelName << " was removed\n";
		channels.erase(it);
	}
}

void Commands::commandKICK(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &reason)
{
	if (channelName.empty() || user.empty() || reason.empty())//Verificar que ninguno de los parametros este vacio
		return(sender.messageToMyself("~ ERROR: Invalid KICK command syntax. Use: KICK <#channel> <user> <reason>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself("~ ERROR: You are not an operator of channel: " + channelName + ".\n"));
	if (user == sender.getNickname())//Verificar que no me este kickeando a mi mismo
		return(sender.messageToMyself("~ ERROR: You cannot kick yourself ('" + sender.getNickname() + "') from a channel\n"));
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
				channel.removeClientChannnel(client);//Eliminar al usuario del canal
				std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' has kicked Client (" << client->getFd() << ") '" << client->getNickname() << "' from channel " << channelName << "\n";
				channel.messageToGroupNoSender("~ '" + user + "' has been kicked by '" + sender.getNickname() + "' from channel " + channelName + "\n", &sender);
				sender.messageToMyself("~ You have kicked '" + user + "' from " + channelName + "\n");
				return(client->messageToMyself("~ You have been kicked from " + channelName + " by '" + sender.getNickname() + "'. Reason: " + reason + "\n"));
			}
			else//Si no se cumple, ese usuario ya estaba fuera del canal
				return(sender.messageToMyself("~ ERROR: User '" + user + "' is not in channel: " + channelName + "\n"));
		}
	}
	sender.messageToMyself("~ ERROR: User '" + user + "' does not exist\n"); //El usuario no esta en el servidor
}

void Commands::commandBAN(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &reason)
{
	if (channelName.empty() || user.empty() || reason.empty())//Verificar que ninguno de los parametros este vacio
		return(sender.messageToMyself("~ ERROR: Invalid BAN command syntax. Use: BAN <#channel> <user> <reason>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself("~ ERROR: You are not an operator of channel: " + channelName + ".\n"));
	if (user == sender.getNickname())//Verificar que no me este baneando a mi mismo
		return(sender.messageToMyself("~ ERROR: You cannot ban yourself ('" + sender.getNickname() + "') from a channel\n"));
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a banear
		{
			if (channel.isBanned(client))// Verificar si ya está baneado
				return(sender.messageToMyself("~ ERROR: User '" + user + "' is already banned in " + channelName + ".\n"));
			if (channel.isInvited(client))//Eliminar al usuario de la lista de invitados del canal si lo estaba
				channel.removeInvitedClient(client);
			if (channel.isOperator(client))//Elimiar al usuario de la lista de operadores del canal si lo estaba
				channel.removeOperator(client);
			if (channel.hasClient(client))//Eliminar al usuario del canal si está dentro
				channel.removeClientChannnel(client);
			channel.banClient(client);// Banear al usuario
			std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' has banned Client (" << client->getFd() << ") '" << client->getNickname() << "' from channel " << channelName << "\n";
			channel.messageToGroupNoSender("~ '" + user + "' has been banned by '" + sender.getNickname() + "' from channel " + channelName + "\n", &sender);
			sender.messageToMyself("~ You have banned '" + user + "' from " + channelName + "\n");
			return(client->messageToMyself("~ You have been banned from " + channelName + " by '" + sender.getNickname() + "'. Reason: " + reason + "\n"));
		}
	}
	sender.messageToMyself("~ ERROR: User '" + user + "' does not exist\n"); //El usuario no esta en el servidor
}

void Commands::commandUNBAN(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &other)
{
	if (channelName.empty() || user.empty())//Verificar que ninguno de los parametros este vacio
		return(sender.messageToMyself("~ ERROR: Invalid UNBAN command syntax. Use: UNBAN <#channel> <user>\n"));
	if (!other.empty())//Verificar ningun otra palabara detras de <user>
		return(sender.messageToMyself("~ ERROR: Command 'UNBAN' does not accept any more parameters than CHANNEL and USER. Use: UNBAN <#channel> <user>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself("~ ERROR: You are not an operator of channel: " + channelName + ".\n"));
	if (user == sender.getNickname())//Verificar que no me este desbaneando a mi mismo
		return(sender.messageToMyself("~ ERROR: You cannot unban yourself ('" + sender.getNickname() + "') from a channel\n"));
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a banear
		{
			if (!channel.isBanned(client))// Verificar que no está baneado
				return(sender.messageToMyself("~ ERROR: User '" + user + "' is not banned in " + channelName + ".\n"));
			channel.unbanClient(client);// Desbanear al usuario
			std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' has unbanned Client (" << client->getFd() << ") '" << client->getNickname() << "' from channel " << channelName << "\n";
			channel.messageToGroupNoSender("~ '" + user + "' has been unbanned by '" + sender.getNickname() + "' from channel " + channelName + "\n", &sender);
			sender.messageToMyself("~ You have unbanned '" + user + "' from " + channelName + "\n");
			return(client->messageToMyself("~ You have been unbanned from " + channelName + " by '" + sender.getNickname() + "\n"));
		}
	}
	sender.messageToMyself("~ ERROR: User '" + user + "' does not exist\n"); //El usuario no esta en el servidor
}

void Commands::commandINVITE(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &other)
{
	if (channelName.empty() || user.empty())//Verificar que ninguno de los parametros este vacio
		return(sender.messageToMyself("~ ERROR: Invalid INVITE command syntax. Use: INVITE <user> <#channel>\n"));
	if (!other.empty())//Verificar ningun otra palabara detras de <user>
		return(sender.messageToMyself("~ ERROR: Command 'INVITE' does not accept any more parameters than CHANNEL and USER. Use: INVITE <#channel> <user>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself("~ ERROR: You are not an operator of channel: " + channelName + ".\n"));
	if (user == sender.getNickname())//Verificar que no me este invitando a mi mismo
		return(sender.messageToMyself("~ ERROR: You cannot invite yourself ('" + sender.getNickname() + "') to a channel\n"));
	if (channel.getUserLimit() > 0 && channel.getUserLimit() <= channel.getClientsNumber())//Verificar si el canal esta lleno
		return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " is already full\n"));
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a invitar
		{
			if (channel.isInvited(client))//Verificar si ya está en la lista de invitados
				return(sender.messageToMyself("~ ERROR: User '" + user + "' is already in the clients invited list in channel: " + channelName + "\n"));
			if (!channel.hasClient(client))//Si el usuario al que se invita no esta ya dentro del canal
			{
				channel.inviteClient(client);//Invitar al usuario al canal
				std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' invited Client (" << client->getFd() << ") '" << client->getNickname() << "' to channel: " << channelName << "\n";
				sender.messageToMyself("~ You invited '" + user + "' to channel: " + channelName + ".\n");
				return(client->messageToMyself("~ '" + sender.getNickname() + "' invited you to channel: " + channelName + ". Use: JOIN <#channel> [key]\n"));
			}
			else//Si no se cumple, ese usuario ya estaba dentro del canal
				return(sender.messageToMyself("~ ERROR: User '" + user + "' is already in channel: " + channelName + "\n"));
		}
	}
	sender.messageToMyself("~ ERROR: User '" + user + "' does not exist\n"); //El usuario no esta en el servidor
}

void Commands::commandUNINVITE(Client &sender, Server &server, const std::string &channelName, const std::string &user, const std::string &other)
{
	if (channelName.empty() || user.empty())//Verificar que ninguno de los parametros este vacio
		return(sender.messageToMyself("~ ERROR: Invalid UNINVITE command syntax. Use: UNINVITE <user> <#channel>\n"));
	if (!other.empty())//Verificar ningun otra palabara detras de <user>
		return(sender.messageToMyself("~ ERROR: Command 'UNINVITE' does not accept any more parameters than CHANNEL and USER. Use: UNINVITE <#channel> <user>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself("~ ERROR: You are not an operator of channel: " + channelName + ".\n"));
	if (user == sender.getNickname())//Verificar que no me este invitando a mi mismo
		return(sender.messageToMyself("~ ERROR: You cannot invite yourself ('" + sender.getNickname() + "') to a channel\n"));
	std::map<int, Client*>& clients = server.getClients();
	for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)//Bucle para recorrer todos los clientes que hay conectados al servidor
	{
		Client *client = clientIt->second;
		if (client->getNickname() == user)//Si el cliente coincide con el usuario al que se va a invitar
		{
			if (!channel.isInvited(client))//Verificar si no está en la lista de invitados
				return(sender.messageToMyself("~ ERROR: User '" + user + "' is not in the clients invited list in channel: " + channelName + "\n"));
			channel.removeInvitedClient(client);//Eliminar al usuario de la lista de invitados del canal
			std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' uninvited Client (" << client->getFd() << ") '" << client->getNickname() << "' to channel: " << channelName << "\n";
			sender.messageToMyself("~ You uninvited '" + user + "' to channel: " + channelName + ".\n");
			return(client->messageToMyself("~ '" + sender.getNickname() + "' uninvited you to channel: " + channelName + ". You are not in the invited list anymore\n"));
		}
	}
	sender.messageToMyself("~ ERROR: User '" + user + "' does not exist\n"); //El usuario no está en el servidor
}

void Commands::commandTOPIC(Client &sender, Server &server, const std::string &channelName, const std::string &topic)
{
	if (channelName.empty())//Comprobar si solo se ha escrito topic
		return(sender.messageToMyself("~ ERROR: Invalid TOPIC command syntax. Use: TOPIC <#channel> [new topic]\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " does not exist\n"));
	Channel &channel = it->second;
	if (topic.empty())//Comprobar si solamente quiero ver el topic
	{
		if (channel.isTopicEmpty())//Comprobar si el canal no tiene topic
			return(sender.messageToMyself("~ Channel: " + channelName + " has no TOPIC yet\n"));
		return(sender.messageToMyself("~ Current TOPIC for channel: " + channelName + " is '" + channel.getTopic() + "'\n"));
	}
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if ((channel.isTopicRestricted() && !channel.isOperator(&sender)))//Comprobar si el canal esta +t y yo no soy operador
		return(sender.messageToMyself("~ ERROR: You are not allowed to change the topic in channel: " + channelName + ". Channel is in MODE +t and you are not an operator\n"));
	if (topic == "REMOVE" || topic == "remove")// Eliminar el tema del canal
	{
		if (channel.getTopic() == "")//Comprobar si estoy intentando eliminar el topic en un canal que no tiene topic
			return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " has no TOPIC\n"));
		channel.setTopic("");
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' removed TOPIC in channel: " << channelName << "\n";
		sender.messageToMyself("~ You removed TOPIC in channel: " + channelName + "\n");
		return(channel.messageToGroupNoSender("~ '" + sender.getNickname() + "' removed TOPIC in channel: " + channelName + "\n", &sender));
	}
	if (channel.getTopic() == topic)//Comprobar si estoy intentando poner un topic en un canal que ya tiene ese mismo topic
			return(sender.messageToMyself("~ ERROR: TOPIC in channel: " + channelName + " is already '" + topic + "'\n"));
	channel.setTopic(topic);
	std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' changed TOPIC to '" << topic << "' in channel: " << channelName << "\n";
	sender.messageToMyself("~ You changed TOPIC to '" + topic + "' in channel: " + channelName + "\n");
	channel.messageToGroupNoSender("~ '" + sender.getNickname() + "' changed TOPIC to '" + topic + "' in channel: " + channelName + "\n", &sender);
}

void Commands::commandKEY(Client &sender, Server &server, const std::string &channelName, const std::string &other)
{
	if (channelName.empty())// Verificar que se ha especificado un canal
		return(sender.messageToMyself("~ ERROR: Invalid KEY command syntax. Use: KEY <#channel>\n"));
	if (!other.empty())
		return(sender.messageToMyself("~ ERROR: Command 'KEY' does not accept any more parameters than CHANNEL. Use: KEY <#channel>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself("~ ERROR: Channel " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself("~ ERROR: You are not an operator in channel: " + channelName + "\n"));
	if(channel.getKey() == "")//Comprobar si el acanal tiene key
		return(sender.messageToMyself("~ Channel: " + channelName + " has no key\n"));
	sender.messageToMyself("~ Channel: " + channelName + " has '" + channel.getKey() + "' as a key\n");
}

void Commands::commandMODE(Client &sender, Server &server, const std::string &channelName, const std::string &mode, const std::string &param)
{
	if (channelName.empty())//Verificar que canal no esté vacío
		return(sender.messageToMyself("~ ERROR: Invalid MODE command syntax. Use: MODE <channel> [+|-mode] [param]\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself("~ ERROR: Channel " + channelName + " does not exist\n"));
	if (mode.empty())// Si no se especifica modo, mostrar los modos activos del canal
	{
		if (!channel.getModes().empty())
			return(sender.messageToMyself("~ Active mode(s) for channel " + channelName + ": " + channel.getModes() + "\n"));
		return(sender.messageToMyself("~ No active modes for channel: " + channelName + "\n"));
	}
	if (!param.empty() && (mode == "+i" || mode == "-i" || mode == "+t" || mode == "-t" || mode  == "-l" || mode  == "-k"))
		return(sender.messageToMyself("~ ERROR: Command 'MODE " + mode +"' does not accept any more parameters than CHANNEL and <+|-mode>. Use: MODE <channel> <+|-mode>\n"));
	size_t spacePos = param.find(' ');
	if(spacePos != std::string::npos)
		return(sender.messageToMyself("~ ERROR: Command 'MODE " + mode +"' does not accept any more parameters than CHANNEL, <+|-mode> and [param]. Use: MODE <channel> <+|-mode> [param]\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself("~ ERROR: You are not an operator in channel: " + channelName + "\n"));
	if (mode == "+o")// Asignar privilegio de operador
	{
		if (param == "")
			return(sender.messageToMyself("~ ERROR: You need to specify a user for MODE +o\n"));
		std::map<int, Client*>& clients = server.getClients();
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			Client *client = clientIt->second;
			if (client->getNickname() == param)
			{
				if (channel.hasClient(client))
				{
					if (channel.isOperator(client))
						return(sender.messageToMyself("~ ERROR: '" + param + "' is already an operator in channel: " + channelName + "\n"));
					channel.addOperator(client);
					std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' gave operator privileges to '" << param << "' in channel: " << channelName << "\n";
					sender.messageToMyself("~ You gave OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\n");
					channel.messageToGroupNoSenderNoReceiver("~ '" + sender.getNickname() + "' gave OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\n", &sender, client);
					return(client->messageToMyself("~ '" + sender.getNickname() + "' gave you OPERATOR PRIVILEGES in channel: " + channelName + "\n"));
				}
				else
					return(sender.messageToMyself("~ ERROR: User '" + param + "' is not in channel: " + channelName + "\n"));
			}
		}
		sender.messageToMyself("~ ERROR: User '" + param + "' does not exist\n");

	}
	else if (mode == "-o")// Eliminar privilegio de operador
	{
		if (param == "")
			return(sender.messageToMyself("~ ERROR: You need to specify a user for MODE -o\n"));
		std::map<int, Client*>& clients = server.getClients();
		for (std::map<int, Client*>::iterator clientIt = clients.begin(); clientIt != clients.end(); ++clientIt)
		{
			Client *client = clientIt->second;
			if (client->getNickname() == param)
			{
				if (channel.hasClient(client))
				{
					if (!channel.isOperator(client))
						return(sender.messageToMyself("~ ERROR: '" + param + "' is not an operator in channel: " + channelName + "\n"));
					channel.removeOperator(client);
					std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' removed operator privileges to '" << param << "' in channel: " << channelName << "\n";
					sender.messageToMyself("~ You removed OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\n");
					channel.messageToGroupNoSenderNoReceiver("~ '" + sender.getNickname() + "' removed OPERATOR PRIVILEGES to '" + param + "' in channel: " + channelName + "\n", &sender, client);
					return(client->messageToMyself("~ '" + sender.getNickname() + "' removed your OPERATOR PRIVILEGES in channel: " + channelName + "\n"));
				}
				else
					return(sender.messageToMyself("~ ERROR: User '" + param + "' is not in channel: " + channelName + "\n"));
			}
		}
		sender.messageToMyself("~ ERROR: User '" + param + "' does not exist\n");

	}
	else if (mode == "+i")//Establecer solo invitados pueden unirse al canal
	{
		if (channel.inviteOnly == true)
			return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " is already on INVITE-ONLY mode\n"));
		channel.setInviteOnly(true);
		for (std::set<Client *>::iterator itClient = channel.getClients().begin(); itClient != channel.getClients().end(); ++itClient)// Agregar a todos los clientes actuales a la lista de invitados
			if (!channel.isInvited(*itClient))// Verifica si ya está invitado
				channel.inviteClient(*itClient);// Agrega a la lista de invitados
		sender.messageToMyself("~ You enabled INVITE-ONLY mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender.getNickname() + "' enabled INVITE-ONLY mode in channel: " + channelName + "\n", &sender);
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' enabled invite-only mode in channel: " << channelName << "\n";
	}
	else if (mode == "-i")//Eliminar solo invitados pueden unirse al canal
	{
		if (channel.inviteOnly == false)
			return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " is not on INVITE-ONLY mode\n"));
		channel.setInviteOnly(false);
		sender.messageToMyself("~ You disabled INVITE-ONLY mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender.getNickname() + "' disabled INVITE-ONLY mode in channel: " + channelName + "\n", &sender);
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' disabled invite-only mode in channel: " << channelName << "\n";
	}
	else if (mode == "+t")//Establecer topic solo lo pueden cambiar operadores
	{
		if (channel.topicRestricted == true)
			return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " is already on TOPIC-RESTRICTED mode\n"));
		channel.setTopicRestricted(true);
		sender.messageToMyself("~ You enabled TOPIC-RESTRICTED mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender.getNickname() + "' enabled TOPIC-RESTRICTED mode in channel: " + channelName + "\n", &sender);
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' enabled topic-restricted mode in channel: " << channelName << "\n";
	}
	else if (mode == "-t")//Eliminar topic solo lo pueden cambiar operadores
	{
		if (channel.topicRestricted == false)
			return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " is not on TOPIC-RESTRICTED mode\n"));
		channel.setTopicRestricted(false);
		sender.messageToMyself("~ You disabled TOPIC-RESTRICTED mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender.getNickname() + "' disabled TOPIC-RESTRICTED mode in channel: " + channelName + "\n", &sender);
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' disabled topic-restricted mode in channel: " << channelName << "\n";
	}
	else if (mode == "+k")//Establecer contraseña al canal
	{
		if (param == "")
			return(sender.messageToMyself("~ ERROR: You need to specify a key for MODE +k\n"));
		if (param.size() > 9)
			return(sender.messageToMyself("~ ERROR: Key cannot be longer than 9 characters\n"));
		if(channel.getKey() == param)
			return(sender.messageToMyself("~ ERROR: KEY is already '" + param + "' in channel: " + channelName + "\n"));
		channel.setKey(param);
		sender.messageToMyself("~ You enabled KEY (" + param + ") mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender.getNickname() + "' enabled KEY (" + param + ") mode in channel: " + channelName + "\n", &sender);
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' enabled key (" << param << ") mode in channel: " << channelName << "\n";
	}
	else if (mode == "-k")//Eliminar contraseña del canal
	{
		if(channel.isKeyEmpty())
			return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " is not on KEY mode\n"));
		channel.clearKey();
		sender.messageToMyself("~ You disabled KEY mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender.getNickname() + "' disabled KEY mode in channel: " + channelName + "\n", &sender);
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' disabled key mode in channel: " << channelName << "\n";
	}
	else if (mode == "+l")//Establecer numero maximo de usuarios en el canal
	{
		if (param == "")
			return(sender.messageToMyself("~ ERROR: You need to specify a number for MODE +l\n"));
		for (std::size_t i = 0; i < param.size(); i++)
			if (!std::isdigit(param[i]))
				return(sender.messageToMyself("~ ERROR: Limit cannot be something different from a positive number\n"));
		size_t limit = std::atoi(param.c_str());
		if (limit == 0 || limit == 1)
			return(sender.messageToMyself("~ ERROR: Limit cannot be less than 2\n"));
		if (limit < channel.getClientsNumber())
			return(sender.messageToMyself("~ ERROR: Limit cannot be less than the channel's actual number of members (" + to_string(channel.getClientsNumber()) + ")\n"));
		if (limit == channel.getUserLimit())
			return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " is already limited to " + param + "\n"));
		channel.setUserLimit(limit);
		sender.messageToMyself("~ You enabled USER-LIMIT (" + param + ") mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender.getNickname() + "' enabled USER-LIMIT (" + param + ") mode in channel: " + channelName + "\n", &sender);
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' enabled user-limit (" << param << ") mode in channel: " << channelName << "\n";
	}
	else if (mode == "-l")//Eliminar numero maximo de usuarios en el canal
	{
		if (channel.getUserLimit() == 0)
			return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " already has no users limit\n"));
		channel.clearUserLimit();
		sender.messageToMyself("~ You disabled USER-LIMIT mode in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender.getNickname() + "' disabled USER-LIMIT mode in channel: " + channelName + "\n", &sender);
		std::cout << "Client (" << sender.getFd() << ") '" << sender.getNickname() << "' disabled user-limit mode in channel: " << channelName << "\n";
	}
	else
		sender.messageToMyself("~ ERROR: Invalid mode command. Modes are: +|- i, t, k, o, l\n");
}

void Commands::commandREMOVE(Client &sender, Server &server, const std::string &channelName, const std::string &param, const std::string &other)
{
	if (channelName.empty() || param.empty())//Verificar que ninguno de los parametros este vacio
		return(sender.messageToMyself("~ ERROR: Invalid REMOVE command syntax. Use: REMOVE <#channel> <topic/modes/invited/banned>\n"));
	if (!other.empty())//Verificar ningun otra palabara detras de <param>
		return(sender.messageToMyself("~ ERROR: Command 'REMOVE' does not accept any more parameters than CHANNEL and a parameter. Use: REMOVE <#channel> <topic/modes/invited/banned>\n"));
	std::map<std::string, Channel>& channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	Channel &channel = it->second;
	if (it == channels.end())//Comprobar si el canal existe
		return(sender.messageToMyself("~ ERROR: Channel: " + channelName + " does not exist\n"));
	if (!channel.hasClient(&sender))//Comprobar si estoy está dentro del canal
		return(sender.messageToMyself("~ ERROR: You are not in channel: " + channelName + "\n"));
	if (!channel.isOperator(&sender))//Comprobar si soy operador
		return(sender.messageToMyself("~ ERROR: You are not an operator of channel: " + channelName + ".\n"));
	if (param == "topic")// Si el param es topic. Eliminar el topic
	{
		if (channel.isTopicEmpty())// Verificar si el topic ya está vacío
			return(sender.messageToMyself("~ ERROR: Channel " + channelName + " has no topic to remove.\n"));
		channel.clearTopic();
		sender.messageToMyself("~ Topic removed from channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender.getNickname() + "' has removed the topic from channel: " + channelName + "\n", &sender);
	}
	else if (param == "modes")// Si el param es modes. Desactivar todos los modos activos
	{
		if (!channel.inviteOnly && !channel.topicRestricted && channel.getKey().empty() && channel.getUserLimit() == 0)// Verificar si todos los modos están ya desactivados
			return(sender.messageToMyself("~ ERROR: All modes are already disabled in channel: " + channelName + "\n"));
		channel.setInviteOnly(false);
		channel.setTopicRestricted(false);
		channel.setKey("");
		channel.clearUserLimit();
		sender.messageToMyself("~ All modes have been cleared in channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender.getNickname() + "' has cleared all modes in channel: " + channelName + "\n", &sender);
	}
	else if (param == "invited")// Si el param es invited. Eliminar lista de inivitados
	{
		if (channel.isInvitedListEmpty())// Verificar si la lista de invitados ya está vacía
			return(sender.messageToMyself("~ ERROR: No invited users to remove in channel: " + channelName + "\n"));
		channel.clearInvitedClients();
		sender.messageToMyself("~ All invited users have been removed from channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender.getNickname() + "' has cleared the invited list in channel: " + channelName + "\n", &sender);
	}
	else if (param == "banned")// Si el param es banned. Eliminar lista de baneados
	{
		if (channel.isBannedListEmpty())// Verificar si la lista de baneados ya está vacía
			return(sender.messageToMyself("~ ERROR: No banned users to remove in channel: " + channelName + "\n"));
		channel.clearBannedClients();
		sender.messageToMyself("~ All banned users have been removed from channel: " + channelName + "\n");
		channel.messageToGroupNoSender("~ '" + sender.getNickname() + "' has cleared the banned list in channel: " + channelName + "\n", &sender);
	}
	else
		sender.messageToMyself("~ ERROR: Invalid parameter for REMOVE. Use: topic, modes, invited or banned.\n");
}
