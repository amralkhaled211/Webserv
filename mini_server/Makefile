NAME	= Server

SRC	= main.cpp Server.cpp parser_utils.cpp types.cpp  RequestHandler.cpp CGI.cpp

OBJ_DIR = obj
OBJ	= $(SRC:%.cpp=$(OBJ_DIR)/%.o)

CXX	= c++

CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -g

all: $(NAME)

$(NAME): $(OBJ) 
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_DIR)/*.o

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re