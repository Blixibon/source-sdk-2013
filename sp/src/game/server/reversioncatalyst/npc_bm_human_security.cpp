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
#define BODYGROUP_HOLSTER 2
#define BODYGROUP_FLASHLIGHT 2

LINK_ENTITY_TO_CLASS( npc_bm_human_security, CNPC_BM_HumanSecurity );
LINK_ENTITY_TO_CLASS( npc_human_security, CNPC_BM_HumanSecurity ); // For simplicity/ease of use/legacy support/etc.

BEGIN_DATADESC( CNPC_BM_HumanSecurity )

	DECLARE_BM_NPC_DATADESC()

END_DATADESC()

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
		if (GetModelPtr())
			m_nSkin = RandomInt(0, GetModelPtr()->numskinfamilies());

		// Always wear a helmet and a vest
		SetBodygroup( BODYGROUP_HELMET, 0 );
		SetBodygroup( BODYGROUP_CHEST, RandomInt(1,2) );
		SetBodygroup( BODYGROUP_FLASHLIGHT, RandomInt(0,1) );

		// Looks weird without this
		SetBodygroup( BODYGROUP_HOLSTER, RandomInt(1,2) );
	}

	if (GetActiveWeapon() && GetActiveWeapon()->WeaponClassify() == WEPCLASS_HANDGUN)
	{
		// Empty holster
		SetBodygroup( BODYGROUP_HOLSTER, 2 );
	}
	else if (!m_bCustomBody)
	{
		// Pretend we have a pistol holstered
		// todo: actual holster/unholster handling
		SetBodygroup( BODYGROUP_HOLSTER, 1 );
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
