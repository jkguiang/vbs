import json
from types import SimpleNamespace

class VBSConfig(SimpleNamespace):
    @classmethod
    def from_json(cls, config_json):
        with open(config_json, "r") as f:
            return json.load(f, object_hook=lambda d: cls(**d))

    def get(self, key, default=None):
        return self.__dict__.get(key, default)

    def keys(self):
        return self.__dict__.keys()

    def items(self):
        return self.__dict__.items()

    def __get_json(self, d):
        json = {}
        for key, val in d.items():
            if type(val) == type(self):
                json[key] = self.__get_json(val)
            else:
                json[key] = val
        return json

    def __str__(self):
        return json.dumps(self.__get_json(self.__dict__), indent=4)

    def __getitem__(self, key):
        return self.get(key)

def print_title(text):
    text = f" {text} "
    print(f"{text:-^50}", flush=True)
