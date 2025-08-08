from os import getcwd

#header
CRLF="\r\n"

current_cwd = getcwd()
msg = "Cgi current working dir:\n\t"

print("Content-Type: text/txt", end=CRLF)
print(f"Content-Length: {len(current_cwd) + len(msg) + 2}", end=CRLF)
print("Server: Ed, Edd n Eddy",end=CRLF)
print("Status: 200", end=CRLF)
print(CRLF, end="")

#body

print(msg, current_cwd)
