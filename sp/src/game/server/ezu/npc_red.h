//=============================================================================//
//
// Purpose: 	Bloody Cop.
//
//=============================================================================//

#ifndef NPC_RED_H
#define NPC_RED_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "npc_bec.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
// Red activities
//=========================================================

class CNPC_Red : public CNPC_Bec
{
public:
	DECLARE_CLASS( CNPC_Red, CNPC_Bec );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	bool IsBloody() { return true; }

	virtual void Precache()
	{
		BaseClass::Precache();
		PrecacheScriptSound( "red.die" );
	}

	void	Spawn( void );
	void	SelectModel();
	void	SelectHealth();
	Class_T Classify( void );

	void DeathSound( const CTakeDamageInfo &info );

#ifdef MAPBASE
	// Use Red's default subtitle color (215,255,255)
	bool	GetGameTextSpeechParams( hudtextparms_t &params ) { params.r1 = 215; params.g1 = 255; params.b1 = 255; return BaseClass::GetGameTextSpeechParams( params ); }
#endif

	CAI_FuncTankBehavior		m_FuncTankBehavior;
	COutputEvent				m_OnPlayerUse;

	DEFINE_CUSTOM_AI;
};

#endif // NPC_BEC_H
