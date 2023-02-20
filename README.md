# ripcord-audio-hook
Fixes Ripcord reading RTP header extensions larger than 8 bytes as voice data, causing garbled audio in voice chats.  
Only supports Ripcord 0.4.29.

## Usage
Download the DLL file from [the Releases tab](https://github.com/geniiii/ripcord-audio-hook/releases) and place it in your Ripcord directory.

## Building
1. Install Visual Studio 2019 or newer
2. Run `build.bat`

## Credits
[@p0358](https://github.com/p0358) for writing [Northstar's `loader_wsock32_proxy`](https://github.com/R2Northstar/NorthstarLauncher/tree/main/loader_wsock32_proxy)
