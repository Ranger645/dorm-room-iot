import os


class Configuration:

    def __init__(self, persistent_storage_path=None):
        if persistent_storage_path is None:
            persistent_storage_path = os.path.join(os.path.dirname(__file__), "../../../config/configuration.cfg")
        self._persistent_storage_path = persistent_storage_path
        self._config = {}
        self._subscribers = {}
        self.load_from_persistent_storage()

    def load_from_persistent_storage(self):
        if not os.path.exists(self._persistent_storage_path) or not os.path.isfile(self._persistent_storage_path):
            return
        config_lines = []
        with open(self._persistent_storage_path, "r") as file:
            config_lines = file.readlines()
        for line in config_lines:
            if len(line) > 0 and '=' in line:
                parsed_line = line[:-1].split("=")
                if len(parsed_line) == 2:
                    self._config[parsed_line[0]] = parsed_line[1]
                else:
                    print("[WARNING]: read config line inconsistent with format: " + str(line))

    def save_to_persistent_storage(self):
        lines = []
        for key in self._config.keys():
            lines.append("=".join([key, str(self._config[key])]))
        with open(self._persistent_storage_path, "w+") as file:
            file.writelines([line + "\n" for line in lines])

    def set_value(self, key, value):
        self._config[str(key)] = str(value)

    def get_value(self, key):
        return self._config[key]

    def add_subscriber(self, key, subscriber):
        if key in self._subscribers.keys():
            self._subscribers[key].append(subscriber)
        else:
            self._subscribers[key] = [subscriber]
        if key not in self._config:
            self._config[key] = None

    def get_subscribers(self, key):
        if key in self._subscribers.keys():
            return self._subscribers[key]
        else:
            return []

    def remove_subscriber(self, key, subscriber):
        if key in self._subscribers.keys() and subscriber in self._subscribers[key]:
            self._subscribers[key].remove(subscriber)

    def remove_subscriber_from_all_keys(self, subscriber):
        for key in self._subscribers.keys():
            self.remove_subscriber(key, subscriber)

    def __str__(self):
        return str(self._config)
