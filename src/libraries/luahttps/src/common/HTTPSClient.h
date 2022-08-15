#pragma once

#include <cstdint>
#include <string>
#include <map>

class HTTPSClient
{
public:
	struct ci_string_less
	{
		bool operator()(const std::string &lhs, const std::string &rhs) const;
	};
	using header_map = std::map<std::string, std::string, ci_string_less>;

	struct Request
	{
		Request(const std::string &url);

		header_map headers;
		std::string url;
		std::string postdata;
		std::string method;
	};

	struct Reply
	{
		header_map headers;
		std::string body;
		int responseCode;
	};

	virtual ~HTTPSClient() {}
	virtual bool valid() const = 0;
	virtual Reply request(const Request &req) = 0;
};
