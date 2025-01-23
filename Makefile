# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/01/13 15:02:37 by ilopez-r          #+#    #+#              #
#    Updated: 2025/01/23 21:25:44 by ilopez-r         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

CC = c++

FLAGS = -Wall -Werror -Wextra -std=c++98 -g

RM = rm -rf

SRC = Channel.cpp\
		Client.cpp\
		main.cpp\
		Server.cpp\
		Command.cpp\
		commands/Quit.cpp\
		commands/Commands.cpp\
		commands/Pass.cpp\
		commands/User.cpp\
		commands/Nick.cpp\
		commands/Profile.cpp\
		commands/Channels.cpp\
		commands/Privmsg.cpp\
		commands/Join.cpp\
		commands/Part.cpp\
		commands/Kick.cpp\
		commands/Invite.cpp\
		commands/Uninvite.cpp\
		commands/Topic.cpp\
		commands/Key.cpp\
		commands/Mode.cpp\
		commands/Remove.cpp\
		bonus/bot.cpp\

SRC_DIR = src/

SRC_FILES = $(addprefix $(SRC_DIR), $(SRC))

OBJ = $(SRC:.cpp=.o)

OBJ_DIR = obj/

OBJ_FILES = $(addprefix $(OBJ_DIR), $(OBJ))

$(NAME): $(OBJ_FILES)
	@$(CC) $(FLAGS) $(OBJ_FILES) -o $(NAME)
	
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR) $(OBJ_DIR)/commands $(OBJ_DIR)/bonus

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(OBJ_DIR)
	@$(CC) $(FLAGS) -c $< -o $@

all: $(NAME)

clean:
	@ $(RM) $(OBJ_DIR)

fclean: clean
	@ $(RM) $(NAME)

re : fclean all

.PHONY: all re fclean clean
