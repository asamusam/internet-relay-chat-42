CXX := c++
INCLUDE := include
CXXFLAGS := -std=c++98 -I$(INCLUDE) # -Wall -Wextra -Werror
SRC := \
	src/main.cpp \
	src/App.cpp
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