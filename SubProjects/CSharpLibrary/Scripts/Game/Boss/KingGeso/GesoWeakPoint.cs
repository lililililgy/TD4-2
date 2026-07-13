using System;

///=============================================
/// ゲソの弱点クラス
///=============================================
public class GesoWeakPoint : MonoScript {

    // ゲソのHP
    [SerializeField]
    private string kingGesoEntityName = "KingGeso";

    // ボスゲソの参照
    private KingGeso kingGeso;

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
        if (kingGeso == null)
        {
            // ボスの参照がない場合は再取得
            ResolveKingGeso();
        }

        if (kingGeso != null)
        {
            // ボスにダメージを与える
            kingGeso.TakeDamage(damage);
        }
    }

    //=============================
    // ボスの参照を解決する
    //=============================
    private void ResolveKingGeso()
    {
        kingGeso = null;

        // ECSGroupからボスのEntityを取得
        if (!String.IsNullOrEmpty(kingGesoEntityName))
        {
            Entity kingGesoEntity = ecsGroup.FindEntity(kingGesoEntityName);
            // ボスのスクリプトを取得
            kingGeso = kingGesoEntity != null ? kingGesoEntity.GetScript<KingGeso>() : null;
        }

        if (kingGeso == null)
        {
            // ボスのEntityが見つからない場合は、親Entityからボスのスクリプトを取得
            kingGeso = entity.GetScript<KingGeso>();
        }
    }
}
