# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ilopez-r <ilopez-r@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/01/13 15:02:37 by ilopez-r          #+#    #+#              #
#    Updated: 2025/01/13 17:45:21 by ilopez-r         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

CC = c++

FLAGS = -Wall -Werror -Wextra -std=c++98

RM = rm -rf

SRC = src/Channel.cpp\
		src/Client.cpp\
		src/main.cpp\
		src/Server.cpp\
		src/Comannds.cpp\
		src/bonus/bot.cpp\

OBJ = $(SRC:.cpp=.o)

$(NAME): $(OBJ)
	@$(CC) $(FLAGS) $(OBJ) -o $(NAME)

$(OBJ): ./%.o : ./%.cpp
	@$(CC) $(FLAGS) -c $< -o $@

all: $(NAME)

clean:
	@$(RM) $(OBJ)

fclean:
	@ $(RM) $(NAME) $(OBJ)

re : fclean all

.PHONY: all re fclean clean
