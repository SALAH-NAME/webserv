import os

env = str(os.environ)
tab = env.split(',')
for elm in tab:
	print(elm)