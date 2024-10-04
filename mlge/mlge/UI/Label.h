#pragma once

#include "mlge/Common.h"
#include "mlge/UI/Widget.h"
#include "mlge/Resource/TTFFont.h"
#include "mlge/Text.h"

namespace mlge
{

MLGE_OBJECT_START(MLabel, MObject, "Display a static text")
class MLabel : public MWidget
{
	MLGE_OBJECT_INTERNALS(MLabel, MWidget)

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

MLGE_OBJECT_END(MLabel)


} // namespace mlge

