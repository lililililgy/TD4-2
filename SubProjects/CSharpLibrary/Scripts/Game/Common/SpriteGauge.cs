using System;
using System.Collections.Generic;

// 0.0〜1.0 の割合をバーとして見せる汎用スクリプト。HPバー・経験値バー・チャージゲージなど。
// SpriteRenderer を持つエンティティに付けて使う。
//
// ■ テクスチャの作り方
// 1枚のテクスチャに「ON（残っている側）」と「OFF（減った側）」の差分を並べて描く。
// 並べる向きは vertical_ で、どちらが先（UV の小さい側）かは onFirst_ で指定する。
//
//   横並び・onFirst_ = true の例（既定）:
//     +--------+--------+
//     |   ON   |   OFF  |   ← 左半分が満タンの絵、右半分が空の絵
//     +--------+--------+
//     u=0     u=0.5    u=1
//
// ■ 表示のしかた
// UV の「窓」を半分の大きさ（0.5）にして ON 側に合わせ、そこから OFF 側へ 0〜0.5 の範囲でずらす。
// 窓のうち ON に残っている部分がそのまま画面上の棒の長さになるので、
//
//   割合 1.0 → 窓 = ON 全部        → 棒は満タン
//   割合 0.5 → 窓 = ON半分+OFF半分 → 棒は半分（境目が画面中央）
//   割合 0.0 → 窓 = OFF 全部       → 棒は空
//
// エンティティのスケールを変えないので、枠や目盛りと重ねてもズレない。
// 減る向き（どちら側に残量が寄るか）は onFirst_ で反転する
// （ON が手前なら手前側に残り、奥から減っていく）。
//
// ■ 値の取り方
// 既定は pull 型で、対象エンティティの IGaugeSource を毎フレーム引く。
// 別シーン（GameUIScene に置いたバーから GameScene の Player を見る等）にも sourceSceneName_ で届く。
// IGaugeSource を実装していない値を出したい場合は、外から SetRatio() で流し込んでもよい。
//
// pull にしているのは、バーが「起きた瞬間」ではなく「今いくつか」を出すものだから。
// MessageBus は型でしか配れないので、イベントにしても結局どのエンティティの値かを
// 名前で絞ることになり、設定は減らない。加えて変化時のみの発行だと初期値が届かない
// （GameUIScene は GameScene の AddSceneScript.Update() で後から追加されるため、
//   それ以前に発行されたイベントはそもそも受け取れない）。
// このプロジェクトでは撃破・レベルアップ・フェーズ開始のような「瞬間」がイベント、
// 継続的な状態は pull、という切り分けにしている（ShowOnPhase / PhaseSpawnActivator も同じ）。
//
// なお、イベントで値を受けること自体は問題ない（下の SetRatio() がその口）。
// やってはいけないのは受信ハンドラの中で uvTransform を書くことで、
// 別 ECSGroup から書くとバッチ同期の Receive に潰される。
// 値は素の C# フィールドに置き、UV の書き込みは必ず自分の Update() で行うこと。
public class SpriteGauge : MonoScript {

    // 値の取得元エンティティ名。空なら自分自身から探す（HP と同じエンティティに付ける場合）。
    [SerializeField] private string sourceEntityName_ = "";

    // 取得元がいるシーン(ECSグループ)名。空なら自分と同じグループ。
    // ecsGroup.FindEntity は自分のグループしか見ないので、別シーンならここに入れる。
    [SerializeField] private string sourceSceneName_ = "";

    // 取得元のスクリプト名。空なら最初に見つかった IGaugeSource を使う。
    // 1つのエンティティに IGaugeSource が複数付く場合（GameController のように
    // 進捗や長押しゲージが同居しうるエンティティ）は、ここで名指しして確定させる。
    [SerializeField] private string sourceScriptName_ = "";

    // ON/OFF がテクスチャ内で縦に並んでいるか。false なら横並び。
    [SerializeField] private bool vertical_ = false;

    // ON（残っている側の絵）が UV の小さい側にあるか。
    // 横並びなら true = 左が ON（棒は左に残り右から減る）、false = 右が ON（右に残り左から減る）。
    [SerializeField] private bool onFirst_ = true;

    // ON と OFF の2枚が並んでいるので、片方だけを映す窓の大きさは半分になる
    private const float kWindowSize = 0.5f;

    private IGaugeSource source_;
    private float ratio_ = 1.0f;

    // SetRatio() で外から入れられたか。入れられて以降は取得元を探さない
    // （直接流し込む使い方をしている＝そちらが真実なので、pull で上書きしない）。
    private bool hasExternalRatio_ = false;

    public override void Initialize() {
        source_ = ResolveSource();
        ApplyUV();
    }

    public override void Update() {
        // 取得元のエンティティがまだ生成されていなかった場合に備えた遅延再解決。
        // SetRatio() で運用している時は探しに行かない（毎フレーム GetScripts() を回さないため）。
        if (source_ == null && !hasExternalRatio_) {
            source_ = ResolveSource();
        }

        if (source_ != null) {
            ratio_ = source_.GetGaugeRatio();
        }

        ApplyUV();
    }

    // IGaugeSource を実装していない値をバーにしたい時の流し込み口。
    // 一度でも呼ぶと pull を止めるので、呼び続ける側が更新の責任を持つ。
    public void SetRatio(float ratio) {
        ratio_ = ratio;
        hasExternalRatio_ = true;
    }

    // 現在表示している割合（0.0〜1.0）
    public float Ratio {
        get { return Mathf.Clamp01(ratio_); }
    }

    // 窓を ON 側の端から OFF 側へずらす。
    // uvTransform は毎フレーム native の値で上書きされる（Receive）ため、
    // 変化が無くても Update のたびに書き直す必要がある。
    private void ApplyUV() {
        SpriteRenderer sprite = entity.GetComponent<SpriteRenderer>();
        if (sprite == null) return;

        // ON が手前にあるなら、減るほど窓を奥（OFF 側）へ送る。
        // ON が奥にあるなら向きが逆になり、増えるほど奥へ送ることで OFF が手前に残る。
        float ratio = Mathf.Clamp01(ratio_);
        float offset = onFirst_ ? (1.0f - ratio) * kWindowSize : ratio * kWindowSize;

        // native から読んだ値を継ぎ足さず、毎回組み立て直す。
        // 前フレームの窓や、他スクリプト（SpriteAnimation 等）が入れた値を引きずらないため。
        UVTransform uv = UVTransform.identity;
        if (vertical_) {
            uv.scale = new Vector2(1.0f, kWindowSize);
            uv.position = new Vector2(0.0f, offset);
        } else {
            uv.scale = new Vector2(kWindowSize, 1.0f);
            uv.position = new Vector2(offset, 0.0f);
        }

        sprite.uvTransform = uv;
    }

    // 取得元エンティティから IGaugeSource を探す。見つからなければ null（次フレームに再試行）。
    private IGaugeSource ResolveSource() {
        Entity sourceEntity = ResolveSourceEntity();
        if (sourceEntity == null) return null;

        // 名指しされている場合はそれだけを見る。
        // 指定した名前のスクリプトが IGaugeSource でなければ、黙って別のを拾わずに諦める
        // （設定ミスが「なぜか別の値が出る」ではなく「出ない」になるようにする）。
        if (!string.IsNullOrEmpty(sourceScriptName_)) {
            return sourceEntity.GetScript(sourceScriptName_) as IGaugeSource;
        }

        // 名指しが無ければ最初に見つかったものを使う。複数付いている場合に
        // どれが選ばれるかは決まらない（scripts_ が Dictionary で順序を保証しない）ので、
        // その時は sourceScriptName_ で確定させること。
        foreach (MonoScript script in sourceEntity.GetScripts()) {
            IGaugeSource gaugeSource = script as IGaugeSource;
            if (gaugeSource != null) return gaugeSource;
        }

        return null;
    }

    private Entity ResolveSourceEntity() {
        if (string.IsNullOrEmpty(sourceEntityName_)) return entity;

        ECSGroup group = ecsGroup;
        if (!string.IsNullOrEmpty(sourceSceneName_)) {
            group = EntityComponentSystem.GetECSGroup(sourceSceneName_);
            if (!group) return null; // シーンがまだ読まれていない
        }

        return group.FindEntity(sourceEntityName_);
    }
}
