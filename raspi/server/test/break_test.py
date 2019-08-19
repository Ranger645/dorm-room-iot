import socket

HOST = 'localhost'  # The server's hostname or IP address
PORT = 10101        # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
	s.connect((HOST, PORT))
	s.sendall(b'execute python3 test_python.py\n')
	s.sendall(chr(3).encode())

