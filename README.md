# rd2lbot

## Configuration:

The bot is configured via environment variables:

- STEAM_USER: Username for Steam
- STEAM_PASS: Password for Steam
- IRC_NICK: Nickname for IRC

Steamguard and 2FA are unsupported.

## Building (Linux)

### Requirements
- [premake5](https://premake.github.io/) 
- gcc 7 and a modern libstdc++

### Building

```
premake5 gmake
cd build
make config=release_x64 # Configs: debug_x86, debug_x64, release_x86, release_x64
```

Binary should now be in build/bin/

## Building (Windows)

### Requirements
- Visual Studio 2017 Update 3 Preview 2.1
- Protobuf
- Boost 
- cryptopp
- zlib

Using vcpkg the dependencies can be installed via 

```
vcpkg install protobuf:x64-windows boost:x64-windows cryptopp:x64-windows zlib:x64-windows libpq:x64-windows
```

## Configuring
```
premake5 vs2017
```

Open build/rd2lbot.sln in Visual Studio and compile.

