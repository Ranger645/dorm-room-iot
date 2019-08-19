import os


class ConfigManager:

    def __init__(self, file_name="../res/config.cfg"):
        self.file_name = file_name

        # Reading lines from config file:
        config_lines = []
        if os.path.isfile(self.get_full_config_file_path()):
            with open(self.get_full_config_file_path(), "r") as config_file:
                config_lines = config_file.readlines()

        # Parsing config lines:
        self._config = {}
        for line in config_lines:
            line_data = line[:-1].split("=")
            self._config[line_data[0]] = line_data[1]
    
    def save_config(self):
        with open(self.get_full_config_file_path(), "w+") as config_file:
            for key in self._config:
                config_file.write(str(key) + "=" + str(self._config[key]))

    def __contains__(self, key):
        return key in self._config

    def __getitem__(self, key):
        return self._config[key]

    def __setitem__(self, key, value):
        self._config[key] = value
        self.save_config()

    def get_full_config_file_path(self):
        return os.path.join(os.path.dirname(__file__), self.file_name)
