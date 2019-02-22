#ifndef __C_TRANSFORM_COMPONENT_HPP__
#define __C_TRANSFORM_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_transform_component.hpp>

namespace pragma
{
	class DLLCLIENT CTransformComponent final
		: public BaseTransformComponent,
		public CBaseNetComponent
	{
	public:
		CTransformComponent(BaseEntity &ent) : BaseTransformComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
};

#endif
