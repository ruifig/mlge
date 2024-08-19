#include "mlge/Resource/Texture.h"
#include "mlge/Render/Renderer.h"
#include "mlge/Editor/ResourceWindow.h"

namespace mlge
{

//////////////////////////////////////////////////////////////////////////
//		MTextureDefinition
//////////////////////////////////////////////////////////////////////////

ObjectPtr<MResource> MTextureDefinition::create() const
{
	auto fullpath = m_root->path / file;
	Size size;

	SDLUniquePtr<SDL_Surface> surface(IMG_Load(fullpath.string().c_str()));
	if (!surface)
	{
		CZ_LOG(Error, "Failed to load surface {} from file{}. ec={}", name, narrow(file.native()), SDL_GetError());
		return nullptr;
	}

	size.w = surface->w;
	size.h = surface->h;

	SDLUniquePtr<SDL_Texture> texture( SDL_CreateTextureFromSurface(Renderer::get().getSDLRenderer(), surface.get()));
	if (!texture)
	{
		CZ_LOG(Error, "Failed to create texture {} from surface loaded from file{}. ec={}", name, narrow(file.native()), SDL_GetError());
		return nullptr;
	}

	auto res = createObject<MTexture>(*this);
	res->m_texture = std::move(texture);
	res->m_size = size;

	return res;
}

//////////////////////////////////////////////////////////////////////////
//		MTextureDefinition Editor
//////////////////////////////////////////////////////////////////////////
#if MLGE_EDITOR
namespace editor
{

class TextureEditor : public ResourceWindow<MTextureDefinition, MTexture>
{
  public:
	using Super = ResourceWindow<MTextureDefinition, MTexture>;
	using Super::Super;

  private:

	virtual void show() override
	{
		MTextureDefinition& def = getDef();
		showString("name", def.name);
		showFilePath("file", def.file, ".bmp,.gif,.jpg,.jpeg,.lbm,.pcx,.png,.pnm,.qoi,.tga,.xcf,.xpm,.svg", "");
		showGenericButtons();
	}

	virtual void reloadResource() override
	{
	}

};

} // namespace editor

std::unique_ptr<editor::BaseResourceWindow> MTextureDefinition::createEditWindow()
{
	return std::make_unique<editor::TextureEditor>(*this);
}
#endif

} // namespace mlge

