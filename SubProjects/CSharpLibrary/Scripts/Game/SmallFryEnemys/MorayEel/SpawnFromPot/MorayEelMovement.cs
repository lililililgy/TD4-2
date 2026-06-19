using System;

public class MorayEelMovement : MonoScript {
	/* ----- パラメータ ----- */
	[SerializeField] private float deraySpeed = 1.0f;      
	[SerializeField] private float chargeSpeed = 5.0f;     
	[SerializeField] private float chargeStartTime = 1.0f; 

	/* ----- 実行時状態 ----- */
	private Vector3 velocity_ = Vector3.zero;  // 現在の速度
	private float timer_ = 0.0f;               // 発射からの経過時間
	private bool launched_ = false;            // Launch 済みフラグ
	private Action updateAction_;              // 状況に応じた移動関数


	public override void Initialize() {

		if (!launched_) {
			velocity_ = Vector3.zero;
			timer_ = 0.0f;
			updateAction_ = SpawnMove;
		}
	}

	public override void Update() {
		updateAction_?.Invoke();
	}


	private void SpawnMove() {
		timer_ += Time.deltaTime;

		// 速度の大きさを減衰させ、向きは維持する
		float speed = velocity_.Length();
		speed = Math.Max(0.0f, speed - deraySpeed * Time.deltaTime);
		velocity_ = velocity_.Normalized() * speed;

		transform.position += velocity_ * Time.deltaTime;

		// 一定時間経過で通常突進へ切り替え
		if (timer_ >= chargeStartTime) {
			SwitchToCharge();
		}
	}

	private void SwitchToCharge() {
		velocity_ = Vector3.left * chargeSpeed;
		updateAction_ = ChargeMove;
	}

	private void ChargeMove() {
		transform.position += velocity_ * Time.deltaTime;
	}

    /// <summary>
    /// ツボから発射された瞬間にツボ側から呼ばれる。
    /// </summary>
    public void Launch(Vector3 initialVelocity)
    {
        velocity_ = initialVelocity;
        timer_ = 0.0f;
        launched_ = true;
        updateAction_ = SpawnMove;
    }
}
