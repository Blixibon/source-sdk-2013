//========= Mapbase - https://github.com/mapbase-source/source-sdk-2013 ============//
//
// Purpose:	Base portal recreated using public Source SDK 2013 code only
// 
//			Based directly on SDK Stencil Portals by Flopgop
//			https://github.com/Flopgop/source-sdk-portals
//
// Author:	Blixibon with code from Flopgop as a basis
//
//===========================================================================//

#include "cbase.h"
#include "c_base_portal.h"
#include "portalrendering.h"
#include "view_scene.h"
#include "c_portal_mimic_entity.h"
#include "mapbase/sdk_portals/sdk_portal_util_shared.h"

ConVar	r_portal_mimic("r_portal_mimic", "1");

static ConVar r_debug_portals_mimic( "r_debug_portals_mimic", "0" );

#undef CBasePortal

IMPLEMENT_CLIENTCLASS_DT( C_BasePortal, DT_BasePortal, CBasePortal )
	RecvPropEHandle( RECVINFO( m_hLinkedPortal ) ),
	RecvPropFloat( RECVINFO( m_fHalfWidth ) ),
	RecvPropFloat( RECVINFO( m_fHalfHeight ) ),
	RecvPropBool( RECVINFO( m_bActivated ) ),
	//RecvPropBool( RECVINFO( m_bDisablePhysics ) ),
	RecvPropBool( RECVINFO( m_bDisableVisuals ) ),
	RecvPropFloat( RECVINFO( m_flScale ) ),
END_RECV_TABLE()

CUtlVector<C_BasePortal*>	C_BasePortal::AllPortals;

C_BasePortal::C_BasePortal()
{
	AllPortals.AddToTail( this );

	m_flScale = 1.0f;
}

C_BasePortal::~C_BasePortal()
{
	AllPortals.FindAndRemove( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BasePortal::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( m_hLinkedPortal != m_hOldLinkedPortal )
	{
		m_bCachedLinkedPortal = false;
		m_hOldLinkedPortal = m_hLinkedPortal;
	}

	//if ( m_bAlwaysUpdate )
	{
		m_bCachedLinkedPortal = false;

		// Make sure any portal that has us as their partner refreshes their position
		FOR_EACH_VEC( AllPortals, i )
		{
			if ( AllPortals[i]->GetPartner() == this )
				AllPortals[i]->NotifyPositionChanged( this );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BasePortal::UpdateOnRemove()
{
#ifdef CLIENTSIDE_PORTAL_MIMIC
	// Remove any leftover mimic entities
	FOR_EACH_VEC_BACK( m_hMimicEntities, i )
	{
		if ( m_hMimicEntities[i] )
		{
			m_hMimicEntities[i]->DelayedRemove();
		}
		m_hMimicEntities.Remove( i );
	}
#endif

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BasePortal::DrawStencil()
{
	Draw( materials->FindMaterial( "tools/toolsblack", "" ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BasePortal::DrawStencilDepth()
{
	// TODO: this needs to get a material that ONLY writes depth and not color.
	// eventually you should be able to put a mask material on this, make real looking portals
	Draw( materials->FindMaterial( "engine/writez", "" ), 0.2f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BasePortal::DrawFakePlane()
{
	// TODO: Fake recursion like in Portal
	Draw( materials->FindMaterial( "dev/dev_windowportal", "" ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BasePortal::Draw( const IMaterial *pMaterial, float forwardOffset )
{
	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->Bind((IMaterial*)pMaterial, GetClientRenderable());

	// This can depend on the Bind command above, so keep this after!
	UpdateFrontBufferTexturesForMaterial((IMaterial*)pMaterial);

	pRenderContext->MatrixMode(MATERIAL_MODEL); //just in case
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();

	Vector m_vForward, m_vRight, m_vUp;
	this->GetVectors(&m_vForward, &m_vRight, &m_vUp);

	Vector ptCenter = this->GetAbsOrigin() + m_vForward * forwardOffset;

	Vector verts[4];
	verts[0] = ptCenter + (m_vRight * m_fHalfWidth) - (m_vUp * m_fHalfHeight);
	verts[1] = ptCenter + (m_vRight * m_fHalfWidth) + (m_vUp * m_fHalfHeight);
	verts[2] = ptCenter - (m_vRight * m_fHalfWidth) - (m_vUp * m_fHalfHeight);
	verts[3] = ptCenter - (m_vRight * m_fHalfWidth) + (m_vUp * m_fHalfHeight);

	CMeshBuilder meshBuilder;
	IMesh* pMesh = pRenderContext->GetDynamicMesh(false);
	meshBuilder.Begin(pMesh, MATERIAL_TRIANGLE_STRIP, 2);

	meshBuilder.Position3fv(&verts[3].x);
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3fv(&verts[2].x);
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3fv(&verts[1].x);
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3fv(&verts[0].x);
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();

	pRenderContext->MatrixMode(MATERIAL_MODEL);
	pRenderContext->PopMatrix();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BasePortal::Simulate()
{
	BaseClass::Simulate();

#ifdef CLIENTSIDE_PORTAL_MIMIC
	if ( !IsActive() || !HasPartner() || !r_portal_mimic.GetBool() )
	{
		FOR_EACH_VEC_BACK( m_hTouchingEntities, i )
		{
			if ( m_hTouchingEntities[i] )
			{
				OnEntityExitPortal( m_hTouchingEntities[i], false );
			}
			//m_hTouchingEntities.Remove( i );
		}

		// Remove any leftover mimic entities
		FOR_EACH_VEC_BACK( m_hMimicEntities, i )
		{
			if ( m_hMimicEntities[i] )
			{
				if ( r_debug_portals_mimic.GetBool() )
					Msg( "Removing leftover mimic entity %i (%s)\n", i, STRING( m_hMimicEntities[i]->GetModelName() ) );
				m_hMimicEntities[i]->DelayedRemove();
			}
			m_hMimicEntities.Remove( i );
		}

		return;
	}

	if ( GetMoveParent() )
	{
		m_bCachedLinkedPortal = false;

		// Make sure any portal that has us as their partner refreshes their position
		FOR_EACH_VEC( AllPortals, i )
		{
			if ( AllPortals[i]->GetPartner() == this )
				AllPortals[i]->NotifyPositionChanged( this );
		}
	}

	VMatrix matPartner = MatrixThisToLinked();

	// Verify mimic entities
	FOR_EACH_VEC_BACK( m_hTouchingEntities, i )
	{
		C_BaseEntity *pSource = m_hTouchingEntities[i];
		if ( !pSource )
		{
			m_hTouchingEntities.Remove( i );
			continue;
		}

		// m_fRenderingClipPlane cuts off the mesh at a particular plane
		// We use it here to prevent models from poking through
		// We have to set it per-tick because the portal's position can change
		Vector vecForward;
		GetVectors( &vecForward, NULL, NULL );
		pSource->m_fRenderingClipPlane[0] = vecForward.x;
		pSource->m_fRenderingClipPlane[1] = vecForward.y;
		pSource->m_fRenderingClipPlane[2] = vecForward.z;
		pSource->m_fRenderingClipPlane[3] = vecForward.Dot( GetAbsOrigin() - (vecForward * 0.2f) );

		if ( pSource->GetBaseAnimating() && pSource->GetBaseAnimating()->IsRagdoll() )
			continue;

		// Owned weapons don't transfer
		if ( pSource->IsBaseCombatWeapon() && pSource->MyCombatWeaponPointer()->GetOwner() )
			continue;

		int j = 0;
		for ( ; j < m_hMimicEntities.Count(); j++ )
		{
			if ( !m_hMimicEntities[j] || m_hMimicEntities[j]->GetOwnerEntity() != pSource)
				continue;
			break;
		}

		C_PortalMimicEntity *pMimic = NULL;
		if ( j == m_hMimicEntities.Count() )
		{
			// Create a new one
			pMimic = (C_PortalMimicEntity*)CreateEntityByName( "portal_mimic_entity" );
			if ( pMimic )
			{
				pMimic->SetOwnerEntity( pSource );

				pMimic->SetModelIndex( pSource->GetModelIndex() );
				pMimic->SetModelName( pSource->GetModelName() );

				pMimic->m_bEnableRenderingClipPlane = true;

				// Add to client entity list
				ClientEntityList().AddNonNetworkableEntity( pMimic );

				//pMimic->AddToLeafSystem( pSource->GetRenderGroup() );

				EHANDLE hMimic = pMimic;
				m_hMimicEntities.AddToTail( hMimic );
			}
		}
		else
		{
			pMimic = m_hMimicEntities[j].Get();
		}
		
		if ( pMimic )
		{
			pMimic->m_matPortal = matPartner;

			Vector vecOrigin = matPartner * pSource->GetAbsOrigin();

			if ( m_hLinkedPortal->GetScale() != 1.0f )
			{
				pMimic->m_vecPortalOrigin = m_hLinkedPortal->GetAbsOrigin(); // Only needed with scale
				pMimic->m_flScale = m_hLinkedPortal->GetScale();
				m_hLinkedPortal->ApplyScale( vecOrigin );
			}

			pMimic->SetAbsOrigin( vecOrigin );
			pMimic->SetAbsAngles( TransformAnglesToWorldSpace( pSource->GetAbsAngles(), matPartner.As3x4() ) );

			// See above for more info
			m_hLinkedPortal->GetVectors( &vecForward, NULL, NULL );
			pMimic->m_fRenderingClipPlane[0] = vecForward.x;
			pMimic->m_fRenderingClipPlane[1] = vecForward.y;
			pMimic->m_fRenderingClipPlane[2] = vecForward.z;
			pMimic->m_fRenderingClipPlane[3] = vecForward.Dot( m_hLinkedPortal->GetAbsOrigin() - (vecForward * 0.2f) );

			if ( pSource->GetBaseAnimating() )
			{
				C_BaseAnimating *pSourceAnim = pSource->GetBaseAnimating();
				pMimic->SetCycle( pSourceAnim->GetCycle() );
				pMimic->SetEffects( pSourceAnim->GetEffects() );
				pMimic->SetSequence( pSourceAnim->GetSequence() );
				pMimic->m_flAnimTime = pSourceAnim->m_flAnimTime;
				pMimic->m_nBody = pSourceAnim->m_nBody;
				pMimic->m_nSkin = pSourceAnim->m_nSkin;

				/*for ( int k = 0; k < pSourceAnim->GetModelPtr()->GetNumPoseParameters(); k++ )
				{
					pMimic->SetPoseParameter( k, pSourceAnim->GetPoseParameter( k ) );
				}*/

				float flScale = pSourceAnim->GetModelScale();
				if ( m_hLinkedPortal->GetScale() != 1.0f )
					flScale *= m_hLinkedPortal->GetScale();

				if ( pMimic->GetModelScale() != flScale )
					pMimic->SetModelScale( flScale );
			}

			pMimic->m_clrRender = pSource->m_clrRender;
			pMimic->SetRenderMode( pSource->GetRenderMode() );
			pMimic->m_nRenderFX = pSource->m_nRenderFX;
			pMimic->m_iViewHideFlags = pSource->m_iViewHideFlags;
			//pMimic->m_fadeMinDist = pSource->m_fadeMinDist;
			//pMimic->m_fadeMaxDist = pSource->m_fadeMaxDist;
			//pMimic->m_flFadeScale = pSource->m_flFadeScale;
		}
	}

	// Remove stale mimic entities
	FOR_EACH_VEC_BACK( m_hMimicEntities, i )
	{
		if ( !m_hMimicEntities[i] )
		{
			if ( r_debug_portals_mimic.GetBool() )
				Msg( "Removing null mimic entity %i (%s)\n", i, STRING( m_hMimicEntities[i]->GetModelName() ) );
			m_hMimicEntities.Remove( i );
			continue;
		}

		EHANDLE hOwner = m_hMimicEntities[i]->GetOwnerEntity();
		if ( m_hTouchingEntities.Find( hOwner ) == m_hTouchingEntities.InvalidIndex() )
		{
			if ( r_debug_portals_mimic.GetBool() )
				Msg( "Removing orphaned mimic entity %i (%s)\n", i, STRING( m_hMimicEntities[i]->GetModelName() ) );
			m_hMimicEntities[i]->DelayedRemove();
			m_hMimicEntities.Remove( i );
			continue;
		}
	}
#endif
}

#ifdef CLIENTSIDE_PORTAL_MIMIC
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_PortalMimicEntity *C_BasePortal::GetMimicEntity( C_BaseEntity *pEntity )
{
	FOR_EACH_VEC( m_hMimicEntities, i )
	{
		if ( m_hMimicEntities[i] && m_hMimicEntities[i]->GetOwnerEntity() == pEntity )
			return m_hMimicEntities[i];
	}

	return NULL;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BasePortal::OnEntityEnterPortal( C_BaseEntity *pOther )
{
#ifdef CLIENTSIDE_PORTAL_MIMIC
	m_hTouchingEntities.AddToTail( pOther );
#endif

	pOther->m_bEnableRenderingClipPlane = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BasePortal::OnEntityExitPortal( C_BaseEntity *pOther, bool bPassedThrough )
{
#ifdef CLIENTSIDE_PORTAL_MIMIC
	m_hTouchingEntities.FindAndRemove( pOther );
#endif

	pOther->m_bEnableRenderingClipPlane = false;

	if ( bPassedThrough )
	{
		C_BasePlayer *pPlayer = ToBasePlayer( pOther );
		if ( pPlayer )
		{
			// Need to change the player's viewmodel facing
			if ( pPlayer->GetViewModel(0) )
			{
				VMatrix matPartner = MatrixThisToLinked();
				C_BaseViewModel *pVM = pPlayer->GetViewModel( 0 );

				Vector vecNewFacing;
				VectorRotate( pVM->m_vecLastFacing, matPartner.As3x4(), vecNewFacing );
				pVM->m_vecLastFacing = vecNewFacing;

				pVM->SetAbsOrigin( matPartner * pVM->GetAbsOrigin() );
				pVM->SetAbsAngles( TransformAnglesToWorldSpace(pVM->GetAbsAngles(), matPartner.As3x4() ) );
			}
		}
	}
}

//-----------------------------------------------------------------------------

bool LocalPlayerIsCloseToPortal()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( GameHasPortals() && pPlayer )
	{
		FOR_EACH_VEC( C_BasePortal::AllPortals, i )
		{
			C_BasePortal *pPortal = C_BasePortal::AllPortals[i];
			if ( (pPlayer->EyePosition() - pPortal->GetAbsOrigin()).LengthSqr() < Square( pPortal->BoundingRadius() ) )
			{
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------

#ifdef _DEBUG
CON_COMMAND( portal_debug_find_mimic_ents, "" )
{
	int nNumEnts = 0;

	Msg( "Finding mimic ents:\n" );

	const CEntInfo *pInfo = ClientEntityList().FirstEntInfo();
	for ( ;pInfo; pInfo = pInfo->m_pNext )
	{
		C_BaseEntity *ent = (C_BaseEntity *)pInfo->m_pEntity;
		if ( !ent )
			continue;

		if ( !FStrEq( ent->GetClassname(), "class C_PortalMimicEntity" ) )
			continue;

		Vector vecOrigin = ent->GetAbsOrigin();
		Msg( "\t%i - [%.2f %.2f %.2f]\n", nNumEnts, vecOrigin.x, vecOrigin.y, vecOrigin.z );

		nNumEnts++;
	}

	Msg( "Found %i ents\n", nNumEnts );
}
#endif
