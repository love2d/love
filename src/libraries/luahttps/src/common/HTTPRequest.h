#pragma once

#include <functional>

#include "HTTPSClient.h"
#include "Connection.h"

class HTTPRequest
{
public:
	struct DissectedURL
	{
		bool valid;
		std::string schema;
		std::string hostname;
		uint16_t port;
		std::string query;
		// TODO: Auth?
	};
	typedef std::function<Connection *()> ConnectionFactory;

	HTTPRequest(ConnectionFactory factory);

	HTTPSClient::Reply request(const HTTPSClient::Request &req);

	static DissectedURL parseUrl(const std::string &url);

private:
	ConnectionFactory factory;
};
