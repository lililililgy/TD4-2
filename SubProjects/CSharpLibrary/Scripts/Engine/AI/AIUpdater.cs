using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public static class AIUpdater {
    private static readonly Dictionary<uint, AgentIntentComponent> _componentCache = new Dictionary<uint, AgentIntentComponent>();

    // エディタ表示用の状態保持
    public static string lastBossName = "None";
    public static string lastBossAction = "Idle";
    public static string lastBossPhase = "Intro";

    /// <summary>
    /// C++ から呼び出されるAI更新のメインエントリーポイント
    /// </summary>
    /// <param name="intentsDataPtr">AgentIntentComponentのネイティブ配列へのポインタ</param>
    /// <param name="entityCount">エンティティの数</param>
    /// <param name="deltaTime">フレーム時間</param>
    /// <param name="groupName">対象のECSグループ名</param>
    public unsafe static void UpdateIntents(AgentIntentComponent.BatchData* intentsDataPtr, int entityCount, float deltaTime, string groupName) {
        if (intentsDataPtr == null) return;

        // デバッグ用：どのグループが更新されているかログに出す（頻度を抑える）
        if ((int)(Time.time * 5) % 100 == 0) {
        }

        // キャッシュを更新
        RefreshCache(groupName);

        for (int i = 0; i < entityCount; i++) {
            AgentIntentComponent.BatchData* nativeData = intentsDataPtr + i;

            if (_componentCache.TryGetValue(nativeData->compId, out var component)) {
                // 安全策：Entityが未設定、IDが無効、または非アクティブな場合はスキップ
                if (component.entity == null || component.entity.Id == 0) {
                    continue;
                }

                // エンティティが非アクティブならスキップ
                if (!component.entity.enable) {
                    continue;
                }

                // AIが一時停止中ならスキップ
                if (component.isPaused) {
                    nativeData->desiredMoveDirection = Vector3.zero;
                    nativeData->isAttacking = 0;
                    continue;
                }

                // ビヘイビアツリーを実行
                if (component.behaviorTree != null) {
                    // 調査用ログ: どのAIが動いているか
                    
                    // 実行前にIntentをリセット（ツリー内で上書きされなければ停止する）
                    component.desiredMoveDirection = Vector3.zero;
                    component.isAttacking = false;
                    component.targetEntityId = 0;

                    // デバッグ用：毎フレームは多すぎるので定期的にログを出す
                    if ((int)(Time.time * 5) % 100 == 0) {
                    }

                    component.behaviorTree.Tick();

                    // ボス状態の記録（エディタ用）
                    // 名前が "Boss" を含むエンティティを監視対象とする
                    if (component.entity.name.IndexOf("Boss", StringComparison.OrdinalIgnoreCase) >= 0)
                    {
                        lastBossName = component.entity.name;
                        
                        // 最も深いノード（実行中の末端アクション）を取得
                        var deepest = component.behaviorTree.DeepestActiveNode;
                        lastBossAction = (deepest != null) ? deepest.name : "Idle";

                        // フェーズの特定 (BossMain.jsonの構成に準拠)
                        float hpRatio = component.behaviorTree.Blackboard.GetFloat(BehaviorTreeLoader.HashString("HPRatio"), 1.0f);
                        if (hpRatio > 1.0f) lastBossPhase = "Intro";
                        else if (hpRatio >= 0.7f) lastBossPhase = "Phase 1";
                        else if (hpRatio >= 0.4f) lastBossPhase = "Phase 2";
                        else lastBossPhase = "Phase 3";
                    }

                    // エディタ用：実行状態を同期
                    // 現在実行中のツリーのパスを使用してノードの状態をC++へ通知
                    component.behaviorTree.GetAllNodeStatuses(new Dictionary<uint, NodeStatus>());

                    // ツリーの実行結果（インテント）をネイティブデータに反映
                    nativeData->desiredMoveDirection = component.desiredMoveDirection;
                    nativeData->desiredRotation = component.desiredRotation;
                    nativeData->rotationSpeed = component.rotationSpeed;
                    nativeData->maxSpeed = component.maxSpeed;
                    nativeData->useDesiredRotation = (byte)(component.useDesiredRotation ? 1 : 0);
                    nativeData->isAttacking = (byte)(component.isAttacking ? 1 : 0);
                    nativeData->targetEntityId = component.targetEntityId;
                }
                else
                {
                    // ツリーがない場合は停止を意図する
                    nativeData->desiredMoveDirection = Vector3.zero;
                    nativeData->isAttacking = 0;
                }
            }
            else
            {
                // コンポーネントが見つからない場合も停止
                nativeData->desiredMoveDirection = Vector3.zero;
            }
        }

        // AIの更新終了後にGizmoデータを一括送信
        GizmoBatch.SubmitBatch();

        // --- 追加: BT内で更新されたTransformやMeshRendererのデータをC++へ強制同期 ---
        var group = EntityComponentSystem.GetECSGroup(groupName);
        if (group != null)
        {
            ComponentBatchManager.SendAllBatches(group.componentCollection, groupName);
        }
    }

    private static void RefreshCache(string groupName) {
        var group = EntityComponentSystem.GetECSGroup(groupName);
        if (group == null) return;

        var array = group.componentCollection.GetArray<AgentIntentComponent>();
        if (array == null) return;

        _componentCache.Clear();
        foreach (var comp in array.components) {
            if (comp != null) {
                _componentCache[comp.compId] = comp;
            }
        }
    }
}

