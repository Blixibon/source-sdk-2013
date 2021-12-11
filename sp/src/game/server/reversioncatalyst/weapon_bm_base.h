//==============================================================================
//
// Purpose: The base class for Black Mesa weapons, which are created from scratch based on HL2 weapons.
// 
//==============================================================================

#include "cbase.h"

template <class BASE_NPC>
class CBase_BM_Weapon : public BASE_NPC
{
	DECLARE_CLASS_NOFRIEND( CBase_BM_Weapon, BASE_NPC );

public:
	virtual bool	IsBlackMesa() { return true; }
};

