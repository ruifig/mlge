#pragma once

#include "mlge/Render/RenderQueue.h"
#include "mlge/Resource/Flipbook.h"

namespace mlge
{

MLGE_OBJECT_START(MFlipbookComponent, MRenderComponent, "Actor component to display a flipbook")
class MFlipbookComponent : public MRenderComponent, public RenderOperation
{
	MLGE_OBJECT_INTERNALS(MFlipbookComponent, MRenderComponent)

  public:

	void setFlipbook(const ObjectPtr<MFlipbook>& flipbook);

  protected:


	//
	// MActorComponent
	//
	virtual void tick(float elapsedSeconds) override;

	//
	// MRenderComponent 
	//
	virtual void updateRenderQueue() override;

	//
	// RenderOperation
	//
	virtual void render(RenderGroup group) override;

	ObjectPtr<MFlipbook> m_flipbook;
	MFlipbook::Position m_pos;
};
MLGE_OBJECT_END(MFlipbookComponent)

} // namespace mlge

