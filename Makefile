NAME := webserv

CXX	:= c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98
INCFLAGS := -I./include/
DEPFLAGS := -MMD -MP

SRC_DIR := src
OBJ_DIR := obj

SRCS := server-module/main.cpp \
		config-module/ConfigTokenizer.cpp config-module/ConfigTypes.cpp config-module/BaseConfig.cpp \
		config-module/GlobalConfig.cpp config-module/LocationConfig.cpp config-module/ServerConfig.cpp \
		config-module/ConfigManager.cpp config-module/ConfigParser.cpp config-module/ConfigPrinter.cpp \
		server-module/ServerManager.cpp server-module/Server.cpp server-module/EventHandler.cpp \
		server-module/ReceiveRequests.cpp server-module/Client.cpp server-module/generateResponse.cpp \
		server-module/SendResponse.cpp server-module/HttpRequest.cpp \
		raii-module/Socket.cpp

OBJS := $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))
DEPS := $(OBJS:.o=.d)

# UNIT_TEST
UNIT_NAME := unit

TEST_DIR := test
UNIT_SRCS := $(filter-out server-module/main.cpp main.cpp, $(SRCS))

TEST_SRCS := unit_test.cpp \
						 testTokenizer.cpp testConfigPrint.cpp \
						 testSharedPtr.cpp testSocket.cpp testHttpRequestParse.cpp

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
