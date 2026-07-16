using System;
using System.Collections.Generic;

/// <summary>
/// 別のビヘイビアツリー資産（JSONファイル）を動的に読み込み、
/// 一つのノードであるかのように実行する「サブツリー（Sub-tree）」ノード。
/// ロジックのモジュール化・再利用（例：索敵ロジックの共通化）に非常に有用。
/// UEの「Run Behavior」ノードに相当する。
/// </summary>
public class RunBehaviorNode : BehaviorNode
{
    /// <summary>
    /// 実行する対象のビヘイビアツリー資産のファイルパス。
    /// </summary>
    [SerializeField]
    public string treePath = "";

    // 読み込んだサブツリーのインスタンス
    private BehaviorTree _subTree = null;
    public BehaviorTree SubTree => _subTree;
    
    // 読み込みに失敗した場合に再試行を防ぐためのフラグ
    private bool _failedToLoad = false;

    /// <summary>
    /// ノードの実行処理。初回実行時にJSONからツリーを展開し、以後はそのルートに処理を委譲する。
    /// </summary>
    protected override NodeStatus Execute(Blackboard blackboard, Entity owner)
    {
        // 過去にロードに失敗している場合は即座にFailureを返す
        if (_failedToLoad) return NodeStatus.Failure;

        // 1. 初回実行時にサブツリーのロードを試みる
        if (_subTree == null && !string.IsNullOrEmpty(treePath))
        {
            try
            {
                _subTree = BehaviorTreeLoader.LoadFromFile(treePath, owner);
                if (_subTree != null)
                {
                }
                else {
                    _failedToLoad = true;
                }
            }
            catch (Exception)
            {
                _failedToLoad = true;
            }
        }

        // ロードできなかった場合（ファイルが存在しない、フォーマットエラー等）
        if (_subTree == null || _subTree.RootNode == null) return NodeStatus.Failure;

        // --- 修正：サブツリーの再実行時にBlackboardをクリアする ---
        // これにより、サブツリー内のアクションノードが持つ「Done」フラグなどがリセットされ、
        // 2回目以降の攻撃が正しく動作するようになる。
        // ResetToDefaults() を使うことで、JSONで定義された初期値（RockRadius等）は確実に復元されます。
        uint startKey = BehaviorTreeLoader.HashString("RunBehaviorStart_" + NodeIdHash);
        if (!blackboard.HasKey(startKey))
        {
            _subTree.Blackboard.ResetToDefaults();
            blackboard.SetFloat(startKey, Time.time);
        }

        // 2. サブツリーの実行
        _subTree.TickCount = this.Tree.TickCount; // Tick回数を同期

        // 共通キーの同期 (親 -> 子)
        SyncBlackboard(blackboard, _subTree.Blackboard);
        
        // 同期したキーを子ツリー側で「デフォルト」として扱う（リセット時に消えないようにする）
        string[] syncKeys = { "CurrentHP", "HPRatio", "TargetPosition", "TargetEntity" };
        foreach (var key in syncKeys)
        {
            _subTree.Blackboard.SaveAsDefault(BehaviorTreeLoader.HashString(key));
        }

        var status = _subTree.RootNode.Tick(_subTree.Blackboard, owner);
        
        // サブツリー内の実行中ノードを更新
        _subTree.ActiveNode = null;
        _subTree.UpdateActiveNodeRecursive(_subTree.RootNode);
        
        // 完了した場合は開始フラグを削除して、次回実行時に再度リセットがかかるようにする
        if (status != NodeStatus.Running)
        {
            blackboard.Remove(startKey);
        }

        return status;
    }

    public override void OnAbort(Blackboard blackboard, Entity owner)
    {
        blackboard.Remove(BehaviorTreeLoader.HashString("RunBehaviorStart_" + NodeIdHash));
        
        // サブツリーも中断処理を行う
        if (_subTree != null && _subTree.RootNode != null)
        {
            _subTree.AbortRecursive(_subTree.RootNode);
        }
    }

    private void SyncBlackboard(Blackboard parent, Blackboard child)
    {
        // 必要なキーを親から子へコピー（各フェーズで必要になる可能性が高いもの）
        string[] syncKeys = { "CurrentHP", "HPRatio", "TargetPosition", "TargetEntity" };
        foreach (var key in syncKeys)
        {
            uint hash = BehaviorTreeLoader.HashString(key);
            if (parent.HasKey(hash))
            {
                // 型を判別してコピー（Blackboard.csの内部構造に依存）
                // 本来はBlackboardにCopyメソッドなどがあるのが望ましい
                object val = parent.GetValueAsObject(hash);
                if (val is int i) child.SetInt(hash, i);
                else if (val is float f) child.SetFloat(hash, f);
                else if (val is bool b) child.SetBool(hash, b);
                else if (val is Vector3 v) child.SetVector3(hash, v);
                else if (val is string s) child.SetString(hash, s);
                else child.SetObject(hash, val);
            }
        }
    }
}

