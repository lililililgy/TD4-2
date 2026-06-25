using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class DebugCamera2D : MonoScript {
    [SerializeField] private bool isActive_;
    [SerializeField] private float moveSpeed_;
    [SerializeField] private float dragSensitivity_;
    [SerializeField] private Vector3 position_;
    [SerializeField] private Vector3 scale_;
    [SerializeField] private Vector3 velocity_;

    public override void Initialize() {
        isActive_ = true;
        moveSpeed_ = 1.0f;
        dragSensitivity_ = 1.0f;
        position_ = Vector3.zero;
        scale_ = Vector3.one;
    }

    public override void Update() {
        if (!isActive_) {
            return;
        }

        float mouseWheel = Input.MouseWheel();

        // Zoom Control (With Ctrl)
        if (Input.PressKey(KeyCode.LeftControl)) {
            bool zoomChanged = false;
            if (mouseWheel > 0f || Input.TriggerKey(KeyCode.Equals)) {
                // Zoom In (Make viewport smaller -> decrease scale)
                scale_ *= 0.9f;
                zoomChanged = true;
            }
            else if (mouseWheel < 0f || Input.TriggerKey(KeyCode.Minus)) {
                // Zoom Out (Make viewport larger -> increase scale)
                scale_ *= 1.1f;
                zoomChanged = true;
            }

            if (zoomChanged) {
                if (scale_.x < 0.1f) scale_ = new Vector3(0.1f, 0.1f, 0.1f);
                if (scale_.x > 10.0f) scale_ = new Vector3(10.0f, 10.0f, 10.0f);
                transform.scale = scale_;
            }
        }
        // Speed Adjustment (No Ctrl)
        else {
            if (Input.TriggerKey(KeyCode.Minus)) {
                moveSpeed_ *= 0.5f;
                if (moveSpeed_ < 0.1f) moveSpeed_ = 0.1f;
            }
            if (mouseWheel < 0f) {
                moveSpeed_ *= 0.9f;
                if (moveSpeed_ < 0.1f) moveSpeed_ = 0.1f;
            }

            if (Input.TriggerKey(KeyCode.Equals)) {
                moveSpeed_ *= 2.0f;
                if (moveSpeed_ > 10.0f) moveSpeed_ = 10.0f;
            }
            if (mouseWheel > 0f) {
                moveSpeed_ *= 1.1f;
                if (moveSpeed_ > 10.0f) moveSpeed_ = 10.0f;
            }
        }

        // Synchronize state when not active/interacting
        if (transform) {
            if (!Input.PressMouse(Mouse.Right)) {
                position_ = transform.position;
                scale_ = transform.scale;
            }
        }

        // Interactive operations (Right Mouse Drag)
        if (Input.PressMouse(Mouse.Right)) {
            float speed = moveSpeed_;
            if (Input.PressKey(KeyCode.LeftShift)) {
                speed *= 2.0f;
            }

            // Keyboard Movement
            velocity_ = Vector3.zero;
            if (Input.PressKey(KeyCode.W)) velocity_.y += speed; // Move Up
            if (Input.PressKey(KeyCode.S)) velocity_.y -= speed; // Move Down
            if (Input.PressKey(KeyCode.A)) velocity_.x -= speed; // Move Left
            if (Input.PressKey(KeyCode.D)) velocity_.x += speed; // Move Right

            position_ += velocity_ * 10f;

            // Mouse Drag Pan
            Vector2 mouseMove = Input.MouseVelocity();
            // In 2D, dragging down should move the camera up (grab-pan effect)
            position_.x -= mouseMove.x * scale_.x * dragSensitivity_;
            position_.y += mouseMove.y * scale_.y * dragSensitivity_;

            transform.position = position_;
        }
    }
}
