//==============================================================================
//
// Purpose: Scientists created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbase.h"
#include "npc_bm_human_scientist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ISoundEmitterSystemBase *soundemitterbase;

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
	DECLARE_BM_HUMAN_DATADESC()

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CNPC_BM_HumanScientist, DT_NPC_BM_HumanScientist )
	SendPropInt( SENDINFO( m_iCharacterIndex ), 16, 0 ),
END_SEND_TABLE()

//=========================================================
// Classify - indicates this NPC's place in the 
// relationship table.
//=========================================================
Class_T	CNPC_BM_HumanScientist::Classify( void )
{
	return	CLASS_PLAYER_ALLY;
}

const char *CNPC_BM_HumanScientist::GetCharacterClassname()
{
	gender_t gender = soundemitterbase->GetActorGender( STRING( GetModelName() ) );
	
	return gender == GENDER_FEMALE ? "npc_human_scientist_female" : "npc_human_scientist";
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
		SelectAndApplyCharacter();
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
