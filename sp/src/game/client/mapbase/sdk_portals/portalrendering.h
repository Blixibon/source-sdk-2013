//========= Mapbase - https://github.com/mapbase-source/source-sdk-2013 ============//
//
// Purpose:	Portal rendering recreated using public Source SDK 2013 code only
// 
//			Based directly on SDK Stencil Portals by Flopgop
//			https://github.com/Flopgop/source-sdk-portals
//
// Author:	Blixibon with code from Flopgop as a basis
//
//===========================================================================//

#ifndef PORTAL_RENDERING_H
#define PORTAL_RENDERING_H

#include "viewrender.h"

#define VIEW_PORTAL VIEW_MONITOR

class PortalRendering
{
public:
	//
	// Portal rendering stubs
	//
	bool DrawPortalsUsingStencils( CViewRender *view );
	bool ShouldUseStencilsToRenderPortals() { return true; }
	bool ShouldObeyStencilForClears() { return IsRenderingPortal(); }

	bool IsRenderingPortal() { return m_iRecursionLevel > 0; }

	view_id_t GetCurrentViewId() { return VIEW_PORTAL; }
	int GetViewRecursionLevel() { return m_iRecursionLevel; }

	// Texture
	void DrawPortalsToTextures( CViewRender *view, const CViewSetup &cameraView ) {} // TODO

	// Fog
	const Vector &GetExitPortalFogOrigin();
	void ShiftFogForExitPortalView() {} // TODO

	// Water (TODO)
	void WaterRenderingHandler_PreReflection() {}
	void WaterRenderingHandler_PostReflection() {}
	void WaterRenderingHandler_PreRefraction() {}
	void WaterRenderingHandler_PostRefraction() {}
	int ShouldForceCheaperWaterLevel() { return MIN( 0, 3 - m_iRecursionLevel ); } // Arbitrarily decrease from recursion level for now
	bool DoesExitPortalViewIntersectWaterPlane( float waterZ, int leafWaterDataID ) { return false; }

	// Skybox
	SkyboxVisibility_t IsSkyboxVisibleFromExitPortal();
	view_id_t GetCurrentSkyboxViewId() { return VIEW_3DSKY; }

private:
	int m_iRecursionLevel;

	Vector m_vecCurrentCameraOrigin = vec3_origin;
	int m_iCurrentEnterPortal	= -1;	// Currently drawing portal entrance; -1 for none
	int m_iCurrentExitPortal	= -1;	// Currently drawing portal exit; -1 for none
};

// SDK Portals
// This matches the singleton in Portal so that we can use overlapping code within the base SDK
extern PortalRendering *g_pPortalRender;
#endif
