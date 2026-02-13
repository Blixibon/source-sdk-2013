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

#ifndef SDK_PORTAL_UTIL_SHARED_H
#define SDK_PORTAL_UTIL_SHARED_H

#include "base_portal_shared.h"

// Stubbed portal classes for utility functions
#define CProp_Portal_Shared CBasePortal
#define CProp_Portal CBasePortal

extern bool GameAllowsPortals();
extern bool GameHasPortals();

extern CBasePortal *UTIL_Portal_FirstAlongRay( const Ray_t &ray, float &flPortalFraction );
extern CBasePortal *UTIL_Portal_TraceRay_Beam( const Ray_t &ray, int mask, ITraceFilter *pFilter, float &flPortalFraction );

extern void UTIL_Portal_TraceRay( const Ray_t &ray, int mask, const IHandleEntity *ignore, int collisionGroup, trace_t *tr );
extern void UTIL_Portal_TraceRay( const Ray_t &ray, int mask, ITraceFilter *pFilter, trace_t *tr );
extern bool UTIL_Portal_TraceRay_Bullets( const CBasePortal *pPortal, const Ray_t &ray, int mask, ITraceFilter *pFilter, trace_t *tr );
extern bool UTIL_Portal_TraceRayHull_Bullets( const CBasePortal *pPortal, const Ray_t &ray, const Vector &hullMin, const Vector &hullMax, int mask, ITraceFilter *pFilter, trace_t *tr ); // custom

extern float UTIL_Portal_DistanceThroughPortalSqr( CBasePortal *pPortal, const Vector &vecSrc, const Vector &vecDest );

extern void UTIL_Portal_PointTransform( const VMatrix &matToPortal, const Vector &vecIn, Vector &vecOut );
extern void UTIL_Portal_AngleTransform( const VMatrix &matToPortal, const QAngle &angIn, QAngle &angOut ); // custom
extern void UTIL_Portal_VectorTransform( const VMatrix &matToPortal, const Vector &vecIn, Vector &vecOut );
extern void UTIL_Portal_Matrix3x4Transform( const VMatrix &matToPortal, const matrix3x4_t &matIn, matrix3x4_t &matOut ); // custom

extern void UTIL_Portal_TraceEntity( CBaseEntity *pEntity, const Vector &vecAbsStart, const Vector &vecAbsEnd, unsigned int mask, trace_t *ptr );

#endif
