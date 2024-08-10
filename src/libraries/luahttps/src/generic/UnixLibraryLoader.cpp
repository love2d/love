#include "../common/config.h"
#include "../common/LibraryLoader.h"

#ifdef HTTPS_LIBRARY_LOADER_UNIX

#include <dlfcn.h>

namespace LibraryLoader
{
	handle *OpenLibrary(const char *name)
	{
		return dlopen(name, RTLD_LAZY);
	}

	void CloseLibrary(handle *handle)
	{
		if (handle)
			dlclose(handle);
	}

	handle* GetCurrentProcessHandle()
	{
		return RTLD_DEFAULT;
	}

	function *GetFunction(handle *handle, const char *name)
	{
		return reinterpret_cast<function *>(dlsym(handle, name));
	}
}

#endif // HTTPS_LIBRARY_LOADER_UNIX

