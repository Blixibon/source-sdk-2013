//==============================================================================
//
// Purpose: The base class for Black Mesa NPCs, which are created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbase.h"

template <class BASE_NPC>
class CAI_Base_BM_NPC : public BASE_NPC
{
	DECLARE_CLASS_NOFRIEND( CAI_Base_BM_NPC, BASE_NPC );

public:
	virtual bool	IsBlackMesa() { return true; }

	bool m_bCustomBody;
};

#define DECLARE_BM_NPC_DATADESC() \
	DEFINE_KEYFIELD( m_bCustomBody, FIELD_BOOLEAN, "CustomBody" ),	\

