#include <stdlib.h>

#include <string>

#include "SP.h"

int main(int argc, char* argv[])
{
	std::string src;
	if (sp::load_file("./demo.sp", src))
		return EXIT_FAILURE;

	sp::Lexer lexer;
	lexer.test(src);

	return EXIT_SUCCESS;
}
