//Raven_SensoryMemory.h

#ifndef RAVEN_SENSORY_SYSTEM_H
#define RAVEN_SENSORY_SYSTEM_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:
//
//  Author: Mat Buckland (ai-junkie.com)
//
//  Desc:
//
//-----------------------------------------------------------------------------
#include <map>
#include <list>
#include "2d/vector2d.h"

class Raven_Bot;


// 보기 좋게 수정하자. 복잡해 영어설명들.
class MemoryRecord
{
public:
    double fTimeLastSensed;
    double fTimeBecameVisible;
    double fTimeLastVisible;
    Vector2D vLastSensedPosition;
    bool bWithinFOV;
    bool bShootable;

    // [추가] 누적 피해량 변수
    double fAccumulatedDamage;

    MemoryRecord() :fTimeLastSensed(-999),
        fTimeBecameVisible(-999),
        fTimeLastVisible(0),
        bWithinFOV(false),
        bShootable(false),
        fAccumulatedDamage(0.0) // [추가] 0으로 초기화
    {
    }
};



class Raven_SensoryMemory
{
private:

  typedef std::map<Raven_Bot*, MemoryRecord> MemoryMap;

private:
  
  //the owner of this instance
  Raven_Bot* m_pOwner;

  //this container is used to simulate memory of sensory events. A MemoryRecord
  //is created for each opponent in the environment. Each record is updated 
  //whenever the opponent is encountered. (when it is seen or heard)
  MemoryMap  m_MemoryMap;

  //a bot has a memory span equivalent to this value. When a bot requests a 
  //list of all recently sensed opponents this value is used to determine if 
  //the bot is able to remember an opponent or not.
  double      m_dMemorySpan;

  //this methods checks to see if there is an existing record for pBot. If
  //not a new MemoryRecord record is made and added to the memory map.(called
  //by UpdateWithSoundSource & UpdateVision)
  void       MakeNewRecordIfNotAlreadyPresent(Raven_Bot* pBot);

public:

  Raven_SensoryMemory(Raven_Bot* owner, double MemorySpan);

  //this method is used to update the memory map whenever an opponent makes
  //a noise
  void     UpdateWithSoundSource(Raven_Bot* pNoiseMaker);

  //this removes a bot's record from memory
  void     RemoveBotFromMemory(Raven_Bot* pBot);

  //this method iterates through all the opponents in the game world and 
  //updates the records of those that are in the owner's FOV
  void     UpdateVision();

  bool     isOpponentShootable(Raven_Bot* pOpponent)const;
  bool     isOpponentWithinFOV(Raven_Bot* pOpponent)const;
  Vector2D GetLastRecordedPositionOfOpponent(Raven_Bot* pOpponent)const;
  double    GetTimeOpponentHasBeenVisible(Raven_Bot* pOpponent)const;
  double    GetTimeSinceLastSensed(Raven_Bot* pOpponent)const;
  double    GetTimeOpponentHasBeenOutOfView(Raven_Bot* pOpponent)const;

  //this method returns a list of all the opponents that have had their
  //records updated within the last m_dMemorySpan seconds.
  std::list<Raven_Bot*> GetListOfRecentlySensedOpponents()const;

  void     RenderBoxesAroundRecentlySensed()const;

  // [추가] 아래 두 함수 선언 추가
  void UpdateWithDamage(Raven_Bot* pShooter, double DamageAmt);
  double GetDamageCausedByOpponent(Raven_Bot* pOpponent)const;

};


#endif