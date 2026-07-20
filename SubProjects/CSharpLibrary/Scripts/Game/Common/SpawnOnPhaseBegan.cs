using System;

// 指定したフェーズの開始でプレハブを1つスポーンさせる。
// スポーンさせたい対象ごとに空エンティティ（マーカー）を置き、これを付けて設定する。
//
// 注意: 1つのエンティティに同じ型のスクリプトは1つしか付けられない
// （Entity の scripts_ が型名をキーにした Dictionary のため）。
// 1フェーズで複数スポーンさせたい場合は、マーカーエンティティを複数用意する。
//
// 購読を Awake() で行っているのは意図的。ObjectiveSystem.Initialize() が最初の
// PhaseBeganEvent を発行するため、Initialize() で購読するとエンティティの並び順次第で
// 初回を取りこぼす（ECSGroup は CallAwake() → CallInitialize() の順に全エンティティを回す）。
//
// 生成をイベントハンドラ内で直接行わず Update() まで遅らせているのも意図的。
// PhaseBeganEvent は Initialize 中にも飛ぶが、ECSGroup.UpdateEntities() は
// CallInitialize() の後に entities_ をコピーして Update を回すため、Initialize 中に生成した
// エンティティは Initialize されないまま同じフレームで Update されうる。
// Update から生成すればコピー済みの配列に載らないので、この経路を避けられる
// （EnemySpawnSystem.SpawnEnemy() が Update から生成しているのと同じタイミング）。
public class SpawnOnPhaseBegan : MonoScript
{

    // このフェーズが始まったらスポーンする（ObjectiveSystem の phaseSequence_ に並べた名前）
    [SerializeField] private string phaseName_ = "";

    // スポーンさせるプレハブ名
    [SerializeField] private string prefabName_ = "";

    // スポーン位置
    [SerializeField] private Vector3 spawnPos_ = default;

    private bool pendingSpawn_ = false;
    private bool subscribed_ = false;

    public override void Initialize()
    {
        if (!subscribed_)
        {
            MessageBus.Subscribe<PhaseBeganEvent>(OnPhaseBegan);
            subscribed_ = true;
        }
    }

    public override void OnDestroy()
    {
        if (subscribed_)
        {
            MessageBus.Unsubscribe<PhaseBeganEvent>(OnPhaseBegan);
            subscribed_ = false;
        }
    }

    public override void Update()
    {
        if (!pendingSpawn_) return;
        pendingSpawn_ = false;

        Spawn();
    }

    private void OnPhaseBegan(PhaseBeganEvent e)
    {
        if (e.phaseName != phaseName_) return;

        // 実際の生成は Update() まで遅らせる
        pendingSpawn_ = true;
    }

    private void Spawn()
    {
        if (string.IsNullOrEmpty(prefabName_)) return;

        Entity spawned = ecsGroup.CreateEntity(prefabName_);
        if (spawned == null) return;

        Transform spawnedT = spawned.GetComponent<Transform>();
        if (spawnedT == null) return;

        spawnedT.position = spawnPos_;
    }
}
