#include "mlge/Delegates.h"

namespace mlge
{

DelegateHandle::~DelegateHandle()
{
	unbind();
}

void DelegateHandle::unbind()
{
	if (m_control)
	{
		if (m_control->source)
		{
			m_control->source->unbind(m_control.get());
		}

		m_control = nullptr;
	}
}

BaseMultiCastDelegate::~BaseMultiCastDelegate()
{
	for (auto&& target : m_targets)
	{
		target.first->source = nullptr;
	}
}

void BaseMultiCastDelegate::unbind(DelegateHandle::ControlBlock* control)
{
	for(auto it=m_targets.begin(); it!=m_targets.end(); ++it)
	{
		if (it->first.get() == control)
		{
			// If we are currently broadcasting, we don't remove anything from the vector. We mark the slot as unused and it will eventually be removed by the broadcast function
			if (m_broadcasting)
			{
				// Reseting just "first" is enough to mark it as empty
				it->first.reset();
			}
			else
			{
				m_targets.erase(it);
			}
			return;
		}
	}
}

} // namespace mlge
