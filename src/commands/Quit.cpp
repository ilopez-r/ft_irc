/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Quit.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:27:35 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/23 20:27:36 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandPART(Client &client, Server &server, const std::string &channelName, const std::string &other);

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