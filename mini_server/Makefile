NAME	= Server

SRC	= main.cpp Server.cpp parser_utils.cpp types.cpp  RequestHandler.cpp config_parser/Parser.cpp config_parser/Block.cpp config_parser/ServerBlock.cpp config_parser/LocationBlock.cpp

HEADERS = Server.hpp RequestHandler.hpp config_parser/Parser.hpp config_parser/Block.hpp config_parser/ServerBlock.hpp config_parser/LocationBlock.hpp

OBJ_DIR = obj
OBJ	= $(SRC:%.cpp=$(OBJ_DIR)/%.o)

CC	= c++

CFLAGS	= -std=c++98 -g

#CFLAGS	= -Wall -Wextra -Werror -std=c++98 -g

all: $(NAME)

$(NAME): $(OBJ)  $(HEADERS)
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp  $(HEADERS)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
#	rm -f $(OBJ)
#	-if [ -d $(OBJ_DIR) ]; then rmdir $(OBJ_DIR); fi
	@if [ -d $(OBJ_DIR) ]; then \
		find $(OBJ_DIR) -type f -name '*.o' -delete; \
		find $(OBJ_DIR) -type d -empty -delete; \
	fi
# this part needs more testing


fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re