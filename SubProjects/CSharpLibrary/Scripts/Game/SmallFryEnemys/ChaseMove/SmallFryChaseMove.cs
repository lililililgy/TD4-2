using System;


public class SmallFryChaseMove : MonoScript {

    [SerializeField] private float chaseSpeed = 5.0f;
    [SerializeField] private float chaseDuration = 3.0f;   
    [SerializeField] private float chaseDistance = 5.0f;   
    [SerializeField] private float rushDistance = 2.0f;    
    [SerializeField] private string playerEntityName = "Player";

    private Vector3 velocity_ = Vector3.zero;
    private float chaseTimer_ = 0.0f;
    private Entity playerEntity_ = null;
    private Action updateAction_ = null;

    public override void Initialize() { }

    public override void Update() {
        updateAction_?.Invoke();
    }


    public void StartChase() {
        // Playerエンティティを探す
        if (playerEntity_ == null) {
            playerEntity_ = ecsGroup.FindEntity(playerEntityName);
        }
        // チェイスActionに移行
        chaseTimer_ = 0.0f;
        updateAction_ = Wait;
    }

    private void ChaseMove() {
        if (playerEntity_ == null) {
            playerEntity_ = ecsGroup.FindEntity(playerEntityName);
        }
        if (playerEntity_ == null) { return; }

        chaseTimer_ += Time.deltaTime;
        if (chaseTimer_ >= chaseDuration) {
            velocity_ = Vector3.zero;
            updateAction_ = Wait;
            return;
        }

        Vector3 toPlayer = playerEntity_.transform.position - transform.position;
        float dist = toPlayer.Length();

        // 近づきすぎたら方向固定で直進
        if (dist <= rushDistance) {
            updateAction_ = RushMove;
            return;
        }

        if (dist > 0.001f) {
            velocity_ = toPlayer.Normalized() * chaseSpeed;
        }
        transform.position += velocity_ * Time.deltaTime;
    }

    private void RushMove() {
        transform.position += velocity_ * Time.deltaTime;
    }

    private void Wait() {
        if (playerEntity_ == null) return;

        Vector3 toPlayer = playerEntity_.transform.position - transform.position;
        if (toPlayer.Length() <= chaseDistance) {
            chaseTimer_ = 0.0f;
            updateAction_ = ChaseMove;
        }
    }
}
