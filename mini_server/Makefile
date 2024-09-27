NAME	= Server

SRC	= main.cpp Server.cpp parser_utils.cpp types.cpp  RequestHandler.cpp 

OBJ_DIR = obj
OBJ	= $(SRC:%.cpp=$(OBJ_DIR)/%.o)

CC	= c++

CFLAGS	= -Wall -Wextra -Werror -std=c++98 -g

all: $(NAME)

$(NAME): $(OBJ) 
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_DIR)/*.o

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re