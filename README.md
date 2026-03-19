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

1. It fetches releases from [Magisk](https://github.com/topjohnwu/magisk), [PlayIntegrityFix](https://github.com/KOWX712/PlayIntegrityFix) and websites like those of [LineageOS](https://wiki.lineageos.org/devices) and [OrangeFox](https://orangefox.download). In reality, any website you'd like ! You can chose your tools.
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

I'm just providing a front-end to these incredible tools, and a package manager for them. Without them this project would be nothing.

Huge shoutouts and warm thanks to them !!

## ⚠️ Disclaimer

This software is released under the GNU GPLv3.

If you derive or redistribute UniFlashTool, you must:
 - Keep the GPL license.
 - Credit the original author (your name or GitHub).
 - Publish the source of your derived version.
 
Flashing can **brick your device** 🧱. Backup everything. Test on throwaways first. No liability—use at your risk. GPL-3.0 licensed.

## Quick Start
0. Save your data on an external drive, because everything on your phone will be deleted
1. Download binaries (Win/Mac/Linux) from Releases (if available, or build from source).
2. Connect phone using USB (USB-A works better).
3. Enter codename (e.g., "beryllium" for Poco F1) or let UniFlashTool detect it for you.
4. Hit Repository settings if you don't want to use the default tools, and set up your own.
5. Flash em' all these images and programs.
6. Check if everything still works (if not, flash again but DON'T DISCONNECT YOUR PHONE)
7. Enjoy your new custom OS

## Supported
- Android bootloader/fastboot/OrangeFox devices.
- Popular codenames via Lineage/OrangeFox lookup.
- Codename auto-detection
- Custom tool addition via local repository, sort em' by device codename and tools versions
- Pick your tools from any website that host archives, or get them directly via GitHub with UFT GitHub utility integration

### Host operating systems

Windows, MacOS and Linux are all supported as hosts for this project.

It was intended to be cross-platform, and leave behind the need to use tools such as WSL (Windows Subsystem for Linux) or emulators to do such a simple task as download a program and flash it on a partition.


## Build It
```bash
git clone --recursive
./build.sh
```

Yeah that wasn't hard gng sorry if this was obvious 🥀

## Footnote

This project was initially started on March, 12 2026, and will receive future updates (it's not finished yet !)

Anyway, thanks for using this tool if you do, or to talk about it with your people. It's really important we finally understand the importance of getting more control over our data, and our devices in general. I'm trying to make this all more simple. If you wish to contribute, check [this file](CONTRIBUTING.md) when it'll come out and learn about my coding practices in [this one](STYLE.md).

Together we can do something big !

Stay tuned fam 🗿
