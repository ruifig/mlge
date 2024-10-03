#pragma once

#include "mlge/Common.h"
#include "mlge/ActorComponent.h"
#include "mlge/Math.h"

#include "crazygaze/core/Singleton.h"

namespace mlge
{

enum class RenderGroup
{
	Background = 0,
	SkiesEarlier = 5,
	Main = 10,
	PhysicsDebug = 20,
	SkiesLate = 21,
	Overlay = 25,
	OverlayDebug = 30,
	Stats = 31,
	MAX = 32
};

class RenderOperation
{
  public:
	virtual ~RenderOperation() {}
	virtual void render(RenderGroup group) = 0;
};

// Forward declarations
class Renderable
{
  public:

	virtual ~Renderable() {}
	virtual void updateRenderQueue() = 0;
};

class RenderQueue : public Singleton<RenderQueue>
{
  public:

	bool init();

	/**
	 * Adds a renderable to the list of objects that wish to render.
	 * During the rendering pass, their updateRenderQueue() function is called
	 */
	void addRenderable(Renderable& renderable);

	/**
	 * Removes a renderable from the list of objects that wish to be rendered.
	 */
	void removeRenderable(Renderable& renderable);

	/**
	 * Called every frame by MRenderComponent instances to add render operations to the queue.
	 */
	void addOp(RenderOperation& op, RenderGroup group);

	void render();

  private:
	std::set<Renderable*> m_renderables;

	struct RenderGroupData
	{
		std::vector<RenderOperation*> ops;
		bool active = true;
	};

	RenderGroupData m_groups[static_cast<int>(RenderGroup::MAX)];
};

} // namespace mlge

