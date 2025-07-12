CONF_SRC=$(shell find ../config-module -name "*.cpp")
CGI_SRC=$(shell find ../cgi-module -name "*.cpp")
RES_SRC=$(shell find ../response-module -name "*.cpp")
Test_SRC=$(shell find test-cases -name "*.cpp")
CPP_FLAGS = -Wall -Wextra -Werror -fsanitize=address -g3 -std=c++98 -I ../../include/

OBJ_DIR = ../../obj
ALL_SRCS = $(CONF_SRC) $(CGI_SRC) $(RES_SRC) $(Test_SRC) main.cpp Request.cpp

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

