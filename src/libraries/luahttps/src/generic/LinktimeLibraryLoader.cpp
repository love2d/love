#include "../common/config.h"
#include "../common/LibraryLoader.h"

#ifdef HTTPS_LIBRARY_LOADER_LINKTIME

#include <cstring>

#ifdef HTTPS_BACKEND_CURL
#include <curl/curl.h>

static char CurlHandle;
#endif

#if defined(HTTPS_BACKEND_OPENSSL) || defined(HTTPS_BACKEND_ANDROID)
#	error "Selected backends that are not compatible with this loader"
#endif

namespace LibraryLoader
{
	handle *OpenLibrary(const char *name)
	{
#ifdef HTTPS_BACKEND_CURL
		if (strstr(name, "libcurl") == name)
			return reinterpret_cast<handle *>(&CurlHandle);
#endif
		return nullptr;
	}

	void CloseLibrary(handle *)
	{
	}

	handle* GetCurrentProcessHandle()
	{
		return nullptr;
	}

	function *GetFunction(handle *handle, const char *name)
	{
#define RETURN_MATCHING_FUNCTION(func) \
	if (strcmp(name, #func) == 0) \
		return reinterpret_cast<function *>(&func);

#ifdef HTTPS_BACKEND_CURL
		if (handle == &CurlHandle)
		{
			RETURN_MATCHING_FUNCTION(curl_global_init);
			RETURN_MATCHING_FUNCTION(curl_global_cleanup);
			RETURN_MATCHING_FUNCTION(curl_easy_init);
			RETURN_MATCHING_FUNCTION(curl_easy_cleanup);
			RETURN_MATCHING_FUNCTION(curl_easy_setopt);
			RETURN_MATCHING_FUNCTION(curl_easy_perform);
			RETURN_MATCHING_FUNCTION(curl_easy_getinfo);
			RETURN_MATCHING_FUNCTION(curl_slist_append);
			RETURN_MATCHING_FUNCTION(curl_slist_free_all);
		}
#endif

#undef RETURN_MATCHING_FUNCTION

		return nullptr;
	}
}

#endif // HTTPS_LIBRARY_LOADER_LINKTIME

