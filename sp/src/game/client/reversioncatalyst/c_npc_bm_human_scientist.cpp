//==============================================================================
//
// Purpose: Scientists created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbase.h"
#include "c_npc_bm_human_scientist.h"
#include "cl_animevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_NPC_BM_HumanScientist, DT_NPC_BM_HumanScientist, CNPC_BM_HumanScientist )
	RecvPropInt( RECVINFO( m_iCharacterIndex ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_NPC_BM_HumanScientist::C_NPC_BM_HumanScientist()
{
}
