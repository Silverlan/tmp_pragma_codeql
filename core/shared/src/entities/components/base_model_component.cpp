#include "stdafx_shared.h"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/model/model.h"
#include <sharedutils/datastream.h>
#include <pragma/util/transform.h>

using namespace pragma;

ComponentEventId BaseModelComponent::EVENT_ON_MODEL_CHANGED = pragma::INVALID_COMPONENT_ID;
ComponentEventId BaseModelComponent::EVENT_ON_MODEL_MATERIALS_LOADED = pragma::INVALID_COMPONENT_ID;
ComponentEventId BaseModelComponent::EVENT_ON_SKIN_CHANGED = pragma::INVALID_COMPONENT_ID;
ComponentEventId BaseModelComponent::EVENT_ON_BODY_GROUP_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseModelComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_MODEL_CHANGED = componentManager.RegisterEvent("ON_MODEL_CHANGED");
	EVENT_ON_MODEL_MATERIALS_LOADED = componentManager.RegisterEvent("ON_MODEL_MATERIALS_LOADED");
	EVENT_ON_SKIN_CHANGED = componentManager.RegisterEvent("ON_SKIN_CHANGED");
	EVENT_ON_BODY_GROUP_CHANGED = componentManager.RegisterEvent("ON_BODY_GROUP_CHANGED");
}
BaseModelComponent::BaseModelComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{
	m_skin = util::SimpleProperty<util::UInt32Property,uint32_t>::Create(0u);
	m_skin->AddCallback([this](std::reference_wrapper<const uint32_t> oldVal,std::reference_wrapper<const uint32_t> newVal) {
		SetSkin(newVal.get());
	});
}

void BaseModelComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvSetBodyGroup = SetupNetEvent("set_body_group");
}

void BaseModelComponent::OnRemove()
{
	SetModel(std::shared_ptr<Model>{nullptr});
}

void BaseModelComponent::GetAnimations(Activity activity,std::vector<unsigned int> &animations) const
{
	auto hModel = GetModel();
	if(hModel == nullptr)
		return;
	hModel->GetAnimations(activity,animations);
}

unsigned char BaseModelComponent::GetAnimationActivityWeight(unsigned int animation) const
{
	auto hModel = GetModel();
	if(hModel == nullptr)
		return 0;
	return hModel->GetAnimationActivityWeight(animation);
}
Activity BaseModelComponent::GetAnimationActivity(unsigned int animation) const
{
	auto hModel = GetModel();
	if(hModel == nullptr)
		return Activity::Invalid;
	return hModel->GetAnimationActivity(animation);
}
float BaseModelComponent::GetAnimationDuration(unsigned int animation) const
{
	auto hModel = GetModel();
	if(hModel == nullptr)
		return 0.f;
	return hModel->GetAnimationDuration(animation);
}
int BaseModelComponent::LookupAttachment(const std::string &name) const
{
	auto &hMdl = GetModel();
	if(hMdl == nullptr)
		return -1;
	return hMdl->LookupAttachment(name);
}
int BaseModelComponent::LookupBlendController(const std::string &controller) const
{
	auto hModel = GetModel();
	if(hModel == nullptr)
		return -1;
	return hModel->LookupBlendController(controller);
}
Int32 BaseModelComponent::LookupBone(const std::string &name) const
{
	auto mdl = GetModel();
	if(mdl == nullptr)
		return -1;
	return mdl->LookupBone(name);
}
std::string BaseModelComponent::GetModelName() const
{
	if(m_modelName == NULL)
		return "";
	return *m_modelName;
}
bool BaseModelComponent::HasModel() const {return m_model != nullptr;}

const std::vector<uint32_t> &BaseModelComponent::GetBodyGroups() const {return m_bodyGroups;}
UInt32 BaseModelComponent::GetBodyGroup(unsigned int groupId) const
{
	if(groupId >= m_bodyGroups.size())
		return 0;
	return m_bodyGroups[groupId];
}
bool BaseModelComponent::SetBodyGroup(UInt32 groupId,UInt32 id)
{
	auto hMdl = GetModel();
	if(hMdl == nullptr)
		return false;
	auto numGroups = hMdl->GetBodyGroupCount();
	if(groupId >= numGroups)
		return false;
	//if(m_bodyGroups.size() != numGroups)
	//	m_bodyGroups.resize(numGroups); // Already done in BaseEntity::SetModel
	m_bodyGroups[groupId] = id;

	CEOnBodyGroupChanged evData{groupId,id};
	BroadcastEvent(EVENT_ON_BODY_GROUP_CHANGED,evData);
	return true;
}
void BaseModelComponent::SetBodyGroup(const std::string &name,UInt32 id)
{
	auto hMdl = GetModel();
	if(hMdl == nullptr)
		return;
	auto groupId = hMdl->GetBodyGroupId(name);
	if(groupId == -1)
		return;
	SetBodyGroup(groupId,id);
}

bool BaseModelComponent::HasModelMaterialsLoaded() const {return m_bMaterialsLoaded;}

void BaseModelComponent::SetModel(const std::string&)
{
	m_modelName = nullptr;
}

void BaseModelComponent::OnModelMaterialsLoaded()
{
	m_bMaterialsLoaded = true;
	BroadcastEvent(EVENT_ON_MODEL_MATERIALS_LOADED);
}
const std::shared_ptr<Model> &BaseModelComponent::GetModel() const {return m_model;}
unsigned int BaseModelComponent::GetSkin() const {return *m_skin;}
const std::shared_ptr<util::UInt32Property> &BaseModelComponent::GetSkinProperty() const {return m_skin;}
void BaseModelComponent::SetSkin(unsigned int skin)
{
	if(*m_skin == skin)
		return;
	*m_skin = skin;
	auto &model = GetModel();
	if(model != nullptr)
		model->PrecacheTextureGroup(skin);

	CEOnSkinChanged evData{skin};
	BroadcastEvent(EVENT_ON_SKIN_CHANGED,evData);
}

void BaseModelComponent::SetModel(const std::shared_ptr<Model> &mdl)
{
	m_model = mdl;
	m_bodyGroups.clear();
	m_bMaterialsLoaded = true;
	if(m_onModelMaterialsLoaded.IsValid())
		m_onModelMaterialsLoaded.Remove();
	if(mdl == NULL)
	{
		OnModelChanged(nullptr);
		return;
	}
	m_bMaterialsLoaded = false;
	m_onModelMaterialsLoaded = mdl->CallOnMaterialsLoaded([this]() {
		OnModelMaterialsLoaded();
	});
	m_bodyGroups.resize(mdl->GetBodyGroupCount());
	/*if(skeleton == NULL)
	{
		UpdateChildParentInfo();
		return;
	}*/
	SetSkin(GetSkin());

	OnModelChanged(mdl);
}

void BaseModelComponent::OnModelChanged(const std::shared_ptr<Model> &model)
{
	CEOnModelChanged evData{model};
	BroadcastEvent(EVENT_ON_MODEL_CHANGED,evData);
}

uint32_t BaseModelComponent::GetHitboxCount() const
{
	auto &mdl = GetModel();
	if(mdl == nullptr)
		return 0;
	return mdl.get()->GetHitboxCount();
}
bool BaseModelComponent::GetHitboxBounds(uint32_t boneId,Vector3 &min,Vector3 &max,Vector3 &origin,Quat &rot) const
{
	if(HasModel() == false)
	{
		min = Vector3{0.f,0.f,0.f};
		max = Vector3{0.f,0.f,0.f};
		origin = Vector3{0.f,0.f,0.f};
		rot = uquat::identity();
		return false;
	}
	GetModel().get()->GetHitboxBounds(boneId,min,max);
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto scale = pTrComponent.valid() ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f};
	min *= scale;
	max *= scale;
	auto animComponent = ent.GetAnimatedComponent();
	if(animComponent.valid() && animComponent->GetGlobalBonePosition(boneId,origin,rot) == false)
	{
		auto pPhysComponent = ent.GetPhysicsComponent();
		if(pPhysComponent.valid())
			origin = pPhysComponent->GetOrigin();
		if(pTrComponent.valid())
			rot = pTrComponent->GetOrientation();
	}
	return true;
}
uint32_t BaseModelComponent::GetFlexControllerCount() const
{
	auto &hMdl = GetModel();
	return (hMdl != nullptr) ? hMdl->GetFlexControllerCount() : 0u;
}
int BaseModelComponent::LookupAnimation(const std::string &name) const
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto hModel = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hModel == nullptr)
		return -1;
	return hModel->LookupAnimation(name);
}
bool BaseModelComponent::LookupFlexController(const std::string &name,uint32_t &flexId) const
{
	auto &hMdl = GetModel();
	if(hMdl == nullptr)
		return false;
	return hMdl->GetFlexControllerId(name,flexId);
}
void BaseModelComponent::Save(DataStream &ds)
{
	BaseEntityComponent::Save(ds);

	ds->WriteString(GetModelName());
	ds->Write<uint32_t>(GetSkin());

	// Write body groups
	auto &bodyGroups = GetBodyGroups();
	ds->Write<std::size_t>(bodyGroups.size());
	for(auto &bodyGroup : bodyGroups)
		ds->Write<uint32_t>(bodyGroup);
}
void BaseModelComponent::Load(DataStream &ds,uint32_t version)
{
	BaseEntityComponent::Load(ds,version);

	auto modelName = ds->ReadString();
	if(modelName.empty() == false)
		SetModel(modelName);

	auto skin = ds->Read<uint32_t>();
	SetSkin(skin);

	// Read body groups
	auto numBodyGroups = ds->Read<std::size_t>();
	for(auto i=decltype(numBodyGroups){0};i<numBodyGroups;++i)
	{
		auto id = ds->Read<uint32_t>();
		SetBodyGroup(i,id);
	}
}

///////////////

CEOnBodyGroupChanged::CEOnBodyGroupChanged(uint32_t groupId,uint32_t typeId)
	: groupId{groupId},typeId{typeId}
{}
void CEOnBodyGroupChanged::PushArguments(lua_State *l)
{
	Lua::PushInt(l,groupId);
	Lua::PushInt(l,typeId);
}

///////////////

CEOnSkinChanged::CEOnSkinChanged(uint32_t skinId)
	: skinId{skinId}
{}
void CEOnSkinChanged::PushArguments(lua_State *l)
{
	Lua::PushInt(l,skinId);
}

///////////////

CEOnModelChanged::CEOnModelChanged(const std::shared_ptr<Model> &model)
	: model{model}
{}
void CEOnModelChanged::PushArguments(lua_State *l)
{
	Lua::Push<std::shared_ptr<Model>>(l,model);
}
