/**
 * Copyright (c) 2006-2024 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "android.h"
#include "Object.h"

#ifdef LOVE_ANDROID

#include <cerrno>
#include <set>
#include <unordered_map>

#include <SDL3/SDL.h>

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "libraries/physfs/physfs.h"
#include "filesystem/physfs/PhysfsIo.h"

namespace love
{
namespace android
{

void setImmersive(bool immersive_active)
{
	JNIEnv *env = (JNIEnv*) SDL_GetAndroidJNIEnv();
	jobject activity = (jobject) SDL_GetAndroidActivity();
	jclass clazz = env->GetObjectClass(activity);

	static jmethodID setImmersiveMethod = env->GetMethodID(clazz, "setImmersiveMode", "(Z)V");
	env->CallVoidMethod(activity, setImmersiveMethod, immersive_active);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);
}

bool getImmersive()
{
	JNIEnv *env = (JNIEnv*) SDL_GetAndroidJNIEnv();
	jobject activity = (jobject) SDL_GetAndroidActivity();
	jclass clazz = env->GetObjectClass(activity);

	static jmethodID getImmersiveMethod = env->GetMethodID(clazz, "getImmersiveMode", "()Z");
	jboolean immersiveActive = env->CallBooleanMethod(activity, getImmersiveMethod);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	return immersiveActive;
}

double getScreenScale()
{
	static double result = -1.;

	if (result == -1.)
	{
		JNIEnv *env = (JNIEnv*) SDL_GetAndroidJNIEnv();
		jobject activity = (jobject) SDL_GetAndroidActivity();
		jclass clazz = env->GetObjectClass(activity);

		jmethodID getDPIMethod = env->GetMethodID(clazz, "getDPIScale", "()F");
		result = (double) env->CallFloatMethod(activity, getDPIMethod);

		env->DeleteLocalRef(clazz);
		env->DeleteLocalRef(activity);
	}

	return result;
}

bool getSafeArea(int &top, int &left, int &bottom, int &right)
{
	JNIEnv *env = (JNIEnv*) SDL_GetAndroidJNIEnv();
	jobject activity = (jobject) SDL_GetAndroidActivity();
	jclass clazz = env->GetObjectClass(activity);
    jclass rectClass = env->FindClass("android/graphics/Rect");
    jmethodID methodID = env->GetMethodID(clazz, "getSafeArea", "()Landroid/graphics/Rect;");
	jobject safeArea = env->CallObjectMethod(activity, methodID);

	if (safeArea != nullptr)
	{
		top = env->GetIntField(activity, env->GetFieldID(rectClass, "top", "I"));
		left = env->GetIntField(activity, env->GetFieldID(rectClass, "left", "I"));
		bottom = env->GetIntField(activity, env->GetFieldID(rectClass, "bottom", "I"));
		right = env->GetIntField(activity, env->GetFieldID(rectClass, "right", "I"));
        env->DeleteLocalRef(safeArea);
	}

    env->DeleteLocalRef(rectClass);
	env->DeleteLocalRef(clazz);
	env->DeleteLocalRef(activity);

	return safeArea != nullptr;
}

bool openURL(const std::string &url)
{
	return SDL_OpenURL(url.c_str()) == 0;
}

void vibrate(double seconds)
{
	JNIEnv *env = (JNIEnv*) SDL_GetAndroidJNIEnv();
	jobject activity = (jobject) SDL_GetAndroidActivity();
	jclass clazz = env->GetObjectClass(activity);

	static jmethodID vibrateMethod = env->GetMethodID(clazz, "vibrate", "(D)V");
	env->CallVoidMethod(activity, vibrateMethod, seconds);

	env->DeleteLocalRef(clazz);
	env->DeleteLocalRef(activity);
}

/*
 * Helper functions for the filesystem module
 */
void freeGameArchiveMemory(void *ptr)
{
	char *game_love_data = static_cast<char*>(ptr);
	delete[] game_love_data;
}

bool directoryExists(const char *path)
{
	struct stat s {};
	int err = stat(path, &s);
	if (err == -1)
	{
		if (errno != ENOENT)
			SDL_Log("Error checking for directory %s errno = %d: %s", path, errno, strerror(errno));
		return false;
	}

	return S_ISDIR(s.st_mode);
}

bool mkdir(const char *path)
{
	int err = ::mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO | S_ISGID);
	if (err == -1)
	{
		const char *error = strerror(errno);
		SDL_Log("Error: Could not create directory '%s': %s", path, error);
		return false;
	}

	return true;
}

bool chmod(const char *path, int mode)
{
	int err = ::chmod(path, mode);
	if (err == -1)
	{
		const char *error = strerror(errno);
		SDL_Log("Error: Could not change mode '%s': %s", path, error);
		return false;
	}

	return true;
}

inline bool tryCreateDirectory(const char *path)
{
	SDL_Log("Trying to create directory '%s'", path);

	if (directoryExists(path))
		return true;
	else if (mkdir(path))
		return true;
	return false;
}

bool createStorageDirectories()
{
	std::string internalStoragePath = SDL_GetAndroidInternalStoragePath();
	std::string externalStoragePath = SDL_GetAndroidExternalStoragePath();

	std::string saveDirectoryInternal = internalStoragePath + "/save";
	if (!tryCreateDirectory(saveDirectoryInternal.c_str()))
		return false;

	std::string saveDirectoryExternal = externalStoragePath + "/save";
	if (!tryCreateDirectory(saveDirectoryExternal.c_str()))
		return false;

	std::string game_directory = externalStoragePath + "/game";
	if (!tryCreateDirectory (game_directory.c_str()))
		return false;

	return true;
}

void fixupPermissionSingleFile(const std::string &savedir, const std::string &path, int mode)
{
    std::string fixedSavedir = savedir.back() == '/' ? savedir : (savedir + "/");
    std::string target = fixedSavedir + path;
    ::chmod(target.c_str(), mode);
}

void fixupExternalStoragePermission(const std::string &savedir, const std::string &path)
{
	std::set<std::string> pathsToFix;
	size_t start = 0;

	while (true)
	{
		size_t pos = path.find('/', start);
		if (pos == std::string::npos)
		{
			pathsToFix.insert(path);
			break;
		}

		pathsToFix.insert(path.substr(0, pos));
		start = pos + 1;
	}

	std::string fixedSavedir = savedir.back() == '/' ? savedir : (savedir + "/");
	chmod(savedir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO | S_ISGID);

	for (const std::string &dir: pathsToFix)
	{
        const char *realPath = PHYSFS_getRealDir(dir.c_str());
		if (!dir.empty() && strcmp(realPath, savedir.c_str()) == 0)
		{
			std::string target = fixedSavedir + dir;
			chmod(target.c_str(), S_IRWXU | S_IRWXG | S_IRWXO | S_ISGID);
		}
	}
}

bool hasBackgroundMusic()
{
	JNIEnv *env = (JNIEnv*) SDL_GetAndroidJNIEnv();
	jobject activity = (jobject) SDL_GetAndroidActivity();

	jclass clazz(env->GetObjectClass(activity));
	jmethodID method_id = env->GetMethodID(clazz, "hasBackgroundMusic", "()Z");

	jboolean result = env->CallBooleanMethod(activity, method_id);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	return result;
}

bool hasRecordingPermission()
{
	JNIEnv *env = (JNIEnv*) SDL_GetAndroidJNIEnv();
	jobject activity = (jobject) SDL_GetAndroidActivity();
	jclass clazz = env->GetObjectClass(activity);

	static jmethodID methodID = env->GetMethodID(clazz, "hasRecordAudioPermission", "()Z");
	jboolean result = false;

	if (methodID == nullptr)
		env->ExceptionClear();
	else
		result = env->CallBooleanMethod(activity, methodID);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	return result;
}


void requestRecordingPermission()
{
	JNIEnv *env = (JNIEnv*) SDL_GetAndroidJNIEnv();
	jobject activity = (jobject) SDL_GetAndroidActivity();
	jclass clazz(env->GetObjectClass(activity));
	jmethodID methodID = env->GetMethodID(clazz, "requestRecordAudioPermission", "()V");

	if (methodID == nullptr)
		env->ExceptionClear();
	else
		env->CallVoidMethod(activity, methodID);

	env->DeleteLocalRef(clazz);
	env->DeleteLocalRef(activity);
}

void showRecordingPermissionMissingDialog()
{
	JNIEnv *env = (JNIEnv*) SDL_GetAndroidJNIEnv();
	jobject activity = (jobject) SDL_GetAndroidActivity();
	jclass clazz(env->GetObjectClass(activity));
	jmethodID methodID = env->GetMethodID(clazz, "showRecordingAudioPermissionMissingDialog", "()V");

	if (methodID == nullptr)
		env->ExceptionClear();
	else
		env->CallVoidMethod(activity, methodID);

	env->DeleteLocalRef(clazz);
	env->DeleteLocalRef(activity);
}

/* A container for AssetManager Java object */
class AssetManagerObject
{
public:
	AssetManagerObject()
	{
		JNIEnv *env = (JNIEnv *) SDL_GetAndroidJNIEnv();
		jobject am = getLocalAssetManager(env);

		assetManager = env->NewGlobalRef(am);
		env->DeleteLocalRef(am);
	}

	~AssetManagerObject()
	{
		JNIEnv *env = (JNIEnv *) SDL_GetAndroidJNIEnv();
		env->DeleteGlobalRef(assetManager);
	}

	static jobject getLocalAssetManager(JNIEnv *env) {
		jobject self = (jobject) SDL_GetAndroidActivity();
		jclass activity = env->GetObjectClass(self);
		jmethodID method = env->GetMethodID(activity, "getAssets", "()Landroid/content/res/AssetManager;");
		jobject am = env->CallObjectMethod(self, method);

		env->DeleteLocalRef(self);
		env->DeleteLocalRef(activity);
		return am;
	}

	explicit operator jobject()
	{
		return assetManager;
	};
private:
	jobject assetManager;
};

/*
 * Helper functions to aid new fusing method
 */

// This returns *global* reference, no need to free it.
static jobject getJavaAssetManager()
{
	static AssetManagerObject assetManager;
	return (jobject) assetManager;
}

static AAssetManager *getAssetManager()
{
	JNIEnv *env = (JNIEnv*) SDL_GetAndroidJNIEnv();
	return AAssetManager_fromJava(env, (jobject) getJavaAssetManager());
}

namespace aasset
{

struct AssetInfo: public love::filesystem::physfs::PhysfsIo<AssetInfo>
{
	static const uint32_t version = 0;

	AAssetManager *assetManager;
	AAsset *asset;
	char *filename;
	size_t size;

	static AssetInfo *fromAAsset(AAssetManager *assetManager, const char *filename, AAsset *asset)
	{
		return new AssetInfo(assetManager, filename, asset);
	}

	int64_t read(void* buf, uint64_t len) const
	{
		int readed = AAsset_read(asset, buf, (size_t) len);

		PHYSFS_setErrorCode(readed < 0 ? PHYSFS_ERR_OS_ERROR : PHYSFS_ERR_OK);
		return (PHYSFS_sint64) readed;
	}

	int64_t write(const void* buf, uint64_t len) const
	{
		LOVE_UNUSED(buf);
		LOVE_UNUSED(len);

		PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
		return -1;
	}

	int64_t seek(uint64_t offset) const
	{
		int64_t success = AAsset_seek64(asset, (off64_t) offset, SEEK_SET) != -1;

		PHYSFS_setErrorCode(success ? PHYSFS_ERR_OK : PHYSFS_ERR_OS_ERROR);
		return success;
	}

	int64_t tell() const
	{
		off64_t len = AAsset_getLength64(asset);
		off64_t remain = AAsset_getRemainingLength64(asset);

		return len - remain;
	}

	int64_t length() const
	{
		return AAsset_getLength64(asset);
	}

	int64_t flush() const
	{
		// Do nothing
		PHYSFS_setErrorCode(PHYSFS_ERR_OK);
		return 1;
	}

	AssetInfo(const AssetInfo &other)
	: assetManager(other.assetManager)
	, size(strlen(other.filename) + 1)
	{
		asset = AAssetManager_open(assetManager, other.filename, AASSET_MODE_RANDOM);

		if (asset == nullptr)
		{
			PHYSFS_setErrorCode(PHYSFS_ERR_OS_ERROR);
			throw new love::Exception("Unable to duplicate AssetInfo");
		}

		filename = new (std::nothrow) char[size];
		memcpy(filename, other.filename, size);
	}

	~AssetInfo() override
	{
		AAsset_close(asset);
		delete[] filename;
	}

private:
	AssetInfo(AAssetManager *assetManager, const char *filename, AAsset *asset)
	: assetManager(assetManager)
	, asset(asset)
	, size(strlen(filename) + 1)
	{
		this->filename = new (std::nothrow) char[size];
		memcpy(this->filename, filename, size);
	}
};

static std::unordered_map<std::string, PHYSFS_FileType> fileTree;

void *openArchive(PHYSFS_Io *io, const char *name, int forWrite, int *claimed)
{
	if (forWrite || io->opaque == nullptr || memcmp(io->opaque, "ASET", 4) != 0)
		return nullptr;

	// It's our archive
	*claimed = 1;
	AAssetManager *assetManager = getAssetManager();

	if (fileTree.empty())
	{
		// AAssetDir_getNextFileName intentionally excludes directories, so
		// we have to use JNI that calls AssetManager.list() recursively.
		JNIEnv *env = (JNIEnv *) SDL_GetAndroidJNIEnv();
		jobject activity = (jobject) SDL_GetAndroidActivity();
		jclass clazz = env->GetObjectClass(activity);

		jmethodID method = env->GetMethodID(clazz, "buildFileTree", "()[Ljava/lang/String;");
		jobjectArray list = (jobjectArray) env->CallObjectMethod(activity, method);

		for (jsize i = 0; i < env->GetArrayLength(list); i++)
		{
			jstring jstr = (jstring) env->GetObjectArrayElement(list, i);
			const char *str = env->GetStringUTFChars(jstr, nullptr);

			fileTree[str + 1] = str[0] == 'd' ? PHYSFS_FILETYPE_DIRECTORY : PHYSFS_FILETYPE_REGULAR;

			env->ReleaseStringUTFChars(jstr, str);
			env->DeleteLocalRef(jstr);
		}

		env->DeleteLocalRef(list);
		env->DeleteLocalRef(clazz);
		env->DeleteLocalRef(activity);
	}

	return assetManager;
}

PHYSFS_EnumerateCallbackResult enumerate(
	void *opaque,
	const char *dirname,
	PHYSFS_EnumerateCallback cb,
	const char *origdir,
	void *callbackdata
)
{
	using FileTreeIterator = std::unordered_map<std::string, PHYSFS_FileType>::iterator;
	LOVE_UNUSED(opaque);

	const char *path = dirname;
	if (path == nullptr || (path[0] == '/' && path[1] == 0))
		path = "";

	if (path[0] != 0)
	{
		FileTreeIterator result = fileTree.find(path);

		if (result == fileTree.end() || result->second != PHYSFS_FILETYPE_DIRECTORY)
		{
			PHYSFS_setErrorCode(PHYSFS_ERR_NOT_FOUND);
			return PHYSFS_ENUM_ERROR;
		}
	}

	JNIEnv *env = (JNIEnv *) SDL_GetAndroidJNIEnv();
	jobject assetManager = getJavaAssetManager();
	jclass clazz = env->GetObjectClass(assetManager);
	jmethodID method = env->GetMethodID(clazz, "list", "(Ljava/lang/String;)[Ljava/lang/String;");

	jstring jstringDir = env->NewStringUTF(path);
	jobjectArray dir = (jobjectArray) env->CallObjectMethod(assetManager, method, jstringDir);

	PHYSFS_EnumerateCallbackResult ret = PHYSFS_ENUM_OK;

	if (env->ExceptionCheck())
	{
		// IOException occured
		ret = PHYSFS_ENUM_ERROR;
		env->ExceptionClear();
	}
	else
	{
		jsize i = 0;
		jsize len = env->GetArrayLength(dir);

		while (ret == PHYSFS_ENUM_OK && i < len) {
			jstring jstr = (jstring) env->GetObjectArrayElement(dir, i++);
			const char *name = env->GetStringUTFChars(jstr, nullptr);

			ret = cb(callbackdata, origdir, name);

			env->ReleaseStringUTFChars(jstr, name);
			env->DeleteLocalRef(jstr);
		}

		env->DeleteLocalRef(dir);
	}

	env->DeleteLocalRef(jstringDir);
	env->DeleteLocalRef(clazz);
	return ret;
}

PHYSFS_Io *openRead(void *opaque, const char *name)
{
	AAssetManager *assetManager = (AAssetManager *) opaque;
	AAsset *file = AAssetManager_open(assetManager, name, AASSET_MODE_UNKNOWN);

	if (file == nullptr)
	{
		PHYSFS_setErrorCode(PHYSFS_ERR_NOT_FOUND);
		return nullptr;
	}

	PHYSFS_setErrorCode(PHYSFS_ERR_OK);
	return AssetInfo::fromAAsset(assetManager, name, file);
}

PHYSFS_Io *openWriteAppend(void *opaque, const char *name)
{
	LOVE_UNUSED(opaque);
	LOVE_UNUSED(name);

	// AAsset doesn't support modification
	PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
	return nullptr;
}

int removeMkdir(void *opaque, const char *name)
{
	LOVE_UNUSED(opaque);
	LOVE_UNUSED(name);

	// AAsset doesn't support modification
	PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
	return 0;
}

int stat(void *opaque, const char *name, PHYSFS_Stat *out)
{
	using FileTreeIterator = std::unordered_map<std::string, PHYSFS_FileType>::iterator;
	LOVE_UNUSED(opaque);

	FileTreeIterator result = fileTree.find(name);

	if (result != fileTree.end())
	{
		out->filetype = result->second;
		out->modtime = -1;
		out->createtime = -1;
		out->accesstime = -1;
		out->readonly = 1;

		PHYSFS_setErrorCode(PHYSFS_ERR_OK);
		return 1;
	}

	PHYSFS_setErrorCode(PHYSFS_ERR_NOT_FOUND);
	return 0;
}

void closeArchive(void *opaque)
{
	// Do nothing
	LOVE_UNUSED(opaque);
	PHYSFS_setErrorCode(PHYSFS_ERR_OK);
}

static PHYSFS_Archiver g_AAssetArchiver = {
	0,
	{
		"AASSET",
		"Android AAsset Wrapper",
		"LOVE Development Team",
		"https://developer.android.com/ndk/reference/group/asset",
		0
	},
	openArchive,
	enumerate,
	openRead,
	openWriteAppend,
	openWriteAppend,
	removeMkdir,
	removeMkdir,
	stat,
	closeArchive
};

static PHYSFS_sint64 dummyReturn0(PHYSFS_Io *io)
{
	LOVE_UNUSED(io);
	PHYSFS_setErrorCode(PHYSFS_ERR_OK);
	return 0;
}

static PHYSFS_Io *getDummyIO(PHYSFS_Io *io);

static char dummyOpaque[] = "ASET";
static PHYSFS_Io dummyIo = {
	0,
	dummyOpaque,
	nullptr,
	nullptr,
	[](PHYSFS_Io *io, PHYSFS_uint64 offset) -> int
	{
		PHYSFS_setErrorCode(offset == 0 ? PHYSFS_ERR_OK : PHYSFS_ERR_PAST_EOF);
		return offset == 0;
	},
	dummyReturn0,
	dummyReturn0,
	getDummyIO,
	nullptr,
	[](PHYSFS_Io *io) { LOVE_UNUSED(io); }
};

static PHYSFS_Io *getDummyIO(PHYSFS_Io *io)
{
	return &dummyIo;
}

}

static bool isVirtualArchiveInitialized = false;

bool initializeVirtualArchive()
{
	if (isVirtualArchiveInitialized)
		return true;

	if (!PHYSFS_registerArchiver(&aasset::g_AAssetArchiver))
		return false;
	if (!PHYSFS_mountIo(&aasset::dummyIo, "ASET.AASSET", nullptr, 0))
	{
		PHYSFS_deregisterArchiver(aasset::g_AAssetArchiver.info.extension);
		return false;
	}

	isVirtualArchiveInitialized = true;
	return true;
}

void deinitializeVirtualArchive()
{
	if (isVirtualArchiveInitialized)
	{
		PHYSFS_deregisterArchiver(aasset::g_AAssetArchiver.info.extension);
		isVirtualArchiveInitialized = false;
	}
}

bool checkFusedGame(void **physfsIO_Out)
{
	PHYSFS_Io *&io = *(PHYSFS_Io **) physfsIO_Out;
	AAssetManager *assetManager = getAssetManager();

	// Prefer main.lua inside assets/ folder
	AAsset *asset = AAssetManager_open(assetManager, "main.lua", AASSET_MODE_STREAMING);
	if (asset)
	{
		AAsset_close(asset);
		io = nullptr;
		return true;
	}

	// If there's no main.lua inside assets/ try game.love
	asset = AAssetManager_open(assetManager, "game.love", AASSET_MODE_RANDOM);
	if (asset)
	{
		io = aasset::AssetInfo::fromAAsset(assetManager, "game.love", asset);
		return true;
	}

	// Not found
	return false;
}

const char *getCRequirePath()
{
	static bool initialized = false;
	static std::string path;

	if (!initialized)
	{
		JNIEnv *env = (JNIEnv*) SDL_GetAndroidJNIEnv();
		jobject activity = (jobject) SDL_GetAndroidActivity();
		jclass clazz = env->GetObjectClass(activity);

		static jmethodID getCRequireMethod = env->GetMethodID(clazz, "getCRequirePath", "()Ljava/lang/String;");

		jstring cpath = (jstring) env->CallObjectMethod(activity, getCRequireMethod);
		const char *utf = env->GetStringUTFChars(cpath, nullptr);
		if (utf)
		{
			path = utf;
			env->ReleaseStringUTFChars(cpath, utf);
		}

		env->DeleteLocalRef(cpath);
		env->DeleteLocalRef(activity);
		env->DeleteLocalRef(clazz);
	}

	return path.c_str();
}

int getFDFromContentProtocol(const char *path)
{
	int fd = -1;

	if (strstr(path, "content://") == path)
	{
		JNIEnv *env = (JNIEnv*) SDL_GetAndroidJNIEnv();
		jobject activity = (jobject) SDL_GetAndroidActivity();
		jclass clazz = env->GetObjectClass(activity);

		static jmethodID converter = env->GetMethodID(clazz, "convertToFileDescriptor", "(Ljava/lang/String;)I");

		jstring uri = env->NewStringUTF(path);
		fd = (int) env->CallIntMethod(activity, converter, uri);

		env->DeleteLocalRef(uri);
		env->DeleteLocalRef(clazz);
		env->DeleteLocalRef(activity);
	}

	return fd;
}

int getFDFromLoveProtocol(const char *path)
{
	constexpr const char PROTOCOL[] = "love2d://fd/";
	constexpr size_t PROTOCOL_LEN = sizeof(PROTOCOL) - 1;

	if (*path == '/')
		path++;

	if (memcmp(path, PROTOCOL, PROTOCOL_LEN) == 0)
	{
		try
		{
			return std::stoi(path + PROTOCOL_LEN, nullptr, 10);
		}
		catch (std::logic_error &)
		{ }
	}

	return -1;
}

class FileDescriptorTracker: public love::Object
{
public:
	explicit FileDescriptorTracker(int fd): Object(), fd(fd) {}
	~FileDescriptorTracker() override { close(fd); }
	int getFd() const { return fd; }
private:
	int fd;
};

struct FileDescriptorIO
{
	FileDescriptorTracker *fd;
	off64_t size;
	off64_t offset;
};

void *getIOFromFD(int fd)
{
	if (fd == -1)
		return nullptr;

	// Create file descriptor IO structure
	FileDescriptorIO *fdio = new FileDescriptorIO();
	fdio->size = lseek64(fd, 0, SEEK_END);
	fdio->offset = 0;
	lseek64(fd, 0, SEEK_SET);

	if (fdio->size == -1)
	{
		// Cannot get size
		delete fdio;
		return nullptr;
	}

	fdio->fd = new FileDescriptorTracker(fd);

	PHYSFS_Io *io = new PHYSFS_Io();
	io->version = 0;
	io->opaque = fdio;
	io->read = [](PHYSFS_Io *io, void *buf, PHYSFS_uint64 size)
	{
		FileDescriptorIO *fdio = (FileDescriptorIO *) io->opaque;
		ssize_t ret = pread64(fdio->fd->getFd(), buf, (size_t) size, fdio->offset);

		if (ret == -1)
			PHYSFS_setErrorCode(PHYSFS_ERR_OTHER_ERROR);
		else
			fdio->offset = std::min(fdio->offset + (off64_t) ret, fdio->size);

		return (PHYSFS_sint64) ret;
	};
	io->write = nullptr;
	io->seek = [](PHYSFS_Io *io, PHYSFS_uint64 offset)
	{
		FileDescriptorIO *fdio = (FileDescriptorIO *) io->opaque;
		fdio->offset = std::min(std::max<off64_t>((off64_t) offset, 0), fdio->size);
		// Always success
		return 1;
	};
	io->tell = [](PHYSFS_Io *io)
	{
		FileDescriptorIO *fdio = (FileDescriptorIO *) io->opaque;
		return (PHYSFS_sint64) fdio->offset;
	};
	io->length = [](PHYSFS_Io *io)
	{
		FileDescriptorIO *fdio = (FileDescriptorIO *) io->opaque;
		return (PHYSFS_sint64) fdio->size;
	};
	io->duplicate = [](PHYSFS_Io *io)
	{
		FileDescriptorIO *fdio = (FileDescriptorIO *) io->opaque;
		FileDescriptorIO *fdio2 = new FileDescriptorIO();
		PHYSFS_Io *io2 = new PHYSFS_Io();

		fdio->fd->retain();

		// Copy data
		*fdio2 = *fdio;
		*io2 = *io;
		io2->opaque = fdio2;

		return io2;
	};
	io->flush = nullptr;
	io->destroy = [](PHYSFS_Io *io)
	{
		FileDescriptorIO *fdio = (FileDescriptorIO *) io->opaque;
		fdio->fd->release();
		delete fdio;
		delete io;
	};

	return io;
}

const char *getArg0()
{
	static PHYSFS_AndroidInit androidInit = {nullptr, nullptr};
	androidInit.jnienv = SDL_GetAndroidJNIEnv();
	androidInit.context = SDL_GetAndroidActivity();
	return (const char *) &androidInit;
}

} // android
} // love

#endif // LOVE_ANDROID
