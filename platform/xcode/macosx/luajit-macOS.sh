mkdir -p Sources/LuaJIT

git clone https://github.com/LuaJIT/LuaJIT.git Sources/LuaJIT
cd Sources/LuaJIT
git pull
git checkout v2.1

INSTALL_NAME="@rpath/Lua.framework/Versions/A/Lua"

export MACOSX_DEPLOYMENT_TARGET=10.7

# build x64
make clean
make -j8 TARGET_FLAGS="-arch x86_64"
install_name_tool -id $INSTALL_NAME src/libluajit.so
cp src/libluajit.so libluajit_x86_64.dylib

# build arm64
make clean
make -j8 TARGET_FLAGS="-arch arm64"
install_name_tool -id $INSTALL_NAME src/libluajit.so
cp src/libluajit.so libluajit_arm64.dylib

# combine lib
lipo -create -output libluajit.dylib libluajit_arm64.dylib libluajit_x86_64.dylib

FRAMEWORK_PATH="../../Frameworks/Lua.framework/Versions/A"

# copy lib
cp libluajit.dylib $FRAMEWORK_PATH/Lua


# copy includes
cp src/lua.hpp $FRAMEWORK_PATH/Headers

cp src/lauxlib.h $FRAMEWORK_PATH/Headers
cp src/lua.h $FRAMEWORK_PATH/Headers
cp src/luaconf.h $FRAMEWORK_PATH/Headers
cp src/lualib.h $FRAMEWORK_PATH/Headers
cp src/luajit.h $FRAMEWORK_PATH/Headers
