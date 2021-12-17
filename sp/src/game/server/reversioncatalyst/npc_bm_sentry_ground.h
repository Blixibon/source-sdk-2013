//==============================================================================
//
// Purpose: Scientists created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "hl2/npc_turret_floor.h"
#include "ai_base_bm_npc.h"

class CNPC_BM_FloorSentry : public CAI_Base_BM_NPC<CNPC_FloorTurret>
{
public:
	DECLARE_CLASS( CNPC_BM_FloorSentry, CAI_Base_BM_NPC<CNPC_FloorTurret> );

	void	Spawn( void );
	void	Precache( void );
	Class_T Classify ( void );

	DECLARE_DATADESC();
};
