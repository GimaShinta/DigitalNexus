#pragma once
#include "../CharacterBase.h"
#include "../Player//Player.h"

class EnemyBase :
    public CharacterBase
{
protected:
	float hp;
	bool on_hit;

	class Player* player;
	int anim_id;
	float delta = 0.0f;
	float beam_damage_timer = 0.0f;

public:
	EnemyBase();
	~EnemyBase();

public:
	virtual void Initialize() override;
	virtual void Update(float delta_second) override;
	virtual void Draw(const Vector2D& screen_offset) const override;
	virtual void Finalize() override;
	virtual void OnHitCollision(GameObjectBase* hit_object) override;
	void SetPlayer(Player* p);

protected:
	void DropItems();  // Å© í«â¡

};

