//==============================================================================
//
// Purpose: Security guards created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "hl2/npc_citizen17.h"
#include "ai_base_bm_npc.h"

class CNPC_BM_HumanSecurity : public CAI_Base_BM_Human<CNPC_Citizen>
{
public:
	DECLARE_CLASS( CNPC_BM_HumanSecurity, CAI_Base_BM_Human<CNPC_Citizen> );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CNPC_BM_HumanSecurity();

	const char *GetCharacterClassname() { return "npc_human_security"; }

	bool	ShouldNPCAutosquad() { return m_bAutosquad; }

	void	Spawn( void );
	void	Precache( void );
	Class_T Classify( void );

	bool	DoHolster( void );
	bool	DoUnholster( void );
	
	WeaponProficiency_t CalcWeaponProficiency( CBaseCombatWeapon *pWeapon );

protected:

	static bool		IsGlock( CBaseEntity* pWeapon ) { return pWeapon->ClassMatches( "weapon_bm_glock" ); }

	bool	m_fWeaponDrawn;

private:

	bool m_bAutosquad;
};
