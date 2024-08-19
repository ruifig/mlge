#pragma once

#include <memory>

namespace mlge
{

struct Root
{
	virtual ~Root() = default;

	static std::unique_ptr<Root> create();

	virtual bool init() = 0;

  protected:

	// Private to force using the create function
	Root() = default;
};

} // namespace mlge

