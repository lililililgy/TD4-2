using System;

// 指定距離へのターゲット入出を検知するコンポーネント
public class TargetRangeDetector : MonoScript
{
    [SerializeField] private string targetEntityName = "Player";
    [SerializeField] private float  range            = 5.0f;

    public Action OnEnter;           // 範囲内に入った瞬間
    public Action OnExit;            // 範囲から出た瞬間
    public bool   IsInRange  { get; private set; } = false;
    public Entity Target     => targetEntity_;

    // ターゲットEntity
    private Entity targetEntity_ = null;
    private bool   wasInRange_   = false;

    public override void Update()
    {
        if (transform == null) { return; }

        // 未キャッシュなら検索
        if (targetEntity_ == null)
        {
            if (ecsGroup == null) { return; }
            targetEntity_ = ecsGroup.FindEntity(targetEntityName);
            if (targetEntity_ == null) { return; }
        }

        if (targetEntity_.transform == null) { return; }

        // 距離判定
        float dist = (targetEntity_.transform.position - transform.position).Length();
        IsInRange = dist <= range;

        // 入出イベント発火
        if (IsInRange && !wasInRange_)
        {
            OnEnter?.Invoke();
        }
        else if (!IsInRange && wasInRange_)
        {
            OnExit?.Invoke();
        }

        // 前フレームの状態を保存
        wasInRange_ = IsInRange;
    }
}
