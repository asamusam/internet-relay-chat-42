CXX := c++
INCLUDE := include
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -I$(INCLUDE)
DBCXXFLAGS := -Og -ggdb3 -Wpedantic $(CXXFLAGS)
SRC := \
	src/connection.cpp \
	src/InternalError.cpp \
	src/ircserv.cpp \
	# src/parser.cpp \
#SRC

OBJ := $(SRC:.cpp=.o)
NAME := ircserv
DBNAME := debug_build

.PHONY: all debug clean fclean re

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

debug: $(DBNAME)

$(DBNAME): $(SRC)
	$(re)
	$(CXX) $(DBCXXFLAGS) $^ -o $@

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME) $(DBNAME)

re: fclean all
