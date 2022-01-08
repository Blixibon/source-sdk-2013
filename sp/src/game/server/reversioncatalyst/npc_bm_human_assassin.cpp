//==============================================================================
//
// Purpose: Black Ops assassins created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbase.h"
#include "npc_bm_human_assassin.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	sk_human_assassin_health( "sk_human_assassin_health", "50" );
ConVar	sk_human_assassin_melee_damage( "sk_human_assassin_melee_damage", "10" );
ConVar	sk_human_assassin_jump_rise( "sk_human_assassin_jump_rise", "300" );
ConVar	sk_human_assassin_jump_distance( "sk_human_assassin_jump_distance", "400" );
ConVar	sk_human_assassin_jump_drop( "sk_human_assassin_jump_drop", "756" );
ConVar	sk_human_assassin_flip_hratio( "sk_human_assassin_flip_hratio", "256" );
ConVar	sk_human_assassin_decloak_damage( "sk_human_assassin_decloak_damage", "4" );
ConVar	sk_human_assassin_decloak_flip( "sk_human_assassin_decloak_flip", "3" );
ConVar	sk_human_assassin_decloak_melee( "sk_human_assassin_decloak_melee", "3" );
ConVar	sk_human_assassin_decloak_shoot( "sk_human_assassin_decloak_shoot", "0.5" );
ConVar	sk_human_assassin_force_cloak( "sk_human_assassin_force_cloak", "0" );

ConVar	sk_human_assassin_flip_cooldown( "sk_human_assassin_flip_cooldown", "10" );

// Animation events
int AE_PISTOL_FIRE_LEFT;
int AE_PISTOL_FIRE_RIGHT;
int AE_MELEE_ATTACK_HIGH;
int AE_MELEE_ATTACK_LOW;

// Activities
Activity ACT_ASSASSIN_FLIP;
Activity ACT_ASSASSIN_FLIP_PISTOL;
Activity ACT_ASSASSIN_FLIP_RIFLE;
Activity ACT_ASSASSIN_FLIP_PISTOL_JUMP;
Activity ACT_ASSASSIN_FLIP_PISTOL_GLIDE;
Activity ACT_ASSASSIN_FLIP_PISTOL_LAND;

// Interactions
extern int	g_interactionCombineBash		= 0; // melee bash attack

// Special stuff
extern void BM_AssassinizeGlock( CBaseEntity *pEntGlock, CBaseAnimating *pLeftGlock );

#define LEFT_HAND_GLOCK_MODEL "models/weapons/w_glock_lh.mdl"

#define FEMALE_ASSASSIN_MAX_RANGE	2048.0f //4096.0f

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_BM_HumanFemaleAssassin )

	DEFINE_FIELD( m_hLeftHandGun, FIELD_EHANDLE ),

	DEFINE_HUMAN_ASSASSIN_DATADESC()

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CNPC_BM_HumanFemaleAssassin, DT_NPC_BM_HumanFemaleAssassin )
	SendPropInt( SENDINFO( m_iCharacterIndex ), 16, 0 ),
	SendPropFloat( SENDINFO( m_flCloakFactor ), 16, 0, 0.0f, 1.0f ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( npc_bm_human_female_assassin, CNPC_BM_HumanFemaleAssassin );
LINK_ENTITY_TO_CLASS( npc_human_assassin, CNPC_BM_HumanFemaleAssassin ); // For simplicity/ease of use/legacy support/etc.

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNPC_BM_HumanFemaleAssassin::CNPC_BM_HumanFemaleAssassin()
{
	m_spawnEquipment = MAKE_STRING( "weapon_bm_glock" );
	m_flDistTooFar = FEMALE_ASSASSIN_MAX_RANGE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_BM_HumanFemaleAssassin::Spawn( void )
{
	Precache();
	SetModel( STRING( GetModelName() ) );

	SetHealth( sk_human_assassin_health.GetFloat() );
	SetMaxHealth( sk_human_assassin_health.GetFloat() );
	SetKickDamage( sk_human_assassin_melee_damage.GetFloat() );

	//if (!m_bCustomBody)
	//{
	//	SelectAndApplyCharacter();
	//}

	CapabilitiesAdd( bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD );
	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );
	CapabilitiesAdd( bits_CAP_DOORS_GROUP );
	CapabilitiesAdd( bits_CAP_MOVE_JUMP );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_BM_HumanFemaleAssassin::Precache()
{
	if( !GetModelName() )
	{
		SetModelName( MAKE_STRING( "models/humans/hassassin.mdl" ) );
	}

	if (!FStrEq( GetClassname(), "npc_bm_human_female_assassin" ))
		SetClassname( "npc_bm_human_female_assassin" );

	PrecacheScriptSound( "NPC_Assassin.jump" );
	PrecacheScriptSound( "NPC_Assassin.land" );

	PrecacheParticleSystem( "npc_assassin_eyeglow" );

	PrecacheModel( STRING( GetModelName() ) );

	PrecacheModel( LEFT_HAND_GLOCK_MODEL );

	UTIL_PrecacheOther( "npc_bm_grenade_frag" ); // weapon_bm_frag?

	BaseClass::Precache();
}

//=========================================================
// Classify - indicates this NPC's place in the 
// relationship table.
//=========================================================
Class_T	CNPC_BM_HumanFemaleAssassin::Classify( void )
{
	return	CLASS_MILITARY;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CNPC_BM_HumanFemaleAssassin::GetCharacterClassname()
{
	return "npc_human_female_assassin";
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
void CNPC_BM_HumanFemaleAssassin::Event_Killed( const CTakeDamageInfo &info )
{
	if (GetActiveWeapon() && GetActiveWeapon()->ClassMatches( "weapon_bm_glock" ))
	{
		// Make it not silenced and not dual-wielded
		variant_t varFalse;
		varFalse.SetBool( false );
		GetActiveWeapon()->AcceptInput( "SetSilenced", this, this, varFalse, 0 );
		GetActiveWeapon()->AcceptInput( "SetDualWield", this, this, variant_t(), 0 );
	}

	if (m_hLeftHandGun)
	{
		if (!HasSpawnFlags(SF_NPC_NO_WEAPON_DROP))
		{
			// Drop the fake gun
			CBaseEntity *pRealGun = CreateNoSpawn( "weapon_bm_glock", m_hLeftHandGun->GetAbsOrigin(), m_hLeftHandGun->GetAbsAngles(), this );
			DispatchSpawn( pRealGun );

			if ( info.GetDamageType() & DMG_DISSOLVE )
			{
				CBaseAnimating *pAnimating = static_cast<CBaseAnimating*>(pRealGun);
				if( pAnimating )
				{
					pAnimating->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
				}
			}
		}

		UTIL_Remove( m_hLeftHandGun );
	}

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
// Update weapon ranges
//-----------------------------------------------------------------------------
void CNPC_BM_HumanFemaleAssassin::Weapon_HandleEquip( CBaseCombatWeapon *pWeapon )
{
	BaseClass::Weapon_HandleEquip( pWeapon );

	if (pWeapon && pWeapon->ClassMatches( "weapon_bm_glock" ))
	{
		// Create a fake second pistol
		CBaseEntity *pEnt = CBaseEntity::CreateNoSpawn( "prop_dynamic_override", this->GetLocalOrigin(), this->GetLocalAngles(), this );
		if (pEnt)
		{
			pEnt->SetModelName( AllocPooledString( LEFT_HAND_GLOCK_MODEL ) );
			DispatchSpawn( pEnt );
			pEnt->FollowEntity( this, true );
		}

		m_hLeftHandGun = static_cast<CBaseAnimating *>(pEnt);

		// Make it silenced and dual-wielded
		BM_AssassinizeGlock( pWeapon, m_hLeftHandGun );

		// Expand range
		pWeapon->m_fMaxRange1 = FEMALE_ASSASSIN_MAX_RANGE;
		pWeapon->m_fMaxRange2 = FEMALE_ASSASSIN_MAX_RANGE;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_BM_HumanFemaleAssassin::KickAttack( bool bLow )
{
	// Does no damage, because damage is applied based upon whether the target can handle the interaction
	CBaseEntity *pHurt = CheckTraceHullAttack( 70, -Vector(16,16,18), Vector(16,16,18), 0, DMG_CLUB );
	CBaseCombatCharacter* pBCC = ToBaseCombatCharacter( pHurt );
	if (pBCC)
	{
		Vector forward, up;
		AngleVectors( GetLocalAngles(), &forward, NULL, &up );

		if ( !pBCC->DispatchInteraction( g_interactionCombineBash, NULL, this ) )
		{
			if ( pBCC->IsPlayer() )
			{
				pBCC->ViewPunch( bLow ? QAngle(12,7,0) : QAngle(-12,-7,0) );
				pHurt->ApplyAbsVelocityImpulse( forward * 100 + up * 50 );
			}

			CTakeDamageInfo info( this, this, sk_human_assassin_melee_damage.GetFloat(), DMG_CLUB ); // TODO: m_nKickDamage?
			CalculateMeleeDamageForce( &info, forward, pBCC->GetAbsOrigin() );
			pBCC->TakeDamage( info );

			EmitSound( "NPC_Combine.WeaponBash" );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNPC_BM_HumanFemaleAssassin::ShouldFlip()
{
	// Don't flip again for a while
	if (gpGlobals->curtime - m_flLastFlipEndTime <= sk_human_assassin_flip_cooldown.GetFloat())
		return false;

	// If we're taking cover while the enemy is facing me, start flipping
	if (this->HasCondition( COND_ENEMY_FACING_ME ) && this->IsCurSchedule( SCHED_COMBINE_TAKE_COVER1, false ))
		return true;

	// If we've recently taken damage, start flipping
	if (gpGlobals->curtime - GetLastDamageTime() <= 3.0f)
		return true;

	return BaseClass::ShouldFlip();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
WeaponProficiency_t CNPC_BM_HumanFemaleAssassin::CalcWeaponProficiency( CBaseCombatWeapon *pWeapon )
{
	if ( pWeapon->ClassMatches( gm_isz_class_Pistol ) || pWeapon->ClassMatches( "weapon_bm_glock" ) )
	{
		return WEAPON_PROFICIENCY_PERFECT;
	}

	return BaseClass::CalcWeaponProficiency( pWeapon );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_BM_HumanFemaleAssassin::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_PISTOL_FIRE_LEFT )
	{
		if (m_hLeftHandGun)
		{
			int sequence = m_hLeftHandGun->SelectWeightedSequence( ACT_RANGE_ATTACK1 );
			if (sequence != ACTIVITY_NOT_AVAILABLE)
			{
				m_hLeftHandGun->SetSequence( sequence );
				m_hLeftHandGun->SetCycle( 0 );
				m_hLeftHandGun->ResetSequenceInfo();
			}
		}
		return;
	}
	else if ( pEvent->event == AE_PISTOL_FIRE_RIGHT )
	{
		//pEvent->event = EVENT_WEAPON_PISTOL_FIRE;
		//BaseClass::HandleAnimEvent( pEvent );
		return;
	}
	else if ( pEvent->event == AE_MELEE_ATTACK_HIGH )
	{
		KickAttack( false );
		SpeakIfAllowed( TLK_CMB_KICK );
		return;
	}
	else if ( pEvent->event == AE_MELEE_ATTACK_LOW )
	{
		KickAttack( true );
		SpeakIfAllowed( TLK_CMB_KICK );
		return;
	}

	//switch( pEvent->event )
	//{
	//default:
		BaseClass::HandleAnimEvent( pEvent );
	//	break;
	//}
}

//-----------------------------------------------------------------------------
// Purpose: Translate base class activities into combot activites
//-----------------------------------------------------------------------------
Activity CNPC_BM_HumanFemaleAssassin::NPC_TranslateActivity( Activity eNewActivity )
{
	switch (eNewActivity)
	{
		case ACT_JUMP:
			eNewActivity = ACT_ASSASSIN_FLIP_PISTOL_JUMP;
			break;
		case ACT_GLIDE:
			eNewActivity = ACT_ASSASSIN_FLIP_PISTOL_GLIDE;
			break;
		case ACT_LAND:
			eNewActivity = ACT_ASSASSIN_FLIP_PISTOL_LAND;
			break;
	}

	if (eNewActivity == ACT_MELEE_ATTACK1)
	{
		if (GetEnemy())
		{
			// Use low kick for low enemies
			CAI_BaseNPC *pNPC = GetEnemy()->MyNPCPointer();
			if (pNPC && pNPC->GetHullHeight() < 72.0f)
				eNewActivity = ACT_MELEE_ATTACK2;
		}
	}

	return BaseClass::NPC_TranslateActivity( eNewActivity );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_BM_HumanFemaleAssassin::OnChangeActivity( Activity eNewActivity )
{
	BaseClass::OnChangeActivity( eNewActivity );

	switch (eNewActivity)
	{
		case ACT_JUMP:
			EmitSound( "NPC_Assassin.jump" );
			break;

		case ACT_LAND:
			EmitSound( "NPC_Assassin.land" );
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_BM_HumanFemaleAssassin::PrescheduleThink( void )
{
	if ( GameRules()->IsSkillLevel( SKILL_HARD ) || sk_human_assassin_force_cloak.GetBool() )
	{
		float flDest = 1.0f;
		float flSpeed = 0.01f;

		// Damage
		if (gpGlobals->curtime - GetLastDamageTime() <= 2.0f)
		{
			flDest *= sk_human_assassin_decloak_damage.GetFloat() / 10.0f;
			flSpeed = 0.02f;
		}
		// Flip
		if (GetActivity() == ACT_ASSASSIN_FLIP || GetActivity() == ACT_ASSASSIN_FLIP_PISTOL)
		{
			flDest *= sk_human_assassin_decloak_flip.GetFloat() / 10.0f;
		}
		// Melee
		if (GetActivity() == ACT_MELEE_ATTACK1 || GetActivity() == ACT_MELEE_ATTACK2)
		{
			flDest *= sk_human_assassin_decloak_melee.GetFloat() / 10.0f;
			flSpeed = 0.05f;
		}
		// Shoot
		if (gpGlobals->curtime - GetLastAttackTime() <= 0.5f)
		{
			flDest *= sk_human_assassin_decloak_melee.GetFloat() / 10.0f;
		}

		if (flDest != m_flCloakFactor)
			m_flCloakFactor = UTIL_Approach( flDest, m_flCloakFactor, flSpeed );
	}
	else
	{
		m_flCloakFactor = 0.0f;
	}

	BaseClass::PrescheduleThink();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CNPC_BM_HumanFemaleAssassin::TranslateSchedule( int scheduleType )
{
	scheduleType = BaseClass::TranslateSchedule( scheduleType );

	switch( scheduleType )
	{
	case SCHED_COMBINE_ESTABLISH_LINE_OF_FIRE:
	case SCHED_COMBINE_RANGE_ATTACK1:
		{
			// Don't stop to attack
			if (gpGlobals->curtime - GetEnemies()->LastTimeSeen(GetEnemy()) < 15.0f)
			{
				if (HasMemory( bits_MEMORY_INCOVER ))
				{
					return SCHED_HUMAN_ASSASSIN_FLANK_RANDOM;
				}
				else
				{
					return SCHED_HUMAN_ASSASSIN_EVADE;
				}
			}
		} break;

	case SCHED_COMBINE_SIGNAL_SUPPRESS:
		{
			// Don't do signals
			return SCHED_COMBINE_SUPPRESS;
		} break;

	case SCHED_COWER:
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		{
			// Avoid falling for grenades
			if (GetEnemy())
				return SCHED_HUMAN_ASSASSIN_EVADE;
		} break;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNPC_BM_HumanFemaleAssassin::SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode )
{
	// Catch the LOF failure and choose another route to take
	if ( failedSchedule == SCHED_ESTABLISH_LINE_OF_FIRE )
		return SCHED_HUMAN_ASSASSIN_FLANK_RANDOM;

	return BaseClass::SelectFailSchedule( failedSchedule, failedTask, taskFailCode );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_BM_HumanFemaleAssassin::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_COMBINE_SIGNAL_BEST_SOUND:
		// Don't do signals
		TaskComplete();
		break;

	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if a reasonable jumping distance
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CNPC_BM_HumanFemaleAssassin::IsJumpLegal(const Vector &startPos, const Vector &apex, const Vector &endPos) const
{
	const float MAX_JUMP_RISE		= sk_human_assassin_jump_rise.GetFloat();
	const float MAX_JUMP_DISTANCE	= sk_human_assassin_jump_distance.GetFloat();
	const float MAX_JUMP_DROP		= sk_human_assassin_jump_drop.GetFloat();

	return BaseClass::IsJumpLegal( startPos, apex, endPos, MAX_JUMP_RISE, MAX_JUMP_DROP, MAX_JUMP_DISTANCE );
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_bm_human_female_assassin, CNPC_BM_HumanFemaleAssassin )

	DECLARE_ANIMEVENT( AE_PISTOL_FIRE_LEFT )
	DECLARE_ANIMEVENT( AE_PISTOL_FIRE_RIGHT )
	DECLARE_ANIMEVENT( AE_MELEE_ATTACK_HIGH )
	DECLARE_ANIMEVENT( AE_MELEE_ATTACK_LOW )

	DECLARE_ACTIVITY( ACT_ASSASSIN_FLIP )
	DECLARE_ACTIVITY( ACT_ASSASSIN_FLIP_PISTOL )
	DECLARE_ACTIVITY( ACT_ASSASSIN_FLIP_RIFLE )
	DECLARE_ACTIVITY( ACT_ASSASSIN_FLIP_PISTOL_JUMP )
	DECLARE_ACTIVITY( ACT_ASSASSIN_FLIP_PISTOL_GLIDE )
	DECLARE_ACTIVITY( ACT_ASSASSIN_FLIP_PISTOL_LAND )

	DEFINE_SCHEDULE
	(
		SCHED_HUMAN_ASSASSIN_RANGE_ATTACK1,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_FACE_ENEMY					0"
		"		TASK_ANNOUNCE_ATTACK			1"	// 1 = primary attack
		"		TASK_WAIT_RANDOM				0.3"
		"		TASK_RANGE_ATTACK1				0"
		"		TASK_COMBINE_IGNORE_ATTACKS		0.5"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_HEAVY_DAMAGE"
		"		COND_LIGHT_DAMAGE"
		"		COND_LOW_PRIMARY_AMMO"
		"		COND_NO_PRIMARY_AMMO"
		"		COND_WEAPON_BLOCKED_BY_FRIEND"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_GIVE_WAY"
		"		COND_HEAR_DANGER"
		"		COND_HEAR_MOVE_AWAY"
		"		COND_COMBINE_NO_FIRE"
		"		COND_ENEMY_FACING_ME" // Assassins don't like it when you see them
		""
		// Enemy_Occluded				Don't interrupt on this.  Means
		//								comibine will fire where player was after
		//								he has moved for a little while.  Good effect!!
		// WEAPON_SIGHT_OCCLUDED		Don't block on this! Looks better for railings, etc.
	)

	DEFINE_SCHEDULE
	(
		SCHED_HUMAN_ASSASSIN_EVADE,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_HUMAN_ASSASSIN_FLANK_RANDOM"
		//"		TASK_STOP_MOVING				0"
		//"		TASK_WAIT						0.2"
	//	"		TASK_SET_TOLERANCE_DISTANCE		24"
		"		TASK_FIND_FAR_NODE_COVER_FROM_ENEMY		250"
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_REMEMBER					MEMORY:INCOVER"
		//"		TASK_FACE_ENEMY					0"
		//"		TASK_SET_ACTIVITY				ACTIVITY:ACT_IDLE"	// Translated to cover
		//"		TASK_WAIT						1"
		""
		"	Interrupts"
		//"		COND_NEW_ENEMY"
		"		COND_HEAR_DANGER"
		"		COND_CAN_MELEE_ATTACK1"
	)

	DEFINE_SCHEDULE
	(
		SCHED_HUMAN_ASSASSIN_FLANK_RANDOM,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE					SCHEDULE:SCHED_HUMAN_ASSASSIN_RANGE_ATTACK1"
		"		TASK_SET_TOLERANCE_DISTANCE				256"
		"		TASK_SET_ROUTE_SEARCH_TIME				1"	// Spend 1 second trying to build a path if stuck
		"		TASK_GET_FLANK_ARC_PATH_TO_ENEMY_LOS	80"
		"		TASK_RUN_PATH							0"
		"		TASK_WAIT_FOR_MOVEMENT					0"
		""
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_HEAVY_DAMAGE"
		//"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK1"
	)

 AI_END_CUSTOM_NPC()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
	 
ConVar	sk_human_male_assassin_health( "sk_human_male_assassin_health", "50" );
ConVar	sk_human_male_assassin_kick( "sk_human_male_assassin_kick", "10" );
ConVar	sk_human_male_assassin_speedmod( "sk_human_male_assassin_speedmod", "1.1" );

ConVar	sk_human_male_assassin_flip_cooldown( "sk_human_male_assassin_flip_cooldown", "20" );

LINK_ENTITY_TO_CLASS( npc_bm_human_male_assassin, CNPC_BM_HumanMaleAssassin );
LINK_ENTITY_TO_CLASS( npc_bm_human_male_assassin_medic, CNPC_BM_HumanMaleAssassin ); // For simplicity/ease of use/legacy support/etc.
LINK_ENTITY_TO_CLASS( npc_bm_human_male_assassin_commander, CNPC_BM_HumanMaleAssassin ); // For simplicity/ease of use/legacy support/etc.

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_BM_HumanMaleAssassin )

	DEFINE_HUMAN_ASSASSIN_DATADESC()

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CNPC_BM_HumanMaleAssassin, DT_NPC_BM_HumanMaleAssassin )
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNPC_BM_HumanMaleAssassin::CNPC_BM_HumanMaleAssassin()
{
	SetAlternateCapable( true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_BM_HumanMaleAssassin::Spawn( void )
{
	BaseClass::Spawn();

	CapabilitiesAdd( bits_CAP_MOVE_JUMP );

	SetHealth( sk_human_male_assassin_health.GetFloat() );
	SetMaxHealth( sk_human_male_assassin_health.GetFloat() );
	SetKickDamage( sk_human_male_assassin_kick.GetFloat() );
	
	if (GetActiveWeapon() && GetActiveWeapon()->ClassMatches( "weapon_bm_glock" ))
	{
		// Make it silenced
		BM_AssassinizeGlock( GetActiveWeapon(), NULL );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_BM_HumanMaleAssassin::Precache()
{
	if( !GetModelName() )
	{
		SetModelName( MAKE_STRING( "models/humans/hmassassin.mdl" ) );
	}

	if (FStrEq( GetClassname(), "npc_bm_human_male_assassin_medic" ))
		m_SoldierType = ST_MEDIC;
	else if (FStrEq( GetClassname(), "npc_bm_human_male_assassin_commander" ))
		m_SoldierType = ST_COMMANDER;

	if (!FStrEq( GetClassname(), "npc_bm_human_male_assassin" ))
		SetClassname( "npc_bm_human_male_assassin" );

	PrecacheScriptSound( "NPC_Assassin.jump" );
	PrecacheScriptSound( "NPC_Assassin.land" );

	PrecacheParticleSystem( "npc_assassin_male_eyeglow" );

	BaseClass::Precache();
}

//=========================================================
// Classify - indicates this NPC's place in the 
// relationship table.
//=========================================================
Class_T	CNPC_BM_HumanMaleAssassin::Classify( void )
{
	return	CLASS_MILITARY;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CNPC_BM_HumanMaleAssassin::GetCharacterClassname()
{
	switch (m_SoldierType)
	{
	default:
	case ST_GRUNT:
		return "npc_human_male_assassin";

	case ST_MEDIC:
		return "npc_human_male_assassin_medic";

	case ST_COMMANDER:
		return "npc_human_male_assassin_commander";
	}
}

//-----------------------------------------------------------------------------
// Purpose: Translate base class activities into combot activites
//-----------------------------------------------------------------------------
Activity CNPC_BM_HumanMaleAssassin::NPC_TranslateActivity( Activity eNewActivity )
{
	return BaseClass::NPC_TranslateActivity( eNewActivity );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_BM_HumanMaleAssassin::OnChangeActivity( Activity eNewActivity )
{
	BaseClass::OnChangeActivity( eNewActivity );

	switch (eNewActivity)
	{
		case ACT_JUMP:
			EmitSound( "NPC_Assassin.jump" );
			break;

		case ACT_LAND:
			EmitSound( "NPC_Assassin.land" );
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNPC_BM_HumanMaleAssassin::ShouldFlip()
{
	// Don't flip again for a while
	if (gpGlobals->curtime - m_flLastFlipEndTime <= sk_human_male_assassin_flip_cooldown.GetFloat())
		return false;

	// If we've recently taken damage and we can't attack (or we have low health), start flipping if the enemy sees us
	if (HasCondition(COND_SEE_ENEMY) && (!HasCondition(COND_CAN_RANGE_ATTACK1) || (GetHealth() / GetMaxHealth()) < 0.5f) && gpGlobals->curtime - GetLastDamageTime() <= 3.0f)
		return true;

	return BaseClass::ShouldFlip();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
inline bool CNPC_BM_HumanMaleAssassin::ShouldHaveSpeedBoost( void )
{
	return GetState() == NPC_STATE_COMBAT;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_BM_HumanMaleAssassin::PrescheduleThink( void )
{
	if (ShouldHaveSpeedBoost() && IsMoving())
		m_flPlaybackRate = sk_human_male_assassin_speedmod.GetFloat();
	else
		m_flPlaybackRate = 1.0f;

	BaseClass::PrescheduleThink();
}

//-----------------------------------------------------------------------------
// Purpose: Get movement speed, multipled by modifier
//-----------------------------------------------------------------------------
float CNPC_BM_HumanMaleAssassin::GetSequenceGroundSpeed( CStudioHdr *pStudioHdr, int iSequence )
{
	float t = SequenceDuration( pStudioHdr, iSequence );

	if (t > 0)
	{
		float flMod = m_flSpeedModifier;

		if (ShouldHaveSpeedBoost())
			flMod *= sk_human_male_assassin_speedmod.GetFloat();

		return (GetSequenceMoveDist( pStudioHdr, iSequence ) * flMod / t);
	}
	else
	{
		return 0;
	}
}
