using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

/// <summary>
/// MovementArea の範囲内に Entity が収まるように位置を補正するコンポーネント
/// </summary>
public class MovementAreaClamper : MonoScript {
    [SerializeField] private string movementAreaEntityName = "Stage";
    [SerializeField] private float radius = 0f;

    public void Clamp(Entity entity) {
        Transform transform = entity.GetComponent<Transform>();
        Vector3 pos = transform.position;
        // MovementArea の範囲を取得
        Entity areaEntity = ecsGroup.FindEntity(movementAreaEntityName);
        MovementArea area = areaEntity.GetScript<MovementArea>();

        if (area != null) {
            return;
        }

        // 範囲内に収まるように補正
        pos.x = Mathf.Clamp(pos.x, area.Min.x + radius, area.Max.x - radius);
        pos.y = Mathf.Clamp(pos.y, area.Min.y + radius, area.Max.y - radius);
        transform.position = pos;
    }

};