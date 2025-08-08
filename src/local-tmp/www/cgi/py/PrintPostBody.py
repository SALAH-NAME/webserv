from sys import stdin

print("Content-type: text/plain", end="\r\n\r\n")

while 1:
	line = stdin.readline()
	if not line:
		break
	print(line, end='')
