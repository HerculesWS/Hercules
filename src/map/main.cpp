#include "common/hercules.h"

#include "map/map.h"
#include "common/core.h"

int main(int argc, char **argv)
{
	hserver_map hs;
	return herc_main(&hs, argc, argv);
}
