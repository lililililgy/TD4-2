using System;

public class SpriteAnimation : MonoScript {
	private SpriteRenderer spriteRenderer;

	// --- 設定パラメータ ---
	[SerializeField] public int rows = 1;
	[SerializeField] public int cols = 1;
	[SerializeField] public float fps = 10f;
	[SerializeField] public bool isLoop = true;
	[SerializeField] public bool isPlay = true;
	[SerializeField] public int totalFrames = 0; // 0 の場合は rows * cols とみなす
	[SerializeField] public bool invertY = false; // DirectX等の左上(0,0)基準の場合は false。左下(0,0)基準の場合は true

	// --- 再生状態 ---
	private float timer = 0f;
	private int currentFrame = 0;
	private bool isFinished = false;

	// --- イベント ---
	public event Action OnAnimationFinished;
	public event Action<int> OnFrameChanged;

	public int CurrentFrame {
		get => currentFrame;
		set => SetFrame(value);
	}

	public bool IsPlaying => isPlay;
	public bool IsFinished => isFinished;

	public override void Initialize() {
		spriteRenderer = entity.GetComponent<SpriteRenderer>();
		if (spriteRenderer == null) {
			Debug.LogError("SpriteAnimation: SpriteRenderer component not found on Entity ID: " + entity.Id);
		}
		ResetAnimation();
	}

	public override void Update() {
		if (spriteRenderer == null) {
			spriteRenderer = entity.GetComponent<SpriteRenderer>();
		}

		if (!isPlay || isFinished || spriteRenderer == null) return;

		float frameDuration = 1f / Math.Max(0.001f, fps);
		timer += Time.deltaTime;

		if (timer >= frameDuration) {
			timer -= frameDuration;
			int maxFrames = totalFrames > 0 ? totalFrames : rows * cols;
			int nextFrame = currentFrame + 1;

			if (nextFrame >= maxFrames) {
				if (isLoop) {
					currentFrame = 0;
					UpdateUV();
					OnFrameChanged?.Invoke(currentFrame);
				} else {
					isFinished = true;
					isPlay = false;
					OnAnimationFinished?.Invoke();
				}
			} else {
				currentFrame = nextFrame;
				UpdateUV();
				OnFrameChanged?.Invoke(currentFrame);
			}
		}
	}

	public void Play() {
		isPlay = true;
		isFinished = false;
	}

	public void Pause() {
		isPlay = false;
	}

	public void Stop() {
		isPlay = false;
		ResetAnimation();
	}

	public void ResetAnimation() {
		timer = 0f;
		currentFrame = 0;
		isFinished = false;
		UpdateUV();
	}

	public void SetFrame(int frameIndex) {
		int maxFrames = totalFrames > 0 ? totalFrames : rows * cols;
		if (frameIndex < 0 || frameIndex >= maxFrames) return;
		currentFrame = frameIndex;
		timer = 0f;
		UpdateUV();
		OnFrameChanged?.Invoke(currentFrame);
	}

	private void UpdateUV() {
		if (spriteRenderer == null || rows <= 0 || cols <= 0) return;

		int colIndex = currentFrame % cols;
		int rowIndex = currentFrame / cols;

		float uSize = 1f / (float)cols;
		float vSize = 1f / (float)rows;

		float uOffset = colIndex * uSize;
		float vOffset = 0f;

		if (invertY) {
			// 左下(0,0)基準の場合、rowIndex=0（1行目）が一番上（V=1-vSize）になるようにする
			vOffset = (rows - 1 - rowIndex) * vSize;
		} else {
			// 左上(0,0)基準の場合
			vOffset = rowIndex * vSize;
		}

		UVTransform uv = new UVTransform();
		uv.scale = new Vector2(uSize, vSize);
		uv.position = new Vector2(uOffset, vOffset);
		uv.rotate = 0.0f;

		spriteRenderer.uvTransform = uv;
	}
}
