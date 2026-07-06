using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

// バイオームの出現テーブル1行ぶん。敵の種類とその重みを束ねる。
//
// ※ class にしているのは意図的。エンジンの SerializeField 復元が
//    「List<参照型>」かつ「引数なしコンストラクタ」を前提にしているため、struct にすると
//    リスト復元時に .ctor() が見つからず実行時クラッシュする。引数なし ctor は必須。
public class SpawnEntry {
    public SpawnEntry() { }

    public SpawnEntry(string enemyType, float weight) {
        this.enemyType = enemyType;
        this.weight = weight;
    }

    [SerializeField] public string enemyType;  // 生成する Entity の種類
    [SerializeField] public float weight;       // 出現の重み(相対値。合計で正規化される)
}