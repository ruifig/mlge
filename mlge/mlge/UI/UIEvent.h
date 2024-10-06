#pragma once

#include "mlge/Math.h"

namespace mlge
{

class UIWidget;

struct UIInternalEvent
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
	//UIWidget* source = nullptr;
	bool consumed = false;
};

struct UIEvent
{
	std::string_view name;
	// Event name, hashed
	uint64_t hash;
	UIWidget* source = nullptr;
};

}

