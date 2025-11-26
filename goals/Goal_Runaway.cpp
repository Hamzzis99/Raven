#include "Goal_RunAway.h"
#include "Goal_Think.h"           
#include "Raven_Goal_Types.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "Goal_MoveToPosition.h"
#include "Raven_Goal_Types.h"
#include "misc/Cgdi.h"
#include "misc/Stream_Utility_Functions.h"

Goal_RunAway::Goal_RunAway(Raven_Bot* pBot)
    : Goal_Composite<Raven_Bot>(pBot, goal_run_away)
{
}

void Goal_RunAway::Activate()
{
    m_iStatus = active;
    RemoveAllSubgoals();

    // 1. 현재 나를 노리는 적(Target) 확인
    Raven_Bot* pTarget = m_pOwner->GetTargetBot();
    Vector2D HidingSpot;
    bool bFoundSpot = false;

    // 2. 적이 있다면 "적이 못 보는 곳"을 찾는다 (Raycast 검사)
    if (pTarget)
    {
        const std::vector<Vector2D>& SpawnPoints = m_pOwner->GetWorld()->GetMap()->GetSpawnPoints();

        // 20번 정도 랜덤하게 찍어서 숨을 곳을 찾아봄
        for (int i = 0; i < 20; ++i)
        {
            int idx = RandInt(0, SpawnPoints.size() - 1);
            Vector2D Candidate = SpawnPoints[idx];

            // isLOSOkay가 false면 시야가 벽에 막혔다는 뜻 -> 숨기 좋은 곳!
            if (m_pOwner->GetWorld()->isLOSOkay(pTarget->Pos(), Candidate) == false)
            {
                HidingSpot = Candidate;
                bFoundSpot = true;
                break; // 찾았으면 루프 종료
            }
        }
    }

    // 3. 적이 없거나 마땅히 숨을 곳 못 찾았으면 랜덤 위치로 도주
    if (!bFoundSpot)
    {
        HidingSpot = m_pOwner->GetWorld()->GetMap()->GetRandomSpawnPoint();
    }

    // 4. 너무 가까우면(제자리) 살짝 옆으로 이동 (버그 방지)
    if (m_pOwner->Pos().DistanceSq(HidingSpot) < 100.0)
    {
        HidingSpot = m_pOwner->Pos() + Vector2D(50, 50);
    }

    m_vCurrentTarget = HidingSpot;

    // 5. 이동 명령 내리기
    AddSubgoal(new Goal_MoveToPosition(m_pOwner, m_vCurrentTarget));
}

int Goal_RunAway::Process()
{
    ActivateIfInactive();
    m_iStatus = ProcessSubgoals();

    // 이동 실패해도 완료 처리해서 다시 생각하게 만듦 (무한 시도)
    if (m_iStatus == failed) m_iStatus = completed;

    return m_iStatus;
}

void Goal_RunAway::Terminate()
{
    m_iStatus = completed;
}

bool Goal_RunAway::HandleMessage(const Telegram& msg)
{
    // [중요] 하위 목표(이동)에게 "길 찾았어!" 메시지 전달
    // 이게 없으면 봇이 멍때림
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
        gdi->Line(m_pOwner->Pos(), m_vCurrentTarget); // 빨간 선 표시
        gdi->Circle(m_vCurrentTarget, 5);
    }
    Goal_Composite<Raven_Bot>::Render();
}

// ============================================================================
//  RunAway_Evaluator 구현 (판단 로직)
// ============================================================================

double RunAway_Evaluator::CalculateDesirability(Raven_Bot* pBot)
{
    // [과제 조건] 체력이 30 이하면 무조건 발동
    if (pBot->Health() <= 30)
    {
        // 100점 만점에 가까운 점수를 줘서 다른 행동을 압도함
        return m_dCharacterBias;
    }
    return 0.0;
}

void RunAway_Evaluator::SetGoal(Raven_Bot* pBot)
{
    // Goal_Think의 함수 호출
    pBot->GetBrain()->AddGoal_RunAway();
}

void RunAway_Evaluator::RenderInfo(Vector2D Position, Raven_Bot* pBot)
{
    // 봇 머리 위에 점수 띄우기 (디버깅용)
    gdi->TextAtPos(Position, "RUN: " + ttos(CalculateDesirability(pBot), 2));
}