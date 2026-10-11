#include "poc.h"

#include "common/db.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"

namespace herc
{
HERCAPI_COMMON_EXPORT poc_interface *poc{};

poc_interface::poc_interface()
{
	ShowDebug("constructing poc (core version)\n");
}

poc_interface::~poc_interface()
{
	ShowDebug("destructing poc (core version)\n");
	Assert_retv(m_foo_db == nullptr);
	Assert_retv(m_foo_vec.empty());
}

void poc_interface::init(bool minimal)
{
	ShowDebug("initializing poc (core version)\n");
	Assert_retv(m_foo_db == nullptr);
	Assert_retv(m_foo_vec.empty());
	m_foo_db = idb_alloc(DB_OPT_RELEASE_DATA);
	m_foo_vec.push_back(42);
	m_foo = 10;
}

void poc_interface::final()
{
	ShowDebug("finalizing poc (core version)\n");
	db_destroy(m_foo_db);
	m_foo_db = nullptr;
	m_foo_vec.clear();
}

int poc_interface::do_stuff(int foo)
{
	m_foo = foo;
	idb_iput(m_foo_db, foo, foo + 10);
	ShowDebug("Doing stuff from poc (core version): %d %" PRId64 "\n", m_foo, m_foo_vec.size());
	return m_foo;
}

int poc_interface::do_const_stuff(int foo) const
{
	ShowDebug("Doing const stuff from poc (core version): %d %" PRId64 "\n", m_foo, m_foo_vec.size());
	return m_foo;
}

void poc_interface::bleh_destroy_self()
{
	this->~poc_interface();
	aFree(this);
}

poc_interface *poc_interface::create()
{
	ShowDebug("creating poc (core version)\n");
	poc_interface *obj = nullptr;
	CREATE(obj, poc_interface, 1);
	return new (obj) poc_interface;
}

void poc_defaults()
{
	poc = poc_interface::create();
}
} // namespace herc
