#include "WinINetClient.h"

#ifdef HTTPS_BACKEND_WININET

#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <vector>

#include <Windows.h>
#include <wininet.h>

#include "../common/HTTPRequest.h"

class LazyHInternetLoader final
{
public:
	LazyHInternetLoader(): hInternet(nullptr) { }
	~LazyHInternetLoader()
	{
		if (hInternet)
			InternetCloseHandle(hInternet);
	}

	HINTERNET getInstance()
	{
		if (!init)
		{
			hInternet = InternetOpenA("", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
			if (hInternet)
			{
				// Try to enable HTTP2
				DWORD httpProtocol = HTTP_PROTOCOL_FLAG_HTTP2;
				InternetSetOptionA(hInternet, INTERNET_OPTION_ENABLE_HTTP_PROTOCOL, &httpProtocol, sizeof(DWORD));
				SetLastError(0); // If it errors, ignore.
			}
		}

		return hInternet;
	}

private:
	bool init;
	HINTERNET hInternet;
};

static thread_local LazyHInternetLoader hInternetCache;

bool WinINetClient::valid() const
{
	// Allow disablement of WinINet backend.
	const char *disabler = getenv("LUAHTTPS_DISABLE_WININET");
	if (disabler && strcmp(disabler, "1") == 0)
		return false;

	return hInternetCache.getInstance() != nullptr;
}

HTTPSClient::Reply WinINetClient::request(const HTTPSClient::Request &req)
{
	Reply reply;
	reply.responseCode = 0;

	// Parse URL
	auto parsedUrl = HTTPRequest::parseUrl(req.url);

	// Default flags
	DWORD inetFlags =
		INTERNET_FLAG_NO_AUTH |
		INTERNET_FLAG_NO_CACHE_WRITE |
		INTERNET_FLAG_NO_COOKIES |
		INTERNET_FLAG_NO_UI;

	if (parsedUrl.schema == "https")
		inetFlags |= INTERNET_FLAG_SECURE;
	else if (parsedUrl.schema != "http")
		return reply;

	// Keep-Alive
	auto connectHeader = req.headers.find("Connection");
	auto headerEnd = req.headers.end();
	if ((connectHeader != headerEnd && connectHeader->second != "close") || connectHeader == headerEnd)
		inetFlags |= INTERNET_FLAG_KEEP_CONNECTION;

	// Open internet
	HINTERNET hInternet = hInternetCache.getInstance();
	if (hInternet == nullptr)
		return reply;

	// Connect
	HINTERNET hConnect = InternetConnectA(
		hInternet,
		parsedUrl.hostname.c_str(),
		parsedUrl.port,
		nullptr, nullptr,
		INTERNET_SERVICE_HTTP,
		INTERNET_FLAG_EXISTING_CONNECT,
		(DWORD_PTR) this
	);
	if (!hConnect)
		return reply;

	std::string httpMethod = req.method;
	std::transform(
		httpMethod.begin(),
		httpMethod.end(),
		httpMethod.begin(),
		[](char c) {return (char)toupper((unsigned char) c); }
	);

	// Open HTTP request
	HINTERNET hHTTP = HttpOpenRequestA(
		hConnect,
		httpMethod.c_str(),
		parsedUrl.query.c_str(),
		nullptr,
		nullptr,
		nullptr,
		inetFlags,
		(DWORD_PTR) this
	);
	if (!hHTTP)
	{
		InternetCloseHandle(hConnect);
		return reply;
	}

	// Send additional headers
	HttpAddRequestHeadersA(hHTTP, "User-Agent:", 0, HTTP_ADDREQ_FLAG_REPLACE);
	for (const auto &header: req.headers)
	{
		std::string headerString = header.first + ": " + header.second + "\r\n";
		HttpAddRequestHeadersA(hHTTP, headerString.c_str(), headerString.length(), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
	}

	// POST data
	const char *postData = nullptr;
	if (req.postdata.length() > 0 && (httpMethod != "GET" && httpMethod != "HEAD"))
	{
		char temp[48];
		int len = sprintf(temp, "Content-Length: %u\r\n", (unsigned int) req.postdata.length());
		postData = req.postdata.c_str();

		HttpAddRequestHeadersA(hHTTP, temp, len, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
	}

	// Send away!
	BOOL result = HttpSendRequestA(hHTTP, nullptr, 0, (void *) postData, (DWORD) req.postdata.length());
	if (!result)
	{
		InternetCloseHandle(hHTTP);
		InternetCloseHandle(hConnect);
		return reply;
	}

	DWORD bufferLength = sizeof(DWORD);
	DWORD headerCounter = 0;

	// Status code
	DWORD statusCode = 0;
	if (!HttpQueryInfoA(hHTTP, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &bufferLength, &headerCounter))
	{
		InternetCloseHandle(hHTTP);
		InternetCloseHandle(hConnect);
		return reply;
	}

	// Query headers
	std::vector<char> responseHeaders;
	bufferLength = 0;
	HttpQueryInfoA(hHTTP, HTTP_QUERY_RAW_HEADERS, responseHeaders.data(), &bufferLength, &headerCounter);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		InternetCloseHandle(hHTTP);
		InternetCloseHandle(hConnect);
		return reply;
	}

	responseHeaders.resize(bufferLength);
	if (!HttpQueryInfoA(hHTTP, HTTP_QUERY_RAW_HEADERS, responseHeaders.data(), &bufferLength, &headerCounter))
	{
		InternetCloseHandle(hHTTP);
		InternetCloseHandle(hConnect);
		return reply;
	}

	for (const char *headerData = responseHeaders.data(); *headerData; headerData += strlen(headerData) + 1)
	{
		const char *value = strchr(headerData, ':');
		if (value)
		{
			ptrdiff_t keyLen = (ptrdiff_t) (value - headerData);
			reply.headers[std::string(headerData, keyLen)] = value + 2; // +2, colon and 1 space character.
		}
	}
	responseHeaders.resize(1);

	// Read response
	std::stringstream responseData;
	for (;;)
	{
		constexpr DWORD BUFFER_SIZE = 4096;
		char buffer[BUFFER_SIZE];
		DWORD readed = 0;

		BOOL ret = InternetQueryDataAvailable(hHTTP, &readed, 0, 0);
		if (!ret || readed == 0)
			break;

		if (!InternetReadFile(hHTTP, buffer, BUFFER_SIZE, &readed))
			break;

		responseData.write(buffer, readed);
	}

	reply.body = responseData.str();
	reply.responseCode = statusCode;

	InternetCloseHandle(hHTTP);
	InternetCloseHandle(hConnect);
	return reply;
}

#endif // HTTPS_BACKEND_WININET
