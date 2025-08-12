def NoSessionFound():
	response_header = ""
	response_header += "Content-Type: text/html\r\n"
	response_header += "Status: 302\r\n"
	response_header += "Location: /sessions.hmtl\r\n"
	response_header += "Content-Lenght: 0\r\n"
	print(response_header, end="\r\n")

NoSessionFound()