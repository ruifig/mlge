#pragma once

#include "mlge/Resource/Resource.h"
#include "mlge/Math.h"

// #RVF : Remove this (if not used) or revise it

namespace mlge
{

MLGE_OBJECT_START(MTextureDefinition, MResourceDefinition, "A GPU Texture")
class MTextureDefinition : public MResourceDefinition
{
	MLGE_OBJECT_INTERNALS(MTextureDefinition, MResourceDefinition)
  public:

  protected:

	virtual void to_json(nlohmann::json& j) const override
	{
		Super::to_json(j);
	}

	virtual void from_json(const nlohmann::json& j) override
	{
		Super::from_json(j);
	}

	virtual ObjectPtr<MResource> create() const override;

	#if MLGE_EDITOR
	virtual std::unique_ptr<editor::BaseResourceWindow> createEditWindow() override;
	#endif
};
MLGE_OBJECT_END(MTextureDefinition)

MLGE_OBJECT_START(MTexture, MResource, "A GPU Texture")
class MTexture : public MResource
{
	MLGE_OBJECT_INTERNALS(MTexture, MResource)

  public:

	friend MTextureDefinition;

	SDL_Texture* getTexture()
	{
		return m_texture.get();
	}

	const Size& getSize()
	{
		return m_size;
	}

  private:

	SDLUniquePtr<SDL_Texture> m_texture;
	Size m_size;
};
MLGE_OBJECT_END(MTexture)

} // namespace mlge

