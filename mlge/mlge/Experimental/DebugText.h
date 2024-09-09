#pragma once

#include "mlge/Game.h"
#include "mlge/Actor.h"
#include "mlge/Text.h"

using namespace mlge;
using namespace std::literals::chrono_literals;

#define addDebugText(fmtStr, ...)                                          \
	{                                                                      \
		std::string _cz_internal_msg = std::format(fmtStr, ##__VA_ARGS__); \
		addDebugTextImpl(Color::Green, _cz_internal_msg);                  \
	}
#define addDebugTextEx(color, fmtStr, ...)                                 \
	{                                                                      \
		std::string _cz_internal_msg = std::format(fmtStr, ##__VA_ARGS__); \
		addDebugTextImpl(color, _cz_internal_msg);                         \
	}

void addDebugTextImpl(const Color& color, const std::string& str);

/**
 * Actor that once added to the level allows displaying debug text on top of the gameplay, through the addDebugText/addDebugTextEx
 * macros.
 */
MLGE_OBJECT_START(ADebugText, AActor, "Test Menu")
class ADebugText : public AActor
{
	MLGE_OBJECT_INTERNALS(ADebugText, AActor)

  public:

	virtual void destruct() override;

	void addEntry(const Color& color, const std::string& str);

	virtual bool defaultConstruct() override;

  protected:

	void tick(float deltaSeconds) override;

	ObjectPtr<MTTFFont> m_font;
	int m_lineHeight = 0;

	struct Entry
	{
		MTextRenderComponent* comp;
		float countdown;
	};

	std::vector<MTextRenderComponent*> m_pool;
	std::vector<Entry> m_entries;
	int m_fontSize;
};
MLGE_OBJECT_END(ADebugText)

