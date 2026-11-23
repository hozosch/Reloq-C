# ReLoq Apple: installation for iPhone, iPad and Mac

[Deutsch](APPLE.de.md)

[ReLoq Apple](https://github.com/Quinton1110/ReLoq-Apple) is a separately maintained app that makes ReLoq C voices available to VoiceOver and Spoken Content. Its installer and updates come from the ReLoq Apple project, not from the ReLoq C 2.0 release assets.

The app requires iOS or iPadOS 18 or later, or macOS 15 or later on an Apple-silicon Mac. The available Mac build does not support Intel Macs. Downloads and the current version are listed on the [ReLoq Apple releases page](https://github.com/Quinton1110/ReLoq-Apple/releases/latest); the project also has a [download page](https://quintonwilliams.me/reloq-apple/).

**Version distinction:** On 23 November 2026, the current ReLoq Apple release is 0.1.5. Its bundled engine is based on ReLoq C 1.4.1 with the Apple fixes, not ReLoq C 2.0. In particular, it does not yet offer Aragonese. Installing ReLoq C 2.0 on another platform does not update the engine inside the Apple app. Check the Apple project's release notes for later changes.

## iPhone and iPad: AltStore Classic

An IPA is not installed simply by opening it in Files. The following method uses AltStore Classic, which needs a Mac or Windows PC for setup and an Apple account.

1. Install AltServer on the computer and use it to install AltStore Classic on the device. Follow the official [Mac](https://faq.altstore.io/altstore-classic/how-to-install-altstore-macos) or [Windows](https://faq.altstore.io/altstore-classic/how-to-install-altstore-windows) guide, including the device pairing and trust prompts.
2. If requested, enable **Developer Mode** under **Settings → Privacy & Security → Developer Mode**, restart the device and confirm the prompt. Trust the developer profile under **Settings → General → VPN & Device Management** if prompted.
3. Open AltStore, go to **Sources**, choose the add button and enter this source address:

   `https://quintonwilliams.me/reloq-apple/source.json`

4. Open the ReLoq Apple source and install ReLoq Apple. If asked whether to keep app extensions, keep them: the speech provider is an extension.
5. Open ReLoq Apple once. In **Settings → Accessibility → VoiceOver → Speech → Voice**, choose a voice ending in **(ReLoq)**. The same voices can also be chosen for Spoken Content.

With a free Apple account, the signing expires after seven days unless refreshed. AltStore can refresh apps while the computer runs AltServer and the device is on the same Wi-Fi network, or connected by USB. Check **My Apps** and use **Refresh All** when needed; background refresh is not guaranteed. An expired app may make its voices unavailable until it is refreshed. AltStore also has limits on active apps and app identifiers; see its [refresh and account guidance](https://faq.altstore.io/altstore-classic/your-altstore).

The Apple project also documents installation through [SideStore](https://sidestore.io/) and [Sideloadly](https://sideloadly.io/) in its [README](https://github.com/Quinton1110/ReLoq-Apple#just-want-to-use-it). Its IPA can be downloaded from the releases page for those methods.

## Mac

1. On an Apple-silicon Mac, download the `-mac.zip` file from the [ReLoq Apple releases page](https://github.com/Quinton1110/ReLoq-Apple/releases/latest).
2. Unzip it, move **ReLoq Apple** to **Applications**, and open it once. Keep the app in Applications: its speech extension provides the voices.
3. Open **VoiceOver Utility → Speech → Voices**, select the primary voice, and choose a voice ending in **(ReLoq)**. With VoiceOver on, **VO+Fn+F8** opens VoiceOver Utility; VO normally means Control and Option together.

Voice settings and the optional pronunciation dictionary are managed in ReLoq Apple. According to the app's documentation, changing the sample rate takes effect in VoiceOver after VoiceOver is restarted. For updates and further troubleshooting, see the [ReLoq Apple README](https://github.com/Quinton1110/ReLoq-Apple#readme).
