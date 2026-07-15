using System;
using System.Collections.Generic;

public class UITest1 : MonoScript {

	bool isSelect = false;
	Vector3 savePos;

	public override void Initialize() {
		savePos = transform.position;
	}

	public override void Update() {
		if (isSelect) {
			float posY = Mathf.Sin(SteppedTimeEasing(Time.time * 3f, 1)) * 0.1f;
			transform.position = savePos + new Vector3(0, posY, 0);
		} else {
			transform.position = savePos;
		}
	}


	/// <summary>
	/// 選択された時
	/// </summary>
	public void OnSelect() {
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if (renderer) {
			renderer.color = Vector4.red;
		}
		isSelect = true;
	}

	public void OnDeselect() {
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if (renderer) {
			renderer.color = Vector4.one;
		}
		isSelect = false;
	}


	public static float SteppedTimeEasing(float t, int steps) {
		// 進行度tを階段状にする
		float steppedT = Mathf.Floor(t * steps) / steps;
		return steppedT;
	}

}
