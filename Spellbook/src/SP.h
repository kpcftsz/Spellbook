#pragma once

#include <assert.h>
#include <stdint.h>

#include <string>
#include <functional>
#include <variant>
#include <iostream>
#include <type_traits>

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
	 * Core
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
				std::cerr << "No available visitor for specified type" << std::endl;
				return Object { OBJ_NIL };
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

	using LexerOutput = std::vector<std::string>;

	/*
	 * Lexer
	 */
	class Lexer
	{
	public:
		Lexer();
		~Lexer();

		void test(const std::string& input);

	private:
		ErrCode test_block(LexerOutput& output, const std::string& input);
		ErrCode tokenize_list(LexerOutput& output, const std::string& input);

	private:

	};

	/*
	 * "Parser"
	 */


	/*
	 * Environment
	 */

	/*
	 * Utils
	 */
	ErrCode load_file(const std::string& path, std::string& out);

}
