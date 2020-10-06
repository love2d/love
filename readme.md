LÖVE is an *awesome* framework you can use to make 2D games in Lua. It's free, open-source, and works on Windows, Mac OS X, Linux, Android, and iOS.

[![Build Status: Windows](https://ci.appveyor.com/api/projects/status/chc0hdr08wv1d5c7?svg=true)](https://ci.appveyor.com/project/AlexSzpakowski/love)
[![Build Status: Github CI](https://github.com/love2d/love/workflows/continuous-integration/badge.svg)](https://github.com/love2d/love/actions?query=workflow%3Acontinuous-integration)

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

We use the 'master' branch for patch development of the current major release, and therefore it should not be considered stable.
There may also be a branch for the next major version in development, which is named after that version.

We tag all our releases (since we started using mercurial and git), and have binary downloads available for them.

Experimental changes are developed in the separate [love-experiments][love-experiments] repository.

Contributing
------------

The best places to contribute are through the issue tracker and the official Discord server or IRC channel.
For code contributions, pull requests and patches are welcome. Be sure to read the [source code style guide][codestyle].

Builds
------

Releases are found in the [releases][releases] section on GitHub, and are linked on [the site][site],
and there's a ppa for ubuntu, [ppa:bartbes/love-stable][stableppa].

There are also unstable/nightly builds:

- Builds for some platforms are automatically created after each commit and are available through GitHub's CI interfaces.
- Otherwise, some less frequently updated builds can can be found [here][builds].
- For ubuntu linux they are in [ppa:bartbes/love-unstable][unstableppa]
- For arch linux there's [love-git][aur] in the AUR.

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
[dependencies-ios]: https://github.com/love2d/love/releases
[megasource]: https://github.com/love2d/megasource
[builds]: http://love2d.org/builds
[stableppa]: https://launchpad.net/~bartbes/+archive/love-stable
[unstableppa]: https://launchpad.net/~bartbes/+archive/love-unstable
[aur]: http://aur.archlinux.org/packages/love-git
[love-experiments]: https://github.com/slime73/love-experiments
[codestyle]: https://love2d.org/wiki/Code_Style
[android-repository]: https://github.com/love2d/love-android
[releases]: https://github.com/love2d/love/releases
