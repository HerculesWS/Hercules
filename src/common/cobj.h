// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef _COBJ_H_
#define _COBJ_H_

// Defines a pattern for creating "classes"

#define COBJ(name) \
	struct name

#define COBJ_DEF_DETAIL \
	void *__detail;

#define COBJ_GET_DETAIL(obj, type) ((type)((obj)->__detail))
#define COBJ_SET_DETAIL(obj, value) (obj)->__detail = (void*)(value)

#define VTABLE_ENTRY(ret, name, ...) \
	ret (*name) (__VA_ARGS__);

#endif
