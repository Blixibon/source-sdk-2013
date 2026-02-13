//========= Mapbase - https://github.com/mapbase-source/source-sdk-2013 ============//
//
// Purpose:	Portal utility functions recreated using public Source SDK 2013 code only
// 
//			Based directly on SDK Stencil Portals by Flopgop
//			https://github.com/Flopgop/source-sdk-portals
//
// Author:	Blixibon with code from Flopgop as a basis
//
//===========================================================================//

#include "cbase.h"
#include "sdk_portal_util_shared.h"
#include "ai_debug_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool GameAllowsPortals()
{
	// TODO: Gameinfo key
	return true;
}

bool GameHasPortals()
{
	return CBasePortal::AllPortals.Count() > 0;
}

CBasePortal *UTIL_Portal_FirstAlongRay( const Ray_t &ray, float &flPortalFraction )
{
	FOR_EACH_VEC( CBasePortal::AllPortals, i )
	{
		CBasePortal *pPortal = CBasePortal::AllPortals[i];
		if ( pPortal->IsDormant() || !pPortal->IsActive() || !pPortal->HasPartner() )
			continue;

		trace_t tr;
		if ( pPortal->TestCollision( ray, MASK_ALL, tr ) )
		{
			flPortalFraction = tr.fraction;
			return pPortal;
		}
	}

	return NULL;
}

void UTIL_Portal_TraceRay( const Ray_t &ray, int mask, const IHandleEntity *ignore, int collisionGroup, trace_t *tr )
{
	CTraceFilterSimple traceFilter( ignore, collisionGroup );
	UTIL_Portal_TraceRay( ray, mask, &traceFilter, tr );
}

void UTIL_Portal_TraceRay( const Ray_t &ray, int mask, ITraceFilter *pFilter, trace_t *tr )
{
	CBasePortal *pBestPortal = NULL;
	trace_t portalTr;
	FOR_EACH_VEC( CBasePortal::AllPortals, i )
	{
		CBasePortal *pPortal = CBasePortal::AllPortals[i];
		if ( pPortal->IsDormant() || !pPortal->IsActive() || !pPortal->HasPartner() )
			continue;

		if ( pPortal->TestCollision( ray, MASK_ALL, portalTr ) )
		{
			pBestPortal = pPortal;
			break;
		}
	}

	if ( !pBestPortal )
	{
		// No portal, do a regular trace
		enginetrace->TraceRay( ray, mask, pFilter, tr );
		return;
	}

	// First, trace between the ray and the portal
	AI_TraceLine( ray.m_Start, portalTr.endpos, mask, pFilter, tr );
	if ( tr->fraction < 1.0f )
	{
		// Hit something before going through the portal
		return;
	}

	// Nothing in between, so go through the portal
	Vector vecEndPortalStart;
	Vector vecEndPortalEnd;
	UTIL_Portal_PointTransform( pBestPortal->MatrixThisToLinked(), portalTr.endpos, vecEndPortalStart );
	UTIL_Portal_PointTransform( pBestPortal->MatrixThisToLinked(), ray.m_Start + ray.m_Delta, vecEndPortalEnd );

	// Now do a new trace from the portal
	AI_TraceLine( vecEndPortalStart, vecEndPortalEnd, mask, pFilter, tr );
}

bool UTIL_Portal_TraceRay_Bullets( const CBasePortal *pPortal, const Ray_t &ray, int mask, ITraceFilter *pFilter, trace_t *tr )
{
	trace_t portalTr;
	if ( pPortal )
		const_cast<CBasePortal*>(pPortal)->TestCollision( ray, MASK_ALL, portalTr );
	else
	{
		// No portal
		enginetrace->TraceRay( ray, mask, pFilter, tr );
		return false;
	}

	// First, trace between the ray and the portal
	AI_TraceLine( ray.m_Start, portalTr.endpos, mask, pFilter, tr );
	if ( tr->fraction < 1.0f )
	{
		// Hit something before going through the portal
		return false;
	}

	// Nothing in between, so go through the portal
	Vector vecEndPortalStart;
	Vector vecEndPortalEnd;
	UTIL_Portal_PointTransform( pPortal->MatrixThisToLinked(), portalTr.endpos, vecEndPortalStart );
	UTIL_Portal_PointTransform( pPortal->MatrixThisToLinked(), ray.m_Start + ray.m_Delta, vecEndPortalEnd );

	// Now do a new trace from the portal
	AI_TraceLine( vecEndPortalStart, vecEndPortalEnd, mask, pFilter, tr );
	return true;
}

bool UTIL_Portal_TraceRayHull_Bullets( const CBasePortal *pPortal, const Ray_t &ray, const Vector &hullMin, const Vector &hullMax, int mask, ITraceFilter *pFilter, trace_t *tr )
{
	trace_t portalTr;
	if ( pPortal )
		const_cast<CBasePortal*>(pPortal)->TestCollision( ray, MASK_ALL, portalTr );
	else
	{
		// No portal
		enginetrace->TraceRay( ray, mask, pFilter, tr );
		return false;
	}

	// First, trace between the ray and the portal
	AI_TraceHull( ray.m_Start, portalTr.endpos, hullMin, hullMax, mask, pFilter, tr );
	if ( tr->fraction < 1.0f )
	{
		// Hit something before going through the portal
		return false;
	}

	// Nothing in between, so go through the portal
	Vector vecEndPortalStart;
	Vector vecEndPortalEnd;
	UTIL_Portal_PointTransform( pPortal->MatrixThisToLinked(), portalTr.endpos, vecEndPortalStart );
	UTIL_Portal_PointTransform( pPortal->MatrixThisToLinked(), ray.m_Start + ray.m_Delta, vecEndPortalEnd );

	// Now do a new trace from the portal
	AI_TraceHull( vecEndPortalStart, vecEndPortalEnd, hullMin, hullMax, mask, pFilter, tr );
	return true;
}

float UTIL_Portal_DistanceThroughPortalSqr( CBasePortal *pPortal, const Vector &vecSrc, const Vector &vecDest )
{
	if ( !pPortal->GetPartner() )
		return FLT_MAX;

	Vector vecOut;
	UTIL_Portal_PointTransform( pPortal->MatrixThisToLinked(), vecSrc, vecOut );
	return (vecOut - vecDest).LengthSqr();
}

void UTIL_Portal_PointTransform( const VMatrix &matToPortal, const Vector &vecIn, Vector &vecOut )
{
	vecOut = matToPortal * vecIn;
}

void UTIL_Portal_AngleTransform( const VMatrix &matToPortal, const QAngle &angIn, QAngle &angOut )
{
	angOut = TransformAnglesToWorldSpace( angIn, matToPortal.As3x4() );
}

void UTIL_Portal_VectorTransform( const VMatrix &matToPortal, const Vector &vecIn, Vector &vecOut )
{
	VectorRotate( vecIn, matToPortal.As3x4(), vecOut );
}

void UTIL_Portal_Matrix3x4Transform( const VMatrix &matToPortal, const matrix3x4_t &matIn, matrix3x4_t &matOut )
{
	ConcatTransforms( matIn, matToPortal.As3x4(), matOut );
}
