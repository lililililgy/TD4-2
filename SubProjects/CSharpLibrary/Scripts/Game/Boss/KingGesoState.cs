
using System.Collections.Generic;

/// <summary>
/// キングゲソの状態(基底クラス)
/// </summary>
internal interface IKingGesoState
{
    void Enter(KingGeso owner);
    void Update(KingGeso owner);
    void Exit(KingGeso owner);
}

//==========================================-
// キングゲソの待機状態クラス
//==========================================
internal sealed class KingGesoIdleState : IKingGesoState
{
    // 経過時間
    private float _elapsed;

    /// <summary>
    /// 状態開始時の処理
    /// </summary>
    /// <param name="owner"></param>
    public void Enter(KingGeso owner)
    {
        _elapsed = 0.0f;
    }

    /// <summary>
    /// 状態更新時の処理
    /// </summary>
    /// <param name="owner"></param>
    public void Update(KingGeso owner)
    {
        _elapsed += Time.deltaTime;
        if (owner.ConsumeAttackRequest() || _elapsed >= owner.IdleDuration)
        {
            // 待機時間が経過したか、攻撃要求があった場合、攻撃状態に遷移
            owner.ChangeState(new KingGesoAttackState());
        }
    }

    /// <summary>
    /// 状態終了時の処理
    /// </summary>
    /// <param name="owner"></param>
    public void Exit(KingGeso owner)
    {
    }
}

//==========================================-
// キングゲソの攻撃状態
//==========================================
internal sealed class KingGesoAttackState : IKingGesoState
{
    private float _elapsed;
    private float _spawnElapsed;
    private int _spawnedCount;
    private readonly List<Entity> _pendingGesos = new List<Entity>();

    public void Enter(KingGeso owner)
    {
        _elapsed = 0.0f;
        _spawnElapsed = 0.0f;
        _spawnedCount = 0;
        _pendingGesos.Clear();

        // 初回のゲソをスポーン
        SpawnNextGeso(owner);
    }

    public void Update(KingGeso owner)
    {
        // まだスポーンしていないゲソの攻撃を開始
        StartPendingGesoAttacks(owner);

        _elapsed += Time.deltaTime;
        _spawnElapsed += Time.deltaTime;

        // ゲソのスポーン間隔に達した場合、次のゲソをスポーン
        if (!SpawnDueGesos(owner))
        {
            return;
        }

        if (_elapsed >= owner.AttackDuration)
        {
            //attack終了後、クールダウン状態に遷移
            owner.ChangeState(new KingGesoCooldownState());
        }
    }

    public void Exit(KingGeso owner)
    {
        // 攻撃終了時にアクティブなゲソを破棄
        owner.DestroyActiveGeso();
    }

    /// <summary>
    /// 待機中のゲソの攻撃を開始する
    /// </summary>
    /// <param name="owner"></param>
    private void StartPendingGesoAttacks(KingGeso owner)
    {

        // 待機中のゲソの攻撃を開始
        for (int i = _pendingGesos.Count - 1; i >= 0; i--)
        {

            // ゲソがnullでない場合、攻撃を開始し、成功した場合はリストから削除
            Entity pendingGeso = _pendingGesos[i];
            if (pendingGeso == null || owner.StartGesoAttack(pendingGeso))
            {
                _pendingGesos.RemoveAt(i);
            }
        }
    }

    /// <summary>
    /// スポーン間隔に達した場合、次のゲソをスポーンする
    /// </summary>
    /// <param name="owner"></param>
    /// <returns></returns>
    private bool SpawnDueGesos(KingGeso owner)
    {

        // スポーン間隔を取得
        float interval = owner.WaveGesoInterval;
        while (_spawnedCount < owner.WaveGesoCount && _spawnElapsed >= interval)
        {
            _spawnElapsed -= interval;

            // 次のゲソをスポーン。スポーンに失敗した場合はクールダウン状態に遷移
            if (!SpawnNextGeso(owner))
            {
                return false;
            }
        }

        return true;
    }

    private bool SpawnNextGeso(KingGeso owner)
    {

        // ゲソをスポーン
        Entity geso = owner.SpawnGeso();
        if (geso == null)
        {

            // ゲソのスポーンに失敗した場合、クールダウン状態に遷移
            owner.ChangeState(new KingGesoCooldownState());
            return false;
        }

        // スポーンしたゲソを待機リストに追加
        _pendingGesos.Add(geso);
        _spawnedCount++;
        return true;
    }
}

//==========================================-
// キングゲソのクールダウン状態
//==========================================
internal sealed class KingGesoCooldownState : IKingGesoState
{
    private float _elapsed;

    public void Enter(KingGeso owner)
    {
        // クールダウン状態に入った時点で経過時間をリセット
        _elapsed = 0.0f;
    }

    public void Update(KingGeso owner)
    {
        _elapsed += Time.deltaTime;
        if (_elapsed >= owner.CooldownDuration)
        {
            // クールダウン終了後、待機状態に遷移
            owner.ChangeState(new KingGesoIdleState());
        }
    }

    public void Exit(KingGeso owner)
    {
    }
}
