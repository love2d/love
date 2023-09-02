#pragma once

#include "../common/config.h"

#ifdef HTTPS_BACKEND_SCHANNEL

#include "../common/Connection.h"
#include "../common/PlaintextConnection.h"

#include <vector>

struct _SecHandle;
using CtxtHandle = _SecHandle;

class SChannelConnection : public Connection
{
public:
	SChannelConnection();
	virtual bool connect(const std::string &hostname, uint16_t port) override;
	virtual size_t read(char *buffer, size_t size) override;
	virtual size_t write(const char *buffer, size_t size) override;
	virtual void close() override;
	virtual ~SChannelConnection();

	static bool valid();

private:
	PlaintextConnection socket;
	CtxtHandle *context;
	std::vector<char> encRecvBuffer;
	std::vector<char> decRecvBuffer;

	size_t decrypt(char *buffer, size_t size, bool recurse = true);
	void destroyContext();
};

#endif // HTTPS_BACKEND_SCHANNEL
