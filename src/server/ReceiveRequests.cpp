
#include "ServerManager.hpp"

void	isolateAndRecordBody(Client& client) {
	std::string& headerPart = client.getHeaderPart();

	if (!headerPart.size()) {
		// no body-data received after header ==> no need to save
		client.setRequestDataPreloadedFlag(OFF);
		return ;
	}
	
	// some body-data received after header-data ==> it needs to be saved and removed from header part
	
	client.setPendingRequestData(headerPart); // here we save the data ('next request' or 'current request body')
	client.getHeaderPart().clear();
	client.setRequestDataPreloadedFlag(ON);
	// std::cout << "ISOLATED\n";
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
				readbytes = client.getSocket().recv((void*)_buffer, BYTES_TO_READ, MSG_DONTWAIT); // Enable NON_Blocking for recv()
			}
			catch(const std::runtime_error& e)
			{
				std::cerr << e.what() << std::endl;
				return ;
			}
		}
		if (readbytes <= 0) {
			if (!readbytes)
				closeConnection(client);
			return ;
		}
		
		// std::cout << "read bytes ==> " << readbytes << " ||  from : " << client.getSocket().getFd() << "\n";
		client.resetLastConnectionTime();
		client.appendToHeaderPart(std::string(_buffer, readbytes));
	
		if (std::string(_buffer, readbytes) == "\r\n")
		{
			// in case of receive empty line (Press Enter) !!
			client.setInputState(INPUT_NONE);
			client.setGenerateResponseInProcess(ON);
		}

		HttpRequest &req = client.getHttpRequest();
		req.appendAndValidate(client.getHeaderPart());
	
		if (req.getState() == HttpRequest::STATE_BODY) {
			// std::cout << "   ====>> request is completed <<=====\n";
			isolateAndRecordBody(client);
			client.setInputState(INPUT_NONE);
			client.setGenerateResponseInProcess(ON);
		}
	
		if (req.getState() == HttpRequest::STATE_ERROR)
		{
			client.setInputState(INPUT_NONE);
			client.setGenerateResponseInProcess(ON);
			return; // Return instead of throwing to allow response generation
		}
	}
	catch (const HttpRequestException &e) {
		std::string error_msg = "HTTP Request Error: ";
		error_msg += e.what();
		std::cerr << error_msg << std::endl;
		client.setInputState(INPUT_NONE);
		client.setGenerateResponseInProcess(ON);
	}
	catch (const std::exception &e) {
		std::string error_msg = "Parsing error: ";
		error_msg += e.what();
		std::cerr << error_msg << std::endl;
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
			closeConnection(client);
			// close the connection at the moment // should send a resposne first
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
