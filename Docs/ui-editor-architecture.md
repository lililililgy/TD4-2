# ONEngine UIシステム アーキテクチャ設計仕様書

## 1. 概要
本システムは、Immediate Mode GUI（ImGui）を用いたノードベースのエディタ機能と、ゲーム本番（Releaseビルド）で動作するEntity Component System（ECS）ベースのUIランタイムを完全に分離したアーキテクチャを採用する。

エディタ上で視覚的に構築したUIのレイアウトおよびナビゲーション（フォーカス移動）情報は、独立したJSONアセット（Prefab）としてシリアライズされ、ランタイム側でC++のECSコアとC#スクリプトによって駆動される。

---

## 2. ワークフローとデータ管理

### エディタ機能（ImGuiノードエディタ）
* **役割:** UI要素の配置、階層の構築、およびナビゲーションリンクの視覚的な定義。
* **操作:** ImGuiのノードエディタ上でUI要素を配置し、要素間の遷移条件（生のキー入力）をピンで接続して定義する。
* **シリアライズ:** 編集結果はシーンファイルとは独立した単一のUIアセット（例: `TitleMenu.prefab`）として保存される。これにより、複数人での開発時におけるシーンファイルのコンフリクトを防止する。

---

## 3. コンポーネント設計（C++ ECSコア）
ゲーム実行時にPrefabからインスタンス化されるデータ構造。

### `UIGroupComponent`
UIのまとまり（画面やメニュー単位）を管理し、階層とフォーカス状態を制御する。
* `EntityId CurrentSelected`: 現在フォーカスされている子要素のEntity。
* `bool IsFocused`: 入力を受け付ける状態かどうかのフラグ。
* `bool IsVisible`: 描画システムによるレンダリングを行うかどうかのフラグ。
* `EntityId ParentGroup`: 親グループのEntity（階層を戻る際に使用）。

### `UIElementComponent`
個々のUI要素（ボタンやテキストなど）の基本情報を保持する。
* `EntityId GroupId`: 自身が所属する親グループ（`UIGroupComponent`）のEntity。
* `std::string ElementId`: C#スクリプトなどから対象を識別するための一意の文字列（例: "StartButton"）。
* `int ElementIndex`: 識別用のインデックス番号。

### `UILinkNavigationComponent`
リンク指定型のナビゲーションデータ。ノードエディタでの結線情報に相当する。
* `std::unordered_map<KeyCode, EntityId> Links`: エンジンの生入力（Raw Input）の列挙体をキーとし、遷移先のEntityを値とするマップ。

---

## 4. システム設計（C++ 処理フロー）

### `UIHierarchySystem`（階層・状態管理）
* 各グループの `IsFocused` および `IsVisible` の状態を監視。
* `IsFocused == false` のグループに属する要素に対する入力処理やスクリプトの更新（Update）をスキップし、パフォーマンスを最適化する。

### `UIInputNavigationSystem`（入力・遷移制御）
1. エンジンの入力システムから生のキー入力を取得。
2. `IsFocused == true` であるグループを特定し、その `CurrentSelected` 要素を取得。
3. 取得した要素の `UILinkNavigationComponent` の `Links` マップを参照。
4. 入力されたキーと一致する遷移先が存在する場合、`CurrentSelected` を新しいEntityに更新し、状態変化のイベントをC#スクリプト側へ通知する。

### `GameUIRenderSystem`（ランタイム描画）
* `IsVisible == true` であるグループに属する要素を抽出。
* ImGuiは使用せず、自作エンジンの描画システム（スプライト、フォントレンダリング等）を用いて画面に描画する。

---

## 5. イベント駆動とスクリプト連携（C#）
エンジンの既存仕様に則り、`"type": "Script"` と `"type": "Variables"` コンポーネントを使用してC#スクリプトをアタッチする。

### 処理の柔軟性（個別制御と一括制御）
* **個別制御:** 各UI要素（Entity）に固有のスクリプトをアタッチし、`OnSelect` や `OnSubmit` といったイベントを個別に処理する。
* **一括制御:** 親となる `UIGroupComponent` を持つEntityに統括スクリプトを1つアタッチし、引数として渡される `ElementId` を用いてSwitch文などでグループ内のイベントを一括処理する（コード修正範囲の最小化）。

---

## 6. シリアライズ形式（JSON構造例）
既存のエンティティ保存形式に準拠したUIデータの保存例。

```json
{
    "active": true,
    "components": [
        {
            "enable": 1,
            "type": "Transform",
            "position": { "x": 0.0, "y": 0.0, "z": 0.0 },
            "rotate": { "w": 1.0, "x": 0.0, "y": 0.0, "z": 0.0 },
            "scale": { "x": 1.0, "y": 1.0, "z": 1.0 }
        },
        {
            "enable": 1,
            "type": "UIElementComponent",
            "groupId": "title_group_guid",
            "elementId": "StartButton",
            "elementIndex": 0
        },
        {
            "enable": 1,
            "type": "UILinkNavigationComponent",
            "links": {
                "KEY_DOWN": "option_button_guid",
                "KEY_UP": "exit_button_guid",
                "GAMEPAD_DPAD_DOWN": "option_button_guid"
            }
        },
        {
            "type": "Variables",
            "TitleMenuScript": {
                "isHovered_": false
            }
        },
        {
            "enable": 1,
            "scripts": [
                {
                    "enable": true,
                    "name": "TitleMenuScript"
                }
            ],
            "type": "Script"
        }
    ],
    "guid": "start_button_guid",
    "name": "StartButtonUI",
    "parentGuid": "title_group_guid",
    "prefabName": "TitleMenu.prefab"
}