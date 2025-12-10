#pragma once

#include "mlge/Common.h"
#include "mlge/Resource/SpriteSheet.h"

namespace mlge
{

MLGE_OBJECT_START(MFlipbookDefinition, MResourceDefinition, "A flip book uses a SpriteSheet to display an animation")
class MFlipbookDefinition : public MResourceDefinition
{
	MLGE_OBJECT_INTERNALS(MFlipbookDefinition, MResourceDefinition)

  public:

	virtual void to_json(nlohmann::json& j) const override
	{
		Super::to_json(j);
		j["spriteSheet"] = m_spriteSheetDef.name;
		j["fps"] = m_fps;
	}

	virtual void from_json(const nlohmann::json& j) override
	{
		Super::from_json(j);
		j.at("spriteSheet").get_to(m_spriteSheetDef.name);
		j.at("fps").get_to(m_fps);
	}

  protected:

	/** Sprite sheet to use as the source of the sprites */
	MResourceDefinition::Ref<MSpriteSheetDefinition> m_spriteSheetDef;

	/** Frames per second */
	int m_fps = 15;

	virtual ObjectPtr<MResource> create() const override;

	#if MLGE_EDITOR
	virtual std::unique_ptr<editor::BaseResourceWindow> createEditWindow() override;
	#endif
};
MLGE_OBJECT_END(MFlipbookDefinition)


MLGE_OBJECT_START(MFlipbook, MResource, "A flip book uses a SpriteSheet to display an animation")
class MFlipbook : public MResource
{
	MLGE_OBJECT_INTERNALS(MFlipbook, MResource)

  public:

	struct Frame
	{
		/** Sprite index */
		int spriteIdx = 0;

		/** how long to display the sprite for (in milliseconds) */
		int durationMs = 100;

		/** Time in seconds in the entire timeline */
		float timeSec = 0;

		/** Returns the time at which this frame ends and the next one should display in the timeline */
		float getNextFrameTime() const
		{
			return timeSec + ((float)durationMs / 1000.0f);
		}
	};

	/**
	 * Represents a position in the flipbook timeline
	 */
	struct Position
	{
		void tick(float deltaSeconds);

		void reset()
		{
			timeSec = 0;
			frameIdx = 0;
			nextFrameTime = 0.1f;
		}

		/** Checks if the current frame should finish display and we should switch to the next frame */
		bool frameEnded() const
		{
			return timeSec >= nextFrameTime ? true : false;
		}

		const Sprite& getSprite() const
		{
			return outer->getSprite(outer->m_frames[static_cast<size_t>(frameIdx)].spriteIdx);
		}

		void render(const Point& pos, float angleDegrees = 0.0f, float scale = 1.0f)
		{
			mlge::renderSprite(getSprite(), pos, angleDegrees, scale);
		}

		/** Flipbook this is tied to */
		const MFlipbook* outer = nullptr;

		/** Time in seconds */
		float timeSec = 0;

		/**
		 * The frame this position refers to (if up to date in relation to timeSec)
		 * This is not meant to be changed externally. It's used internally by the getSprite method to speed up calculations.
		 */
		int frameIdx = 0;

		/** Time in seconds when the next frame should start */
		float nextFrameTime = 0.1f;

	};

	int getNumSprites() const
	{
		return m_spriteSheet->getNumSprites();
	}

	Position getTimelineStartPosition() const;

	/**
	 * Gets a sprite by index
	 */
	const Sprite& getSprite(int idx) const
	{
		return m_spriteSheet->getSprite(idx);
	}

  protected:
	friend class MFlipbookDefinition;

	int getFrameAtTime(float timeSec) const;

	int m_fps = 15;
	// Total playback time of the flipbook, in seconds
	float m_duration = 0;
	ObjectPtr<MSpriteSheet> m_spriteSheet;
	std::vector<Frame> m_frames;
};
MLGE_OBJECT_END(MFlipbook)

} // mlge

