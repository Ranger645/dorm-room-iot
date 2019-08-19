import os
import socket
import threading

os.chdir(".")
from config_manager import ConfigManager


CONFIG_FILE = "../res/config.cfg"
PORT = 10102


class Daemon(threading.Thread):
    
    def __init__(self, *args, **kwargs):
        self.config_manager = None
        self.daemon_running = False

        return super().__init__(*args, **kwargs)

    def set_server_socket(self, server_socket):
        self.server_socket = server_socket

    def run(self):
        self.config_manager = ConfigManager(CONFIG_FILE)
        self.daemon_running = True

        while self.daemon_running:
            command = self.wait_for_command()
            self.daemon_running = self.handle_command(command)

    def wait_for_command(self):
        try:
            # Waiting for client to connect and request a command:
            client, client_address = self.server_socket.accept()
            print("Client " + str(client_address) + " connected")
            # Waiting for client to send command:
            data = client.recv(2048)
            print("Received command " + str(data))
            return data
        except ConnectionAbortedError:
            return None

    def handle_command(self, command):
        if command is None or command.startswith(b'stop'):
            return False
        return True


def main():
    # Setting up socket for receiving commands:
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((socket.gethostbyname('localhost'), PORT))
    server_socket.listen(5)

    daemon = Daemon()
    daemon.set_server_socket(server_socket)
    daemon.start()

    try:
        while True:
            pass
    except KeyboardInterrupt:
        pass
    
    server_socket.shutdown(socket.SHUT_RD)
    server_socket.close()
    print("Server closed normally.")


if __name__ == "__main__":
    main()
