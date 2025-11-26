#include "Goal_AttackTarget.h"
#include "Goal_SeekToPosition.h"
#include "Goal_HuntTarget.h"
#include "../Raven_Bot.h"
#include "misc/utils.h" // RandFloat() 사용을 위해 필요

// 인클루드는 놔둠 (사용은 안 함)
#include "Goal_DodgeSideToSide.h" 


//------------------------------- Activate ------------------------------------
void Goal_AttackTarget::Activate()
{
    m_iStatus = active;
    RemoveAllSubgoals();

    // 1. 타겟이 없으면 종료
    if (!m_pOwner->GetTargetSys()->isTargetPresent())
    {
        m_iStatus = completed;
        return;
    }

    // 2. 적을 쏠 수 있는 상황(전투 모드)인가?
    if (m_pOwner->GetTargetSys()->isTargetShootable())
    {
        // [변수 선언]
        Vector2D strafePos;      // 이동할 좌표를 받아올 변수
        bool bCanStrafe = false; // "이동할 곳 찾았니?" (처음엔 못 찾음=false)

        // =========================================================
        // [랜덤 무빙 로직] 상태 저장 없이 매번 주사위를 굴립니다.
        // =========================================================

        // 3. 50% 확률로 "오른쪽 우선" vs "왼쪽 우선" 결정
        if (RandFloat() < 0.5)
        {
            // [Case A] 오른쪽을 먼저 시도!
            if (m_pOwner->canStepRight(strafePos))
            {
                bCanStrafe = true; // 찾았다!
            }
            else if (m_pOwner->canStepLeft(strafePos)) // 막혔으면 왼쪽 시도
            {
                bCanStrafe = true; // 찾았다!
            }
        }
        else
        {
            // [Case B] 왼쪽을 먼저 시도!
            if (m_pOwner->canStepLeft(strafePos))
            {
                bCanStrafe = true; // 찾았다!
            }
            else if (m_pOwner->canStepRight(strafePos)) // 막혔으면 오른쪽 시도
            {
                bCanStrafe = true; // 찾았다!
            }
        }

        // 4. 결정: 무빙할 곳을 찾았으면 그쪽으로, 못 찾았으면 적에게 돌격
        if (bCanStrafe)
        {
            // strafePos에는 canStep 함수가 찾아낸 "옆자리 좌표"가 들어있음
            AddSubgoal(new Goal_SeekToPosition(m_pOwner, strafePos));
        }
        else
        {
            // 공간이 꽉 막혔으면 그냥 적을 향해 전진
            AddSubgoal(new Goal_SeekToPosition(m_pOwner, m_pOwner->GetTargetBot()->Pos()));
        }
    }
    // 5. 적이 안 보이면 추격(Hunt)
    else
    {
        AddSubgoal(new Goal_HuntTarget(m_pOwner));
    }
}

//-------------------------- Process ------------------------------------------
int Goal_AttackTarget::Process()
{
    ActivateIfInactive();

    m_iStatus = ProcessSubgoals();

    // 서브골(이동)이 완료되면 다시 Activate를 호출 -> 다시 랜덤 방향 결정
    ReactivateIfFailed();

    return m_iStatus;
}