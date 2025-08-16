
#include "ServerManager.hpp"

void	isolateAndRecordBody(Client& client) {
	std::string& headerPart = client.getHeaderPart();

	if (!headerPart.size()) {
		// no body-data received after header ==> no need to save
		client.setRequestDataPreloadedFlag(REQUEST_DATA_PRELOADED_OFF);
		client.setBodyDataPreloadedFlag(BODY_DATA_PRELOADED_OFF);
		return ;
	}
	
	// some body-data received after header-data ==> it needs to be saved and removed from header part
	
	client.setPendingRequestData(headerPart); // here we save the data ('next request' or 'current request body')
	client.getHeaderPart().clear();
	client.setRequestDataPreloadedFlag(REQUEST_DATA_PRELOADED_ON);
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
		if (client.getRequestDataPreloadedFlag() == REQUEST_DATA_PRELOADED_ON)
			client.getBufferFromPendingData(_buffer, &readbytes);
		else
			readbytes = client.getSocket().recv((void*)_buffer, BYTES_TO_READ, MSG_DONTWAIT); // Enable NON_Blocking for recv()
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
			client.setIncomingHeaderDataDetectedFlag(INCOMING_HEADER_DATA_OFF);
			client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
			// std::cout << " ==> Empty line\n";
		}

		HttpRequest &req = client.getHttpRequest();
		req.appendAndValidate(client.getHeaderPart());
	
		if (req.getState() == HttpRequest::STATE_BODY) {
			// std::cout << "   ====>> request is completed <<=====\n";
			isolateAndRecordBody(client);
			client.setIncomingHeaderDataDetectedFlag(INCOMING_HEADER_DATA_OFF);
			client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
		}
	
		if (req.getState() == HttpRequest::STATE_ERROR)
		{
			client.setIncomingHeaderDataDetectedFlag(INCOMING_HEADER_DATA_OFF);
			client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
			return; // Return instead of throwing to allow response generation
		}
	}
	catch (const HttpRequestException &e) {
		std::string error_msg = "HTTP Request Error: ";
		error_msg += e.what();
		std::cerr << error_msg << std::endl;
		client.setIncomingHeaderDataDetectedFlag(INCOMING_HEADER_DATA_OFF);
		client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
	}
	catch (const std::exception &e) {
		std::string error_msg = "Parsing error: ";
		error_msg += e.what();
		std::cerr << error_msg << std::endl;
		client.setIncomingHeaderDataDetectedFlag(INCOMING_HEADER_DATA_OFF);
		client.setGenerateResponseInProcess(GENERATE_RESPONSE_ON);
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

	if (client.getIsPipeClosedByPeer() == PIPE_IS_CLOSED || client.getIsPipeClosedByPeer() == PIPE_CLOSED_NO_INPUT) {
		// std::cout << " ***** input is ready to read from Pipe : " << client.getResponseHandler()->GetCgiOutPipe().getReadFd() << "  ****\n";
		consumeCgiOutput(client);
	}
	else if (client.getIncomingHeaderDataDetectedFlag() == INCOMING_HEADER_DATA_ON) {
		// std::cout << " ***** incoming Header data from : " << client.getSocket().getFd() << "  ****\n";
		collectRequestData(client);
	}
	else if (client.getIncomingBodyDataDetectedFlag() == INCOMING_BODY_DATA_ON) {
		// std::cout << " ***** incoming Body data from : " << client.getSocket().getFd() << "  ****\n";
		transferBodyToFile(client);
	}
	eraseMarked();
}
