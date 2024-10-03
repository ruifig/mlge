#include "DebugText.h"
#include "mlge/Config.h"

namespace
{
	ADebugText* gDebugText = nullptr;
}

void addDebugTextImpl(const Color& color, const std::string& str)
{
	if (gDebugText)
	{
		gDebugText->addEntry(color, str);
	}
}

void ADebugText::destruct()
{
	gDebugText = nullptr;
	Super::destruct();
}

void ADebugText::addEntry(const Color& color, const std::string& str)
{
	m_entries.push_back({});
	m_entries.back().countdown = 15.0f;

	if (m_pool.size())
	{
		m_entries.back().comp = m_pool.back();
		m_pool.pop_back();
	}
	else
	{
		MTextRenderComponent* comp = addNewComponent<MTextRenderComponent>().get();
		comp->setFont(m_font);
		comp->setPtSize(m_fontSize);

		comp->setAlignment(HAlign::Right, VAlign::Bottom);
		m_entries.back().comp = comp;
		m_lineHeight = comp->getFontHeight();
	}

	m_entries.back().comp->setColor(color);
	m_entries.back().comp->setRelativePosition({0.0f, 0.0f + static_cast<float>((m_entries.size()-1) * m_lineHeight)});
	m_entries.back().comp->setText(str);
}

bool ADebugText::preConstruct()
{
	gDebugText = this;

	// Load the font
	{
		std::string fontName = Config::get().getValueOrDefault<std::string>("Engine", "debugfont", "fonts/RobotoCondensed-Medium");
		m_font = ResourceRef<MTTFFont>(fontName);
		m_fontSize = Config::get().getValueOrDefault<int>("Engine", "debugfont_size", m_font->getDefinition().getDefaultSize());
	}

	m_font->loadASCIIGlyphs(m_fontSize);

	return Super::preConstruct();
}

void ADebugText::tick(float deltaSeconds)
{
	for (size_t idx = 0; idx != m_entries.size(); )
	{
		Entry& e = m_entries[idx];
		e.countdown -= deltaSeconds;
		if (e.countdown <= 0)
		{
			m_pool.push_back(e.comp);
			e.comp->setText("");
			m_entries.erase(m_entries.begin() + static_cast<int>(idx));
		}
		else
		{
			e.comp->setRelativePosition({0.0f, 0.0f + static_cast<float>(idx * m_lineHeight)});
			idx++;
		}
	}
}

