#pragma once

#include "mlge/Common.h"
#include "crazygaze/core/SharedPtr.h"

namespace mlge
{

class MObject;

namespace details
{
	struct SharedPtrMObjectDeleter
	{
		void operator()(MObject* obj) const;
	};
}

template<typename T>
using ObjectPtr = SharedPtr<T, details::SharedPtrMObjectDeleter>;

template<typename T>
using WeakObjectPtr = WeakPtr<T, details::SharedPtrMObjectDeleter>;

}

namespace std
{
	template<typename T>
	struct hash<mlge::ObjectPtr<T>>
	{
		size_t operator()(const mlge::ObjectPtr<T>& key) const noexcept
		{
			return hash<T*>(key.get());
		}
	};
}

