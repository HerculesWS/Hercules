// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.

struct HookingPointData HookingPoints[] = {
/* pincode */
	{ HP_POP(pincode->handle, HP_pincode_handle) },
	{ HP_POP(pincode->decrypt, HP_pincode_decrypt) },
	{ HP_POP(pincode->error, HP_pincode_error) },
	{ HP_POP(pincode->update, HP_pincode_update) },
	{ HP_POP(pincode->sendstate, HP_pincode_sendstate) },
	{ HP_POP(pincode->setnew, HP_pincode_setnew) },
	{ HP_POP(pincode->change, HP_pincode_change) },
	{ HP_POP(pincode->compare, HP_pincode_compare) },
	{ HP_POP(pincode->check, HP_pincode_check) },
	{ HP_POP(pincode->config_read, HP_pincode_config_read) },
};

int HookingPointsLenMax = 20;
