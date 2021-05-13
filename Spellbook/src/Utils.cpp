#include "SP.h"

#include <fstream>
#include <iostream>
#include <exception>

namespace sp
{
	ErrCode load_file(const std::string& path, std::string& out)
	{
		std::ifstream file(path);

		if (!file)
		{
			std::cerr << "Failed to load file: " << path << std::endl;

			return FAILURE;
		}

		out = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		return SUCCESS;
	}

	void abort(ErrCode status)
	{
		exit(status);
	}
}
