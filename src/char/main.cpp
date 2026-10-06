#include "common/hercules.h"

#include "char/char.h"
#include "common/core.h"

int main(int argc, char **argv)
{
	hserver_char hs;
	return herc_main(&hs, argc, argv);
}
