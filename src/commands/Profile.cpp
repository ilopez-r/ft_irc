/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Profile.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/23 20:27:30 by ilopez-r          #+#    #+#             */
/*   Updated: 2025/01/23 20:27:31 by ilopez-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Command.hpp"

void commandPROFILE(Client &client, const std::string &param)
{
	if (!param.empty())// Verificar ningun otra palabara detras de PROFILE
			return(client.messageToMyself("~ ERROR: Command 'PROFILE' does not accept any parameters\n"));
	client.messageToMyself("~ Your profile information:\n");
	client.messageToMyself("	- Username: " + client.getUsername() + "\n");
	client.messageToMyself("	- Nickname: " + client.getNickname() + "\n");
}
