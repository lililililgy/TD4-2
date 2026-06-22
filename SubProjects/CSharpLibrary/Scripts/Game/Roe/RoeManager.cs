using System;
using System.Collections.Generic;

// プレイヤー（母体）が抱える卵(roe)を Prefab から生成し、隊列の順番を記録する。
// 卵は別 Entity。実際の追従は各卵の RoeFollower が行う。
// 卵の総数は broodCount_ で決まる（残機 Life とは別物）。成熟した卵の数は MatureCount() で公開し、
// PlayerLifeComponent がそれを残機として参照する。
//
// 順番は「生成時に卵へ push」ではなく「entityId をキーに記録し、卵が自分の Initialize で pull」する。
// 生成直後の卵はまだ engine 管理下にスクリプトインスタンスが無く、直接 push できないため。
public class RoeManager : MonoScript {

    [SerializeField] private string roePrefabName_ = "Roe";     // 生成する卵プレハブ名
    [SerializeField] private string leaderName_    = "Player";  // 卵が追従する対象（トレイル保持者）

    [SerializeField] private int broodCount_ = 3; // 抱える卵の総数（生成する卵の数）

    private readonly List<Entity> roe_ = new List<Entity>();
    private readonly Dictionary<int, int> orderById_ = new Dictionary<int, int>(); // entityId -> 隊列順(1始まり)

    public override void Update() {
        int want = broodCount_;

        // 足りなければ生成（生成失敗時は無限ループ防止のため中断）
        while (roe_.Count < want) {
            if (!Spawn(roe_.Count + 1)) {
                break;
            }
        }

        // 多ければ末尾から削除（残った卵の順番は不変）
        while (roe_.Count > want) {
            int last = roe_.Count - 1;
            Entity dead = roe_[last];
            if (dead != null) {
                orderById_.Remove(dead.Id);
                dead.Destroy();
            }
            roe_.RemoveAt(last);
        }
    }

    // 卵が自分の順番を引くための問い合わせ（未登録は -1）
    public int GetOrder(int entityId) {
        return orderById_.TryGetValue(entityId, out int order) ? order : -1;
    }

    // 残機として有効な卵の数（＝成熟した卵の数）。生まれたての卵は数えない。
    public int MatureCount() {
        int count = 0;
        for (int i = 0; i < roe_.Count; i++) {
            Entity e = roe_[i];
            if (e == null) {
                continue;
            }
            RoeGrowth growth = e.GetScript<RoeGrowth>();
            if (growth != null && growth.IsMature()) {
                count++;
            }
        }
        return count;
    }

    // order は 1 始まり（先頭の卵が 1）。生成できたら true。
    private bool Spawn(int order) {
        Entity e = ecsGroup.CreateEntity(roePrefabName_);
        if (e == null) {
            return false;
        }

        // 順番は entityId をキーに記録。卵側が Initialize で GetOrder(entity.Id) で引く。
        orderById_[e.Id] = order;

        // 初期位置はプレイヤー位置に寄せる（以後 RoeFollower が追従）
        Transform t = e.GetComponent<Transform>();
        if (t != null) {
            t.position = transform.position;
        }

        roe_.Add(e);
        return true;
    }
}
