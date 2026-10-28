"""Load real NVDA property/settings/GUI contracts without Windows or wx.

Only external services, profile storage, validation and visual controls are stubs.
The tested NVDA methods and property metaclass are unmodified upstream code.
The pinned sources and their license are in testdata/nvda (never in the add-on).
"""

import ast
from collections import OrderedDict
from abc import abstractmethod
import importlib.util
import itertools
from pathlib import Path
import re
import sys
import types

SOURCE = Path(__file__).with_name("testdata") / "nvda"


class Validator:
    """External schema service for the scalar specs used by these drivers."""

    def get_default_value(self, spec):
        value = re.search(r"default=([^,)]+)", spec)[1]
        if value == "None":
            return None
        return self.check(spec, value)

    def check(self, spec, value):
        if value is None:
            return None
        if spec.startswith("boolean"):
            return value is True or str(value) == "True"
        if spec.startswith("integer"):
            return int(value)
        return str(value)


def configuration_manager(*profiles):
    """Execute upstream AggregatedSection unchanged over in-memory profiles."""
    source = SOURCE / "config/__init__.py"
    tree = ast.parse(source.read_text())
    section = next(node for node in tree.body if isinstance(node, ast.ClassDef)
                   and node.name == "AggregatedSection")
    namespace = dict(itertools=itertools, BACK_COMPAT_TO=(2027, 1, 0))
    future = ast.ImportFrom(module="__future__",
                            names=[ast.alias(name="annotations")], level=0)
    module = ast.fix_missing_locations(ast.Module(body=[future, section], type_ignores=[]))
    exec(compile(module, str(source), "exec"), namespace)

    class Manager:
        def __init__(self):
            self.validator = Validator()
            self.writes = 0
            self.profiles = list(profiles)
            self.rootSection = namespace["AggregatedSection"](self, (), {}, self.profiles)

        def _markWriteProfileDirty(self):
            self.writes += 1

    return Manager()


def load(name, relative):
    spec = importlib.util.spec_from_file_location(name, SOURCE / relative)
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


def install():
    config = sys.modules["config"]
    manager = configuration_manager(dict(config.conf))
    config.conf = manager.rootSection
    config._testManager = manager
    config.conf["speech"] = {}
    config.confspec = {"speech": {"__many__": {}}}
    config.pre_configSave = types.SimpleNamespace(register=lambda f: None, unregister=lambda f: None)
    log = sys.modules["logHandler"].log
    log.debug = log.debugWarning = lambda *a, **kw: None
    sys.modules["garbageHandler"] = types.SimpleNamespace(TrackedObject=type("TrackedObject", (), {}))
    load("baseObject", "baseObject.py")
    settings = load("autoSettingsUtils.driverSetting", "autoSettingsUtils/driverSetting.py")
    utils = load("autoSettingsUtils.utils", "autoSettingsUtils/utils.py")
    auto = load("autoSettingsUtils.autoSettings", "autoSettingsUtils/autoSettings.py")

    class Driver(auto.AutoSettings):
        @classmethod
        def getId(cls):
            return cls.name

        @classmethod
        def getDisplayName(cls):
            return cls.description

        @classmethod
        def _getConfigSection(cls):
            return cls._configSection

    module = sys.modules["synthDriverHandler"]
    vars(module).update(vars(settings))
    module.__name__ = "synthDriverHandler"
    vars(module).update(OrderedDict=OrderedDict, StringParameterInfo=utils.StringParameterInfo,
        abstractmethod=abstractmethod, driverHandler=types.SimpleNamespace(Driver=Driver),
        config=config, log=log, pgettext=lambda context, text: text,
        changeVoice=lambda synth, voice: setattr(synth, "voice", voice) if voice else None,
        languageHandler=types.SimpleNamespace(normalizeLanguage=lambda s: s.replace("-", "_"),
            getLanguageDescription=lambda s: "NVDA " + s))
    tree = ast.parse((SOURCE / "synthDriverHandler.py").read_text())
    classes = [node for node in tree.body if isinstance(node, ast.ClassDef)
               and node.name in ("LanguageInfo", "VoiceInfo", "SynthDriver")]
    exec(compile(ast.Module(body=classes, type_ignores=[]), str(SOURCE / "synthDriverHandler.py"), "exec"), vars(module))

    # Execute the actual GUI lookup and selection logic, with wx drawing stubbed.
    tree = ast.parse((SOURCE / "gui/settingsDialogs.py").read_text())
    method = next(node for node in ast.walk(tree) if isinstance(node, ast.FunctionDef)
                  and node.name == "_makeStringSettingControl")

    class Choice:
        def __init__(self, choices):
            self.choices = choices
            self.selection = None

        def SetSelection(self, value):
            self.selection = value

        def Bind(self, *args):
            pass

        def MoveAfterInTabOrder(self, *args):
            pass

    def labeled_control(parent, label, cls, *, choices):
        control = Choice(choices)
        return types.SimpleNamespace(control=control, sizer=control)

    namespace = dict(DriverSetting=settings.DriverSetting, Any=object,
                     guiHelper=types.SimpleNamespace(LabeledControlHelper=labeled_control),
                     wx=types.SimpleNamespace(Choice=Choice, EVT_CHOICE=object()),
                     StringDriverSettingChanger=lambda *args: None)
    exec(compile(ast.Module(body=[method], type_ignores=[]), str(SOURCE / "gui/settingsDialogs.py"), "exec"), namespace)

    class Panel:
        lastControl = None

        def __init__(self, synth):
            self.synth = synth

        def getSettings(self):
            return self.synth

        def bindHelpEvent(self, *args):
            pass

        def _getSettingControlHelpId(self, name):
            return name

    Panel.make = namespace[method.name]
    return settings, Panel
