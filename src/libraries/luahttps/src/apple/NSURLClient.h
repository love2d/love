#pragma once

#include "../common/config.h"

#ifdef HTTPS_BACKEND_NSURL

#include "../common/HTTPSClient.h"

class NSURLClient : public HTTPSClient
{
public:
	virtual bool valid() const override;
	virtual HTTPSClient::Reply request(const HTTPSClient::Request &req) override;
};

#endif // HTTPS_BACKEND_NSURL
