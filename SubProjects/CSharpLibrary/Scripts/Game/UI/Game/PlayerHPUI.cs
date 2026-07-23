using System;
using System.Collections.Generic;

public class PlayerHPUI : MonoScript
{

	Entity playerEntity = null;

	int prevLives = -1;
	int currentLives = 5;
	[SerializeField] int startLives = 3;

	public override void Initialize()
	{
		prevLives = startLives;
		currentLives = startLives;

		InitialHPUI(startLives);


		ECSGroup gameScene = EntityComponentSystem.GetECSGroup("GameScene");
		if (gameScene == null)
		{
			return;
		}

		playerEntity = gameScene.FindEntity("Player");
	}

	public override void Update()
	{
		if (playerEntity == null)
		{
			ECSGroup gameScene = EntityComponentSystem.GetECSGroup("GameScene");
			if (gameScene != null)
			{
				playerEntity = gameScene.FindEntity("Player");
			}
		}

		if (playerEntity == null)
		{
			return;
		}

		PlayerLifeComponent playerLifeComponent = playerEntity.GetScript<PlayerLifeComponent>();
		if (playerLifeComponent)
		{
			int lives = playerLifeComponent.RemainingLives();
			UpdateLives(lives);
		}
	}


	/// <summary>
	/// hp iconの整列
	/// 縦6, 横2で配置
	/// </summary>
	void InitialHPUI(int startLives)
	{
		for (int y = 0; y < 2; y++)
		{
			for (int x = 0; x < 6; x++)
			{
				int index = y * 6 + x;
				Entity child = entity.GetChild((uint)index);
				if (child)
				{
					child.enable = true;
					child.transform.position = new Vector3(x * 1.5f, -y * 1.5f, 0);
					child.transform.scale = Vector3.one * 1.5f;

					SpriteAnimation spriteAnimation = child.GetScript<SpriteAnimation>();
					if (spriteAnimation)
					{
						spriteAnimation.startFrame = 4;
						spriteAnimation.endFrame = 4;

						if (index < startLives)
						{
							spriteAnimation.startFrame = 0;
							spriteAnimation.endFrame = 2;
						}
					}
				}
			}
		}

	}




	/// <summary>
	/// 残機の同期
	/// </summary>
	void UpdateLives(int lives)
	{

		prevLives = currentLives;
		currentLives = lives;
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
				for (int i = prevLives; i < currentLives; i++)
				{
					// 残機が増えたときのUI更新処理をここに追加
					Entity child = entity.GetChild((uint)i);
					if (child)
					{
						SpriteAnimation spriteAnimation = child.GetScript<SpriteAnimation>();
						if (spriteAnimation)
						{
							/// 幼生として発射できるなら。
							spriteAnimation.startFrame = 0;
							spriteAnimation.endFrame = 2;
						}
					}
				}
			}
			else if (diff < 0)
			{
				// 残機が減った場合の処理
				for (int i = currentLives; i < prevLives; i++)
				{
					// 残機が減ったときのUI更新処理をここに追加
					Entity child = entity.GetChild((uint)i);
					if (child)
					{
						SpriteAnimation spriteAnimation = child.GetScript<SpriteAnimation>();
						if (spriteAnimation)
						{
							spriteAnimation.startFrame = 4;
							spriteAnimation.endFrame = 4;
						}
					}
				}
			}
		}
	}
}
