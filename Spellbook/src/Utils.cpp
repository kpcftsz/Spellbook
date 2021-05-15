#include "SP.h"

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

	void abort(ErrCode status, const std::string& message)
	{
		std::cerr << message << std::endl;
		exit(status);
	}

	std::string print(Object& object, bool in_list)
	{
		std::string printed;

		switch (object.type)
		{
		case OBJ_NIL:
			printed = "<nil>";
			break;
		case OBJ_SYMBOL:
			printed = object.get_data<std::string>();
			break;
		case OBJ_STRING:
			{
				std::string quote = in_list ? "\"" : "";
				printed = quote + object.get_data<std::string>() + quote;
			}
			break;
		case OBJ_NUMBER:
			printed = std::to_string(object.get_data<float>());
			break;
		case OBJ_TRUE:
			printed = "<true>";
			break;
		case OBJ_FALSE:
			printed = "<false>";
			break;
		case OBJ_LIST:
			{
				std::list<Object> list = object.get_data<std::list<Object>>();
				printed += "[";
				int i = 0;
				for (auto&& item: list) {
					printed += print(item, true) + (i != list.size() - 1 ? ", " : "");
					i++;
				}
				printed += "]";
			}
			break;
		case OBJ_BUILT_IN:
			printed = "<built-in>";
			break;
		}

		return printed;
	}
}
