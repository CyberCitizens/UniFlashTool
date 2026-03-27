// Copyright (C) 2026 Mathéo Allart <imacybercitizen@gmail.com>
// SPDX‑License‑Identifier: GPLv3.0

#ifndef UFT_FLASH
#define UFT_FLASH

#include "../platform/deps.hpp"
#include "tools.h"

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

	
	// Gets the name of the currently connected device
	::std::string const GetConnectedDeviceCodename();
	// Gets the state of the currently connected device.
	DEVICE_STATE const GetConnectedDeviceState();
	// Reboots a device to its bootloader, enabling fastboot commands.
	::std::string const RebootToBootloader();
	// Ensures ADB is running, starts a new server if no server is connected, and returns the result output.
	::std::string const EnsureADB();
	// Loads a .zip into a phone.
	::std::string const Sideload(::std::string const& filePath);
	namespace FastBoot
	{
		// Returns true if a device is connected with fastboot, false if not.
		bool HasDevice();
		// Flashes a file into a given partition, if a device is connected
		::std::string const Flash(PARTITION const partition, ::std::string const& filename);
		// Reboots into a known partition
		// (leave empty for standard reboot)
		::std::string const Reboot(PARTITION const partition = SYSTEM);
		// Boots on a file without flashing anything (memory boot, doesn't affect storage data)
		::std::string const Boot(::std::string const& imagePath);
	}
}

#endif
