import os
#this script is used to test the passed environment
env = str(os.environ)
flag = False
PrevChar = 'c'
for c in str(env):
	if PrevChar == '{':
		flag = True
	elif c == '}':
		flag = False
	if flag and c == ',':
		print()
	elif flag:
			print(c, end='')
	PrevChar = c