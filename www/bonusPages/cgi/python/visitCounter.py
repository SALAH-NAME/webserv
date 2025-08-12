from os import environ
from datetime import datetime

response_header = "Content-Type: text/html\r\n"
timestamp = ""
counter = 0

def InitializeCookies():
	global response_header
	global timestamp
	global counter

	if counter < 0:
		counter = 0
	now = datetime.now()
	timestamp = now.strftime("%Y-%m-%d %H:%M:%S")
	response_header += f"Set-Cookie: visit-counter={counter + 1}\r\n"
	response_header += f"Set-Cookie: last-visit={timestamp}\r\n\r\n"
	return

def SetVisiCount():
	global counter
	Cookies = ""

	if "HTTP_cookie" not in environ:
		return 0
	else:
		Cookies = environ["HTTP_cookie"]
	pairs = Cookies.split(';')
	for pair in pairs:
		key_value = pair.split('=')
		if len(key_value) != 2:
			continue
		key = key_value[0]
		value = key_value[1]
		if key == "visit-counter":
			counter = int(value)

SetVisiCount()
InitializeCookies()
print(response_header)
print(
f"""
<html>
	<head>
		<title>Cookies Demonstration</title>
		<style>
			body {{
				display: flex;
				justify-content: center;
				align-items: center;
				height: 100vh;
				background-color: #f0f0f0;
			}}
			.container {{
				text-align: center;
				background: #fff;
				padding: 40px 60px;
				border-radius: 12px;
				box-shadow: 0 2px 8px rgba(0,0,0,0.1);
			}}
			.counter {{
				font-size: 2.5em;
				font-weight: bold;
			}}
			.last-visit {{
				font-size: 1.3em;
				color: #333;
				margin-top: 20px;
			}}
		</style>
	</head>
	<body>
		<div class="container">
			<div class="counter">
				You have visited this page {counter + 1} times!
			</div>
			<div class="last-visit">
				The last time you visited: {timestamp}
			</div>
		</div>
	</body>
</html>
"""
)