#pragma once

#include <assert.h>
#include <stdint.h>

#include <string>
#include <functional>
#include <variant>
#include <iostream>
#include <type_traits>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iostream>
#include <fstream>
#include <regex>

namespace sp
{
	using byte = uint8_t;

	enum ErrCode: byte
	{
		SUCCESS,
		FAILURE
	};

	enum ObjectType: byte
	{
		OBJ_NIL,
		OBJ_SYMBOL,
		OBJ_STRING,
		OBJ_NUMBER,
		OBJ_TRUE,
		OBJ_FALSE,
		OBJ_LIST,
		OBJ_BUILT_IN
	};



	/*
	 * -
	 * Utils
	 * -
	 */

	ErrCode load_file(const std::string& path, std::string& out);
	void abort(ErrCode status);



	/*
	 * -
	 * Core
	 * -
	 */

	class Object
	{
	public:
		template <typename T>
		void visit_data(std::function<void(T&)> callback)
		{
			std::visit(
				[&](auto&& value)
				{
					if constexpr (std::is_same_v<decltype(value), T&>)
						callback(value);
					else
						std::cerr << "Type error: " << typeid(value).name() << ", " << data.index() << std::endl;
				},
				data
			);
		}

		template <typename T>
		T* get_data_ptr()
		{
			T* res = nullptr;

			if constexpr (std::is_same_v<T, float>)
			{
				visit_data<float>([&res](float& f) {res = &f;});
			}
			else if constexpr (std::is_same_v<T, std::string>)
			{
				visit_data<std::string>([&res](std::string& s) {res = &s;});
			}
			else if constexpr (std::is_same_v<T, std::list<Object>>)
			{
				visit_data<std::list<Object>>([&res](std::list<Object>& l) {res = &l;});
			}
			else
			{
				std::cerr << "Fatal error: No available visitor for specified type" << std::endl;
				abort(FAILURE);
			}

			return res;
		}

		template <typename T>
		T get_data()
		{
			auto* ptr = get_data_ptr<T>();
			assert(ptr);

			return *ptr;
		}

	public:
		ObjectType type;
		std::variant<
			// Numbers
			float,
			// Strings/Symbols/Identifiers
			std::string,
			// Lists/Functions
			std::list<Object>,
			// Built-in functions
			std::function<Object(std::list<Object>)>
		> data;

	};



	/*
	 * -
	 * Lexer
	 * -
	 */

	class TokenGroup: public std::pair<const char*, int>
	{
	public:
		constexpr TokenGroup(const char* regex, int capture_group_id):
			std::pair<const char*, int>(regex, capture_group_id)
		{
		}

		inline constexpr operator const char*() const
		{
			return first;
		}

		inline constexpr operator int() const
		{
			return second;
		}

	};

	inline static constexpr TokenGroup GROUP_BLOCK      = {"([A-Za-z0-9_\\-]+)\\s\\{([^{}]+)\\}", -1};
	inline static constexpr TokenGroup GROUP_WHITESPACE = {"[\\s,]+", 0};
	inline static constexpr TokenGroup GROUP_NUMBERS    = {"([+-]?([0-9]+([.][0-9]*)?|[.][0-9]+))", 1};
	inline static constexpr TokenGroup GROUP_SYMBOLS    = {"(([A-Za-z\\-_]+)([0-9]+)?)", 4};
	inline static constexpr TokenGroup GROUP_STRINGS    = {"(\"(?:[^\"\\\\]|\\\\.)*\")", 7};
	inline static constexpr TokenGroup GROUP_LISTS      = {"(\\[(.*)\\])", 9};

	using LexerOutput = std::vector<std::string>;

	class Lexer
	{
	public:
		Lexer();
		~Lexer();

		void test(const std::string& input);

	private:
		ErrCode test_block(LexerOutput& output, const std::string& input);
		ErrCode tokenize_list(LexerOutput& output, const std::string& input, int ident = 0);

		void split(LexerOutput& output, const std::string& input, const std::string& delimiters);
		
		template <typename Arg>
		std::ostream& build_tokenizer_regex(std::ostream& o, Arg&& arg)
		{
			return o << static_cast<std::string>(std::forward<Arg>(arg));
		}

		template <typename Arg, typename... Args>
		std::ostream& build_tokenizer_regex(std::ostream& o, Arg&& arg, Args&&... args)
		{
			o << static_cast<std::string>(std::forward<Arg>(arg)) << "|";

			return build_tokenizer_regex(o, std::forward<Args>(args)...);
		}

	};
	


	/*
	 * -
	 * Parser
	 * -
	 */
	 // ...
	


	/*
	 * -
	 * Environment
	 * -
	 */
	// ...
	


}
