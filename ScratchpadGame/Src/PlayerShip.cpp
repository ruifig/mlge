#include "PlayerShip.h"
#include "mlge/Engine.h"
#include "mlge/Experimental/DebugText.h"

//////////////////////////////////////////////////////////////////////////
// MShipMoveComponent
//////////////////////////////////////////////////////////////////////////

FPoint moveWithRotation(const FPoint& pos, float angleDegrees, float speedPixelsPerSecond)
{
	// Convert the angle from degrees to radians
	float angleRadians = angleDegrees * (std::numbers::pi_v<float> / 180);

	// Calculate change in position
	FPoint newPos = pos;
	newPos.x += speedPixelsPerSecond * std::cos(angleRadians);
	newPos.y += speedPixelsPerSecond * std::sin(angleRadians);

	return newPos;
}

bool MShipMoveComponent::preConstruct()
{
	m_onProcessEventHandle = Engine::get().processEventDelegate.bind(this, &MShipMoveComponent::onProcessEvent);
	return Super::preConstruct();
}

void MShipMoveComponent::onProcessEvent(SDL_Event& evt)
{
	if (!m_owner)
	{
		return;
	}

	if (evt.type == SDL_MOUSEMOTION)
	{
		#if 0
		CZ_LOG(Log, "mousemotion: timestamp={}, state={}, x={}, y={}, xrel={}, yrel={}",
			evt.motion.timestamp,
			evt.motion.state,
			evt.motion.x, evt.motion.y,
			evt.motion.xrel, evt.motion.yrel);
		#endif
	}

	if (evt.type == SDL_KEYDOWN)
	{
		if (evt.key.keysym.scancode == SDL_SCANCODE_LEFT)
		{
			if (m_rotationOn != -1)
			{
				m_angularSpeed = 0;
			}
			m_rotationOn = -1;
		}
		else if (evt.key.keysym.scancode == SDL_SCANCODE_RIGHT)
		{
			if (m_rotationOn != +1)
			{
				m_angularSpeed = 0;
			}
			m_rotationOn = +1;
		}
		else if (evt.key.keysym.scancode == SDL_SCANCODE_UP)
		{
			m_engineOn = true;
		}

	}
	else if (evt.type == SDL_KEYUP)
	{
		if (evt.key.keysym.scancode == SDL_SCANCODE_LEFT || evt.key.keysym.scancode == SDL_SCANCODE_RIGHT)
		{
			m_rotationOn = 0;
		}
		else if (evt.key.keysym.scancode == SDL_SCANCODE_UP)
		{
			m_engineOn = false;
		}
	}
}

void MShipMoveComponent::tick(float deltaSeconds)
{
	if (m_rotationOn == 0)
	{
		float change = m_angularDecel * deltaSeconds;
		if (m_angularSpeed > 0)
		{
			m_angularSpeed -= change;
			if (m_angularSpeed < 0)
			{
				m_angularSpeed = 0;
			}
		}
		else if (m_angularSpeed < 0)
		{
			m_angularSpeed += change;
			if (m_angularSpeed > 0)
			{
				m_angularSpeed = 0;
			}
		}
	}
	else
	{
		// Using m_rotationOn to set the sign (- or +)
		float change = m_angularAcel * deltaSeconds * m_rotationOn;
		m_angularSpeed += change;
		if (m_rotationOn == -1)
		{
			if (m_angularSpeed < -m_topAngularSpeed)
			{
				m_angularSpeed = -m_topAngularSpeed;
			}
		}
		else
		{
			if (m_angularSpeed > m_topAngularSpeed)
			{
				m_angularSpeed = m_topAngularSpeed;
			}
		}
	}

	if (m_angularSpeed != 0)
	{
		m_owner->setRotation(m_owner->getRotation() + m_angularSpeed * deltaSeconds);
	}

	if (m_engineOn)
	{
		if (m_speed < m_topSpeed)
		{
			m_speed += m_accel * deltaSeconds;
			if (m_speed > m_topSpeed)
			{
				m_speed = m_topSpeed;
			}
		}
	}
	else
	{
		if (m_speed > 0)
		{
			m_speed -= (m_accel * 2) * deltaSeconds;

			if (m_speed < 0)
			{
				 m_speed = 0;
			}
		}
	}

	m_owner->setPosition(moveWithRotation(m_owner->getPosition(), m_owner->getRotation(), m_speed * deltaSeconds));

	Super::tick(deltaSeconds);
}

//////////////////////////////////////////////////////////////////////////
// APlayerShip
//////////////////////////////////////////////////////////////////////////

void APlayerShip::destruct()
{
	ms_instance = nullptr;
	Super::destruct();
}

bool APlayerShip::preConstruct()
{
	m_renderComp = addNewComponent<MFlipbookComponent>().get();
	m_renderComp->setFlipbook(m_move);

	m_moveComp = addNewComponent<MShipMoveComponent>().get();

	ms_instance = this;

	m_txt = addNewComponent<MTextRenderComponent>().get();
	m_txt->setText("Hello");
	m_txt->setFont(m_font);
	m_txt->setAlignment(HAlign::Center, VAlign::Center);
	m_txt->setRelativePosition({-m_move->getSprite(0).rect.w / 4.0f, -m_move->getSprite(0).rect.h / 4.0f});

	return Super::preConstruct();
}

void APlayerShip::tick(float deltaSeconds)
{
	m_txt->setText(std::to_string(getRotation()));
	Super::tick(deltaSeconds);
}

