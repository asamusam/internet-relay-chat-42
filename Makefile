CXX := c++
INCLUDE := include
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -I$(INCLUDE)
SRC := src/ircserv.cpp src/parser.cpp
OBJ := $(SRC:.cpp=.o)
NAME := ircserv

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(OBJ) -o $(NAME)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all