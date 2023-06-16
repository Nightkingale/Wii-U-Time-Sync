# Wii U Time Sync

A plugin that synchronizes a Wii U's clock to the Internet.

Wii U Time Sync is a Wii U homebrew plugin for the Aroma environment. It allows the console to automatically synchronize it's date and time through the Internet, similar to the feature found on the Nintendo Switch and other modern devices.

## Installation
A Wii U plugin file will be bundled with each release. It should be placed on your SD card, particularly in `wiiu/environments/aroma/plugins`.
* It's important to have the Aroma environment installed for Wii U Time Sync to work. Please visit our [hacking guide](https://wiiu.hacks.guide/) and the [Aroma webpage](https://aroma.foryour.cafe/) if you would like to softmod your Wii U console.

## Usage
If the program is placed correctly on an SD card, Wii U Time Sync will be listed in the Aroma environment's Wii U Plugin System Config Menu.

* If Wii U Time Sync doesn't show up in the Wii U Plugin System Config Menu, confirm you placed the WPS file on your SD card correctly and restart your console.
* `Configuration -> Syncing Enabled`: Enables syncing to the Internet, `false` by default.
* `Configuration -> Daylight Savings`: Adds an extra hour to account for Daylight Savings Time, `false` by default.
* `Configuration -> Time Offsets (hours)`: The amount of hours to add/subtract from the coordinated universal time, `0` by default.
* `Configuration -> Time Offsets (minutes)`: The amount of minutes to add/subtract from the coordinated universal time, `0` by default.
* `Preview Time`: Lets you preview what the system's clock is currently set to.

As long as syncing is enabled by the user, the clock will sync whenever Wii U Time Sync starts, or when the plugin settings are exited.

**The changes will not be reflected in the Home Menu and most other applications right away, so the console will need to be rebooted for changes to be completed.**

## Credits
I hope that I am able to express my thanks as much as possible to those who made this repository possible.
* [GaryOderNichts](https://github.com/GaryOderNichts), for writing the network connection code and figuring out how to set the console's date and time through homebrew (so basically all the functionality).
* [LumaTeam](https://github.com/LumaTeam), for the time syncing code in [Luma3DS](https://github.com/LumaTeam/Luma3DS), which we based our code off of.
* [Lettier](https://github.com/lettier), for his work on [NTP Client](https://github.com/lettier/ntpclient), which in turn led to the code in both Luma3DS and Wii U Time Sync.