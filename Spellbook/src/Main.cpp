#include "SP.h"

int main(int argc, char* argv[])
{
	std::string src;
	if (sp::load_file("./demo.sp", src))
		return EXIT_FAILURE;

	sp::Lexer lexer;
	sp::ListProc lisp(lexer);

	// lexer.process(src);

	bool running = true;
	std::string input;
	while (running)
	{
		std::cout << "sp> ";
		std::getline(std::cin, input);
		if (input.empty())
			continue;
		lisp.clear();
		lisp.tokenize(input);
		sp::Object o = lisp.read();
		if (o.type == sp::OBJ_LIST)
		{
			if (!o.get_data<std::list<sp::Object>>().empty())
			{
				if (o.get_data<std::list<sp::Object>>().front().get_data<std::string>() == "exit")
					std::exit(0);
				if (o.get_data<std::list<sp::Object>>().front().get_data<std::string>() == "load" &&
					o.get_data<std::list<sp::Object>>().size() == 2)
				{
					sp::load_file(o.get_data<std::list<sp::Object>>().back().get_data<std::string>(), src);
					lexer.process(src);
				}
			}
		}
		std::cout << print(o) << std::endl;
	}

	return EXIT_SUCCESS;
}
