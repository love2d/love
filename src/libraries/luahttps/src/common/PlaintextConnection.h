#pragma once

#include "Connection.h"

class PlaintextConnection : public Connection
{
public:
	PlaintextConnection();
	virtual bool connect(const std::string &hostname, uint16_t port);
	virtual size_t read(char *buffer, size_t size);
	virtual size_t write(const char *buffer, size_t size);
	virtual void close();
	virtual ~PlaintextConnection();

	int getFd() const;

private:
	int fd;
};
