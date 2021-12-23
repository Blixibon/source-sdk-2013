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
class CNPC_BM_HumanGrunt : public CAI_Base_BM_Human<CNPC_Combine>
{
	DECLARE_CLASS( CNPC_BM_HumanGrunt, CAI_Base_BM_Human<CNPC_Combine> );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	DEFINE_CUSTOM_AI;

	CNPC_BM_HumanGrunt();

public: 
	void		Spawn( void );
	void		Precache( void );
	Class_T		Classify( void );
	void		ModifyOrAppendCriteria( AI_CriteriaSet& set );
	const char	*GetCharacterClassname();
	void		DeathSound( const CTakeDamageInfo &info );

	void		PrescheduleThink( void );
	void		BuildScheduleTestBits( void );
	int			SelectSchedule ( void );
	int 		SelectSchedulePriorityAction();
	int 		SelectScheduleHeal();
	
	void 		StartTask( const Task_t *pTask );
	void 		RunTask( const Task_t *pTask );
	void		TaskFail( AI_TaskFailureCode_t code );

	//---------------------------------
	// Special abilities
	//---------------------------------
	bool 			IsMedic() 			{ return m_SoldierType == ST_MEDIC; }
	
	bool 			CanHeal();
	bool 			ShouldHealTarget( CBaseEntity *pTarget, bool bActiveUse = false );
	bool 			ShouldHealTossTarget( CBaseEntity *pTarget, bool bActiveUse = false );
	void 			Heal();

	void			TossHealthKit( CBaseCombatCharacter *pThrowAt, const Vector &offset ); // create a healthkit and throw it at someone
	void			InputForceHealthKitToss( inputdata_t &inputdata );

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
	
	const char*		GetGrenadeAttachment() { return "anim_attachment_LH"; }

private:
	bool		ShouldHitPlayer( const Vector &targetDir, float targetDist );

public:
	Activity	NPC_TranslateActivity( Activity eNewActivity );

protected:
	/// whether to use the more casual march anim in ep2_outland_05
	int		m_iUseMarch;

	enum SoldierType_t
	{
		ST_GRUNT,
		ST_MEDIC,
		ST_COMMANDER,
	};
	SoldierType_t m_SoldierType;

	float			m_flPlayerHealTime;
	float			m_flAllyHealTime;

	COutputEvent		m_OnHealedNPC;
	COutputEvent		m_OnHealedPlayer;
	COutputEHANDLE		m_OnThrowMedkit;
	
private:
	//-----------------------------------------------------
	// Conditions, Schedules, Tasks
	//-----------------------------------------------------
	enum
	{
		COND_GRUNT_PLAYERHEALREQUEST = BaseClass::NEXT_CONDITION,
		COND_GRUNT_COMMANDHEAL,
		
		SCHED_GRUNT_HEAL = BaseClass::NEXT_SCHEDULE,
		SCHED_GRUNT_HEAL_TOSS,
		
		TASK_GRUNT_HEAL = BaseClass::NEXT_TASK,
		TASK_GRUNT_HEAL_TOSS,

	};
};

#endif // NPC_BM_HUMAN_GRUNT_H
