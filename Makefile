# Update later
EXECUTABLE = ircserv
SRC_DIR = ./srcs
OBJ_DIR = ./obj
INC_DIR = ./includes
BT_DIR = ./builtins

INCLUDES := $(wildcard $(INC_DIR)/*.hpp)
#INCLUDES =   ./includes/Channel.hpp ./includes/Client.hpp ./includes/EpollLoop.hpp ./includes/Exception.hpp ./includes/IEventLoop.hpp ./includes/IRC_server.hpp ./includes/PollLoop.hpp ./includes/Signals.hpp ./includes/Socket



SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp)  $(wildcard $(SRC_DIR)/*/*/*.cpp) $(wildcard $(SRC_DIR)/*/*/*/*.cpp)
#SRC = main.cpp \
		Channel.cpp\
		Client.cpp \
		EpollLoop.cpp \
		IRC_server.cpp \
		PollLoop.cpp \
		Signals.cpp \
		Socket.cpp

OBJ = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
CFLAGS := -Wall -Wextra -Werror -I$(INC_DIR) -std=c++98

RM := rm -f

CC := c++


MAKEFLAGS += --no-print-directory


all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $(OBJ) -o $@


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(CC) $(CFLAGS)  -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	$(RM) $(OBJ)

fclean: clean
	rm -rf $(OBJ_DIR)
	$(RM) $(EXECUTABLE)

re: fclean all

.PHONY: all clean fclean re