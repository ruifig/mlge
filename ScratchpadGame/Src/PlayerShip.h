#pragma once

#include "mlge/Game.h"
#include "mlge/Actor.h"
#include "mlge/Resource/TTFFont.h"
#include "mlge/TextureComponent.h"
#include "mlge/Text.h"
#include "mlge/FlipbookComponent.h"

using namespace mlge;
using namespace std::literals::chrono_literals;

/**
 * Component that moves the ships.
 * This is a component instead of part of the actor's code, so it can be reused
 */
MLGE_OBJECT_START(MShipMoveComponent, MActorComponent, "Actor component that moves the actor as if it was a ship")
class MShipMoveComponent : public MActorComponent
{
private:
	MLGE_OBJECT_INTERNALS(MShipMoveComponent, MActorComponent)
public:

	~MShipMoveComponent()
	{
	}

	virtual bool defaultConstruct() override;
	virtual void tick(float deltaSeconds) override;
	void saveLudeoData(const char* compName);

	// Acceleration/Decelaration in pixels per second^2
	static float constexpr m_accel = 100;
	static float constexpr m_decel = m_accel * 2;  // Deceleration to apply with the engine is OFF
	static float constexpr m_topSpeed = 400; // Pixels per second

	// Rotation related
	static float constexpr m_angularAcel = 360; // Degrees per second^2 
	static float constexpr m_angularDecel = m_angularAcel*2; // Applied with not rotating
	static float constexpr m_topAngularSpeed = 360; // Degrees per second

protected:

	DelegateHandle m_onProcessEventHandle;
	void onProcessEvent(SDL_Event& evt);

	bool m_engineOn = false;

	// -1 : Key pressed to rotate it counterclockwise
	//  0 : No key pressed
	// +1 : Key pressed to rotate it clockwise
	int8_t m_rotationOn = 0;

	float m_speed = 0; // Speed at pixels per second
	float m_angularSpeed = 0; // Speed at which the ship is rotating (in angles per second)
};
MLGE_OBJECT_END(MShipMoveComponent)

MLGE_OBJECT_START(APlayerShip, AActor, "Player Ship")
class APlayerShip : public AActor
{
	MLGE_OBJECT_INTERNALS(APlayerShip, AActor)

  public:

	virtual void destruct() override;

	virtual bool defaultConstruct() override;

	virtual void tick(float deltaSeconds) override;

	static APlayerShip* tryGet()
	{
		return ms_instance;
	}

  protected:

	static inline APlayerShip* ms_instance = nullptr;

	// Acceleration/Decelaration in pixels per second^2
	static float constexpr m_accel = 100;
	static float constexpr m_decel = m_accel * 2;  // Deceleration to apply with the engine is OFF
	static float constexpr m_topSpeed = 400; // Pixels per second

	// Rotation related
	static float constexpr m_angularAcel = 360; // Degrees per second^2 
	static float constexpr m_angularDecel = m_angularAcel*2; // Applied with not rotating
	static float constexpr m_topAngularSpeed = 360; // Degrees per second

	ResourceRef<MFlipbook> m_move = "Flipbook/Fighter_Move";
	ResourceRef<MFlipbook> m_destroyed = "Flipbook/Fighter_Destroyed";

	MFlipbookComponent* m_renderComp = nullptr;
	MShipMoveComponent* m_moveComp = nullptr;

	inline static StaticResourceRef<MTTFFont> ms_fontRef = "fonts/RobotoCondensed-Medium";
	ResourceRef<MTTFFont> m_font = ms_fontRef;
	MTextRenderComponent* m_txt = nullptr;
	
};
MLGE_OBJECT_END(APlayerShip)

