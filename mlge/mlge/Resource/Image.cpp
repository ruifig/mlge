#include "mlge/Resource/Image.h"

namespace mlge
{
//////////////////////////////////////////////////////////////////////////
//		MImageDefinition
//////////////////////////////////////////////////////////////////////////

ObjectPtr<MResource> MImageDefinition::create() const
{
	auto res = createObject<MImage>(*this);

	auto fullpath = m_root->path / file;

	res->m_surface.reset(IMG_Load(fullpath.string().c_str()));
	if (!res->m_surface)
	{
		CZ_LOG(Error, "Failed to load image {} from file{}. ec={}", name, narrow(file.native()), SDL_GetError());
		return nullptr;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
//		MImageDefinition Editor
//////////////////////////////////////////////////////////////////////////
#if MLGE_EDITOR
namespace editor
{

} // namespace editor

std::unique_ptr<editor::BaseResourceWindow> MImageDefinition::createEditWindow()
{
	return nullptr;
}
#endif

} // namespace mlge


