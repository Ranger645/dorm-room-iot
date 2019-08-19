
import sys


try:
	while True:
		incoming = bytearray(input().encode())
		for b in incoming:
			print("%x" % b, end=" ", file=sys.stderr)
		print(file=sys.stderr)
except KeyboardInterrupt:
	print("Successful Keyboard interrupt")

