#pragma once

#include <vector>
#include <string>
#include <algorithm>

class Helpers {
public:
	static std::vector<std::string> splitString(std::string str, char delimiter = ' ') {
		std::string::iterator new_end = std::unique(str.begin(), str.end(), [](const char &x, const char &y) {
			return x == y and x == ' ';
		});

		str.erase(new_end, str.end());

		while (str[str.length() - 1] == delimiter) {
			str.pop_back();
		}

		std::vector<std::string> splits;

		size_t i = 0;
		size_t pos = str.find(delimiter);

		while (pos != std::string::npos) {
			splits.push_back(str.substr(i, pos - i));

			i = pos + 1;
			pos = str.find(delimiter, i);
		}

		splits.push_back(str.substr(i, std::min(pos, str.length()) - i + 1));

		return splits;
	}
};