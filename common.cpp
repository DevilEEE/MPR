#include "stdafx.h"
#include "common.h"

///safety open a file
FILE* fopen_(const char* p, const char* m)
{
	errno_t err;
	FILE* f;
	err = fopen_s(&f, p, m);
	if (err)
	{
		printf("Failed to open %s\n", p);
		exit(1);
	}
	return f;
}

void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
	std::string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}