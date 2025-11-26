#include "Goal_RunAway.h"
#include "Goal_Think.h"            
#include "Raven_Goal_Types.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "Goal_MoveToPosition.h"
#include "misc/Cgdi.h"
#include "misc/Stream_Utility_Functions.h"
#include "Triggers/Trigger.h" // [중요] 트리거 확인을 위해 추가

// ============================================================================
//  Goal_RunAway 구현
// ============================================================================

Goal_RunAway::Goal_RunAway(Raven_Bot* pBot)
    : Goal_Composite<Raven_Bot>(pBot, goal_run_away)
{
}

void Goal_RunAway::Activate()
{
    m_iStatus = active;
    RemoveAllSubgoals();

    // [핵심] 도망 시작! -> "총 쏘지 마!" (스위치 ON)
    m_pOwner->SetHoldFire(true);

    // 1. 적(Target) 확인
    Raven_Bot* pTarget = m_pOwner->GetTargetBot();
    Vector2D HidingSpot;
    bool bFoundSpot = false;

    // 2. 적이 있다면 "적이 못 보는 곳"을 찾는다
    if (pTarget)
    {
        const std::vector<Vector2D>& SpawnPoints = m_pOwner->GetWorld()->GetMap()->GetSpawnPoints();

        for (int i = 0; i < 20; ++i)
        {
            int idx = RandInt(0, SpawnPoints.size() - 1);
            Vector2D Candidate = SpawnPoints[idx];

            if (m_pOwner->GetWorld()->isLOSOkay(pTarget->Pos(), Candidate) == false)
            {
                HidingSpot = Candidate;
                bFoundSpot = true;
                break;
            }
        }
    }

    // 3. 못 찾았으면 랜덤
    if (!bFoundSpot)
    {
        HidingSpot = m_pOwner->GetWorld()->GetMap()->GetRandomSpawnPoint();
    }

    // 4. 제자리 방지
    if (m_pOwner->Pos().DistanceSq(HidingSpot) < 100.0)
    {
        HidingSpot = m_pOwner->Pos() + Vector2D(50, 50);
    }

    m_vCurrentTarget = HidingSpot;

    // 5. 이동 명령
    AddSubgoal(new Goal_MoveToPosition(m_pOwner, m_vCurrentTarget));
}

int Goal_RunAway::Process()
{
    ActivateIfInactive();
    m_iStatus = ProcessSubgoals();

    if (m_iStatus == failed) m_iStatus = completed;

    return m_iStatus;
}

void Goal_RunAway::Terminate()
{
    // [핵심] 도망 끝! -> "이제 쏴도 돼" (스위치 OFF)
    m_pOwner->SetHoldFire(false);
    m_iStatus = completed;
}

bool Goal_RunAway::HandleMessage(const Telegram& msg)
{
    if (!m_SubGoals.empty())
    {
        return m_SubGoals.front()->HandleMessage(msg);
    }
    return false;
}

void Goal_RunAway::Render()
{
    if (m_iStatus == active)
    {
        gdi->RedPen();
        gdi->Line(m_pOwner->Pos(), m_vCurrentTarget);
        gdi->Circle(m_vCurrentTarget, 5);
    }
    Goal_Composite<Raven_Bot>::Render();
}

// ============================================================================
//  RunAway_Evaluator 구현
// ============================================================================

double RunAway_Evaluator::CalculateDesirability(Raven_Bot* pBot)
{
    // 1. 체력이 30 이하인가?
    if (pBot->Health() <= 30)
    {
        // 2. [중요] 맵에 활성화된 체력 키트가 있는지 확인한다!
        // 있다면 도망가지 말고 체력을 먹으러 가게 유도하기 위해 점수를 0으로 반환한다.

        const Raven_Map::TriggerSystem::TriggerList& triggers = pBot->GetWorld()->GetMap()->GetTriggers();

        for (auto it = triggers.begin(); it != triggers.end(); ++it)
        {
            // 타입이 '체력'이고, 현재 '활성화(먹을 수 있음)' 상태라면?
            if ((*it)->EntityType() == type_health && (*it)->isActive())
            {
                return 0.0; // "나(도망)는 빠질게, GetHealthGoal 네가 처리해!"
            }
        }

        // 3. 체력 키트가 하나도 없다면? -> 그때 비로소 도망 전략 발동!
        return m_dCharacterBias; // 100점 반환
    }

    return 0.0;
}

void RunAway_Evaluator::SetGoal(Raven_Bot* pBot)
{
    pBot->GetBrain()->AddGoal_RunAway();
}

void RunAway_Evaluator::RenderInfo(Vector2D Position, Raven_Bot* pBot)
{
    gdi->TextAtPos(Position, "RUN: " + ttos(CalculateDesirability(pBot), 2));
}