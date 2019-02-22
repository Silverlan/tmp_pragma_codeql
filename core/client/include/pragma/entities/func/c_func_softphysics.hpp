#ifndef __C_FUNC_SOFTPHYSICS_H__
#define __C_FUNC_SOFTPHYSICS_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/func/basefuncsoftphysics.hpp>

namespace pragma
{
	class DLLCLIENT CFuncSoftPhysicsComponent final
		: public BaseFuncSoftPhysicsComponent
	{
	public:
		CFuncSoftPhysicsComponent(BaseEntity &ent) : BaseFuncSoftPhysicsComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CFuncSoftPhysics
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
