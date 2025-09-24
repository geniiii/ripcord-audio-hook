# ripcord-audio-hook (Linux)
Fixes Ripcord:
1. not connecting <sup>due to the binary ETF parser aborting upon discovering invalid/unhandled map keys</sup>
2. having garbled audio in voice channels <sup>due to reading RTP header extensions larger than 8 bytes as voice data</sup>
3. getting stuck on "Routing..." when attempting to join voice channels <sup>due to sending and receiving an old (?) version of the IP discovery packet</sup>
4. failing to connect to voice channels <sup>due to creating malformed WebSocket gateway URIs</sup>
6. not loading image previews
7. not showing "Stage" voice channels  
   *(This is accomplished by making Ripcord think stages are regular voice channels. This means that there is no way to become a speaker, and no way to differentiate a stage from a regular voice channel)*

## Usage
For `hook.so`, see [Building](#Building). I don't distribute binaries as I have no idea what the proper way to do it is on Linux.
1. Extract the Ripcord AppImage. (`./Ripcord-0.4.29-x86_64.AppImage --appimage-extract`)
2. Copy `hook.so` to the directory where the AppImage was extracted (should be `squashfs-root`).
3. Run Ripcord using `OPENSSL_CONF="" LD_PRELOAD=./hook.so ./Ripcord`. You should now be able to connect to voice channels.  
*If you get an error that looks something like "cannot open shared object file: No such file or directory" even after installing funchook, check your `LD_LIBRARY_PATH` or run `ldconfig`.*

## Building
1. Build and install [funchook](https://github.com/kubo/funchook).
2. `gcc -shared -fPIC -o hook.so hook.c -ldl -lfunchook` (or the equivalent command for your compiler of choice)

## Credits
[@u130b8](https://github.com/u130b8) for fixing image previews
