using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public static class AIUpdater {
    private static readonly Dictionary<uint, AgentIntentComponent> _componentCache = new Dictionary<uint, AgentIntentComponent>();

    // BehaviorTree と isPaused は AgentIntentComponent(ECSデータ)には乗せず、
    // AIUpdater が compId をキーに管理する
    private static readonly Dictionary<uint, BehaviorTree> _behaviorTrees = new Dictionary<uint, BehaviorTree>();
    private static readonly HashSet<uint> _pausedAgents = new HashSet<uint>();

    // エディタ表示用の状態保持
    public static string lastBossName = "None";
    public static string lastBossAction = "Idle";
    public static string lastBossPhase = "Intro";

    // --- BehaviorTree / isPaused 登録 API ---
    public static void SetBehaviorTree(uint compId, BehaviorTree bt) {
        if (bt == null) _behaviorTrees.Remove(compId);
        else _behaviorTrees[compId] = bt;
    }
    public static BehaviorTree GetBehaviorTree(uint compId) {
        _behaviorTrees.TryGetValue(compId, out var bt);
        return bt;
    }
    public static void SetPaused(uint compId, bool paused) {
        if (paused) _pausedAgents.Add(compId);
        else _pausedAgents.Remove(compId);
    }

    /// <summary>
    /// C++ から呼び出されるAI更新のメインエントリーポイント
    /// </summary>
    public unsafe static void UpdateIntents(AgentIntentComponent.BatchData* intentsDataPtr, int entityCount, float deltaTime, string groupName) {
        if (intentsDataPtr == null) return;

        if ((int)(Time.time * 5) % 100 == 0) {
        }

        RefreshCache(groupName);

        for (int i = 0; i < entityCount; i++) {
            AgentIntentComponent.BatchData* nativeData = intentsDataPtr + i;

            if (_componentCache.TryGetValue(nativeData->compId, out var component)) {
                if (component.entity == null || component.entity.Id == 0) {
                    continue;
                }

                if (!component.entity.enable) {
                    continue;
                }

                // AIが一時停止中ならスキップ
                if (_pausedAgents.Contains(nativeData->compId)) {
                    nativeData->desiredMoveDirection = Vector3.zero;
                    nativeData->isAttacking = 0;
                    continue;
                }

                // ビヘイビアツリーを実行
                BehaviorTree bt;
                if (_behaviorTrees.TryGetValue(nativeData->compId, out bt) && bt != null) {
                    if ((int)(Time.time * 5) % 100 == 0) {
                    }

                    component.desiredMoveDirection = Vector3.zero;
                    component.isAttacking = false;
                    component.targetEntityId = 0;

                    // Tick(float deltaTime) はフラット版 BehaviorTree に定義済み
                    bt.Tick(deltaTime);

                    // ボス状態の記録（エディタ用）
                    if (component.entity.name.IndexOf("Boss", StringComparison.OrdinalIgnoreCase) >= 0) {
                        lastBossName = component.entity.name;

                        // DeepestActiveNode はフラット版 BT では未実装のためスキップ
                        lastBossAction = "Idle";

                        // GetBlackboardValue<T>(string, T) はフラット版 BT に定義済み
                        float hpRatio = bt.GetBlackboardValue<float>("HPRatio", 1.0f);
                        if (hpRatio > 1.0f) lastBossPhase = "Intro";
                        else if (hpRatio >= 0.7f) lastBossPhase = "Phase 1";
                        else if (hpRatio >= 0.4f) lastBossPhase = "Phase 2";
                        else lastBossPhase = "Phase 3";
                    }

                    // フラット版 BT は Tick() 内で UpdateNodeStatus() を自動呼び出しするため
                    // GetAllNodeStatuses() は不要

                    nativeData->desiredMoveDirection = component.desiredMoveDirection;
                    nativeData->desiredRotation = component.desiredRotation;
                    nativeData->rotationSpeed = component.rotationSpeed;
                    nativeData->maxSpeed = component.maxSpeed;
                    nativeData->useDesiredRotation = (byte)(component.useDesiredRotation ? 1 : 0);
                    nativeData->isAttacking = (byte)(component.isAttacking ? 1 : 0);
                    nativeData->targetEntityId = component.targetEntityId;
                } else {
                    nativeData->desiredMoveDirection = Vector3.zero;
                    nativeData->isAttacking = 0;
                }
            } else {
                nativeData->desiredMoveDirection = Vector3.zero;
            }
        }

        GizmoBatch.SubmitBatch();

        var group = EntityComponentSystem.GetECSGroup(groupName);
        if (group != null) {
            ComponentBatchManager.SendAllBatches(group.componentCollection, groupName);
        }
    }

    private static void RefreshCache(string groupName) {
        var group = EntityComponentSystem.GetECSGroup(groupName);
        if (group == null) return;

        // GetArray<T>() は未実装のため TryGetArray + キャストを使う
        IComponentArray rawArray;
        if (!group.componentCollection.TryGetArray(typeof(AgentIntentComponent), out rawArray)) return;

        var array = rawArray as ComponentArray<AgentIntentComponent>;
        if (array == null) return;

        _componentCache.Clear();
        foreach (var comp in array.components) {
            if (comp != null) {
                _componentCache[comp.compId] = comp;
            }
        }
    }
}
