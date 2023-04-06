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

    def __getitem__(self, key):
        return self.get(key)

def print_title(text):
    text = f" {text} "
    print(f"{text:-^50}")
