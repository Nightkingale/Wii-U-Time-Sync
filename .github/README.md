# Wii U Time Sync

A plugin that synchronizes a Wii U's clock to the Internet.

<p align="left">
  <a href="https://discord.nightkingale.com/">
    <img src="https://img.shields.io/badge/Discord-5865F2?style=for-the-badge&logo=discord&logoColor=white" alt="Join us!" width="10%" height="10%">
  </a>
  <a href="https://donate.nightkingale.com/">
    <img src="https://img.shields.io/badge/PayPal-00457C?style=for-the-badge&logo=paypal&logoColor=white" alt="Thank you!" width="10%" height="10%">
  </a>
  <a href="https://nightkingale.com/">
    <img src="https://img.shields.io/badge/website-000000?style=for-the-badge&logo=About.me&logoColor=white" alt="Visit us!" width="10%" height="10%">
  </a>
</p>

Wii U Time Sync is an Aroma plugin which automatically synchronizes the console's clock using one or more NTP servers.

## Installation
For convenience, it is recommended that you download Wii U Time Sync from the [Homebrew App Store](https://hb-app.store/wiiu/Wii-U-Time-Sync). Both are maintained by me, and I ensure that the mirror is updated instantaneously after a new release.

<p align="center">
  <a href="https://hb-app.store/wiiu/Wii-U-Time-Sync">
    <img src="appstore.png" alt="Get it on the Homebrew App Store!" width="50%" height="50%">
  </a>
</p>

Otherwise, a Wii U plugin file will be bundled with each release. It should be placed on your SD card, particularly in `wiiu/environments/aroma/plugins`.
* It's important to have the Aroma environment installed for Wii U Time Sync to work. Please visit our [hacking guide](https://wiiu.hacks.guide/) and the [Aroma webpage](https://aroma.foryour.cafe/) if you would like to softmod your Wii U console.

## Usage
If the program is placed correctly on an SD card, Wii U Time Sync will be listed in the Aroma environment's Wii U Plugin System Config Menu.

* If Wii U Time Sync doesn't show up in the Wii U Plugin System Config Menu, confirm you placed the WPS file on your SD card correctly and restart your console.

### Configuration
* `Configuration -> Syncing Enabled`: Enables syncing to the Internet, `off` by default.
* `Configuration -> Show Notifications`: Shows a notification whenever Wii U Time Sync adjusts the clock, `normal` by default.
    * `quiet` means that no notifications will appear on success.
    * `normal` means that only success or failure notifications will appear, but no others.
    * `verbose` means that all notifications (statistics and such) will appear, useful for debugging.
* `Configuration -> Time Offset (UTC)`: The amount of time to add/subtract from the coordinated universal time, `+00:00` by default.
* `Configuration -> Detect Time Zone (press A)`: Uses one of three different APIs to guess the time zone, setting the offset accordingly.
  * http://ip-api.com
  * https://ipwho.is
  * https://ipapi.co
* `Configuration -> Auto Update Time Zone`: Automatically utilizes an IP Geolocation API to set your offset accordingly, `off` by default.
* `Configuration -> Notification Duration`: The amount of seconds which notifications will appear on screen for, `5 s` by default.
* `Configuration -> Timeout`: The amount of seconds before an established NTP connection will timeout, `5 s` by default.
* `Configuration -> Tolerance`: The amount of milliseconds in which Wii U Time Sync will tolerate differences, `500 ms` by default.
* `Configuration -> Background Threads`: Controls how many servers are queried at once, `4` by default.
    * If you stick to the default server, you do not need to set this to more than `4`.
* `Configuration -> NTP Servers`: The list of NTP servers in which the plugin connects to, only `pool.ntp.org` by default.
    * This cannot be edited on the console. However, you can edit the Wii U Time Sync configuration file on a computer to adjust the default server, or add more.
        * The configuration file: `wiiu/environments/aroma/plugins/config/Wii U Time Sync.json`
        * An example edit: `"server": "pool.ntp.org time.windows.com",`
* `Preview Time`: Lets you preview what the system's clock is currently set to, as well as correction and latency statistics.

For values you would like to set back to default, you can press the X button while highlighting the option you would like to reset.

As long as syncing is enabled by the user, the clock will sync using NTP whenever Wii U Time Sync starts, or when the plugin settings are exited.

## Assistance
If you encounter bugs, the best place to report them would be the [Issues](https://github.com/Nightkingale/Wii-U-Time-Sync/issues) tab. This allows for easy tracking and reference, though please check for duplicates first and comment there if possible!

For assistance or other inquiries, the best place to reach out would be the [Nightkingale Studios](https://discord.nightkingale.com/) Discord server ([#chat-hangout](https://discord.com/channels/450846070025748480/1127657272315740260) is okay). I am active in many other Wii U homebrew Discord servers as well.

### Frequently Asked Questions

**1. Why doesn't it seem like the time changes when I manually run a sync?**

The changes will not be reflected in the HOME Menu and most other applications right away, so the console will need to be rebooted for changes to be completed.

## Compilation
You should have [wut](https://github.com/devkitPro/wut) installed. Following the [devkitPro Getting Started guide](https://devkitpro.org/wiki/Getting_Started) will set you up with it and all other basic dependancies. Additionally, you will need the following dependancies:

* [libcurlwrapper](https://github.com/wiiu-env/libcurlwrapper)
* [libnotifications](https://github.com/wiiu-env/libnotifications)
* [wiiupluginsystem](https://github.com/wiiu-env/wiiupluginsystem)

After all dependancies have been installed, you can just run `make`.

* In addition, you can pass `DEBUG=1` as an argument, which will include the commit hash as part of the version string inside the binary. All products produced by [workflows](https://github.com/Nightkingale/Wii-U-Time-Sync/actions) utilize this.
* The resulting binary will be a `.wps` file. You can use this the same way as a release binary.

## Credits
I hope that I am able to express my thanks as much as possible to those who made this repository possible.

* [dkosmari](https://github.com/dkosmari), who is now co-developing Wii U Time Sync with me and has greatly contributed to the plugin for a much longer period, including his refactoring in the v2.0.0 release.
* [GaryOderNichts](https://github.com/GaryOderNichts), for writing the network connection code and figuring out how to set the console's date and time through homebrew (so basically all the functionality).
* [Maschell](https://github.com/Maschell), for his work not only with figuring out setting the date and time, but also his work on the Aroma environment.
* [LumaTeam](https://github.com/LumaTeam), for the time syncing code in [Luma3DS](https://github.com/LumaTeam/Luma3DS), which we based our code off of.
* [Lettier](https://github.com/lettier), for his work on [NTP Client](https://github.com/lettier/ntpclient), which in turn led to the code in both Luma3DS and Wii U Time Sync.