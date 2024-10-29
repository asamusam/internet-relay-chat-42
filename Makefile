CXX := c++
INCLUDE := include
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -I$(INCLUDE)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
    CXXFLAGS += -D__APPLE__
endif
DBCXXFLAGS := -Og -ggdb3 $(CXXFLAGS)
SRC := \
	src/App.cpp \
	src/Channel.cpp \
	src/InternalError.cpp \
	src/IRCReply.cpp \
	src/SystemCallErrorMessage.cpp \
	src/connection.cpp \
	src/main.cpp \
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
