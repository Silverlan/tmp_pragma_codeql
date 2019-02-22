#include "stdafx_server.h"
#include "pragma/entities/func/s_func_softphysics.hpp"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_physics_softbody,FuncSoftPhysics);

void SFuncSoftPhysicsComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<PhysSoftBodyInfo>(m_softBodyInfo);
}

luabind::object SFuncSoftPhysicsComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SFuncSoftPhysicsComponentHandleWrapper>(l);}

////////

extern DLLSERVER SGame *s_game;

void FuncSoftPhysics::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFuncSoftPhysicsComponent>();
}
