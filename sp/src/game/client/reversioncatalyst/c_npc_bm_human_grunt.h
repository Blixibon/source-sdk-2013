//==============================================================================
//
// Purpose: HECU marines created from scratch based on HL2 NPCs.
// 
//==============================================================================

#ifndef C_NPC_BM_HUMAN_GRUNT_H
#define C_NPC_BM_HUMAN_GRUNT_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "c_ai_basenpc.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_NPC_BM_HumanGrunt : public C_AI_BaseNPC
{
	DECLARE_CLASS( C_NPC_BM_HumanGrunt, C_AI_BaseNPC );
public:
	//DECLARE_CLIENTCLASS();

	C_NPC_BM_HumanGrunt() {}

	void FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options );

	int m_iUseMarch;
};

#endif // C_BREAKABLEPROP_H
