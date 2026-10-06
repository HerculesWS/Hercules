#include "common/hercules.h"

#include "login/login.h"
#include "common/core.h"

int main(int argc, char **argv)
{
	hserver_login hs;
	return herc_main(&hs, argc, argv);
}
