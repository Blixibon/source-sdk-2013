//==============================================================================
//
// Purpose: Black Ops assassins created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "ai_base_bm_npc.h"
#include "npc_bm_human_grunt.h"

//=========================================================
//	>> CAI_Base_BM_Assassin
//=========================================================
template <class BASE_NPC>
class CAI_Base_BM_Assassin : public BASE_NPC
{
	DECLARE_CLASS_NOFRIEND( CAI_Base_BM_Assassin, BASE_NPC );

public:

	bool		IsHumanAssassin() { return true; }

	bool		MovementCost( int moveType, const Vector &vecStart, const Vector &vecEnd, float *pCost );

	void		PrescheduleThink( void );

	Activity	NPC_TranslateActivity( Activity eNewActivity );
	void		OnChangeActivity( Activity eNewActivity );

	virtual bool		ShouldFlip();

	float		m_flLastFlipEndTime;
};

#define DEFINE_HUMAN_ASSASSIN_DATADESC() \
	DEFINE_FIELD( m_flLastFlipEndTime, FIELD_TIME ),	\

//-----------------------------------------------------------------------------
// Purpose: Causes the assassin to prefer to run away, rather than towards her target
//-----------------------------------------------------------------------------
template <class BASE_NPC>
bool CAI_Base_BM_Assassin<BASE_NPC>::MovementCost( int moveType, const Vector &vecStart, const Vector &vecEnd, float *pCost )
{
	if ( this->GetEnemy() == NULL )
		return BaseClass::MovementCost( moveType, vecStart, vecEnd, pCost );

	float	multiplier = 1.0f;

	Vector	moveDir = ( vecEnd - vecStart );
	VectorNormalize( moveDir );

	Vector	enemyDir = ( this->GetEnemy()->GetAbsOrigin() - vecStart );
	VectorNormalize( enemyDir );

	// If we're moving towards our enemy, then the cost is much higher than normal
	if ( DotProduct( enemyDir, moveDir ) > 0.5f )
	{
		multiplier = 16.0f;
	}

	*pCost *= multiplier;

	return ( multiplier != 1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template <class BASE_NPC>
void CAI_Base_BM_Assassin<BASE_NPC>::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();
}

//-----------------------------------------------------------------------------
// Purpose: Translate base class activities into combot activites
//-----------------------------------------------------------------------------
template <class BASE_NPC>
Activity CAI_Base_BM_Assassin<BASE_NPC>::NPC_TranslateActivity( Activity eNewActivity )
{
	if (eNewActivity == ACT_RUN || eNewActivity == ACT_RUN_AIM)
	{
		if (ShouldFlip())
		{
			WeaponClass_t iWepClass = this->GetActiveWeapon() ? this->GetActiveWeapon()->WeaponClassify() : WEPCLASS_INVALID;
			switch (iWepClass)
			{
				default:					eNewActivity = ACT_ASSASSIN_FLIP; break;
				case WEPCLASS_HANDGUN:		eNewActivity = ACT_ASSASSIN_FLIP_PISTOL; break;
				case WEPCLASS_RIFLE:		eNewActivity = ACT_ASSASSIN_FLIP_RIFLE; break;
			}
		}
	}

	return BaseClass::NPC_TranslateActivity( eNewActivity );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template <class BASE_NPC>
void CAI_Base_BM_Assassin<BASE_NPC>::OnChangeActivity( Activity eNewActivity )
{
	BaseClass::OnChangeActivity( eNewActivity );

	//if (eNewActivity == ACT_ASSASSIN_FLIP || eNewActivity == ACT_ASSASSIN_FLIP_PISTOL)
	if (GetTranslatedActivity() == ACT_ASSASSIN_FLIP || GetTranslatedActivity() == ACT_ASSASSIN_FLIP_PISTOL)
	{
		m_flLastFlipEndTime = gpGlobals->curtime;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
template <class BASE_NPC>
bool CAI_Base_BM_Assassin<BASE_NPC>::ShouldFlip()
{
	return false;
}

//=========================================================
//	>> CNPC_BM_HumanFemaleAssassin
//=========================================================
class CNPC_BM_HumanFemaleAssassin : public CAI_Base_BM_Assassin<CAI_Base_BM_Human<CNPC_Combine>>
{
	DECLARE_CLASS( CNPC_BM_HumanFemaleAssassin, CAI_Base_BM_Assassin<CAI_Base_BM_Human<CNPC_Combine>> );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	DEFINE_CUSTOM_AI;

	CNPC_BM_HumanFemaleAssassin();

public: 
	void		Spawn( void );
	void		Precache( void );
	Class_T		Classify( void );
	const char	*GetCharacterClassname();

	void		Event_Killed( const CTakeDamageInfo &info );
	void		Weapon_HandleEquip( CBaseCombatWeapon *pWeapon );

	void		KickAttack( bool bLow );
	bool		ShouldFlip();

	WeaponProficiency_t CalcWeaponProficiency( CBaseCombatWeapon *pWeapon );

	void		HandleAnimEvent( animevent_t *pEvent );

	Activity	NPC_TranslateActivity( Activity eNewActivity );
	void		OnChangeActivity( Activity eNewActivity );

	void		PrescheduleThink( void );

	int			TranslateSchedule( int scheduleType );
	int			SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );

	void 		StartTask( const Task_t *pTask );

	bool		IsJumpLegal( const Vector &startPos, const Vector &apex, const Vector &endPos ) const;

	const char *GetGrenadeAttachment() { return "anim_attachment_LH"; }

private:
	//-----------------------------------------------------
	// Conditions, Schedules, Tasks
	//-----------------------------------------------------
	enum
	{
		COND_GRUNT_PLAYERHEALREQUEST = BaseClass::NEXT_CONDITION,
		COND_GRUNT_COMMANDHEAL,
		
		SCHED_HUMAN_ASSASSIN_RANGE_ATTACK1 = BaseClass::NEXT_SCHEDULE,
		SCHED_HUMAN_ASSASSIN_EVADE,
		SCHED_HUMAN_ASSASSIN_FLANK_RANDOM,
		
		TASK_GRUNT_HEAL = BaseClass::NEXT_TASK,
		TASK_GRUNT_HEAL_TOSS,

	};

	CHandle<CBaseAnimating>		m_hLeftHandGun;

	CNetworkVar( float, m_flCloakFactor );
};

//=========================================================
//	>> CNPC_BM_HumanMaleAssassin
// (technically a completely new NPC in relation to Black Mesa: Source)
//=========================================================
class CNPC_BM_HumanMaleAssassin : public CAI_Base_BM_Assassin<CNPC_BM_HumanGrunt>
{
	DECLARE_CLASS( CNPC_BM_HumanMaleAssassin, CAI_Base_BM_Assassin<CNPC_BM_HumanGrunt> );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CNPC_BM_HumanMaleAssassin();

public:
	bool		IsHumanGrunt() { return false; }

	void		Spawn( void );
	void		Precache( void );
	Class_T		Classify( void );
	const char	*GetCharacterClassname();

	Activity	NPC_TranslateActivity( Activity eNewActivity );
	void		OnChangeActivity( Activity eNewActivity );

	bool		ShouldFlip();
	bool		ShouldHaveSpeedBoost();

	void		PrescheduleThink( void );
	float		GetSequenceGroundSpeed( CStudioHdr *pStudioHdr, int iSequence );
};
