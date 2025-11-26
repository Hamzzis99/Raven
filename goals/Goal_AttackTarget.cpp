#include "Goal_AttackTarget.h"
#include "Goal_SeekToPosition.h"
#include "Goal_HuntTarget.h"
#include "../Raven_Bot.h"
#include "misc/utils.h" // RandFloat() 사용을 위해 필요



//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------
void Goal_AttackTarget::Activate()
{
    m_iStatus = active;

    // 기존 서브골(하위 목표) 초기화
    RemoveAllSubgoals();

    // 1. 타겟이 존재하는지 확인 (없으면 종료)
    if (!m_pOwner->GetTargetSys()->isTargetPresent())
    {
        m_iStatus = completed;
        return;
    }

    // 2. 적을 쏠 수 있는 상황(전투 거리/시야 확보)이라면?
    if (m_pOwner->GetTargetSys()->isTargetShootable())
    {
        // [중요] canStep 함수들에 넘겨줄 좌표 변수 (이게 없어서 에러가 났던 것임)
        Vector2D strafePos;
        bool bCanStrafe = false; // 이동할 곳을 찾았는지 체크하는 플래그

        // 50% 확률로 "오른쪽 먼저 체크" vs "왼쪽 먼저 체크"
        // (이렇게 하면 예측 불가능하게 좌우로 왔다 갔다 합니다)
        if (RandFloat() < 0.5)
        {
            // [Case A] 오른쪽을 먼저 시도!
            // 인자로 strafePos를 꼭 넣어줘야 에러가 안 납니다!
            if (m_pOwner->canStepRight(strafePos))
            {
                bCanStrafe = true;
            }
            else if (m_pOwner->canStepLeft(strafePos)) // 오른쪽 막혔으면 왼쪽 시도
            {
                bCanStrafe = true;
            }
        }
        else
        {
            // [Case B] 왼쪽을 먼저 시도!
            if (m_pOwner->canStepLeft(strafePos))
            {
                bCanStrafe = true;
            }
            else if (m_pOwner->canStepRight(strafePos)) // 왼쪽 막혔으면 오른쪽 시도
            {
                bCanStrafe = true;
            }
        }

        // 3. 무빙할 곳을 찾았으면? -> 그 좌표로 '직선 이동(Seek)'
        if (bCanStrafe)
        {
            // strafePos에는 이미 봇의 한 발자국 옆 좌표가 계산되어 들어있음
            AddSubgoal(new Goal_SeekToPosition(m_pOwner, strafePos));
        }
        else
        {
            // 4. 좌우가 다 막혀있으면 그냥 적을 향해 전진
            AddSubgoal(new Goal_SeekToPosition(m_pOwner, m_pOwner->GetTargetBot()->Pos()));
        }
    }

    // 5. 적이 시야에 없으면 추격(Hunt)
    else
    {
        AddSubgoal(new Goal_HuntTarget(m_pOwner));
    }
}

//-------------------------- Process ------------------------------------------
//-----------------------------------------------------------------------------
int Goal_AttackTarget::Process()
{
    // 활성화 체크
    ActivateIfInactive();

    // 서브골 처리
    m_iStatus = ProcessSubgoals();

    // 서브골(이동)이 완료되거나 실패하면
    // Reactivate를 호출하여 다시 Activate()가 실행되게 함.
    // -> 결과적으로 [이동 -> 멈춤 -> 다시 랜덤 방향 결정 -> 이동]을 무한 반복
    ReactivateIfFailed();

    return m_iStatus;
}