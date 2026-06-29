using System;
using System.Collections.Generic;

// TrailFollower 隊列(尻尾・卵など)を率いる側に付ける PBD(FTL)ソルバ。
// TrailFollower 各ノードが自分を Register() し、ここが毎フレーム「点列」を一括で解く。
//
// なぜ集約するか:
//   PBD で尻尾らしい遅れ・カールを出すには、各ノードを「前のノードの更新後の位置」から
//   順番に解く必要がある(下のループの最後で prev = 解いた位置)。この前→後の順序保証は
//   全ノードを1ループで回せるここでしか作れない。Follower 各自に解かせるとスクリプト
//   実行順に依存して壊れる。
//
// chainName ごとに独立した点列として解くので、1人のリーダーに複数の隊列
//   (胴体の尻尾 / 卵の群れ など、rest 距離も order 体系も別物)をぶら下げられる。
// ノード参照は Entity で持ち、毎フレーム GetScript で生存を確認するため、
//   途中ノードが破棄されても点列が壊れない(詰めて解き直すだけ)。
public class TrailLeader : MonoScript {

    // 登録された全メンバー(chain 混在)。生存確認は毎フレーム GetScript で行う。
    private readonly List<Entity> members_ = new List<Entity>();

    // chain 名 -> その frame の生存ノード。Update のたびに作り直す使い捨てバッファ。
    private readonly Dictionary<string, List<TrailFollower>> groups_ =
        new Dictionary<string, List<TrailFollower>>();

    // TrailFollower 側から一度だけ呼ばれる。重複登録は呼び出し側(registered_ フラグ)が防ぐ。
    public void Register(TrailFollower follower) {
        if (follower == null || follower.entity == null) {
            return;
        }
        members_.Add(follower.entity);
    }

    public override void Update() {
        // 破棄済みメンバーを掃除(RoeManager.Prune と同じく null 比較で判定)。
        for (int i = members_.Count - 1; i >= 0; i--) {
            if (members_[i] == null) {
                members_.RemoveAt(i);
            }
        }

        // chain ごとに生存ノードを集める。
        foreach (KeyValuePair<string, List<TrailFollower>> kv in groups_) {
            kv.Value.Clear();
        }
        for (int i = 0; i < members_.Count; i++) {
            Entity e = members_[i];
            if (e == null) {
                continue;
            }
            TrailFollower f = e.GetScript<TrailFollower>();
            if (f == null) {
                continue; // 破棄直後など。次フレームの null 掃除で除去される。
            }
            List<TrailFollower> list;
            if (!groups_.TryGetValue(f.ChainName, out list)) {
                list = new List<TrailFollower>();
                groups_[f.ChainName] = list;
            }
            list.Add(f);
        }

        // 曲げ角拘束の基準向き：先頭ノードはリーダーの「後方」を基準にする。
        // （リーダーの向きが未確定で長さが出ない時は真下にフォールバック）
        Vector3 anchor   = transform.position;
        Vector3 back     = -Quaternion.RotateVector(transform.rotate, Vector3.up);
        float   bl       = back.Length();
        Vector3 leaderBackward = (bl > kEps) ? back * (1.0f / bl) : new Vector3(0.0f, -1.0f, 0.0f);

        // 各 chain を order 順に、前ノードの更新後位置から1パスで解く(FTL/PBD)。
        float dt = Time.deltaTime;
        foreach (KeyValuePair<string, List<TrailFollower>> kv in groups_) {
            List<TrailFollower> list = kv.Value;
            list.Sort(CompareOrder);

            Vector3 prev   = anchor;
            Vector3 refDir = leaderBackward; // 先頭の基準＝リーダー後方。以降は前セグメントの向き。
            for (int i = 0; i < list.Count; i++) {
                // 先頭(生存ノードの最前)はリーダーに leadOffset で繋ぐ。以降は前ノードに unitOffset。
                Vector3 newPos = list[i].Solve(prev, refDir, i == 0, dt);

                // 次ノードの基準向き＝今解いたセグメント(prev→newPos)の向き。
                // 潰れて長さが出ない時は基準を更新せず前の向きを引き継ぐ。
                Vector3 seg = newPos - prev;
                float   sl  = seg.Length();
                if (sl > kEps) {
                    refDir = seg * (1.0f / sl);
                }
                prev = newPos;
            }
        }
    }

    // ベクトル長のゼロ割り回避しきい値
    private const float kEps = 1e-4f;

    private static int CompareOrder(TrailFollower a, TrailFollower b) {
        return a.Order.CompareTo(b.Order);
    }
}
