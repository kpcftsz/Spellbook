#include "SP.h"

namespace sp
{
	Lexer::Lexer()
	{
	}

	Lexer::~Lexer()
	{
	}

	void Lexer::process(const std::string& input)
	{
		std::vector<std::string> blocks;
		auto stat = process_scene_blocks(blocks, input);
		if (stat == FAILURE || blocks.empty())
			return;

		std::regex dialogue_regex(GROUP_DIALOGUE);
		std::smatch dialogue_match;
		while (std::regex_search(blocks[1], dialogue_match, dialogue_regex))
		{
			std::string name = dialogue_match.str((GROUP_DIALOGUE >> 8) & 0xFF);
			std::string message = dialogue_match.str((GROUP_DIALOGUE) & 0xFF);
			std::string statement = "\nSay \"" + name + "\" \"" + normalize_paragraph(message) + "\"\n";

			blocks[1] = std::string(dialogue_match.prefix()) + statement + std::string(dialogue_match.suffix());
		}

		std::vector<std::string> lines;
		split(lines, blocks[1], "\n");

		std::vector<std::string> object_strs;
		for (auto& l : lines)
		{
			if (l.empty() || std::regex_search(l, std::regex("^\\s+$")))
				continue;

			std::list<Object> objs;
			process_all_tokens(objs, l);

			for (auto&& obj: objs)
				std::cout << print(obj) << std::endl;
		}
	}

	ErrCode Lexer::process_scene_blocks(std::vector<std::string>& output, const std::string& input)
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

	ErrCode Lexer::process_all_tokens(std::list<Object>& output, const std::string& input)
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
					if (!match.str(i).empty())
					{
						switch (i)
						{
						case GROUP_NUMBERS:
							{
								Object obj{OBJ_NUMBER};
								obj.data = std::stof(match.str(i));
								output.push_back(obj);
							}
							break;
						case GROUP_SYMBOLS:
							{
								Object obj{OBJ_SYMBOL};
								obj.data = match.str(i);
								output.push_back(obj);
							}
							break;
						case GROUP_STRINGS:
							{
								std::string str = match.str(i);
								std::string trimmed = str.substr(1, str.size() - 2);
								Object obj{OBJ_STRING};
								obj.data = normalize_string(trimmed);
								output.push_back(obj);
							}
							break;
						case GROUP_LISTS:
							output.push_back(ListProc(*this).process_list(match.str(i)));
							break;
						}
					}
				}

				// ...
			}
		}
		catch (std::regex_error& e)
		{
			std::cerr << e.what() << std::endl;

			return FAILURE;
		}

		if (output.empty())
			output.push_back(Object{OBJ_NIL});

		return SUCCESS;
	}

	//

	void Lexer::split(std::vector<std::string>& output, const std::string& input, const std::string& delimiters)
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
		std::unordered_map<std::string, std::string> exception_codes = {
			{"^[\\s]+", ""},
			{"[\\s]+$", ""},
			{"[\\s]+", " "},
			{"\"", "\\\""},
			{"\\\\\"", "\""}
		};

		for (auto&& [original, replacement]: exception_codes)
			text = std::regex_replace(text, std::regex(original), replacement);

		return text;
	}

	std::string Lexer::normalize_string(std::string text)
	{
		std::unordered_map<std::string, std::string> escape_codes = {
			{"\\\\\"", "\""},
			{"\\\\n", "\n"}
		};

		for (auto&& [original, replacement]: escape_codes)
			text = std::regex_replace(text, std::regex(original), replacement);

		return text;
	}

	/****************************************************************/

	ListProc::ListProc(Lexer& lexer):
		m_lexer(lexer),
		m_read_index(0)
	{
	}

	Object ListProc::process_list(const std::string& input)
	{
		tokenize(input);
		return read();
	}

	Object ListProc::read()
	{
		std::string token = peek();

		Object form{OBJ_NIL};

		// TODO: Consider removing this
		if (token.empty())
			return form;

		switch (token[0])
		{
		case '[':
			std::cout << "-- Reading a list --" << std::endl;
			form = read_list(OBJ_LIST, '[', ']');
			break;
		case ']':
			std::cerr << "Unexpected ']'" << std::endl;
			abort(FAILURE);
			break;
		default:
			form = read_object();
		}

		return form;
	}

	std::vector<std::string> ListProc::tokenize(const std::string& input)
	{
		std::vector<std::string> tokens;

		// TODO: Clean up this regex
		std::regex tokenization_regex("[\\s,]*(~@|[\\[\\]\\\\\\{}()'`~^@]|\\\"(?:\\\\.|[^\\\"])*\"?|;.*|[^\\s\\[\\]\\\\\\{}('\"`,;)]*)");
		for (auto it = std::sregex_iterator(input.begin(), input.end(), tokenization_regex); it != std::sregex_iterator(); it++)
		{
			std::smatch match = *it;
			tokens.push_back(match.str(1));
		}

		m_tokens = tokens;

		return tokens;
	}

	void ListProc::clear()
	{
		m_read_index = 0;
		m_tokens.clear();
	}

	//

	Object ListProc::read_list(ObjectType type, char start, char end)
	{
		std::string token = next();

		if (token.empty())
		{
			std::cerr << "Invalid token" << std::endl;
			abort(FAILURE);
		}

		Object form{OBJ_NIL};
		Object list{OBJ_LIST};
		list.data = std::list<Object>();

		if (token[0] != start)
		{
			std::cerr << "Expected \'" << start << "\'" << std::endl;
			abort(FAILURE);
		}

		token = peek();
		while (token[0] != end)
		{
			form = read();
			list.get_data_ptr<std::list<Object>>()->push_back(form);
			token = peek();
		}

		next();

		return list;
	}

	Object ListProc::read_object()
	{
		std::list<Object> output;
		m_lexer.process_all_tokens(output, next());

		return output.back();
	}

	std::string ListProc::peek()
	{
		return m_tokens[m_read_index];
	}

	std::string ListProc::next()
	{
		return m_read_index >= m_tokens.size() ? "" : m_tokens[m_read_index++];
	}
}
