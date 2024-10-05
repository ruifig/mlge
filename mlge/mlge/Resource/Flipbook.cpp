#include "mlge/Resource/Flipbook.h"

namespace mlge
{

//////////////////////////////////////////////////////////////////////////
//		MFlipbookDefinition
//////////////////////////////////////////////////////////////////////////

ObjectPtr<MResource> MFlipbookDefinition::create() const
{
	auto res = createObject<MFlipbook>(*this);

	res->m_fps = m_fps;
	res->m_spriteSheet = dynamic_pointer_cast<MSpriteSheet>(m_spriteSheetDef.get()->getResource());
	if (!res->m_spriteSheet)
	{
		return nullptr;
	}

	res->m_frames.reserve(static_cast<size_t>(res->m_spriteSheet->getNumSprites()));
	float totalTime = 0;
	for(int i=0; i< res->m_spriteSheet->getNumSprites(); i++)
	{
		MFlipbook::Frame frame;
		frame.spriteIdx = i;
		frame.durationMs = 1000 / m_fps;
		frame.timeSec = totalTime;
		totalTime += (float)frame.durationMs / 1000.0f;
		res->m_frames.push_back(frame);
	}

	res->m_duration = totalTime;

	return res;
}


//////////////////////////////////////////////////////////////////////////
//		MFlipbookDefinition Editor
//////////////////////////////////////////////////////////////////////////
#if MLGE_EDITOR
namespace editor
{

} // namespace editor

std::unique_ptr<editor::BaseResourceWindow> MFlipbookDefinition::createEditWindow()
{
	return nullptr;
}
#endif

//////////////////////////////////////////////////////////////////////////
//		MFlipbook
//////////////////////////////////////////////////////////////////////////

int MFlipbook::getFrameAtTime(float timeSec) const
{
	for (size_t i = 0; i < m_frames.size(); i++)
	{
		if (timeSec >= m_frames[i].timeSec && timeSec < m_frames[i].getNextFrameTime())
		{
			return static_cast<int>(i);
		}
	}

	CZ_CHECK(false);
	return 0;
}

MFlipbook::Position MFlipbook::getTimelineStartPosition() const
{
	Position pos;
	pos.timeSec = 0;
	pos.frameIdx = 0;
	pos.nextFrameTime = m_frames[0].getNextFrameTime();
	return pos;
}

const Sprite& MFlipbook::getSpriteAndUpdatePosition(MFlipbook::Position& pos) const
{
	if (pos.frameEnded())
	{
		if (pos.timeSec >= m_duration)
		{
			pos.timeSec = fmod(pos.timeSec, m_duration);
		}
		pos.frameIdx = getFrameAtTime(pos.timeSec);
		pos.nextFrameTime = m_frames[static_cast<size_t>(pos.frameIdx)].getNextFrameTime();
	}

	return getSprite(m_frames[static_cast<size_t>(pos.frameIdx)].spriteIdx);
}

} // mlge

