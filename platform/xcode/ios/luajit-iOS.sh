mkdir -p include/luajit
mkdir -p libraries/luajit

git clone https://github.com/LuaJIT/LuaJIT.git luajit-git
cd luajit-git
git pull
git checkout v2.1

# iOS device binaries

ISDKP=$(xcrun --sdk iphoneos --show-sdk-path)
ICC=$(xcrun --sdk iphoneos --find clang)

ISDKF="-arch armv7 -isysroot $ISDKP -mios-version-min=6.0"
make clean
make -j8 HOST_CC="clang -m32 -arch i386" CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
cp src/libluajit.a ../libraries/luajit/libluajit_arm7.a

ISDKF="-arch arm64 -isysroot $ISDKP -mios-version-min=6.0"
make clean
make -j8 CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
cp src/libluajit.a ../libraries/luajit/libluajit_arm64.a


# iOS simulator binaries

ISDKP=$(xcrun --sdk iphonesimulator --show-sdk-path)
ICC=$(xcrun --sdk iphonesimulator --find clang)

ISDKF="-arch i386 -isysroot $ISDKP -mios-simulator-version-min=6.0"
make clean
make -j8 HOST_CC="clang -m32 -arch i386" CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
cp src/libluajit.a ../libraries/luajit/libluajit_x86.a

ISDKF="-arch x86_64 -isysroot $ISDKP -mios-simulator-version-min=6.0"
make clean
make -j8 CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
cp src/libluajit.a ../libraries/luajit/libluajit_x86_64.a


# copy includes
cp src/lua.hpp ../include/luajit

cp src/lauxlib.h ../include/luajit
cp src/lua.h ../include/luajit
cp src/luaconf.h ../include/luajit
cp src/lualib.h ../include/luajit
cp src/luajit.h ../include/luajit

# combine lib
cd ../libraries/luajit
lipo -create -output libluajit.a libluajit_arm7.a libluajit_arm64.a libluajit_x86.a libluajit_x86_64.a
