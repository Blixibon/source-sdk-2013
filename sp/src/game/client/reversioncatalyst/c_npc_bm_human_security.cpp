//==============================================================================
//
// Purpose: Security guards created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbase.h"
#include "c_npc_bm_human_security.h"
#include "cl_animevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_NPC_BM_HumanSecurity, DT_NPC_BM_HumanSecurity, CNPC_BM_HumanSecurity )
	RecvPropInt( RECVINFO( m_iCharacterIndex ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_NPC_BM_HumanSecurity::C_NPC_BM_HumanSecurity()
{
}
