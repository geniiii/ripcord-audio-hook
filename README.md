# ripcord-audio-hook
Fixes Ripcord:
1. not connecting due to the binary ETF parser aborting upon discovering invalid/unhandled map keys
2. reading RTP header extensions larger than 8 bytes as voice data, causing garbled audio in voice chats
3. sending and receiving an old (?) version of the IP discovery packet, causing it to get stuck on "Routing..." when attempting to join voice channels
4. not loading image previews
5. not showing "Stage" voice channels  
   *(This is accomplished by making Ripcord think stages are regular voice channels. This means that there is no way to become a speaker, and no way to differentiate a stage from a regular voice channel)*

Supports Ripcord 0.4.29 on Windows. [Supports Ripcord 0.4.29 on Linux, albeit with limited testing.](https://github.com/geniiii/ripcord-audio-hook/tree/linux)

## Usage
Download the DLL file from [the Releases page](https://github.com/geniiii/ripcord-audio-hook/releases) and place it in your Ripcord directory.

## Building
**If you don't know what you're doing, look at the Usage section above instead.**
1. Install Visual Studio 2019 or newer
2. Run `build.bat`

## Credits
[@p0358](https://github.com/p0358) for writing [Northstar's `loader_wsock32_proxy`](https://github.com/R2Northstar/NorthstarLauncher/tree/main/loader_wsock32_proxy)  
[@u130b8](https://github.com/u130b8) for fixing image previews  
[@ouwou](https://github.com/ouwou) for determining the cause of garbled audio and suggesting hijacking `profapi.dll`
