// Copyright (C) 2026 Mathéo Allart <imacybercitizen@gmail.com>
// SPDX‑License‑Identifier: GPLv3.0

#ifndef UFT_FLASH
#define UFT_FLASH

#include "../platform/deps.hpp"
#include "tools.h"

#include <filesystem>

// This file provides a useful range of functions to easily flash a phone entierely.
// Basically ADB and FastBoot tools.

namespace uft::Tools::Flash
{
	enum PARTITION
	{
		// Doesn't insert any additional argument, and does it "normal mode"
		SYSTEM,
		// Data Tree Blob for Overlay; initiates hardware communication with coming software.
		DTBO,
		// Prepares the environment to launch other softwares, is the first software layer.
		BOOT,
		// Same as BOOT, but may be called this on older devices.
		VENDOR_BOOT,
		// Android recuperation partition.
		RECOVERY,
		// Fastbootd user-space
		FASTBOOT,
		// Bootloader
		BOOTLOADER,
	};

	::std::map<PARTITION, ::std::string const> const PARTITIONS
	{
		{ DTBO,			"dtbo"			},
		{ BOOT,			"boot"			},
		{ VENDOR_BOOT,	"vendor_boot"	},
		{ RECOVERY,		"recovery"		},
		{ FASTBOOT,		"fastboot"		},
		{ BOOTLOADER,	"bootloader"	},
	};

	enum DEVICE_STATE
	{
		STATE_NOT_CONNECTED,	// No device are connected.
		STATE_DEVICE,			// The device is being used normally, booted in the system.
		STATE_RECOVERY,			// The system is in recovery mode.
		STATE_SIDELOAD,			// The system is accepting sideload traffic.
		STATE_FASTBOOT,			// The device is in Fastboot / Fastbootd mode.
		STATE_UNKNOWN,			// The system is connected, but its state cannot be mapped to a known state.
	};

	::std::map<DEVICE_STATE, ::std::string const> const DEVICE_STATES
	{
		{ STATE_NOT_CONNECTED, 	"not connected"	},
		{ STATE_DEVICE, 		"device"		},
		{ STATE_RECOVERY, 		"recovery"		},
		{ STATE_SIDELOAD, 		"sideload"		},
		{ STATE_UNKNOWN,		"unknown"		},
	};

	void WaitForState(DEVICE_STATE state); // Blocks this thread (without burning the CPU) until the device reaches a certain state.

	bool HasDevice(); // ADB check to know if there is a device connected.
	// Gets the name of the currently connected device
	::std::string const GetConnectedDeviceCodename();
	// Gets the state of the currently connected device.
	DEVICE_STATE const GetConnectedDeviceState();
	// Blocking operation waiting for ADB to have an open connection to talk with an adbd instance on a remote device for a Sideload operation.
	void WaitForSideload();
	// Simply performs a reboot, rebooting to normal system.
	::std::string const Reboot();
	// Reboots a device to its bootloader, enabling fastboot commands.
	::std::string const RebootToFastBoot();
	// Ensures ADB is running, starts a new server if no server is connected, and returns the result output.
	void EnsureADB();
	// Loads a .zip into a phone.
	Platform::ProcessResult const Sideload(::std::string const& filePath, on_write_function onWrite = nullptr);
	// Installs an app.
	Platform::ProcessResult const Install(::std::string const& appPath, on_write_function onWrite = nullptr);
	// Pushes data on the phone on a given directory.
	Platform::ProcessResult const Push(::std::string const& source, ::std::string const& destination, on_write_function onWrite = nullptr);
	// Enters a command in the ADB shell as the device's user.
	Platform::ProcessResult const Shell(::std::string const& command, on_write_function onWrite = nullptr);
	// Alias to fasten the de-googlisation process
	// Runs wipe-frp to remove any remaining trace of Google on the device,
	// preventing app installation and password setting.
	inline Platform::ProcessResult const UnGoogle(on_write_function onWrite = nullptr)
	{
		WaitForState(STATE_RECOVERY);
		return Shell("wipe-frp", onWrite);
	}
	// Looks for a program in the device, returns its path if it's present
	Platform::ProcessResult const Which(::std::string const& program);
	namespace FastBoot
	{
		void WaitForFastBoot(); // Waits until the device is in fastboot mode.
		// Returns true if a device is connected with fastboot, false if not.
		bool HasDevice();
		// Wipe data on every partition of this device.
		Platform::ProcessResult const Format(on_write_function onWrite = nullptr);
		// Flashes a file into a given partition, if a device is connected
		Platform::ProcessResult const Flash(PARTITION const partition, ::std::string const& filename, on_write_function onWrite = nullptr);
		// Reboots into a known partition
		// (leave empty for standard reboot)
		Platform::ProcessResult const Reboot(PARTITION const partition = SYSTEM, on_write_function onWrite = nullptr);
		// Boots on a file without flashing anything (memory boot, doesn't affect storage data)
		Platform::ProcessResult const Boot(::std::string const& imagePath, on_write_function onWrite = nullptr);
	}
}

#endif
