#pragma once

#include "../common/config.h"

#ifdef HTTPS_BACKEND_CURL

#include <curl/curl.h>

#include "../common/HTTPSClient.h"

class CurlClient : public HTTPSClient
{
public:
	virtual bool valid() const override;
	virtual HTTPSClient::Reply request(const HTTPSClient::Request &req) override;

private:
	static struct Curl
	{
		Curl();
		bool loaded;

		CURL *(*easy_init)();
		void (*easy_cleanup)(CURL *handle);
		CURLcode (*easy_setopt)(CURL *handle, CURLoption option, ...);
		CURLcode (*easy_perform)(CURL *easy_handle);
		CURLcode (*easy_getinfo)(CURL *curl, CURLINFO info, ...);

		curl_slist *(*slist_append)(curl_slist *list, const char *string);
		void (*slist_free_all)(curl_slist *list);
	} curl;
};

#endif // HTTPS_BACKEND_CURL
