#include "SP.h"

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

		std::cout << "Scene : " << blocks[0] << std::endl;
		std::cout << "Body  : " << blocks[1] << std::endl;

		std::cout << "----------\n";

		std::regex dialogue_regex(GROUP_DIALOGUE);
		std::smatch dialogue_match;
		while (std::regex_search(blocks[1], dialogue_match, dialogue_regex))
		{
			std::string name = dialogue_match.str((GROUP_DIALOGUE >> 8) & 0xFF);
			std::string message = dialogue_match.str((GROUP_DIALOGUE) & 0xFF);
			std::string statement = "\nSay \"" + name + "\" \"" + normalize_paragraph(message) + "\"\n";

			blocks[1] = std::string(dialogue_match.prefix()) + statement + std::string(dialogue_match.suffix());
		}

		LexerOutput lines;
		split(lines, blocks[1], "\n");

		LexerOutput object_strs;
		for (auto& l : lines)
		{
			if (l.empty() || std::regex_search(l, std::regex("^\\s+$")))
				continue;
			tokenize_list(object_strs, l);
			std::cout << "--------\n";
		}
	}

	//

	ErrCode Lexer::test_block(LexerOutput& output, const std::string& input)
	{
		try
		{
			std::regex block_regex(GROUP_BLOCK);
			for (auto it = std::sregex_iterator(input.begin(), input.end(), block_regex); it != std::sregex_iterator(); it++)
			{
				std::smatch match = *it;

				// The output will be an interleaved list of strings, e.g.
				// [Name, Body, Name, Body, ...]
				output.push_back(match.str((GROUP_BLOCK >> 8) & 0xFF));
				output.push_back(match.str((GROUP_BLOCK) & 0xFF));
			}
		}
		catch (std::regex_error& e)
		{
			std::cerr << e.what() << std::endl;

			return FAILURE;
		}

		return SUCCESS;
	}

	ErrCode Lexer::tokenize_list(LexerOutput& output, const std::string& input, int ident)
	{
		try
		{
			std::ostringstream regex_stream;
			build_tokenizer_regex(
				regex_stream,
				GROUP_WHITESPACE,
				GROUP_NUMBERS,
				GROUP_SYMBOLS,
				GROUP_STRINGS,
				GROUP_LISTS
			);

			std::regex tokenization_regex(regex_stream.str());

			for (auto it = std::sregex_iterator(input.begin(), input.end(), tokenization_regex); it != std::sregex_iterator(); it++)
			{
				std::smatch match = *it;
				for (int i = 0; i < match.size(); i++)
				{
					switch (i)
					{
					case GROUP_SYMBOLS:
						if (!match.str(i).empty())
							std::cout << std::string(ident*4, ' ') << "Symbol: " + match.str(i) + "\n";
						break;
					case GROUP_NUMBERS:
						if (!match.str(i).empty())
							std::cout << std::string(ident * 4, ' ') << "Number: " + match.str(i) + "\n";
						break;
					case GROUP_LISTS:
						if (!match.str(i).empty())
						{
							std::cout << std::string(ident * 4, ' ') << "List: {\n";
							LexerOutput out;
							tokenize_list(out, match.str(i), ++ident);
							std::cout << std::string((ident-- - 1) * 4, ' ') << "}\n";
						}
						break;
					case GROUP_STRINGS:
						if (!match.str(i).empty())
						{
							std::string str = match.str(i);
							std::string trimmed = str.substr(1, str.size() - 2); // For use later
							std::cout << std::string(ident * 4, ' ') << "String: " + str + "\n";
						}
						break;
					}
				}

				output.push_back(match.str());

			}
		}
		catch (std::regex_error& e)
		{
			std::cerr << e.what() << std::endl;

			return FAILURE;
		}

		return SUCCESS;
	}

	void Lexer::split(LexerOutput& output, const std::string& input, const std::string& delimiters)
	{
		std::string::size_type last_pos = input.find_first_not_of(delimiters, 0);
		std::string::size_type pos = input.find_first_of(delimiters, last_pos);

		while (std::string::npos != pos || std::string::npos != last_pos)
		{
			output.push_back(input.substr(last_pos, pos - last_pos));
			last_pos = input.find_first_not_of(delimiters, pos);
			pos = input.find_first_of(delimiters, last_pos);
		}
	}

	std::string Lexer::normalize_paragraph(std::string text)
	{
		text = std::regex_replace(text, std::regex("^[\\s]+|[\\s]+$"), "");
		text = std::regex_replace(text, std::regex("[\\s]+"), " ");

		return text;
	}
}
