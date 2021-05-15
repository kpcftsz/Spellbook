#include "SP.h"

namespace
{
	// TODO: Move this somewhere else, the dialogue macro might need this later
	std::string unescape(std::string input)
	{
		size_t pos = input.find("\\");

		while (pos != std::string::npos)
		{
			if (pos + 1 < input.length())
			{
				switch (input[pos + 1])
				{
				case 'n': input.replace(pos, 2, "\n"); break;
				case 'r': input.replace(pos, 2, "\r"); break;
				case 't': input.replace(pos, 2, "\t"); break;
				}
			}

			pos = input.find("\\", pos + 1);
		}

		return input;
	}

	sp::Object f_print()
	{
		using namespace sp;

		Object obj{OBJ_BUILT_IN};
		obj.data = [](std::list<Object> args) -> Object
		{
			std::string prn;

			for (auto&& arg: args)
				prn += print(arg);

			std::cout << unescape(prn) << std::endl;

			return Object{OBJ_VOID};
		};

		return obj;
	};

	sp::Object f_exit()
	{
		using namespace sp;

		Object obj{OBJ_BUILT_IN};
		obj.data = [](std::list<Object> args) -> Object
		{
			exit(0);
		};

		return obj;
	};
}

namespace sp
{
	Environment::Environment(Object& source)
	{
		std::list<Object> list = source.get_data<std::list<Object>>();

		for (auto&& binding_obj: list)
		{
			std::list<Object> binding_list = binding_obj.get_data<std::list<Object>>();
			Object sym = binding_list.front();
			std::string sym_str = sym.get_data<std::string>();
			Object binding = binding_list.back();

			m_sym_table.emplace(sym_str, binding);
		}
	}

	void Environment::set(const std::string& sym_key, Object obj_value)
	{
		m_sym_table.emplace(sym_key, obj_value);
	}

	Environment* Environment::find(const std::string& sym_key)
	{
		if (m_sym_table.count(sym_key) != 0)
			return this;

		return nullptr;
	}

	Object* Environment::get(const std::string& sym_key)
	{
		Environment* env = find(sym_key);

		return env ? &env->m_sym_table.at(sym_key) : nullptr;
	}

	void Environment::init()
	{
		// Typical built-in functions
		set("Print", f_print());
		set("Exit", f_exit());

		// Testing...
		Object pi{OBJ_NUMBER};
		pi.data = 3.14159265358979f;
		set("PI", pi);
	}

	Object Environment::as_object()
	{
		std::list<Object> list = to_list(*this);

		Object as_obj{OBJ_ENVIRONMENT};
		as_obj.data = list;

		return as_obj;
	}

	//

	std::list<Object> Environment::to_list(const Environment& env)
	{
		std::list<Object> list;

		for (const auto&[sym, binding] : m_sym_table)
		{
			Object symbol{OBJ_SYMBOL};
			symbol.data = sym;

			std::list<Object> to_add_list;
			to_add_list.push_back(symbol);
			to_add_list.push_back(binding);

			Object to_add{OBJ_LIST};
			to_add.data = to_add_list;

			list.push_back(to_add);
		}

		return list;
	}
}
