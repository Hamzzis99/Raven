#ifndef GOAL_ATTACKTARGET_H
#define GOAL_ATTACKTARGET_H
#pragma warning (disable:4786)

#include "Goals/Goal_Composite.h"
#include "Raven_Goal_Types.h"
#include "../Raven_Bot.h"

class Goal_AttackTarget : public Goal_Composite<Raven_Bot>
{
private:
    // [추가] 다음 이동 방향 기억 변수 (true: 오른쪽, false: 왼쪽)
    bool m_bStrafeRight;

public:

    Goal_AttackTarget(Raven_Bot* pOwner)
        : Goal_Composite<Raven_Bot>(pOwner, goal_attack_target),
        m_bStrafeRight(true) // [추가] 처음엔 오른쪽부터 시작
    {
    }

    void Activate();
    int  Process();
    void Terminate() { m_iStatus = completed; }
};

#endif