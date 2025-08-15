#pragma once
#include "../ItemBase.h"
#include "../../../Utility/ProjectConfig.h"

class Exp2 : public ItemBase
{
private:
    float lifetime;        //5�b�Ŏ����폜

public:
    Exp2();
    ~Exp2();

    /// <summary>
    /// ����������
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// �X�V����
    /// </summary>
    /// <param name="delta"></param>
    void Update(float delta) override;

    /// <summary>
    /// �`�揈��
    /// </summary>
    /// <param name="offset"></param>
    void Draw(const Vector2D& offset) const override;

    /// <summary>
    /// �����蔻�菈��
    /// </summary>
    /// <param name="�q�b�g�I�u�W�F�N�g"></param>
    void OnHitCollision(GameObjectBase* hit_object) override;

};
