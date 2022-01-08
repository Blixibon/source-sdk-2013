//==============================================================================
//
// Purpose: Scientists created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "hl2/npc_citizen17.h"
#include "ai_base_bm_npc.h"

class CNPC_BM_HumanScientist : public CAI_Base_BM_Human<CNPC_Citizen>
{
public:
	DECLARE_CLASS( CNPC_BM_HumanScientist, CAI_Base_BM_Human<CNPC_Citizen> );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	DEFINE_CUSTOM_AI;

	CNPC_BM_HumanScientist();

	const char *GetCharacterClassname();

	bool	ShouldNPCAutosquad() { return m_bAutosquad; }

	void	Spawn( void );
	void	Precache( void );
	Class_T Classify ( void );

	void		ModifyOrAppendCriteria( AI_CriteriaSet &set );

	void		HandleAnimEvent( animevent_t *pEvent );

	Activity	NPC_TranslateActivity( Activity eNewActivity );

private:

	bool m_bAutosquad;
};
