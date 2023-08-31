#include <sstream>
#include <string>
#include <memory>
#include <limits>
#include <stdexcept>

#include "HTTPRequest.h"
#include "PlaintextConnection.h"

HTTPRequest::HTTPRequest(ConnectionFactory factory)
	: factory(factory)
{
}

HTTPSClient::Reply HTTPRequest::request(const HTTPSClient::Request &req)
{
	HTTPSClient::Reply reply;
	reply.responseCode = 0;

	auto info = parseUrl(req.url);
	if (!info.valid)
		return reply;

	std::unique_ptr<Connection> conn;
	if (info.schema == "http")
		conn.reset(new PlaintextConnection());
	else if (info.schema == "https")
		conn.reset(factory());
	else
		throw std::runtime_error("Unknown url schema");

	if (!conn->connect(info.hostname, info.port))
		return reply;

	// Build the request
	{
		std::stringstream request;
		std::string method = req.method;
		bool hasData = req.postdata.length() > 0;

		if (method.length() == 0)
			method = hasData ? "POST" : "GET";

		request << method << " " << info.query << " HTTP/1.1\r\n";

		for (auto &header : req.headers)
			request << header.first << ": " << header.second << "\r\n";

		request << "Connection: Close\r\n";

		request << "Host: " << info.hostname << "\r\n";

		if (hasData)
			request << "Content-Length: " << req.postdata.size() << "\r\n";

		request << "\r\n";

		if (hasData)
			request << req.postdata;

		// Send it
		std::string requestData = request.str();
		conn->write(requestData.c_str(), requestData.size());
	}

	// Now receive the reply
	std::stringstream response;
	{
		char buffer[8192];

		while (true)
		{
			size_t read = conn->read(buffer, sizeof(buffer));
			response.write(buffer, read);
			if (read == 0)
				break;
		}

		conn->close();
	}

	reply.responseCode = 500;
	// And parse it
	{
		std::string protocol;
		response >> protocol;
		if (protocol != "HTTP/1.1")
			return reply;

		response >> reply.responseCode;
		response.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		for (std::string line; getline(response, line, '\n') && line != "\r"; )
		{
			auto sep = line.find(':');
			reply.headers[line.substr(0, sep)] = line.substr(sep+1, line.size()-sep-1);
		}

		auto begin = std::istreambuf_iterator<char>(response);
		auto end = std::istreambuf_iterator<char>();
		reply.body = std::string(begin, end);
	}

	return reply;
}

HTTPRequest::DissectedURL HTTPRequest::parseUrl(const std::string &url)
{
	DissectedURL dis;
	dis.valid = false;

	// Schema
	auto schemaStart = 0;
	auto schemaEnd = url.find("://");
	dis.schema = url.substr(schemaStart, schemaEnd-schemaStart);

	// Auth+Hostname+Port
	auto connStart = schemaEnd+3;
	auto connEnd = url.find('/', connStart);
	if (connEnd == std::string::npos)
		connEnd = url.size();

	// TODO: Auth
	if (url.find("@", connStart, connEnd-connStart) != std::string::npos)
		return dis;

	// Port
	auto portStart = url.find(':', connStart);
	auto portEnd = connEnd;
	if (portStart == std::string::npos || portStart > portEnd)
	{
		dis.port = dis.schema == "http" ? 80 : 443;
		portStart = portEnd;
	}
	else
		dis.port = std::stoi(url.substr(portStart+1, portEnd-portStart-1));

	// Hostname
	auto hostnameStart = connStart;
	auto hostnameEnd = portStart;
	dis.hostname = url.substr(hostnameStart, hostnameEnd-hostnameStart);

	// And the query
	dis.query = url.substr(connEnd);
	if (dis.query.size() == 0)
		dis.query = "/";

	dis.valid = true;
	
	return dis;
}
