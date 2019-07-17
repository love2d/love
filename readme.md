LÖVE is an *awesome* framework you can use to make 2D games in Lua. It's free, open-source, and works on Windows, Mac OS X, Linux, Android, and iOS.

[![Build Status: Windows](https://ci.appveyor.com/api/projects/status/u1a69u5o5ej1pus4?svg=true)](https://ci.appveyor.com/project/AlexSzpakowski/love)

Documentation
-------------

We use our [wiki][wiki] for documentation.
If you need further help, feel free to ask on our [forums][forums], our [Discord server][discord], or our IRC channel [#love on OFTC][irc].

Compilation
-----------

### Windows
Follow the instructions at the [megasource][megasource] repository page.

### *nix
Run `platform/unix/automagic` from the repository root, then run ./configure and make.

	$ platform/unix/automagic
	$ ./configure
	$ make

When using a source release, automagic has already been run, and the first step can be skipped.

### macOS
Download the required frameworks from [here][dependencies] and place them in `/Library/Frameworks/`.

Then use the Xcode project found at `platform/xcode/love.xcodeproj` to build the `love-macosx` target.

### iOS
Download the `ios-libraries` zip file corresponding to the LÖVE version being used from [here][dependencies-ios],
unzip it, and place the `include` and `libraries` subfolders into LÖVE's `platform/xcode/ios` folder.

Then use the Xcode project found at `platform/xcode/love.xcodeproj` to build the `love-ios` target.

See `readme-iOS.rtf` for more information.

### Android
Visit the [Android build repository][android-repository] for build instructions.

Repository information
----------------------

We use the 'default' branch for development, and therefore it should not be considered stable.
Also used is the 'minor' branch, which is used for features in the next minor version and it is
not our development target (which would be the next revision - version numbers are formatted major.minor.revision.)

We tag all our releases (since we started using mercurial), and have binary downloads available for them.

Experimental changes are developed in the separate [love-experiments][love-experiments] repository.

Contributing
------------

The best places to contribute are through the Bitbucket issue tracker and the official Discord server or IRC channel.
For code contributions, pull requests and patches are welcome. Be sure to read the [source code style guide][codestyle].

Builds
------

Releases are found in the 'downloads' section on bitbucket, are linked on [the site][site],
and there's a ppa for ubuntu, [ppa:bartbes/love-stable][stableppa].

There are also unstable/nightly builds:

- Most can be found [here][builds].
- For ubuntu linux they are in [ppa:bartbes/love-unstable][unstableppa]
- For arch linux there's [love-hg][aur] in the AUR.

Dependencies
------------

- SDL2
- OpenGL 2.1+ / OpenGL ES 2+
- OpenAL
- Lua / LuaJIT / LLVM-lua
- FreeType
- ModPlug
- mpg123
- Vorbisfile
- Theora

[site]: http://love2d.org
[wiki]: http://love2d.org/wiki
[forums]: http://love2d.org/forums
[discord]: https://discord.gg/rhUets9
[irc]: irc://irc.oftc.net/love
[dependencies]: http://love2d.org/sdk
[dependencies-ios]: https://bitbucket.org/rude/love/downloads/
[megasource]: https://bitbucket.org/rude/megasource
[builds]: http://love2d.org/builds
[stableppa]: https://launchpad.net/~bartbes/+archive/love-stable
[unstableppa]: https://launchpad.net/~bartbes/+archive/love-unstable
[aur]: http://aur.archlinux.org/packages/love-hg
[love-experiments]: https://bitbucket.org/bartbes/love-experiments
[codestyle]: https://love2d.org/wiki/Code_Style
[android-repository]: https://bitbucket.org/MartinFelis/love-android-sdl2
