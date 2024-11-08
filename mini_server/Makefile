NAME	= Server

SRC_DIR = .
CONFIG_PARSER_DIR = config_parser

SRC = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(CONFIG_PARSER_DIR)/*.cpp)
HEADERS = $(wildcard $(SRC_DIR)/*.hpp) $(wildcard $(CONFIG_PARSER_DIR)/*.hpp)

OBJ_DIR = obj
OBJ	= $(SRC:%.cpp=$(OBJ_DIR)/%.o)

CC	= c++ -std=c++98

CFLAGS	=  -g

#CFLAGS	= -Wall -Wextra -Werror -std=c++98 -g

all: $(NAME)

$(NAME): $(OBJ)  $(HEADERS)
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp  $(HEADERS)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@if [ -d $(OBJ_DIR) ]; then \
		find $(OBJ_DIR) -type f -name '*.o' -delete; \
		find $(OBJ_DIR) -type d -empty -delete; \
	fi

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re