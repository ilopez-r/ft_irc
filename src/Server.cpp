/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 12:28:54 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/13 00:38:29 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

Server::Server(int port, const std::string &password): _port(port), _password(password),  _commands(new Commands())
{
	_design =
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
	initializeServer();
}

Server::~Server()
{
	delete (_commands);
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
		delete (it->second);
	close(_serverSocket);
}

const std::string Server::getPassword() const
{
	return (_password);
}

std::map<int, Client*>& Server::getClients()
{
	return clients;
}

std::map<std::string, Channel>& Server::getChannels()
{
	return channels;
}

std::vector<struct pollfd>& Server::getPollFds()
{
	return pollFds;
}

void Server::initializeServer()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket < 0)
		throw (std::runtime_error("Failed to create socket"));
	if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) == -1)// Configurar el socket como no bloqueante
		throw (std::runtime_error("Failed to set non-blocking mode on server socket"));
	sockaddr_in serverAddr;
	std::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(_port);
	if (bind(_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		throw (std::runtime_error("Failed to bind socket"));
	if (listen(_serverSocket, SOMAXCONN) < 0)
		throw (std::runtime_error("Failed to listen on socket"));
	struct pollfd pfd;
	pfd.fd = _serverSocket;
	pfd.events = POLLIN;
	pfd.revents = 0;
	pollFds.push_back(pfd);
	run();
}

void Server::run()
{
	while (true)
	{
		int pollCount = poll(&pollFds[0], pollFds.size(), -1);
		if (pollCount < 0)
			throw (std::runtime_error("Poll failed"));
		for (size_t i = 0; i < pollFds.size(); ++i)
		{
			if (pollFds[i].revents & POLLIN)
			{
				if (pollFds[i].fd == _serverSocket)
					acceptNewClient();
				else
					handleClientActions(pollFds[i].fd);
			}
		}
	}
}

void Server::acceptNewClient()
{
	sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int clientFd = accept(_serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
	if (clientFd < 0)
	{
		std::cerr << "Error: Failed to accept new client\n";
		return;
	}
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)// Configurar el cliente como no bloqueante
	{
		std::cerr << "Error: Failed to set non-blocking mode on client socket\n";
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
	clients[clientFd]->messageToMyself(_design + "Welcome to the IRC Server!\nType HELP to show the instructions\n");
}

void Server::handleClientActions(int clientFd)
{
	char buffer[512];
	std::memset(buffer, 0, sizeof(buffer));
	int bytesReceived = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
	
	if (bytesReceived <= 0)//Si se cierra la ventana sin hacer QUIT
		return(_commands->handleCommand(*clients[clientFd], *this, "QUIT", "", "", "", ""));// Llamar a handleCommand con QUIT para desconectar al cliente del servidor
	std::string& clientBuffer = clients[clientFd]->getBuffer(); // Obtener el búfer del cliente
	clientBuffer += std::string(buffer, bytesReceived);// Acumular datos recibidos en el búfer del cliente
	size_t pos;
	while ((pos = clientBuffer.find('\n')) != std::string::npos)// Procesar mensajes completos en el búfer
	{
		std::string fullLine = clientBuffer.substr(0, pos); // Extraer un mensaje completo
		clientBuffer.erase(0, pos + 1); // Eliminar el mensaje procesado del búfer
		processClientLine(clients[clientFd], fullLine);// Procesar la línea escrita por el cliente
	}
}

void Server::processClientLine(Client *client, const std::string &rawInput)
{
	std::string line = trim(rawInput);
	if (line.empty()) // No hacer nada si la línea está vacía.
		return;
	// Separar el mensaje en 3 partes.
	size_t spacePos = line.find(' ');
	std::string cmd = line.substr(0, spacePos);
	std::string paramraw = "";
	std::string param = "";
	std::string paramraw2 = "";
	std::string param2 = "";
	std::string param3 = "";
	if (spacePos != std::string::npos)//Si encuentra el espacio
	{
		paramraw =  trim(line.substr(spacePos + 1));
		spacePos = paramraw.find(' ');
		if (spacePos != std::string::npos)//Si encuentra el espacio
		{
			param = paramraw.substr(0, spacePos);
			paramraw2 = trim(paramraw.substr(spacePos + 1));
			spacePos = paramraw2.find(' ');
			param2 = paramraw2.substr(0, spacePos);
			if (spacePos != std::string::npos)
				param3 = trim(paramraw2.substr(spacePos + 1));
		}
		else
			param = paramraw;
	}
	for (std::size_t i = 0; i < cmd.size(); i++)
		cmd[i] = toupper(cmd[i]);
/* 	std::cout << "\n[DEBUG] input:" << line << ".\n";
	std::cout << "[DEBUG] paramraw:" << paramraw << ".\n";
	std::cout << "[DEBUG] param:" << param << ".\n";
	std::cout << "[DEBUG] paramraw2:" << paramraw2 << ".\n";
	std::cout << "[DEBUG] param2:" << param2 << ".\n";
	std::cout << "[DEBUG] param3:" << param3 << ".\n"; */
	_commands->handleCommand(*client, *this, cmd, param, paramraw2, param2, param3);
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
