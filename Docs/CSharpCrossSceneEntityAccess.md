# C# クロスシーン（複数シーン間）のエンティティデータ取得マニュアル

本ドキュメントでは、ONEngine (TD4-2) において、複数のアクティブシーン（`SceneA` や `SceneB` など）が存在するマルチシーン構成下で、別シーンに存在する `Entity` やそのコンポーネント、およびC#スクリプトのデータを取得する方法について説明します。

---

## 1. 概要

ONEngineのC#側ECS（Entity Component System）では、各シーンは `ECSGroup` という単位で管理されています。
通常、C#スクリプト（`MonoScript` を継承したクラス）からは自身が所属するシーンの `ECSGroup` のみが操作対象となりますが、静的クラス [EntityComponentSystem](file:///c:/Users/k023g/source/repos/TD4-2-Clone2/SubProjects/CSharpLibrary/Scripts/Engine/ECS/EntityComponentSystem/EntityComponentSystem.cs) を経由することで、別シーン（別 `ECSGroup`）のエンティティやコンポーネントのデータを取得・操作することができます。

---

## 2. クロスシーンアクセスの基本手順

別シーンのエンティティデータを取得するには、以下の3つのステップを行います。

1. **対象シーンの `ECSGroup` を取得する**
2. **`ECSGroup` から対象の `Entity` を名前またはIDで取得する**
3. **`Entity` から必要なコンポーネントやC#スクリプトを取得する**

---

## 3. 主要API

### 3.1. ECSGroup の取得
別シーンの `ECSGroup` インスタンスを取得します。

```csharp
static public ECSGroup EntityComponentSystem.GetECSGroup(string name)
```
- **`name`**: 対象シーンの名前（例: `"SceneA"`）。

---

### 3.2. Entity の取得
シーンの `ECSGroup` から特定のエンティティを取得します。

- **名前で検索する場合**
  ```csharp
  public Entity ECSGroup.FindEntity(string name)
  ```
  - **`name`**: 探索対象のエンティティの名前（例: `"Player"`）。

- **IDで取得する場合**
  ```csharp
  public Entity ECSGroup.GetEntity(int id)
  ```
  - **`id`**: 探索対象のエンティティのユニークID。

- **静的ヘルパーによる直接取得**
  `ECSGroup` を直接介さず、1ステップで別シーンのエンティティを取得することも可能です。
  ```csharp
  static public Entity EntityComponentSystem.GetEntity(string groupName, int id)
  ```

---

### 3.3. コンポーネントおよびスクリプトの取得
取得した `Entity` から、必要なコンポーネントやアタッチされている `MonoScript` データを取得します。

- **コンポーネントの取得**: `T Entity.GetComponent<T>() where T : Component`
- **スクリプトの取得**: `T Entity.GetScript<T>() where T : MonoScript`

---

## 4. 実装例

以下は、`SceneB` にある Entity のスクリプトから、現在同時にアクティブになっている `SceneA` の `"Player"` という名前の Entity を検索し、その位置座標（Transform）およびステータスデータを取得する例です。

```csharp
using System;

public class CrossSceneDataTracker : MonoScript {
    
    public override void Update() {
        // 1. 対象となる SceneA の ECSGroup を取得する
        ECSGroup sceneAGroup = EntityComponentSystem.GetECSGroup("SceneA");
        if (sceneAGroup == null) {
            // SceneA がロードされていない、または無効な場合は何もしない
            return;
        }

        // 2. SceneA から "Player" という名前の Entity を検索する
        Entity playerEntity = sceneAGroup.FindEntity("Player");
        if (playerEntity == null) {
            // Playerが見つからない場合は処理をスキップ
            return;
        }

        // 3. Player の座標データを取得する
        var playerPosition = playerEntity.transform.position;
        Console.WriteLine($"[SceneB] Player Position in SceneA: {playerPosition}");

        // 4. Player にアタッチされているカスタムスクリプトからHPなどのデータを取得する
        PlayerStatus playerStatus = playerEntity.GetScript<PlayerStatus>();
        if (playerStatus != null) {
            int currentHp = playerStatus.Hp;
            Console.WriteLine($"[SceneB] Player HP in SceneA: {currentHp}");
        }
    }
}
```

---

## 5. 注意点

1. **ヌルチェック（Null-Safety）の徹底**
   別シーンがアンロードされていたり、対象のエンティティが削除されたりしている可能性があるため、必ず以下の要素に対してヌルチェックを行ってください。
   - `ECSGroup` の取得結果
   - `Entity` の検索結果
   - `GetComponent<T>()` や `GetScript<T>()` の取得結果

2. **探索コストの考慮**
   `ECSGroup.FindEntity(string name)` はグループ内の全エンティティを線形探索するため、毎フレーム `Update` 内で呼び出すとパフォーマンスの低下につながる恐れがあります。
   - **対策**: `Initialize()` などの初期化時に対象エンティティの参照やIDをキャッシュしておく、またはIDベース of `GetEntity(int id)` を使用することをお勧めします。

3. **シーン名の正確性**
   C#側の `groupName` はC++エンジン側でシーン読み込み時に自動設定される文字列に対応します。一般的には拡張子を除いたシーンアセット名（`"SceneA"`など）となりますが、C++側から渡される名前と正確に一致している必要があります。
