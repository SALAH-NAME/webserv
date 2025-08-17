from sys import exit, stdin, stderr
from os import mkdir, environ, access, remove, F_OK, R_OK, makedirs, path
from random import random
from urllib.parse import unquote


session_max_size = 256
session_id = 0
location = "/tmp/webserv-sessions/"
id_created = False

log_dir = "/tmp/webserv"
log_file_path = path.join(log_dir, "logs.txt")

makedirs(log_dir, exist_ok=True)

if not path.exists(log_file_path):
    open(log_file_path, "w").close()

response_header = ""
log_file = open(log_file_path, "w+")

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
"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Session Created - WebServ</title>
    <link rel="stylesheet" href="/style.css">
    <style>
        .success-container {
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            padding: var(--spacing-xl);
            position: relative;
        }

        .success-card {
            background: var(--color-bg-glass);
            border: 2px solid var(--color-border-primary);
            border-radius: var(--border-radius-lg);
            padding: var(--spacing-2xl);
            backdrop-filter: blur(15px);
            max-width: 700px;
            width: 100%;
            text-align: center;
            position: relative;
            animation: cardPulse 4s ease-in-out infinite;
        }

        .success-card::before {
            content: '';
            position: absolute;
            top: -3px;
            left: -3px;
            right: -3px;
            bottom: -3px;
            background: linear-gradient(45deg, #00ff00, #00ffff);
            border-radius: var(--border-radius-lg);
            z-index: -1;
            opacity: 0.6;
            animation: successBorder 3s ease-in-out infinite;
        }

        .success-title {
            font-size: var(--font-size-4xl);
            font-weight: var(--font-weight-extra-bold);
            color: #00ff00;
            text-transform: uppercase;
            letter-spacing: 0.1em;
            margin-bottom: var(--spacing-xl);
            text-shadow: 0 0 20px #00ff00;
            animation: successGlow 2s ease-in-out infinite alternate;
        }

        .success-description {
            font-size: var(--font-size-base);
            color: var(--color-text-secondary);
            line-height: 1.6;
            margin-bottom: var(--spacing-2xl);
        }

        .fetch-button {
            background: var(--gradient-primary);
            border: none;
            border-radius: var(--border-radius-base);
            padding: var(--spacing-base) var(--spacing-2xl);
            color: var(--color-text-primary);
            font-weight: var(--font-weight-bold);
            font-family: inherit;
            font-size: var(--font-size-lg);
            cursor: pointer;
            transition: all var(--transition-base);
            text-transform: uppercase;
            letter-spacing: 0.1em;
            position: relative;
            overflow: hidden;
        }

        .fetch-button:hover {
            transform: scale(1.05) translateY(-2px);
            box-shadow: var(--shadow-button);
        }

        @keyframes cardPulse {
            0%, 100% { box-shadow: 0 0 20px rgba(0, 255, 0, 0.3); }
            50% { box-shadow: 0 0 40px rgba(0, 255, 255, 0.3); }
        }

        @keyframes successBorder {
            0%, 100% { opacity: 0.6; }
            50% { opacity: 0.9; }
        }

        @keyframes successGlow {
            0%, 100% { text-shadow: 0 0 20px #00ff00; }
            50% { text-shadow: 0 0 30px #00ff00, 0 0 40px #00ffff; }
        }
    </style>
</head>
<body>
    <div class="success-container">
        <div class="success-card">
            <h1 class="success-title">Success!</h1>
            <div class="success-description">
                <p style="color: #222;">
                    The submitted data is now stored on the server.<br>
                    If the server host restarts, the session will be deleted.<br>
                    The server recognizes you by the value of the session-id cookie.<br>
                    If the session-id cookie expires or changes, the server will redirect<br>
                    to the previous page so you can create a new session.
                </p>
            </div>
            <form action="sessionManager.py" method="get">
                <button type="submit" class="fetch-button">Fetch Session Data</button>
            </form>
        </div>
    </div>
</body>
</html>"""
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
	response_header += "Location: /session\r\n"
	response_header += "Content-Length: 0\r\n"
	print(response_header, end="\r\n")

def formatSessionLine(line):
	decoded_line = unquote(line)
	formatted_line = decoded_line.replace('&', '<br>')
	print(formatted_line, end='')

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
	"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Session Data - WebServ</title>
    <link rel="stylesheet" href="/style.css">
    <style>
        .data-container {
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            padding: var(--spacing-xl);
            position: relative;
        }

        .data-card {
            background: var(--color-bg-glass);
            border: 2px solid var(--color-border-primary);
            border-radius: var(--border-radius-lg);
            padding: var(--spacing-2xl);
            backdrop-filter: blur(15px);
            max-width: 700px;
            width: 100%;
            position: relative;
            animation: dataGlow 4s ease-in-out infinite;
        }

        .data-card::before {
            content: '';
            position: absolute;
            top: -3px;
            left: -3px;
            right: -3px;
            bottom: -3px;
            background: var(--gradient-primary);
            border-radius: var(--border-radius-lg);
            z-index: -1;
            opacity: 0.4;
            animation: borderPulse 3s ease-in-out infinite;
        }

        .data-title {
            font-size: var(--font-size-4xl);
            font-weight: var(--font-weight-extra-bold);
            color: var(--color-primary-cyan);
            text-transform: uppercase;
            letter-spacing: 0.1em;
            margin-bottom: var(--spacing-2xl);
            text-align: center;
            text-shadow: 0 0 20px var(--color-primary-cyan);
            animation: titleGlow 2s ease-in-out infinite alternate;
        }

        .data-content {
            background: rgba(0, 0, 0, 0.3);
            border: 1px solid var(--color-border-primary);
            border-radius: var(--border-radius-base);
            padding: var(--spacing-xl);
            font-family: var(--font-family-primary);
            font-size: var(--font-size-lg);
            line-height: 2;
            color: var(--color-text-primary);
            backdrop-filter: blur(10px);
            position: relative;
            overflow: hidden;
        }

        .data-content::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: linear-gradient(90deg, transparent, rgba(255, 0, 128, 0.1), transparent);
            animation: scanLine 3s ease-in-out infinite;
        }

        .data-field {
            margin-bottom: var(--spacing-base);
            padding: var(--spacing-sm) 0;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
            transition: all var(--transition-base);
        }

        .data-field:last-child {
            border-bottom: none;
            margin-bottom: 0;
        }

        .data-field:hover {
            color: var(--color-primary-pink);
            text-shadow: 0 0 10px var(--color-primary-pink);
            transform: translateX(10px);
        }

        .back-link {
            display: inline-block;
            margin-top: var(--spacing-xl);
            padding: var(--spacing-sm) var(--spacing-base);
            background: var(--color-bg-glass);
            border: 1px solid var(--color-border-primary);
            border-radius: var(--border-radius-base);
            color: var(--color-text-secondary);
            text-decoration: none;
            font-family: inherit;
            font-size: var(--font-size-sm);
            font-weight: var(--font-weight-bold);
            text-transform: uppercase;
            letter-spacing: 0.05em;
            transition: all var(--transition-base);
            backdrop-filter: blur(10px);
        }

        .back-link:hover {
            color: var(--color-primary-cyan);
            border-color: var(--color-primary-cyan);
            box-shadow: 0 0 15px rgba(0, 255, 255, 0.3);
            transform: translateY(-2px);
        }

        @keyframes dataGlow {
            0%, 100% { box-shadow: 0 0 20px rgba(255, 0, 128, 0.2); }
            50% { box-shadow: 0 0 40px rgba(0, 255, 255, 0.2); }
        }

        @keyframes borderPulse {
            0%, 100% { opacity: 0.4; }
            50% { opacity: 0.7; }
        }

        @keyframes titleGlow {
            0%, 100% { text-shadow: 0 0 20px var(--color-primary-cyan); }
            50% { text-shadow: 0 0 30px var(--color-primary-cyan), 0 0 40px var(--color-primary-pink); }
        }

        @keyframes scanLine {
            0% { left: -100%; }
            100% { left: 100%; }
        }
    </style>
</head>
<body>
    <div class="data-container">
        <div class="data-card">
            <h1 class="data-title">Session Data</h1>
            <div class="data-content">""")
	for line in sessionFile:
		logger(f"line = {line}")
		formatted_line = unquote(line.strip()).replace('&', '</div><div class="data-field">')
		print(f'<div class="data-field">{formatted_line}</div>', end='')
	print("""
            </div>
            <a href="/session" class="back-link">← Create New Session</a>
        </div>
    </div>
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

