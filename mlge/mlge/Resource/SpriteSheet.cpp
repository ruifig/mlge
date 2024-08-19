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
			res->m_sprites.push_back(sprite);
		}
	}

	return res;
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

} // namespace mlge
