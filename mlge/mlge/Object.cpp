#include "mlge/Object.h"

namespace mlge
{


mlge::Class* Class::find(const char* name)
{
	for (auto&& c : ms_all)
	{
		if (strcmp(name, c->m_name) == 0)
		{
			return c;
		}
	}

	CZ_LOG(Error, "Class '{}' not found", name);
	return nullptr;
}

} // namespace mlge

