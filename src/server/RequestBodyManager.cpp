#include "Client.hpp"

size_t	getBytesToExtract(size_t dataSize, size_t contentLength, size_t uploadedBytes) {
	size_t	bytesToExtract;
	size_t	leftByets = contentLength - uploadedBytes;

	if (leftByets >= dataSize)
		bytesToExtract = dataSize;
	else
		bytesToExtract = leftByets;

	return bytesToExtract;
}

void	Client::receiveFromSocket(void) {
	char	buffer[BYTES_TO_READ+1];
	std::memset(buffer, 0, sizeof(buffer));

	ssize_t	readBytes = _socket.recv(buffer, BYTES_TO_READ, MSG_DONTWAIT); // Enable NON_Blocking for recv()
	if (readBytes == -1)
		return ;
	if (readBytes > 0) {
		resetLastConnectionTime();
		buffer[readBytes] = 0;
		_pendingRequestDataHolder += std::string(buffer, readBytes);
		
		if (_isChunked && _chunkBodySize == -1)
			_state = ValidateChunkSize;
		else
			_state = ExtractingBody;
	}
	else if (!readBytes)
		_state = CloseConnection;
	else
		throwIfSocketError("recv()");
}

void	Client::validateChunkBodySize(void) {
	if (_isChunked) {

		if (_pendingRequestDataHolder.find(_CRLF) != std::string::npos) {
			try {
				_chunkBodySize = _httpRequest.validateChunkSize(_pendingRequestDataHolder);

			} catch (...) {
				std::cout << "catching an Excepion\n";
				_state = InvalidBody;
				return ;
			}
		}
		else
			return ; // *keep the same state "ReceivingData" *need to find "CRLF"
	}
	_state = ExtractingBody;
}

void	Client::extractBodyFromPendingData(void) {
	if (!_pendingRequestDataHolder.size()) {
		_state = ReceivingData;
		return ;
	}

	size_t BytesToExtract;

	if (_isChunked) {
		if (_pendingRequestDataHolder.size() >= (static_cast<size_t>(_chunkBodySize + 2))) // check if the data it enough including "\r\n"
			BytesToExtract = _chunkBodySize + 2;
		else {
			_state = ReceivingData; // data is not enough need to receive more data
			return ;
		}
	}
	else
		BytesToExtract = getBytesToExtract(_pendingRequestDataHolder.size(), _contentLength, _uploadedBytes);

	_requestBodyPart += _pendingRequestDataHolder.substr(0, BytesToExtract);
	_pendingRequestDataHolder = _pendingRequestDataHolder.substr(BytesToExtract);
	if (!_pendingRequestDataHolder.size())
		_requestDataPreloadedFlag = REQUEST_DATA_PRELOADED_OFF;

	if (_isChunked) {
		if (_chunkBodySize == 0) {
			if (_requestBodyPart != _CRLF) // when the chunkBodySize is 0 it's requires to receive "CRLF" in the last chunked
				_state = InvalidBody;
			else
				_state = Completed;
			return ;
		}
		if (_requestBodyPart.substr(BytesToExtract-2) != _CRLF) {
			_state = InvalidBody; // missed "CRFL" after chunked body "it's required after any chunked data"
			return ;
		}
		else {
			if (_pipeBodyToCgi)
				_state = PipingToCGI;
			else
				_state = UploadingToFile;
		}
	}
	else {
		if (_pipeBodyToCgi)
			_state = PipingToCGI;
		else
			_state = UploadingToFile;
	}
}

void	Client::writeBodyToTargetFile(void) {

	std::fstream *targetFile = _responseHandler->GetTargetFilePtr();
	size_t bytesToWrite;

	if (_isChunked)
		bytesToWrite = _chunkBodySize;
	else
		bytesToWrite = _requestBodyPart.size();
	
	targetFile->write(_requestBodyPart.c_str(), bytesToWrite);
	targetFile->flush();
	
	_uploadedBytes += bytesToWrite;

	if (_isChunked) {
		_chunkBodySize = -1;
		if (_uploadedBytes == _conf.getClientMaxBodySize())
			_state = Completed;
		else
			_state = ReceivingData;
	}
	else {
		if (_uploadedBytes == _contentLength)
			_state = Completed;
		else
			_state = ReceivingData;
	}
	_requestBodyPart.clear();
}

void	Client::pipeBodyToCGI(void) {
	
	if (!_isCgiInputAvailable) // the pipe isn't available yet to write in	
			return ;

	size_t bytesToWrite;
	if (_isChunked)
		bytesToWrite = _chunkBodySize;
	else
		bytesToWrite = _requestBodyPart.size();
 
	ssize_t pipedBytes = write(_CGI_InPipeFD, _requestBodyPart.c_str(), bytesToWrite);
	if (pipedBytes < 0)
		return ; // An error occured // we keep the same state, we will try to write again until write success or get timeout
	
	_chunkBodySize = -1;
	_uploadedBytes += bytesToWrite;

	if (_isChunked) {
		if (_uploadedBytes == _conf.getClientMaxBodySize())
			_state = Completed;
		else {
			if (_pendingRequestDataHolder.size())
				_state = ValidateChunkSize;
			else
				_state = ReceivingData;
		}
	}
	else {
		if (_uploadedBytes == _contentLength)
			_state = Completed;
		else
			_state = ReceivingData;
	}
	_requestBodyPart.clear();

}

void	Client::finalizeBodyProccess(void) {
	
	_incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_OFF;
	_state = DefaultState;
	if (_pipeBodyToCgi) {
		_pipeBodyToCgi = NO_PIPE;
		close(_CGI_InPipeFD);
		_CGI_InPipeFD = -1;
	}
	else
		_fullResponseFlag = FULL_RESPONSE_READY;
}

void	Client::handleInvalidBody(void) {
	_state = DefaultState;
	_incomingBodyDataDetectedFlag = INCOMING_BODY_DATA_OFF;
	// here where to handle the invalid cases
	// need to generate a response
}