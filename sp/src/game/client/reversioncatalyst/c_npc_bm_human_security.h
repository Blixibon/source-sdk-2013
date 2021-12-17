//==============================================================================
//
// Purpose: Security guards created from scratch based on HL2 NPCs.
// 
//==============================================================================

#ifndef C_NPC_BM_HUMAN_SECURITY_H
#define C_NPC_BM_HUMAN_SECURITY_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "ai_base_bm_npc.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_NPC_BM_HumanSecurity : public CAI_Base_BM_Human<C_AI_BaseNPC>
{
	DECLARE_CLASS( C_NPC_BM_HumanSecurity, CAI_Base_BM_Human<C_AI_BaseNPC> );
public:
	DECLARE_CLIENTCLASS();

	C_NPC_BM_HumanSecurity();
};

#endif // C_NPC_BM_HUMAN_SECURITY_H
