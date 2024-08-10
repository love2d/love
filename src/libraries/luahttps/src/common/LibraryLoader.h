#pragma once

namespace LibraryLoader
{
	using handle = void;
	using function = void();

	handle *OpenLibrary(const char *name);
	void CloseLibrary(handle *handle);
	handle* GetCurrentProcessHandle();

	function *GetFunction(handle *handle, const char *name);

	template<class T>
	inline bool LoadSymbol(T& var, handle *handle, const char *name)
	{
		var = reinterpret_cast<T>(GetFunction(handle, name));
		return var != nullptr;
	}
}
