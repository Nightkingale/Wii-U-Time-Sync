# Wii U Time Sync

A plugin that synchronizes a Wii U's clock to the Internet.

Wii U Time Sync is a Wii U homebrew plugin for the Aroma environment. It allows the console to automatically synchronize its date and time through the Internet, similar to the feature found on the Nintendo Switch and other modern devices.

## Installation
A Wii U plugin file will be bundled with each release. It should be placed on your SD card, particularly in `wiiu/environments/aroma/plugins`.
* It's important to have the Aroma environment installed for Wii U Time Sync to work. Please visit our [hacking guide](https://wiiu.hacks.guide/) and the [Aroma webpage](https://aroma.foryour.cafe/) if you would like to softmod your Wii U console.

## Usage
If the program is placed correctly on an SD card, Wii U Time Sync will be listed in the Aroma environment's Wii U Plugin System Config Menu.

* If Wii U Time Sync doesn't show up in the Wii U Plugin System Config Menu, confirm you placed the WPS file on your SD card correctly and restart your console.
* `Configuration -> Syncing Enabled`: Enables syncing to the Internet, `off` by default.
* `Configuration -> Show Notifications`: Shows a notification whenever Wii U Time Sync adjusts the clock, `normal` by default.
    * `quiet` means that no notifications will appear on success.
    * `normal` means that only success or failure notifications will appear, but no others.
    * `verbose` means that all notifications (statistics and such) will appear, useful for debugging.
* `Configuration -> Time Offset (UTC)`: The amount of time to add/subtract from the coordinated universal time, `+00:00` by default.
* `Configuration -> Detect Timezone (press A)`: Uses the IP Geolocation API to guess the timezone, setting the offset accordingly.
* `Configuration -> Auto-update Timezone`: Automatically utilizes the IP Geolocation API to set your offset accordingly, `off` by default.
* `Configuration -> Notification Duration (seconds)`: The amount of seconds which notifications will appear on screen for, `5` by default.
* `Configuration -> Tolerance (milliseconds)`: The amount of milliseconds in which Wii U Time Sync will tolerate differences, `500` by default.
* `Configuration -> Background Threads`: Controls how many servers are queried at once, `4` by default.
    * If you stick to the default server, you do not need to set this to more than `4`.
* `Configuration -> NTP Servers`: The list of NTP servers in which the plugin connects to, only `ntp.pool.org` by default.
    * This cannot be edited on the console. However, you can edit the Wii U Time Sync configuration file on a computer to adjust the default server, or add more.
        * The configuration file: `wiiu/environments/aroma/plugins/config/Wii U Time Sync.json`
        * An example edit: `"server": "pool.ntp.org time.windows.com",`
* `Preview Time`: Lets you preview what the system's clock is currently set to, as well as correction and latency statistics.

As long as syncing is enabled by the user, the clock will sync whenever Wii U Time Sync starts, or when the plugin settings are exited.

**The changes will not be reflected in the HOME Menu and most other applications right away, so the console will need to be rebooted for changes to be completed.**

## Credits
I hope that I am able to express my thanks as much as possible to those who made this repository possible.
* [dkosmari](https://github.com/dkosmari), who is now co-developing Wii U Time Sync with me and has greatly contributed to the plugin for a much longer period, including his refactoring in the v2.0.0 release.
* [GaryOderNichts](https://github.com/GaryOderNichts), for writing the network connection code and figuring out how to set the console's date and time through homebrew (so basically all the functionality).
* [Maschell](https://github.com/Maschell), for his work not only with figuring out setting the date and time, but also his work on the Aroma environment.
* [LumaTeam](https://github.com/LumaTeam), for the time syncing code in [Luma3DS](https://github.com/LumaTeam/Luma3DS), which we based our code off of.
* [Lettier](https://github.com/lettier), for his work on [NTP Client](https://github.com/lettier/ntpclient), which in turn led to the code in both Luma3DS and Wii U Time Sync.