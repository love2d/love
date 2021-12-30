mkdir -p include/luajit
mkdir -p libraries/luajit

if [ ! -d luajit-git ]; then
	git clone https://github.com/LuaJIT/LuaJIT.git luajit-git
fi
cd luajit-git
git pull --no-rebase
git checkout v2.1

export MACOSX_DEPLOYMENT_TARGET=10.7

# iOS device binaries
# LuaJIT does not support building for armv7 on modern macOS versions.

ISDKP=$(xcrun --sdk iphoneos --show-sdk-path)
ICC=$(xcrun --sdk iphoneos --find clang)

ISDKF="-arch arm64 -isysroot $ISDKP -mios-version-min=8.0"
make clean TARGET_SYS=iOS
make -j8 CC="clang" CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
cp src/libluajit.a ../libraries/luajit/libluajit_arm64_device.a


# iOS simulator binaries

ISDKP=$(xcrun --sdk iphonesimulator --show-sdk-path)
ICC=$(xcrun --sdk iphonesimulator --find clang)

ISDKF="-arch x86_64 -isysroot $ISDKP -mios-simulator-version-min=8.0"
make clean TARGET_SYS=iOS
make -j8 CC="clang" CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
cp src/libluajit.a ../libraries/luajit/libluajit_x86_64_sim.a

ISDKF="-arch arm64 -isysroot $ISDKP -mios-simulator-version-min=8.0"
make clean TARGET_SYS=iOS
make -j8 CC="clang" CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
cp src/libluajit.a ../libraries/luajit/libluajit_arm64_sim.a


# copy includes
cp src/lua.hpp ../include/luajit

cp src/lauxlib.h ../include/luajit
cp src/lua.h ../include/luajit
cp src/luaconf.h ../include/luajit
cp src/lualib.h ../include/luajit
cp src/luajit.h ../include/luajit

# combine lib
cd ../libraries/luajit
lipo -create -output libluajit_device.a libluajit_arm64_device.a
lipo -create -output libluajit_sim.a libluajit_x86_64_sim.a libluajit_arm64_sim.a

# create xcframework with all platforms
rm -rf Lua.xcframework
xcodebuild -create-xcframework -library libluajit_device.a -headers ../../include/luajit -library libluajit_sim.a -headers ../../include/luajit -output Lua.xcframework
