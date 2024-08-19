#pragma once

#include "mlge/Resource/Resource.h"
#include "mlge/Math.h"

namespace mlge
{

struct Sprite
{
	SDL_Texture* texture;
	Rect rect;
};

MLGE_OBJECT_START(MSpriteSheetDefinition, MResourceDefinition, "A sprite sheet is a collection of textures created from a single image")
class MSpriteSheetDefinition : public MResourceDefinition
{
	MLGE_OBJECT_INTERNALS(MSpriteSheetDefinition, MResourceDefinition)
  public:

  protected:

	virtual void to_json(nlohmann::json& j) const override
	{
		Super::to_json(j);
		j["cellWidth"] = m_cellWidth;
		j["cellHeight"] = m_cellHeight;
	}

	virtual void from_json(const nlohmann::json& j) override
	{
		Super::from_json(j);
		j.at("cellWidth").get_to(m_cellWidth);
		j.at("cellHeight").get_to(m_cellHeight);
	}

	virtual ObjectPtr<MResource> create() const override;

	#if MLGE_EDITOR
	virtual std::unique_ptr<editor::BaseResourceWindow> createEditWindow() override;
	#endif

	int m_cellWidth = 0;
	int m_cellHeight = 0;
};
MLGE_OBJECT_END(MSpriteSheetDefinition)



MLGE_OBJECT_START(MSpriteSheet, MResource, "A sprite sheet is a collection of textures created from a single image")
class MSpriteSheet : public MResource
{
	MLGE_OBJECT_INTERNALS(MSpriteSheet, MResource)
  public:
	friend MSpriteSheetDefinition;

	int getNumSprites() const
	{
		return static_cast<int>(m_sprites.size());
	}

	const Sprite& getSprite(int idx)
	{
		CZ_CHECK(idx >= 0 && idx < static_cast<int>(m_sprites.size()));
		return m_sprites[static_cast<unsigned int>(idx)];
	}

  protected:

	SDLUniquePtr<SDL_Texture> m_texture;
	std::vector<Sprite> m_sprites;
};
MLGE_OBJECT_END(MSpriteSheet)

}; // namespace mlge

