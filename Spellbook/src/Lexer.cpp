#include "SP.h"

#include <regex>

namespace sp
{
	Lexer::Lexer()
	{

	}

	Lexer::~Lexer()
	{

	}

	void Lexer::test(const std::string& input)
	{
		LexerOutput blocks;
		auto stat = test_block(blocks, input);
		if (stat == FAILURE || blocks.empty())
			return;

		std::cout << "Name: " << blocks[0] << std::endl;
		std::cout << "Contents: " << blocks[1] << std::endl;

		std::cout << "----------\n";

		LexerOutput object_strs;
		tokenize_list(object_strs, blocks[1]);

	}

	//

	ErrCode Lexer::test_block(LexerOutput& output, const std::string& input)
	{
		using namespace std::regex_constants;

		try
		{
			std::regex block_regex("([A-Za-z0-9_\\-]+)\\s\\{([^{}]+)\\}");
			for (auto it = std::sregex_iterator(input.begin(), input.end(), block_regex); it != std::sregex_iterator(); it++)
			{
				std::smatch match = *it;

				// This makes it so that even elements in the output list
				// are names of the symbol, odd elements are the bodies.
				// Could get more elegant with this, idgaf honestly
				output.push_back(match.str(1));
				output.push_back(match.str(2));
			}
		}
		catch (std::regex_error& e)
		{
			std::cerr << e.what() << std::endl;

			return FAILURE;
		}

		return SUCCESS;
	}

	ErrCode Lexer::tokenize_list(LexerOutput& output, const std::string& input)
	{
		using namespace std::regex_constants;

		try
		{
			// std::regex tokenization_regex("[\\s,]+|([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+))|([A-Za-z\\-_]+)([0-9]+)?|(\\\".*\\\")|(\\[(.*)\\])");
			std::regex tokenization_regex("[\\s,]+|([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+))|(([A-Za-z\\-_]+)([0-9]+)?)|(\"(?:[^\"\\\\]|\\\\.)*\")|(\\[(.*)\\])");
			for (auto it = std::sregex_iterator(input.begin(), input.end(), tokenization_regex); it != std::sregex_iterator(); it++)
			{
				std::smatch match = *it;
				for (int i = 0; i < match.size(); i++)
				{
					switch (i)
					{
					case 4:
						// Symbol
						if (!match.str(i).empty())
							std::cout << "Symbol: " + match.str(i) + ", ";
						break;
					case 1:
						// Number
						if (!match.str(i).empty())
							std::cout << "Number: " + match.str(i) + ", ";
						break;
					case 9:
						// List
						if (!match.str(i).empty())
						{
							std::cout << "List: {\n\t" << std::endl;

							LexerOutput out;
							tokenize_list(out, match.str(i));

							std::cout << "}\n";

							//std::cout << "List: " + match.str(i) + ", ";
						}
						break;
					case 7:
						// String
						if (!match.str(i).empty())
						{
							std::string str = match.str(i);
							std::string trimmed = str.substr(1, str.size() - 2);
							std::cout << "String: " + trimmed + ", ";
						}
						break;
					}
				}

				output.push_back(match.str());
				std::cout << std::endl;

			}
		}
		catch (std::regex_error& e)
		{
			std::cerr << e.what() << std::endl;

			return FAILURE;
		}

		return SUCCESS;
	}
}
