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

#include "cbase.h"
#include "portalrendering.h"
#include "c_linked_portal_door.h"
#include "c_stencil.h"
#include "viewrender.h"
#include "view.h"

#include "engine/ivdebugoverlay.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#include <view_scene.h>

static ConVar r_portal_stencil_depth( "r_portal_stencil_depth", "2", FCVAR_CLIENTDLL, "When using stencil views, this changes how many views within views we see" );
static ConVar r_debug_portals( "r_debug_portals", "0", FCVAR_CLIENTDLL, "Shows debug information about portals" );

static PortalRendering s_PortalRendering;
PortalRendering* g_pPortalRender = &s_PortalRendering;

#ifdef PORTAL
#error "This codebase mounts both SDK Portals and Portal. This implementation collides and must be sorted out!"
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool PortalRendering::DrawPortalsUsingStencils(CViewRender* view)
{
	if ( C_BasePortal::AllPortals.Size() > 0 )
	{
		const CViewSetup *viewSetup = view->GetViewSetup();

		Frustum_t frustum;
		GeneratePerspectiveFrustum( CurrentViewOrigin(), CurrentViewAngles(), viewSetup->zNear, viewSetup->zFar,
			viewSetup->fov, viewSetup->m_flAspectRatio, frustum );

		// First, make sure we have valid portals from this perspective
		CUtlVector<int> vActivePortals;
		for (int i = 0; i < C_BasePortal::AllPortals.Size(); ++i)
		{
			C_BasePortal* window = C_BasePortal::AllPortals[i];
			if ( window->IsDormant() || !window->HasPartner() || !window->IsActive() || window->VisualsDisabled() )
				continue;

			// Plane clip test
			Vector vecForward;
			window->GetVectors( &vecForward, NULL, NULL );
			if ( DotProduct( vecForward, CurrentViewOrigin() - window->GetAbsOrigin() ) < 0.0f )
				continue;

			// Bounding box test
			Vector vecMins, vecMaxs;
			window->CollisionProp()->WorldSpaceAABB( &vecMins, &vecMaxs );
			if ( R_CullBox( vecMins, vecMaxs, frustum ) )
				continue;

			vActivePortals.AddToTail( i );
		}

		if ( vActivePortals.Count() == 0 )
			return false;

		if ( m_iRecursionLevel >= r_portal_stencil_depth.GetInt() )
		{
			// Render meshes only
			// TODO: Fake recursion
			for (int j = 0; j < vActivePortals.Size(); ++j)
			{
				int i = vActivePortals[j];
				C_BasePortal::AllPortals[i]->DrawFakePlane();
			}
			return false;
		}

		CMatRenderContextPtr pRenderContext(materials);
		pRenderContext->Flush(true);

		CViewSetup viewBackup;
		memcpy(&viewBackup, viewSetup, sizeof(CViewSetup));

		const_cast<CViewSetup *>(viewSetup)->zNear = 0.1f;

		if (m_iRecursionLevel == 0)
			g_pStencilTool->SetupInitialStencilRendering(pRenderContext, m_iRecursionLevel);

		for (int j = 0; j < vActivePortals.Size(); ++j)
		{
			int i = vActivePortals[j];

			//const float forwardOffset = 0.1;
			C_BasePortal* window = C_BasePortal::AllPortals[i];

			m_iCurrentEnterPortal = i;
			m_iCurrentExitPortal = C_BasePortal::AllPortals.Find( window->GetPartner() );

			g_pStencilTool->SetStencilReferenceValue(pRenderContext, m_iRecursionLevel);
			window->DrawStencil();
			m_iRecursionLevel += 1;
			g_pStencilTool->ClearDepthBuffer(pRenderContext, m_iRecursionLevel);

			CViewSetup windowView = *viewSetup;

			Vector friendFwd;
			window->GetPartner()->GetVectors(&friendFwd, NULL, NULL);

			VMatrix viewToPortalSpaceMat = window->MatrixThisToLinked();

			Vector vecViewOrigin = CurrentViewOrigin();

			if ( window->GetPartner()->GetScale() != 1.0f )
			{
				// TODO: Better way of doing this?
				vecViewOrigin -= window->GetAbsOrigin();
				VectorScale( vecViewOrigin, window->GetPartner()->GetScale(), vecViewOrigin );
				vecViewOrigin += window->GetAbsOrigin();
			}

			m_vecCurrentCameraOrigin = viewToPortalSpaceMat * vecViewOrigin;

			QAngle angles = TransformAnglesToWorldSpace(CurrentViewAngles(), viewToPortalSpaceMat.As3x4());
			windowView.zNear = 1.0f;
			windowView.origin = m_vecCurrentCameraOrigin;
			windowView.angles = angles;

			// while this is technically a 180 degree rotation about up, it doesn't produce the right result
			// I didn't pay attention in linear algebra enough to understand why
			// all I know is manually making a matrix that inverts x and y produces the correct result without more complex matrix calculations in the render thread.
			//MatrixBuildRotationAboutAxis(rotation, myUp, 180.0f);

			if (r_debug_portals.GetBool())
			{
				debugoverlay->AddTextOverlay(window->GetAbsOrigin(), m_iRecursionLevel, 0.0f, "window %d [%i]", i, m_iRecursionLevel);

				Vector vecDebugCameraOrigin = m_vecCurrentCameraOrigin + Vector(0,0,8 * m_iRecursionLevel);
				debugoverlay->AddTextOverlay( vecDebugCameraOrigin, 0.0f, "m_vecCurrentCameraOrigin %d [%i]", i, m_iRecursionLevel );

				static Color faceColor( 0, 0, 0, 0 );
				static Color edgeColor( 160, 160, 255, 255 );
				debugoverlay->AddBoxOverlay2( window->GetAbsOrigin(), window->CollisionProp()->OBBMins(), window->CollisionProp()->OBBMaxs(), window->GetAbsAngles(), faceColor, edgeColor, 0.0f );
			}

			float clipPlane[4];
			clipPlane[0] = friendFwd.x;
			clipPlane[1] = friendFwd.y;
			clipPlane[2] = friendFwd.z;
			clipPlane[3] = friendFwd.Dot(window->GetPartner()->GetAbsOrigin() - (friendFwd * 0.2f));

			pRenderContext->PushCustomClipPlane(clipPlane);

			render->Push3DView(windowView, 0, nullptr, view->GetFrustum());

			render->OverrideViewFrustum(view->GetFrustum());

			ViewCustomVisibility_t customVisibility;

			Vector vecSurfaceOrigin = window->GetPartner()->GetAbsOrigin();

			customVisibility.AddVisOrigin( vecSurfaceOrigin );
			customVisibility.m_VisData.m_vecVisOrigin = vecSurfaceOrigin;

			view->ViewDrawScene_PortalStencil( windowView, &customVisibility );

			/*VisibleFogVolumeInfo_t fogInfo;
			render->GetVisibleFogVolume(windowView.origin, &fogInfo);

			WaterRenderInfo_t waterInfo;
			view->DetermineWaterRenderInfo(fogInfo, waterInfo);


			pClientView->Setup(windowView, VIEW_CLEAR_OBEY_STENCIL, true, fogInfo, waterInfo, nullptr);
			view->AddViewToScene(pClientView);*/

			render->PopView(view->GetFrustum());

			memcpy((void*)viewSetup, &viewBackup, sizeof(CViewSetup));
			g_pStencilTool->SetupInitialStencilRendering(pRenderContext, m_iRecursionLevel); // basically just reset everything
			pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_EQUAL);
			pRenderContext->SetStencilPassOperation(STENCILOPERATION_KEEP);

			g_pStencilTool->RestoreStencilMask(pRenderContext, m_iRecursionLevel);

			m_iRecursionLevel -= 1;
			pRenderContext->SetStencilReferenceValue(m_iRecursionLevel);

			window->DrawStencilDepth();

			pRenderContext->PopCustomClipPlane();
			UpdateFullScreenDepthTexture();
		}

		if (m_iRecursionLevel == 0)
			g_pStencilTool->DisableInitialStencilRendering( pRenderContext );
		else
			g_pStencilTool->SetStencilReferenceValue( pRenderContext, m_iRecursionLevel );

		pRenderContext->Flush(true);
		return true;
	}

	m_vecCurrentCameraOrigin = vec3_origin;
	m_iCurrentEnterPortal = -1;
	m_iCurrentExitPortal = -1;
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const Vector &PortalRendering::GetExitPortalFogOrigin()
{
	return m_vecCurrentCameraOrigin; // temp?
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
SkyboxVisibility_t PortalRendering::IsSkyboxVisibleFromExitPortal()
{
	if ( m_iCurrentExitPortal == -1 )
		return SKYBOX_NOT_VISIBLE;

	return engine->IsSkyboxVisibleFromPoint( C_BasePortal::AllPortals[m_iCurrentExitPortal]->GetAbsOrigin() ); // TODO: Smarter?
}
