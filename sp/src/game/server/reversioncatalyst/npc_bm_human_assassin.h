//==============================================================================
//
// Purpose: Black Ops assassins created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "ai_base_bm_npc.h"
#include "npc_bm_human_grunt.h"

//=========================================================
//	>> CNPC_BM_HumanFemaleAssassin
// (technically a completely new NPC in relation to Black Mesa: Source)
//=========================================================
class CNPC_BM_HumanFemaleAssassin : public CAI_Base_BM_Human<CNPC_Combine>
{
	DECLARE_CLASS( CNPC_BM_HumanFemaleAssassin, CAI_Base_BM_Human<CNPC_Combine> );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	DEFINE_CUSTOM_AI;

	CNPC_BM_HumanFemaleAssassin();

public: 
	void		Spawn( void );
	void		Precache( void );
	Class_T		Classify( void );
	const char	*GetCharacterClassname();

	void		Event_Killed( const CTakeDamageInfo &info );

	void		HandleAnimEvent( animevent_t *pEvent );

	Activity	NPC_TranslateActivity( Activity eNewActivity );
	void		OnChangeActivity( Activity eNewActivity );

	bool		IsHumanAssassin() { return true; }

	void		PrescheduleThink( void );

private:

	CHandle<CBaseAnimating>		m_hLeftHandGun;
};

//=========================================================
//	>> CNPC_BM_HumanMaleAssassin
// (technically a completely new NPC in relation to Black Mesa: Source)
//=========================================================
class CNPC_BM_HumanMaleAssassin : public CNPC_BM_HumanGrunt
{
	DECLARE_CLASS( CNPC_BM_HumanMaleAssassin, CNPC_BM_HumanGrunt );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CNPC_BM_HumanMaleAssassin();

public: 
	void		Spawn( void );
	void		Precache( void );
	Class_T		Classify( void );
	const char	*GetCharacterClassname();

	Activity	NPC_TranslateActivity( Activity eNewActivity );
	void		OnChangeActivity( Activity eNewActivity );

	bool		ShouldHaveSpeedBoost();

	bool		IsHumanAssassin() { return true; }

	void		PrescheduleThink( void );
	float		GetSequenceGroundSpeed( CStudioHdr *pStudioHdr, int iSequence );
};
