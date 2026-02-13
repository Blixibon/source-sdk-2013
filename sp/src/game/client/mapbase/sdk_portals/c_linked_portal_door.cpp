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
#include "c_linked_portal_door.h"
#include "portalrendering.h"
#include "view_scene.h"

LINK_ENTITY_TO_CLASS( linked_portal_door, C_LinkedPortalDoor );
//LINK_ENTITY_TO_CLASS( point_portal, C_LinkedPortalDoor ); // Flopgop's portal class

IMPLEMENT_CLIENTCLASS_DT( C_LinkedPortalDoor, DT_LinkedPortalDoor, CLinkedPortalDoor )
END_RECV_TABLE()

C_LinkedPortalDoor::C_LinkedPortalDoor()
{
}

C_LinkedPortalDoor::~C_LinkedPortalDoor()
{
}
