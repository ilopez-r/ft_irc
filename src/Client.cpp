/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/19 20:21:41 by ilopez-r          #+#    #+#             */
/*   Updated: 2024/12/19 20:30:24 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"

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

void Client::sendMessage(const std::string &message)
{
    if (send(fd, message.c_str(), message.size(), 0) < 0)
        std::cerr << "Failed to send message to client: " << fd << "\n";
}

void Client::processMessage(const std::string &message, Server &server) {
    handleCommand(message, server);
}

void Client::handleCommand(const std::string &command, Server &server) {
    // Separar el comando en partes.
    size_t spacePos = command.find(' ');
    std::string cmd = command.substr(0, spacePos);
    std::string paramraw = (spacePos != std::string::npos) ? command.substr(spacePos + 1) : "";
    std::cout << "praw:" << paramraw << ".\n";
    std::string param = "";
    std::string param2 = "";
    if (paramraw != "")
    {
        spacePos = paramraw.find(' ');
        if (spacePos != 0)
        {
            param = paramraw.substr(0, spacePos);
            param2 = (spacePos != std::string::npos) ? paramraw.substr(spacePos + 1) : "";
        }
        else
        {
            param = paramraw;
        }
        std::cout << "p1:" << param << ".\n";
        std::cout << "p2:" << param2 << ".\n";
    }
    if (cmd == "QUIT" || cmd == "quit")// Sintaxis: QUIT
    {
        if (!param.empty())// Verificar ningun otra palabara detras de QUIT
        {
            sendMessage("~ ERROR: Command 'QUIT' does not accept any parameters\n");
            return;
        }
        server.disconnectClient(this);// Termina la conexión del cliente.
        return;
    }
    if (cmd == "VIVA" || cmd == "viva")
    {
        if (cmd == "VIVA")
            sendMessage("~ ESPAÑA\n");
        else
            sendMessage("~ ¡PONLO EN MAYÚSCULAS HOMBRE!\n");
        return;
    }
    if (cmd == "HELP" || cmd == "help")// Sintaxis: HELP
    {
        if (!param.empty())// Verificar ningun otra palabara detras de HELP
        {
            sendMessage("~ ERROR: Command 'HELP' does not accept any parameters\n");
            return;
        }
        sendMessage(server.help);//Envia el mensaje de help
        return;
    }
    if (cmd == "PASS" || cmd == "pass")// Sintaxis: PASS <password>
    {
        if (passwordSent)//Verificar que no se mande 2 veces la misma pass
        {
            sendMessage("~ ERROR: 'PASS' command already sent\n");
            return;
        }
        if (param2 != "")// Verificar ninguna otra palabara detras de la password
        {
            sendMessage("~ ERROR: Command 'PASS' does not accept any more parameters than the PASSWORD. Use: PASS <password>\n");
            return;
        }
        if (server.validatePassword(param))// Verificar si la pass es correcta
        {
            std::cout << "Client (" << getFd() << ") accepted in server\n";
            setPasswordSent(true);
            sendMessage("~ Password accepted.\n");
        }
        else //password incorrecta
            sendMessage("~ ERROR: Incorrect password. Use: PASS <password>\n");
    }
    else if (!passwordSent) //Obligar a que el primer comando que se debe introducir es pass
    {
        sendMessage("~ ERROR: You must authenticate with 'PASS' before any other command. Use: PASS <password>\n");
        return;
    }
    else if (cmd == "USER" || cmd == "user" || cmd == "NICK" || cmd == "nick")// Sintaxis: USER <username> Sintaxis: NICK <nickname>
    {
        if ((cmd == "NICK" || cmd == "nick") && username == "")// PARA NICK. Verificar que se ha introducido un username
        {
            sendMessage("~ ERROR: First you have to use command 'USER' and specify your username. Use: USER <username>\n");
            return;
        }
        if (param.empty())// Validar username/nickname no esta vacio
        {
            if (cmd == "USER" || cmd == "user")
                sendMessage("~ ERROR: No username provided. Use: USER <username>\n");
            else
                sendMessage("~ ERROR: No nickname provided. Use: NICK <nickname>\n");
            return;
        }
        if (param2 != "") // Verificar ningun otra palabara detras del username/nickname
        {
            if (cmd == "USER" || cmd == "user")
                sendMessage("~ ERROR: Command 'USER' does not accept any more parameters than the USERNAME. Use: USER <username>\n");
            else
                sendMessage("~ ERROR: Command 'NICK' does not accept any more parameters than the NICKNAME. Use: NICK <nickname>\n");
            return;
        }
        if (param.length() > 9)// Verificar que no sea mas largo de 9 caracteres
        {
            if (cmd == "USER" || cmd == "user")
                sendMessage("~ ERROR: Username cannot be longer than 9 characters\n");
            else
                sendMessage("~ ERROR: Nickname cannot be longer than 9 characters\n");
            return;
        }
        if (cmd == "USER" || cmd == "user")// Asignar el username al cliente.
        {
            setUsername(param);
            sendMessage("~ You setted your username to '" + param + "'\n");
        }
        else// Asignar el nickname al cliente
        {
            if (server.isNicknameInUse(param)) // Verificar si el nickname ya está en uso.
            {
                sendMessage("ERROR: Nickname '" + param + "' is already in use\n");
                return;
            }
            std::string oldNickname = getNickname(); //Guardar el nickname anterior
            setNickname(param);// Asignar el nickname al cliente.
            if (oldNickname != "")// Mensaje si se ha cambiado el nickname a otro distinto 
            {
                sendMessage("~ You changed your nickname from '" + oldNickname + "' to '" + param + "'\n");
                std::cout << "Client (" << getFd() << ") '" << oldNickname << "' changed his nickname to '" << getNickname() << "'\n";
                server.notifyChannelsOfNicknameChange(this, oldNickname, param);//Anunicar a todos los canales en los que esta el cliente el cambio de nickname
            }
            else //Mensaje si es el primer nickname que se pone
            {
                sendMessage("~ You setted your nickname to '" + param + "'\n");
                std::cout << "Client (" << getFd() << ") setted his nickname to '" << getNickname() << "'\n";
            }
        }
    }
    else if (nickname == "")// Obligar a que haya un nickname para poder hacer el resto de comandos
    {
        sendMessage("~ ERROR: First you have to use command 'NICK' and specify your nickname\n");
        return;
    }
    else if (cmd == "JOIN" || cmd == "join" || cmd == "LEAVE" || cmd == "leave")// Sintaxis: JOIN <channel> [key]. Sintaxis: LEAVE <channel>
    {
        if (param.empty())// Verificar que se ha especificado un canal
        {
            if (cmd == "JOIN" || cmd == "join")
                sendMessage("~ ERROR: No channel name provided. Use: JOIN <#channel> [key]\n");
            else
                sendMessage("~ ERROR: No channel name provided. Use: LEAVE <#channel>\n");
            return;
        }
        if (param[0] != '#')//Verificar que el canal empieze por #
        {
            sendMessage("~ ERROR: Channel name must start with '#'\n");
            return;
        }
        if ((cmd == "LEAVE" || cmd == "leave") && !param2.empty())// PARA LEAVE. Verificar ningun otra palabara detras del canal
        {
            sendMessage("~ ERROR: Command 'LEAVE' does not accept any more parameters than the #CHANNEL. Use: LEAVE <#channel>\n");
            return;
        }
        if (cmd == "JOIN" || cmd == "join")
            server.joinChannel(param, this, param2);//funcion para unir a un cliente aun canal
        else
            server.leaveChannel(param,this);//funcion para salir un cliente de un canal
    }
    else if (cmd == "CHANNELS" || cmd == "channels")// Sintaxis: CHANNELS
    {
        if (!param.empty())//Verificar ningun otra palabara detras de CHANNELS
        {
            sendMessage("~ ERROR: Command 'CHANNELS' does not accept any parameters\n");
            return;
        }
        server.showChannels(this);//mostrar todos los canales activos
    }
    else if (cmd == "MSG" || cmd == "msg")// Sintaxis: MSG <user/#channel> <message>
    {
        if (param.empty() || param2.empty())//Verificar que el destinatario y el mensaje no esten vacios
        {
            sendMessage("~ ERROR: Invalid MSG format. Use MSG <receiver> <message>\n");
            return;
        }
        if (param == getNickname())//Verificar que no me este mandando un mensaje a mi mismo
        {
            sendMessage("~ ERROR: You cannot send a message to yourself (" + getNickname() + ").\n");
            return;
        }
        server.sendMessageToReceiver(param, param2, this);// Enviar el mensaje al receptor.
    }
    else if (cmd == "KICK" || cmd == "kick")// Sintaxis: KICK <#channel> <user> <reason>
    {
        std::string userToKick = "";
        std::string reason = "";
        if (param2 != "")//Si param2 no esta vacio, dividirlo en user y reason
        {
            size_t spacePos = param2.find(' ');
            userToKick = param2.substr(0, spacePos);
            reason = (spacePos != std::string::npos) ? param2.substr(spacePos + 1) : "";
        }
        if (param.empty() || userToKick.empty() || reason.empty())//Verificar que ninguno de los parametros este vacio
        {
            sendMessage("~ ERROR: Invalid KICK command syntax. Use KICK <#channel> <user> <reason>\n");
            return;
        }
        server.kickUserFromChannel(param, userToKick, this, reason);//Funcion de kickear
    }
    else if (cmd == "INVITE" || cmd == "invite")// Sintaxis: INVITE <user> <#channel>
    {
        size_t spacePos2 = param.find(' ');
        std::string userToInvite = param.substr(0, spacePos2);
        std::string channelName = param.substr(spacePos2 + 1);

        if (userToInvite.empty() || channelName.empty())
        {
            sendMessage("ERROR: Invalid INVITE command syntax.\n");
            return;
        }

        server.inviteUserToChannel(channelName, userToInvite, this);

    }
    else if (cmd == "TOPIC" || cmd == "topic")// Sintaxis: TOPIC <#channel> [new topic]
    {
        if (param.empty())
        {
            sendMessage("~ ERROR: Invalid TOPIC command syntax. Use TOPIC <#channel> [new topic]\n");
            return;
        }
        server.setChannelTopic(param, param2, this);
    }
    else if (cmd == "MODE" || cmd == "mode")// Sintaxis: MODE <channel> <+|-mode> [param]
    {
        if (nickname == "")
        {
            sendMessage("~ ERROR: First you have to use command 'NICK' and specify your nickname.\n");
            return;
        }
        
        size_t firstSpace = param.find(' ');
        std::string channelName = param.substr(0, firstSpace);
        std::string remaining = (firstSpace != std::string::npos) ? param.substr(firstSpace + 1) : "";

        size_t secondSpace = remaining.find(' ');
        std::string mode = remaining.substr(0, secondSpace);
        std::string modeParam = (secondSpace != std::string::npos) ? remaining.substr(secondSpace + 1) : "";

        if (channelName.empty() || mode.empty())
        {
            sendMessage("ERROR: Invalid MODE command syntax. Use MODE <channel> <+|-mode> [param]\n");
            return;
        }

        // Enviar la solicitud de configuración del modo al servidor
        server.setChannelMode(channelName, mode, modeParam, this);
        return;
    }
    else
        sendMessage("~ Unknown command: " + cmd + "\n");
}
