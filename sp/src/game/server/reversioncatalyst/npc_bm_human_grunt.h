//==============================================================================
//
// Purpose: HECU marines created from scratch based on HL2 NPCs.
// 
//==============================================================================

#ifndef NPC_BM_HUMAN_GRUNT_H
#define NPC_BM_HUMAN_GRUNT_H
#ifdef _WIN32
#pragma once
#endif

#include "hl2/npc_combine.h"
#include "ai_base_bm_npc.h"

//=========================================================
//	>> CNPC_BM_HumanGrunt
//=========================================================
class CNPC_BM_HumanGrunt : public CAI_Base_BM_NPC<CNPC_Combine>
{
	DECLARE_CLASS( CNPC_BM_HumanGrunt, CAI_Base_BM_NPC<CNPC_Combine> );
	DECLARE_DATADESC();
	//DECLARE_SERVERCLASS();

public: 
	void		Spawn( void );
	void		Precache( void );
	Class_T		Classify( void );
	void		ModifyOrAppendCriteria( AI_CriteriaSet& set );
	void		DeathSound( const CTakeDamageInfo &info );
	void		PrescheduleThink( void );
	void		BuildScheduleTestBits( void );
	int			SelectSchedule ( void );
	float		GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info );
	void		HandleAnimEvent( animevent_t *pEvent );
	void		OnChangeActivity( Activity eNewActivity );
	void		Event_Killed( const CTakeDamageInfo &info );
	void		OnListened();

	void		ClearAttackConditions( void );

	bool		m_fIsBlocking;

	bool		IsLightDamage( const CTakeDamageInfo &info );
	bool		IsHeavyDamage( const CTakeDamageInfo &info );

	virtual	bool		AllowedToIgnite( void ) { return true; }

private:
	bool		ShouldHitPlayer( const Vector &targetDir, float targetDist );

public:
	Activity	NPC_TranslateActivity( Activity eNewActivity );

protected:
	/// whether to use the more casual march anim in ep2_outland_05
	CNetworkVar( int, m_iUseMarch );

	enum SoldierType_t
	{
		ST_GRUNT,
		ST_MEDIC,
		ST_COMMANDER,
	};
	SoldierType_t m_SoldierType;

};

#endif // NPC_BM_HUMAN_GRUNT_H
