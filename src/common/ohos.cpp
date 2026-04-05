#include "config.h"

#ifdef LOVE_OHOS

#include "ohos.h"

#include <sys/stat.h>
#include <sys/types.h>

namespace love
{
namespace ohos
{

static std::string g_sandboxPath;
static std::string g_gameResourcePath;

static bool directoryExists(const char *path)
{
	struct stat st;
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static bool tryCreateDirectory(const std::string &path)
{
	if (path.empty())
		return false;
	if (directoryExists(path.c_str()))
		return true;
	return ::mkdir(path.c_str(), 0755) == 0;
}

void setSandboxPath(const char *path)
{
	g_sandboxPath = path ? path : "";
}

const std::string &getSandboxPath()
{
	return g_sandboxPath;
}

void setGameResourcePath(const char *path)
{
	g_gameResourcePath = path ? path : "";
}

const std::string &getGameResourcePath()
{
	return g_gameResourcePath;
}

bool createStorageDirectories()
{
	if (g_sandboxPath.empty())
		return false;

	if (!tryCreateDirectory(g_sandboxPath))
		return false;

	if (!tryCreateDirectory(g_sandboxPath + "/save"))
		return false;

	if (!tryCreateDirectory(g_sandboxPath + "/game"))
		return false;

	return true;
}

} // namespace ohos
} // namespace love

#endif
