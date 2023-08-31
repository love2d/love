#pragma once

#include "../common/config.h"

#ifdef HTTPS_BACKEND_WININET

#include "../common/HTTPSClient.h"

class WinINetClient: public HTTPSClient
{
public:
	bool valid() const override;
	HTTPSClient::Reply request(const HTTPSClient::Request &req) override;
};

#endif // HTTPS_BACKEND_WININET
