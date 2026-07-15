using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


// スポーンする敵の種類と発生確率の分布を持つ。
// データの実体は SpawnEntry のリスト。確率は重みの相対比で表現する。
public class Biome {
    private readonly List<SpawnEntry> entries_;

    public Biome(List<SpawnEntry> entries) {
        entries_ = entries ?? new List<SpawnEntry>();
    }

    // 重み付き抽選で敵の種類を1つ返す。テーブルが空なら null。
    public string PickEnemyType() {
        float total = 0.0f;
        foreach (var e in entries_) {
            if (e.weight > 0.0f) total += e.weight;
        }
        if (total <= 0.0f) return null;

        float r = RandomUtil.RandomRange(0.0f, total);
        foreach (var e in entries_) {
            if (e.weight <= 0.0f) continue;
            r -= e.weight;
            if (r <= 0.0f) return e.enemyType;
        }
        // 浮動小数の誤差で抜けた場合のフォールバック
        return entries_[entries_.Count - 1].enemyType;
    }
}

// 敵のスポーンシステム
public class EnemySpawnSystem : MonoScript {
    [SerializeField] private bool isActive_ = false;

    [SerializeField] private string originName_ = "EnemySpawnOrigin";
    [SerializeField] private string heatMapEntityName_ = "EnemyHeatMap";
    // スポーン範囲
    [SerializeField] private Vector2 spawnAreaMin_ = default;
    [SerializeField] private Vector2 spawnAreaMax_ = default;
    // 除外範囲（スポーンしない範囲）
    [SerializeField] private Vector2 exclusionAreaMin_ = default;
    [SerializeField] private Vector2 exclusionAreaMax_ = default;
    // スポーン間隔
    [SerializeField] private float spawnInterval_ = 5.0f;
    private float spawnTimer_ = 0.0f;

    // 1フレームでスポーンする最大数
    [SerializeField] private int maxSpawnCount_ = 10;
    // セル内の敵の数がこの値以上の場合、そのセルの重みを0にする
    [SerializeField] private int heatWeightThreshold_ = 5;
    // 重みの畳み込み範囲(セル数)。周囲のセルの重みを合成し、密集地帯の影響が近隣セルにも及ぶようにする
    [SerializeField] private int convolutionRadius_ = 1;
    // スポーン可能な敵の総数の上限。上限に達している場合はスポーンしない
    [SerializeField] private int maxEnemyCount_ = 100;

    // 現在のバイオームの出現テーブル。エディタで設定する。
    [SerializeField]
    private List<SpawnEntry> defaultSpawnTable_ = new List<SpawnEntry> {
        new SpawnEntry { enemyType = "MorayEel",  weight = 1.0f },
        new SpawnEntry { enemyType = "SpikeFish", weight = 1.0f },
    };

    // 矩形リージョン方式のバイオームエリア。名前はシーンのエンティティ名と一致させる。
    // リスト順＝優先度（重なりは先勝ち）
    [SerializeField] private List<string> biomeNames_ = new List<string>();
    // biomeNames_ から解決済みの BiomeArea のキャッシュ
    private List<BiomeArea> biomeAreas_ = new List<BiomeArea>();

    // 抽選候補のセルとその重み。畳み込み後の重みは実数になるため、セルIDと組にしてリストで保持する。
    private struct WeightedCell {
        public Vector2Int cellId;
        public float weight;
    }
    private List<WeightedCell> spawnCellWeights_ = new List<WeightedCell>();

    // Biome（実行時にテーブルから構築する）
    private Biome defaultBiome_;

    public override void Initialize() {
        defaultBiome_ = new Biome(defaultSpawnTable_);

        biomeAreas_.Clear();
        foreach (var name in biomeNames_) {
            Entity biomeEntity = ecsGroup.FindEntity(name);
            if (biomeEntity == null) continue;

            BiomeArea biomeArea = biomeEntity.GetScript<BiomeArea>();
            if (biomeArea == null) continue;

            biomeAreas_.Add(biomeArea);
        }
    }

    public override void Update() {
        if (!isActive_) return;

        spawnTimer_ += Time.deltaTime;

        // スポーンセルの重みをクリアする
        spawnCellWeights_.Clear();

        // スポーン可能な敵の総数が上限に達している場合はスポーンしない
        Entity heatMapEnt = ecsGroup.FindEntity(heatMapEntityName_);
        EnemyHeatMap heatMap = heatMapEnt?.GetScript<EnemyHeatMap>();
        if (heatMap == null) return;

        CalculateSpawnCellWeights(heatMap);

        if (heatMap.GetEnemyCount() >= maxEnemyCount_) return;

        int spawnCount = 0;
        // スポーン間隔を超えた場合、スポーン処理を行う
        while (spawnTimer_ > spawnInterval_ && spawnCount < maxSpawnCount_) {
            spawnTimer_ -= spawnInterval_;
            spawnCount++;

            // 敵の発生処理
            Vector2 spawnPos = CalculateSpawnPos();
            Biome biome = ResolveBiome(spawnPos);
            if (biome == null) continue;

            string enemyType = biome.PickEnemyType();
            if (enemyType == null) continue;

            SpawnEnemy(enemyType, spawnPos);
        }
    }

    public override void OnCollisionStay(Entity collision) {
        DespawnTimer despawnTimer = collision.GetScript<DespawnTimer>();
        if (despawnTimer == null) return;
        despawnTimer.ResetTimer();
    }

    // スポーン地点に適用するバイオームを解決する。
    // biomeAreas_ をリスト順（＝優先度）に走査し、最初にヒットしたエリアの Biome を返す。
    // 該当エリアが無ければ defaultBiome_（defaultSpawnTable_）にフォールバックする。
    private Biome ResolveBiome(Vector2 spawnPos) {
        foreach (var biomeArea in biomeAreas_) {
            if (biomeArea.Contains(spawnPos)) {
                return biomeArea.GetBiome();
            }
        }
        return defaultBiome_;
    }

    // 敵のスポーン位置を計算する。セルを重みに応じて抽選し、そのセル範囲内のランダムな座標を返す。
    private Vector2 CalculateSpawnPos() {
        EnemyHeatMap heatMap = entity.GetScript<EnemyHeatMap>();

        float total = 0.0f;
        foreach (var c in spawnCellWeights_) {
            if (c.weight > 0.0f) total += c.weight;
        }

        if (total <= 0.0f) {
            // 候補セルが無い場合は原点にスポーンさせる
            Vector2 originePos = new Vector2();
            Entity origineEntity = ecsGroup.FindEntity(originName_);
            if (origineEntity != null) {
                Transform origineT = origineEntity.GetComponent<Transform>();
                originePos = new Vector2(origineT.position.x, origineT.position.y);
            }
            return originePos;
        }

        // 浮動小数の誤差で最後まで抜けた場合に備え、末尾の候補をフォールバックにしておく
        Vector2Int selectedCellId = spawnCellWeights_[spawnCellWeights_.Count - 1].cellId;
        float r = RandomUtil.RandomRange(0.0f, total);
        foreach (var c in spawnCellWeights_) {
            if (c.weight <= 0.0f) continue;
            r -= c.weight;
            if (r <= 0.0f) {
                selectedCellId = c.cellId;
                break;
            }
        }

        // 選ばれたセルの範囲内でランダムな座標を返す
        Vector2 cellSize = heatMap.GetCellSize();
        Vector2 cellMin = new Vector2(selectedCellId.x * cellSize.x, selectedCellId.y * cellSize.y);
        return new Vector2(
            RandomUtil.RandomRange(cellMin.x, cellMin.x + cellSize.x),
            RandomUtil.RandomRange(cellMin.y, cellMin.y + cellSize.y)
        );
    }

    private void SpawnEnemy(string enemyType, Vector2 spawnPos) {
        // 敵の生成処理
        Entity spawnedEnemy = ecsGroup.CreateEntity(enemyType);
        Transform enemyT = spawnedEnemy.GetComponent<Transform>();
        enemyT.position = new Vector3(spawnPos.x, spawnPos.y, 0.0f);
    }

    private void CalculateSpawnCellWeights(EnemyHeatMap heatMap) {
        // cell の候補を求める。
        Vector2 originePos = new Vector2();
        Entity origineEntity = ecsGroup.FindEntity(originName_);
        if (origineEntity != null) {
            Transform origineT = origineEntity.GetComponent<Transform>();
            originePos = new Vector2(origineT.position.x, origineT.position.y);
        }

        Vector2Int origineCell = heatMap.GetCellId(originePos);

        Vector2 spawnAreaSize = spawnAreaMax_ - spawnAreaMin_;
        Vector2Int spawnAreaForCell = heatMap.GetCellId(spawnAreaSize);
        Vector2Int ltCellId = new Vector2Int(origineCell.x - spawnAreaForCell.x / 2, origineCell.y - spawnAreaForCell.y / 2);
        Vector2Int rbCellId = new Vector2Int(origineCell.x + spawnAreaForCell.x / 2, origineCell.y + spawnAreaForCell.y / 2);

        Vector2 exclusionAreaSize = exclusionAreaMax_ - exclusionAreaMin_;
        Vector2Int exclusionAreaForCell = heatMap.GetCellId(exclusionAreaSize);
        Vector2Int exclusionLtCellId = new Vector2Int(origineCell.x - exclusionAreaForCell.x / 2, origineCell.y - exclusionAreaForCell.y / 2);
        Vector2Int exclusionRbCellId = new Vector2Int(origineCell.x + exclusionAreaForCell.x / 2, origineCell.y + exclusionAreaForCell.y / 2);

        // 2次元の畳み込みをそのまま行うと1セルあたり (2r+1)^2 回の参照が必要になるため、
        // カーネルを dx 方向 × dy 方向に分解できる形にし、横→縦の1次元畳み込み2回
        // (合計 2*(2r+1) 回の参照)に落とすことでループ回数を削減する。
        float kernelSum = ComputeKernelSum();

        // 横方向(x)の1次元畳み込み。縦方向のパスで参照するぶん、y は畳み込み半径ぶん広げて計算しておく。
        Dictionary<int, float> horizontalBlur = new Dictionary<int, float>();
        for (int y = ltCellId.y - convolutionRadius_; y <= rbCellId.y + convolutionRadius_; y++) {
            for (int x = ltCellId.x; x <= rbCellId.x; x++) {
                float sum = 0.0f;
                for (int dx = -convolutionRadius_; dx <= convolutionRadius_; dx++) {
                    sum += BaseWeight(heatMap, new Vector2Int(x + dx, y)) * KernelWeight(dx);
                }
                horizontalBlur[heatMap.CalclateHash(new Vector2Int(x, y))] = sum / kernelSum;
            }
        }

        for (int x = ltCellId.x; x <= rbCellId.x; x++) {
            for (int y = ltCellId.y; y <= rbCellId.y; y++) {
                // 除外範囲のセルをスキップする
                for (int ex = exclusionLtCellId.x; ex <= exclusionRbCellId.x; ex++) {
                    for (int ey = exclusionLtCellId.y; ey <= exclusionRbCellId.y; ey++) {
                        if (x == ex && y == ey) {
                            // 除外範囲内のセルはスキップ
                            continue;
                        }
                    }
                }

                // 縦方向(y)の1次元畳み込み。横方向の結果を再利用する。
                float sum = 0.0f;
                for (int dy = -convolutionRadius_; dy <= convolutionRadius_; dy++) {
                    sum += horizontalBlur[heatMap.CalclateHash(new Vector2Int(x, y + dy))] * KernelWeight(dy);
                }

                Vector2Int cellId = new Vector2Int(x, y);
                spawnCellWeights_.Add(new WeightedCell { cellId = cellId, weight = sum / kernelSum });
            }
        }
    }

    // カーネル(距離が離れるほど影響を弱める)の総和。畳み込み結果の正規化に使う。
    private float ComputeKernelSum() {
        float sum = 0.0f;
        for (int d = -convolutionRadius_; d <= convolutionRadius_; d++) {
            sum += KernelWeight(d);
        }
        return sum;
    }

    // 1次元方向のカーネル重み。dx, dy それぞれに独立して適用することで2次元カーネルを分解する。
    private float KernelWeight(int d) {
        return 1.0f / (1.0f + Math.Abs(d));
    }

    // セル単体の基礎重み。敵の数が heatWeightThreshold_ 以上なら 0 にする
    private float BaseWeight(EnemyHeatMap heatMap, Vector2Int cellId) {
        int heat = heatMap.GetHeatByCellId(cellId);
        return (heat >= heatWeightThreshold_) ? 0.0f : 1.0f;
    }
}
