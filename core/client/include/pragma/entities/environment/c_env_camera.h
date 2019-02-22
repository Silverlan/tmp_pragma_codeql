#ifndef __C_ENV_CAMERA_H__
#define __C_ENV_CAMERA_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/env_camera.h"

namespace pragma
{
	class DLLCLIENT CCameraComponent final
		: public BaseEnvCameraComponent
	{
	public:
		CCameraComponent(BaseEntity &ent) : BaseEnvCameraComponent(ent) {}
		virtual ~CCameraComponent() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		CallbackHandle m_cbCameraUpdate;
	};
};

class DLLCLIENT CEnvCamera
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif