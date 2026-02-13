//========= Mapbase - https://github.com/mapbase-source/source-sdk-2013 ============//
//
// Purpose: Mapbase-specific user messages.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include "usermessages.h"
#ifdef CLIENT_DLL
#include "hud_macros.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
void HookMapbaseUserMessages( void )
{
	// VScript
	//HOOK_MESSAGE( ScriptMsg ); // Hooked in CNetMsgScriptHelper

	//HOOK_MESSAGE( ShowMenuComplex ); // Hooked in CHudMenu

	//HOOK_MESSAGE( EntityEnterPortal ); // Hooked in ClientModeShared
	//HOOK_MESSAGE( EntityExitPortal ); // Hooked in ClientModeShared
}
#endif

void RegisterMapbaseUserMessages( void )
{
	// VScript
	usermessages->Register( "ScriptMsg", -1 ); // CNetMsgScriptHelper

	usermessages->Register( "ShowMenuComplex", -1 ); // CHudMenu

#ifdef USE_PORTALS
	usermessages->Register( "EntityEnterPortal", 4 ); // ClientModeShared
	usermessages->Register( "EntityExitPortal", 5 ); // ClientModeShared
#endif

#ifdef CLIENT_DLL
	// TODO: Better placement?
	HookMapbaseUserMessages();
#endif
}
