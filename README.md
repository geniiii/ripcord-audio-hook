# ripcord-audio-hook (Linux)
Fixes Ripcord sending and receiving an old (?) version of the IP discovery packet, causing it to get stuck on "Routing..." when attempting to join voice channels

## Usage
For `hook.so`, see [Building](#Building). I don't distribute binaries as I have no idea what the proper way to do it is on Linux.
1. Extract the Ripcord AppImage. (`./Ripcord-0.4.29-x86_64.AppImage --appimage-extract`)
2. Copy `hook.so` to the directory where the AppImage was extracted (should be `squashfs-root`).
3. Run Ripcord using `LD_PRELOAD=./hook.so ./Ripcord`. You should now be able to connect to voice channels.

## Building
`gcc -shared -fPIC -ldl -o hook.so hook.c` (or the equivalent command for your compiler of choice)
