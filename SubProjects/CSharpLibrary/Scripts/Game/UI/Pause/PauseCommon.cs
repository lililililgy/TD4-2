using System;
using System.Collections.Generic;

public class PauseCommon : MonoScript {

	protected bool isSelect = false;
	Vector3 savePos;

	[SerializeField] float time = 0f;
	//[SerializeField] float 
	[SerializeField] Vector4 defaultColor = Vector4.one;
	[SerializeField] Vector4 selectedColor = Vector4.red;

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


	public static float SteppedTimeEasing(float t, int steps) {
		// 進行度tを階段状にする
		float steppedT = Mathf.Floor(t) / steps;
		return steppedT;
	}
}
