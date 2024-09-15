#pragma once

#include "mlge/Math.h"

namespace mlge
{

class UIWidget;

struct UIEvent
{
	enum class Type
	{
		None,
		Pressed,
		Released,
		Click
	};

	Type type = Type::None;
	Point pos;
	UIWidget* source = nullptr;
};

}
