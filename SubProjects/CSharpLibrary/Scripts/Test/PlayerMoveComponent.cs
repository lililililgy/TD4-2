using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class PlayerMoveComponent
    : MonoScript {

    public override void Initialize() { }

    public override void Update() {
        Transform transform = entity.GetComponent<Transform>();
        PlayerInputComponent inputComp = entity.GetScript<PlayerInputComponent>();

        Vector3 moveDir = new Vector3(inputComp.moveDir_.x, 0.0f, inputComp.moveDir_.y);
        transform.position = moveDir * moveSpeed_ * Time.deltaTime;
    }

    [SerializeField] private float moveSpeed_ = 5.0f;

}
