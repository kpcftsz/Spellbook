#include "SP.h"

namespace sp
{
	Object evlist(Object& object, Environment& env)
	{
		Object result = object;

		if (object.type == OBJ_SYMBOL)
		{
			std::string sym_name = object.get_data<std::string>();
			Object* found = env.get(sym_name);
			
			if (found)
				return *found;
			else
				abort(FAILURE, "Unbound symbol");
		}
		else if (object.type == OBJ_LIST)
		{
			std::list<Object> from_list = object.get_data<std::list<Object>>();
			std::list<Object> evaluated_objs;

			for (auto&& obj: from_list)
				evaluated_objs.push_back(eval(obj, env));

			result.type = OBJ_LIST;
			result.data = evaluated_objs;
		}

		return result;
	}

	Object eval(Object& object, Environment& env)
	{
		Object result = object;

		if (object.type == OBJ_LIST)
		{
			std::list<Object> data = object.get_data<std::list<Object>>();

			if (data.size() == 0)
			{
				return object;
			}
			else
			{
				// Apply
				Object tmp_result = evlist(object, env);
				std::list<Object> tmp_result_data = tmp_result.get_data<std::list<Object>>();
				Object car = tmp_result_data.front();
				tmp_result_data.pop_front();

				if (car.type == OBJ_BUILT_IN)
					return std::get<std::function<Object(std::list<Object>)>>(car.data)(tmp_result_data);
				// ... else if (car.type == OBJ_LAMBDA) ...
				// I want to be a Lisp interpreter when I grow up!
			}

		}

		return evlist(object, env);
	}
}
