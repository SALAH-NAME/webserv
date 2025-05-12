NAME = server
CXX = c++
CXXFLAGS =--std=c++98 -fsanitize=address -Wall -Wextra -Werror
SERVER_SRC = main.cpp server.cpp event_handler.cpp request.cpp receive_Requests.cpp \
			response.cpp sendResponse.cpp
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)

all: $(NAME)

$(NAME):	$(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) $(SERVER_OBJ) -o $(NAME)

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(SERVER_OBJ)

fclean: clean
	@rm -f $(NAME)

re: fclean all