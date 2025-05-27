NAME = response

CXX = c++

CXXFLAGS = -std=c++98 # -Wall -Wextra -Werror

SRC = main.cpp ResponseManager.cpp Response.cpp Request.cpp \
		CreateResponse.cpp Response.utils.cpp
 
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJ)

fclean: clean
	@rm -f $(NAME)

re: fclean all
