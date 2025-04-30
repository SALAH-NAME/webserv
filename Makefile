NAME = webserv

CXX	:= c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98
INCFLAGS := -I./include/
DEPFLAGS := -MMD -MP

SRC_DIR := src
OBJ_DIR := obj

SRCS := main.cpp \

OBJS := $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))
DEPS := $(OBJS:.o=.d)

all: $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCFLAGS) $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re
