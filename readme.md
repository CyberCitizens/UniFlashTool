# Universal Flashing Tool

Universal GUI used to flash any phone with a custom ROM.

## Why ?

First of all; because I can.

Second of all; doing a video about flashing phones, I came accross the fact that flashing phones may not be a hard thing to do in the end; still, typing commands in a terminal can rebuke most of genuine users looking for more privacy and control over their phones.

As a tenet, I wanted to keep things transparent and secure. That's why this project is open source.

> [!NOTE]
> The name "UniFlashTool" is inspired by the generic idea of a universal flashing utility.
> Similar concepts have been explored in other projects (e.g., UniFlash for PC BIOS flashing),
> but this implementation is written from scratch in C++/Qt for Android.

## How does it work ?

1. It fetches releases from [Magisk](https://github.com/topjohnwu/magisk), [PlayIntegrityFix](https://github.com/KOWX712/PlayIntegrityFix) and websites like those of [LineageOS](https://wiki.lineageos.org/devices) and [OrangeFox](https://orangefox.download).
2. It downloads latest releases for your model (if available) using its codename with your consent.
3. It asks you how to modify your device:
	- Do we root it ?
	- Do we install PlayIntegrityFix along with Magisk, if rooted ?
	- Which custom ROM to install ?
4. It reboots your phone on its bootloader (fastboot) and use it to flash important partitions, then reboot on **OrangeFox**.
5. It guides you step-by-step telling you where to click, and flash / loads content according to the settings you gave and ordered.
6. It reboots your phone.
7. Asks you if you want to clean up downloaded files, and congrats you for respecting your data !

## Shoutouts

My work is **CLEARLY NOT** the most amazing one in there. This project is fully dependant on;

- [Magisk](https://github.com/topjohnwu/magisk), the good ol' rooting software
- [OrangeFox](https://orangefox.download), the recovery image that makes everything possible
- [PlayIntegrityFix](https://github.com/KOWX712/PlayIntegrityFix), transforms a rooted-sovereign brick into a recognized and authentic Device to use with (almost) any app
- [LineageOS](https://wiki.lineageos.org/devices), my custom ROM of choice because of its outstanding range of supported devices

I'm just providing a front-end to these incredible tools, without them this project would be nothing.

Huge shoutouts and warm thanks to them !!

## ⚠️ Disclaimer

This software is released under the GNU GPLv3.

If you derive or redistribute UniFlash, you must:
 - Keep the GPL license.
 - Credit the original author (your name or GitHub).
 - Publish the source of your derived version.
 
Flashing can **brick your device** 🧱. Backup everything. Test on throwaways first. No liability—use at your risk. GPL-3.0 licensed.

## Quick Start
1. Download binaries (Win/Mac/Linux) from Releases.
2. Connect phone in fastboot: `app.exe detect` (or GUI button).
3. Enter codename (e.g., "beryllium" for Poco F1).
4. Hit "Collect Tools" → checkboxes → "Flash Everything".
5. Follow on-screen steps.

## Supported
- Android bootloader/fastboot/OrangeFox devices.
- Popular codenames via Lineage/OrangeFox lookup.
- Bundles adb/fastboot—no install needed.

## Build It
```bash
git clone --recursive
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Final Thoughts
This README will convert lurkers to testers—add screenshots/GIF of the GUI flow post-MVP. Link XDA thread for device requests. Ship it as-is for alpha; it'll grow with PRs. You're onto something big! 🚀

