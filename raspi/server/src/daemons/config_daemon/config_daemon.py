import os, sys

sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
from configuration import Configuration
from python_daemon import PythonDaemon


def init_config():
    return Configuration()


def handle_config_command(daemon_connection, client, command, config):
    parsed_command = command.split(" ")
    if parsed_command[0] == "exit":
        return False
    elif parsed_command[0] == "closed":
        config.remove_subscriber_from_all_keys(client)
    elif parsed_command[0] == "add_subscriber":
        # add_subscriber <key>
        config.add_subscriber(parsed_command[1], client)
        daemon_connection.send_to_client(client, config.get_value(parsed_command[1]))
    elif parsed_command[0] == "set_value":
        # set_value <key> <value>
        config.set_value(parsed_command[1], " ".join(parsed_command[2:]))
        for sub in config.get_subscribers(parsed_command[1]):
            daemon_connection.send_to_client(sub, config.get_value(parsed_command[1]))
    elif parsed_command[0] == "get_value":
        # get_value <key>
        daemon_connection.send_to_client(client, config.get_value(parsed_command[1]))
    return True


def terminate_config(config):
    config.save_to_persistent_storage()


def main():
    daemon = PythonDaemon(handle_config_command, on_init=init_config, on_terminate=terminate_config)
    daemon.loop()


if __name__ == "__main__":
    main()
