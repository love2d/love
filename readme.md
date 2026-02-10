# LÖVE with Lua 5.4

本仓库是 [LÖVE](https://love2d.org) 2D 游戏框架的 **Lua 5.4 移植分支**，基于 LÖVE 12.0 (main) 修改，使其能够使用 **Lua 5.4.7** 替代默认的 LuaJIT / Lua 5.1 进行编译和运行。

## 目的

LÖVE 官方默认使用 LuaJIT 作为脚本引擎，但 LuaJIT 长期停留在 Lua 5.1 语法级别。本仓库的目标是让 LÖVE 支持 Lua 5.4 的全部新特性，包括：

- `<const>` / `<close>` 变量属性
- `//` 整数除法运算符
- 原生位运算符（`&`, `|`, `~`, `<<`, `>>`）
- `math.type()` 区分 integer / float
- 内置 `utf8` 标准库
- `warn()` 警告系统
- `math.maxinteger` / `math.mininteger`
- 整数与浮点数的严格类型区分

## 构建环境

- **编译器**: GCC 14.2.0 (MSYS2 MinGW-w64)
- **构建系统**: CMake 3.31+
- **Lua**: 5.4.7 (`pacman -S mingw-w64-x86_64-lua`)
- **平台**: Windows (MSYS2 MinGW64)

### 构建命令

```bash
cmake -B build -S . -G "MinGW Makefiles" -DLOVE_JIT=OFF
cmake --build build -j$(nproc)
```

## 修改内容

相对于 LÖVE 12.0 官方 main 分支，本仓库做了以下修改：

### 1. CMakeLists.txt

- **Lua 5.4 查找与兼容宏**：将 `find_package(Lua51)` 改为 `find_package(Lua 5.4)`，添加 `LUA_COMPAT_5_3` 编译宏以保留向后兼容 API
- **CPack 空目标修复**：为 `$<TARGET_FILE:${MEGA_SDL3}>` 添加 `if(MEGA_SDL3)` 守护，修复非 MEGA 构建时的 CMake 生成器表达式语法错误
- **MinGW 链接器循环依赖修复**：GCC 单遍链接器无法自动处理 LÖVE 静态库间的循环引用，显式列出所有 STATIC 库目标并重复两次来解决
- **luahttps Windows 库链接**：将 `wininet`、`ws2_32`、`secur32` 的链接条件从 `if(MSVC)` 扩展为 `if(MSVC OR MINGW)`

### 2. src/modules/event/wrap_Event.cpp

- **lua_cpcall 兼容**：`lua_cpcall` 在 Lua 5.2+ 中被移除，添加条件编译使用 `lua_pushcfunction` + `lua_pcall` 替代

### 3. src/modules/love/love.cpp

- **utf8 库预加载**：Lua 5.4 内置 `utf8` 标准库，在 `LUA_VERSION_NUM >= 504` 时跳过 LÖVE 自定义的 `luaopen_luautf8` 预加载

### 4. .gitignore

- 添加 `/build/` 目录

## 验证

构建产物通过了 8 项 Lua 5.4 特性验证测试（见 `snake/` 目录中的贪吃蛇示例游戏）：

```
✓ 1. <const> 变量属性
✓ 2. // 整数除法
✓ 3. 位运算符 &, |, <<
✓ 4. math.type() 类型区分
✓ 5. 内置 utf8 标准库
✓ 6. warn() 警告系统
✓ 7. math.maxinteger 常量
✓ 8. 整数/浮点严格区分
```

```
$ lovec --version
LOVE 12.0 (Bestest Friend)
```

## 原始项目

LÖVE 是一个优秀的开源 2D 游戏框架，详见 [原始 README](readme.md.bak) 和 [官方网站](https://love2d.org)。
