#pragma once
#include "../CharacterBase.h"
#include <algorithm>

const int BULLET_MAX = 100; // ���@�����˂���e�̍ő吔
const int PLAYER_SHIELD_MAX = 8; // ���@�̃V�[���h�̍ő�l
const int WEAPON_LV_MAX = 10; // ���탌�x���̍ő�l
const int PLAYER_SPEED_MAX = 450; // ���@�̑����̍ő�l
const float SHOT_INTERVAL = 0.1f; // 0.25�b���ƂɌ���
const float BEAM_TIME = 5.0f; // 0.25�b���ƂɌ���
const float beam_duration = 5.0f; // 5�b��

enum
{
	LEVEL1 = 1,
	LEVEL2,
	LEVEL3
};

class Player : public CharacterBase
{
private:
	enum class PlayerAnimState
	{
		Neutral,   // ����
		TiltLeft,  // ���X��
		TiltRight  // �E�X��
	};

	PlayerAnimState anim_state = PlayerAnimState::Neutral;
	float anim_timer = 0.0f;
	int anim_index = 0;
	float anim_interval = 0.1f;

public:
	int life;
private:
	bool is_shot = false;
	float shot_timer = 0.0f;
	bool on_hit = false;
	bool beam_on = false;
	bool stop = false;
	bool is_damage = false;
	bool shot_flip = false;
	int reach_count = 0;
	int brend = 255;
	float beam_timer = 0.0f;
	float damage_timer = 0.0f;
	float shield_damage_timer = 0.0f;
	bool is_shield_damage = false;
	// �ǉ��F�V�[���h�̐؂�ւ�����1�t���[���X�L�b�v�p�t���O
	bool just_switched_to_second = false;
	int powerd = 2;
	bool powerd_on = false;
	float powerd_time = 0.0f;
	bool is_alive = true;
	bool is_shield = false;
	bool shield_secand = false;

	bool is_dead_animation_playing = false;
	float dead_animation_timer = 0.0f;    // ���o�Đ��p�^�C�}�[
	const float dead_animation_duration = 2.0f; // ��F2�b�Ԃ̎��̉��o

private:
	float charge = 0.0f;
	const float charge_max = 100.0f;
	bool charge_ready = false;

	float engen_time = 0.0f;
	int engen_count = 0;

	bool is_shot_anim = false;
	int se_shot = NULL;

	float shield_time = 0.0f;
	int shield_count = 0;

	bool shield_anim_on = false;

	bool shot_stop = false;

	// �摜�n
	std::vector<int> player_image_right;
	std::vector<int> player_image_left;
	std::vector<int> player_jet;
	std::vector<int> engens;
	std::vector<int> shields;
	std::vector<int> shields2;
	int engen = NULL;
	int jet = NULL;
	int shield = NULL;

	float last_explosion_time = -999.0f; // �Ō�ɔ������Đ���������
	float invincible_time = 0.0f;

public:
	Player();
	~Player();

public:
	// ����������
	void Initialize() override;

	/// <summary>
	/// �X�V����
	/// </summary>
	/// <param name="delata_second">�P�t���[��������̎���</param>
	void Update(float delta_second) override;

	/// <summary>
	/// �`�揈��
	/// </summary>
	/// <param name="screen_offset"></param>
	void Draw(const Vector2D& screen_offset)const override;

	// �I��������
	void Finalize() override;

public:
	/// <summary>
	/// �q�b�g������
	/// </summary>
	/// <param name="hit_object">������������</param>
	void OnHitCollision(GameObjectBase* hit_object) override;

private:
	/// <summary>
	/// �ړ�����
	/// </summary>
	/// <param name="delta_second">�P�t���[��������̎���</param>
	void Movement(float delta_second) override;

	void Shot(float delta_second);

	void Damage(float delta_second);

	void BuhinAnim(float delta_second);

	void GenarateBullet();

public:
	bool GetIsAlive() const;
	bool GetBeamOn() const;
	bool GetShotFlip() const;
	void SetBeamOn();

public:
	void AddCharge(float value);
	bool CanUseSpecial() const;
	void UseSpecial();
	float GetChargeRate() const;
	int GetPowerd() const;
	bool GetShieldOn() const;
	int GetLife() const;
	void SetShotStop(bool stoping);

public:
	void SetLocation(const Vector2D& pos) { location = pos; }
	void SetMobility(bool enable) { is_mobility = enable; }

	void ForceNeutralAnim(bool enable); // �����j���[�g�����p
private:
	bool force_neutral_anim = false;


};

