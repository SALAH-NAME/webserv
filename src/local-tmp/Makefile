NAME=tester
CONF_SRC=$(shell find ../config -name "*.cpp")
CGI_SRC=$(shell find ../cgi -name "*.cpp")
RES_SRC=$(shell find ../response -name "*.cpp")
RAII_SRC=$(shell find ../raii -name "*.cpp")
Test_SRC=$(shell find ./test-cases/tests-srcs -name "*.cpp")
CPP_FLAGS = -g3 -Wall -Wextra -Werror -I ../../include/

OBJ_DIR = ../../obj
ALL_SRCS = $(RAII_SRC) $(CONF_SRC) $(CGI_SRC) $(RES_SRC) ../server/HttpRequest.cpp main.cpp

OBJ = $(patsubst ../%.cpp,$(OBJ_DIR)/%.o,$(ALL_SRCS)) $(patsubst ./%.cpp,$(OBJ_DIR)/%.o,$(Test_SRC))

all: $(OBJ)
	c++ $(CPP_FLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: ../%.cpp
	@mkdir -p $(dir $@)
	c++ $(CPP_FLAGS) -c $< -o $@

$(OBJ_DIR)/test-cases/tests-srcs/%.o: ./test-cases/tests-srcs/%.cpp
	@mkdir -p $(dir $@)
	c++ $(CPP_FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: clean all

d:
	@echo $(Test_SRC)
