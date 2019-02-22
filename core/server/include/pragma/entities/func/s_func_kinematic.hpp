#ifndef __S_FUNC_KINEMATIC_HPP__
#define __S_FUNC_KINEMATIC_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/func/basefunckinematic.hpp>

namespace pragma
{
	class DLLSERVER SKinematicComponent final
		: public BaseFuncKinematicComponent,
		public SBaseNetComponent
	{
	public:
		SKinematicComponent(BaseEntity &ent) : BaseFuncKinematicComponent(ent) {}
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual void StartForward() override;
		virtual void StartBackward() override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER FuncKinematic
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
