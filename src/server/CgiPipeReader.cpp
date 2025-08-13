#include "ServerManager.hpp"

void    ServerManager::consumeCgiOutput(Client& client, int serverIndex) {

	(void)serverIndex;

	ResponseHandler* responseHandler = client.getResponseHandler();
	Pipe& cgiOutPipe = responseHandler->GetCgiOutPipe();
	ssize_t readBytes;
	char    buffer[BYTES_TO_READ+1];

	if (client.getIsPipeClosedByPeer() == PIPE_CLOSED_NO_INPUT) {
		responseHandler->LoadErrorPage(client.getHttpRequest().getVersion() + " 502 Bad Gateway", 502);
		client.CgiExceptionHandler();
		return ;
	}

	if (!responseHandler->ReachedCgiBodyPhase()) {
		// reading CGI header
		readBytes = cgiOutPipe.read(buffer, BYTES_TO_READ); // reade cgi headers
		std::cout << "read header bytes from CGI: " << readBytes << "\n";
		if (readBytes > 0) {
			buffer[readBytes] = 0;
			try {
				responseHandler->AppendCgiOutput(buffer);
			} catch (ResponseHandler::ResponseHandlerError& e) {
				client.setIsPipeClosedByPeer(PIPE_IS_NOT_CLOSED);
				client.setIsPipeReadable(PIPE_IS_NOT_READABLE);
				responseHandler->LoadErrorPage(e.what(), e.getStatusCode());

				client.CgiExceptionHandler();
			}
		}
		else if (!readBytes){
			try {
				responseHandler->CheckForContentType();

				client.setIsPipeClosedByPeer(PIPE_IS_NOT_CLOSED);
				client.setPipeReadComplete(READ_PIPE_COMPLETE);

			} catch (ResponseHandler::ResponseHandlerError& e) {
				client.setIsPipeClosedByPeer(PIPE_IS_NOT_CLOSED);
				client.setIsPipeReadable(PIPE_IS_NOT_READABLE);
				responseHandler->LoadErrorPage(e.what(), e.getStatusCode());

				client.CgiExceptionHandler();
			}

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

				client.CgiExceptionHandler();
			}
		}

		readBytes = cgiOutPipe.read(buffer, BYTES_TO_READ);
		// std::cout << "read body bytes from pipe: " << readBytes << "\n";
		if (readBytes > 0) {
			buffer[readBytes] = 0;
			try {
				responseHandler->AppendBufferToTmpFile(buffer);
			} catch (ResponseHandler::ResponseHandlerError& e) {
				client.setIsPipeClosedByPeer(PIPE_IS_NOT_CLOSED);
				client.setIsPipeReadable(PIPE_IS_NOT_READABLE);
				responseHandler->LoadErrorPage(e.what(), e.getStatusCode());
				
				client.CgiExceptionHandler();
			}
		}
		else if (!readBytes) {
			try {
				responseHandler->FinishCgiResponse();
				client.setResponseHolder(responseHandler->GetResponseHeader());
				// printRequestAndResponse("response header", client.getResponseHolder());
				client.setResponseHeaderFlag(RESPONSE_HEADER_READY);
				client.setIsPipeClosedByPeer(PIPE_IS_NOT_CLOSED);
				client.closeAndDeregisterPipe(client.getCGI_OutpipeFD());
				responseHandler->GetTargetFilePtr()->seekg(0);
				
			} catch (ResponseHandler::ResponseHandlerError& e) {
				client.setIsPipeClosedByPeer(PIPE_IS_NOT_CLOSED);
				client.setIsPipeReadable(PIPE_IS_NOT_READABLE);
				responseHandler->LoadErrorPage(e.what(), e.getStatusCode());

				client.CgiExceptionHandler();
			}
		}
	}
}
