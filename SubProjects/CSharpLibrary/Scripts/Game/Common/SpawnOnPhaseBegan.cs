using System;

// 指定したフェーズの開始でプレハブをスポーンさせる。
// スポーンさせたい対象ごとに空エンティティ（マーカー）を置き、これを付けて設定する。
// フェーズエンティティ自身に付けてもよい（BossBattlePhase_2 は KillCountObjective と同居）。
//
// count_ を 2 以上にすると、その数だけまとめてスポーンする。
// spawnAroundEntityName_ を入れると、そのエンティティの位置を中心に spawnRadius_ の
// 円周上へ等間隔（360°/count_）に並べる。開始角だけ毎回ランダムにするので向きは変わるが、
// 必ず全方位に散る（完全ランダムだと偏って固まることがあるため）。
// spawnAroundEntityName_ が空なら spawnPos_ をそのまま中心に使う。
//
// 注意: 1つのエンティティに同じ型のスクリプトは1つしか付けられない
// （Entity の scripts_ が型名をキーにした Dictionary のため）。
// 1フェーズで違うプレハブを混ぜたい場合は、マーカーエンティティを複数用意する。
//
// PhaseBeganEvent の購読は Initialize() で行っている。ObjectiveSystem.Initialize() が
// 最初の PhaseBeganEvent を発行するため、phaseName_ に「先頭のフェーズ」を指定した場合だけは
// 購読が間に合わず取りこぼしうる（Entity.GetScripts() は Dictionary 列挙で順序が保証されない）。
// 先頭フェーズでスポーンさせたくなったら、ShowOnPhase のように pull 型へ寄せること。
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

    // スポーン位置。spawnAroundEntityName_ が空のときの中心になる
    [SerializeField] private Vector3 spawnPos_ = default;

    // スポーンさせる数
    [SerializeField] private int count_ = 1;

    // この名前のエンティティの位置を中心にする。空なら spawnPos_ を使う
    [SerializeField] private string spawnAroundEntityName_ = "";

    // 中心からの距離。0 なら中心にそのまま重ねてスポーンする
    [SerializeField] private float spawnRadius_ = 0.0f;

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
        if (count_ <= 0) return;

        Vector3 center = ResolveCenter();

        float startAngle = RandomUtil.RandomRange(0.0f, (float)(Math.PI * 2.0));
        float step = (float)(Math.PI * 2.0) / count_;

        for (int i = 0; i < count_; i++)
        {
            Vector3 pos = center;
            if (spawnRadius_ > 0.0f)
            {
                float angle = startAngle + step * i;
                pos.x += (float)Math.Cos(angle) * spawnRadius_;
                pos.y += (float)Math.Sin(angle) * spawnRadius_;
            }

            SpawnOne(pos);
        }
    }

    // 円周の中心。対象エンティティが見つからなければ spawnPos_ にフォールバックする
    private Vector3 ResolveCenter()
    {
        if (string.IsNullOrEmpty(spawnAroundEntityName_)) return spawnPos_;

        Entity centerEntity = ecsGroup.FindEntity(spawnAroundEntityName_);
        if (centerEntity == null) return spawnPos_;

        Transform centerT = centerEntity.GetComponent<Transform>();
        if (centerT == null) return spawnPos_;

        return centerT.position;
    }

    private void SpawnOne(Vector3 pos)
    {
        Entity spawned = ecsGroup.CreateEntity(prefabName_);
        if (spawned == null) return;

        Transform spawnedT = spawned.GetComponent<Transform>();
        if (spawnedT == null) return;

        spawnedT.position = pos;
    }
}
