#include "ServerManager.hpp"

void    ServerManager::consumeCgiOutput(Client& client, int serverIndex) {

	(void)serverIndex;

	if (client.getIsPipeClosedByPeer() == PIPE_IS_NOT_CLOSED)
		return ;
	
	// if (client.getIsPipeReadable() == PIPE_IS_READABLE) {
	ResponseHandler* responseHandler = client.getResponseHandler();
	Pipe& cgiOutPipe = responseHandler->GetCgiOutPipe();
	ssize_t readBytes;
	char    buffer[BYTES_TO_READ+1];

	if (!responseHandler->ReachedCgiBodyPhase()) {
		// reading CGI header
		readBytes = cgiOutPipe.read(buffer, BYTES_TO_READ); // reade cgi headers
		// std::cout << "read header bytes: " << readBytes << "\n";
		if (readBytes > 0) {
			buffer[readBytes] = 0;
			try {
				responseHandler->AppendCgiOutput(buffer);
			} catch (ResponseHandler::ResponseHandlerError& e) {
				client.setIsPipeClosedByPeer(PIPE_IS_NOT_CLOSED);
				client.setIsPipeReadable(PIPE_IS_NOT_READABLE);
				responseHandler->LoadErrorPage(e.what(), e.getStatusCode());
			}
		}
		else if (!readBytes){
			client.setIsPipeClosedByPeer(PIPE_IS_NOT_CLOSED);
			client.setPipeReadComplete(READ_PIPE_COMPLETE);
		}
		else {
			// read failed !!
		}
	}
	else {
		// reading CGI body

		if (!client.getSetTargetFile()) {
			client.setSetTargetFile(true);
			try {
				client.getResponseHandler()->SetTargetFileForCgi(client.getSocket().getFd());
			} catch (ResponseHandler::ResponseHandlerError& e) {
				client.setIsPipeClosedByPeer(PIPE_IS_NOT_CLOSED);
				client.setIsPipeReadable(PIPE_IS_NOT_READABLE);
				client.getResponseHandler()->LoadErrorPage(e.what(), e.getStatusCode());
			}
		}

		readBytes = cgiOutPipe.read(buffer, BYTES_TO_READ);
		// std::cout << "read body bytes: " << readBytes << "\n";
		if (readBytes > 0) {
			buffer[readBytes] = 0;
			try {
				responseHandler->AppendBufferToTmpFile(buffer);
			} catch (ResponseHandler::ResponseHandlerError& e) {
				client.setIsPipeClosedByPeer(PIPE_IS_NOT_CLOSED);
				client.setIsPipeReadable(PIPE_IS_NOT_READABLE);
				responseHandler->LoadErrorPage(e.what(), e.getStatusCode());
			}
		}
		else if (!readBytes) {
			try {
				responseHandler->FinishCgiResponse();
				client.setResponseHolder(responseHandler->GetResponseHeader());
				client.setResponseHeaderFlag(RESPONSE_HEADER_READY);
				client.setIsPipeClosedByPeer(PIPE_IS_NOT_CLOSED);
				client.closeAndDeregisterPipe();
				responseHandler->GetTargetFilePtr()->seekg(0);
				
			} catch (ResponseHandler::ResponseHandlerError& e) {
				client.setIsPipeClosedByPeer(PIPE_IS_NOT_CLOSED);
				client.setIsPipeReadable(PIPE_IS_NOT_READABLE);
				responseHandler->LoadErrorPage(e.what(), e.getStatusCode());
			}
		}
	}
}
