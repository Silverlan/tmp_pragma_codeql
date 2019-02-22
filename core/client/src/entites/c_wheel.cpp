#include "stdafx_client.h"
#include "pragma/entities/c_wheel.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(vhc_wheel,CWheel);

void CWheel::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CWheelComponent>();
}

/////////

luabind::object CWheelComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CWheelComponentHandleWrapper>(l);}
void CWheelComponent::ReceiveData(NetPacket &packet)
{
	auto *ent = nwm::read_entity(packet);
	auto wheelId = packet->Read<UChar>();
	auto bFrontWheel = packet->Read<Bool>();
	auto conPoint = packet->Read<Vector3>();
	auto axle = packet->Read<Vector3>();
	auto dir = packet->Read<Vector3>();
	auto suspensionLength = packet->Read<Float>();
	auto suspensionCompression = packet->Read<Float>();
	auto dampingRelaxation = packet->Read<Float>();
	auto radius = packet->Read<Float>();
	auto stiffness = packet->Read<Float>();
	auto dampingCompression = packet->Read<Float>();
	auto slip = packet->Read<Float>();
	auto steeringAngle = packet->Read<Float>();
	auto wheelRot = packet->Read<Float>();
	auto rollInfluence = packet->Read<Float>();
	if(ent != nullptr && ent->IsVehicle())
		Attach(ent,wheelId);
	SetFrontWheel(bFrontWheel);
	SetChassisConnectionPoint(conPoint);
	SetWheelAxle(axle);
	SetWheelDirection(dir);
	SetMaxSuspensionLength(suspensionLength);
	SetMaxSuspensionCompression(suspensionCompression);
	SetMaxDampingRelaxation(dampingRelaxation);
	SetWheelRadius(radius);
	SetSuspensionStiffness(stiffness);
	SetWheelDampingCompression(dampingCompression);
	SetFrictionSlip(slip);
	SetSteeringAngle(steeringAngle);
	SetWheelRotation(wheelRot);
	SetRollInfluence(rollInfluence);
}

void CWheelComponent::ReceiveSnapshotData(NetPacket &packet)
{
	auto steeringAngle = packet->Read<Float>();
	SetSteeringAngle(steeringAngle);
	auto wheelRot = packet->Read<Float>();
	SetWheelRotation(wheelRot);
	auto origin = packet->Read<Vector3>();
	auto rot = packet->Read<Quat>();
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	auto &t = info->m_worldTransform;
	t.setOrigin(btVector3(origin.x,origin.y,origin.z));
	t.setRotation(btQuaternion(rot.x,rot.y,rot.z,rot.w));
}

Bool CWheelComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvAttach)
	{
		auto *ent = nwm::read_entity(packet);
		if(ent != nullptr && ent->IsVehicle())
		{
			auto wheelId = packet->Read<UChar>();
			Attach(ent,wheelId);
		}
	}
	else if(eventId == m_netEvDetach)
		Detach();
	else if(eventId == m_netEvFrontWheel)
	{
		auto b = packet->Read<Bool>();
		SetFrontWheel(b);
	}
	else if(eventId == m_netEvAxle)
	{
		auto axle = packet->Read<Vector3>();
		SetWheelAxle(axle);
	}
	else if(eventId == m_netEvDirection)
	{
		auto dir = packet->Read<Vector3>();
		SetWheelDirection(dir);
	}
	else if(eventId == m_netEvMaxSuspensionLength)
	{
		auto len = packet->Read<Float>();
		SetMaxSuspensionLength(len);
	}
	else if(eventId == m_netEvMaxSuspensionCompression)
	{
		auto cmp = packet->Read<Float>();
		SetMaxSuspensionCompression(cmp);
	}
	else if(eventId == m_netEvRadius)
	{
		auto radius = packet->Read<Float>();
		SetWheelRadius(radius);
	}
	else if(eventId == m_netEvSuspensionStiffness)
	{
		auto stiffness = packet->Read<Float>();
		SetSuspensionStiffness(stiffness);
	}
	else if(eventId == m_netEvDampingCompression)
	{
		auto damping = packet->Read<Float>();
		SetWheelDampingCompression(damping);
	}
	else if(eventId == m_netEvFrictionSlip)
	{
		auto slip = packet->Read<Float>();
		SetFrictionSlip(slip);
	}
	else if(eventId == m_netEvSteeringAngle)
	{
		auto ang = packet->Read<Float>();
		SetSteeringAngle(ang);
	}
	else if(eventId == m_netEvRotation)
	{
		auto rot = packet->Read<Float>();
		SetWheelRotation(rot);
	}
	else if(eventId == m_netEvRollInfluence)
	{
		auto roll = packet->Read<Float>();
		SetRollInfluence(roll);
	}
	else if(eventId == m_netEvChassisConnectionPoint)
	{
		auto p = packet->Read<Vector3>();
		SetChassisConnectionPoint(p);
	}
	else if(eventId == m_netEvMaxDampingRelaxation)
	{
		auto damping = packet->Read<Float>();
		SetMaxDampingRelaxation(damping);
	}
	else
		return false;
	return true;
}
