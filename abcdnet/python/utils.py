import json
from types import SimpleNamespace

class VBSConfig(SimpleNamespace):
    @classmethod
    def from_json(cls, config_json, extra={}):
        with open(config_json, "r") as f:
            d = json.load(f)
            d["name"] = config_json.split("/")[-1].replace(".json", "")
            d.update(extra)
            return json.loads(json.dumps(d), object_hook=lambda d: cls(**d))

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

class VBSOutput:
    def __init__(self, file_name):
        self.file_name = file_name
    def write(self):
        raise NotImplementedError()
    def close(self):
        raise NotImplementedError()

def print_title(text):
    text = f" {text} "
    print(f"{text:-^50}", flush=True)
