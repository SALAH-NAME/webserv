CONF_SRC=$(shell find ../config -name "*.cpp")
CGI_SRC=$(shell find ../cgi -name "*.cpp")
RES_SRC=$(shell find ../response -name "*.cpp")
RAII_SRC=$(shell find ../raii -name "*.cpp")
Test_SRC=$(shell find test-cases -name "*.cpp")
CPP_FLAGS = -fsanitize=address -g3 -Wall -Wextra -Werror -I ../../include/

OBJ_DIR = ../../obj
ALL_SRCS = $(RAII_SRC) $(CONF_SRC) $(CGI_SRC) $(RES_SRC) $(Test_SRC) ../server/HttpRequest.cpp main.cpp

OBJ = $(patsubst ../%.cpp,$(OBJ_DIR)/%.o,$(ALL_SRCS))

all: $(OBJ)
	c++ $(CPP_FLAGS) $(OBJ) -o test

$(OBJ_DIR)/%.o: ../%.cpp
	@mkdir -p $(dir $@)
	c++ $(CPP_FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f test

