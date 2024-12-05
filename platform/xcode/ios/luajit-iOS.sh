# mkdir -p Sources/LuaJIT

git clone https://github.com/LuaJIT/LuaJIT.git Sources/LuaJIT
cd Sources/LuaJIT
git pull --no-rebase
git checkout v2.1

mkdir tmpbuild

export MACOSX_DEPLOYMENT_TARGET=10.9

# iOS device binaries
# LuaJIT does not support building for armv7 on modern macOS versions.

ISDKP=$(xcrun --sdk iphoneos --show-sdk-path)
ICC=$(xcrun --sdk iphoneos --find clang)

ISDKF="-arch arm64 -isysroot $ISDKP -mios-version-min=8.0"
make clean TARGET_SYS=iOS
make -j8 CC="clang" CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
cp src/libluajit.a tmpbuild/libluajit_arm64_device.a


# iOS simulator binaries

ISDKP=$(xcrun --sdk iphonesimulator --show-sdk-path)
ICC=$(xcrun --sdk iphonesimulator --find clang)

ISDKF="-arch x86_64 -isysroot $ISDKP -mios-simulator-version-min=8.0"
make clean TARGET_SYS=iOS
make -j8 CC="clang" CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
cp src/libluajit.a tmpbuild/libluajit_x86_64_sim.a

ISDKF="-arch arm64 -isysroot $ISDKP -mios-simulator-version-min=8.0"
make clean TARGET_SYS=iOS
make -j8 CC="clang" CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
cp src/libluajit.a tmpbuild/libluajit_arm64_sim.a

# copy includes
mkdir tmpbuild/include

cp src/lua.hpp tmpbuild/include
cp src/lauxlib.h tmpbuild/include
cp src/lua.h tmpbuild/include
cp src/luaconf.h tmpbuild/include
cp src/lualib.h tmpbuild/include
cp src/luajit.h tmpbuild/include

# combine lib
lipo -create -output tmpbuild/libluajit_device.a tmpbuild/libluajit_arm64_device.a
lipo -create -output tmpbuild/libluajit_sim.a tmpbuild/libluajit_x86_64_sim.a tmpbuild/libluajit_arm64_sim.a

# create xcframework with all platforms
rm -rf tmpbuild/Lua.xcframework
xcodebuild -create-xcframework -library tmpbuild/libluajit_device.a -headers tmpbuild/include -library tmpbuild/libluajit_sim.a -headers tmpbuild/include -output tmpbuild/Lua.xcframework

cd ../../
rm -rf libraries/Lua.xcframework
cp -R Sources/LuaJIT/tmpbuild/Lua.xcframework libraries
