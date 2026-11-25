# ReLoq C on Linux

ReLoq C runs on 64-bit Intel and AMD Linux systems in two ways. ARM Linux and BT Speak Pro are outside the currently tested targets.

- **Speech Dispatcher module** `reloq`, for Orca and every other program that speaks through Speech Dispatcher. It comes in `reloq-c-linux-x64-<version>.tar.gz`.
- **Linda add-on** `reloq-c-synth-linux-x64-<version>.nvda-addon`, for [Linda](https://github.com/Mudb0y/linda), the NVDA port to Linux. It has the same voices, settings, dictionaries and update checks as the NVDA add-on on Windows.

Both contain the same C engine as the Windows and Android packages, with all fifteen language variants and the eight voice presets.

## Requirements

- A 64-bit Intel or AMD processor.
- The GNU C library 2.35 or later, as in Ubuntu 22.04, Debian 12, Fedora 36 or newer.
- For the module: Speech Dispatcher 0.11 or later.
- For the add-on: Linda.

## Speech Dispatcher module

Unpack the archive and run the installer from its directory:

```sh
tar xf reloq-c-linux-x64-<version>.tar.gz
cd reloq-c-linux-x64-<version>
sudo ./install.sh --system    # for every user
./install.sh --user           # or for yourself only, without root
```

The installer does not restart Speech Dispatcher, because a restart briefly stops the speech you may be using. Restart it when you are ready, for example with `killall speech-dispatcher`; it starts again the next time a program speaks. Then choose **reloq** as the speech synthesizer in Orca's voice settings, or try it:

```sh
spd-say -o reloq "Hello from ReLoq C"
spd-say -o reloq -l de -y "Shelly (German)" "Guten Tag"
```

### Voices

Each language offers the eight presets, named `<Preset> (<Language>)`, for example `Reed (American English)` or `Grandma (Greek)`. `spd-say -o reloq -L` lists them. A screen reader that selects a voice type rather than a name gets Reed for male 1, Rocko for male 2, Glen for male 3, Shelly for female 1, Sandy for female 2, Grandma for female 3 and Bobby for a child voice.

Speech Dispatcher's rate, pitch, pitch range and volume work relative to the preset. Rate 0 is the engine's normal speed; the top of the range reaches the NVDA add-on's rate boost.

### Settings

Settings that Speech Dispatcher does not send, such as the sample rate, pause shortening, mellow mode, question intonation and pronunciation dictionaries, are in `reloq.conf`: `/etc/speech-dispatcher/modules/reloq.conf` for a system installation or `~/.config/speech-dispatcher/modules/reloq.conf` for a user installation. Each setting is explained there. An update keeps your edited file.

The community pronunciation dictionaries for German and American English are installed with the module. To use them, remove the `#` before the `DictionaryDirectory` line in `reloq.conf` and restart Speech Dispatcher.

### How the module is registered

Speech Dispatcher finds a module in its own module directory by itself, as long as `speechd.conf` lists no module explicitly. The first `AddModule` line there turns that discovery off for every module.

`reloq-speechd-enable`, which the installer runs, takes care of this:

- After a system installation it changes nothing when discovery is in use, and adds `reloq` next to any explicitly listed modules.
- A user installation is outside the system directory, so it has to be listed. The script lists it in your `~/.config/speech-dispatcher/speechd.conf` together with every module discovery would have found, so your other voices stay available.

Its lines are enclosed in `# BEGIN ReLoq C` and `# END ReLoq C`, and the first change keeps a backup named `speechd.conf.reloq.bak`.

### Removing

```sh
sudo ./install.sh --system --uninstall
./install.sh --user --uninstall
```

This removes the installed files and the lines `reloq-speechd-enable` added, and keeps `reloq.conf`.

## Linda add-on

Install `reloq-c-synth-linux-x64-<version>.nvda-addon` as you would any NVDA add-on: open it from Linda's add-on store with **Install from external source**, restart Linda, and select **Eloquence C** with **Linda+Control+S**. The [English user guide](../docs/USER-GUIDE.en.md) applies unchanged.

Use the Linux package with Linda: the Windows package carries `eci.dll`, which Linda cannot load. The add-on's update check only offers Linux packages to a Linux installation, and Windows packages to Windows.

## Building from source

You need GCC, GNU Make, Python 3 and, for the module, Speech Dispatcher's development files (`libspeechd-dev` on Debian and Ubuntu, `speech-dispatcher-devel` on Fedora). From the repository root:

```sh
make -C eci -j4 libreloq_eci.so sd_reloq
make -C eci speechd-test                       # the module's protocol test
python3 addon/build_synth.py --platform linux  # the Linda add-on, in dist/
python3 linux/package.py                       # the module archive, in dist/
```

`make -C eci speechd-install` installs the module for every user, and `make -C eci speechd-install-user` for yourself. `make -C eci install-lib` installs the library and its header under `PREFIX`, `/usr/local` by default.

A library built on a recent distribution needs that distribution's C library. Release packages are built on Ubuntu 22.04 so that they also run on older systems.
