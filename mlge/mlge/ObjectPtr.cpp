#include "mlge/ObjectPtr.h"
#include "mlge/Object.h"

namespace mlge::details
{

void SharedPtrMObjectDeleter::operator()(MObject* obj) const
{
	obj->destruct();
	obj->~MObject();
}

}

