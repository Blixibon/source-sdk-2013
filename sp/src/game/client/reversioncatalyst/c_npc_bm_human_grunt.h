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
#include "ai_base_bm_npc.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_NPC_BM_HumanGrunt : public CAI_Base_BM_Human<C_AI_BaseNPC>
{
	DECLARE_CLASS( C_NPC_BM_HumanGrunt, CAI_Base_BM_Human<C_AI_BaseNPC> );
public:
	DECLARE_CLIENTCLASS();

	C_NPC_BM_HumanGrunt();

	void	OnDataChanged( DataUpdateType_t type );

	void	FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options );

	virtual bool IsHumanAssassin() { return false; }

	//bool	m_bHasCigar;
	//LocalFlexController_t		m_iCigarFlex;
};

#endif // C_NPC_BM_HUMAN_GRUNT_H
