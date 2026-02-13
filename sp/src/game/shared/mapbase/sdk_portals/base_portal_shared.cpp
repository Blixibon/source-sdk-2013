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
#include "base_portal_shared.h"
#include "sdk_portal_util_shared.h"
#include <collisionutils.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const VMatrix &CBasePortal::MatrixThisToLinked() const
{
	if ( m_bCachedLinkedPortal )
		return m_matCachedThisToLinked;

	static bool bSetRotation = false;
	static VMatrix rotation;
	if ( !bSetRotation )
	{
		rotation.Identity();
		rotation[0][0] = -1.0f;
		rotation[1][1] = -1.0f;
		bSetRotation = true;
	}

	Vector myPos = GetAbsOrigin();
	Vector myFwd, myRht, myUp;
	GetVectors(&myFwd, &myRht, &myUp);

	VMatrix myModel(
		myFwd,
		-myRht,
		myUp,
		GetAbsOrigin()
	);

	Vector friendPos = m_hLinkedPortal->GetAbsOrigin();
	Vector friendFwd, friendRht, friendUp;
	m_hLinkedPortal->GetVectors(&friendFwd, &friendRht, &friendUp);
	VMatrix friendModel(
		friendFwd,
		-friendRht,
		friendUp,
		friendPos
	);

	// HACKHACK - Some functions that call MatrixThisToLinked are const
	const_cast<CBasePortal*>(this)->m_matCachedThisToLinked = (VMatrix(friendModel) * rotation * VMatrix(myModel).InverseTR());
	const_cast<CBasePortal*>(this)->m_bCachedLinkedPortal = true;

	return m_matCachedThisToLinked;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::NotifyPositionChanged( CBaseEntity *pEntity )
{
	if ( pEntity == m_hLinkedPortal.Get() )
	{
		m_bCachedLinkedPortal = false;
		m_hLinkedPortal->m_bCachedLinkedPortal = false;
	}
#ifndef CLIENT_DLL
	else if ( pEntity == this )
	{
		Vector myFwd, myRht, myUp;
		GetVectors( &myFwd, &myRht, &myUp );

		// Recalculate corners
		m_vPortalCorners[0] = GetAbsOrigin() + ( myRht * m_fHalfWidth ) + ( myUp * m_fHalfHeight );
		m_vPortalCorners[1] = GetAbsOrigin() + ( myRht * m_fHalfWidth ) - ( myUp * m_fHalfHeight );
		m_vPortalCorners[2] = GetAbsOrigin() - ( myRht * m_fHalfWidth ) + ( myUp * m_fHalfHeight );
		m_vPortalCorners[3] = GetAbsOrigin() - ( myRht * m_fHalfWidth ) - ( myUp * m_fHalfHeight );

		// For now, just assign normal to plane
		m_plane_Origin.normal = myFwd;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::ApplyScale( Vector &vecOut )
{
	// TODO: Better way of doing this?
	vecOut -= GetAbsOrigin();
	VectorScale( vecOut, m_flScale, vecOut );
	vecOut += GetAbsOrigin();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBasePortal::IsBehindPortal( const Vector &vecOrigin, const Vector &vecForward )
{
	Vector vecToEnt = (vecOrigin - GetAbsOrigin());
	VectorNormalize( vecToEnt );

	//Msg( "%s: %.2f; [%f %f %f]\n", GetDebugName(), DotProduct( vecToEnt, vecForward ), vecOrigin.x, vecOrigin.y, vecOrigin.z );

	if ( DotProduct( vecToEnt, vecForward ) < 0.0f )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBasePortal::TestCollision( const Ray_t& ray, unsigned int fContentsMask, trace_t& tr )
{
	return IntersectRayWithOBB( ray, this->EntityToWorldTransform(), { -0.5, -this->m_fHalfWidth, -this->m_fHalfHeight }, { 0.5, this->m_fHalfWidth, this->m_fHalfHeight }, 0.0f, &tr );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBasePortal::IsEntityTouching( CBaseEntity *pEntity )
{
	EHANDLE hEnt = pEntity;
	return m_hTouchingEntities.Find( hEnt ) != m_hTouchingEntities.InvalidIndex();
}
