//=============================================================================//
//
// Purpose: 	Bloody Cop.
//
//=============================================================================//

#include "cbase.h"
#include "npc_red.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "activitylist.h"
#include "engine/IEngineSound.h"
#include "sceneentity.h"
#include "ai_behavior_functank.h"
#include "npc_citizen17.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define RED_MODEL "models/bloody_cop.mdl"

ConVar	sk_red_health( "sk_red_health","0");

// Convars from npc_citizen17 but ported to Red
ConVar  sk_red_ar2_proficiency("sk_red_ar2_proficiency", "2"); // Added by 1upD. Skill rating 0 - 4 of how accurate the AR2 should be
ConVar  sk_red_default_proficiency("sk_red_default_proficiency", "1"); // Added by 1upD. Skill rating 0 - 4 of how accurate all weapons but the AR2 should be

LINK_ENTITY_TO_CLASS( npc_red, CNPC_Red );

//---------------------------------------------------------
// 
//---------------------------------------------------------
IMPLEMENT_SERVERCLASS_ST(CNPC_Red, DT_NPC_Red)
END_SEND_TABLE()


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_Red )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Red::SelectModel()
{
#ifdef MAPBASE
	if (GetModelName() == NULL_STRING)
#endif
	SetModelName( AllocPooledString( RED_MODEL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Red::SelectHealth()
{
#ifdef MAPBASE
	m_iHealth = sk_red_health.GetInt();
#else
	m_iHealth = 80;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Red::Spawn( void )
{
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Red::Classify( void )
{
	return	CLASS_METROPOLICE;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CNPC_Red::DeathSound( const CTakeDamageInfo &info )
{
	// Sentences don't play on dead NPCs
	SentenceStop();

	EmitSound( "red.die" );

}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_red, CNPC_Red )

AI_END_CUSTOM_NPC()
