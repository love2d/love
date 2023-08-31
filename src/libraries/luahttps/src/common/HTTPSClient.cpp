#include <algorithm>
#include <cctype>

#include "HTTPSClient.h"

// This may not be the order you expect, as shorter strings always compare less,
// but it's sufficient for our map
bool HTTPSClient::ci_string_less::operator()(const std::string &lhs, const std::string &rhs) const
{
	const size_t lhs_size = lhs.size();
	const size_t rhs_size = rhs.size();
	const size_t steps = std::min(lhs_size, rhs_size);

	if (lhs_size < rhs_size)
		return true;
	else if (lhs_size > rhs_size)
		return false;

	for (size_t i = 0; i < steps; ++i)
	{
		char l = std::tolower(lhs[i]);
		char r = std::tolower(rhs[i]);
		if (l < r)
			return true;
		else if (l > r)
			return false;
	}

	return false;
}

HTTPSClient::Request::Request(const std::string &url)
: url(url)
, method("GET")
{
}

