#include "GameFramework.h"
#include "DebugSceneGenerator.h"
#include "Engine/ECS/Component/Components/RendererComponents/ScreenPostEffectTag/ScreenPostEffectTag.h"
#include "Engine/Core/Utility/Tools/Assert.h"

#include "Engine/ECS/Component/Components/ComputeComponents/Transform/Transform.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Agent/AgentIntentComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Animator/Animator.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/MeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/DissolveMeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Sprite/SpriteRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Text/TextRenderer.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIGroupComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIElementComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/BoxCollider2D.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Rigidbody2D/Rigidbody2D.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/BoxCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/CircleCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/SphereCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Audio/BGMPlayer.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Audio/SEPlayer.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Animation/AnimationPlayer.h"
#include "Engine/ECS/Component/Components/RendererComponents/SkinMesh/SkinMeshRenderer.h"

using namespace ONEngine;

/// std
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>


/// engine
#include "Engine/Core/Utility/Input/Input.h"
#include "Engine/Core/Utility/Time/Time.h"
#include "Engine/Core/Utility/Time/CPUTimeStamp.h"
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Script/Script.h"
#include "Engine/Core/Threading/ThreadPool.h"
#include "Engine/Core/Event/FrameEventQueue.h"
#include "Engine/ECS/System/Audio/AudioPlaybackSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/ParticleSystem2D/ParticleSystem2D.h"
#include "Engine/ECS/System/ParticleSystem2DUpdateSystem/ParticleSystem2DUpdateSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/ParticleSystem/ParticleSystem.h"
#include "Engine/ECS/System/ParticleSystemUpdateSystem/ParticleSystemUpdateSystem.h"

GameFramework::GameFramework() {}
GameFramework::~GameFramework() {
	/// gpuの処理が終わるまで待つ
	dxManager_->GetDxCommand()->WaitForGpuComplete();

	/// シーン再生中なら停止処理を行う
	if (DebugConfig::isDebugging) {
		DebugConfig::isDebugging = false;
		if (sceneManager_) {
			// sceneManager_->ReloadScene(true); // Prevent C# ECS cleanups during engine shutdown
			sceneManager_->ClearTemporarySavedSceneName();
		}
	}

	/// debug用のシーンを保存
	// if (sceneManager_ && entityComponentSystem_) {
	// 	sceneManager_->SaveScene("Debug", entityComponentSystem_->GetECSGroup("Debug"));
	// }

	// ライフサイクルの依存関係を解決するため、明示的に先に破棄
	editorManager_.reset();
	imGuiManager_->Finalize();
	imGuiManager_.reset();

	// ECSの破棄前にMonoScriptEngineをFinalizeしてC#側のラッパーをクリーンアップ
	MonoScriptEngine::GetInstance().Finalize();

	entityComponentSystem_.reset();

	Time::Finalize();
	Input::Finalize();
	Console::Finalize();
	ThreadPool::Instance().Shutdown();

	/// engineの終了処理
	windowManager_->Finalize();
}

void GameFramework::Initialize(const GameFrameworkConfig& startSetting) {

	/// 初期化にかかる時間の計測開始
	auto startTime = std::chrono::high_resolution_clock::now();

	/// ログ出力の初期化
	Console::Initialize();

	/// エンジン設定のロード
	EngineConfig::LoadConfig();

	CreateSubsystems();
	InitializeCore(startSetting);
	InitializeGraphics();
	InitializeECS();
	InitializeEditor(startSetting);

	/// 初期化にかかった時間の計測終了と出力
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
	Console::Log("################################################################################");
	Console::Log("#");
	Console::Log("# Initialization completed in " + std::to_string(duration) + " ms");
	Console::Log("#");
	Console::Log("################################################################################");

}

void GameFramework::CreateSubsystems() {
	dxManager_ = std::make_unique<DxManager>();
	windowManager_ = std::make_unique<WindowManager>(dxManager_.get());
	entityComponentSystem_ = std::make_unique<EntityComponentSystem>(dxManager_.get());
	renderingFramework_ = std::make_unique<RenderingFramework>();
	sceneManager_ = std::make_unique<SceneManager>(entityComponentSystem_.get());

	editorManager_ = std::make_unique<Editor::EditorManager>(entityComponentSystem_.get());
	imGuiManager_ = std::make_unique<Editor::ImGuiManager>(dxManager_.get(), windowManager_.get(), entityComponentSystem_.get(), editorManager_.get(), sceneManager_.get());
}

void GameFramework::InitializeCore(const GameFrameworkConfig& config) {
	dxManager_->Initialize();
	ThreadPool::Instance().Initialize(dxManager_->GetDxDevice(), 4);
	windowManager_->Initialize();

	/// main windowの生成
#ifdef DEBUG_MODE
	UINT style = WS_OVERLAPPEDWINDOW;
	style &= ~WS_THICKFRAME;
	windowManager_->GenerateWindow(config.windowName + L" : debug mode", DebugConfig::kDebugWindowSize, WindowManager::WindowType::Main, style);
#else
	windowManager_->GenerateWindow(config.windowName, config.windowSize, WindowManager::WindowType::Main);
#endif // DEBUG_MODE

	Time::Initialize();
}

void GameFramework::InitializeGraphics() {
	renderingFramework_->Initialize(dxManager_.get(), windowManager_.get(), entityComponentSystem_.get());
}

void GameFramework::InitializeECS() {
	MonoScriptEngine::GetInstance().SetEcsPtr(entityComponentSystem_.get());
	MonoScriptEngine::GetInstance().Initialize();

	entityComponentSystem_->Initialize(renderingFramework_->GetAssetCollection());

	/// input systemの初期化
	Input::Initialize(windowManager_.get(), imGuiManager_.get());

	/// scene managerの初期化
	sceneManager_->Initialize(renderingFramework_->GetAssetCollection());
	if (!EngineConfig::isTestMode) {
		LoadDebugScene();
	}

	if (EngineConfig::isTestMode) {
		DebugConfig::isDebugging = true;
		DebugConfig::isPause = false;
	}
}

void GameFramework::InitializeEditor(const GameFrameworkConfig& /*config*/) {
#ifdef DEBUG_MODE
	imGuiManager_->Initialize(renderingFramework_->GetAssetCollection());
	imGuiManager_->SetImGuiWindow(windowManager_->GetMainWindow());
	renderingFramework_->SetImGuiManager(imGuiManager_.get());
#endif // DEBUG_MODE

	editorManager_->Initialize(dxManager_.get(), renderingFramework_->GetShaderCompiler(), sceneManager_.get());
	SetEntityComponentSystemPtr(entityComponentSystem_->GetECSGroup("GameScene"), entityComponentSystem_->GetECSGroup("Debug"));
}

void GameFramework::Run() {

	/// game loopが終了するまで回す
	while(true) {
		Update();
		Draw();

		/// ウィンドウの終了リクエストを確認（非デバッグ時は即終了、デバッグ時はEditor側で処理）
#ifndef DEBUG_MODE
		if(windowManager_->IsCloseRequested()) {
			PostQuitMessage(0);
		}
#endif

		/// 破棄されたら終了
		if(windowManager_->GetMainWindow()->GetProcessMessage()) {
			break;
		}
	}

}

void GameFramework::Update() {
	/// 更新処理
	Input::Update();
	Time::Update();

#ifdef DEBUG_MODE
	// デバッガ再接続時の自動同期をトリガー
	MonoScriptEngine::GetInstance().UpdateDebuggerStatus();
#endif

	if (EngineConfig::isTestMode) {
		static int testFrameCount = 0;
		testFrameCount++;

		if (EngineConfig::testScene == "PostEffectTest" && testFrameCount == 60) {
			auto* ecsGroup = entityComponentSystem_->GetECSGroup("GameScene");
			ONEngine::Assert(ecsGroup != nullptr, "ecsGroup 'GameScene' should not be null");
			if (ecsGroup) {
				auto* tagArray = ecsGroup->GetComponentArray<ScreenPostEffectTag>();
				ONEngine::Assert(tagArray != nullptr, "ScreenPostEffectTag component array should exist in GameScene");
				if (tagArray && !tagArray->GetUsedComponents().empty()) {
					ScreenPostEffectTag* tag = nullptr;
					for (auto* comp : tagArray->GetUsedComponents()) {
						if (comp && comp->enable) {
							tag = comp;
							break;
						}
					}
					ONEngine::Assert(tag != nullptr, "Active ScreenPostEffectTag should exist in GameScene");
					if (tag) {
						bool isFisheyeEnabled = tag->GetPostEffectEnable(PostEffectType_Fisheye);
						bool isWaterDistortionEnabled = tag->GetPostEffectEnable(PostEffectType_WaterDistortion);
						bool isWaterCausticsEnabled = tag->GetPostEffectEnable(PostEffectType_WaterCausticsLightShafts);
						bool isWaterColorGradingEnabled = tag->GetPostEffectEnable(PostEffectType_WaterColorGrading);
						bool isWaterDepthFogEnabled = tag->GetPostEffectEnable(PostEffectType_WaterDepthFogVignette);

						ONEngine::Assert(isFisheyeEnabled, "Fisheye posteffect should still be enabled in GameScene");
						ONEngine::Assert(isWaterDistortionEnabled, "WaterDistortion posteffect should still be enabled in GameScene");
						ONEngine::Assert(isWaterCausticsEnabled, "WaterCaustics posteffect should still be enabled in GameScene");
						ONEngine::Assert(isWaterColorGradingEnabled, "WaterColorGrading posteffect should still be enabled in GameScene");
						ONEngine::Assert(isWaterDepthFogEnabled, "WaterDepthFog posteffect should still be enabled in GameScene");

						ONEngine::Assert(tag->GetPostEffectWidth() == -1, "Default postEffectWidth should be -1");
						ONEngine::Assert(tag->GetPostEffectHeight() == -1, "Default postEffectHeight should be -1");

						tag->SetPostEffectWidth(800);
						tag->SetPostEffectHeight(600);
						ONEngine::Assert(tag->GetPostEffectWidth() == 800, "postEffectWidth should be 800 after setting");
						ONEngine::Assert(tag->GetPostEffectHeight() == 600, "postEffectHeight should be 600 after setting");
						
						Vector2 size = ScreenPostEffectTag::GetDispatchSize(ecsGroup, entityComponentSystem_.get());
						ONEngine::Assert(size.x == 800.0f, "GetDispatchSize.x should be 800");
						ONEngine::Assert(size.y == 600.0f, "GetDispatchSize.y should be 600");

						ONEngine::Assert(tag->GetPostEffectStartX() == 0, "Default postEffectStartX should be 0");
						ONEngine::Assert(tag->GetPostEffectStartY() == 0, "Default postEffectStartY should be 0");
						ONEngine::Assert(tag->GetPostEffectPivot() == 0, "Default postEffectPivot should be 0");

						Vector2 offsetDefault = ScreenPostEffectTag::GetDispatchStartOffset(ecsGroup, entityComponentSystem_.get());
						ONEngine::Assert(offsetDefault.x == 0.0f, "GetDispatchStartOffset.x should be 0 by default");
						ONEngine::Assert(offsetDefault.y == 0.0f, "GetDispatchStartOffset.y should be 0 by default");

						tag->SetPostEffectStartX(500);
						tag->SetPostEffectStartY(400);
						ONEngine::Assert(tag->GetPostEffectStartX() == 500, "postEffectStartX should be 500 after setting");
						ONEngine::Assert(tag->GetPostEffectStartY() == 400, "postEffectStartY should be 400 after setting");

						// Top-Left pivot mode (0)
						Vector2 offsetTopLeft = ScreenPostEffectTag::GetDispatchStartOffset(ecsGroup, entityComponentSystem_.get());
						ONEngine::Assert(offsetTopLeft.x == 500.0f, "GetDispatchStartOffset.x should be 500 in Top-Left mode");
						ONEngine::Assert(offsetTopLeft.y == 400.0f, "GetDispatchStartOffset.y should be 400 in Top-Left mode");

						// Center pivot mode (1)
						tag->SetPostEffectPivot(1);
						ONEngine::Assert(tag->GetPostEffectPivot() == 1, "postEffectPivot should be 1 after setting");

						Vector2 offsetCenter = ScreenPostEffectTag::GetDispatchStartOffset(ecsGroup, entityComponentSystem_.get());
						// 500 - 800/2 = 100
						// 400 - 600/2 = 100
						ONEngine::Assert(offsetCenter.x == 100.0f, "GetDispatchStartOffset.x should be 100 in Center mode");
						ONEngine::Assert(offsetCenter.y == 100.0f, "GetDispatchStartOffset.y should be 100 in Center mode");
					}
				}
			}
		}

		if (EngineConfig::testScene == "Rigidbody2DTest" && testFrameCount == 60) {
			auto* ecsGroup = entityComponentSystem_->GetECSGroup("Rigidbody2DTest");
			ONEngine::Assert(ecsGroup != nullptr, "ecsGroup 'Rigidbody2DTest' should not be null");
			if (ecsGroup) {
				auto* rbArray = ecsGroup->GetComponentArray<Rigidbody2D>();
				ONEngine::Assert(rbArray != nullptr, "Rigidbody2D array should exist");
				if (rbArray && rbArray->GetUsedComponents().size() >= 2) {
					auto used = rbArray->GetUsedComponents();
					Rigidbody2D* realRbA = nullptr;
					Rigidbody2D* realRbB = nullptr;
					for (auto* rb : used) {
						if (rb->GetOwner()->GetName() == "EntityA") realRbA = rb;
						if (rb->GetOwner()->GetName() == "EntityB") realRbB = rb;
					}
					
					ONEngine::Assert(realRbA != nullptr && realRbB != nullptr, "Could not find EntityA and EntityB");
					if (realRbA && realRbB) {
						Vector2 velA = realRbA->GetVelocity();
						Vector2 velB = realRbB->GetVelocity();
						
						ONEngine::Assert(velA.x < 0.0f, "EntityA velocity.x should be negative after collision");
						ONEngine::Assert(velB.x > 0.0f, "EntityB velocity.x should be positive after collision");
					}
				}
			}
		}

		if (EngineConfig::testScene == "ComponentEnableTest" && testFrameCount == 60) {
			auto* ecsGroup = entityComponentSystem_->GetECSGroup("ComponentEnableTest");
			ONEngine::Assert(ecsGroup != nullptr, "ecsGroup 'ComponentEnableTest' should not be null");
			if (ecsGroup) {
				#define ASSERT_COMP_DISABLED(CompType) \
				{ \
					auto* array = ecsGroup->GetComponentArray<CompType>(); \
					ONEngine::Assert(array != nullptr, #CompType " array should exist"); \
					if (array && !array->GetUsedComponents().empty()) { \
						auto* comp = array->GetUsedComponents().front(); \
						ONEngine::Assert(comp != nullptr, #CompType " component should exist"); \
						ONEngine::Assert(comp->enable == 0, #CompType " should be disabled by C# script"); \
					} \
				}

				ASSERT_COMP_DISABLED(Transform);
				ASSERT_COMP_DISABLED(MeshRenderer);
				ASSERT_COMP_DISABLED(DissolveMeshRenderer);
				ASSERT_COMP_DISABLED(SpriteRenderer);
				ASSERT_COMP_DISABLED(TextRenderer);
				ASSERT_COMP_DISABLED(BoxCollider2D);
				ASSERT_COMP_DISABLED(CameraComponent);
				ASSERT_COMP_DISABLED(AgentIntentComponent);
				ASSERT_COMP_DISABLED(UIGroupComponent);
				ASSERT_COMP_DISABLED(UIElementComponent);
				ASSERT_COMP_DISABLED(BGMPlayer);
				ASSERT_COMP_DISABLED(SEPlayer);
				ASSERT_COMP_DISABLED(BoxCollider);
				ASSERT_COMP_DISABLED(CircleCollider);
				ASSERT_COMP_DISABLED(SphereCollider);
				ASSERT_COMP_DISABLED(AnimationPlayer);
				ASSERT_COMP_DISABLED(SkinMeshRenderer);

				{
					auto* array = ecsGroup->GetComponentArray<Script>();
					ONEngine::Assert(array != nullptr, "Script array should exist");
					if (array && !array->GetUsedComponents().empty()) {
						auto* comp = array->GetUsedComponents().front();
						ONEngine::Assert(comp != nullptr, "Script component should exist");
						bool scriptEnable = comp->GetEnable("ComponentEnableTestScript");
						ONEngine::Assert(scriptEnable == false, "ComponentEnableTestScript should be disabled");
					}
				}

				#undef ASSERT_COMP_DISABLED
			}
		}

		if (EngineConfig::testScene == "ParticlePersistenceTest") {
			auto* ecsGroup = entityComponentSystem_->GetECSGroup("ParticlePersistenceTest");
			ONEngine::Assert(ecsGroup != nullptr, "ecsGroup 'ParticlePersistenceTest' should not be null");
			if (ecsGroup) {
				if (testFrameCount == 20) {
					auto* psArray = ecsGroup->GetComponentArray<ParticleSystem2D>();
					ONEngine::Assert(psArray != nullptr, "ParticleSystem2D array should exist");
					if (psArray && !psArray->GetUsedComponents().empty()) {
						auto* ps = psArray->GetUsedComponents().front();
						ONEngine::Assert(ps != nullptr, "ParticleSystem2D component should exist");
						ONEngine::Assert(ps->aliveCount > 0, "Particles should be emitted by frame 20");
					}
				}
				else if (testFrameCount == 40) {
					// Simulate playback stop
					DebugConfig::isDebugging = false;
					if (sceneManager_) {
						sceneManager_->ReloadScene(true);
					}
				}
				else if (testFrameCount == 60) {
					// Verify ghost particles are cleared
					auto* updateSys = ecsGroup->GetSystem<ParticleSystem2DUpdateSystem>();
					ONEngine::Assert(updateSys != nullptr, "ParticleSystem2DUpdateSystem should exist");
					if (updateSys) {
						ONEngine::Assert(updateSys->GetGhosts().empty(), "Ghost particles should be cleared after playback stops");
					}
				}
			}
		}

		if (EngineConfig::testScene == "ParticleRotationTest") {
			auto* ecsGroup = entityComponentSystem_->GetECSGroup("ParticleRotationTest");
			ONEngine::Assert(ecsGroup != nullptr, "ecsGroup 'ParticleRotationTest' should not be null");
			if (ecsGroup) {
				if (testFrameCount == 20) {
					auto* psArray = ecsGroup->GetComponentArray<ParticleSystem>();
					ONEngine::Assert(psArray != nullptr, "ParticleSystem array should exist");
					if (psArray && !psArray->GetUsedComponents().empty()) {
						auto* ps = psArray->GetUsedComponents().front();
						ONEngine::Assert(ps != nullptr, "ParticleSystem component should exist");
						ONEngine::Assert(ps->aliveCount > 0, "Particles should be emitted by frame 20");
						
						bool rotated = false;
						for (size_t i = 0; i < ps->aliveCount; ++i) {
							if (ps->particles[i].rotation != 0.0f) {
								rotated = true;
								break;
							}
						}
						ONEngine::Assert(rotated, "Particles should rotate over lifetime");
					}
				}
			}
		}
		if (testFrameCount >= EngineConfig::testDuration) {
			nlohmann::json results;
			results["success"] = true;
			results["message"] = "Test duration reached without assertion failures.";
			results["frames"] = testFrameCount;
			std::ofstream ofs(EngineConfig::testOutputPath);
			if (ofs.is_open()) {
				ofs << results.dump(4);
				ofs.close();
			}
			PostQuitMessage(0);
			ExitProcess(0);
		}
	}

	renderingFramework_->HeapBindToCommandList();
	windowManager_->Update();

#ifdef DEBUG_MODE
	editorManager_->Update(renderingFramework_->GetAssetCollection());
	imGuiManager_->Update();

	CPUTimeStamp::GetInstance().BeginTimeStamp(CPUTimeStampID::ECSUpdate);
	entityComponentSystem_->DebuggingUpdate();
	entityComponentSystem_->OutsideOfUpdate();

	sceneManager_->Update();

	if (!DebugConfig::isDebugging) {
		wasPause_ = false;
	} else {
		bool currentPause = DebugConfig::isPause;
		if (currentPause != wasPause_) {
			wasPause_ = currentPause;
			if (currentPause) {
				const auto& activeGroups = entityComponentSystem_->GetActiveGroupNames();
				if (activeGroups.empty()) {
					if (auto* group = entityComponentSystem_->GetCurrentGroup()) {
						if (auto* audioSys = group->GetSystem<AudioPlaybackSystem>()) {
							audioSys->PauseAllAudio();
						}
					}
				} else {
					for (const auto& name : activeGroups) {
						if (auto* group = entityComponentSystem_->GetECSGroup(name)) {
							if (auto* audioSys = group->GetSystem<AudioPlaybackSystem>()) {
								audioSys->PauseAllAudio();
							}
						}
					}
				}
			} else {
				const auto& activeGroups = entityComponentSystem_->GetActiveGroupNames();
				if (activeGroups.empty()) {
					if (auto* group = entityComponentSystem_->GetCurrentGroup()) {
						if (auto* audioSys = group->GetSystem<AudioPlaybackSystem>()) {
							audioSys->ResumeAllAudio();
						}
					}
				} else {
					for (const auto& name : activeGroups) {
						if (auto* group = entityComponentSystem_->GetECSGroup(name)) {
							if (auto* audioSys = group->GetSystem<AudioPlaybackSystem>()) {
								audioSys->ResumeAllAudio();
							}
						}
					}
				}
			}
		}
	}

	///!< ゲームデバッグモードの場合は更新処理を行う
	if(DebugConfig::isDebugging && !DebugConfig::isPause) {
		entityComponentSystem_->Update();
	}
	CPUTimeStamp::GetInstance().EndTimeStamp(CPUTimeStampID::ECSUpdate);
#else
	CPUTimeStamp::GetInstance().BeginTimeStamp(CPUTimeStampID::ECSUpdate);
	editorManager_->Update(renderingFramework_->GetAssetCollection());
	entityComponentSystem_->DebuggingUpdate();
	entityComponentSystem_->OutsideOfUpdate();
	sceneManager_->Update();
	entityComponentSystem_->Update();
	CPUTimeStamp::GetInstance().EndTimeStamp(CPUTimeStampID::ECSUpdate);
#endif // DEBUG_MODE

	// 古いMonoドメインが残っていれば安全にアンロード（解放）する
	MonoScriptEngine::GetInstance().ClearPendingDomains();

	// ホットリロード要求フラグをクリアする
	MonoScriptEngine::GetInstance().SetIsHotReloadRequest(false);

	// Process all queued events for this frame
	FrameEventQueue::GetInstance().Flush();
}

void GameFramework::Draw() {
	/// 描画処理
	CPUTimeStamp::GetInstance().BeginTimeStamp(CPUTimeStampID::RenderUpdate);
	renderingFramework_->Draw();
	CPUTimeStamp::GetInstance().EndTimeStamp(CPUTimeStampID::RenderUpdate);
}

void GameFramework::LoadDebugScene() {
	DebugSceneGenerator::GenerateDefaultDebugSceneIfNeeded();
	sceneManager_->GetSceneIO()->Input("Debug", entityComponentSystem_->GetECSGroup("Debug"));
}


