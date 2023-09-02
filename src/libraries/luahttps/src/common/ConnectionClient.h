#pragma once

#include "HTTPSClient.h"
#include "HTTPRequest.h"
#include "Connection.h"

template<typename Connection>
class ConnectionClient : public HTTPSClient
{
public:
	virtual bool valid() const override;
	virtual HTTPSClient::Reply request(const HTTPSClient::Request &req) override;

private:
	static Connection *factory();
};

template<typename Connection>
bool ConnectionClient<Connection>::valid() const
{
	return Connection::valid();
}

template<typename Connection>
Connection *ConnectionClient<Connection>::factory()
{
	return new Connection();
}

template<typename Connection>
HTTPSClient::Reply ConnectionClient<Connection>::request(const HTTPSClient::Request &req)
{
	HTTPRequest request(factory);
	return request.request(req);
}
