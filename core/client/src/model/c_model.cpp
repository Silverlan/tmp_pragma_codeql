#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"

extern DLLCLIENT ClientState *client;

CModel::CModel(NetworkState *nw,unsigned int numBones,const std::string &name)
	: Model(nw,numBones,name)
{}

void CModel::PrecacheTextureGroup(uint32_t i)
{
	Model::PrecacheTextureGroup([](const std::string &path,bool bReload) -> Material* {
		return client->LoadMaterial(path,bReload);
	},i);
}

void CModel::OnMaterialMissing(const std::string &matName)
{
	NetPacket p {};
	p->WriteString(GetName());
	p->WriteString(matName);
	client->SendPacketUDP("query_model_texture",p);
}

void CModel::PrecacheTexture(uint32_t texId,bool bReload)
{
	Model::PrecacheTexture(texId,[](const std::string &path,bool bReload) -> Material* {
		return client->LoadMaterial(path,bReload);
	},bReload);
}

void CModel::AddMesh(const std::string &meshGroup,const std::shared_ptr<ModelMesh> &mesh) {}

void CModel::AddMesh(const std::string &meshGroup,const std::shared_ptr<CModelMesh> &mesh) {Model::AddMesh(meshGroup,mesh);}

void CModel::Update(ModelUpdateFlags flags)
{
	Model::Update(flags);
	if((flags &ModelUpdateFlags::UpdateVertexAnimationBuffer) != ModelUpdateFlags::None)
		UpdateVertexAnimationBuffer();
}
