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

#ifndef C_BASE_PORTAL_H
#define C_BASE_PORTAL_H

#include "c_baseentity.h"

#define CBasePortal C_BasePortal

class C_PortalMimicEntity;

class C_BasePortal : public C_BaseEntity
{
	DECLARE_CLASS( C_BasePortal, C_BaseEntity );
	DECLARE_CLIENTCLASS();
public:
	C_BasePortal();
	~C_BasePortal();

	inline bool HasPartner() const { return (m_hLinkedPortal != NULL); }
	inline C_BasePortal *GetPartner() const { return m_hLinkedPortal; }

	inline float GetWidth() const { return m_fHalfWidth; }
	inline float GetHeight() const { return m_fHalfHeight; }
	inline bool IsActive() const { return m_bActivated; }
	inline bool VisualsDisabled() const { return m_bDisableVisuals; }
	inline float GetScale() const { return m_flScale; }

	void OnDataChanged( DataUpdateType_t type );
	void UpdateOnRemove();

	void DrawStencil();
	void DrawStencilDepth();
	void DrawFakePlane();
	void Draw( const IMaterial *pMaterial, float forwardOffset = 0.1f );

	void Simulate();
	void OnEntityEnterPortal( C_BaseEntity *pOther );
	void OnEntityExitPortal( C_BaseEntity *pOther, bool bPassedThrough );

	// Portal stubs
	const VMatrix &MatrixThisToLinked() const;

	void			ApplyScale( Vector &vecOut );

	bool			IsBehindPortal( const Vector &vecOrigin, const Vector &vecForward );
	virtual bool	TestCollision( const Ray_t &ray, unsigned int fContentsMask, trace_t &tr );

	bool					IsEntityTouching( CBaseEntity *pEntity );
#ifdef CLIENTSIDE_PORTAL_MIMIC
	C_PortalMimicEntity		*GetMimicEntity( CBaseEntity *pEntity );
#endif

	virtual void NotifyPositionChanged( CBaseEntity *pEntity );

	static CUtlVector<CBasePortal*>	AllPortals;

private:
	CHandle<C_BasePortal>	m_hLinkedPortal, m_hOldLinkedPortal;
	float m_fHalfWidth, m_fHalfHeight, m_flScale;
	bool m_bActivated, m_bDisableVisuals;

#ifdef CLIENTSIDE_PORTAL_MIMIC
	CUtlVector<EHANDLE>							m_hTouchingEntities;	// Entities touching the portal
	CUtlVector<CHandle<C_PortalMimicEntity>>	m_hMimicEntities;		// Entities on the other portal that reflect the ones touching this one
#endif

	// Caching the position of the linked portal
	bool			m_bCachedLinkedPortal;
	VMatrix			m_matCachedThisToLinked;
};

#endif
