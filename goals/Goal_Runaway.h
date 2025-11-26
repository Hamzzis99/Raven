#ifndef GOAL_RUN_AWAY_H
#define GOAL_RUN_AWAY_H

#pragma warning (disable:4786)

#include "Goals/Goal_Composite.h"
#include "Goal_Evaluator.h"

#include "../Raven_Bot.h"

class Goal_RunAway : public Goal_Composite<Raven_Bot>
{
private:
    Vector2D m_vCurrentTarget; // 도망갈 위치

public:
    Goal_RunAway(Raven_Bot* pBot);

    void Activate();
    int  Process();
    void Terminate();

    // 메시지 처리 (길찾기 완료 신호 받기용)
    bool HandleMessage(const Telegram& msg);

    // 디버깅 렌더링 (빨간 선)
    void Render();
};

class RunAway_Evaluator : public Goal_Evaluator
{
public:
    RunAway_Evaluator(double bias) : Goal_Evaluator(bias) {}

    double CalculateDesirability(Raven_Bot* pBot);
    void   SetGoal(Raven_Bot* pBot);
    void   RenderInfo(Vector2D Position, Raven_Bot* pBot);
};

#endif