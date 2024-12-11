NAME = ircserv

CC = c++

FLAGS = -Wall -Werror -Wextra -std=c++98

RM = rm -rf

SRC = src/Channel.cpp\
		src/Client.cpp\
		src/CommandParser.cpp\
		src/main.cpp\
		src/Server.cpp\
		src/Utils.cpp\

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
