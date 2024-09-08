#pragma once

#include "mlge/Resource/TTFFont.h"
#include "mlge/Render/RenderQueue.h"

namespace mlge
{

enum class HAlign
{
	Left,
	Center,
	Right
};

enum class VAlign
{
	Top,
	Center,
	Bottom
};

struct TextRendererSettings
{
	MTTFFont* font = nullptr;
	const MTTFFont::Instance* fontInstance = nullptr;
	Rect area = {0, 0, 0, 0};
	HAlign halign = HAlign::Left;
	VAlign valign = VAlign::Top;
	Color color = Color::White;
	int ptsize = 12;
	bool dirtySize = true;

	Size calcTextSize(std::string_view str) const;
	bool prepareFont();
	void setFont(MTTFFont& font_);
	void setPtSize(int ptsize_);
	void renderImpl(std::string_view text, const Size& textSize) const;
};

template<bool CacheText>
class TextRendererBase;

template<>
class TextRendererBase<false>
{
	protected:
};

template<>
class TextRendererBase<true>
{
	protected:
	std::string m_text;
	Size m_textSize;
};


/**
 * Renders an UTF8 string.
 *
 * The object is lightweight with the intention of being created on the spot. This makes it possible to setup the common
 * parameters once, then making the calls to print text.
 *
 * @param CacheText If false, then it doesn't cache the text or it's size.
 *	If true, then a `setText` method exists to set the text and a `render()` exists to render it. This is more efficient since
 * it calculates the text dimensions just once (if the text doesn't change between render() calls)
 */
 template<bool CacheText = false>
struct TextRenderer : public TextRendererBase<CacheText>
{
  public:

	TextRenderer() = default;
	TextRenderer(const TextRenderer& other) = default;

	explicit TextRenderer(MTTFFont& font)
	{
		m_settings.font = &font;
	}

	explicit TextRenderer(MTTFFont& font, int ptsize)
		: m_settings(font, ptsize)
	{
		m_settings.font = &font;
		m_settings.ptsize = ptsize;
		m_settings.prepareFont();
	}

	TextRenderer& setFont(MTTFFont& font)
	{
		m_settings.setFont(font);
		return *this;
	}

	TextRenderer& setPtSize(int ptsize)
	{
		m_settings.setPtSize(ptsize);
		return *this;
	}

	int getPtSize() const
	{
		return m_settings.ptsize;
	}

	int getFontHeight()
	{
		if (!m_settings.prepareFont())
		{
			return 0;
		}
		return m_settings.fontInstance->height;
	}

	TextRenderer& setArea(const Rect& area)
	{
		m_settings.area = area;
		return *this;
	}

	const Rect& getArea() const
	{
		return m_settings.area;
	}

	TextRenderer& setHAlign(HAlign align)
	{
		m_settings.halign = align;
		return *this;
	}

	TextRenderer& setVAlign(VAlign align)
	{
		m_settings.valign = align;
		return *this;
	}

	TextRenderer& setAlign(HAlign halign, VAlign valign)
	{
		m_settings.halign = halign;
		m_settings.valign = valign;
		return *this;
	}

	TextRenderer& setColor(const Color& color)
	{
		m_settings.color = color;
		return *this;
	}

	/**
	 * Calculates text render dimensions if it were to render with the current settings
	 * NOTE: This doesn't change any of the state.
	 */
	Size calcTextSize(std::string_view str) const
	{
		return m_settings.calcTextSize(str);
	}

	/**
	 * Returns the text that will be displayed
	 * This is function is only available when CacheText == true
	 */
	template<typename R = const std::string&>
	auto getText() const -> std::enable_if_t<CacheText==true, R>
	{
		return this->m_text;
	}

	/**
	 * Calculates the text dimensions of the cached text.
	 * This is function is only available when CacheText == true
	 */
	template<typename R = Size>
	auto calcTextSize() -> std::enable_if_t<CacheText==true, R>
	{
		if (!m_settings.prepareFont())
		{
			return {0,0};
		}

		if (m_settings.dirtySize)
		{
			this->m_textSize = m_settings.calcTextSize(this->m_text);
			m_settings.dirtySize = false;
		}

		return this->m_textSize;
	}

	TextRenderer& skipLine(int /*count = 1*/)
	{
		if (m_settings.fontInstance)
		{
			// #RVF : Use m_fontInstance->lineSkip to advance to new line
		}
		return *this;
	}

	/**
	 * Renders uncached text.
	 * This is only available for when CacheText is false
	 */
	//typename std::enable_if<CacheText == false, TextRenderer<false>>::type& render(std::string_view text)
	template<typename R = TextRenderer>
	auto render(std::string_view text) -> std::enable_if_t<CacheText==false, R>&
	{
		if (!m_settings.prepareFont())
		{
			return *this;
		}

		m_settings.renderImpl(text, m_settings.calcTextSize(text));
		return *this;
	}

	/**
	 * Sets the text for when CacheText is true
	 *
	 */
	template<typename R = TextRenderer>
	auto setText(std::string_view text) -> std::enable_if_t<CacheText==true, R>&
	{
		this->m_text = text;
		m_settings.dirtySize = true;
		return *this;
	}

	/**
	 * 
	 */
	template<typename R = TextRenderer>
	auto render() -> std::enable_if_t<CacheText == true, R>&
	{
		if (!m_settings.prepareFont())
		{
			return *this;
		}

		calcTextSize();

		m_settings.renderImpl(this->m_text, this->m_textSize);

		return *this;
	}

  protected:
	TextRendererSettings m_settings;
};

MLGE_OBJECT_START(MTextRenderComponent, MRenderComponent, "An actor component that can render text")
/**
 * Component that can be attached to an Actor to render text
 * The text is displayed at the component's final position, and the alignment is done relative to that position.
 *
 * For example, if the component's final position is {0,100}, and alignment is {HAlign::Left, VAlign::Center}, the text will be
 * aligned on the left size of {0,100}, therefore outside the viewing area and not visible at all.
 * 
 * See `setRelativePosition`.
 */
class MTextRenderComponent : public MRenderComponent,  public RenderOperation
{
	MLGE_OBJECT_INTERNALS(MTextRenderComponent, MRenderComponent)

  public:

	virtual bool defaultConstruct() override;
	bool construct(std::string text);

	/**
	 * Sets the font to use
	 */
	void setFont(ObjectPtr<MTTFFont> font);

	/**
	 * Gets the font that is being used
	 */
	const ObjectPtr<MTTFFont>& getFont() const
	{
		return m_font;
	}

	/**
	 * Sets the font size to use
	 */
	void setPtSize(int ptsize);

	/**
	 * Gets the font size being used
	 */
	int getPtSize() const;

	/**
	 * Returns the height in pixels for the selected font and ptsize
	 */
	int getFontHeight();

	/**
	 * Set the text to render
	 */
	void setText(std::string_view text);

	/**
	 * Sets the text alignment
	 *
	 * Sets how the text is aligned in relation to the component's relative position.
	 * When rendering the component a render position point is calculated relative to the owning actor's position. These
	 * alignments specify how the text is alignment in relation to that point
	 */
	void setAlignment(HAlign halign, VAlign valign);

	/**
	 * Sets the text color
	 */
	void setColor(const Color& color);

  protected:

	// RenderComponent interface
	virtual void updateRenderQueue() override;

	// RenderOperation interface
	virtual void render(RenderGroup group) override;

  private:

	ObjectPtr<MTTFFont> m_font;
	TextRenderer<true> m_textRenderer;
};
MLGE_OBJECT_END(MTextRenderComponent)

} // namespace mlge

