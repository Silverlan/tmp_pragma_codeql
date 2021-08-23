/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ANIMATED_COMPONENT_HPP__
#define __ANIMATED_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/types.hpp"
#include <sharedutils/property/util_property.hpp>

struct AnimationEvent;
namespace util {class Path;};
namespace pragma
{
	namespace animation
	{
		class AnimationManager;
		using PAnimationManager = std::shared_ptr<AnimationManager>;
		using ChannelValueSubmitter = std::function<void(panima::Channel&,uint32_t&,double)>;
	};
	class AnimationDriverComponent;
	class DLLNETWORK Animated2Component final
		: public BaseEntityComponent
	{
	public:
		static ComponentEventId EVENT_HANDLE_ANIMATION_EVENT;
		static ComponentEventId EVENT_ON_PLAY_ANIMATION;
		static ComponentEventId EVENT_ON_ANIMATION_COMPLETE;
		static ComponentEventId EVENT_ON_ANIMATION_START;
		static ComponentEventId EVENT_MAINTAIN_ANIMATIONS;
		static ComponentEventId EVENT_ON_ANIMATIONS_UPDATED;
		static ComponentEventId EVENT_PLAY_ANIMATION;
		static ComponentEventId EVENT_TRANSLATE_ANIMATION;
		static ComponentEventId EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);
		
		Animated2Component(BaseEntity &ent);
		void SetPlaybackRate(float rate);
		float GetPlaybackRate() const;
		const util::PFloatProperty &GetPlaybackRateProperty() const;

		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
		
		std::vector<animation::PAnimationManager> &GetAnimationManagers() {return m_animationManagers;}
		const std::vector<animation::PAnimationManager> &GetAnimationManagers() const {return const_cast<Animated2Component*>(this)->GetAnimationManagers();}
		animation::PAnimationManager AddAnimationManager();
		void RemoveAnimationManager(const animation::AnimationManager &player);
		void ClearAnimationManagers();

		bool MaintainAnimations(double dt);
		virtual void OnTick(double dt) override;
		void AdvanceAnimations(double dt);

		virtual void Initialize() override;
		void PlayAnimation(animation::AnimationManager &manager,panima::Animation &anim);

		AnimationDriverComponent *GetDriverComponent() {return m_driverComponent;}
		const AnimationDriverComponent *GetDriverComponent() const {return const_cast<Animated2Component*>(this)->GetDriverComponent();}
		
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		using BaseEntityComponent::Load;
	protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm,uint32_t version) override;
		void InitializeAnimationChannelValueSubmitters();
		void InitializeAnimationChannelValueSubmitters(animation::AnimationManager &manager);
		void ResetAnimation(const std::shared_ptr<Model> &mdl);
		util::PFloatProperty m_playbackRate = nullptr;
		std::vector<animation::PAnimationManager> m_animationManagers;
		AnimationDriverComponent *m_driverComponent = nullptr;
	};

	struct DLLNETWORK CEAnim2OnAnimationComplete
		: public ComponentEvent
	{
		CEAnim2OnAnimationComplete(int32_t animation,Activity activity);
		virtual void PushArguments(lua_State *l) override;
		int32_t animation;
		Activity activity;
	};
	struct DLLNETWORK CEAnim2HandleAnimationEvent
		: public ComponentEvent
	{
		CEAnim2HandleAnimationEvent(const AnimationEvent &animationEvent);
		virtual void PushArguments(lua_State *l) override;
		void PushArgumentVariadic(lua_State *l);
		const AnimationEvent &animationEvent;
	};
	struct DLLNETWORK CEAnim2OnPlayAnimation
		: public ComponentEvent
	{
		CEAnim2OnPlayAnimation(panima::AnimationId animation,pragma::FPlayAnim flags);
		virtual void PushArguments(lua_State *l) override;
		panima::AnimationId animation;
		pragma::FPlayAnim flags;
	};
	struct DLLNETWORK CEAnim2OnAnimationStart
		: public ComponentEvent
	{
		CEAnim2OnAnimationStart(int32_t animation,Activity activity,pragma::FPlayAnim flags);
		virtual void PushArguments(lua_State *l) override;
		int32_t animation;
		Activity activity;
		pragma::FPlayAnim flags;
	};
	struct DLLNETWORK CEAnim2TranslateAnimation
		: public ComponentEvent
	{
		CEAnim2TranslateAnimation(panima::AnimationId &animation,pragma::FPlayAnim &flags);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		panima::AnimationId &animation;
		pragma::FPlayAnim &flags;
	};
	struct DLLNETWORK CEAnim2MaintainAnimations
		: public ComponentEvent
	{
		CEAnim2MaintainAnimations(double deltaTime);
		virtual void PushArguments(lua_State *l) override;
		double deltaTime;
	};
	struct DLLNETWORK CEAnim2InitializeChannelValueSubmitter
		: public ComponentEvent
	{
		CEAnim2InitializeChannelValueSubmitter(util::Path &path);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		util::Path &path;
		animation::ChannelValueSubmitter submitter = nullptr;
	};
};

#endif
