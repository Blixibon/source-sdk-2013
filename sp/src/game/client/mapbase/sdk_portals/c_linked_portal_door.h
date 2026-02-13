//========= Mapbase - https://github.com/mapbase-source/source-sdk-2013 ============//
//
// Purpose:	linked_portal_door recreated using public Source SDK 2013 code only
// 
//			Based directly on SDK Stencil Portals by Flopgop
//			https://github.com/Flopgop/source-sdk-portals
//
// Author:	Blixibon with code from Flopgop as a basis
//
//===========================================================================//

#ifndef C_LINKEDPORTALDOOR_H
#define C_LINKEDPORTALDOOR_H

#include "c_baseentity.h"
#include "c_base_portal.h"
#include "mapbase/sdk_portals/sdk_portal_util_shared.h"

class C_LinkedPortalDoor : public C_BasePortal
{
	DECLARE_CLASS( C_LinkedPortalDoor, C_BasePortal );
	DECLARE_CLIENTCLASS();
public:
	C_LinkedPortalDoor();
	~C_LinkedPortalDoor();
};

#endif
