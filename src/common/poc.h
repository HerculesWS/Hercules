#pragma once

#include "common/hercules.h"

#include <functional>
#include <optional>
#include <vector>

struct DBMap;

#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable : 4251)
#endif

namespace herc
{
class HERCAPI_COMMON_EXPORT poc_interface
{
  public:
	poc_interface();
	virtual ~poc_interface();
	virtual void init(bool minimal);
	virtual void final();

	virtual int do_stuff(int foo);
	virtual int do_const_stuff(int foo) const;

	void bleh_destroy_self();

	static poc_interface *create();

  protected:
	int m_foo{0};
	struct DBMap *m_foo_db{nullptr};
	std::vector<int> m_foo_vec;
};

#ifdef _MSC_VER
  #pragma warning(pop)
#endif

#ifdef HERCULES_CORE
void poc_defaults();
// HERCAPI_COMMON_EXTERN std::optional<std::function<poc_interface *()>> poc_hpm_factory;
#endif // HERCULES_CORE

HERCAPI_COMMON_EXTERN poc_interface *poc;
} // namespace herc
