//Raven_TargetingSystem.cpp
#include "Raven_TargetingSystem.h"
#include "Raven_Bot.h"
#include "Raven_SensoryMemory.h"



//-------------------------------- ctor ---------------------------------------
//-----------------------------------------------------------------------------
Raven_TargetingSystem::Raven_TargetingSystem(Raven_Bot* owner):m_pOwner(owner),
                                                               m_pCurrentTarget(0)
{}



// [Raven_TargetingSystem.cpp] Update 함수 전체 교체

void Raven_TargetingSystem::Update()
{
    double BestScore = -100000.0; // 아주 낮은 점수로 초기화
    m_pCurrentTarget = 0;

    // 감지된 적 리스트 가져오기
    std::list<Raven_Bot*> SensedBots;
    SensedBots = m_pOwner->GetSensoryMem()->GetListOfRecentlySensedOpponents();

    std::list<Raven_Bot*>::const_iterator curBot = SensedBots.begin();
    for (curBot; curBot != SensedBots.end(); ++curBot)
    {
        // 살아있고 자기 자신이 아닌 경우만 계산
        if ((*curBot)->isAlive() && (*curBot != m_pOwner))
        {
            double dist = Vec2DDistance((*curBot)->Pos(), m_pOwner->Pos());

            // [과제 구현] SensoryMemory에서 이 적이 준 피해량 가져오기
            double damage = m_pOwner->GetSensoryMem()->GetDamageCausedByOpponent(*curBot);

            // [과제 구현] 점수(Desirability) 계산
            // 공식: (피해량 * 가중치) - 거리
            // 가중치(Weight) 50.0의 의미: 데미지 1을 입힌 놈은 거리 50만큼 더 멀리 있어도 쫓아감.
            const double DamageWeight = 50.0;

            double CurrentScore = (damage * DamageWeight) - dist;

            // 가장 높은 점수를 가진 봇을 타겟으로 선정
            if (CurrentScore > BestScore)
            {
                BestScore = CurrentScore;
                m_pCurrentTarget = *curBot;
            }
        }
    }
}




bool Raven_TargetingSystem::isTargetWithinFOV()const
{
  return m_pOwner->GetSensoryMem()->isOpponentWithinFOV(m_pCurrentTarget);
}

bool Raven_TargetingSystem::isTargetShootable()const
{
  return m_pOwner->GetSensoryMem()->isOpponentShootable(m_pCurrentTarget);
}

Vector2D Raven_TargetingSystem::GetLastRecordedPosition()const
{
  return m_pOwner->GetSensoryMem()->GetLastRecordedPositionOfOpponent(m_pCurrentTarget);
}

double Raven_TargetingSystem::GetTimeTargetHasBeenVisible()const
{
  return m_pOwner->GetSensoryMem()->GetTimeOpponentHasBeenVisible(m_pCurrentTarget);
}

double Raven_TargetingSystem::GetTimeTargetHasBeenOutOfView()const
{
  return m_pOwner->GetSensoryMem()->GetTimeOpponentHasBeenOutOfView(m_pCurrentTarget);
}
