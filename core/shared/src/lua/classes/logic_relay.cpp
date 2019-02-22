#include "stdafx_shared.h"
#include "pragma/entities/logic/logic_relay.h"

using namespace pragma;

void BaseLogicRelayComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("toggle");
}
