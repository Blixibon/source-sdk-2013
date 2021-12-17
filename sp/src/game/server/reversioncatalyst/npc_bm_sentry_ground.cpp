//==============================================================================
//
// Purpose: Scientists created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbase.h"
#include "npc_bm_sentry_ground.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define SENTRY_FLOOR_MODEL "models/NPCs/sentry_ground.mdl"

LINK_ENTITY_TO_CLASS( npc_bm_sentry_floor, CNPC_BM_FloorSentry );
LINK_ENTITY_TO_CLASS( npc_sentry_ground, CNPC_BM_FloorSentry ); // For simplicity/ease of use/legacy support/etc.

BEGIN_DATADESC( CNPC_BM_FloorSentry )

	DECLARE_BM_NPC_DATADESC()

END_DATADESC()

//=========================================================
// Classify - indicates this NPC's place in the 
// relationship table.
//=========================================================
Class_T	CNPC_BM_FloorSentry::Classify( void )
{
	return CLASS_MILITARY;
}

//=========================================================
// Spawn
//=========================================================
void CNPC_BM_FloorSentry::Spawn()
{
	BaseClass::Spawn();

	m_takedamage = DAMAGE_YES;
	SetHealth( 50 ); // test
}

//=========================================================
// Precache - precaches all resources this NPC needs
//=========================================================
void CNPC_BM_FloorSentry::Precache()
{
	if (GetModelName() == NULL_STRING)
		SetModelName( AllocPooledString( SENTRY_FLOOR_MODEL ) );

	BaseClass::Precache();
}
