//==============================================================================
//
// Purpose: The base class for Black Mesa weapons, which are created from scratch based on HL2 weapons.
// 
//==============================================================================

#include "cbase.h"

template <class BASE_WEAPON>
class CBase_BM_Weapon : public BASE_WEAPON
{
	DECLARE_CLASS_NOFRIEND( CBase_BM_Weapon, BASE_WEAPON );

public:
	virtual bool	IsBlackMesa() { return true; }
};


// TODO: MOVE THIS TO ITS OWN HEADER
template <class BASE_ITEM>
class CBase_BM_Item : public BASE_ITEM
{
	DECLARE_CLASS_NOFRIEND( CBase_BM_Item, BASE_ITEM );

public:
	virtual bool	IsBlackMesa() { return true; }
};

