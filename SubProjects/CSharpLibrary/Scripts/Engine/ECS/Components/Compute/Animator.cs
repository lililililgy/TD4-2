using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public class Animator : Component {
	public const int MAX_ANIMATION_LAYERS = 4;
	public const int MAX_ANIMATION_STATES_PER_LAYER = 2;

	[StructLayout(LayoutKind.Sequential)]
	public struct AnimationState {
		public uint clipId;
		public float time;
		public float weight;
		public byte isLoop;
		private byte pad1, pad2, pad3; // Alignment for next float
		public float playbackSpeed;
		public float prevTime;
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct AnimationLayer {
		public AnimationState state0;
		public AnimationState state1;
		public float weight;
		public uint boneMaskHash;
		public float transitionDuration;
		public float transitionTimer;
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct BatchData {
		public uint compId;
		public AnimationLayer layer0;
		public AnimationLayer layer1;
		public AnimationLayer layer2;
		public AnimationLayer layer3;
	}
}
