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

// Animation events
int AE_NPC_ITEM_ATTACH;
int AE_NPC_ITEM_REMOVE;
int AE_HEAL;
extern int AE_CITIZEN_HEAL;

// Activities
Activity ACT_HEAL;
extern int ACT_CIT_HEAL;

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

	DEFINE_INPUT( m_bAutosquad, FIELD_BOOLEAN, "SetAutosquad" ),

	DECLARE_BM_NPC_DATADESC()
	DECLARE_BM_HUMAN_DATADESC()

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CNPC_BM_HumanScientist, DT_NPC_BM_HumanScientist )
	SendPropInt( SENDINFO( m_iCharacterIndex ), 16, 0 ),
END_SEND_TABLE()

CNPC_BM_HumanScientist::CNPC_BM_HumanScientist()
{
	SetCitizenType( CT_UNIQUE );
	m_bDontPickupWeapons = true;
	m_bAutosquad = false;
}

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

	RemoveEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );

	m_iHealth = sk_human_scientist_health.GetFloat();
	m_iMaxHealth = sk_human_scientist_health.GetFloat();
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
			SetModelName( AllocPooledString( "models/humans/scientist_kleiner.mdl" ) );
			m_bCustomBody = true;
		}
		else if ( FStrEq( GetClassname(), "npc_human_scientist_eli" ) )
		{
			SetModelName( AllocPooledString( "models/humans/scientist_eli.mdl" ) );
			m_bCustomBody = true;
		}
		else
		{
			SetModelName( AllocPooledString( "models/humans/scientist.mdl" ) );
		}
	}

	if (!FStrEq( GetClassname(), "npc_bm_human_scientist" ))
		SetClassname( "npc_bm_human_scientist" );

	if (!m_bCustomBody)
	{
		SelectAndApplyCharacter();
	}

	PrecacheModel( STRING( GetModelName() ) );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_BM_HumanScientist::ModifyOrAppendCriteria( AI_CriteriaSet& set )
{
	BaseClass::ModifyOrAppendCriteria( set );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_BM_HumanScientist::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_HEAL )
	{
		pEvent->event = AE_CITIZEN_HEAL;
	}
	else if ( pEvent->event == AE_NPC_ITEM_ATTACH )
	{
		// TODO
		return;
	}
	else if ( pEvent->event == AE_NPC_ITEM_REMOVE )
	{
		// TODO
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
Activity CNPC_BM_HumanScientist::NPC_TranslateActivity( Activity eNewActivity )
{
	if (eNewActivity == (Activity)ACT_CIT_HEAL)
	{
		if (IsMedic() && !GetActiveWeapon())
			eNewActivity = ACT_HEAL;
	}

	return BaseClass::NPC_TranslateActivity( eNewActivity );
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_bm_human_scientist, CNPC_BM_HumanScientist )

	DECLARE_ANIMEVENT( AE_NPC_ITEM_ATTACH )
	DECLARE_ANIMEVENT( AE_NPC_ITEM_REMOVE )
	DECLARE_ANIMEVENT( AE_HEAL )

	DECLARE_ACTIVITY( ACT_HEAL )

 AI_END_CUSTOM_NPC()
