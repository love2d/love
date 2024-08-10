#include "../common/config.h"
#include "../common/LibraryLoader.h"

#ifdef HTTPS_LIBRARY_LOADER_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

namespace LibraryLoader
{
	handle *OpenLibrary(const char *name)
	{
		return reinterpret_cast<handle *>(LoadLibraryA(name));
	}

	void CloseLibrary(handle *handle)
	{
		if (handle)
			FreeLibrary(reinterpret_cast<HMODULE>(handle));
	}

	handle* GetCurrentProcessHandle()
	{
		return reinterpret_cast<handle *>(GetModuleHandle(nullptr));
	}

	function *GetFunction(handle *handle, const char *name)
	{
		HMODULE nativeHandle = reinterpret_cast<HMODULE>(handle);
		return reinterpret_cast<function *>(GetProcAddress(nativeHandle, name));
	}
}

#endif // HTTPS_LIBRARY_LOADER_WINDOWS
