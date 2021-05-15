#include "SP.h"

int main(int argc, char* argv[])
{
	std::string src;
	if (sp::load_file("./demo.sp", src))
		return EXIT_FAILURE;

	std::vector<sp::SceneBlock> blocks;

	sp::Environment env;
	env.init();

	sp::Lexer lexer;
	lexer.process(blocks, src);

	for (int i = 0; i < blocks.size(); i++)
	{
		std::cout << "Now displaying scene \"" << blocks[i].name << "\":" << std::endl;
		for (auto& call_obj : blocks[i].calls)
		{
			sp::Object evaluated = sp::eval(call_obj, env);
			std::cout << sp::print(evaluated) << std::endl;
		}
	}

	return EXIT_SUCCESS;
}
