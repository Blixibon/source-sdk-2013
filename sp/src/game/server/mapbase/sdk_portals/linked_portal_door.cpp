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

#include "cbase.h"
#include "base_portal.h"
#include "mapbase/sdk_portals/sdk_portal_util_shared.h"
#include <collisionutils.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CLinkedPortalDoor : public CBasePortal
{
public:
	DECLARE_CLASS( CLinkedPortalDoor, CBasePortal );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CLinkedPortalDoor();
	~CLinkedPortalDoor();
};

BEGIN_DATADESC( CLinkedPortalDoor )
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CLinkedPortalDoor, DT_LinkedPortalDoor )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( linked_portal_door, CLinkedPortalDoor );
LINK_ENTITY_TO_CLASS( point_portal, CLinkedPortalDoor ); // Flopgop's portal class

CLinkedPortalDoor::CLinkedPortalDoor()
{
}

CLinkedPortalDoor::~CLinkedPortalDoor()
{
}
