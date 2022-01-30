#pragma once

#include <functional>

#include "HTTPSClient.h"
#include "Connection.h"

class HTTPRequest
{
public:
	typedef std::function<Connection *()> ConnectionFactory;
	HTTPRequest(ConnectionFactory factory);

	HTTPSClient::Reply request(const HTTPSClient::Request &req);

private:
	ConnectionFactory factory;

	struct DissectedURL
	{
		bool valid;
		std::string schema;
		std::string hostname;
		uint16_t port;
		std::string query;
		// TODO: Auth?
	};

	DissectedURL parseUrl(const std::string &url);
};
