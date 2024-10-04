#pragma once

#include "mlge/Common.h"
#include "mlge/UI/UIWidget.h"
#include "mlge/Resource/TTFFont.h"
#include "mlge/Text.h"

namespace mlge
{

MLGE_OBJECT_START(MUILabel, MObject, "Display a static text")
class MUILabel : public MUIWidget
{
	MLGE_OBJECT_INTERNALS(MUILabel, MUIWidget)

  public:

	void setText(std::string_view text);
	void setFont(ObjectPtr<MTTFFont> font);
	void setPtSize(int ptSize);

	void setAlign(HAlign halign, VAlign valign);

	void setStyle(const ObjectPtr<MUIStyle>& style) override;

  protected:

	virtual bool preConstruct() override;
	virtual void postConstruct() override;

	// Renderable operation
	virtual void updateRenderQueue() override;

	// RenderOperation interface
	virtual void render(RenderGroup group) override;

	ObjectPtr<MTTFFont> m_font;
	TextRenderer<true> m_textRenderer;
};

MLGE_OBJECT_END(MUILabel)


} // namespace mlge

