//==============================================================================
//
// Purpose: Scientists created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbase.h"
#include "npc_bm_human_scientist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar	sk_human_scientist_health( "sk_human_scientist_health", "35" );

#define BODYGROUP_MALE_GLASSES 2
#define BODYGROUP_MALE_SYRINGE 3

#define BODYGROUP_FEMALE_HAIR 3
#define BODYGROUP_FEMALE_GLASSES 4
#define BODYGROUP_FEMALE_SYRINGE 5

LINK_ENTITY_TO_CLASS( npc_bm_human_scientist, CNPC_BM_HumanScientist );
LINK_ENTITY_TO_CLASS( npc_human_scientist, CNPC_BM_HumanScientist ); // For simplicity/ease of use/legacy support/etc.
LINK_ENTITY_TO_CLASS( npc_human_scientist_female, CNPC_BM_HumanScientist ); // For simplicity/ease of use/legacy support/etc.
LINK_ENTITY_TO_CLASS( npc_human_scientist_kleiner, CNPC_BM_HumanScientist ); // For simplicity/ease of use/legacy support/etc.
LINK_ENTITY_TO_CLASS( npc_human_scientist_eli, CNPC_BM_HumanScientist ); // For simplicity/ease of use/legacy support/etc.

BEGIN_DATADESC( CNPC_BM_HumanScientist )

	DECLARE_BM_NPC_DATADESC()

END_DATADESC()

//=========================================================
// Classify - indicates this NPC's place in the 
// relationship table.
//=========================================================
Class_T	CNPC_BM_HumanScientist::Classify( void )
{
	return	CLASS_PLAYER_ALLY;
}

//=========================================================
// Spawn
//=========================================================
void CNPC_BM_HumanScientist::Spawn()
{
	BaseClass::Spawn();

	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );

	m_iHealth = sk_human_scientist_health.GetFloat();
	m_iMaxHealth = sk_human_scientist_health.GetFloat();

	if (!m_bCustomBody)
	{
		if (GetModelPtr())
			m_nSkin = RandomInt(0, GetModelPtr()->numskinfamilies());

		if (Q_strstr(STRING(GetModelName()), "female"))
		{
			// Use any type of hair
			SetBodygroup( BODYGROUP_FEMALE_HAIR, RandomInt(0,4) );

			// Half of all scientists wear glasses
			// (one of them is a pencil, but still)
			if (RandomInt(0, 1) == 1)
				SetBodygroup( BODYGROUP_FEMALE_GLASSES, RandomInt(1,6) );
		}
		else
		{
			// Half of all scientists wear glasses
			// (one of them is a pencil, but still)
			if (RandomInt(0, 1) == 1)
				SetBodygroup( BODYGROUP_MALE_GLASSES, RandomInt(1,6) );
		}
	}

	NPCInit();
}

//=========================================================
// Precache - precaches all resources this NPC needs
//=========================================================
void CNPC_BM_HumanScientist::Precache()
{
	if ( GetModelName() == NULL_STRING )
	{
		if ( FStrEq( GetClassname(), "npc_human_scientist_female" ) )
		{
			SetModelName( AllocPooledString( "models/humans/scientist_female.mdl" ) );
		}
		else if ( FStrEq( GetClassname(), "npc_human_scientist_kleiner" ) )
		{
			SetModelName( AllocPooledString( "models/humans/scientist_kliener.mdl" ) );
		}
		else if ( FStrEq( GetClassname(), "npc_human_scientist_eli" ) )
		{
			SetModelName( AllocPooledString( "models/humans/scientist_eli.mdl" ) );
		}
		else
		{
			SetModelName( AllocPooledString( "models/humans/scientist.mdl" ) );
		}
	}

	if (!FStrEq( GetClassname(), "npc_bm_human_scientist" ))
		SetClassname( "npc_bm_human_scientist" );

	PrecacheModel( STRING( GetModelName() ) );

	BaseClass::Precache();
}
