#ifndef __C_WORLD_H__
#define __C_WORLD_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/baseworld.h>
#include "pragma/rendering/occlusion_culling/c_occlusion_octree.hpp"
#include "pragma/rendering/occlusion_culling/chc.hpp"
#include <unordered_map>

#pragma warning(push)
#pragma warning(disable : 4251)
namespace bsp {class File;};
namespace prosper {class Buffer;};
namespace util {class BSPTree;};
namespace pragma
{
	class DLLCLIENT CWorldComponent final
		: public BaseWorldComponent
	{
	public:
		CWorldComponent(BaseEntity &ent) : BaseWorldComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		void ReloadMeshCache();
		std::shared_ptr<OcclusionOctree<std::shared_ptr<ModelMesh>>> GetMeshTree() const;
		std::shared_ptr<CHC> GetCHCController() const;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;

		void SetBSPTree(const std::shared_ptr<util::BSPTree> &bspTree);
		const std::shared_ptr<util::BSPTree> &GetBSPTree() const;
	protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		void OnUpdateLOD(const Vector3 &posCam);
		void ReloadCHCController();
		std::unordered_map<uint32_t,bool> m_lodBaseMeshIds;
		std::shared_ptr<OcclusionOctree<std::shared_ptr<ModelMesh>>> m_meshTree;
		std::shared_ptr<CHC> m_chcController;

		std::shared_ptr<util::BSPTree> m_bspTree = nullptr;
	};
};
#pragma warning(pop)

class CPolyMesh;
class ModelMesh;
class CHC;
template<class T>
	class OcclusionOctree;
class DLLCLIENT CWorld
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
	virtual bool IsWorld() const override; // TODO: Move this to component
	Con::c_cout& print(Con::c_cout&);
	std::ostream& print(std::ostream&);
};

#endif