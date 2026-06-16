#pragma once

#include "EntityComponentSystem.h"
#include "Engine/ECS/Component/Collection/ComponentCollection.h"

/// compute
#include "Engine/ECS/Component/Components/ComputeComponents/Script/Script.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Terrain/Terrain.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Light/Light.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Audio/AudioSource.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Effect/Effect.h"
#include "Engine/ECS/Component/Components/ComputeComponents/ParticleSystem/ParticleSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Terrain/TerrainCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Terrain/Grass/GrassField.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/BoxCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/SphereCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/ShadowCaster/ShadowCaster.h"
#include "Engine/ECS/Component/Components/ComputeComponents/VoxelTerrain/VoxelTerrain.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Agent/AgentIntentComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Animator/Animator.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Animation/AnimationPlayer.h"

/// renderer
#include "Engine/ECS/Component/Components/RendererComponents/Skybox/Skybox.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/MeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/DissolveMeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/CustomMeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/SkinMesh/SkinMeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Sprite/SpriteRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Primitive/Line2DRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Primitive/Line3DRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/ScreenPostEffectTag/ScreenPostEffectTag.h"

namespace ONEngine {

inline void AddComponentFactoryFunction(ComponentCollection* compCollection) {
	/// compute
	compCollection->RegisterComponentFactory<Script>();
	compCollection->RegisterComponentFactory<Transform>();
	compCollection->RegisterComponentFactory<DirectionalLight>();
	compCollection->RegisterComponentFactory<AudioSource>();
	compCollection->RegisterComponentFactory<Variables>();
	compCollection->RegisterComponentFactory<Effect>();
	compCollection->RegisterComponentFactory<ParticleSystem>();
	compCollection->RegisterComponentFactory<Terrain>();
	compCollection->RegisterComponentFactory<GrassField>();
	compCollection->RegisterComponentFactory<TerrainCollider>();
	compCollection->RegisterComponentFactory<CameraComponent>();
	compCollection->RegisterComponentFactory<ShadowCaster>();
	compCollection->RegisterComponentFactory<VoxelTerrain>();
	compCollection->RegisterComponentFactory<AgentIntentComponent>();
	compCollection->RegisterComponentFactory<Animator>();
	compCollection->RegisterComponentFactory<AnimationPlayer>();


	/// renderer
	compCollection->RegisterComponentFactory<MeshRenderer>();
	compCollection->RegisterComponentFactory<DissolveMeshRenderer>();
	compCollection->RegisterComponentFactory<CustomMeshRenderer>();
	compCollection->RegisterComponentFactory<SkinMeshRenderer>();
	compCollection->RegisterComponentFactory<SpriteRenderer>();
	compCollection->RegisterComponentFactory<Line2DRenderer>();
	compCollection->RegisterComponentFactory<Line3DRenderer>();
	compCollection->RegisterComponentFactory<ScreenPostEffectTag>();
	compCollection->RegisterComponentFactory<Skybox>();

	/// collider
	compCollection->RegisterComponentFactory<SphereCollider>();
	compCollection->RegisterComponentFactory<BoxCollider>();
}

} /// namespace ONEngine