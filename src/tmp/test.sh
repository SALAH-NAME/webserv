CONF_SRC=$(find ../config-module -mindepth 1 -maxdepth 1)  
CGI_SRC=$(find ../cgi-module -mindepth 1 -maxdepth 1)
RES_SRC=$(find ../response-module -mindepth 1 -maxdepth 1)

FLAGS="-Wall -Wextra -Werror -std=c++98 -I ../../include/"
c++ $FLAGS main.cpp $CONF_SRC $CGI_SRC $RES_SRC Request.cpp