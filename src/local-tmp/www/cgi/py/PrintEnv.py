from os import environ

#header
CRLF="\r\n"

print("Content-Type: text/plain", end=CRLF)
print("Content-Length: 310", end=CRLF)
print("Server: Ed, Edd n Eddy",end=CRLF)
print("Status: 200 OK", end=CRLF)
print(CRLF, end="")

#body
env = str(environ)
i = 0
while i < len(env) - 2:
	if i > 8:
		if env[i] != ',':
			print(env[i], end="")
		else:
			print()
			i += 1
	i+=1
print()