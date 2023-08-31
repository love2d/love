#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif

#include "CurlClient.h"

#ifdef HTTPS_BACKEND_CURL

#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <vector>

// Dynamic library loader
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

typedef struct StringReader
{
	const std::string *str;
	size_t pos;
} StringReader;

template <class T>
static inline bool loadSymbol(T &var, void *handle, const char *name)
{
#ifdef _WIN32
	var = (T) GetProcAddress((HMODULE) handle, name);
#else
	var = (T) dlsym(handle, name);
#endif
	return var != nullptr;
}

CurlClient::Curl::Curl()
: handle(nullptr)
, loaded(false)
, global_cleanup(nullptr)
, easy_init(nullptr)
, easy_cleanup(nullptr)
, easy_setopt(nullptr)
, easy_perform(nullptr)
, easy_getinfo(nullptr)
, slist_append(nullptr)
, slist_free_all(nullptr)
{
#ifdef _WIN32
	handle = (void *) LoadLibraryA("libcurl.dll");
#else
	handle = dlopen("libcurl.so.4", RTLD_LAZY);
#endif
	if (!handle)
		return;

	// Load symbols
	decltype(&curl_global_init) global_init = nullptr;
	if (!loadSymbol(global_init, handle, "curl_global_init"))
		return;
	if (!loadSymbol(global_cleanup, handle, "curl_global_cleanup"))
		return;
	if (!loadSymbol(easy_init, handle, "curl_easy_init"))
		return;
	if (!loadSymbol(easy_cleanup, handle, "curl_easy_cleanup"))
		return;
	if (!loadSymbol(easy_setopt, handle, "curl_easy_setopt"))
		return;
	if (!loadSymbol(easy_perform, handle, "curl_easy_perform"))
		return;
	if (!loadSymbol(easy_getinfo, handle, "curl_easy_getinfo"))
		return;
	if (!loadSymbol(slist_append, handle, "curl_slist_append"))
		return;
	if (!loadSymbol(slist_free_all, handle, "curl_slist_free_all"))
		return;

	global_init(CURL_GLOBAL_DEFAULT);
	loaded = true;
}

CurlClient::Curl::~Curl()
{
	if (loaded)
		global_cleanup();

	if (handle)
#ifdef _WIN32
		FreeLibrary((HMODULE) handle);
#else
		dlclose(handle);
#endif
}

static char toUppercase(char c)
{
	int ch = (unsigned char) c;
	return toupper(ch);
}

static size_t stringReader(char *ptr, size_t size, size_t nmemb, StringReader *reader)
{
	const char *data = reader->str->data();
	size_t len = reader->str->length();
	size_t maxCount = (len - reader->pos) / size;
	size_t desiredCount = std::min(maxCount, nmemb);
	size_t desiredBytes = desiredCount * size;

	std::copy(data + reader->pos, data + desiredBytes, ptr);
	reader->pos += desiredBytes;

	return desiredCount;
}

static size_t stringstreamWriter(char *ptr, size_t size, size_t nmemb, std::stringstream *ss)
{
	size_t count = size*nmemb;
	ss->write(ptr, count);
	return count;
}

static size_t headerWriter(char *ptr, size_t size, size_t nmemb, std::map<std::string,std::string> *userdata)
{
	std::map<std::string, std::string> &headers = *userdata;
	size_t count = size*nmemb;
	std::string line(ptr, count);
	size_t split = line.find(':');
	size_t newline = line.find('\r');
	if (newline == std::string::npos)
		newline = line.size();

	if (split != std::string::npos)
		headers[line.substr(0, split)] = line.substr(split+1, newline-split-1);
	return count;
}

bool CurlClient::valid() const
{
	return curl.loaded;
}

HTTPSClient::Reply CurlClient::request(const HTTPSClient::Request &req)
{
	Reply reply;
	reply.responseCode = 0;

	// Use sensible default header for later
	HTTPSClient::header_map newHeaders = req.headers;

	CURL *handle = curl.easy_init();
	if (!handle)
		throw std::runtime_error("Could not create curl request");

	curl.easy_setopt(handle, CURLOPT_URL, req.url.c_str());
	curl.easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl.easy_setopt(handle, CURLOPT_CUSTOMREQUEST, req.method.c_str());

	StringReader reader {};

	if (req.postdata.size() > 0 && (req.method != "GET" && req.method != "HEAD"))
	{
		reader.str = &req.postdata;
		reader.pos = 0;
		curl.easy_setopt(handle, CURLOPT_UPLOAD, 1L);
		curl.easy_setopt(handle, CURLOPT_READFUNCTION, stringReader);
		curl.easy_setopt(handle, CURLOPT_READDATA, &reader);
		curl.easy_setopt(handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t) req.postdata.length());
	}

	if (req.method == "HEAD")
		curl.easy_setopt(handle, CURLOPT_NOBODY, 1L);

	// Curl doesn't copy memory, keep the strings around
	std::vector<std::string> lines;
	for (auto &header : newHeaders)
	{
		std::stringstream line;
		line << header.first << ": " << header.second;
		lines.push_back(line.str());
	}

	curl_slist *sendHeaders = nullptr;
	for (auto &line : lines)
		sendHeaders = curl.slist_append(sendHeaders, line.c_str());

	if (sendHeaders)
		curl.easy_setopt(handle, CURLOPT_HTTPHEADER, sendHeaders);

	std::stringstream body;

	curl.easy_setopt(handle, CURLOPT_WRITEFUNCTION, stringstreamWriter);
	curl.easy_setopt(handle, CURLOPT_WRITEDATA, &body);

	curl.easy_setopt(handle, CURLOPT_HEADERFUNCTION, headerWriter);
	curl.easy_setopt(handle, CURLOPT_HEADERDATA, &reply.headers);

	curl.easy_perform(handle);

	if (sendHeaders)
		curl.slist_free_all(sendHeaders);

	{
		long responseCode;
		curl.easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &responseCode);
		reply.responseCode = (int) responseCode;
	}

	reply.body = body.str();

	curl.easy_cleanup(handle);
	return reply;
}

CurlClient::Curl CurlClient::curl;

#endif // HTTPS_BACKEND_CURL
