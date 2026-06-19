using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

/// <summary>
/// AIのデータをツリー全体で共有するための記憶領域（ストレージ）。
/// ボクシング（GCアロケーション）を防ぐため、値型ごとに個別の Dictionary を持つ。
/// また、文字列キーの代わりにハッシュ値 (uint) を使用して高速なアクセスを実現している。
/// 値が変更された際にツリーに通知を行う Observer パターンをサポートし、
/// エディタ側の Live Watcher 用にC++への通知も行う。
/// </summary>
public class Blackboard
{
    /// <summary>
    /// Blackboard内のいずれかの変数が変更された時に発火するイベント。
    /// BehaviorTree がこれを購読し、Observer Aborts（割り込み）の判定を行う。
    /// </summary>
    public event Action<uint> OnValueChanged;

    // --- 型ごとのデータストレージ ---
    private readonly Dictionary<uint, int> _intData = new Dictionary<uint, int>();
    private readonly Dictionary<uint, float> _floatData = new Dictionary<uint, float>();
    private readonly Dictionary<uint, bool> _boolData = new Dictionary<uint, bool>();
    private readonly Dictionary<uint, Vector3> _vector3Data = new Dictionary<uint, Vector3>();
    private readonly Dictionary<uint, string> _stringData = new Dictionary<uint, string>();
    private readonly Dictionary<uint, object> _objectData = new Dictionary<uint, object>();

    // ロード時の初期値を保存するストレージ
    private readonly Dictionary<uint, object> _defaultValues = new Dictionary<uint, object>();

    /// <summary>
    /// 現在の値を「デフォルト値」として保存する。
    /// ResetToDefaults() を実行した際に、この値に復元される。
    /// </summary>
    public void SaveAsDefault(uint key)
    {
        object val = GetValueAsObject(key);
        if (val != null)
        {
            _defaultValues[key] = val;
        }
    }

    /// <summary>
    /// ブラックボードをロード時の初期状態にリセットする。
    /// 実行中に生成された一時的なデータは削除され、初期設定値は復元される。
    /// </summary>
    public void ResetToDefaults()
    {
        // 1. 全てを一度クリア
        _intData.Clear();
        _floatData.Clear();
        _boolData.Clear();
        _vector3Data.Clear();
        _stringData.Clear();
        _objectData.Clear();

        // 2. デフォルト値を復元
        foreach (var kvp in _defaultValues)
        {
            uint key = kvp.Key;
            object val = kvp.Value;

            if (val is int i) _intData[key] = i;
            else if (val is float f) _floatData[key] = f;
            else if (val is bool b) _boolData[key] = b;
            else if (val is Vector3 v) _vector3Data[key] = v;
            else if (val is string s) _stringData[key] = s;
            else _objectData[key] = val;
            
            // エディタへの通知などは省略（初期化時のみのため）
        }
    }

    public void SetInt(uint key, int value) 
    { 
        if (_intData.TryGetValue(key, out var old) && old == value) return;
        Remove(key);
        _intData[key] = value; 
        OnValueChanged?.Invoke(key);
        Internal_UpdateBlackboardValue(key, value.ToString(), "Int");
    }
    
    public void SetFloat(uint key, float value) 
    { 
        if (_floatData.TryGetValue(key, out var old) && Math.Abs(old - value) < 0.001f) return;
        Remove(key);
        _floatData[key] = value; 
        OnValueChanged?.Invoke(key);
        Internal_UpdateBlackboardValue(key, value.ToString(), "Float");
    }

    public void SetBool(uint key, bool value) 
    { 
        if (_boolData.TryGetValue(key, out var old) && old == value) return;
        Remove(key);
        _boolData[key] = value; 
        OnValueChanged?.Invoke(key);
        Internal_UpdateBlackboardValue(key, value ? "true" : "false", "Bool");
    }

    public void SetVector3(uint key, Vector3 value) 
    { 
        if (_vector3Data.TryGetValue(key, out var old) && Vector3.Distance(old, value) < 0.01f) return;
        Remove(key);
        _vector3Data[key] = value; 
        OnValueChanged?.Invoke(key);
        Internal_UpdateBlackboardValue(key, value.x + "," + value.y + "," + value.z, "Vector3");
    }

    public void SetString(uint key, string value) 
    { 
        if (_stringData.TryGetValue(key, out var old) && old == value) return;
        Remove(key);
        _stringData[key] = value; 
        OnValueChanged?.Invoke(key);
        Internal_UpdateBlackboardValue(key, value, "String");
    }

    public void SetObject(uint key, object value) 
    { 
        if (_objectData.TryGetValue(key, out var old) && old == value) return;
        Remove(key);
        _objectData[key] = value; 
        OnValueChanged?.Invoke(key);
        Internal_UpdateBlackboardValue(key, value != null ? value.ToString() : "null", "Object");
    }

    /// <summary>
    /// Int型の値を取得する。キーが存在しない場合はデフォルト値を返す。
    /// </summary>
    public int GetInt(uint key, int defaultValue = 0) => _intData.TryGetValue(key, out var val) ? val : defaultValue;

    /// <summary>
    /// Float型の値を取得する。キーが存在しない場合はデフォルト値を返す。
    /// </summary>
    public float GetFloat(uint key, float defaultValue = 0f) => _floatData.TryGetValue(key, out var val) ? val : defaultValue;

    /// <summary>
    /// Bool型の値を取得する。キーが存在しない場合はデフォルト値を返す。
    /// </summary>
    public bool GetBool(uint key, bool defaultValue = false) => _boolData.TryGetValue(key, out var val) ? val : defaultValue;

    /// <summary>
    /// Vector3型の値を取得する。キーが存在しない場合は Vector3.zero を返す。
    /// </summary>
    public Vector3 GetVector3(uint key) => _vector3Data.TryGetValue(key, out var val) ? val : Vector3.zero;

    /// <summary>
    /// String型の値を取得する。キーが存在しない場合はデフォルト値を返す。
    /// </summary>
    public string GetString(uint key, string defaultValue = "") => _stringData.TryGetValue(key, out var val) ? val : defaultValue;

    /// <summary>
    /// Entity型の値を取得する。キーが存在しない場合は null を返す。
    /// </summary>
    public Entity GetEntity(uint key) => GetObject<Entity>(key);

    /// <summary>
    /// 任意のオブジェクト型の値を取得し、指定した型にキャストして返す。失敗した場合はnullを返す。
    /// </summary>
    public T GetObject<T>(uint key) where T : class
    {
        if (_objectData.TryGetValue(key, out var val) && val is T typedVal) return typedVal;
        return null;
    }

    /// <summary>
    /// 指定したキーの値をobject型として（ボクシングして）取得する。
    /// デコレーター等で型が事前定義されていない変数の中身を検証・判定する際に使用する。
    /// </summary>
    /// <param name="key">取得するキーのハッシュ値</param>
    /// <returns>キーが存在すればその値のobject、存在しなければnull</returns>
    public object GetValueAsObject(uint key)
    {
        if (_objectData.TryGetValue(key, out var o)) return o;
        if (_intData.TryGetValue(key, out var i)) return i;
        if (_floatData.TryGetValue(key, out var f)) return f;
        if (_boolData.TryGetValue(key, out var b)) return b;
        if (_vector3Data.TryGetValue(key, out var v)) return v;
        if (_stringData.TryGetValue(key, out var s)) return s;
        return null;
    }

    /// <summary>
    /// 指定したキーがいずれかの型のストレージに存在するかどうかを確認する。
    /// </summary>
    public bool HasKey(uint key)
    {
        return _intData.ContainsKey(key) || _floatData.ContainsKey(key) || 
               _boolData.ContainsKey(key) || _vector3Data.ContainsKey(key) || 
               _stringData.ContainsKey(key) || _objectData.ContainsKey(key);
    }

    /// <summary>
    /// 指定したキーのデータをすべての型のストレージから削除する。
    /// </summary>
    public void Remove(uint key)
    {
        _intData.Remove(key);
        _floatData.Remove(key);
        _boolData.Remove(key);
        _vector3Data.Remove(key);
        _stringData.Remove(key);
        _objectData.Remove(key);
    }

    /// <summary>
    /// Blackboard内のすべてのデータをクリアする。
    /// </summary>
    public void Clear()
    {
        _intData.Clear();
        _floatData.Clear();
        _boolData.Clear();
        _vector3Data.Clear();
        _stringData.Clear();
        _objectData.Clear();
    }

    /// <summary>
    /// C++エディタ側の「Live Watcher (Runtimeタブ)」に値を送信するための内部呼び出し。
    /// </summary>
    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_UpdateBlackboardValue(uint keyHash, string valueStr, string typeName);
}
