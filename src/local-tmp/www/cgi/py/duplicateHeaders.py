
CRLF = "\r\n"

print("Content-Type: text/html", end=CRLF)
print("Content-Length: 0", end=CRLF)
print("Set-Cookie: cookie1", end=CRLF)
print("Set-Cookie: cookie2", end=CRLF)
print("Set-Cookie: cookie3", end=CRLF)
print("Http-Header1: value1", end=CRLF)
print("Http-Header1: value2", end=CRLF)
print("Http-Header1: value3", end=CRLF)
print("Http-Header2: value1", end=CRLF)
print("Http-Header2: value2", end=CRLF)

print(CRLF, end='')
