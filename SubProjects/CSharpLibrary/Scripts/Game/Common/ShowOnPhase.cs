using System;
using System.Collections.Generic;

// 指定したフェーズの間だけ、同じエンティティの SpriteRenderer / TextRenderer を表示する。
// フェーズ外・進行終了後は隠す。両方持っている必要はなく、付いている方だけを切り替える。
//
// 表示切り替えは Component.enable(0/1) で行う。ComponentBatchManager が
// batch[i].enable = comp.enable として native に送るため、これが表示を制御する実体
// （BatchData 側にも enable があるが、そちらは native からの読み取り用）。
//
// PhaseBeganEvent は購読せず、Update() で ObjectiveSystem を毎フレーム引く（pull 型）。
// このスクリプトは GameUIScene に置かれ、ObjectiveSystem は GameScene にいる、という
// 別 ECSGroup 構成が前提で、イベント購読だと以下の2点で必ず壊れるため:
//
// (1) 最初のフェーズのイベントを取りこぼす。GameUIScene は GameScene 側の
//     AddSceneScript.Update() で追加されるので、ObjectiveSystem.Initialize() が
//     発行する最初の PhaseBeganEvent の時点でまだ存在しない。
// (2) 受け取れても enable が捨てられる。ECSGroup.UpdateEntities() は
//     ReceiveAllBatches → Awake/Initialize → Send → Update → Send の順で、
//     先頭の Receive が comp.enable を native 値で上書きする。
//     activeGroupNames_ は読み込み順（GameScene → GameUIScene）なので、
//     GameScene の Update 中に飛んできたイベントで enable=1 にしても、
//     直後に始まる GameUIScene の Receive がそれを 0 に戻してしまう。
//
// Update() の中で書けば Receive の後・Send の前になるので、どちらも起きない。
// 同じ理由で、バッチ同期される値は他グループのイベントハンドラから書いてはいけない。
public class ShowOnPhase : MonoScript
{

    // この名前のフェーズの間だけ表示する（例: "TutorialPhase_Move"）。
    // 複数フェーズにまたがって出しっぱなしにもできる。
    [SerializeField] private List<string> visiblePhases_ = new List<string>();

    // ObjectiveSystem が付いているエンティティ名
    [SerializeField] private string objectiveSystemEntityName_ = "GameController";

    // ObjectiveSystem がいるシーン(ECSグループ)名。
    // このUIを GameUIScene のような別シーンに置く場合は、ecsGroup.FindEntity では
    // 自分のグループしか探せないため、ここに探索先のシーン名を入れる。
    // 空文字なら自分と同じグループを探す。
    [SerializeField] private string objectiveSystemSceneName_ = "GameScene";

    private ObjectiveSystem objectiveSystem_;

    // エディタ側で enable=false にし忘れても契約（= visiblePhases_ の間だけ見える）が
    // 守られるように、まず隠しておく。
    public override void Initialize()
    {
        objectiveSystem_ = ResolveObjectiveSystem();
        SetVisible(false);
    }

    public override void Update()
    {
        // ObjectiveSystem 側のシーンやエンティティがまだ無かった場合に備えた遅延再解決
        if (objectiveSystem_ == null)
        {
            objectiveSystem_ = ResolveObjectiveSystem();
            if (objectiveSystem_ == null)
            {
                SetVisible(false);
                return;
            }
        }

        // IsPhaseBegan を見るのは、フェーズエンティティの解決待ち中
        // （目標がまだ動き出していない状態）を表示対象から外すため。
        // 進行終了後は CurrentPhaseName が "" になるので、そのまま隠れる。
        SetVisible(objectiveSystem_.IsPhaseBegan &&
                   visiblePhases_ != null &&
                   visiblePhases_.Contains(objectiveSystem_.CurrentPhaseName));
    }

    private void SetVisible(bool visible)
    {
        int enable = visible ? 1 : 0;

        SpriteRenderer sprite = entity.GetComponent<SpriteRenderer>();
        if (sprite != null)
        {
            sprite.enable = enable;
        }

        TextRenderer text = entity.GetComponent<TextRenderer>();
        if (text != null)
        {
            text.enable = enable;
        }
    }

    private ObjectiveSystem ResolveObjectiveSystem()
    {
        ECSGroup group = ecsGroup;
        if (!string.IsNullOrEmpty(objectiveSystemSceneName_))
        {
            group = EntityComponentSystem.GetECSGroup(objectiveSystemSceneName_);
            if (!group) return null; // シーンがまだ読まれていない
        }

        Entity systemEntity = group.FindEntity(objectiveSystemEntityName_);
        return systemEntity != null ? systemEntity.GetScript<ObjectiveSystem>() : null;
    }
}
