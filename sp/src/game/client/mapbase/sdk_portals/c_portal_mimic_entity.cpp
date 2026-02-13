//========= Mapbase - https://github.com/mapbase-source/source-sdk-2013 ============//
//
// Purpose:	Portal mimic entity recreated using public Source SDK 2013 code only
//
// Author:	Blixibon
//
//===========================================================================//

#include "cbase.h"
#include "c_portal_mimic_entity.h"
#include "c_base_portal.h"
#include "portalrendering.h"

LINK_ENTITY_TO_CLASS( portal_mimic_entity, C_PortalMimicEntity );

#ifndef CLIENTSIDE_PORTAL_MIMIC
IMPLEMENT_CLIENTCLASS_DT( C_PortalMimicEntity, DT_PortalMimicEntity, CPortalMimicEntity )
	RecvPropEHandle( RECVINFO( m_hSourcePortal ) ),
END_RECV_TABLE()
#endif

C_PortalMimicEntity::C_PortalMimicEntity()
{
	m_flScale = 1.0f;
}

C_PortalMimicEntity::~C_PortalMimicEntity()
{
}

#ifndef CLIENTSIDE_PORTAL_MIMIC
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PortalMimicEntity::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( !m_bEnableRenderingClipPlane && m_hSourcePortal )
	{
		// m_fRenderingClipPlane cuts off the mesh at a particular plane
		// We use it here to prevent models from poking through
		m_bEnableRenderingClipPlane = true;

		Vector vecForward;
		m_hSourcePortal->GetVectors( &vecForward, NULL, NULL );
		m_fRenderingClipPlane[0] = vecForward.x;
		m_fRenderingClipPlane[1] = vecForward.y;
		m_fRenderingClipPlane[2] = vecForward.z;
		m_fRenderingClipPlane[3] = vecForward.Dot( m_hSourcePortal->GetAbsOrigin() - (vecForward * 0.2f) );
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PortalMimicEntity::DelayedRemove( void )
{
	AddEffects( EF_NODRAW );
	SetNextClientThink( gpGlobals->curtime + 0.3f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PortalMimicEntity::ClientThink( void )
{
	BaseClass::ClientThink();

	SetNextClientThink( TICK_NEVER_THINK );
	Remove();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PortalMimicEntity::Release( void )
{
	RemoveFromLeafSystem();
	ClientEntityList().RemoveEntity( GetClientHandle() );

	BaseClass::Release();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_PortalMimicEntity::ShouldDraw()
{
	if ( GetOwnerEntity() )
		return GetOwnerEntity()->ShouldDraw();

	return BaseClass::ShouldDraw();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_PortalMimicEntity::DrawModel( int flags )
{
	if ( C_BasePlayer::GetLocalPlayer() == GetOwnerEntity() )
	{
		// Don't draw player when we shouldn't
		view_id_t viewID = CurrentViewID();
		int nPlayerRecursionLevel = 1;
		if ( C_BasePlayer::GetLocalPlayer()->m_bPokingThroughPortal )
		{
			nPlayerRecursionLevel = 0;

			// Based on C_BasePlayer::InPerspectiveView
			if ( C_BasePlayer::GetLocalPlayer()->m_bPokingThroughPortal && g_pPortalRender->GetViewRecursionLevel() <= 1
				&& (viewID == VIEW_MAIN || viewID == VIEW_INTRO_CAMERA || viewID == VIEW_REFRACTION || viewID == VIEW_NONE) )
				return 0;
		}

		if ( !C_BasePlayer::ShouldDrawLocalPlayer() && viewID == VIEW_PORTAL && g_pPortalRender->GetViewRecursionLevel() == nPlayerRecursionLevel )
			return 0;
	}

	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const Vector &C_PortalMimicEntity::GetRenderOrigin()
{
	if ( GetOwnerEntity() )
	{
		static Vector vecTranslatedOrigin;
		vecTranslatedOrigin = GetMatrix() * GetOwnerEntity()->GetRenderOrigin();

		if ( m_flScale != 1.0f )
		{
			// TODO: Better way of doing this?
			vecTranslatedOrigin -= m_vecPortalOrigin;
			VectorScale( vecTranslatedOrigin, m_flScale, vecTranslatedOrigin );
			vecTranslatedOrigin += m_vecPortalOrigin;
		}

		return vecTranslatedOrigin;
	}

	return BaseClass::GetRenderOrigin();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const QAngle &C_PortalMimicEntity::GetRenderAngles()
{
	if ( GetOwnerEntity() )
	{
		static QAngle angTranslatedAngles;
		angTranslatedAngles = TransformAnglesToWorldSpace( GetOwnerEntity()->GetRenderAngles(), GetMatrix().As3x4() );

		return angTranslatedAngles;
	}

	return BaseClass::GetRenderAngles();
}
