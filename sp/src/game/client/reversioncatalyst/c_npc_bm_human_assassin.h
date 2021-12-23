//==============================================================================
//
// Purpose: Black Ops assassins created from scratch based on HL2 NPCs.
// 
//==============================================================================

#ifndef C_NPC_BM_HUMAN_ASSASSIN_H
#define C_NPC_BM_HUMAN_ASSASSIN_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "ai_base_bm_npc.h"
#include "c_npc_bm_human_grunt.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_NPC_BM_HumanFemaleAssassin : public CAI_Base_BM_Human<C_AI_BaseNPC>
{
	DECLARE_CLASS( C_NPC_BM_HumanFemaleAssassin, CAI_Base_BM_Human<C_AI_BaseNPC> );
public:
	DECLARE_CLIENTCLASS();

	C_NPC_BM_HumanFemaleAssassin();

	void	OnDataChanged( DataUpdateType_t type );

	bool IsHumanAssassin() { return true; }

	// TODO
	float m_flCloakFactor = 0.0f;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_NPC_BM_HumanMaleAssassin : public C_NPC_BM_HumanGrunt
{
	DECLARE_CLASS( C_NPC_BM_HumanMaleAssassin, C_NPC_BM_HumanGrunt );
public:
	DECLARE_CLIENTCLASS();

	C_NPC_BM_HumanMaleAssassin();

	void	OnDataChanged( DataUpdateType_t type );

	bool IsHumanAssassin() { return true; }
};

#endif // C_NPC_BM_HUMAN_ASSASSIN_H
