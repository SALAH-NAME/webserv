from sys import exit, stdin
from os import mkdir, environ, access, remove, F_OK, R_OK
from random import random

#to do: limit session number and size

session_id = 0
location = "/tmp/webserv-sessions/"
id_created = False

response_header = ""

def SetSessionId():
	global session_id
	print("setting the session id") #logger
	if "HTTP_cookie" not in environ:
		print ("no cookie set") #logger
		return
	cookies = environ["HTTP_cookie"]
	key = ""
	value = ""
	for pairs in cookies.split(';'):
		print("current elm :", end= '') #logger
		print(pairs) #logger
		key = pairs.split('=')[0]
		value = pairs.split('=')[1]
		if key == "session-id":
			session_id = int(value)
			return
	print("session-id 'cookie' is not found") #logger
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

	print("inside write to session")#logger
	response_header += "Content-Type: text/plain\r\n"
	response_header += "Content-Length: 0\r\n"
	response_header += "location: sessions.html"
	sessionFile = CreateSession()
	print(f"created a session id: {id_created}")#logger
	if not id_created:
		remove(f"{location}session_{session_id}")
		sessionFile = open(f"{location}session_{session_id}", "w+")
		print("deleted old and created new session file with the same name")#logger
	if not sessionFile.writable():
		print("shit can't write in file")#logger
		exit(1)
	print("before reading stdin loop\n", end='')#logger
	while True:
		line = stdin.readline()
		if not line:
			break
		print(line, file=sessionFile, end='')
	response_header += f"Set-Cookie: session-id={session_id}\r\n"
	print(response_header, end="\r\n")

def NoSessionFound():
	global response_header

	response_header += "Content-Type: text/html\r\n"
	response_header += "Status: 302\r\n"
	response_header += "Location: /sessions.hmtl\r\f"
	response_header += "Content-Lenght: 0"
	print(response_header, end="\r\n")

def ListSessionData():
	global location
	global response_header

	sessionFile = open(f"{location}session_{session_id}", "r")
	if not sessionFile.readable:
		exit(1)
	response_header += "Content-Type: text/html\r\n"
	response_header += "status: 200\r\n"
	print(response_header, end="\r\n")
	print(
	"""
<html>
	<header>
		<title>fetched session data</title>
	</header>
	<body>
	   	<h1>the session data:</h1>
	   	<p>\n""")
	for line in sessionFile:
		print("\t\t\t" + line + "<br>", end="")
	print("""
		<\p>
	<body>
</html>\n""")

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
	print(f"session id value: {session_id}")
	if isPost():
		WriteToSession()
	else:
		SendSessionData()

main()

