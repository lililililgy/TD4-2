using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

/// <summary>
/// MovementArea の範囲内に Entity が収まるように位置を補正するコンポーネント
/// </summary>
public class MovementAreaClamper : MonoScript {

    [SerializeField] private bool existArea_ = false;
    [SerializeField] private bool fixedCollisionBounds_ = false; // radius に Colliderの大きさを反映するかどうか
    [SerializeField] private string movementAreaEntityName_ = "Stage";
    [SerializeField] private float radius_ = 0f;
    [SerializeField] private Vector3 fixedPos_ = Vector3.zero;
    public override void Update() {
        if (fixedCollisionBounds_) {
            FixRadius();
        }

        // MovementAreaClamper を持つ Entity を取得
        Entity areaEnt = ecsGroup.FindEntity(movementAreaEntityName_);

        if (areaEnt == null) {
            existArea_ = false;
            return;
        }

        existArea_ = true;
        Clamp(areaEnt);
        _ = existArea_;
    }

    private void FixRadius() {
        CircleCollider circleCollider = entity.GetComponent<CircleCollider>();
        if (transform == null || circleCollider == null) {
            return;
        }

        radius_ = transform.scale.x * circleCollider.radius;
    }

    public void Clamp(Entity areaEnt) {
        if (areaEnt == null) {
            return;
        }
        Transform transform = entity.GetComponent<Transform>();
        Vector3 pos = transform.position;

        // MovementArea の範囲を取得
        MovementArea area = areaEnt.GetScript<MovementArea>();

        // 範囲内に収まるように補正
        pos.x = Mathf.Clamp(pos.x, area.Min.x + radius_, area.Max.x - radius_);
        pos.y = Mathf.Clamp(pos.y, area.Min.y + radius_, area.Max.y - radius_);
        fixedPos_ = pos;
        transform.position = pos;
    }

};