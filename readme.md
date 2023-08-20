LÖVE is an *awesome* framework you can use to make 2D games in Lua. It's free, open-source, and works on Windows, macOS, Linux, Android, and iOS.

[![Build Status: Windows](https://ci.appveyor.com/api/projects/status/chc0hdr08wv1d5c7?svg=true)](https://ci.appveyor.com/project/slime73/love)
[![Build Status: Github CI](https://github.com/love2d/love/workflows/continuous-integration/badge.svg)](https://github.com/love2d/love/actions?query=workflow%3Acontinuous-integration)

Documentation
-------------

We use our [wiki][wiki] for documentation.
If you need further help, feel free to ask on our [forums][forums], our [Discord server][discord], or our [subreddit][subreddit].

Repository
----------

We use the 'main' branch for patch development of the current major release, and therefore it should not be considered stable.
There may also be a branch for the next major version in development, which is named after that version.

We tag all our releases (since we started using mercurial and git), and have binary downloads available for them.

Experimental changes are developed in a separate [love-experiments][love-experiments] repository.

Builds
------

Files for releases are in the [releases][releases] section on GitHub. [The site][site] has links to files and additional platform content for the latest release.

There are also unstable/nightly builds:

- Builds for some platforms are automatically created after each commit and are available through GitHub's CI interfaces.
- For ubuntu linux they are in [ppa:bartbes/love-unstable][unstableppa]
- For arch linux there's [love-git][aur] in the AUR.

Contributing
------------

The best places to contribute are through the issue tracker and the official Discord server or IRC channel.

For code contributions, pull requests and patches are welcome. Be sure to read the [source code style guide][codestyle].
Changes and new features typically get discussed in the issue tracker or on Discord or the forums before a pull request is made.

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
Download or clone [this repository][dependencies-apple] and copy, move, or symlink the `macOS/Frameworks` subfolder into love's `platform/xcode/macosx` folder.

Then use the Xcode project found at `platform/xcode/love.xcodeproj` to build the `love-macosx` target.

### iOS
Building for iOS requires macOS and Xcode.

#### LÖVE 11.4 and newer
Download the `love-apple-dependencies` zip file corresponding to the LÖVE version being used from the [Releases page][dependencies-ios],
unzip it, and place the `iOS/libraries` subfolder into love's `platform/xcode/ios` folder.

Or, download or clone [this repository][dependencies-apple] and copy, move, or symlink the `iOS/libraries` subfolder into love's `platform/xcode/ios` folder.

Then use the Xcode project found at `platform/xcode/love.xcodeproj` to build the `love-ios` target.

See `readme-iOS.rtf` for more information.

#### LÖVE 11.3 and older
Download the `ios-libraries` zip file corresponding to the LÖVE version being used from the [Releases page][dependencies-ios],
unzip it, and place the `include` and `libraries` subfolders into love's `platform/xcode/ios` folder.

Then use the Xcode project found at `platform/xcode/love.xcodeproj` to build the `love-ios` target.

See `readme-iOS.rtf` for more information.

### Android
Visit the [Android build repository][android-repository] for build instructions.

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

[site]: https://love2d.org
[wiki]: https://love2d.org/wiki
[forums]: https://love2d.org/forums
[discord]: https://discord.gg/rhUets9
[subreddit]: https://www.reddit.com/r/love2d
[dependencies-apple]: https://github.com/love2d/love-apple-dependencies
[dependencies-ios]: https://github.com/love2d/love/releases
[megasource]: https://github.com/love2d/megasource
[unstableppa]: https://launchpad.net/~bartbes/+archive/love-unstable
[aur]: https://aur.archlinux.org/packages/love-git
[love-experiments]: https://github.com/slime73/love-experiments
[codestyle]: https://love2d.org/wiki/Code_Style
[android-repository]: https://github.com/love2d/love-android
[releases]: https://github.com/love2d/love/releases
