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
abstract_class C_CAI_AssassinSink
{
public:
	virtual float	GetCloakFactor() const = 0;
};

template <class BASE_NPC>
class C_CAI_Base_BM_Assassin : public BASE_NPC, public C_CAI_AssassinSink
{
	DECLARE_CLASS_NOFRIEND( C_CAI_Base_BM_Assassin, BASE_NPC );

public:

	bool		IsHumanAssassin() { return true; }

	float		GetCloakFactor() const { return m_flCloakFactor; }

	float m_flCloakFactor = 0.0f;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_NPC_BM_HumanFemaleAssassin : public C_CAI_Base_BM_Assassin<CAI_Base_BM_Human<C_AI_BaseNPC>>
{
	DECLARE_CLASS( C_NPC_BM_HumanFemaleAssassin, C_CAI_Base_BM_Assassin<CAI_Base_BM_Human<C_AI_BaseNPC>> );
public:
	DECLARE_CLIENTCLASS();

	C_NPC_BM_HumanFemaleAssassin();

	void	OnDataChanged( DataUpdateType_t type );
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_NPC_BM_HumanMaleAssassin : public C_CAI_Base_BM_Assassin<C_NPC_BM_HumanGrunt>
{
	DECLARE_CLASS( C_NPC_BM_HumanMaleAssassin, C_CAI_Base_BM_Assassin<C_NPC_BM_HumanGrunt> );
public:
	DECLARE_CLIENTCLASS();

	C_NPC_BM_HumanMaleAssassin();

	void	OnDataChanged( DataUpdateType_t type );
};

#endif // C_NPC_BM_HUMAN_ASSASSIN_H
