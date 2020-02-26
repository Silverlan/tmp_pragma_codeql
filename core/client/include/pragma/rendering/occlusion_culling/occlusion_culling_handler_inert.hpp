#ifndef __OCCLUSION_CULLING_HANDLER_INERT_HPP__
#define __OCCLUSION_CULLING_HANDLER_INERT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler.hpp"

namespace pragma
{
	class DLLCLIENT OcclusionCullingHandlerInert
		: public OcclusionCullingHandler
	{
	public:
		OcclusionCullingHandlerInert()=default;
		virtual void PerformCulling(
			const pragma::rendering::RasterizationRenderer &renderer,const Vector3 &camPos,
			std::vector<pragma::CParticleSystemComponent*> &particlesOut
		) override;
		virtual void PerformCulling(
			const pragma::rendering::RasterizationRenderer &renderer,const Vector3 &camPos,
			std::vector<pragma::OcclusionMeshInfo> &culledMeshesOut,bool cullByViewFrustum=true
		) override;
	};
};

#endif
