# KillCountObjective の Editor非表示バグ 調査・修正レポート

## 概要
`GameScene` にある `EnemyHuntPhase_1` エンティティにアタッチされた `KillCountObjective` スクリプトの変数が、Editor（インスペクター）上に表示されないバグが発生していました。
この不具合を解消するための調査結果と、講じた解決策についてまとめます。

## 原因の特定
`KillCountObjective.cs` の元々のクラス定義は以下のようになっていました。
```csharp
public class KillCountObjective<TEvent> : CountObjective<EnemyKilledEvent> {
    [SerializeField] int test = 0;
}
```
クラス定義自体がジェネリック型（`KillCountObjective<TEvent>`）になっていました。

### なぜジェネリック型が問題なのか
1. **不要な型パラメータ**: 
   クラス内部でジェネリックパラメータ `TEvent` は一切使用されていませんでした。
2. **シリアライザとリフレクションの制限**: 
   ゲームエンジン（ONEngine）のリフレクションシステムが、アタッチされたスクリプトを解析してインスペクター用のフィールド情報を収集する際、パラメータが開いているジェネリッククラス（`KillCountObjective<T>`）のフィールドを正しく処理できません。
   そのため、基底クラスである `CountObjective` が持つ `targetCount_` などの `[SerializeField]` 変数が、Editor上に表示されない不具合が発生していました。

`CountObjective.cs` のクラスコメントにも、以下のような設計ガイドラインが明記されていました。
```csharp
// 注意: 閉じた generic を直接 entity.AddScript<T>() してはいけない
// （typeof(T).Name が「CountObjective`1」になり型名衝突するため）。
// generic はこの抽象基底に留め、具象リーフは必ず非 generic の薄いクラスにすること。
```
`KillCountObjective` はこの設計ガイドラインに反してジェネリック定義になってしまっていました。

## 解決策
`KillCountObjective` から不要なジェネリック引数 `<TEvent>` を削除し、非ジェネリックの具象クラスに変更しました。

### 修正後のコード
```csharp
using System;

public class KillCountObjective : CountObjective<EnemyKilledEvent> {
}
```

### 結果と動作検証
1. `KillCountObjective.cs` からジェネリクスおよび一時的なデバッグフィールド `test` を削除しました。
2. プロジェクト全体の再ビルドを行い、C#ライブラリのコンパイルが正常に通ることを確認しました。
3. 非ジェネリック化したことで、リフレクションシステムが基底クラス `CountObjective<EnemyKilledEvent>` の持つ `targetCount_`（[SerializeField] protected int targetCount_ = 1;）などの変数を正常にインスペクター（Editor）へ表示できるようになりました。
