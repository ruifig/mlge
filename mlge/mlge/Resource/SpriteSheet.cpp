#include "mlge/Resource/SpriteSheet.h"
#include "mlge/Render/Renderer.h"

namespace mlge
{

//////////////////////////////////////////////////////////////////////////
//		MSpriteSheetDefinition
//////////////////////////////////////////////////////////////////////////
ObjectPtr<MResource> MSpriteSheetDefinition::create() const
{
	auto fullpath = m_root->path / file;
	Size sheetSize;

	SDLUniquePtr<SDL_Surface> surface(IMG_Load(fullpath.string().c_str()));
	if (!surface)
	{
		CZ_LOG(Error, "Failed to load surface {} from file{}. ec={}", name, narrow(file.native()), SDL_GetError());
		return nullptr;
	}

	sheetSize.w = surface->w;
	sheetSize.h = surface->h;

	auto res = createObject<MSpriteSheet>(*this);

	res->m_texture.reset(SDL_CreateTextureFromSurface(Renderer::get().getSDLRenderer(), surface.get()));
	if (!res->m_texture)
	{
		CZ_LOG(Error, "Failed to create texture {} from surface loaded from file{}. ec={}", name, narrow(file.native()), SDL_GetError());
		return nullptr;
	}

	Rect rect(0,0, m_cellWidth, m_cellHeight);

	if ((sheetSize.w % m_cellWidth) || (sheetSize.h % m_cellHeight))
	{
		CZ_LOG(Error, "Sheet size is not divisible by cell width and/or cell height.")
		return nullptr;
	}

	int cols = sheetSize.w / m_cellWidth;
	int rows = sheetSize.h / m_cellHeight;

	for (int row = 0; row < rows; row++)
	{
		for(int col = 0; col < cols; col++)
		{
			Sprite sprite;
			sprite.rect = Rect({col * m_cellWidth, row * m_cellHeight}, m_cellWidth, m_cellHeight);
			sprite.texture = res->m_texture.get();
			sprite.origin = {m_originX, m_originY};
			res->m_sprites.push_back(sprite);
		}
	}

	return res;
}

void MSpriteSheetDefinition::to_json(nlohmann::json& j) const
{
	Super::to_json(j);
	j["cellWidth"] = m_cellWidth;
	j["cellHeight"] = m_cellHeight;
	j["originX"] = m_originX;
	j["originY"] = m_originY;
}

void MSpriteSheetDefinition::from_json(const nlohmann::json& j)
{
	Super::from_json(j);
	j.at("cellWidth").get_to(m_cellWidth);
	j.at("cellHeight").get_to(m_cellHeight);

	m_originX = m_cellWidth / 2;
	m_originY = m_cellHeight / 2;

	if (j.count("originX") != 0)
	{
		j.at("originX").get_to(m_originX);
	}

	if (j.count("originY") != 0)
	{
		j.at("originY").get_to(m_originY);
	}
}


//////////////////////////////////////////////////////////////////////////
//		MSpriteSheetDefinition Editor
//////////////////////////////////////////////////////////////////////////
#if MLGE_EDITOR
namespace editor
{

} // namespace editor

std::unique_ptr<editor::BaseResourceWindow> MSpriteSheetDefinition::createEditWindow()
{
	return nullptr;
}
#endif

void renderSprite(const Sprite& sprite, const Point& pos, float angleDegrees, float scale)
{
	SDL_Renderer* sdlRenderer = Renderer::get().getSDLRenderer();

	Rect dstRect = {
		0, 0,
		static_cast<int>(static_cast<float>(sprite.rect.w) * scale), static_cast<int>(static_cast<float>(sprite.rect.h) * scale)};

	dstRect.move(pos);

	FPoint offset = {static_cast<float>(sprite.origin.x) * scale, static_cast<float>(sprite.origin.y) * scale};
	dstRect.translate(static_cast<int>(-offset.x), static_cast<int>(-offset.y));

	SDL_RenderCopyEx(sdlRenderer, sprite.texture,
		&sprite.rect, &dstRect,
		angleDegrees,
		nullptr, // Rotation center
		SDL_FLIP_NONE
		);
}

} // namespace mlge

