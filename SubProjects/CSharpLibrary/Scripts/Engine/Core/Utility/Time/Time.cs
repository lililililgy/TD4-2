using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.CompilerServices;

static public class Time
{

	static public float time
	{
		get
		{
			return InternalGetTime();
		}
	}

	static public float deltaTime
	{
		get
		{
			return InternalGetDeltaTime();
		}
	}

	static public float unscaledTime
	{
		get
		{
			return InternalGetUnscaledDeltaTime();
		}
	}

	// timeScale の影響を受けない実フレーム差分(秒)。ヒットストップ等で減速中でも実時間で進む。
	// ※現状 unscaledTime も同じ内部呼び出しを指すが、「差分」であることを名前で明示するための別名。
	static public float unscaledDeltaTime
	{
		get
		{
			return InternalGetUnscaledDeltaTime();
		}
	}

	static public float timeScale
	{
		get
		{
			return InternalGetTimeScale();
		}
		set
		{
			InternalSetTimeScale(value);
		}
	}


	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern float InternalGetTime();

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern float InternalGetDeltaTime();

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern float InternalGetUnscaledDeltaTime();

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern float InternalGetTimeScale();

	[MethodImpl(MethodImplOptions.InternalCall)]
	static extern void InternalSetTimeScale(float scale);


}
