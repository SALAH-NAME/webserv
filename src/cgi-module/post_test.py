#script used to test the data sent via pipe to the cgi (used in the body of a post)

from sys import stdin, stdout

print("==GET request body==")
stdout.flush()
for line in stdin:
    print(line, end='')