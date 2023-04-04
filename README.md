# ripcord-audio-hook (Linux)
Fixes Ripcord:
1. reading RTP header extensions larger than 8 bytes as voice data, causing garbled audio in voice chats
2. sending and receiving an old (?) version of the IP discovery packet, causing it to get stuck on "Routing..." when attempting to join voice channels

## Usage
For `hook.so`, see [Building](#Building). I don't distribute binaries as I have no idea what the proper way to do it is on Linux.
1. Extract the Ripcord AppImage. (`./Ripcord-0.4.29-x86_64.AppImage --appimage-extract`)
2. Copy `hook.so` to the directory where the AppImage was extracted (should be `squashfs-root`).
3. Run Ripcord using `LD_PRELOAD=./hook.so ./Ripcord`. You should now be able to connect to voice channels.  
*If you get an error that looks something like "cannot open shared object file: No such file or directory" even after installing funchook, check your `LD_LIBRARY_PATH` or run `ldconfig`.*

## Building
1. Build and install [funchook](https://github.com/kubo/funchook).
2. `gcc -shared -fPIC -o hook.so hook.c -ldl -lfunchook` (or the equivalent command for your compiler of choice)
