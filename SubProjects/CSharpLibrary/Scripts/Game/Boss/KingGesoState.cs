
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
    private float elapsed;

    /// <summary>
    /// 状態開始時の処理
    /// </summary>
    /// <param name="owner"></param>
    public void Enter(KingGeso owner)
    {
        elapsed = 0.0f;
    }

    /// <summary>
    /// 状態更新時の処理
    /// </summary>
    /// <param name="owner"></param>
    public void Update(KingGeso owner)
    {
        elapsed += Time.deltaTime;
        if (owner.ConsumeAttackRequest() || elapsed >= owner.IdleDuration)
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
    private float elapsed;
    private float spawnElapsed;
    private int spawnedCount;
    private KingGesoAttackType attackType;
    private readonly List<Entity> pendingGesos = new List<Entity>();

    public void Enter(KingGeso owner)
    {
        elapsed = 0.0f;
        spawnElapsed = 0.0f;
        spawnedCount = 0;
        attackType = owner.SelectAttackType();
        pendingGesos.Clear();

        if (attackType == KingGesoAttackType.PincerThrust)
        {
            SpawnPincerGesos(owner);
        }
        else
        {
            // 初回のゲソをスポーン
            SpawnPincerGesos(owner);
        }
    }

    public void Update(KingGeso owner)
    {
        // まだスポーンしていないゲソの攻撃を開始
        StartPendingGesoAttacks(owner);

        elapsed += Time.deltaTime;
        spawnElapsed += Time.deltaTime;

        if (attackType == KingGesoAttackType.WaveThrust)
        {
            // ゲソのスポーン間隔に達した場合、次のゲソをスポーン
            if (!SpawnDueGesos(owner))
            {
                return;
            }
        }

        if (elapsed >= owner.AttackDuration)
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
        for (int i = pendingGesos.Count - 1; i >= 0; i--)
        {

            // ゲソがnullでない場合、攻撃を開始し、成功した場合はリストから削除
            Entity pendingGeso = pendingGesos[i];
            if (pendingGeso == null || owner.StartGesoAttack(pendingGeso))
            {
                pendingGesos.RemoveAt(i);
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
        while (spawnedCount < owner.WaveGesoCount && spawnElapsed >= interval)
        {
            spawnElapsed -= interval;

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
        pendingGesos.Add(geso);
        spawnedCount++;
        return true;
    }

    //===========================================
    // 挟み撃ち攻撃用の触手の生成
    //===========================================
    private bool SpawnPincerGesos(KingGeso owner)
    {
        List<Entity> spawnedGesos = new List<Entity>();
        if (!owner.SpawnPincerGesos(spawnedGesos))
        {
            owner.ChangeState(new KingGesoCooldownState());
            return false;
        }

        for (int i = 0; i < spawnedGesos.Count; i++)
        {
            pendingGesos.Add(spawnedGesos[i]);
        }

        spawnedCount += spawnedGesos.Count;
        return true;
    }
}

//==========================================-
// キングゲソのクールダウン状態
//==========================================
internal sealed class KingGesoCooldownState : IKingGesoState
{
    private float elapsed;

    public void Enter(KingGeso owner)
    {
        // クールダウン状態に入った時点で経過時間をリセット
        elapsed = 0.0f;
    }

    public void Update(KingGeso owner)
    {
        elapsed += Time.deltaTime;
        if (elapsed >= owner.CooldownDuration)
        {
            // クールダウン終了後、待機状態に遷移
            owner.ChangeState(new KingGesoIdleState());
        }
    }

    public void Exit(KingGeso owner)
    {
    }
}

//==========================================
// キングゲソの死亡状態
//==========================================
internal sealed class KingGesoDeadState : IKingGesoState
{
    public void Enter(KingGeso owner)
    {
        // 死亡時の処理（必要に応じて追加）
        owner.DestroyActiveGeso();
    }
    public void Update(KingGeso owner)
    {
        // 死亡状態では特に更新処理は不要
    }
    public void Exit(KingGeso owner)
    {
        // 死亡状態からの遷移は通常ないため、特に処理は不要
    }
}