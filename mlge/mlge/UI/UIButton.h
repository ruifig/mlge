#pragma once

#include "mlge/Common.h"
#include "mlge/UI/UIWidget.h"
#include "mlge/Resource/TTFFont.h"
#include "mlge/Text.h"
#include "crazygaze/core/FNVHash.h"

namespace mlge
{

using namespace cz::hash;

MLGE_OBJECT_START(MUIButton, MObject, "Base button. Can optionally display a text")
class MUIButton : public MUIWidget
{
	MLGE_OBJECT_INTERNALS(MUIButton, MUIWidget)

  public:

	void setText(std::string_view text);
	void setFont(ObjectPtr<MTTFFont> font);
	void setPtSize(int ptSize);
	void setAlign(HAlign halign, VAlign valign);
	void setStyle(const ObjectPtr<MUIStyle>& style) override;
	void setEventID(std::string_view id);

  protected:

	virtual bool preConstruct() override;
	virtual void postConstruct() override;

	// Renderable operation
	virtual void updateRenderQueue() override;

	// RenderOperation interface
	virtual void render(RenderGroup group) override;

	virtual void onMouseEnter() override;
	virtual void onMouseLeave() override;

	virtual void onUIInternalEvent(UIInternalEvent& evt) override;

	ObjectPtr<MTTFFont> m_font;
	TextRenderer<true> m_textRenderer;
	struct
	{
		std::string id;
		uint64_t hash = ""_fnv1a_64;
	} m_eventId;
};

MLGE_OBJECT_END(MUIButton)


} // namespace mlge

