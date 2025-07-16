# Update later
NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

OBJDIR = obj

SRC = main.cpp \
		IRC_server.cpp \
		Socket.cpp \
		EpollLoop.cpp \
		PollLoop.cpp \
		Signals.cpp

OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))
DEPS = $(patsubst %.cpp,$(OBJDIR)/%.d,$(SRC))

-include $(DEPS)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -f $(OBJ) $(DEPS)
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all
.PHONY: all clean fclean re