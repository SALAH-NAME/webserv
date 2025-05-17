NAME := webserv

CXX	:= c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98
INCFLAGS := -I./include/
DEPFLAGS := -MMD -MP

SRC_DIR := src
OBJ_DIR := obj

SRCS := main.cpp \
				ConfigTokenizer.cpp \
				ConfigTypes.cpp BaseConfig.cpp GlobalConfig.cpp LocationConfig.cpp ServerConfig.cpp \
				ConfigManager.cpp ConfigPrinter.cpp	ConfigParser.cpp \

OBJS := $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))
DEPS := $(OBJS:.o=.d)

# UNIT_TEST
UNIT_NAME := unit

TEST_DIR := test
UNIT_SRCS := $(filter-out main.cpp, $(SRCS))

TEST_SRCS := unit_test.cpp \
						 testTokenizer.cpp testConfigPrint.cpp

UNIT_OBJS := $(addprefix $(OBJ_DIR)/, $(UNIT_SRCS:.cpp=.o)) $(addprefix $(OBJ_DIR)/, $(TEST_SRCS:.cpp=.o))
UNIT_DEPS := $(UNIT_OBJS:.o=.d)


GREEN := \033[0;32m
YELLOW := \033[1;33m
RED := \033[0;31m
RESET := \033[0m

all: $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "$(YELLOW)Building $<...$(RESET)"
	$(CXX) $(CXXFLAGS) $(INCFLAGS) $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR):
	@echo "$(YELLOW)Creating object directory...$(RESET)"
	mkdir -p $(OBJ_DIR)

$(NAME): $(OBJS)
	@echo "$(GREEN)Linking $(NAME)...$(RESET)"
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(UNIT_NAME): $(UNIT_OBJS)
	@echo "$(GREEN)Linking $(UNIT_NAME)...$(RESET)"
	$(CXX) $(CXXFLAGS) $(UNIT_OBJS) -o $(UNIT_NAME)

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp | $(OBJ_DIR)
	@echo "$(YELLOW)Building test $<...$(RESET)"
	$(CXX) $(CXXFLAGS) $(INCFLAGS) $(DEPFLAGS) -c $< -o $@

clean:
	@echo "$(RED)Cleaning up...$(RESET)"
	rm -rf $(OBJS) $(UNIT_OBJS)
	rm -rf $(DEPS) $(UNIT_DEPS)

fclean: clean
	@echo "$(RED)Removing executables...$(RESET)"
	rm -f $(NAME) $(UNIT_NAME)

re: fclean all

$(UNIT_NAME): $(UNIT_OBJS)

test: $(UNIT_NAME)
	@echo "$(GREEN)Running unit tests...$(RESET)"
	./unit

-include $(DEPS)
-include $(UNIT_DEPS)

.PHONY: all clean fclean re test
.SECONDARY: $(OBJS) $(UNIT_OBJS)
