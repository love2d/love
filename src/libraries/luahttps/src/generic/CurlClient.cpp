#include "CurlClient.h"

#ifdef HTTPS_BACKEND_CURL

#include <dlfcn.h>
#include <stdexcept>
#include <sstream>
#include <vector>

CurlClient::Curl::Curl()
{
	void *handle = dlopen("libcurl.so", RTLD_LAZY);
	if (!handle)
	{
		loaded = false;
		return;
	}

	void (*global_init)() = (void(*)()) dlsym(handle, "curl_global_init");
	easy_init = (CURL*(*)()) dlsym(handle, "curl_easy_init");
	easy_cleanup = (void(*)(CURL*)) dlsym(handle, "curl_easy_cleanup");
	easy_setopt = (CURLcode(*)(CURL*,CURLoption,...)) dlsym(handle, "curl_easy_setopt");
	easy_perform = (CURLcode(*)(CURL*)) dlsym(handle, "curl_easy_perform");
	easy_getinfo = (CURLcode(*)(CURL*,CURLINFO,...)) dlsym(handle, "curl_easy_getinfo");
	slist_append = (curl_slist*(*)(curl_slist*,const char*)) dlsym(handle, "curl_slist_append");
	slist_free_all = (void(*)(curl_slist*)) dlsym(handle, "curl_slist_free_all");

	loaded = (global_init && easy_init && easy_cleanup && easy_setopt && easy_perform && easy_getinfo && slist_append && slist_free_all);

	if (!loaded)
		return;

	global_init();
}

static size_t stringstreamWriter(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	std::stringstream *ss = (std::stringstream*) userdata;
	size_t count = size*nmemb;
	ss->write(ptr, count);
	return count;
}

static size_t headerWriter(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	std::map<std::string, std::string> &headers = *((std::map<std::string,std::string>*) userdata);
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
	reply.responseCode = 400;

	CURL *handle = curl.easy_init();
	if (!handle)
		throw std::runtime_error("Could not create curl request");

	curl.easy_setopt(handle, CURLOPT_URL, req.url.c_str());
	curl.easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);

	if (req.method == "PUT")
		curl.easy_setopt(handle, CURLOPT_PUT, 1L);
	else if (req.method == "POST")
		curl.easy_setopt(handle, CURLOPT_POST, 1L);
	else
		curl.easy_setopt(handle, CURLOPT_CUSTOMREQUEST, req.method.c_str());

	if (req.postdata.size() > 0 && (req.method != "GET" && req.method != "HEAD"))
	{
		curl.easy_setopt(handle, CURLOPT_POSTFIELDS, req.postdata.c_str());
		curl.easy_setopt(handle, CURLOPT_POSTFIELDSIZE, req.postdata.size());
	}

	// Curl doesn't copy memory, keep the strings around
	std::vector<std::string> lines;
	for (auto &header : req.headers)
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
