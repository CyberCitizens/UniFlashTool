#include "strings_on_steroids.hpp"

namespace std
{
	deque<string> strsplit(const string &str, char delimiter)
	{
		deque<string> tokens;
		std::string token;
		std::istringstream tokenStream(str);
		while (std::getline(tokenStream, token, delimiter))
		{
			tokens.push_back(token);
		}
		return tokens;
	}
}