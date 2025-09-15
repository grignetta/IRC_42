#include "Server.hpp"

bool safeParseInt(const std::string &s, int &out)
{
	errno = 0;
	char *end = 0;
	long val = strtol(s.c_str(), &end, 10);

	if (*end != '\0')
		return false;
	if (errno == ERANGE || val > INT_MAX || val < INT_MIN)
		return false;
	out = static_cast<int>(val);
	return true;
}
