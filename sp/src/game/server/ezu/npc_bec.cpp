//=============================================================================//
//
// Purpose: 	Broken Eyed Cop. The powerhouse of the cell.
//
//=============================================================================//

#include "cbase.h"
#include "npc_bec.h"
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

#define BEC_MODEL "models/bec.mdl"

ConVar	sk_bec_health( "sk_bec_health","0");

// Convars from npc_citizen17 but ported to Bec
ConVar  sk_bec_ar2_proficiency("sk_bec_ar2_proficiency", "2"); // Added by 1upD. Skill rating 0 - 4 of how accurate the AR2 should be
ConVar  sk_bec_default_proficiency("sk_bec_default_proficiency", "1"); // Added by 1upD. Skill rating 0 - 4 of how accurate all weapons but the AR2 should be

LINK_ENTITY_TO_CLASS( npc_bec, CNPC_Bec );

//---------------------------------------------------------
// 
//---------------------------------------------------------
IMPLEMENT_SERVERCLASS_ST(CNPC_Bec, DT_NPC_Bec)
END_SEND_TABLE()


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_Bec )
//						m_FuncTankBehavior
	DEFINE_OUTPUT( m_OnPlayerUse, "OnPlayerUse" ),
	DEFINE_USEFUNC( UseFunc ),
END_DATADESC()

//---------------------------------------------------------
// For fast lookups of Bec and Bloody
//---------------------------------------------------------
CEntityClassList<CNPC_Bec> g_BecList;
template <> CNPC_Bec *CEntityClassList<CNPC_Bec>::m_pClassList = NULL;

CNPC_Bec *CNPC_Bec::GetDuo( void )
{
	return g_BecList.m_pClassList;
}

CNPC_Bec *CNPC_Bec::GetBec( void )
{
	CNPC_Bec *pBec = g_BecList.m_pClassList;
	while (pBec)
	{
		if (pBec->IsBec())
			return pBec;

		pBec = pBec->m_pNext;
	}

	return NULL;
}

CNPC_Red *CNPC_Bec::GetBloody( void )
{
	CNPC_Bec *pBec = g_BecList.m_pClassList;
	while (pBec)
	{
		if (pBec->IsBloody())
			return static_cast<CNPC_Red*>(pBec);

		pBec = pBec->m_pNext;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CNPC_Bec::CNPC_Bec( void )
{
	g_BecList.Insert(this);
}

CNPC_Bec::~CNPC_Bec( void )
{
	g_BecList.Remove(this);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bec::SelectModel()
{
#ifdef MAPBASE
	if (GetModelName() == NULL_STRING)
#endif
	SetModelName( AllocPooledString( BEC_MODEL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bec::SelectHealth()
{
#ifdef MAPBASE
	m_iHealth = sk_bec_health.GetInt();
#else
	m_iHealth = 80;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bec::Spawn( void )
{
	Precache();

	SelectHealth();

	// Bec technically doesn't have a face so idk if we'll have use for this
	//m_iszIdleExpression = MAKE_STRING("scenes/Expressions/BarneyIdle.vcd");
	//m_iszAlertExpression = MAKE_STRING("scenes/Expressions/BarneyAlert.vcd");
	//m_iszCombatExpression = MAKE_STRING("scenes/Expressions/BarneyCombat.vcd");

	BaseClass::Spawn();

	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );

	NPCInit();

	SetUse( &CNPC_Bec::UseFunc );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Bec::Classify( void )
{
	return	CLASS_METROPOLICE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Bec::Weapon_Equip( CBaseCombatWeapon *pWeapon )
{
	BaseClass::Weapon_Equip( pWeapon );

	if( hl2_episodic.GetBool() && FClassnameIs( pWeapon, "weapon_ar2" ) )
	{
		// Allow Bec to defend himself at point-blank range in c17_05.
		// Inherited from Barney
		pWeapon->m_fMinRange1 = 0.0f;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Bec::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
	case NPC_EVENT_LEFTFOOT:
		{
			EmitSound( "NPC_Metropolice.FootstepLeft", pEvent->eventtime );
		}
		break;
	case NPC_EVENT_RIGHTFOOT:
		{
			EmitSound( "NPC_Metropolice.FootstepRight", pEvent->eventtime );
		}
		break;

	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CNPC_Bec::GetHitgroupDamageMultiplier(int iHitGroup, const CTakeDamageInfo& info)
{
	return BaseClass::GetHitgroupDamageMultiplier(iHitGroup, info);
}

//-----------------------------------------------------------------------------
// Purpose: Tack on extra criteria for responses
//-----------------------------------------------------------------------------
void CNPC_Bec::ModifyOrAppendCriteria( AI_CriteriaSet &set )
{
	BaseClass::ModifyOrAppendCriteria( set );

	if (GetEnemy())
	{
		if (CNPC_Citizen *pCitizen = dynamic_cast<CNPC_Citizen *>(GetEnemy()))
		{
			set.AppendCriteria( "citizentype", UTIL_VarArgs( "%i", pCitizen->GetCitiznType() ) );
		}
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CNPC_Bec::DeathSound( const CTakeDamageInfo &info )
{
	// Sentences don't play on dead NPCs
	SentenceStop();

	EmitSound( "bec.die" );

}

bool CNPC_Bec::CreateBehaviors( void )
{
	BaseClass::CreateBehaviors();
	AddBehavior( &m_FuncTankBehavior );

	return true;
}

void CNPC_Bec::OnChangeRunningBehavior( CAI_BehaviorBase *pOldBehavior,  CAI_BehaviorBase *pNewBehavior )
{
	if ( pNewBehavior == &m_FuncTankBehavior )
	{
		m_bReadinessCapable = false;
	}
	else if ( pOldBehavior == &m_FuncTankBehavior )
	{
		m_bReadinessCapable = IsReadinessCapable();
	}

	BaseClass::OnChangeRunningBehavior( pOldBehavior, pNewBehavior );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Bec::GatherConditions()
{
	BaseClass::GatherConditions();

	// Handle speech AI. Don't do AI speech if we're in scripts unless permitted by the EnableSpeakWhileScripting input.
	if ( m_NPCState == NPC_STATE_IDLE || m_NPCState == NPC_STATE_ALERT || m_NPCState == NPC_STATE_COMBAT ||
		( ( m_NPCState == NPC_STATE_SCRIPT ) && CanSpeakWhileScripting() ) )
	{
		DoCustomSpeechAI();
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Bec::UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_bDontUseSemaphore = true;
	SpeakIfAllowed( TLK_USE );
	m_bDontUseSemaphore = false;

	m_OnPlayerUse.FireOutput( pActivator, pCaller );
}

//-----------------------------------------------------------------------------
// Taken from npc_combine but edited for Bec regeneration
//-----------------------------------------------------------------------------
bool CNPC_Bec::ShouldRegenerateHealth(void)
{
	return true;
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_bec, CNPC_Bec )

AI_END_CUSTOM_NPC()
