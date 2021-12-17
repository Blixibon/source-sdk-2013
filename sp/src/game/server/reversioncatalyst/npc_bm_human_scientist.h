//==============================================================================
//
// Purpose: Scientists created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "hl2/npc_playercompanion.h"
#include "ai_base_bm_npc.h"

class CNPC_BM_HumanScientist : public CAI_Base_BM_Human<CNPC_PlayerCompanion>
{
public:
	DECLARE_CLASS( CNPC_BM_HumanScientist, CAI_Base_BM_Human<CNPC_PlayerCompanion> );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	const char *GetCharacterClassname();

	void	Spawn( void );
	void	Precache( void );
	Class_T Classify ( void );
};
