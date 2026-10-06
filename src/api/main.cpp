#include "common/hercules.h"

#include "api/api.h"
#include "common/core.h"

int main(int argc, char **argv)
{
	hserver_api hs;
	return herc_main(&hs, argc, argv);
}
