#include "stdafx_server.h"
#include "pragma/model/s_modelmanager.h"
#include "pragma/entities/components/s_model_component.hpp"
#include <pragma/entities/components/base_model_component_setmodel.hpp>

using namespace pragma;

extern DLLSERVER ServerState *server;

void SModelComponent::Initialize()
{
	BaseModelComponent::Initialize();
}
luabind::object SModelComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SModelComponentHandleWrapper>(l);}

void SModelComponent::SetModel(const std::string &mdl)
{
	BaseModelComponent::SetModel(mdl);
	BaseModelComponent::SetModel<Model,ModelMesh,ModelManager>(mdl);
}

void SModelComponent::OnModelChanged(const std::shared_ptr<Model> &model)
{
	BaseModelComponent::OnModelChanged(model);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->WriteString(GetModelName());
		server->BroadcastTCP("ent_model",p);
	}
}

bool SModelComponent::SetBodyGroup(UInt32 groupId,UInt32 id)
{
	if(GetBodyGroup(groupId) == id)
		return true;
	auto r = BaseModelComponent::SetBodyGroup(groupId,id);
	if(r == false)
		return r;
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return r;
	NetPacket p;
	p->Write<UInt32>(groupId);
	p->Write<UInt32>(id);
	ent.SendNetEventTCP(m_netEvSetBodyGroup,p);
	return r;
}

void SModelComponent::SetSkin(unsigned int skin)
{
	if(skin == GetSkin())
		return;
	BaseModelComponent::SetSkin(skin);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	nwm::write_entity(p,&ent);
	p->Write<unsigned int>(skin);
	server->BroadcastTCP("ent_skin",p);
}

void SModelComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	std::string mdl = GetModelName();
	packet->WriteString(mdl);
	packet->Write<unsigned int>(GetSkin());

	auto &bodyGroups = GetBodyGroups();
	packet->Write<uint32_t>(static_cast<uint32_t>(bodyGroups.size()));
	for(auto bg : bodyGroups)
		packet->Write<uint32_t>(bg);
}
