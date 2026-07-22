using System;
using System.Collections.Generic;

public class PlayerHPUI : MonoScript
{

	Entity playerEntity = null;

	int prevLives = 0;
	int currentLives = 0;
	[SerializeField] int maxLives = 12;

	[SerializeField] int testValue = 20;

	public override void Initialize()
	{

		InitialHPUI();


		ECSGroup gameScene = EntityComponentSystem.GetECSGroup("GameScene");
		if (gameScene == null)
		{
			return;
		}

		playerEntity = gameScene.FindEntity("Player");
		if (playerEntity)
		{
			HP hp = playerEntity.GetScript<HP>();
			if (hp)
			{
				maxLives = (int)hp.MaxHp;
			}
		}


	}

	public override void Update()
	{
		UpdateLives();
	}


	/// <summary>
	/// hp iconの整列
	/// 縦6, 横2で配置
	/// </summary>
	void InitialHPUI()
	{
		for (int x = 0; x < 6; x++)
		{
			for (int y = 0; y < 2; y++)
			{
				int index = x * 2 + y;
				Entity child = entity.GetChild((uint)index);
				if (child)
				{
					child.enable = true;
					child.transform.position = new Vector3(x * 1.5f, -y * 1.5f, 0);
				}
			}
		}

	}




	/// <summary>
	/// 残機の同期
	/// </summary>
	void UpdateLives()
	{

		if (!playerEntity)
		{
			return;
		}

		PlayerLifeComponent life = playerEntity.GetScript<PlayerLifeComponent>();
		if (life)
		{
			int lives = life.RemainingLives();
			prevLives = currentLives;
			currentLives = lives;
		}

		UpdateUI();
	}


	void UpdateUI()
	{
		if (prevLives != currentLives)
		{
			int diff = currentLives - prevLives;
			if (diff > 0)
			{
				// 残機が増えた場合の処理
				for (int i = currentLives; i < currentLives + diff; i++)
				{
					// 残機が増えたときのUI更新処理をここに追加
					Entity child = entity.GetChild((uint)i);
					if (child)
					{
						child.enable = true;
					}
				}
			}
			else if (diff < 0)
			{
				// 残機が減った場合の処理
				for (int i = prevLives - 1; i >= currentLives; i--)
				{
					// 残機が減ったときのUI更新処理をここに追加
					Entity child = entity.GetChild((uint)i);
					if (child)
					{
						child.enable = false;
					}
				}
			}
		}
	}
}
