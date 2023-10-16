/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2023 Hercules Dev Team
 * Copyright (C) 2023 KirieZ
 *
 * Hercules is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define HERCULES_CORE

#include "common/cbasetypes.h"
#include "common/base62.h"
#include "common/core.h"
#include "common/showmsg.h"
#include "common/strlib.h"

#include <stdlib.h>

#define TEST(name, function) do { \
	ShowMessage("-------------------------------------------------------------------------------\n"); \
	ShowNotice("Testing %s...\n", (name)); \
	if (!(function)()) { \
		ShowError("Failed.\n"); \
		ShowMessage("===============================================================================\n"); \
		ShowFatalError("Failure. Aborting further tests.\n"); \
		exit(EXIT_FAILURE); \
	} \
	ShowInfo("Test passed.\n"); \
} while (false)

#define context(message, ...) do { \
	ShowNotice("\n"); \
	ShowNotice("> " message "\n", ##__VA_ARGS__); \
} while (false)

#define expect(formatter, pass_expr, message, actual, expected, ...) do { \
	ShowNotice("\t" message "... ", ##__VA_ARGS__); \
	if (!(pass_expr)) { \
		passed = false; \
		ShowMessage("" CL_RED "Failed" CL_RESET "\n"); \
		ShowNotice("\t\tExpected: " CL_GREEN formatter CL_RESET ",\n", expected); \
		ShowNotice("\t\tReceived: " CL_RED formatter CL_RESET "\n", actual); \
	} else { \
		ShowMessage("" CL_GREEN "Passed" CL_RESET "\n"); \
	} \
} while (false)

#define expect_int(message, actual, expected, ...) \
	expect("%d", ((actual) == (expected)), message, (actual), (expected), ##__VA_ARGS__)
#define expect_str(message, actual, expected, ...) \
	expect("%s", (strcmp((actual), (expected)) == 0), message, (actual), (expected), ##__VA_ARGS__)

static bool test_base62_encode_int_padded(void)
{
	bool passed = true;

	{
		context("Encoding int '2' in a buffer with min length = 5");
		char output[30];
		bool res = base62->encode_int_padded(2, output, 5, sizeof(output));
		expect_int("To encode successfully", res, true);
		expect_str("To pad extra fields with 0", output, "00002");
		expect_int("To have a NULL-terminated buffer", output[6], '\0');
	}

	{
		context("Encoding int '1201' in a bufferwith min length = 5 (5 spaces + NULL terminator)");
		char output[30];
		bool res = base62->encode_int_padded(1201, output, 5, sizeof(output));
		expect_int("To encode successfully", res, true);
		expect_str("To pad extra fields with 0", output, "000jn");
		expect_int("To have a NULL-terminated buffer", output[6], '\0');
	}

	{
		context("Encoding int 'INT_MAX - 1' in a buffer with min length = 5, but enough buffer size");
		char output[30];
		bool res = base62->encode_int_padded(INT_MAX - 1 , output, 5, sizeof(output));
		expect_int("To encode successfully", res, true);
		expect_str("to return the encoded value without truncating it", output, "2lkCB0");
		expect_int("To have a NULL-terminated buffer", output[6], '\0');
	}

	{
		context("Encoding int 'INT_MAX - 1' in a buffer of length 6 (5 spaces + NULL terminator), which does not support the number");
		char output[6];
		// This will show an assert error to alert server owners that the used buffer is too small
		bool res = base62->encode_int_padded(INT_MAX - 1, output, 5, sizeof(output));
		expect_int("To fail the encoding", res, false);
		expect_int("To have a NULL-terminated buffer", output[5], '\0');
	}

	return passed;
}

int do_init(int argc, char **argv)
{
	ShowMessage("===============================================================================\n");
	ShowStatus("Starting tests.\n");

	TEST("Base62: encode int padded", test_base62_encode_int_padded);

	core->runflag = CORE_ST_STOP;
	return EXIT_SUCCESS;
}

int do_final(void) {
	ShowMessage("===============================================================================\n");
	ShowStatus("All tests passed.\n");
	return EXIT_SUCCESS;
}

void do_abort(void) { }

void set_server_type(void)
{
	SERVER_TYPE = SERVER_TYPE_UNKNOWN;
}

void cmdline_args_init_local(void) { }
