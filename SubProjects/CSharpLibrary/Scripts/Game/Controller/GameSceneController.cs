using System;
using System.Collections.Generic;



public class GameSceneController : MonoScript
{

	// ポーズを開くキー割り当て（キーボードとゲームパッドの両方をサポート）
	[SerializeField] private List<KeyCode> pauseKeys_ = new List<KeyCode> { KeyCode.Escape };
	[SerializeField] private List<Gamepad> pauseButtons_ = new List<Gamepad> { Gamepad.Start };

	// PlayerDeadEvent を受け取ったら立てるラッチ。以降クリアされない
	// （死亡フレームのうちに遷移するが、発行元の実行順に依存しないようフラグで受ける）。
	private bool isGameOver_ = false;
	private bool subscribed_ = false;

	// 購読は Awake() で行う。Initialize() だとエンティティの並び順次第で
	// 同フレームの発行を取りこぼしうるため。
	public override void Awake()
	{
		if (!subscribed_)
		{
			MessageBus.Subscribe<PlayerDeadEvent>(OnPlayerDead);
			subscribed_ = true;
		}
	}

	public override void Initialize()
	{

	}

	public override void OnDestroy()
	{
		if (subscribed_)
		{
			MessageBus.Unsubscribe<PlayerDeadEvent>(OnPlayerDead);
			subscribed_ = false;
		}
	}

	public override void Update()
	{
		if (CheckInput())
		{
			SceneManager.Add("PauseScene");
			SceneManager.SetUpdatePaused("GameScene", true);
		}

		// ゲームオーバーはクリアより先に判定する（同フレームで両立した場合は死亡を優先）
		if (CheckGameOver())
		{
			// ゲームオーバーシーンへ遷移
			//SceneManager.LoadScene("GameOverScene");
			return;
		}

		// ゲームをクリアしたか
		if (CheckClear())
		{
			// 最後まで遊び切ったのでコンティニュー地点を破棄する（次回は最初から）
			ContinueState.Clear();

			// クリアシーンへ遷移
			SceneManager.LoadScene("GameClearScene");
		}

	}

	private void OnPlayerDead(PlayerDeadEvent e)
	{
		isGameOver_ = true;
	}



	// ポーズは押しっぱなしで開き直されると困るので、押された瞬間だけを見る
	private bool CheckInput()
	{
		return InputUtil.AnyTriggered(pauseKeys_, pauseButtons_);
	}

	private bool CheckClear()
	{
		ObjectiveSystem objectiveSystem = entity.GetScript<ObjectiveSystem>();
		if (objectiveSystem == null)
		{
			return false;
		}
		return objectiveSystem.IsFinished;
	}

	private bool CheckGameOver()
	{
		return isGameOver_;
	}

}
