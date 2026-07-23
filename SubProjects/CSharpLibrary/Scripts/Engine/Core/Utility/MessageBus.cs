using System;
using System.Collections.Generic;

// メッセージ型をキーにした汎用 Pub/Sub バス。
// Publisher は購読者を知らずに Publish するだけ、Subscriber は型で購読するだけで疎結合になる。
// Publish は同期呼び出し（発行したフレーム内でハンドラが走る）なので、
// 「毎フレームリセットするフラグ」のような実行順依存は発生しない。
// 静的クラスなのでシーンを跨いで生存する。購読した MonoScript は必ず OnDestroy() で
// Unsubscribe すること（破棄済みインスタンスへの配信が残るため）。
public static class MessageBus {
    private static readonly Dictionary<Type, Delegate> handlers_ = new Dictionary<Type, Delegate>();

    public static void Subscribe<T>(Action<T> handler) {
        if (handler == null) return;

        Delegate existing;
        handlers_.TryGetValue(typeof(T), out existing);
        handlers_[typeof(T)] = Delegate.Combine(existing, handler);
    }

    public static void Unsubscribe<T>(Action<T> handler) {
        if (handler == null) return;

        Delegate existing;
        if (!handlers_.TryGetValue(typeof(T), out existing)) return;

        Delegate remaining = Delegate.Remove(existing, handler);
        if (remaining == null) {
            handlers_.Remove(typeof(T));
        } else {
            handlers_[typeof(T)] = remaining;
        }
    }

    public static void Publish<T>(T message) {
        Delegate existing;
        if (!handlers_.TryGetValue(typeof(T), out existing)) return;

        Action<T> handler = existing as Action<T>;
        if (handler != null) {
            try {
                handler(message);
            } catch (Exception e) {
                Debug.LogError("Error in MessageBus.Publish<" + typeof(T).Name + ">: " + e.Message);
            }
        }
    }

    // 新しいゲーム開始時などに全購読を破棄したい場合に使う。
    public static void Clear() {
        handlers_.Clear();
    }
}
