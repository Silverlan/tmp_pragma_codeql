#include "stdafx_client.h"
#include "pragma/entities/components/c_flex_component.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/audio/c_alsound.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <stack>
#include <pragma/model/model.h>
#include <se_scene.hpp>
#include <alsound_buffer.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

#pragma optimize("",off)
static auto cvFlexPhonemeDrag = GetClientConVar("cl_flex_phoneme_drag");
luabind::object CFlexComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CFlexComponentHandleWrapper>(l);}
void CFlexComponent::UpdateFlexControllers()
{
	auto t = c_game->CurTime();
	auto dt = c_game->DeltaTime();
	auto flexDrag = cvFlexPhonemeDrag->GetFloat();
	for(auto &pair : m_flexControllers)
	{
		auto &info = pair.second;
		if(info.endTime != 0.f && t >= info.endTime && info.targetValue != 0.f)
			info.targetValue = 0.f;
		info.value = umath::lerp(info.value,info.targetValue,umath::min(dt /flexDrag,1.0));
	}
}

void CFlexComponent::SetFlexWeight(uint32_t flexIdx,float weight)
{
	if(flexIdx >= m_flexWeights.size())
		return;
	m_flexWeights.at(flexIdx) = weight;
	m_updatedFlexWeights.at(flexIdx) = true;
}

// rotate by the inverse of the matrix
// TODO
static void VectorIRotate( const float *in1, const Mat3x4& in2, float *out )
{
	out[0] = in1[0]*in2[0][0] + in1[1]*in2[1][0] + in1[2]*in2[2][0];
	out[1] = in1[0]*in2[0][1] + in1[1]*in2[1][1] + in1[2]*in2[2][1];
	out[2] = in1[0]*in2[0][2] + in1[1]*in2[1][2] + in1[2]*in2[2][2];
}
void CFlexComponent::UpdateEyeFlexes(Eyeball &eyeball,uint32_t eyeballIdx)
{
	// TODO: Move this code to eye component
	auto eyeC = GetEntity().GetComponent<CEyeComponent>();
	auto *eyeballData = eyeC.valid() ? eyeC->GetEyeballData(eyeballIdx) : nullptr;
	if(eyeballData == nullptr)
		return;
	auto &state = eyeballData->state;
	Vector3 headup {};
	Vector3 headforward {};
	Vector3 pos {};

	// get weighted position of eyeball angles based on the "raiser", "neutral", and "lowerer" controls

	auto upperLid = 0.f;
	auto lowerLid = 0.f;
	for(auto i=decltype(eyeball.upperFlexDesc.size()){0u};i<eyeball.upperFlexDesc.size();++i)
	{
		upperLid += GetFlexWeight(eyeball.upperFlexDesc.at(i)) *umath::asin(eyeball.upperTarget.at(i) /eyeball.radius);
		lowerLid += GetFlexWeight(eyeball.lowerFlexDesc.at(i)) *umath::asin(eyeball.lowerTarget.at(i) /eyeball.radius);
	}

	// Con_DPrintf("%.1f %.1f\n", RAD2DEG( upperlid ), RAD2DEG( lowerlid ) );		

	float sinupper, cosupper, sinlower, coslower;
	sinupper = umath::sin(upperLid);
	cosupper = umath::cos(upperLid);

	sinlower = umath::sin(lowerLid);
	coslower = umath::cos(lowerLid);

	// convert to head relative space
	headup = state.up;
	headforward = state.forward;
	// TODO
	//VectorIRotate( &state.up, m_BoneToWorld[peyeball->bone], &headup );
	//VectorIRotate( &state.forward, m_BoneToWorld[peyeball->bone], &headforward );

	// upper lid
	pos = headup *(sinupper *eyeball.radius);
	pos = pos +(cosupper *eyeball.radius) *headforward;
	SetFlexWeight(eyeball.upperLidFlexDesc,uvec::dot(pos,eyeball.up));

	// lower lid
	pos = headup *(sinlower *eyeball.radius);
	pos = pos +(coslower *eyeball.radius) *headforward;
	SetFlexWeight(eyeball.lowerLidFlexDesc,uvec::dot(pos,eyeball.up));
}

void CFlexComponent::UpdateEyeFlexes()
{
	auto &mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	auto &eyeballs = mdl->GetEyeballs();
	for(auto eyeballIdx=decltype(eyeballs.size()){0u};eyeballIdx<eyeballs.size();++eyeballIdx)
		UpdateEyeFlexes(eyeballs.at(eyeballIdx),eyeballIdx);
}

void CFlexComponent::OnModelChanged(const std::shared_ptr<Model> &mdl)
{
	m_flexControllers.clear();
	m_flexWeights.clear();
	m_updatedFlexWeights.clear();
	if(mdl == nullptr)
		return;
	m_flexWeights.resize(mdl->GetFlexCount(),0.f);
	m_updatedFlexWeights.resize(m_flexWeights.size(),false);
}

void CFlexComponent::Initialize()
{
	BaseFlexComponent::Initialize();
	BindEventUnhandled(LogicComponent::EVENT_ON_TICK,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		UpdateFlexControllers();
	});
	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnModelChanged(static_cast<pragma::CEOnModelChanged&>(evData.get()).model);
	});
	GetEntity().AddComponent<LogicComponent>();
	OnModelChanged(GetEntity().GetModel());
}

void CFlexComponent::SetFlexController(uint32_t flexId,float val,float duration,bool clampToLimits)
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return;
	auto *flexC = mdl->GetFlexController(flexId);
	if(flexC == nullptr)
		return;
	if(clampToLimits)
		val = umath::clamp(val,flexC->min,flexC->max);
	auto it = m_flexControllers.find(flexId);
	if(it == m_flexControllers.end())
		it = m_flexControllers.insert(std::make_pair(flexId,FlexControllerInfo{})).first;
	auto &flexInfo = it->second;
	flexInfo.targetValue = val;
	flexInfo.endTime = (duration > 0.f) ? c_game->CurTime() +duration : 0.f;
	
	//InitializeVertexAnimationBuffer();

	//if(m_vertexAnimationBuffer == nullptr) // prosper TODO
	//	InitializeVertexAnimationBuffer(); // prosper TODO
}
bool CFlexComponent::GetFlexController(uint32_t flexId,float &val) const
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr || mdl->GetFlexController(flexId) == nullptr)
		return false;
	auto it = m_flexControllers.find(flexId);
	if(it != m_flexControllers.end())
		val = it->second.value;
	else
		val = 0.f;
	return true;
}

void CFlexComponent::UpdateFlexWeights()
{
	auto mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	auto &flexes = mdl->GetFlexes();
	assert(flexes.size() == m_flexWeights.size());
	auto numFlexes = umath::min(flexes.size(),m_flexWeights.size());
	for(auto flexId=decltype(numFlexes){0u};flexId<numFlexes;++flexId)
	{
		auto flexVal = 0.f;
		UpdateFlexWeight(flexId,flexVal);
		m_flexWeights.at(flexId) = flexVal;
	}
	// UpdateEyeFlexes();

	// Clear for next update
	std::fill(m_updatedFlexWeights.begin(),m_updatedFlexWeights.end(),false);
}

const std::vector<float> &CFlexComponent::GetFlexWeights() const {return m_flexWeights;}

float CFlexComponent::GetFlexWeight(uint32_t flexId) const
{
	auto weight = 0.f;
	GetFlexWeight(flexId,weight);
	return weight;
}
bool CFlexComponent::GetFlexWeight(uint32_t flexId,float &outWeight) const
{
	if(flexId >= m_flexWeights.size())
		return false;
	outWeight = m_flexWeights.at(flexId);
	return true;
}

bool CFlexComponent::CalcFlexValue(uint32_t flexId,float &val) const {return const_cast<CFlexComponent*>(this)->UpdateFlexWeight(flexId,val,false);}

bool CFlexComponent::UpdateFlexWeight(uint32_t flexId,float &val,bool storeInCache)
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr || flexId >= m_updatedFlexWeights.size())
		return false;
	if(m_updatedFlexWeights.at(flexId))
	{
		val = m_flexWeights.at(flexId);
		return true;
	}
	auto *flex = mdl->GetFlex(flexId);
	if(flex == nullptr)
		return false;
	auto &ops = flex->GetOperations();
	std::stack<float> opStack {};
	for(auto &op : ops)
	{
		switch(op.type)
		{
			case Flex::Operation::Type::None:
				break;
			case Flex::Operation::Type::Const:
				opStack.push(op.d.value);
				break;
			case Flex::Operation::Type::Fetch:
			{
				auto val = 0.f;
				if(GetScaledFlexController(op.d.index,val) == false)
					return false;
				opStack.push(val);
				break;
			}
			case Flex::Operation::Type::Fetch2:
			{
				auto val = 0.f;
				if(UpdateFlexWeight(op.d.index,val,storeInCache) == false)
					return false;
				opStack.push(val);
				break;
			}
			case Flex::Operation::Type::Add:
			{
				if(opStack.size() < 2)
					return false;
				auto r = opStack.top();
				opStack.pop();
				auto &l = opStack.top();
				l += r;
				break;
			}
			case Flex::Operation::Type::Sub:
			{
				if(opStack.size() < 2)
					return false;
				auto r = opStack.top();
				opStack.pop();
				auto &l = opStack.top();
				l -= r;
				break;
			}
			case Flex::Operation::Type::Mul:
			{
				if(opStack.size() < 2)
					return false;
				auto r = opStack.top();
				opStack.pop();
				auto &l = opStack.top();
				l *= r;
				break;
			}
			case Flex::Operation::Type::Div:
			{
				if(opStack.size() < 2)
					return false;
				auto r = opStack.top();
				opStack.pop();
				auto &l = opStack.top();
				if(r != 0.f)
					l /= r;
				break;
			}
			case Flex::Operation::Type::Neg:
			{
				if(opStack.size() < 1)
					return false;
				auto &r = opStack.top();
				r = -r;
				break;
			}
			case Flex::Operation::Type::Exp:
				break;
			case Flex::Operation::Type::Open:
				break;
			case Flex::Operation::Type::Close:
				break;
			case Flex::Operation::Type::Comma:
				break;
			case Flex::Operation::Type::Max:
			{
				if(opStack.size() < 2)
					return false;
				auto r = opStack.top();
				opStack.pop();
				auto &l = opStack.top();
				l = umath::max(l,r);
				break;
			}
			case Flex::Operation::Type::Min:
			{
				if(opStack.size() < 2)
					return false;
				auto r = opStack.top();
				opStack.pop();
				auto &l = opStack.top();
				l = umath::min(l,r);
				break;
			}
			case Flex::Operation::Type::TwoWay0:
			{
				auto val = 0.f;
				if(GetScaledFlexController(op.d.index,val) == false)
					return false;
				opStack.push(1.f -(umath::min(umath::max(val +1.f,0.f),1.f)));
				break;
			}
			case Flex::Operation::Type::TwoWay1:
			{
				auto val = 0.f;
				if(GetScaledFlexController(op.d.index,val) == false)
					return false;
				opStack.push(umath::min(umath::max(val,0.f),1.f));
				break;
			}
			case Flex::Operation::Type::NWay:
			{
				auto v = 0.f;
				if(GetScaledFlexController(op.d.index,v) == false)
					return false;
				auto valueControllerIndex = op.d.index;
				opStack.pop();

				auto flValue = 0.f;
				if(GetScaledFlexController(valueControllerIndex,flValue) == false)
					return false;

				auto filterRampW = opStack.top();
				opStack.pop();
				auto filterRampZ = opStack.top();
				opStack.pop();
				auto filterRampY = opStack.top();
				opStack.pop();
				auto filterRampX = opStack.top();
				opStack.pop();

				auto greaterThanX = umath::min(1.f,(-umath::min(0.f,(filterRampX -flValue))));
				auto lessThanY = umath::min(1.f,(-umath::min(0.f,(flValue -filterRampY))));
				auto remapX = umath::min(umath::max((flValue -filterRampX) /(filterRampY -filterRampX),0.f),1.f);
				auto greaterThanEqualY = -(umath::min(1.f,(-umath::min(0.f,(flValue -filterRampY)))) -1.f);
				auto lessThanEqualZ = -(umath::min(1.f,(-umath::min(0.f,(filterRampZ -flValue)))) -1.f);
				auto greaterThanZ = umath::min(1.f,(-umath::min(0.f,(filterRampZ -flValue))));
				auto lessThanW = umath::min(1.f,(-umath::min(0.f,(flValue -filterRampW))));
				auto remapZ = (1.f -(umath::min(umath::max((flValue -filterRampZ) /(filterRampW -filterRampZ),0.f),1.f)));

				auto expValue = ((greaterThanX *lessThanY) *remapX) +(greaterThanEqualY *lessThanEqualZ) +((greaterThanZ *lessThanW) *remapZ);

				opStack.push(expValue *v);
				break;
			}
			case Flex::Operation::Type::Combo:
			{
				if(opStack.size() < op.d.index)
					return false;
				auto v = (op.d.index > 0) ? 1.f : 0.f;;
				for(auto i=decltype(op.d.index){0};i<op.d.index;++i)
				{
					v *= opStack.top();
					opStack.pop();
				}
				opStack.push(v);
				break;
			}
			case Flex::Operation::Type::Dominate:
			{
				if(opStack.size() < op.d.index +1)
					return false;
				auto v = (op.d.index > 0) ? 1.f : 0.f;
				for(auto i=decltype(op.d.index){0};i<op.d.index;++i)
				{
					v *= opStack.top();
					opStack.pop();
				}
				opStack.top() *= 1.f -v;
				break;
			}
			case Flex::Operation::Type::DMELowerEyelid:
			{
				auto pCloseLidV = 0.f;
				if(GetScaledFlexController(op.d.index,pCloseLidV) == false)
					return false;
				auto &pCloseLidVController = *mdl->GetFlexController(op.d.index);
				auto flCloseLidV = (umath::min(umath::max((pCloseLidV -pCloseLidVController.min) /(pCloseLidVController.max -pCloseLidVController.min),0.f),1.f));

				auto closeLidIndex = std::lroundf(opStack.top()); // Index is an integer stored as float, we'll round to make sure we get the right value
				opStack.pop();

				auto pCloseLid = 0.f;
				if(GetScaledFlexController(closeLidIndex,pCloseLid) == false)
					return false;
				auto &pCloseLidController = *mdl->GetFlexController(closeLidIndex);
				auto flCloseLid = (umath::min(umath::max((pCloseLid -pCloseLidController.min) /(pCloseLidController.max -pCloseLidController.min),0.f),1.f));

				opStack.pop();

				auto eyeUpDownIndex = std::lroundf(opStack.top()); // Index is an integer stored as float, we'll round to make sure we get the right value
				opStack.pop();
				auto pEyeUpDown = 0.f;
				if(GetScaledFlexController(eyeUpDownIndex,pEyeUpDown) == false)
					return false;
				auto &pEyeUpDownController = *mdl->GetFlexController(eyeUpDownIndex);
				auto flEyeUpDown = (-1.f +2.f *(umath::min(umath::max((pEyeUpDown -pEyeUpDownController.min) /(pEyeUpDownController.max -pEyeUpDownController.min),0.f),1.f)));

				opStack.push(umath::min(1.f,(1.f -flEyeUpDown)) *(1 -flCloseLidV) *flCloseLid);
				break;
			}
			case Flex::Operation::Type::DMEUpperEyelid:
			{
				auto pCloseLidV = 0.f;
				if(GetScaledFlexController(op.d.index,pCloseLidV) == false)
					return false;
				auto &pCloseLidVController = *mdl->GetFlexController(op.d.index);
				auto flCloseLidV = (umath::min(umath::max((pCloseLidV -pCloseLidVController.min) /(pCloseLidVController.max -pCloseLidVController.min),0.f),1.f));

				auto closeLidIndex = std::lroundf(opStack.top()); // Index is an integer stored as float, we'll round to make sure we get the right value
				opStack.pop();

				auto pCloseLid = 0.f;
				if(GetScaledFlexController(closeLidIndex,pCloseLid) == false)
					return false;
				auto &pCloseLidController = *mdl->GetFlexController(closeLidIndex);
				auto flCloseLid = (umath::min(umath::max((pCloseLid -pCloseLidController.min) /(pCloseLidController.max -pCloseLidController.min),0.f),1.f));

				opStack.pop();

				auto eyeUpDownIndex = std::lroundf(opStack.top()); // Index is an integer stored as float, we'll round to make sure we get the right value
				opStack.pop();
				auto pEyeUpDown = 0.f;
				if(GetScaledFlexController(eyeUpDownIndex,pEyeUpDown) == false)
					return false;
				auto &pEyeUpDownController = *mdl->GetFlexController(eyeUpDownIndex);
				auto flEyeUpDown = (-1.f +2.f *(umath::min(umath::max((pEyeUpDown -pEyeUpDownController.min) /(pEyeUpDownController.max -pEyeUpDownController.min),0.f),1.f)));

				opStack.push(umath::min(1.f,(1.f +flEyeUpDown)) *flCloseLidV *flCloseLid);
				break;
			}
		}
	}
	if(opStack.size() != 1) // If we don't have a single result left on the stack something went wrong
		return false;
	val = opStack.top();
	if(storeInCache)
	{
		m_flexWeights.at(flexId) = val;
		m_updatedFlexWeights.at(flexId) = true;
	}
	return true;
}
void CFlexComponent::UpdateSoundPhonemes(CALSound &snd)
{
	al::impl::BufferBase *buf = snd.GetBuffer();
	if(buf == nullptr)
		buf = snd.GetDecoder();
	auto userData = (buf != nullptr) ? buf->GetUserData() : nullptr;
	if(userData == nullptr)
		return;
	auto mdlComponent = GetEntity().GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return;
	auto &phonemeMap = mdl->GetPhonemeMap();
	auto &phonemeData = *static_cast<se::SoundPhonemeData*>(userData.get());
	auto t = snd.GetTimeOffset();
	for(auto itWord=phonemeData.words.begin();itWord!=phonemeData.words.end();++itWord)
	{
		auto &word = *itWord;
		if(t < word.tStart || t >= word.tEnd)
			continue;
		for(auto itPhoneme=word.phonemes.begin();itPhoneme!=word.phonemes.end();++itPhoneme)
		{
			auto &phoneme = *itPhoneme;
			if(t < phoneme.tStart || t >= phoneme.tEnd)
				continue;
			auto it = phonemeMap.phonemes.find(phoneme.phoneme);
			if(it != phonemeMap.phonemes.end())
			{
				// Find next phoneme to interpolate
				se::SoundPhonemeData::PhonemeData *nextPhoneme = nullptr;
				PhonemeInfo *nextPhonemeInfo = nullptr;
				auto itPhonemeNext = itPhoneme +1;
				if(itPhonemeNext != word.phonemes.end())
					nextPhoneme = &(*itPhonemeNext);
				else
				{
					auto itWordNext = itWord +1;
					if(itWordNext != phonemeData.words.end())
					{
						auto &nextWord = *itWordNext;
						if(nextWord.phonemes.empty() == false)
							nextPhoneme = &nextWord.phonemes.front();
					}
				}
				if(nextPhoneme != nullptr && umath::abs(nextPhoneme->tStart -phoneme.tEnd) < 0.1f)
				{
					auto it = phonemeMap.phonemes.find(nextPhoneme->phoneme);
					if(it != phonemeMap.phonemes.end())
						nextPhonemeInfo = &it->second;
				}
				else
					nextPhoneme = nullptr;
				//

				auto weight = 1.f -(t -phoneme.tStart) /(phoneme.tEnd -phoneme.tStart);
				for(auto &pair : it->second.flexControllers)
				{
					auto id = 0u;
					if(mdl->GetFlexControllerId(pair.first,id) == false)
						continue;
					auto flexWeight = pair.second *weight;
					if(nextPhonemeInfo != nullptr)
					{
						auto it = nextPhonemeInfo->flexControllers.find(pair.first);
						if(it != nextPhonemeInfo->flexControllers.end())
							flexWeight = umath::lerp(pair.second,it->second,1.f -weight); // Interpolate between current phoneme and next phoneme
					}
					auto curFlexWeight = 0.f;
					if(GetFlexController(id,curFlexWeight) == true)
						SetFlexController(id,flexWeight,0.1f);
				}
			}
			break;
		}
	}
}
#pragma optimize("",on)
