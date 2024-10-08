#pragma once

#include "mlge/Game.h"
#include "mlge/UI/UIScene.h"

using namespace mlge;

class ScratchpadGame : public Game
{
  public:
	using Super = Game;

	ScratchpadGame();
	~ScratchpadGame();

	virtual bool init() override;
	virtual void tick(float deltaSeconds) override;
	virtual float startShutdown() override;
	virtual bool isShutdownFinished() override;
	virtual void shutdown() override;

  private:

	std::vector<DelegateHandle> m_delegateHandles;

	void onReadyToBegin();
	void onEndLudeoRun();
	void onLudeoSelected(const char* ludeoId);
	void onSessionClosed();

	WeakObjectPtr<AActor> m_debugText;
	WeakObjectPtr<AActor> m_playerShip;
};

MLGE_DECLARE_GAME(ScratchpadGame);

