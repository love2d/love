#pragma once

#include "../common/config.h"

#ifdef HTTPS_BACKEND_CURL

#include <curl/curl.h>

#include "../common/HTTPSClient.h"
#include "../common/LibraryLoader.h"

class CurlClient : public HTTPSClient
{
public:
	virtual bool valid() const override;
	virtual HTTPSClient::Reply request(const HTTPSClient::Request &req) override;

private:
	static struct Curl
	{
		Curl();
		~Curl();
		LibraryLoader::handle *handle;
		bool loaded;

		decltype(&curl_global_cleanup) global_cleanup;

		decltype(&curl_easy_init) easy_init;
		decltype(&curl_easy_cleanup) easy_cleanup;
		decltype(&curl_easy_setopt) easy_setopt;
		decltype(&curl_easy_perform) easy_perform;
		decltype(&curl_easy_getinfo) easy_getinfo;

		decltype(&curl_slist_append) slist_append;
		decltype(&curl_slist_free_all) slist_free_all;
	} curl;
};

#endif // HTTPS_BACKEND_CURL
