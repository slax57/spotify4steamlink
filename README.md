# spotify4steamlink
On this repo you will find a basic example showing how you can run [librespot](https://github.com/librespot-org/librespot) on the [steamlink](https://store.steampowered.com/app/353380/Steam_Link/).

## Disclaimer
What you will find on this repo is just the result of some experiments I made to get something that fitted my own needs concerning spotify and the steamlink. It might not necessarily fit yours. I am just sharing this code in case some people happen to have the same exact needs as me, or are looking after some technical information about how you can pipe the librespot audio backend to another process.

As such, note that:
* The code is really not optimised nor modular. You might have to refactor it to fit your needs.
* I may try to improve it in the future, mainly to add some configuration variables, or maybe to add some features like displaying the current track and so... But please note that I only do this on my free time, so this might come late (if ever), and for now I do not intend to offer much support for what I posted. As I said earlier, this is just an experiment I made for myself, and I just thought it might be useful to others if I shared it here :)
* Using _librespot_ to connect to Spotify's API is probably forbidden by them. Use at your own risk.

## Prerequisite
* This example is based on the open source unofficial spotify client [librespot](https://github.com/librespot-org/librespot). You will have to get it and build it prior to using this app. Although I give some tips about the cross-compilation parameters I used, if you need support for building you need to address your issues to them directly.
* _librespot_ requires a Premium Spotify account.
* This app needs the [steamlink SDK](https://github.com/ValveSoftware/steamlink-sdk) to build. Download it and get familiar with it first.

## Functionalities
This code allows to build an app that you can install and run on your _steamlink_. Once it is installed, you can launch it directly from the _steamlink_ home menu.
This app starts _librespot_ as a background program, and pipes the audio to the first audio device it finds.
You juste have to use _spotify connect_ (from your phone or a computer, using the official Spotify application) to start a song.
Once you're done, you can exit the app by pressing any key on your keyboard or any buttons on your gamepad (should be compatible whith the Steam gamepad).

**Be aware that this is all the app does for now!** Since the latest version, the librespot logs are displayed, allowing you to know which track is being played. But still, there is no complex GUI with album cover or so, it's basically just a headless player with a very simple interface.

# Instructions
## Building
### librespot
Follow [librespot cross compiling instructions](https://github.com/librespot-org/librespot/wiki/Cross-compiling) provided on their wiki.
The target architecture `arm-unknown-linux-gnueabihf` is compatible with the _steamlink_.

**Before you use the docker image to build librespot**, you need to apply the patch described [here](https://github.com/librespot-org/librespot/wiki/Compile-librespot-for-kernel-prior-3.9).
This will enable the _discovery_ feature, allowing you to launch librespot without having to provide your Spotify account credentials.

Once that _mDNS_ is patched, build the _docker_ image as suggested:
```Shell
docker build -t librespot-cross -f contrib/Dockerfile .
```

Then, use this line to build for the correct architecture:
```Shell
docker run -v /tmp/librespot-build:/build librespot-cross cargo build --release --target arm-unknown-linux-gnueabihf --no-default-features
```

### spotify4steamlink
The sources of this app are meant to be placed in the `examples` folder of the **steamlink sdk**.
They contain a `build_steamlink.sh` script that takes care of the cross compilation for you.

You need to paste the _librespot_ binaries you juste compiled in the `librespot-org-build` folder of this repo.

Then you can now launch
```Shell
./build_steamlink.sh
```

If the build succeeds, it should generate this file in your current directory:
```
steamlink/apps/spotify4steamlink.tgz
```

## Installation
Now we are going to install _spotify4steamlink_ on the _steamlink_.

Copy the `steamlink` folder that was created during the build step to a USB flash drive.
Insert it into the Steam Link and power cycle the device.

After the reboot, you should see a nice **Spotify** icon next to your computer(s)! :)
Start the app, and you're ready to play some Spotify tunes directly from your _steamlink_!

# Troubleshooting
By default the app is configured to redirect _librespot_ logs into this file:
```
/tmp/spotify.log
```
If you have trouble using _spotify4steamlink_, you should start by having a look at this file ;).

You might also encounter troubles when exiting the app. For now, the only workaround is to unplug the _steamlink_, and then plug it back again.
Still, during my tests, I noticed the problem seems **not to happen** if you exit **while playing a song**.
Hopefully having this knowledge will prevent you troubles :).

# Acknowledgements
* [plietar](https://github.com/plietar/) for the original [librespot](https://github.com/plietar/librespot) project
* The [librespot-org](https://github.com/librespot-org/librespot) team, for their great work and maintenance of the current implementation
* The [steamlink sdk](https://github.com/ValveSoftware/steamlink-sdk/) which, although not enough documented for my taste, still offers a good start to enhance you _steamlink_ device
* [This example](https://gist.github.com/armornick/3447121) on how to play audio with SDL
