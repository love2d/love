#ifndef LOVE_OHOS_H
#define LOVE_OHOS_H

#include <string>

namespace love
{
namespace ohos
{

void setSandboxPath(const char *path);
const std::string &getSandboxPath();

void setGameResourcePath(const char *path);
const std::string &getGameResourcePath();

bool createStorageDirectories();

} // namespace ohos
} // namespace love

#endif
