#include "config.h"
#include <cstring>
#ifndef HTTPS_USE_WINSOCK
#	include <netdb.h>
#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#else
#	include <winsock2.h>
#	include <ws2tcpip.h>
#endif // HTTPS_USE_WINSOCK

#include "PlaintextConnection.h"

#ifdef HTTPS_USE_WINSOCK
	static void close(int fd)
	{
		closesocket(fd);
	}
#endif // HTTPS_USE_WINSOCK

PlaintextConnection::PlaintextConnection()
	: fd(-1)
{
#ifdef HTTPS_USE_WINSOCK
	static bool wsaInit = false;
	if (!wsaInit)
	{
		WSADATA data;
		WSAStartup(MAKEWORD(2, 2), &data);
	}
#endif // HTTPS_USE_WINSOCK
}

PlaintextConnection::~PlaintextConnection()
{
	if (fd != -1)
		::close(fd);
}

bool PlaintextConnection::connect(const std::string &hostname, uint16_t port)
{
	addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_flags = hints.ai_protocol = 0;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	addrinfo *addrs = nullptr;
	std::string portString = std::to_string(port);
	getaddrinfo(hostname.c_str(), portString.c_str(), &hints, &addrs);

	// Try all addresses returned
	bool connected = false;
	for (addrinfo *addr = addrs; !connected && addr; addr = addr->ai_next)
	{
		fd = socket(addr->ai_family, SOCK_STREAM, 0);
		connected = ::connect(fd, addr->ai_addr, addr->ai_addrlen) == 0;
		if (!connected)
			::close(fd);
	}

	freeaddrinfo(addrs);

	if (!connected)
	{
		fd = -1;
		return false;
	}

	return true;
}

size_t PlaintextConnection::read(char *buffer, size_t size)
{
	auto read = ::recv(fd, buffer, size, 0);
	if (read < 0)
		read = 0;
	return static_cast<size_t>(read);
}

size_t PlaintextConnection::write(const char *buffer, size_t size)
{
	auto written = ::send(fd, buffer, size, 0);
	if (written < 0)
		written = 0;
	return static_cast<size_t>(written);
}

void PlaintextConnection::close()
{
	::close(fd);
	fd = -1;
}

int PlaintextConnection::getFd() const
{
	return fd;
}
