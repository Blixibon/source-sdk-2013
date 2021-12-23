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

// Animation events
int AE_PISTOL_FIRE_LEFT;
int AE_PISTOL_FIRE_RIGHT;

// Activities
Activity ACT_ASSASSIN_FLIP;
Activity ACT_ASSASSIN_FLIP_PISTOL;

#define LEFT_HAND_GLOCK_MODEL "models/weapons/w_glock_lh.mdl"

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_BM_HumanFemaleAssassin )

	DEFINE_FIELD( m_hLeftHandGun, FIELD_EHANDLE ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CNPC_BM_HumanFemaleAssassin, DT_NPC_BM_HumanFemaleAssassin )
	SendPropInt( SENDINFO( m_iCharacterIndex ), 16, 0 ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( npc_bm_human_female_assassin, CNPC_BM_HumanFemaleAssassin );
LINK_ENTITY_TO_CLASS( npc_human_assassin, CNPC_BM_HumanFemaleAssassin ); // For simplicity/ease of use/legacy support/etc.

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNPC_BM_HumanFemaleAssassin::CNPC_BM_HumanFemaleAssassin()
{
	m_spawnEquipment = MAKE_STRING( "weapon_bm_glock" );
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

	BaseClass::Spawn();

	if (GetActiveWeapon() && GetActiveWeapon()->ClassMatches( "weapon_bm_glock" ))
	{
		// Make it silenced and dual-wielded
		variant_t varTrue;
		varTrue.SetBool( true );
		GetActiveWeapon()->AcceptInput( "SetSilenced", this, this, varTrue, 0 );
		GetActiveWeapon()->AcceptInput( "SetDualWield", this, this, varTrue, 0 );

		// Create a fake second pistol
		CBaseEntity *pEnt = CBaseEntity::CreateNoSpawn( "prop_dynamic_override", this->GetLocalOrigin(), this->GetLocalAngles(), this );
		if (pEnt)
		{
			pEnt->SetModelName( AllocPooledString( LEFT_HAND_GLOCK_MODEL ) );
			DispatchSpawn( pEnt );
			pEnt->FollowEntity( this, true );
		}

		m_hLeftHandGun = static_cast<CBaseAnimating*>(pEnt);
	}
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
		GetActiveWeapon()->AcceptInput( "SetDualWield", this, this, varFalse, 0 );
	}

	if (m_hLeftHandGun)
	{
		if (!HasSpawnFlags(SF_NPC_NO_WEAPON_DROP))
		{
			// Drop the fake gun
			CBaseEntity *pRealGun = CreateNoSpawn( "weapon_bm_glock", m_hLeftHandGun->GetAbsOrigin(), m_hLeftHandGun->GetAbsAngles(), this );
			DispatchSpawn( pRealGun );
		}

		UTIL_Remove( m_hLeftHandGun );
	}

	BaseClass::Event_Killed( info );
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
	if (eNewActivity == ACT_RUN)
	{
		//if (IsStrategySlotRangeOccupied( SQUAD_SLOT_ATTACK1, SQUAD_SLOT_ATTACK2 ))

		if (gpGlobals->curtime - GetLastDamageTime() <= 3.0f)
		{
			eNewActivity = ACT_ASSASSIN_FLIP_PISTOL; // TODO
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
	BaseClass::PrescheduleThink();
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_bm_human_female_assassin, CNPC_BM_HumanFemaleAssassin )

	DECLARE_ANIMEVENT( AE_PISTOL_FIRE_LEFT )
	DECLARE_ANIMEVENT( AE_PISTOL_FIRE_RIGHT )

	DECLARE_ACTIVITY( ACT_ASSASSIN_FLIP )

 AI_END_CUSTOM_NPC()

ConVar	sk_human_male_assassin_speedmod( "sk_human_male_assassin_speedmod", "1.5" );

LINK_ENTITY_TO_CLASS( npc_bm_human_male_assassin, CNPC_BM_HumanMaleAssassin );
LINK_ENTITY_TO_CLASS( npc_bm_human_male_assassin_medic, CNPC_BM_HumanMaleAssassin ); // For simplicity/ease of use/legacy support/etc.
LINK_ENTITY_TO_CLASS( npc_bm_human_male_assassin_commander, CNPC_BM_HumanMaleAssassin ); // For simplicity/ease of use/legacy support/etc.

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_BM_HumanMaleAssassin )
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

	PrecacheParticleSystem( "npc_assassin_eyeglow" );

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
