#pragma once

#include "mlge/Resource/Resource.h"

// #RVF : Remove this (if not used) or revise it

namespace mlge
{

MLGE_OBJECT_START(MImageDefinition, MResourceDefinition, "An image loaded from disk.\nBy itself, it can't do much, as it needs to be converted to a Texture")
class MImageDefinition : public MResourceDefinition
{
	MLGE_OBJECT_INTERNALS(MImageDefinition, MResourceDefinition)
  public:

  protected:

	uint32_t m_x = 0;
	uint32_t m_y = 0;

	virtual void to_json(nlohmann::json& j) const override
	{
		Super::to_json(j);
		j["x"] = m_x;
		j["y"] = m_y;
	}

	virtual void from_json(const nlohmann::json& j) override
	{
		Super::from_json(j);
		j.at("x").get_to(m_x);
		j.at("y").get_to(m_y);
	}

	virtual ObjectPtr<MResource> create() const override;

	#if MLGE_EDITOR
	virtual std::unique_ptr<editor::BaseResourceWindow> createEditWindow() override;
	#endif
};
MLGE_OBJECT_END(MImageDefinition)

MLGE_OBJECT_START(MImage, MResource, "An image loaded from disk. By itself, it can't do much, as it needs to be converted to a Texture")
class MImage : public MResource
{
	MLGE_OBJECT_INTERNALS(MImage, MResource)

  public:

	friend MImageDefinition;

  private:

	SDLUniquePtr<SDL_Surface> m_surface;
};
MLGE_OBJECT_END(MImage)

} // namespace mlge

