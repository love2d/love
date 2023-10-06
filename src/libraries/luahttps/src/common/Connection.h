#pragma once

#include <cstdint>
#include <string>

class Connection
{
public:
	virtual bool connect(const std::string &hostname, uint16_t port) = 0;
	virtual size_t read(char *buffer, size_t size) = 0;
	virtual size_t write(const char *buffer, size_t size) = 0;
	virtual void close() = 0;
	virtual ~Connection() {};
};
