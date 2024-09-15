#pragma once

#include "mlge/Common.h"
#include "mlge/RenderComponent.h"
#include "mlge/Resource/Texture.h"

namespace mlge
{

MLGE_OBJECT_START(MTextureComponent, MRenderComponent, "Actor component that renders a single texture")
class MTextureComponent : public MRenderComponent, public RenderOperation
{
	MLGE_OBJECT_INTERNALS(MTextureComponent, MRenderComponent)

  public:

	void setTexture(ObjectPtr<MTexture> texture);

  protected:

	//
	// MRenderComponent 
	//
	virtual void updateRenderQueue() override;

	//
	// RenderOperation
	//
	virtual void render(RenderGroup group) override;

	ObjectPtr<MTexture> m_texture;
};
MLGE_OBJECT_END(MTextureComponent)


} // namespace mlge

