#include "Request.hpp"

Request::Request(){return ;}


void   Request::set_method(const std::string &method){this->method = method;}
void   Request::set_path(const std::string &path){this->path = path;}
void   Request::set_http_version(const std::string &http_version){this->http_version = http_version;}
void   Request::set_interpiter(const std::string &interpiter){this->interpiter = interpiter;}
void   Request::set_path_info(const std::string &path_info){this->path_info = path_info;}
void   Request::set_query_string(const std::string &query_string){this->query_string = query_string;}
void   Request::set_content_type(const std::string &content_type){this->content_type = content_type;}
void   Request::set_content_length(const std::string &content_length){this->content_length = content_length;}
void   Request::set_server_name(const std::string &server_name){this->server_name = server_name;}
void   Request::set_server_port(int server_port){this->server_port = server_port;}
void   Request::set_client_addrs(const std::string &client_addrs){this->client_addrs = client_addrs;}
void   Request::set_client_port(int client_port){this->client_port = client_port;}
void   Request::set_headers(const std::map<std::string, std::string> &headers){this->headers = headers;}
void   Request::set_cgi_required(bool cgi_required){this->cgi_required = cgi_required;}


std::string Request::getMethod(){return method;}

std::string Request::getPath(){return path;}

std::string Request::getHttpVersion(){return http_version;}

std::string Request::getInterpiter(){return interpiter;}

std::string Request::getPathInfo(){return path_info;}

std::string Request::getQueryString(){return query_string;}

std::string Request::getContentType(){return content_type;}

std::string Request::getContentLength(){return content_length;}

std::string Request::getServerName(){return server_name;}

int Request::getServerPort(){return server_port;}

std::string Request::getClientAddrs(){return client_addrs;}

int Request::getClientPort(){return client_port;}

std::map<std::string, std::string> &Request::getHeaders(){return headers;}

bool Request::require_cgi(){return cgi_required;}
