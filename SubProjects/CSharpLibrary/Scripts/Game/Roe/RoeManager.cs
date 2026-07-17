using System;
using System.Collections.Generic;

// プレイヤー（母体）が抱える卵(roe)を統括する。卵は別 Entity で、各卵の状態は RoeStateComponent が持つ。
// このリスト＋各卵の状態が唯一の真実(single source of truth)で、残機・弾はここから導出する：
//   残機 = HP = 全卵数(EggCount。未成熟の子たまごも含む)。弾 = 成熟卵(MatureCount)。
//   発射(TryConsumeMature)は成熟卵を消費する（ただし最後の1卵は残す＝発射で即ゲームオーバーしない）。
//   被弾(TryKillLife)は隊列末尾の卵を成熟/未成熟問わず1つ失う（最後の1個も失いゲームオーバーになる）。
//
// 卵は有限資源。上限は maxRoe_ で管理する（母体が時間（暫定。本来は経験値）で産卵する分の上限）。
//
// 隊列順は entityId をキーに記録し、卵が自分の Initialize / Update で pull する
// （生成直後の卵はまだスクリプトインスタンスが無く直接 push できないため）。
public class RoeManager : MonoScript {

    [SerializeField] private string roePrefabName_ = "Roe";  // 生成する卵プレハブ名

    [SerializeField] private int maxRoe_ = 5;    // 卵の上限

    private readonly List<Entity> roe_ = new List<Entity>();
    private float lastDistributedPlayerExp_ = 0f; // 前フレームまでに卵へ分配済みのプレイヤー累計経験値

    public override void Update() {
        // 破棄済み（発射・撃破）の卵を隊列から掃除
        Prune();

        // 隊列順(リスト順)を各卵の TrailFollower に push する。
        // 生成直後でスクリプト未生成の卵も、次フレーム以降にここで順番が入る。
        PushOrders();

        LevelingComponent levelingComponent = entity.GetScript<LevelingComponent>();
        // Levelが上がったタイミングで産卵
        if (levelingComponent.IsLevelUp) {
            if (EggCount() < maxRoe_) {
                Spawn();
            }
        }

        // プレイヤーが新たに取得した経験値(前フレームからの増分)を卵に分配する。
        // AddedExp は LevelingComponent.Update が毎フレーム先頭でリセットするため、
        // スクリプト実行順(Leveling→RoeManager)では常に 0 になってしまう。
        // 累計値(TotalGainedExp)の差分を自分で取ることで実行順に依存せず増分を得る。
        float playerTotalExp = levelingComponent.TotalGainedExp;
        float gainedExp = playerTotalExp - lastDistributedPlayerExp_;
        lastDistributedPlayerExp_ = playerTotalExp;
        if (gainedExp > 0) {
            foreach (Entity e in roe_) {
                if (e == null) {
                    continue;
                }
                RoeStateComponent state = e.GetScript<RoeStateComponent>();
                if (state == null || state.IsMature) {
                    continue;
                }

                LevelingComponent roeLevel = e.GetScript<LevelingComponent>();
                roeLevel?.AddExperience(gainedExp);
            }
        }
    }

    // ---- 導出値（残機・弾・HP） ----

    // 弾数 = 成熟した卵の数
    public int MatureCount() {
        int count = 0;
        for (int i = 0; i < roe_.Count; i++) {
            RoeStateComponent state = GetState(roe_[i]);
            if (state != null && state.IsMature) {
                count++;
            }
        }
        return count;
    }

    // 残機 = HP = 隊列の全卵数（未成熟の子たまごも含む）。産卵上限の判定にも使う。
    public int EggCount() {
        return roe_.Count;
    }

    public int MaxRoe { get { return maxRoe_; } }

    // ---- 状態遷移（発射・被弾） ----

    // ダメージで残機を1つ失う：隊列末尾の卵を成熟/未成熟問わず1つ外して破棄する。成功で true。
    // 全卵が残機なので状態を問わない。発射と違い最後の1個も失う（残機0でゲームオーバー）。
    public bool TryKillLife() {
        for (int i = roe_.Count - 1; i >= 0; i--) {
            if (roe_[i] != null) {
                Entity e = roe_[i];
                roe_.RemoveAt(i);
                e.Destroy();
                return true;
            }
        }
        return false;
    }

    // 成熟卵(弾)を1つ隊列から外して返す（発射用）。無ければ null。破棄は呼び出し側に任せる。
    // 卵が合計1個のときは発射不可（発射での即ゲームオーバー防止。子たまごが残っていれば最後の成熟卵も撃てる）。
    public Entity TryConsumeMature() {
        if (EggCount() <= 1) {
            return null;
        }
        return ConsumeMature();
    }

    // ---- 内部 ----

    // 成熟卵を1つ隊列から外して返す（発射専用ヘルパー）。末尾側から探す。無ければ null。
    private Entity ConsumeMature() {
        for (int i = roe_.Count - 1; i >= 0; i--) {
            RoeStateComponent state = GetState(roe_[i]);
            if (state != null && state.IsMature) {
                Entity e = roe_[i];
                roe_.RemoveAt(i);
                return e;
            }
        }
        return null;
    }

    private RoeStateComponent GetState(Entity e) {
        return e != null ? e.GetScript<RoeStateComponent>() : null;
    }

    // 破棄済みの卵をリストから除去する（隊列順は Update の PushOrders で詰め直す）
    private void Prune() {
        for (int i = roe_.Count - 1; i >= 0; i--) {
            if (roe_[i] == null) {
                roe_.RemoveAt(i);
            }
        }
    }

    // リスト順 = 隊列順(1始まり) を各卵の TrailFollower に push する。
    // 各卵は order を自分で持ち、母体(Player)を order に応じた距離で追従する。
    private void PushOrders() {
        for (int i = 0; i < roe_.Count; i++) {
            if (roe_[i] != null) {
                TrailFollower follower = roe_[i].GetScript<TrailFollower>();
                follower?.SetOrder(i + 1);
            }
        }
    }

    // 卵を1個生成し隊列末尾に加える。生成できたら true。
    private bool Spawn() {
        Entity e = ecsGroup.CreateEntity(roePrefabName_);
        if (e == null) {
            return false;
        }

        // 初期位置はプレイヤー位置に寄せる（以後 TrailFollower が追従）
        Transform t = e.GetComponent<Transform>();
        if (t != null) {
            t.position = transform.position;
        }

        roe_.Add(e);
        return true;
    }
}
