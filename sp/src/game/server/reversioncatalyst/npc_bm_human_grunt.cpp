//==============================================================================
//
// Purpose: HECU marines created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbase.h"
#include "npc_bm_human_grunt.h"
#include "ammodef.h"
#include "gameweaponmanager.h"
#include "vehicle_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	sk_human_grunt_health( "sk_human_grunt_health","50" );
ConVar	sk_human_grunt_kick( "sk_human_grunt_kick", "10" );
ConVar	sk_human_grunt_heal_player( "sk_human_grunt_heal_player", "25" );
ConVar	sk_human_grunt_heal_player_delay( "sk_human_grunt_heal_player_delay", "25" );
ConVar	sk_human_grunt_heal_player_min_pct( "sk_human_grunt_heal_player_min_pct", "0.60" );
ConVar	sk_human_grunt_heal_player_min_forced( "sk_human_grunt_heal_player_min_forced", "10.0" );
ConVar	sk_human_grunt_heal_ally( "sk_human_grunt_heal_ally", "30" );
ConVar	sk_human_grunt_heal_ally_delay( "sk_human_grunt_heal_ally_delay", "20" );
ConVar	sk_human_grunt_heal_ally_min_pct( "sk_human_grunt_heal_ally_min_pct", "0.90" );
ConVar	sk_human_grunt_player_stare_time( "sk_human_grunt_player_stare_time", "1.0" );
ConVar  sk_human_grunt_player_stare_dist( "sk_human_grunt_player_stare_dist", "72" );
ConVar	sk_human_grunt_stare_heal_time( "sk_human_grunt_stare_heal_time", "5" );

ConVar  npc_human_grunt_medic_emit_sound("npc_human_grunt_medic_emit_sound", "1" );
ConVar	npc_human_grunt_heal_chuck_medkit("npc_human_grunt_heal_chuck_medkit" , "1" , FCVAR_ARCHIVE, "Set to 1 to use new experimental healthkit-throwing medic.");
ConVar	npc_human_grunt_medic_throw_style( "npc_human_grunt_medic_throw_style", "1", FCVAR_ARCHIVE, "Set to 0 for a lobbier trajectory" );
ConVar	npc_human_grunt_medic_throw_speed( "npc_human_grunt_medic_throw_speed", "650" );
ConVar	sk_human_grunt_heal_toss_player_delay("sk_human_grunt_heal_toss_player_delay", "26", FCVAR_NONE, "how long between throwing healthkits" );

#define MEDIC_THROW_SPEED npc_human_grunt_medic_throw_speed.GetFloat()
#define USE_EXPERIMENTAL_MEDIC_CODE() (npc_human_grunt_heal_chuck_medkit.GetBool() /*&& m_bTossesMedkits*/)

extern ConVar sk_plr_dmg_buckshot;	
extern ConVar sk_plr_num_shotgun_pellets;

const float HEAL_MOVE_RANGE = 30*12;
const float HEAL_TARGET_RANGE = 120; // 10 feet
const float HEAL_TOSS_TARGET_RANGE = 480; // 40 feet when we are throwing medkits 
const float HEAL_TARGET_RANGE_Z = 72; // a second check that Gordon isn't too far above us -- 6 feet

LINK_ENTITY_TO_CLASS( npc_bm_human_grunt, CNPC_BM_HumanGrunt );
LINK_ENTITY_TO_CLASS( npc_human_grunt, CNPC_BM_HumanGrunt ); // For simplicity/ease of use/legacy support/etc.
LINK_ENTITY_TO_CLASS( npc_human_medic, CNPC_BM_HumanGrunt ); // For simplicity/ease of use/legacy support/etc.
LINK_ENTITY_TO_CLASS( npc_human_commander, CNPC_BM_HumanGrunt ); // For simplicity/ease of use/legacy support/etc.
LINK_ENTITY_TO_CLASS( npc_human_grenadier, CNPC_BM_HumanGrunt ); // For simplicity/ease of use/legacy support/etc.

extern int AE_CITIZEN_HEAL;

extern Activity ACT_WALK_EASY;
extern Activity ACT_WALK_MARCH;
extern int ACT_CIT_HEAL;

#define BODYGROUP_CIGAR 2
#define BODYGROUP_GLOVES 3
#define BODYGROUP_HELMET_NV 4
#define BODYGROUP_HEAD 5
#define BODYGROUP_HELMET_MEDIC 6
#define BODYGROUP_HOLSTER 7
#define BODYGROUP_PACKS_CHEST 8
#define BODYGROUP_PACKS_HIPS 9
#define BODYGROUP_PACKS_THIGH 10

#define BERET_MODEL "models/humans/props/marine_beret.mdl"

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_BM_HumanGrunt )

	DEFINE_KEYFIELD( m_iUseMarch, FIELD_INTEGER, "usemarch" ),
	DEFINE_KEYFIELD( m_SoldierType, FIELD_INTEGER, "SoldierType" ),

	DEFINE_FIELD( m_flPlayerHealTime, FIELD_TIME ),
	DEFINE_FIELD( m_flAllyHealTime, FIELD_TIME ),

	DEFINE_OUTPUT(		m_OnHealedNPC,			"OnHealedNPC" ),
	DEFINE_OUTPUT(		m_OnHealedPlayer,		"OnHealedPlayer" ),
	DEFINE_OUTPUT(		m_OnThrowMedkit,		"OnTossMedkit" ),

	DEFINE_INPUTFUNC( FIELD_VOID,   "ThrowHealthKit", InputForceHealthKitToss ),

	DECLARE_BM_NPC_DATADESC()
	DECLARE_BM_HUMAN_DATADESC()

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CNPC_BM_HumanGrunt, DT_NPC_BM_HumanGrunt )
	//SendPropBool( SENDINFO( m_bHasCigar ) ),

	SendPropInt( SENDINFO( m_iCharacterIndex ), 16, 0 ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNPC_BM_HumanGrunt::CNPC_BM_HumanGrunt()
{
	SetUnderthrowGrenades( true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_BM_HumanGrunt::Spawn( void )
{
	Precache();
	SetModel( STRING( GetModelName() ) );

	SetHealth( sk_human_grunt_health.GetFloat() );
	SetMaxHealth( sk_human_grunt_health.GetFloat() );
	SetKickDamage( sk_human_grunt_kick.GetFloat() );

	CapabilitiesAdd( bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD );
	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );
	CapabilitiesAdd( bits_CAP_DOORS_GROUP );

	BaseClass::Spawn();

#if HL2_EPISODIC
	if (m_iUseMarch && !HasSpawnFlags(SF_NPC_START_EFFICIENT))
	{
		Msg( "Soldier %s is set to use march anim, but is not an efficient AI. The blended march anim can only be used for dead-ahead walks!\n", GetDebugName() );
	}
#endif

	if (m_SoldierType == ST_COMMANDER)
	{
		m_fIsElite = true;

		/*
		if (GetSquad())
		{
			// Move commanders to the front of the squad so that they become leaders
			GetSquad()->MoveToFront( this );
		}
		*/
	}

	if (GetActiveWeapon() && GetActiveWeapon()->WeaponClassify() == WEPCLASS_HANDGUN)
	{
		/*if (!m_fWeaponDrawn)
		{
			DoHolster();
		}
		else*/ if (GetBodygroup(BODYGROUP_HOLSTER) != 0)
		{
			// Empty holster
			SetBodygroup( BODYGROUP_HOLSTER, 2 );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_BM_HumanGrunt::Precache()
{
	if (!IsHumanAssassin())
	{
		if( !GetModelName() )
		{
			SetModelName( MAKE_STRING( "models/humans/marine.mdl" ) );
		}

		if (FStrEq( GetClassname(), "npc_human_medic" ))
			m_SoldierType = ST_MEDIC;
		else if (FStrEq( GetClassname(), "npc_human_commander" ))
			m_SoldierType = ST_COMMANDER;

		if (!FStrEq( GetClassname(), "npc_bm_human_grunt" ))
			SetClassname( "npc_bm_human_grunt" );

		//PrecacheModel( BERET_MODEL );
	}

	if (!m_bCustomBody)
	{
		SelectAndApplyCharacter();
	}

	PrecacheModel( STRING( GetModelName() ) );

	UTIL_PrecacheOther( "npc_bm_grenade_frag" ); // weapon_bm_frag?
	UTIL_PrecacheOther( "item_ammo_smg1_grenade" );

	BaseClass::Precache();
}

//=========================================================
// Classify - indicates this NPC's place in the 
// relationship table.
//=========================================================
Class_T	CNPC_BM_HumanGrunt::Classify( void )
{
	return	CLASS_MILITARY;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_BM_HumanGrunt::ModifyOrAppendCriteria( AI_CriteriaSet& set )
{
	BaseClass::ModifyOrAppendCriteria( set );

	//set.AppendCriteria( "young", entindex() % 2 == 1 ? "1" : "0" );
	set.AppendCriteria( "young", m_SoldierType == ST_MEDIC ? "1" : "0" );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const char *CNPC_BM_HumanGrunt::GetCharacterClassname()
{
	switch (m_SoldierType)
	{
		default:
		case ST_GRUNT:
			return "npc_human_grunt";

		case ST_MEDIC:
			return "npc_human_medic";

		case ST_COMMANDER:
			return "npc_human_commander";
	}
}


void CNPC_BM_HumanGrunt::DeathSound( const CTakeDamageInfo &info )
{
#ifdef COMBINE_SOLDIER_USES_RESPONSE_SYSTEM
	AI_CriteriaSet set;
	ModifyOrAppendDamageCriteria(set, info);
	SpeakIfAllowed( TLK_CMB_DIE, set, SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS );
#else
	// NOTE: The response system deals with this at the moment
	if ( GetFlags() & FL_DISSOLVING )
		return;

	GetSentences()->Speak( "COMBINE_DIE", SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS ); 
#endif
}


//-----------------------------------------------------------------------------
// Purpose: Allows NPC to holster from more than just the animation event
//-----------------------------------------------------------------------------
bool CNPC_BM_HumanGrunt::DoHolster( void )
{
	if (GetActiveWeapon() && IsSidearm( GetActiveWeapon() ) && GetBodygroup( BODYGROUP_HOLSTER ) == 2)
	{
		// Toggle our holster bodygroup
		SetBodygroup( BODYGROUP_HOLSTER, 1 );
	}

	return BaseClass::DoHolster();
}

//-----------------------------------------------------------------------------
// Purpose: Allows NPC to unholster from more than just the animation event
//-----------------------------------------------------------------------------
bool CNPC_BM_HumanGrunt::DoUnholster( void )
{
	bool bBase = BaseClass::DoUnholster();

	if (bBase)
	{
		if (GetActiveWeapon() && IsSidearm( GetActiveWeapon() ) && GetBodygroup( BODYGROUP_HOLSTER ) == 1)
		{
			// Toggle our holster bodygroup
			SetBodygroup( BODYGROUP_HOLSTER, 2 );
		}
	}

	return bBase;
}


//-----------------------------------------------------------------------------
// Purpose: Soldiers use CAN_RANGE_ATTACK2 to indicate whether they can throw
//			a grenade. Because they check only every half-second or so, this
//			condition must persist until it is updated again by the code
//			that determines whether a grenade can be thrown, so prevent the 
//			base class from clearing it out. (sjb)
//-----------------------------------------------------------------------------
void CNPC_BM_HumanGrunt::ClearAttackConditions()
{
	bool fCanRangeAttack2 = HasCondition( COND_CAN_RANGE_ATTACK2 );

	// Call the base class.
	BaseClass::ClearAttackConditions();

	if( fCanRangeAttack2 )
	{
		// We don't allow the base class to clear this condition because we
		// don't sense for it every frame.
		SetCondition( COND_CAN_RANGE_ATTACK2 );
	}
}

void CNPC_BM_HumanGrunt::PrescheduleThink( void )
{
	/*//FIXME: This doesn't need to be in here, it's all debug info
	if( HasCondition( COND_HEAR_PHYSICS_DANGER ) )
	{
		// Don't react unless we see the item!!
		CSound *pSound = NULL;

		pSound = GetLoudestSoundOfType( SOUND_PHYSICS_DANGER );

		if( pSound )
		{
			if( FInViewCone( pSound->GetSoundReactOrigin() ) )
			{
				DevMsg( "OH CRAP!\n" );
				NDebugOverlay::Line( EyePosition(), pSound->GetSoundReactOrigin(), 0, 0, 255, false, 2.0f );
			}
		}
	}
	*/

	BaseClass::PrescheduleThink();
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_BM_HumanGrunt::BuildScheduleTestBits( void )
{
	//Interrupt any schedule with physics danger (as long as I'm not moving or already trying to block)
	if ( m_flGroundSpeed == 0.0 && !IsCurSchedule( SCHED_FLINCH_PHYSICS ) )
	{
		SetCustomInterruptCondition( COND_HEAR_PHYSICS_DANGER );
	}

	BaseClass::BuildScheduleTestBits();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CNPC_BM_HumanGrunt::SelectSchedule( void )
{
	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CNPC_BM_HumanGrunt::SelectSchedulePriorityAction()
{
	int schedule = SelectScheduleHeal();
	if ( schedule != SCHED_NONE )
		return schedule;

	return BaseClass::SelectSchedulePriorityAction();
}

//-----------------------------------------------------------------------------
// Determine if human_grunt should perform heal action.
//-----------------------------------------------------------------------------
int CNPC_BM_HumanGrunt::SelectScheduleHeal()
{
	// episodic medics may toss the healthkits rather than poke you with them
	if ( CanHeal() )
	{
		CBaseEntity *pEntity = PlayerInRange( GetLocalOrigin(), HEAL_TOSS_TARGET_RANGE );
		if ( pEntity )
		{
			if ( USE_EXPERIMENTAL_MEDIC_CODE() && IsMedic() )
			{
				// use the new heal toss algorithm
				if ( ShouldHealTossTarget( pEntity, HasCondition( COND_GRUNT_PLAYERHEALREQUEST ) ) )
				{
					SetTarget( pEntity );
					return SCHED_GRUNT_HEAL_TOSS;
				}
			}
			else if ( PlayerInRange( GetLocalOrigin(), HEAL_MOVE_RANGE ) )
			{
				// use old mechanism for ammo
				if ( ShouldHealTarget( pEntity, HasCondition( COND_GRUNT_PLAYERHEALREQUEST ) ) )
				{
					SetTarget( pEntity );
					return SCHED_GRUNT_HEAL;
				}
			}

		}
		
		if ( m_pSquad )
		{
			pEntity = NULL;
			float distClosestSq = HEAL_MOVE_RANGE*HEAL_MOVE_RANGE;
			float distCurSq;
			
			AISquadIter_t iter;
			CAI_BaseNPC *pSquadmate = m_pSquad->GetFirstMember( &iter );
			while ( pSquadmate )
			{
				if ( pSquadmate != this )
				{
					distCurSq = ( GetAbsOrigin() - pSquadmate->GetAbsOrigin() ).LengthSqr();
					if ( distCurSq < distClosestSq && ShouldHealTarget( pSquadmate ) )
					{
						distClosestSq = distCurSq;
						pEntity = pSquadmate;
					}
				}

				pSquadmate = m_pSquad->GetNextMember( &iter );
			}
			
			if ( pEntity )
			{
				SetTarget( pEntity );
				return SCHED_GRUNT_HEAL;
			}
		}
	}
	else
	{
		if ( HasCondition( COND_GRUNT_PLAYERHEALREQUEST ) )
			DevMsg( "Would say: sorry, need to recharge\n" );
	}
	
	return SCHED_NONE;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_BM_HumanGrunt::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_GRUNT_HEAL:
	case TASK_GRUNT_HEAL_TOSS:
		if ( IsMedic() )
		{
			if ( GetTarget() && GetTarget()->IsPlayer() && GetTarget()->m_iMaxHealth == GetTarget()->m_iHealth )
			{
				// Doesn't need us anymore
				TaskComplete();
				break;
			}

			Speak( "TLK_HEAL" );
		}
		SetIdealActivity( (Activity)ACT_CIT_HEAL );
		break;

	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_BM_HumanGrunt::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
		case TASK_GRUNT_HEAL:
			if ( IsSequenceFinished() )
			{
				TaskComplete();
			}
			else if (!GetTarget())
			{
				// Our heal target was killed or deleted somehow.
				TaskFail(FAIL_NO_TARGET);
			}
			else
			{
				if ( ( GetTarget()->GetAbsOrigin() - GetAbsOrigin() ).Length2D() > HEAL_MOVE_RANGE/2 )
					TaskComplete();

				GetMotor()->SetIdealYawToTargetAndUpdate( GetTarget()->GetAbsOrigin() );
			}
			break;

		case TASK_GRUNT_HEAL_TOSS:
			if ( IsSequenceFinished() )
			{
				TaskComplete();
			}
			else if (!GetTarget())
			{
				// Our heal target was killed or deleted somehow.
				TaskFail(FAIL_NO_TARGET);
			}
			else
			{
				GetMotor()->SetIdealYawToTargetAndUpdate( GetTarget()->GetAbsOrigin() );
			}
			break;

		default:
			BaseClass::RunTask( pTask );
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : code - 
//-----------------------------------------------------------------------------
void CNPC_BM_HumanGrunt::TaskFail( AI_TaskFailureCode_t code )
{
	// If our heal task has failed, push out the heal time
	if ( IsCurSchedule( SCHED_GRUNT_HEAL ) )
	{
		m_flPlayerHealTime 	= gpGlobals->curtime + sk_human_grunt_heal_ally_delay.GetFloat();
	}

	BaseClass::TaskFail( code );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CNPC_BM_HumanGrunt::CanHeal()
{ 
	if ( !IsMedic() )
		return false;

	if ( IsInAScript() || (m_NPCState == NPC_STATE_SCRIPT) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CNPC_BM_HumanGrunt::ShouldHealTarget( CBaseEntity *pTarget, bool bActiveUse )
{
	Disposition_t disposition;
	
	if ( pTarget && ( ( disposition = IRelationType( pTarget ) ) != D_LI && disposition != D_NU ) )
		return false;

	// Don't heal if I'm in the middle of talking
	if ( IsSpeaking() )
		return false;

	bool bTargetIsPlayer = pTarget->IsPlayer();

	// Don't heal or give ammo to targets in vehicles
	CBaseCombatCharacter *pCCTarget = pTarget->MyCombatCharacterPointer();
	if ( pCCTarget != NULL && pCCTarget->IsInAVehicle() )
		return false;

	if ( IsMedic() )
	{
		Vector toPlayer = ( pTarget->GetAbsOrigin() - GetAbsOrigin() );
	 	if (( bActiveUse /*|| !HaveCommandGoal()*/ || toPlayer.Length() < HEAL_TARGET_RANGE) 
			&& fabs(toPlayer.z) < HEAL_TARGET_RANGE_Z
			)
	 	{
			if ( pTarget->m_iHealth > 0 )
			{
	 			if ( bActiveUse )
				{
					// Ignore heal requests if we're going to heal a tiny amount
					float timeFullHeal = m_flPlayerHealTime;
					float timeRecharge = sk_human_grunt_heal_player_delay.GetFloat();
					float maximumHealAmount = sk_human_grunt_heal_player.GetFloat();
					float healAmt = ( maximumHealAmount * ( 1.0 - ( timeFullHeal - gpGlobals->curtime ) / timeRecharge ) );
					if ( healAmt > pTarget->m_iMaxHealth - pTarget->m_iHealth )
						healAmt = pTarget->m_iMaxHealth - pTarget->m_iHealth;
					if ( healAmt < sk_human_grunt_heal_player_min_forced.GetFloat() )
						return false;

	 				return ( pTarget->m_iMaxHealth > pTarget->m_iHealth );
				}
	 				
				// Are we ready to heal again?
				bool bReadyToHeal = ( ( bTargetIsPlayer && m_flPlayerHealTime <= gpGlobals->curtime ) || 
									  ( !bTargetIsPlayer && m_flAllyHealTime <= gpGlobals->curtime ) );

				// Only heal if we're ready
				if ( bReadyToHeal )
				{
					int requiredHealth;

					if ( bTargetIsPlayer )
						requiredHealth = pTarget->GetMaxHealth() - sk_human_grunt_heal_player.GetFloat();
					else
						requiredHealth = pTarget->GetMaxHealth() * sk_human_grunt_heal_player_min_pct.GetFloat();

					if ( ( pTarget->m_iHealth <= requiredHealth ) && IRelationType( pTarget ) == D_LI )
						return true;
				}
			}
		}
	}
	return false;
}

#ifdef HL2_EPISODIC
//-----------------------------------------------------------------------------
// Determine if the human_grunt is in a position to be throwing medkits
//-----------------------------------------------------------------------------
bool CNPC_BM_HumanGrunt::ShouldHealTossTarget( CBaseEntity *pTarget, bool bActiveUse )
{
	Disposition_t disposition;

	Assert( IsMedic() );
	if ( !IsMedic() )
		return false;
	
	if ( pTarget && ( ( disposition = IRelationType( pTarget ) ) != D_LI && disposition != D_NU ) )
		return false;

	// Don't heal if I'm in the middle of talking
	if ( IsSpeaking() )
		return false;

	// NPCs cannot be healed by throwing medkits at them.
	// I don't think NPCs even pass through this function anyway, it's just the actual heal event that's the problem.
	if (!pTarget->IsPlayer())
		return false;

	// Don't heal or give ammo to targets in vehicles
	CBaseCombatCharacter *pCCTarget = pTarget->MyCombatCharacterPointer();
	if ( pCCTarget != NULL && pCCTarget->IsInAVehicle() )
		return false;

	Vector toPlayer = ( pTarget->GetAbsOrigin() - GetAbsOrigin() );
	if ( bActiveUse /*|| !HaveCommandGoal()*/ || toPlayer.Length() < HEAL_TOSS_TARGET_RANGE )
	{
		if ( pTarget->m_iHealth > 0 )
		{
			if ( bActiveUse )
			{
				// Ignore heal requests if we're going to heal a tiny amount
				float timeFullHeal = m_flPlayerHealTime;
				float timeRecharge = sk_human_grunt_heal_player_delay.GetFloat();
				float maximumHealAmount = sk_human_grunt_heal_player.GetFloat();
				float healAmt = ( maximumHealAmount * ( 1.0 - ( timeFullHeal - gpGlobals->curtime ) / timeRecharge ) );
				if ( healAmt > pTarget->m_iMaxHealth - pTarget->m_iHealth )
					healAmt = pTarget->m_iMaxHealth - pTarget->m_iHealth;
				if ( healAmt < sk_human_grunt_heal_player_min_forced.GetFloat() )
					return false;

				return ( pTarget->m_iMaxHealth > pTarget->m_iHealth );
			}

			// Are we ready to heal again?
#ifdef MAPBASE
			bool bReadyToHeal = m_flPlayerHealTime <= gpGlobals->curtime;
#else
			bool bReadyToHeal = ( ( bTargetIsPlayer && m_flPlayerHealTime <= gpGlobals->curtime ) || 
				( !bTargetIsPlayer && m_flAllyHealTime <= gpGlobals->curtime ) );
#endif

			// Only heal if we're ready
			if ( bReadyToHeal )
			{
				int requiredHealth;

#ifdef MAPBASE
				requiredHealth = pTarget->GetMaxHealth() - sk_human_grunt_heal_player.GetFloat();
#else
				if ( bTargetIsPlayer )
					requiredHealth = pTarget->GetMaxHealth() - sk_human_grunt_heal_player.GetFloat();
				else
					requiredHealth = pTarget->GetMaxHealth() * sk_human_grunt_heal_player_min_pct.GetFloat();
#endif

				if ( ( pTarget->m_iHealth <= requiredHealth ) && IRelationType( pTarget ) == D_LI )
					return true;
			}
		}
	}
	
	return false;
}
#endif


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_BM_HumanGrunt::Heal()
{
	if ( !CanHeal() )
		  return;

	CBaseEntity *pTarget = GetTarget();
	if ( !pTarget )
		return;

	Vector target = pTarget->GetAbsOrigin() - GetAbsOrigin();
	if ( target.Length() > HEAL_TARGET_RANGE * 2 )
		return;

	if ( IsMedic() )
	{
		float timeFullHeal;
		float timeRecharge;
		float maximumHealAmount;
		if ( pTarget->IsPlayer() )
		{
			timeFullHeal 		= m_flPlayerHealTime;
			timeRecharge 		= sk_human_grunt_heal_player_delay.GetFloat();
			maximumHealAmount 	= sk_human_grunt_heal_player.GetFloat();
			m_flPlayerHealTime 	= gpGlobals->curtime + timeRecharge;
		}
		else
		{
			timeFullHeal 		= m_flAllyHealTime;
			timeRecharge 		= sk_human_grunt_heal_ally_delay.GetFloat();
			maximumHealAmount 	= sk_human_grunt_heal_ally.GetFloat();
			m_flAllyHealTime 	= gpGlobals->curtime + timeRecharge;
		}
		
		float healAmt = ( maximumHealAmount * ( 1.0 - ( timeFullHeal - gpGlobals->curtime ) / timeRecharge ) );
		
		if ( healAmt > maximumHealAmount )
			healAmt = maximumHealAmount;
		else
			healAmt = RoundFloatToInt( healAmt );
		
		if ( healAmt > 0 )
		{
			if ( pTarget->IsPlayer() && npc_human_grunt_medic_emit_sound.GetBool() )
			{
				CPASAttenuationFilter filter( pTarget, "HealthKit.Touch" );
				EmitSound( filter, pTarget->entindex(), "HealthKit.Touch" );
			}

			pTarget->IsPlayer() ? m_OnHealedPlayer.FireOutput(pTarget, this) : m_OnHealedNPC.FireOutput(pTarget, this);

			pTarget->TakeHealth( healAmt, DMG_GENERIC );
			pTarget->RemoveAllDecals();
		}
	}
}

//-----------------------------------------------------------------------------
// Like Heal(), but tosses a healthkit in front of the player rather than just juicing him up.
//-----------------------------------------------------------------------------
void CNPC_BM_HumanGrunt::TossHealthKit(CBaseCombatCharacter *pThrowAt, const Vector &offset)
{
	Assert( pThrowAt );

	Vector forward, right, up;
	GetVectors( &forward, &right, &up );
	Vector medKitOriginPoint = WorldSpaceCenter() + ( forward * 20.0f );
	Vector destinationPoint;
	// this doesn't work without a moveparent: pThrowAt->ComputeAbsPosition( offset, &destinationPoint );
	VectorTransform( offset, pThrowAt->EntityToWorldTransform(), destinationPoint );
	// flatten out any z change due to player looking up/down
	destinationPoint.z = pThrowAt->EyePosition().z;

	Vector tossVelocity;

	if (npc_human_grunt_medic_throw_style.GetInt() == 0)
	{
		CTraceFilterSkipTwoEntities tracefilter( this, pThrowAt, COLLISION_GROUP_NONE );
		tossVelocity = VecCheckToss( this, &tracefilter, medKitOriginPoint, destinationPoint, 0.233f, 1.0f, false );
	}
	else
	{
		tossVelocity = VecCheckThrow( this, medKitOriginPoint, destinationPoint, MEDIC_THROW_SPEED, 1.0f );

		if (vec3_origin == tossVelocity)
		{
			// if out of range, just throw it as close as I can
			tossVelocity = destinationPoint - medKitOriginPoint;

			// rotate upwards against gravity
			float len = VectorLength(tossVelocity);
			tossVelocity *= (MEDIC_THROW_SPEED / len);
			tossVelocity.z += 0.57735026918962576450914878050196 * MEDIC_THROW_SPEED;
		}
	}

	// create a healthkit and toss it into the world
	CBaseEntity *pHealthKit = CreateEntityByName( "item_healthkit" );
	Assert(pHealthKit);
	if (pHealthKit)
	{
		pHealthKit->SetAbsOrigin( medKitOriginPoint );
		pHealthKit->SetOwnerEntity( this );
		// pHealthKit->SetAbsVelocity( tossVelocity );
		DispatchSpawn( pHealthKit );

		{
			IPhysicsObject *pPhysicsObject = pHealthKit->VPhysicsGetObject();
			Assert( pPhysicsObject );
			if ( pPhysicsObject )
			{
				unsigned int cointoss = random->RandomInt(0,0xFF); // int bits used for bools

				// some random precession
				Vector angDummy(random->RandomFloat(-200,200), random->RandomFloat(-200,200), 
					cointoss & 0x01 ? random->RandomFloat(200,600) : -1.0f * random->RandomFloat(200,600));
				pPhysicsObject->SetVelocity( &tossVelocity, &angDummy );
			}
		}

		m_OnThrowMedkit.Set(pHealthKit, pHealthKit, this);
	}
	else
	{
		Warning("Grunt tried to heal but could not spawn item_healthkit!\n");
	}
}

//-----------------------------------------------------------------------------
// cause an immediate call to TossHealthKit with some default numbers
//-----------------------------------------------------------------------------
void CNPC_BM_HumanGrunt::InputForceHealthKitToss( inputdata_t &inputdata )
{
	TossHealthKit( UTIL_GetLocalPlayer(), Vector(48.0f, 0.0f, 0.0f)  );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CNPC_BM_HumanGrunt::GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info )
{
	switch( iHitGroup )
	{
	case HITGROUP_HEAD:
		{
			// Soldiers take double headshot damage
			return 2.0f;
		}
	}

	return BaseClass::GetHitgroupDamageMultiplier( iHitGroup, info );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_BM_HumanGrunt::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_CITIZEN_HEAL )
	{
		// Heal my target (if within range)
		if ( USE_EXPERIMENTAL_MEDIC_CODE() && IsMedic() && GetTarget() && !GetTarget()->IsNPC() )
		{
			CBaseCombatCharacter *pTarget = dynamic_cast<CBaseCombatCharacter *>( GetTarget() );
			Assert(pTarget);
			if ( pTarget )
			{
				m_flPlayerHealTime 	= gpGlobals->curtime + sk_human_grunt_heal_toss_player_delay.GetFloat();;
				TossHealthKit( pTarget, Vector(48.0f, 0.0f, 0.0f)  );
			}
		}
		else
		{
			Heal();
		}
		return;
	}

	//switch( pEvent->event )
	//{
	//default:
		BaseClass::HandleAnimEvent( pEvent );
	//	break;
	//}
}

void CNPC_BM_HumanGrunt::OnChangeActivity( Activity eNewActivity )
{
	// Any new sequence stops us blocking.
	m_fIsBlocking = false;

	BaseClass::OnChangeActivity( eNewActivity );

#if HL2_EPISODIC
	// Give each trooper a varied look for his march. Done here because if you do it earlier (eg Spawn, StartTask), the
	// pose param gets overwritten.
	if (m_iUseMarch)
	{
		SetPoseParameter("casual", RandomFloat());
	}
#endif
}

void CNPC_BM_HumanGrunt::OnListened()
{
	BaseClass::OnListened();

	if ( HasCondition( COND_HEAR_DANGER ) && HasCondition( COND_HEAR_PHYSICS_DANGER ) )
	{
		if ( HasInterruptCondition( COND_HEAR_DANGER ) )
		{
			ClearCondition( COND_HEAR_PHYSICS_DANGER );
		}
	}

	// debugging to find missed schedules
#if 0
	if ( HasCondition( COND_HEAR_DANGER ) && !HasInterruptCondition( COND_HEAR_DANGER ) )
	{
		DevMsg("Ignore danger in %s\n", GetCurSchedule()->GetName() );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
void CNPC_BM_HumanGrunt::Event_Killed( const CTakeDamageInfo &info )
{
	for (int i = 0; i < m_hBonemergeProps.Count(); i++)
	{
		if (CBaseEntity *pProp = m_hBonemergeProps[i])
		{
			Vector vecHeadPos;
			QAngle vecHeadAng;
			GetAttachment( "anim_attachment_head", vecHeadPos, vecHeadAng );

			// Drop the beret
			CBaseEntity *pGib = CreateNoSpawn( "prop_physics", vecHeadPos, vecHeadAng, this );
			if (pGib)
			{
				pGib->SetModelName( pProp->GetModelName() );
				pGib->AddSpawnFlags( SF_PHYSPROP_DEBRIS | SF_PHYSPROP_IS_GIB );
				DispatchSpawn( pGib );

				if (VPhysicsGetObject() && pGib->VPhysicsGetObject())
				{
					Vector velocity = info.GetDamageForce() * VPhysicsGetObject()->GetInvMass();

					// Give the beret some extra velocity so it's easier to see
					velocity.z += 20.0f;
					velocity *= 3.0f;

					pGib->VPhysicsGetObject()->AddVelocity(&velocity, NULL);
				}

				if (info.GetDamageType() & DMG_DISSOLVE)
				{
					pGib->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
				}
				else
				{
					pGib->SUB_StartFadeOut( 10.0f, false );
				}
			}

			UTIL_Remove( pProp );
		}
	}

	// Disabled for now
	/*
	CBasePlayer *pPlayer = ToBasePlayer( info.GetAttacker() );

	if ( !pPlayer )
	{
		CPropVehicleDriveable *pVehicle = dynamic_cast<CPropVehicleDriveable *>( info.GetAttacker() ) ;
		if ( pVehicle && pVehicle->GetDriver() && pVehicle->GetDriver()->IsPlayer() )
		{
			pPlayer = assert_cast<CBasePlayer *>( pVehicle->GetDriver() );
		}
	}

	if ( pPlayer != NULL )
	{
		CHalfLife2 *pHL2GameRules = static_cast<CHalfLife2 *>(g_pGameRules);

		// Attempt to drop health
		if ( pHL2GameRules->NPC_ShouldDropHealth( pPlayer ) )
		{
			DropItem( "item_healthvial", WorldSpaceCenter()+RandomVector(-4,4), RandomAngle(0,360) );
			pHL2GameRules->NPC_DroppedHealth();
		}
		
		if ( HasSpawnFlags( SF_COMBINE_NO_GRENADEDROP ) == false )
		{
			// Attempt to drop a grenade
			if ( pHL2GameRules->NPC_ShouldDropGrenade( pPlayer ) )
			{
				DropItem( "weapon_frag", WorldSpaceCenter()+RandomVector(-4,4), RandomAngle(0,360) );
				pHL2GameRules->NPC_DroppedGrenade();
			}
		}
	}
	*/

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_BM_HumanGrunt::IsLightDamage( const CTakeDamageInfo &info )
{
	return BaseClass::IsLightDamage( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_BM_HumanGrunt::IsHeavyDamage( const CTakeDamageInfo &info )
{
	// Combine considers AR2 fire to be heavy damage
	if ( info.GetAmmoType() == GetAmmoDef()->Index("AR2") )
		return true;

	// 357 rounds are heavy damage
	if ( info.GetAmmoType() == GetAmmoDef()->Index("357") )
		return true;

	// Shotgun blasts where at least half the pellets hit me are heavy damage
	if ( info.GetDamageType() & DMG_BUCKSHOT )
	{
		int iHalfMax = sk_plr_dmg_buckshot.GetFloat() * sk_plr_num_shotgun_pellets.GetInt() * 0.5;
		if ( info.GetDamage() >= iHalfMax )
			return true;
	}

	// Rollermine shocks
	if( (info.GetDamageType() & DMG_SHOCK) && hl2_episodic.GetBool() )
	{
		return true;
	}

	return BaseClass::IsHeavyDamage( info );
}

//-----------------------------------------------------------------------------
// Purpose: Translate base class activities into combot activites
//-----------------------------------------------------------------------------
Activity CNPC_BM_HumanGrunt::NPC_TranslateActivity( Activity eNewActivity )
{
	// If the special ep2_outland_05 "use march" flag is set, use the more casual marching anim.
	if ( m_iUseMarch && eNewActivity == ACT_WALK )
	{
		eNewActivity = ACT_WALK_MARCH;
	}

	// Use HL1-like activities
	if ( eNewActivity == ACT_MELEE_ATTACK1 )
	{
		eNewActivity = ACT_MELEE_ATTACK2;
	}

	return BaseClass::NPC_TranslateActivity( eNewActivity );
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_bm_human_grunt, CNPC_BM_HumanGrunt )

	DECLARE_TASK( TASK_GRUNT_HEAL )
	DECLARE_TASK( TASK_GRUNT_HEAL_TOSS )

	DECLARE_CONDITION( COND_GRUNT_PLAYERHEALREQUEST )
	DECLARE_CONDITION( COND_GRUNT_COMMANDHEAL )

	
	//=========================================================
	// > SCHED_SCI_HEAL
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_GRUNT_HEAL,

		"	Tasks"
		"		TASK_GET_PATH_TO_TARGET				0"
		"		TASK_MOVE_TO_TARGET_RANGE			50"
		"		TASK_STOP_MOVING					0"
		"		TASK_FACE_IDEAL						0"
//		"		TASK_SAY_HEAL						0"
//		"		TASK_PLAY_SEQUENCE_FACE_TARGET		ACTIVITY:ACT_ARM"
		"		TASK_GRUNT_HEAL						0"
//		"		TASK_PLAY_SEQUENCE_FACE_TARGET		ACTIVITY:ACT_DISARM"
		"	"
		"	Interrupts"
	)

#if HL2_EPISODIC
	//=========================================================
	// > SCHED_GRUNT_HEAL_TOSS
	// this is for the episodic behavior where the human_grunt hurls the medkit
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_GRUNT_HEAL_TOSS,

	"	Tasks"
//  "		TASK_GET_PATH_TO_TARGET				0"
//  "		TASK_MOVE_TO_TARGET_RANGE			50"
	"		TASK_STOP_MOVING					0"
	"		TASK_FACE_IDEAL						0"
//	"		TASK_SAY_HEAL						0"
//	"		TASK_PLAY_SEQUENCE_FACE_TARGET		ACTIVITY:ACT_ARM"
	"		TASK_GRUNT_HEAL_TOSS				0"
//	"		TASK_PLAY_SEQUENCE_FACE_TARGET		ACTIVITY:ACT_DISARM"
	"	"
	"	Interrupts"
	)
#endif

 AI_END_CUSTOM_NPC()
