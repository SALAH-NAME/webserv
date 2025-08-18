
#include "ServerManager.hpp"
#include "SimpleLogger.hpp"

void	isolateAndRecordBody(Client& client) {
	std::string& headerPart = client.getHeaderPart();

	if (!headerPart.size()) {
		client.setRequestDataPreloadedFlag(OFF);
		return ;
	}
	
	client.setPendingRequestData(headerPart);
	client.getHeaderPart().clear();
	client.setRequestDataPreloadedFlag(ON);
}

void    ServerManager::collectRequestData(Client& client) {
	ssize_t	readbytes;

	if (client.getResponseBodyFlag()) {
		client.setPendingHeaderFlag(true);
		return ;
	}

	std::memset(_buffer, 0, sizeof(_buffer));
	try {
		if (client.getRequestDataPreloadedFlag() == ON)
			client.getBufferFromPendingData(_buffer, &readbytes);
		else
		{
			try
			{
				readbytes = client.getSocket().recv((void*)_buffer, BYTES_TO_READ, MSG_DONTWAIT);
			}
			catch(const std::runtime_error& e)
			{
				std::string clientIP = client.getClientInfos().clientAddr;
				int clientPort = std::atoi(client.getClientInfos().port.c_str());
				LOG_ERROR_CLIENT("Socket receive error", clientIP, clientPort);
				LOG_ERROR_F("Socket error details: {}", e.what());
				return ;
			}
		}
		if (readbytes <= 0) {
			if (!readbytes)
				closeConnection(client);
			return ;
		}

		client.resetLastConnectionTime();
		client.appendToHeaderPart(std::string(_buffer, readbytes));
	
		if (std::string(_buffer, readbytes) == "\r\n")
		{
			client.setInputState(INPUT_NONE);
			client.setGenerateResponseInProcess(ON);
		}

		HttpRequest &req = client.getHttpRequest();
		req.appendAndValidate(client.getHeaderPart());
	
		if (req.getState() == HttpRequest::STATE_BODY) {
			std::string clientIP = client.getClientInfos().clientAddr;
			int clientPort = std::atoi(client.getClientInfos().port.c_str());
			LOG_REQUEST(clientIP, clientPort, req.getMethod(), req.getUri());

			isolateAndRecordBody(client);
			client.setInputState(INPUT_NONE);
			client.setGenerateResponseInProcess(ON);
		}
	
		if (req.getState() == HttpRequest::STATE_ERROR)
		{
			client.setInputState(INPUT_NONE);
			client.setGenerateResponseInProcess(ON);
			return;
		}
	}
	catch (const HttpRequestException &e) {
		std::string clientIP = client.getClientInfos().clientAddr;
		int clientPort = std::atoi(client.getClientInfos().port.c_str());
		LOG_ERROR_CLIENT("HTTP request parsing error", clientIP, clientPort);
		LOG_ERROR_F("HTTP request error details: {}", e.what());

		client.setInputState(INPUT_NONE);
		client.setGenerateResponseInProcess(ON);
	}
	catch (const std::exception &e) {
		std::string clientIP = client.getClientInfos().clientAddr;
		int clientPort = std::atoi(client.getClientInfos().port.c_str());
		LOG_ERROR_CLIENT("Request parsing error", clientIP, clientPort);
		LOG_ERROR_F("Parsing error details: {}", e.what());
		client.setInputState(INPUT_NONE);
		client.setGenerateResponseInProcess(ON);
	}
}

void	ServerManager::transferBodyToFile(Client& client) {

	PostMethodProcessingState& state = client.getState();	

	switch (state) {

		case DefaultState:
			break;

		case ReceivingData:
        	client.receiveFromSocket();
        	break;

		case ExtractingBody:
			client.extractBodyFromPendingData();
			break;
		
		case ValidateChunkSize:
			client.validateChunkBodySize();
			break ;
		
		case UploadingToFile:
        	client.writeBodyToTargetFile();
        	break;
		
    	case PipingToCGI:
        	client.pipeBodyToCGI();
        	break;
		
		case Completed:
        	client.finalizeBodyProccess();
        	break;
		
		case CloseConnection:
			closeConnection(client);
			break ;

		case InvalidBody:
			client.handleInvalidBody();
	}
}

void	ServerManager::receiveClientsData(int i) {
	std::map<int, Client>::iterator it = _clients.find(_events[i].data.fd);
	if (it == _clients.end())
		return ;

	Client& client = it->second;
	ClientInputState inputState = client.getInputState();

	switch (inputState)
	{
		case INPUT_NONE:
			break ;

		case INPUT_HEADER_READY:
			collectRequestData(client);
			break ;
		
		case INPUT_BODY_READY:
			transferBodyToFile(client);
			break;
		
		case INPUT_PIPE_HAS_DATA:
			consumeCgiOutput(client);
			break ;
		
		case INPUT_PIPE_NO_DATA:
			consumeCgiOutput(client);
			break ;
	}
	eraseMarked();
}
