from sys import stdin, stderr

buff = ""

while 1:
	line = stdin.readline()
	if not line:
		break
	buff += line

if not buff:
	exit(1)

print("Content-Type: text/html", end="\r\n\r\n")

print(buff)
