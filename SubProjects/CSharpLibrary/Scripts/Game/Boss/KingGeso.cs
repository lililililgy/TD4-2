using System;
using System.Collections.Generic;

public class KingGeso : MonoScript
{
    // Blackboardのキーをハッシュ化して定数として保持
    private static readonly uint CurrentHpKey = BehaviorTreeLoader.HashString("CurrentHP");
    private static readonly uint HpRatioKey = BehaviorTreeLoader.HashString("HPRatio");
    private static readonly uint TargetEntityKey = BehaviorTreeLoader.HashString("TargetEntity");

    [SerializeField]
    public string treePath = "Assets/AITrees/DefaultTree.json";
    [SerializeField]
    public int maxHp = 10;

    private AgentIntentComponent _intent;
    HP _hp;
    private readonly List<GesoHand> _hands = new List<GesoHand>();
    private Entity _targetEntity;
    private int _nextHandIndex;
    private bool _wasAttackRequested;


    //=============================================================
    // 初期化
    //=============================================================
    public override void Initialize()
    {

        _intent = entity.GetComponent<AgentIntentComponent>();
        if (_intent == null)
        {
            _intent = entity.AddComponent<AgentIntentComponent>();
        }

        // エディタで作成したツリーをロード
        _intent.LoadBehaviorTree(treePath);

        // HP is owned and managed by KingGeso.
        _hp = entity.GetScript<HP>();
        if (_hp == null)
        {
            _hp = entity.AddScript<HP>();
        }

        _hp.MAX_HP = maxHp > 0 ? maxHp : 1;
        _hp.Initialize();

        _hands.Clear();
        // 子エンティティを再帰的に探索してGesoHandを収集
        CollectHands(entity);
        _nextHandIndex = 0;
        _wasAttackRequested = false;
    }

    //=============================================================
    // 更新
    //=============================================================
    public override void Update()
    {
        // --- デバッグ用：HキーでHPを10%減らす ---
        if (Input.TriggerKey(KeyCode.H))
        {
            if (_hp != null)
            {
                _hp.TakeDamage(1);
            }
        }

        // --- デバッグ用：Jキーで攻撃を要求する ---
        if (Input.TriggerKey(KeyCode.J))
        {
            _wasAttackRequested = true;
        }

        //ボスの状態をBlackboardに同期
        SyncBossStateToBlackboard();
        //Blackboardからターゲットを取得
        UpdateTargetFromBlackboard();
        //手の状態を更新
        DispatchHandCommands();

        // --- デバッグ用：視線の表示 ---
        GizmoBatch.DrawRay(transform.position + Vector3.up * 2.0f, transform.forward * 5.0f, new Vector4(0, 1, 0, 1));
    }

    //=============================================================
    // ターゲットの設定
    //=============================================================
    public void SetTarget(Entity target)
    {
        _targetEntity = target;
        if (_intent != null && _intent.behaviorTree != null)
        {
            // Blackboardにターゲットを設定
            _intent.behaviorTree.Blackboard.SetObject(TargetEntityKey, target);
        }
    }

    //=============================================================
    // 手の収集
    //=============================================================
    private void CollectHands(Entity root)
    {
        // 再帰的に子エンティティを探索してGesoHandを収集
        uint childCount = root.GetChildCount();
        for (uint i = 0; i < childCount; i++)
        {
            // 子エンティティを取得
            Entity child = root.GetChild(i);
            if (child == null)
            {
                continue;
            }

            // GesoHandスクリプトを取得
            GesoHand hand = child.GetScript<GesoHand>();
            if (hand != null)
            {
                // GesoHandをリストに追加
                _hands.Add(hand);
            }

            // 再帰的に子エンティティを探索
            CollectHands(child);
        }
    }

    //=============================================================
    // Blackboardとの同期
    //=============================================================
    private void SyncBossStateToBlackboard()
    {
        if (_hp == null || _intent == null || _intent.behaviorTree == null)
        {
            return;
        }
        // Blackboardに現在のHPとHP比率を設定
        Blackboard blackboard = _intent.behaviorTree.Blackboard;
        blackboard.SetInt(CurrentHpKey, _hp.currentHp);
        blackboard.SetFloat(HpRatioKey, _hp.CurrentHpRatio());
    }

    //=============================================================
    // Blackboardからターゲットを取得
    //=============================================================
    private void UpdateTargetFromBlackboard()
    {
        if (_intent == null || _intent.behaviorTree == null)
        {
            return;
        }

        // Blackboardからターゲットエンティティを取得
        Entity blackboardTarget = _intent.behaviorTree.Blackboard.GetEntity(TargetEntityKey);
        if (blackboardTarget != null)
        {
            _targetEntity = blackboardTarget;
        }
    }

    //=============================================================
    // 手の状態を更新
    //=============================================================
    private void DispatchHandCommands()
    {
        if (_hands.Count == 0)
        {
            return;
        }

        // ターゲットがいない場合は手を待機状態にする
        if (_targetEntity == null)
        {
            foreach (GesoHand hand in _hands)
            {
                hand.CommandIdle();
            }
            _wasAttackRequested = false;
            return;
        }

        // ターゲットがいる場合は手をターゲットに向ける
        foreach (GesoHand hand in _hands)
        {
            hand.CommandAim(_targetEntity);
        }

        // 攻撃が要求されたかどうかを判定
        bool attackRequested = _intent != null && _intent.isAttacking;
       
        if (attackRequested && !_wasAttackRequested)
        {
            // 攻撃が要求され、前回は要求されていなかった場合に攻撃を試みる
            TryAttackWithNextHand();
        }

        _wasAttackRequested = attackRequested;
    }

    //=============================================================
    // 次の手で攻撃を試みる
    //=============================================================
    private void TryAttackWithNextHand()
    {
        for (int i = 0; i < _hands.Count; i++)
        {
            // 次の手のインデックスを計算
            int index = (_nextHandIndex + i) % _hands.Count;
            if (_hands[index].CommandAttack(_targetEntity))
            {
                // 攻撃が成功した場合、次の手のインデックスを更新
                _nextHandIndex = (index + 1) % _hands.Count;
                return;
            }
        }
    }
}
