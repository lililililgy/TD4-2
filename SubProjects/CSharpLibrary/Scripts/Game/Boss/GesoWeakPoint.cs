using System;

///=============================================
/// ボスの弱点を表すクラス
///=============================================
public class GesoWeakPoint : MonoScript {

    // ボスのエンティティ名
    [SerializeField]
    private string kingGesoEntityName = "Boss";

    // ボスの参照
    private KingGeso _kingGeso;

    //=============================
    // 初期化
    //=============================
    public override void Initialize() {
        ResolveKingGeso();
	}

    //=============================
    // 更新
    //=============================
    public override void Update() {
		
	}

    //=============================
    // ダメージを与える
    //=============================
    public void Damage(int damage)
    {
        Damage((float)damage);
    }


    public void Damage(float damage)
    {
        if (_kingGeso == null)
        {
            // ボスの参照がない場合は再取得
            ResolveKingGeso();
        }

        if (_kingGeso != null)
        {
            // ボスにダメージを与える
            _kingGeso.TakeDamage(damage);
        }
    }

    //=============================
    // ボスの参照を解決する
    //=============================
    private void ResolveKingGeso()
    {
        _kingGeso = null;

        // ECSグループを取得
        if (!String.IsNullOrEmpty(kingGesoEntityName))
        {
            Entity kingGesoEntity = ecsGroup.FindEntity(kingGesoEntityName);
            //取得できなかった場合はnullを返す
            _kingGeso = kingGesoEntity != null ? kingGesoEntity.GetScript<KingGeso>() : null;
        }

        if (_kingGeso == null)
        {
            // 取得できなかった場合は、親のエンティティから取得を試みる
            _kingGeso = entity.GetScript<KingGeso>();
        }
    }
}
