//==============================================================================
//
// Purpose: Security guards created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbase.h"
#include "npc_bm_human_security.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar	sk_human_security_health( "sk_human_security_health", "40" );

#define BODYGROUP_HELMET 2
#define BODYGROUP_CHEST 3
#define BODYGROUP_HOLSTER 4
#define BODYGROUP_FLASHLIGHT 5

LINK_ENTITY_TO_CLASS( npc_bm_human_security, CNPC_BM_HumanSecurity );
LINK_ENTITY_TO_CLASS( npc_human_security, CNPC_BM_HumanSecurity ); // For simplicity/ease of use/legacy support/etc.

BEGIN_DATADESC( CNPC_BM_HumanSecurity )

	DEFINE_KEYFIELD( m_fWeaponDrawn, FIELD_BOOLEAN, "weapondrawn" ),
	DECLARE_BM_NPC_DATADESC()
	DECLARE_BM_HUMAN_DATADESC()

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CNPC_BM_HumanSecurity, DT_NPC_BM_HumanSecurity )
	SendPropInt( SENDINFO( m_iCharacterIndex ), 16, 0 ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CNPC_BM_HumanSecurity::CNPC_BM_HumanSecurity()
{
	m_fWeaponDrawn = false;
}

//=========================================================
// Classify - indicates this NPC's place in the 
// relationship table.
//=========================================================
Class_T	CNPC_BM_HumanSecurity::Classify( void )
{
	return	CLASS_PLAYER_ALLY;
}

//=========================================================
// Spawn
//=========================================================
void CNPC_BM_HumanSecurity::Spawn()
{
	BaseClass::Spawn();

	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );

	m_iHealth = sk_human_security_health.GetFloat();
	m_iMaxHealth = sk_human_security_health.GetFloat();

	if (!m_bCustomBody)
	{
		SelectAndApplyCharacter();
	}

	if (GetActiveWeapon() && GetActiveWeapon()->WeaponClassify() == WEPCLASS_HANDGUN)
	{
		if (!m_fWeaponDrawn)
		{
			DoHolster();
		}
		else if (GetBodygroup(BODYGROUP_HOLSTER) != 0)
		{
			// Empty holster
			SetBodygroup( BODYGROUP_HOLSTER, 2 );
		}
	}

	NPCInit();
}

//=========================================================
// Precache - precaches all resources this NPC needs
//=========================================================
void CNPC_BM_HumanSecurity::Precache()
{
	if (!GetModelName())
	{
		SetModelName( MAKE_STRING( "models/humans/guard.mdl" ) );
	}

	if (!FStrEq( GetClassname(), "npc_bm_human_security" ))
		SetClassname( "npc_bm_human_security" );

	PrecacheModel( STRING( GetModelName() ) );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Allows NPC to holster from more than just the animation event
//-----------------------------------------------------------------------------
bool CNPC_BM_HumanSecurity::DoHolster( void )
{
	if (GetActiveWeapon() && IsGlock( GetActiveWeapon() ) && GetBodygroup( BODYGROUP_HOLSTER ) == 2)
	{
		// Toggle our holster bodygroup
		SetBodygroup( BODYGROUP_HOLSTER, 1 );
	}

	return BaseClass::DoHolster();
}

//-----------------------------------------------------------------------------
// Purpose: Allows NPC to unholster from more than just the animation event
//-----------------------------------------------------------------------------
bool CNPC_BM_HumanSecurity::DoUnholster( void )
{
	bool bBase = BaseClass::DoUnholster();

	if (bBase)
	{
		if (GetActiveWeapon() && IsGlock( GetActiveWeapon() ) && GetBodygroup( BODYGROUP_HOLSTER ) == 1)
		{
			// Toggle our holster bodygroup
			SetBodygroup( BODYGROUP_HOLSTER, 2 );
		}
	}

	return bBase;
}

//------------------------------------------------------------------------------
// Purpose: 
//------------------------------------------------------------------------------
WeaponProficiency_t CNPC_BM_HumanSecurity::CalcWeaponProficiency( CBaseCombatWeapon *pWeapon )
{
	// Only allied guards have perfect accuracy
	return IsPlayerAlly() ? WEAPON_PROFICIENCY_PERFECT : WEAPON_PROFICIENCY_VERY_GOOD;
}
