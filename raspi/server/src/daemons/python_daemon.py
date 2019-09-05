import sys


class PythonDaemon:
    
    def __init__(self, handle_command, on_init=None, on_terminate=None):
        self._handle_command = handle_command
        self._on_init = on_init
        self._on_terminate = on_terminate

    def loop(self):
        state = None
        if self._on_init is not None:
            state = self._on_init()
        try:
            while True:
                client, command = self.read_data_from_server()
                if not self._handle_command(self, client, command, state):
                    break
        except EOFError as e:
            pass
        finally:
            self._on_terminate(state)

    def send_to_server(self, message):
        print(str(message))
        sys.stdout.flush()

    def send_to_client(self, client_id, message):
        # Command for sending data to a client
        self.send_to_server("send_to_client " + str(client_id) + " " + str(message))

    def read_data_from_server(self):
        # Waits for and reads data from the server value should be given by the server in the form:
        #   <client-id> <message>
        # Returns client-id, message
        value = input("")
        parsed_data = value.split(" ")
        return parsed_data[0], " ".join(parsed_data[1:])
