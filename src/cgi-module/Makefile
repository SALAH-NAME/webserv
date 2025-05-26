NAME=cgi
SRC=$(shell ls | grep .cpp) ../server-module/Request.cpp

all:
	c++ -std=c++98 -Wall -Wextra -Werror $(SRC) -o $(NAME)

clean:
	rm cgi