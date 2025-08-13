from sys import exit, stdin, stderr
from os import mkdir, environ, access, remove, F_OK, R_OK
from random import random

session_max_size = 256
session_id = 0
location = "/tmp/webserv-sessions/"
id_created = False

response_header = ""
log_file = open("/home/midbella/Desktop/webserv/logs.txt", "w+") 

def logger(msg, end='\n'):
	global log_file
	print(msg, file=log_file, end=end)

def SetSessionId():
	global session_id
	logger("setting the session id")
	if "HTTP_cookie" not in environ:
		logger ("no cookie set")
		return
	cookies = environ["HTTP_cookie"]
	logger(f"cookie header value: {cookies}")
	key = ""
	value = ""
	for pairs in cookies.split(';'):
		logger("current elm :", end= '')
		logger(pairs)
		key = pairs.split('=')[0].strip()
		value = pairs.split('=')[1].strip()
		logger(f"key= {key}, value= {value}")
		if key == "session-id":
			session_id = int(value)
			return
	logger("session-id 'cookie' is not found")
	return

def isPost():
	if "REQUEST_METHOD" not in environ:
		exit(1)
	return (environ["REQUEST_METHOD"] == "POST")

def CreateSession():
	global session_id
	global id_created
	global location

	if session_id == 0:
		session_id = round(random() * 1000000)
		id_created = True
	if not access(location, F_OK):
		mkdir(location)
	if id_created:
		while access(f"{location}session_{session_id}", F_OK):
			session_id = round(random() * 1000000)
	return open(f"{location}session_{session_id}", "w+")
	

def	WriteToSession():
	global session_id
	global response_header
	global location
	global session_max_size
	current_size = 0

	logger("inside write to session (post req)")
	response_header += "Content-Type: text/html\r\n"
	response_header += "Status: 201 \r\n"
	sessionFile = CreateSession()
	logger(f"created a session id: {id_created}")
	logger(f"session id = {session_id}")
	if not id_created:
		remove(f"{location}session_{session_id}")
		sessionFile = open(f"{location}session_{session_id}", "w+")
		logger("deleted old and created new session file with the same name")
	if not sessionFile.writable():
		logger("can't write in file")
		exit(1)
	logger("before reading stdin loop\n", end='')
	while 1:
		line = stdin.readline()
		if not line:
			logger("line is invalid")
			break
		if current_size + len(line) > session_max_size:
			logger("reached max session size")
			remove(f"{location}session_{session_id}")
			exit(1)
		logger("will appen line : [" + line +"]")
		print(line, file=sessionFile, end='')
		current_size += len(line)
	response_header += f"Set-Cookie: session-id={session_id}\r\n\r\n"
	print(response_header, end="")
	print(
"""
<html>
	<header>
		<title>Successful session creation</title>
	</header>
	<body>
		<h1>Success!</h1>
		<p>
			The submitted data is now stored on the server.<br>
			If the server host restarts, the session will be deleted.<br>
			The server recognizes you by the value of the session-id cookie.<br>
			If the session-id cookie expires or changes, the server will redirect<br>
			to the previous page so you can create a new session.<br>
		</p>
		<form action="sessionManager.py" method="get">
			<button type="submit">Fetch your session data</button>
		</form>
	</body>
</html>
"""
)
	logger("generated res header :")
	for c in response_header:
		if c == '\r':
			logger("\\r", end='')
		elif c == '\n':
			logger('\\n')
		else:
			logger(c, end='')

def NoSessionFound():
	global response_header

	response_header += "Content-Type: text/html\r\n"
	response_header += "Status: 302\r\n"
	response_header += "Location: /sessions.html\r\n"
	response_header += "Content-Length: 0\r\n"
	print(response_header, end="\r\n")

def formatSessionLine(line):
	i = 0
	while i in range(len(line)):
		if line[i] == '&':
			print("<br>")
		elif line[i] == '%':
			print('@', end='')
		else:
			print(line[i], end='')
		i += 1

def ListSessionData():
	global location
	global response_header

	sessionFile = open(f"{location}session_{session_id}", "r")
	if not sessionFile.readable:
		exit(1)
	logger(f"{location}session_{session_id}")
	response_header += "Content-Type: text/html\r\n"
	response_header += "status: 200\r\n"
	print(response_header, end="\r\n")
	print(
	"""
<!DOCTYPE html>
<html>
	<header>
		<title>fetched session data</title>
	</header>
	<body>
	   	<h1>the session data:</h1>
	   	<p>\n""")
	for line in sessionFile:
		logger(f"line = {line}")
		formatSessionLine(line)
	print("""
		</p>
	</body>
</html>
""")

def SendSessionData():
	global session_id
	global location

	if session_id == 0 or not access(f"{location}session_{session_id}", F_OK):
		NoSessionFound()
	else:
		ListSessionData()


def main():
	global session_id
	SetSessionId()
	logger(f"session id value: {session_id}")
	if isPost():
		logger("recieved a POST request")
		WriteToSession()
	else:
		logger("recieved a GET request")
		SendSessionData()

main()

