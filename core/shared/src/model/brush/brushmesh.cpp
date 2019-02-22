#include "stdafx_shared.h"
#include "pragma/model/brush/brushmesh.h"
#include "pragma/physics/physshape.h"

Con::c_cout &operator<<(Con::c_cout& os,const BrushMesh& mesh)
{
	Vector3 min,max;
	mesh.GetBounds(&min,&max);
	os<<"BrushMesh["<<mesh.m_sides.size()<<"]["<<min<<"]["<<max<<"]";
	return os;
}

std::ostream &operator<<(std::ostream& os,const BrushMesh& mesh)
{
	Vector3 min,max;
	mesh.GetBounds(&min,&max);
	os<<"BrushMesh["<<mesh.m_sides.size()<<"]["<<min<<"]["<<max<<"]";
	return os;
}

BrushMesh::BrushMesh()
	: m_bConvex(true),m_shape(NULL)
{
	uvec::zero(&m_min);
	uvec::zero(&m_max);
}

std::shared_ptr<PhysShape> BrushMesh::GetShape() {return m_shape;}
void BrushMesh::UpdateHullShape(const std::vector<SurfaceMaterial> *surfaceMaterials)
{
	if(m_shape != NULL)
	{
		m_shape->Remove();
		m_shape = NULL;
	}
	if(IsConvex())
	{
		m_shape = std::make_shared<PhysConvexHullShape>();
		auto *shape = static_cast<PhysConvexHullShape*>(m_shape.get());
		shape->SetLocalScaling(Vector3(1.f,1.f,1.f));
		for(unsigned int i=0;i<m_sides.size();i++)
		{
			auto &side = m_sides[i];
			auto &verts = side->GetVertices();
			for(unsigned int j=0;j<verts.size();j++)
			{
				Vector3 &v = verts[j];
				shape->AddPoint(Vector3(v.x,v.y,v.z));
			}
		}
		return;
	}
	m_shape = std::make_shared<PhysTriangleShape>();
	auto *shape = static_cast<PhysTriangleShape*>(m_shape.get());
	for(auto it=m_sides.begin();it!=m_sides.end();++it)
	{
		auto &side = *it;
		auto &verts = side->GetVertices();
		auto *triangles = side->GetTriangles();
		const SurfaceMaterial *surfaceMat = nullptr;
		auto *mat = side->GetMaterial();
		if(mat != nullptr)
		{
			auto &data = mat->GetDataBlock();
			if(data != nullptr)
			{
				std::string surfaceIdentifier;
				if(data->GetString("surfacematerial",&surfaceIdentifier) == true)
				{
					auto it = std::find_if(surfaceMaterials->begin(),surfaceMaterials->end(),[&surfaceIdentifier](const SurfaceMaterial &mat) {
						return (mat.GetIdentifier() == surfaceIdentifier) ? true : false;
					});
					if(it != surfaceMaterials->end())
						surfaceMat = &(*it);
				}
			}
		}
		auto numTris = (triangles != nullptr) ? triangles->size() : 0ull;
		for(UInt i=0;i<numTris;i+=3)
		{
			auto &a = verts[(*triangles)[i]];
			auto &b = verts[(*triangles)[i +1]];
			auto &c = verts[(*triangles)[i +2]];
			shape->AddTriangle(a,b,c,surfaceMat);
		}
	}
	shape->Build(surfaceMaterials);
}

bool BrushMesh::IsConvex() {return m_bConvex;}
void BrushMesh::SetConvex(bool b) {m_bConvex = b;}
void BrushMesh::AddSide(std::shared_ptr<Side> side) {m_sides.push_back(side);}
const std::vector<std::shared_ptr<Side>> &BrushMesh::GetSides() const {return const_cast<BrushMesh*>(this)->GetSides();}
std::vector<std::shared_ptr<Side>> &BrushMesh::GetSides() {return m_sides;}

bool BrushMesh::IntersectAABB(const Vector3 &pos,const Vector3 &posNew,const Vector3 &extents,const Vector3 &posObj,float *entryTime,float *exitTime,Vector3 *hitnormal) const
{
	Vector3 extentsThis = (m_max -m_min) *0.5f;
	Vector3 posThis = posObj +m_min +extentsThis;
	return Sweep::AABBWithAABB(pos,posNew,extents,posThis,posThis,extentsThis,entryTime,exitTime,hitnormal);
}

bool BrushMesh::IntersectAABB(Vector3 *min,Vector3 *max) const
{
	if(!Intersection::AABBAABB(m_min,m_max,*min,*max))
		return false;
	return true;
}

void BrushMesh::Optimize()
{
	/*for(auto it=m_sides.begin();it!=m_sides.end();++it)
	{
		auto *side = *it;
		auto *mat = side->GetMaterial();
		for(auto it=m_sides.begin();it!=m_sides.end();)
		{
			auto *side2 = *it;
			auto *mat2 = side2->GetMaterial();
			if(side2 != side && mat2 == mat)
			{
				side->Merge(side2);
				it = m_sides.erase(it);
			}
			else
				++it;
		}
	}*/
}

void BrushMesh::Calculate(const std::vector<SurfaceMaterial> *surfaceMaterials)
{
	uvec::zero(&m_min);
	uvec::zero(&m_max);
	for(int i=0;i<m_sides.size();i++)
	{
		Vector3 min,max;
		m_sides[i]->GetBounds(&min,&max);
		if(i == 0)
		{
			m_min = min;
			m_max = max;
		}
		else
		{
			uvec::min(&m_min,min);
			uvec::max(&m_max,max);
		}
	}
	UpdateHullShape(surfaceMaterials);
}

void BrushMesh::GetBounds(Vector3 *min,Vector3 *max) const
{
	*min = m_min;
	*max = m_max;
}

bool BrushMesh::PointInside(Vector3 &p,double epsilon)
{
	for(int i=0;i<m_sides.size();i++)
	{
		auto &side = m_sides[i];
		auto &n = side->GetNormal();
		Vector3 of = p -(n *-side->GetDistance());
		float d = glm::dot(of,n);
		if(d >= epsilon) return false;
	}
	return true;
}