#pragma once
#include "../Singleton.h"
#include "../Scene/SceneType/Stage/StageBase.h"

class SelectStage :
    public Singleton<SelectStage>
{
private:
    // 選択ステージ保存用
    StageID stage_id = StageID::Stage1;

public:
    SelectStage(){}
    ~SelectStage(){}

public:
    // 選択したステージを設定
    void SetSelectStage(StageID id)
    {
        stage_id = id;
    }
    // 選択したステージを取得
    StageID GetSelectStage()
    {
        return stage_id;
    }
};