//=============================================================================//
//
// Purpose: 	Broken Eyed Cop. The powerhouse of the cell.
//
//=============================================================================//

#ifndef NPC_BEC_H
#define NPC_BEC_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "npc_playercompanion.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
// Bec activities
//=========================================================

class CNPC_Red;

class CNPC_Bec : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS( CNPC_Bec, CNPC_PlayerCompanion );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CNPC_Bec();
	~CNPC_Bec();

	static CNPC_Bec *GetDuo();		// Gets Bec OR Bloody
	static CNPC_Bec *GetBec();		// Gets Bec only
	static CNPC_Red *GetBloody();	// Gets Bloody only
	CNPC_Bec *m_pNext;

	virtual bool IsBloody() { return false; }
	inline bool IsBec() { return !IsBloody(); }

	virtual void Precache()
	{
#ifndef MAPBASE // This is now done in CNPC_PlayerCompanion::Precache()
		// Prevents a warning
		SelectModel( );
#endif
		BaseClass::Precache();

		PrecacheScriptSound( "NPC_Metropolice.FootstepLeft" );
		PrecacheScriptSound( "NPC_Metropolice.FootstepRight" );
		PrecacheScriptSound( "bec.die" );

		// Commenting these out because idrk whether they're necessary for Bec
		//PrecacheInstancedScene( "scenes/Expressions/BecIdle.vcd" );
		//PrecacheInstancedScene( "scenes/Expressions/BecAlert.vcd" );
		//PrecacheInstancedScene( "scenes/Expressions/BecCombat.vcd" );
	}

	void	Spawn( void );
	void	SelectModel();
	virtual void	SelectHealth();
	Class_T Classify( void );
	void	Weapon_Equip( CBaseCombatWeapon *pWeapon );

	bool CreateBehaviors( void );

	void HandleAnimEvent( animevent_t *pEvent );

	float GetHitgroupDamageMultiplier(int iHitGroup, const CTakeDamageInfo& info);

	void ModifyOrAppendCriteria( AI_CriteriaSet &set );
	bool Remark( AI_CriteriaSet &modifiers, CBaseEntity *pRemarkable ) { return SpeakIfAllowed( TLK_REMARK, modifiers ); }

	bool ShouldLookForBetterWeapon() { return false; }

	void OnChangeRunningBehavior( CAI_BehaviorBase *pOldBehavior,  CAI_BehaviorBase *pNewBehavior );

	void DeathSound( const CTakeDamageInfo &info );
	void GatherConditions();
	void UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	bool ShouldRegenerateHealth(void);

#ifdef MAPBASE
	// Use Bec's default subtitle color (215,255,255)
	bool	GetGameTextSpeechParams( hudtextparms_t &params ) { params.r1 = 215; params.g1 = 255; params.b1 = 255; return BaseClass::GetGameTextSpeechParams( params ); }
#endif

	CAI_FuncTankBehavior		m_FuncTankBehavior;
	COutputEvent				m_OnPlayerUse;

	DEFINE_CUSTOM_AI;
};

#endif // NPC_BEC_H
