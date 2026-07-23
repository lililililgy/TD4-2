using System;
using System.Collections.Generic;

// プレイヤー（母体）が抱える卵(roe)を統括する。卵は別 Entity で、各卵の状態は RoeStateComponent が持つ。
//
// 唯一の真実(single source of truth)は HP。卵の数は毎フレーム HP.CurrentHp に合わせて増減させる：
//   卵数 = HP.CurrentHp（= 残機）。上限は HP.MaxHp。弾数 = 成熟卵(MatureCount)。
//   被弾で HP が減れば隊列末尾の卵が消え、回復（レベルアップ）で増えれば卵が生える。
//   発射(TryConsumeMature)は成熟卵を消費し、そのぶん HP も1減らす
//   （減らさないと次フレームに卵が再生成されて弾が無限になる。ただし最後の1卵は残す）。
//
// 卵の増減を要求する側は HP を書き換えるだけでよく、卵の生成・破棄はここが吸収する。
//
// 隊列順は entityId をキーに記録し、卵が自分の Initialize / Update で pull する
// （生成直後の卵はまだスクリプトインスタンスが無く直接 push できないため）。
public class RoeManager : MonoScript
{

    [SerializeField] private string roePrefabName_ = "Roe";  // 生成する卵プレハブ名

    private readonly List<Entity> roe_ = new List<Entity>();
    private HP hp_;
    private float lastDistributedPlayerExp_ = 0f; // 前フレームまでに卵へ分配済みのプレイヤー累計経験値
    private int pendingMatureGrants_ = 0; // 成熟させる予約の残り（卵の実体が生えてから消化する）

    public override void Initialize()
    {
        hp_ = entity.GetScript<HP>();
        // 初期卵は Spawn しない。最初の Update で HP.CurrentHp ぶんが揃う
    }

    public override void Update()
    {
        // 破棄済み（発射・撃破）の卵を隊列から掃除
        Prune();

        // 卵の数を HP に合わせる（被弾で減った／回復で増えた分をここで吸収する）
        SyncEggsToHp();

        // 隊列順(リスト順)を各卵の TrailFollower に push する。
        // 生成直後でスクリプト未生成の卵も、次フレーム以降にここで順番が入る。
        PushOrders();

        // 付与予約された成熟卵を、実体が生えてスクリプトが付いてから成熟させる
        ResolveMatureGrants();

        LevelingComponent levelingComponent = entity.GetScript<LevelingComponent>();

        // プレイヤーが新たに取得した経験値(前フレームからの増分)を卵に分配する。
        // AddedExp は LevelingComponent.Update が毎フレーム先頭でリセットするため、
        // スクリプト実行順(Leveling→RoeManager)では常に 0 になってしまう。
        // 累計値(TotalGainedExp)の差分を自分で取ることで実行順に依存せず増分を得る。
        float playerTotalExp = levelingComponent.TotalGainedExp;
        float gainedExp = playerTotalExp - lastDistributedPlayerExp_;
        lastDistributedPlayerExp_ = playerTotalExp;
        if (gainedExp > 0)
        {
            foreach (Entity e in roe_)
            {
                if (e == null)
                {
                    continue;
                }
                RoeStateComponent state = e.GetScript<RoeStateComponent>();
                if (state == null || state.IsMature)
                {
                    continue;
                }

                LevelingComponent roeLevel = e.GetScript<LevelingComponent>();
                roeLevel?.AddExperience(gainedExp);
            }
        }
    }

    // ---- 導出値（残機・弾・HP） ----

    // 弾数 = 成熟した卵の数
    public int MatureCount()
    {
        int count = 0;
        for (int i = 0; i < roe_.Count; i++)
        {
            RoeStateComponent state = GetState(roe_[i]);
            if (state != null && state.IsMature)
            {
                count++;
            }
        }
        return count;
    }

    // 隊列の全卵数（未成熟の子たまごも含む）。HP に追従するので、実質「現在の残機」と同じ。
    public int EggCount()
    {
        return roe_.Count;
    }

    // ---- 付与（フェーズクリア報酬など） ----

    // 成熟済みの卵（＝すぐ撃てる弾）を1つ与える。
    //
    // その場では HP を1増やすだけで、卵の実体は次の Update の SyncEggsToHp が生やす。
    // 生えたばかりの卵はまだスクリプトインスタンスが無く成熟させられないので、
    // 「成熟させる」ぶんは予約(pendingMatureGrants_)として持ち越し、生え揃ってから消化する。
    //
    // 卵数が上限(MaxHp)で増やせない場合は数はそのままで、手持ちの未成熟卵を1つ成熟させる。
    public void GrantMatureEgg()
    {
        // 死亡済み（＝ゲームオーバー）を報酬で生き返らせない
        if (hp_ == null || hp_.IsDead)
        {
            return;
        }

        if (hp_.CurrentHp < hp_.MaxHp)
        {
            // ※ Heal ではなく SetHp なのは TryConsumeMature と同じ理由（被弾演出を出さないため）
            hp_.SetHp(hp_.CurrentHp + 1);
        }

        pendingMatureGrants_++;
    }

    // ---- 内部 ----

    // 付与予約された成熟卵を、卵の実体が生え揃ってから消化する。
    private void ResolveMatureGrants()
    {
        if (pendingMatureGrants_ <= 0)
        {
            return;
        }

        // 予約したぶんの卵が生えてスクリプトが付くまで待つ（それまで成熟判定を正しく数えられない）
        if (!IsRoeReady())
        {
            return;
        }

        while (pendingMatureGrants_ > 0 && MatureOneEgg())
        {
            pendingMatureGrants_--;
        }

        // 未成熟の卵が尽きた＝全部成熟済み。もう与えるものが無いので残りの予約は捨てる
        pendingMatureGrants_ = 0;
    }

    // 卵の増減が HP に追いつき、かつ全ての卵のスクリプトが取れる状態か。
    // 生成した卵はその場ではスクリプトインスタンスが無く MatureCount() に数えられない。
    private bool IsRoeReady()
    {
        if (hp_ == null || roe_.Count != (int)hp_.CurrentHp)
        {
            return false;
        }

        for (int i = 0; i < roe_.Count; i++)
        {
            if (GetState(roe_[i]) == null)
            {
                return false;
            }
        }
        return true;
    }

    // 未成熟の卵を1つ成熟させる。成功で true（未成熟の卵が無ければ false）。
    // IsMature を直接立てず経験値でレベルアップさせるのは、RoeStateComponent が自分の Update で
    // 成熟（見た目・アニメ再生）まで面倒を見る通常の経路にそのまま乗せるため。
    // 成熟は次フレームの LevelingComponent.Update まで反映されないので、
    // 「経験値は足りているがまだ IsMature が立っていない」卵は成熟済み扱いで飛ばす（二重付与の防止）。
    private bool MatureOneEgg()
    {
        for (int i = 0; i < roe_.Count; i++)
        {
            RoeStateComponent state = GetState(roe_[i]);
            if (state == null || state.IsMature)
            {
                continue;
            }

            LevelingComponent leveling = roe_[i].GetScript<LevelingComponent>();
            if (leveling == null || leveling.CurrentExp >= leveling.RequiredExp)
            {
                continue;
            }

            leveling.AddExperience(leveling.RequiredExp - leveling.CurrentExp);
            return true;
        }
        return false;
    }

    // ---- 状態遷移（発射） ----

    // 成熟卵(弾)を1つ隊列から外して返す（発射用）。無ければ null。破棄は呼び出し側に任せる。
    // 卵が合計1個のときは発射不可（発射での即ゲームオーバー防止。子たまごが残っていれば最後の成熟卵も撃てる）。
    public Entity TryConsumeMature()
    {
        if (EggCount() <= 1)
        {
            return null;
        }

        Entity e = ConsumeMature();
        if (e == null)
        {
            return null;
        }

        // 発射ぶん HP も減らす。減らさないと SyncEggsToHp が次フレームに卵を生やし直し、
        // 弾が無限になる。※ TakeDamage ではなく SetHp を使う
        //（TakeDamage は被弾扱いになり、被弾演出(ShakeOnDamaged)が発射のたびに出てしまう）。
        if (hp_ != null)
        {
            hp_.SetHp(hp_.CurrentHp - 1);
        }
        return e;
    }

    // 卵の数を HP.CurrentHp に合わせる。多ければ末尾から破棄し、足りなければ生成する。
    private void SyncEggsToHp()
    {
        if (hp_ == null)
        {
            return;
        }

        int target = (int)hp_.CurrentHp;
        if (target < 0)
        {
            target = 0;
        }

        // 被弾で減った分：隊列末尾の卵を成熟/未成熟問わず外して破棄する
        while (roe_.Count > target)
        {
            if (!RemoveTailEgg())
            {
                break;
            }
        }

        // 回復（レベルアップ）で増えた分：生成に失敗したらそこで打ち切る
        while (roe_.Count < target)
        {
            if (!Spawn())
            {
                break;
            }
        }
    }

    // 隊列末尾の卵を1つ外して破棄する。成功で true
    private bool RemoveTailEgg()
    {
        for (int i = roe_.Count - 1; i >= 0; i--)
        {
            if (roe_[i] != null)
            {
                Entity e = roe_[i];
                roe_.RemoveAt(i);
                e.Destroy();
                return true;
            }
        }
        return false;
    }

    // 成熟卵を1つ隊列から外して返す（発射専用ヘルパー）。末尾側から探す。無ければ null。
    private Entity ConsumeMature()
    {
        for (int i = roe_.Count - 1; i >= 0; i--)
        {
            RoeStateComponent state = GetState(roe_[i]);
            if (state != null && state.IsMature)
            {
                Entity e = roe_[i];
                roe_.RemoveAt(i);
                return e;
            }
        }
        return null;
    }

    private RoeStateComponent GetState(Entity e)
    {
        return e != null ? e.GetScript<RoeStateComponent>() : null;
    }

    // 破棄済みの卵をリストから除去する（隊列順は Update の PushOrders で詰め直す）
    private void Prune()
    {
        for (int i = roe_.Count - 1; i >= 0; i--)
        {
            if (roe_[i] == null)
            {
                roe_.RemoveAt(i);
            }
        }
    }

    // リスト順 = 隊列順(1始まり) を各卵の TrailFollower に push する。
    // 各卵は order を自分で持ち、母体(Player)を order に応じた距離で追従する。
    private void PushOrders()
    {
        for (int i = 0; i < roe_.Count; i++)
        {
            if (roe_[i] != null)
            {
                TrailFollower follower = roe_[i].GetScript<TrailFollower>();
                follower?.SetOrder(i + 1);
            }
        }
    }

    // 卵を1個生成し隊列末尾に加える。生成できたら true。
    private bool Spawn()
    {
        Entity e = ecsGroup.CreateEntity(roePrefabName_);
        if (e == null)
        {
            return false;
        }

        // 初期位置はプレイヤー位置に寄せる（以後 TrailFollower が追従）
        Transform t = e.GetComponent<Transform>();
        if (t != null)
        {
            t.position = transform.position;
        }

        roe_.Add(e);
        return true;
    }
}
