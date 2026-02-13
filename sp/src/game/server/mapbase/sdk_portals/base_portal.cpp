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
#include "base_portal.h"
#include "portal_mimic_entity.h"
#include "mapbase/sdk_portals/sdk_portal_util_shared.h"
#include "saverestore_utlvector.h"
#include <collisionutils.h>
#include "eventqueue.h"
#include "envmicrophone.h"
#include "filters.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	g_debug_portal_collision( "g_debug_portal_collision", "0" );

BEGIN_DATADESC( CBasePortal )
	DEFINE_KEYFIELD( m_sPartnerName, FIELD_STRING, "partnername" ),
	//DEFINE_FIELD( m_hLinkedPortal, FIELD_EHANDLE ),
	DEFINE_KEYFIELD( m_fHalfWidth, FIELD_FLOAT, "width" ),
	DEFINE_KEYFIELD( m_fHalfHeight, FIELD_FLOAT, "height" ),
	DEFINE_KEYFIELD( m_bActivated, FIELD_BOOLEAN, "startactive" ),
	DEFINE_KEYFIELD( m_bDisablePhysics, FIELD_BOOLEAN, "PhysicsDisabled" ),
	DEFINE_KEYFIELD( m_bDisableVisuals, FIELD_BOOLEAN, "VisualsDisabled" ),

	DEFINE_KEYFIELD( m_iszPortalFilter, FIELD_STRING, "PortalFilter" ),
	//DEFINE_FIELD( m_hPortalFilter, FIELD_EHANDLE ),

	DEFINE_KEYFIELD( m_flMicRange, FIELD_FLOAT, "MicRange" ),
	DEFINE_FIELD( m_hMicrophone, FIELD_EHANDLE ),

	DEFINE_KEYFIELD( m_flScale, FIELD_FLOAT, "Scale" ),
	DEFINE_KEYFIELD( m_bScaleEntities, FIELD_BOOLEAN, "ScaleEntities" ),

	DEFINE_UTLVECTOR( m_hTouchingEntities, FIELD_EHANDLE ),

	DEFINE_INPUTFUNC( FIELD_STRING, "SetPartner", InputSetPartner ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetWidth", InputSetWidth ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetHeight", InputSetHeight ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Open", InputOpen ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Close", InputClose ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnablePhysics", InputEnablePhysics ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnableVisuals", InputEnableVisuals ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisablePhysics", InputDisablePhysics ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisableVisuals", InputDisableVisuals ),
	DEFINE_INPUTFUNC( FIELD_STRING, "SetPortalFilter", InputSetPortalFilter ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetScale", InputSetScale ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnableScaleEntities", InputEnableScaleEntities ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisableScaleEntities", InputDisableScaleEntities ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "TeleportEntity", InputTeleportEntity ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "TeleportEntityForce", InputTeleportEntityForce ),

	DEFINE_OUTPUT( m_OnOpen, "OnOpen" ),
	DEFINE_OUTPUT( m_OnClose, "OnClose" ),
	DEFINE_OUTPUT( m_OnEntityTeleportFromMe, "OnEntityTeleportFromMe" ),
	DEFINE_OUTPUT( m_OnPlayerTeleportFromMe, "OnPlayerTeleportFromMe" ),
	DEFINE_OUTPUT( m_OnEntityTeleportToMe, "OnEntityTeleportToMe" ),
	DEFINE_OUTPUT( m_OnPlayerTeleportToMe, "OnPlayerTeleportToMe" ),

	DEFINE_THINKFUNC( EntitiesTouchingThink ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CBasePortal, DT_BasePortal )
	SendPropEHandle( SENDINFO( m_hLinkedPortal ) ),
	SendPropFloat( SENDINFO( m_fHalfWidth ) ),
	SendPropFloat( SENDINFO( m_fHalfHeight ) ),
	SendPropBool( SENDINFO( m_bActivated ) ),
	//SendPropBool( SENDINFO( m_bDisablePhysics ) ),
	SendPropBool( SENDINFO( m_bDisableVisuals ) ),
	SendPropFloat( SENDINFO( m_flScale ) ),
END_SEND_TABLE()

CUtlVector<CBasePortal*>	CBasePortal::AllPortals;

CBasePortal::CBasePortal()
{
	AllPortals.AddToTail( this );

	//m_flMicRange = 1000.0f;
	m_flScale = 1.0f;
}

CBasePortal::~CBasePortal()
{
	AllPortals.FindAndRemove( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::Activate( void )
{
	BaseClass::Activate();

	CBaseEntity *pPartner = gEntList.FindEntityByName( NULL, m_sPartnerName, this );
	SetPartner( dynamic_cast<CBasePortal *>(pPartner) );
	
	if ( m_iszPortalFilter != NULL_STRING )
	{
		m_hPortalFilter = gEntList.FindEntityByName( NULL, m_iszPortalFilter );
	}

	DispatchUpdateTransmitState();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::Spawn()
{
	BaseClass::Spawn();

	SetSolid( SOLID_OBB );
	SetSolidFlags( FSOLID_TRIGGER | FSOLID_NOT_SOLID | FSOLID_CUSTOMBOXTEST | FSOLID_CUSTOMRAYTEST );
	if ( m_bDisablePhysics || !m_bActivated )
		RemoveSolidFlags( FSOLID_TRIGGER );

	SetSize( { -0.5, -m_fHalfWidth, -m_fHalfHeight }, { 0.5, m_fHalfWidth, m_fHalfHeight } );

	if ( m_flMicRange > 0.0f )
	{
		m_hMicrophone = (CEnvMicrophone*)CreateNoSpawn( "env_microphone", GetAbsOrigin(), GetAbsAngles(), this );
		if ( m_hMicrophone )
		{
			m_hMicrophone->SetName( AllocPooledString( UTIL_VarArgs( "%s_mic", STRING( GetEntityName() ) ) ) );
			m_hMicrophone->KeyValue( "MaxRange", UTIL_VarArgs( "%f", m_flMicRange ) );
			m_hMicrophone->KeyValue( "StartDisabled", m_bActivated ? "0" : "1" );
			m_hMicrophone->KeyValue( "VolumeScale", "0.25" );
			m_hMicrophone->KeyValue( "spawnflags", "175" ); // Hears all sounds
			//m_hMicrophone->KeyValue( "target", STRING( GetEntityName() ) );

			DispatchSpawn( m_hMicrophone );
			m_hMicrophone->Activate();	// Needed to get measure target
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::UpdateOnRemove( void )
{
	if ( m_hMicrophone )
	{
		UTIL_Remove( m_hMicrophone );
		m_hMicrophone = NULL;
	}
	
	CleanupMimicEntities();

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::Open( CBaseEntity *pActivator )
{
	m_bActivated = true;
	m_OnOpen.FireOutput( pActivator, this );

	if ( m_hMicrophone )
	{
		inputdata_t inputdata;
		m_hMicrophone->InputEnable( inputdata );
	}
	
	if ( !m_bDisablePhysics )
		AddSolidFlags( FSOLID_TRIGGER );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::Close( CBaseEntity *pActivator )
{
	m_bActivated = false;
	m_OnClose.FireOutput( pActivator, this );

	if ( m_hMicrophone )
	{
		inputdata_t inputdata;
		m_hMicrophone->InputDisable( inputdata );
	}

	CleanupMimicEntities();
	
	if ( !m_bDisablePhysics )
		RemoveSolidFlags( FSOLID_TRIGGER );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::SetPartner( CBasePortal *pPortalPartner )
{
	if ( m_hLinkedPortal )
		RemovePositionWatcher( this, m_hLinkedPortal );

	if ( pPortalPartner )
	{
		m_hLinkedPortal.Set( pPortalPartner );
		WatchPositionChanges( this, pPortalPartner );
		m_bCachedLinkedPortal = false;
	
		if ( m_hTouchingEntities.Count() > 0 )
			SetContextThink( &CBasePortal::EntitiesTouchingThink, gpGlobals->curtime, "EntitiesTouchingThink" );
		
		if ( m_hMicrophone )
		{
			m_hMicrophone->SetParent( pPortalPartner );
			m_hMicrophone->SetLocalOrigin( vec3_origin );
			m_hMicrophone->SetLocalAngles( vec3_angle );
			m_hMicrophone->KeyValue( "landmark", STRING( pPortalPartner->GetEntityName() ) );
			m_hMicrophone->SetSpeakerName( GetEntityName() );
		}
	}
	else
	{
		m_hLinkedPortal.Set( NULL );

		if ( m_hMicrophone )
		{
			m_hMicrophone->SetParent( NULL );

			inputdata_t inputdata;
			m_hMicrophone->InputDisable( inputdata );
		}

		CleanupMimicEntities();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBasePortal::FInViewCone( const Vector &vecSpot )
{
	Vector facingDir;
	GetVectors( &facingDir, NULL, NULL );

	return !IsBehindPortal( vecSpot, facingDir );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::InputSetPartner( inputdata_t &inputdata )
{
	m_sPartnerName = inputdata.value.StringID();

	CBaseEntity *pPartner = gEntList.FindEntityByName( NULL, m_sPartnerName, this, inputdata.pActivator, inputdata.pCaller );
	SetPartner( dynamic_cast<CBasePortal *>(pPartner) );
}

//-----------------------------------------------------------------------------
// Purpose: Sets the portal filter on the object
//-----------------------------------------------------------------------------
void CBasePortal::InputSetPortalFilter( inputdata_t &inputdata )
{
	m_iszPortalFilter = inputdata.value.StringID();
	if ( m_iszPortalFilter != NULL_STRING )
	{
		m_hPortalFilter = gEntList.FindEntityByName( NULL, m_iszPortalFilter );
	}
	else
	{
		m_hPortalFilter = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::InputTeleportEntity( inputdata_t &inputdata )
{
	TeleportEntity( inputdata.value.Entity() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::InputTeleportEntityForce( inputdata_t &inputdata )
{
	TeleportEntity( inputdata.value.Entity(), true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBasePortal::KeyValue( const char *szKeyName, const char *szValue )
{
	// prop_portal keyvalues
	if ( FStrEq( szKeyName, "halfwidth" ) )
	{
		m_fHalfWidth.Set( atof( szValue ) );
		return true;
	}
	else if ( FStrEq( szKeyName, "halfheight" ) )
	{
		m_fHalfHeight.Set( atof( szValue ) );
		return true;
	}
	else if ( FStrEq( szKeyName, "activated" ) )
	{
		m_bActivated.Set( atoi( szValue ) > 0 );
		return true;
	}
	else if ( FStrEq( szKeyName, "partner" ) ) // Flopgop's point_portal
	{
		m_sPartnerName = AllocPooledString( szValue );
		return true;
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}

bool CBasePortal::GetKeyValue( const char *szKeyName, char *szValue, int iMaxLen )
{
	// prop_portal keyvalues
	if ( FStrEq( szKeyName, "halfwidth" ) )
	{
		V_snprintf( szValue, iMaxLen, "%f", m_fHalfWidth.Get() );
		return true;
	}
	else if ( FStrEq( szKeyName, "halfheight" ) )
	{
		V_snprintf( szValue, iMaxLen, "%f", m_fHalfHeight.Get() );
		return true;
	}
	else if ( FStrEq( szKeyName, "activated" ) )
	{
		V_strncpy( szValue, m_bActivated ? "1" : "0", iMaxLen );
		return true;
	}
	else if ( FStrEq( szKeyName, "partner" ) ) // Flopgop's point_portal
	{
		V_strncpy( szValue, STRING( m_sPartnerName ), iMaxLen );
		return true;
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::PushEntitiesNearLinked( CBaseEntity *pEntity )
{
	const VMatrix matPartner = MatrixThisToLinked();
	Vector vecOrigin = matPartner * pEntity->WorldSpaceCenter(); // GetAbsOrigin

	Vector vecMyMins, vecMyMaxs;
	pEntity->CollisionProp()->WorldSpaceAABB( &vecMyMins, &vecMyMaxs );
	TransformAABB( matPartner.As3x4(), vecMyMins, vecMyMaxs,
		vecMyMins, vecMyMaxs );

	if ( m_hLinkedPortal->m_flScale != 1.0f )
	{
		m_hLinkedPortal->ApplyScale( vecOrigin );

		vecMyMins *= m_hLinkedPortal->m_flScale;
		vecMyMaxs *= m_hLinkedPortal->m_flScale;
	}

	if ( g_debug_portal_collision.GetBool() )
		NDebugOverlay::Box( vec3_origin, vecMyMins, vecMyMaxs, 128, 255, 255, 128, 0.1f );

	CBaseEntity *pPushEnt = gEntList.FirstEnt();
	for ( ; pPushEnt != NULL; pPushEnt = gEntList.NextEnt( pPushEnt ) )
	{
		if ( !pPushEnt->edict() )
			continue;

		if ( !pPushEnt->VPhysicsGetObject() || !pPushEnt->VPhysicsGetObject()->IsMoveable() || pPushEnt->GetMoveType() == MOVETYPE_PUSH )
			continue;

		Vector vecTheirMins, vecTheirMaxs;
		pPushEnt->CollisionProp()->WorldSpaceAABB( &vecTheirMins, &vecTheirMaxs );
		if ( !IsBoxIntersectingBox( vecMyMins, vecMyMaxs, vecTheirMins, vecTheirMaxs ) )
			continue;

		Vector vecDelta = pPushEnt->GetAbsOrigin() - vecOrigin;
		float flLength = VectorNormalize( vecDelta );
		pPushEnt->ApplyAbsVelocityImpulse( vecDelta * MIN( flLength, 30 ) );

		if ( pEntity->VPhysicsGetObject() && pEntity->VPhysicsGetObject()->IsMoveable() && pEntity->GetMoveType() != MOVETYPE_PUSH )
		{
			// Push ourselves away as well
			vecDelta = pEntity->GetAbsOrigin() - ( m_hLinkedPortal->MatrixThisToLinked() * pPushEnt->GetAbsOrigin() );
			flLength = VectorNormalize( vecDelta );
			pEntity->ApplyAbsVelocityImpulse( vecDelta * MIN( flLength, 30 ) );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::EntitiesTouchingThink()
{
	if ( !IsActive() || !HasPartner() )
	{
		SetContextThink( NULL, gpGlobals->curtime, "EntitiesTouchingThink" );
		return;
	}

	Vector vecForward;
	GetVectors( &vecForward, NULL, NULL );

	// Rudimentary portal physics
	// Check if any of our touching entities have passed through
	FOR_EACH_VEC_BACK( m_hTouchingEntities, i )
	{
		CBaseEntity *pEntity = m_hTouchingEntities[i];
		if ( pEntity )
		{
			if ( ShouldTeleportEntity( pEntity, vecForward ) )
			{
				TeleportEntity( pEntity );

				m_hTouchingEntities.Remove( i );
				if ( m_hTouchingEntities.Count() == 0 )
					SetContextThink( NULL, gpGlobals->curtime, "EntitiesTouchingThink" );

				continue;
			}
			else
			{
				// See if we should push stuff away near the linked portal
				PushEntitiesNearLinked( pEntity );
			}
		}
		else
		{
			m_hTouchingEntities.Remove( i );
			continue;
		}

#ifdef SERVERSIDE_PORTAL_MIMIC
		int j = 0;
		for ( ; j < m_hMimicEntities.Count(); j++ )
		{
			if ( !m_hMimicEntities[j] || m_hMimicEntities[j]->GetOwnerEntity() != pEntity )
				continue;
			break;
		}

		CPortalMimicEntity *pMimic = NULL;
		if ( j == m_hMimicEntities.Count() )
		{
			// Create a new one
			pMimic = (CPortalMimicEntity*)CreateNoSpawn( "portal_mimic_entity", m_hLinkedPortal->GetAbsOrigin(), m_hLinkedPortal->GetAbsAngles(), pEntity );
			if ( pMimic )
			{
				pMimic->InitCopy( pEntity, this );
				pMimic->MaintainCopy( pEntity );

				DispatchSpawn( pMimic );
				pMimic->DispatchUpdateTransmitState();

				EHANDLE hMimic = pMimic;
				m_hMimicEntities.AddToTail( hMimic );
			}
		}
		else
		{
			pMimic = (CPortalMimicEntity *)(m_hMimicEntities[j].Get());

			if ( pMimic )
			{
				pMimic->MaintainCopy( pEntity );
			}
		}
#endif
	}

#ifdef SERVERSIDE_PORTAL_MIMIC
	// Remove stale mimic entities
	FOR_EACH_VEC_BACK( m_hMimicEntities, i )
	{
		if ( !m_hMimicEntities[i] )
		{
			Msg( "Removing null mimic entity %i (%s)\n", i, STRING( m_hMimicEntities[i]->GetModelName() ) );
			m_hMimicEntities.Remove( i );
			continue;
		}

		EHANDLE hOwner = m_hMimicEntities[i]->GetOwnerEntity();
		if ( m_hTouchingEntities.Find( hOwner ) == m_hTouchingEntities.InvalidIndex() )
		{
			Msg( "Removing orphaned mimic entity %i (%s)\n", i, STRING( m_hMimicEntities[i]->GetModelName() ) );
			UTIL_Remove( m_hMimicEntities[i] );
			m_hMimicEntities.Remove( i );
			continue;
		}
	}
#endif

	if ( m_hTouchingEntities.Count() == 0 )
	{
		SetContextThink( NULL, gpGlobals->curtime, "EntitiesTouchingThink" );
		return;
	}

	SetContextThink( &CBasePortal::EntitiesTouchingThink, gpGlobals->curtime + TICK_INTERVAL, "EntitiesTouchingThink" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::StartTouch( CBaseEntity *pOther )
{
	BaseClass::StartTouch( pOther );

	if ( !PassesTriggerFilters( pOther ) )
		return;

	EHANDLE hOther;
	hOther = pOther;
	
	if ( m_hTouchingEntities.Find( hOther ) == m_hTouchingEntities.InvalidIndex() )
	{
		m_hTouchingEntities.AddToTail( hOther );

		CRecipientFilter filter;
		filter.AddAllPlayers();
		filter.MakeReliable();

		UserMessageBegin( filter, "EntityEnterPortal" );
			WRITE_ENTITY( pOther->entindex() );
			WRITE_ENTITY( entindex() );
			//WRITE_ENTITY( m_hLinkedPortal ? m_hLinkedPortal->entindex() : entindex() );
		MessageEnd();
		
		SetContextThink( &CBasePortal::EntitiesTouchingThink, gpGlobals->curtime, "EntitiesTouchingThink" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::EndTouch( CBaseEntity *pOther )
{
	BaseClass::EndTouch( pOther );

	EHANDLE hOther;
	hOther = pOther;
	
	if ( m_hTouchingEntities.Find( hOther ) != m_hTouchingEntities.InvalidIndex() )
	{
		m_hTouchingEntities.FindAndRemove( hOther );

		Vector vecForward;
		GetVectors( &vecForward, NULL, NULL );

		if ( ShouldTeleportEntity( pOther, vecForward ) )
		{
			// The entity passed through the portal before EntitiesTouchingThink could catch it
			TeleportEntity( pOther );
		}
		else
		{
			CRecipientFilter filter;
			filter.AddAllPlayers();
			filter.MakeReliable();

			UserMessageBegin( filter, "EntityExitPortal" );
				WRITE_ENTITY( pOther->entindex() );
				WRITE_ENTITY( entindex() );
				//WRITE_ENTITY( m_hLinkedPortal ? m_hLinkedPortal->entindex() : entindex() );
				WRITE_BOOL( false );
			MessageEnd();
		}
		
		if ( m_hTouchingEntities.Count() == 0 )
		{
			SetContextThink( NULL, gpGlobals->curtime, "EntitiesTouchingThink" );
			CleanupMimicEntities();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if this entity passes the filter criteria, false if not.
// Input  : pOther - The entity to be filtered.
//-----------------------------------------------------------------------------
bool CBasePortal::PassesTriggerFilters( CBaseEntity *pOther )
{
	if ( pOther->ClassMatches( "portal_mimic_entity" ) )
		return false;

	if ( m_hPortalFilter )
		return ((CBaseFilter *)(m_hPortalFilter.Get()))->PassesFilter( this, pOther );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBasePortal::ShouldTeleportEntity( CBaseEntity *pOther, const Vector &vecForward )
{
	//Msg("checking to teleport %s\n", pOther->GetDebugName() );
	return IsBehindPortal( pOther->GetAbsOrigin(), vecForward );
}

//------------------------------------------------------------------------------
// Copied from FindPassableSpace in client.cpp
// Searches along the direction ray in steps of "step" to see if 
// the entity position is passible.
// Used for putting the player in valid space when toggling off noclip mode.
//------------------------------------------------------------------------------
static bool FindPassablePortalSpace( CBaseEntity *pOther, int nMask, int nCollisionGroup, const Vector& direction, float step, Vector& outOrigin )
{
	Vector origin = outOrigin;
	int i;
	for ( i = 0; i < 64; i++ )
	{
		trace_t trace;
		UTIL_TraceEntity( pOther, origin, origin, nMask, pOther, nCollisionGroup, &trace );
		if ( trace.startsolid )
			VectorMA( origin, step, direction, origin );
		else
		{
			VectorCopy( origin, outOrigin );
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBasePortal::VerifyOutPosition( CBaseEntity *pOther, Vector &vecOrigin )
{
	int nCollisionGroup = COLLISION_GROUP_NONE;
	int nMask = MASK_SOLID;

	if ( pOther->IsPlayer() )
	{
		nCollisionGroup = COLLISION_GROUP_PLAYER_MOVEMENT;
		nMask = MASK_PLAYERSOLID;
	}
	else if ( pOther->IsNPC() )
	{
		nCollisionGroup = COLLISION_GROUP_NPC;
		nMask = MASK_NPCSOLID;
	}

	trace_t trace;
	UTIL_TraceEntity( pOther, vecOrigin, vecOrigin, nMask, pOther, nCollisionGroup, &trace );

	Vector vecLinkedForward, vecLinkedRight, vecLinkedUp;
	m_hLinkedPortal->GetVectors( &vecLinkedForward, &vecLinkedRight, &vecLinkedUp );

	if ( trace.startsolid )
	{
		if ( g_debug_portal_collision.GetBool() )
		{
			NDebugOverlay::BoxDirection( vecOrigin, pOther->CollisionProp()->OBBMins(), pOther->CollisionProp()->OBBMaxs(),
				vecLinkedForward, 255, 0, 0, 128, 5.0f );
		}

		// Try to move out of the solid
		if ( !FindPassablePortalSpace( pOther, nMask, nCollisionGroup, vecLinkedForward, 1, vecOrigin ) )
		{
			if ( !FindPassablePortalSpace( pOther, nMask, nCollisionGroup, vecLinkedRight, 1, vecOrigin ) )
			{
				if ( !FindPassablePortalSpace( pOther, nMask, nCollisionGroup, vecLinkedRight, -1, vecOrigin ) )		// left
				{
					if ( !FindPassablePortalSpace( pOther, nMask, nCollisionGroup, vecLinkedUp, 1, vecOrigin ) )	// up
					{
						if ( !FindPassablePortalSpace( pOther, nMask, nCollisionGroup, vecLinkedUp, -1, vecOrigin ) )	// down
						{
							//if ( !FindPassableSpace( pOther, forward, -1, oldorigin ) )	// back
							{
								Warning( "%s: Can't find unstuck position for %s\n", GetDebugName(), pOther->GetDebugName() );
								return false;
							}
						}
					}
				}
			}
		}
	}

	/*if ( m_hLinkedPortal->IsBehindPortal( vecOrigin, vecLinkedForward ) )
	{
		// Need to move the origin out a bit so that the entity doesn't immediately get teleported back
		Vector vecToLinked = (vecOrigin - m_hLinkedPortal->GetAbsOrigin());
		float flDot = DotProduct( vecLinkedForward, vecToLinked.Normalized() );

		Vector vecToAdd = vecLinkedForward * ((vecToLinked.Length() * flDot) + 0.1f);
		vecOrigin += vecToAdd;

		DevMsg( "%s: Had to move %s %.2f units from behind partner %s\n", GetDebugName(), pOther->GetDebugName(), vecToAdd.Length(), m_hLinkedPortal->GetDebugName() );
	}*/

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::TeleportEntity( CBaseEntity *pOther, bool bForce )
{
	//Msg( "(%.2f) %s: Teleporting entity %s\n", gpGlobals->curtime, GetDebugName(), pOther->GetDebugName() );

#ifdef SERVERSIDE_PORTAL_MIMIC
	// First, remove any potential mimic entity that belongs to this
	FOR_EACH_VEC( m_hMimicEntities, i )
	{
		if ( m_hMimicEntities[i] && m_hMimicEntities[i]->GetOwnerEntity() == pOther)
		{
			UTIL_Remove( m_hMimicEntities[i].Get() );
			m_hMimicEntities.Remove( i );
			break;
		}
	}
#endif

	// Teleport this entity to the other portal
	const VMatrix matPartner = MatrixThisToLinked();
	Vector vecOrigin = matPartner * pOther->GetAbsOrigin();
	QAngle angAngles = TransformAnglesToWorldSpace( pOther->GetAbsAngles(), matPartner.As3x4() );

	if ( m_hLinkedPortal->m_flScale != 1.0f )
	{
		// Prevent a hitch
		if ( pOther->IsPlayer() )
			vecOrigin += (pOther->EyePosition() - pOther->GetAbsOrigin());

		m_hLinkedPortal->ApplyScale( vecOrigin );
	}

	Vector vecInVelocity;
	if ( pOther->GetMoveType() == MOVETYPE_VPHYSICS && pOther->VPhysicsGetObject() )
	{
		pOther->VPhysicsGetObject()->GetVelocity( &vecInVelocity, NULL );
	}
	else
	{
		vecInVelocity = pOther->GetAbsVelocity();
	}

	Vector vecVelocity;
	VectorRotate( vecInVelocity, matPartner.As3x4(), vecVelocity );

	// Do any operations needed on the teleport params
	// (Portal didn't seem to need this, but it scales better for generalized cases)
	pOther->PreEnterPortal( this, vecOrigin, angAngles, vecVelocity );

	// Correct position if needed
	if ( !bForce )
	{
		if ( !VerifyOutPosition( pOther, vecOrigin ) )
		{
			// Just teleport to center
			vecOrigin = m_hLinkedPortal->GetAbsOrigin() + ( pOther->GetAbsOrigin() - pOther->WorldSpaceCenter() );

			Vector vecLinkedForward;
			m_hLinkedPortal->GetVectors( &vecLinkedForward, NULL, NULL );
			vecOrigin += (vecLinkedForward * 0.1f);
		}
	}

	pOther->Teleport( &vecOrigin, &angAngles, &vecVelocity );

	OnTeleportFrom( pOther );
	m_hLinkedPortal->OnTeleportTo( pOther );

	CRecipientFilter filter;
	filter.AddAllPlayers();
	filter.MakeReliable();

	UserMessageBegin( filter, "EntityExitPortal" );
		WRITE_ENTITY( pOther->entindex() );
		WRITE_ENTITY( entindex() );
		//WRITE_ENTITY( m_hLinkedPortal ? m_hLinkedPortal->entindex() : entindex() );
		WRITE_BOOL( true );
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::OnTeleportFrom( CBaseEntity *pOther )
{
	// Hack for physics objects
	if ( pOther->GetMoveType() == MOVETYPE_VPHYSICS && pOther->VPhysicsGetObject() )
		pOther->SetAbsVelocity( vec3_origin );
	
	if ( pOther->IsPlayer() )
	{
		m_OnPlayerTeleportFromMe.FireOutput( pOther, this );
	}

	m_OnEntityTeleportFromMe.FireOutput( pOther, this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::OnTeleportTo( CBaseEntity *pOther )
{
	if ( pOther->IsPlayer() )
	{
		m_OnPlayerTeleportToMe.FireOutput( pOther, this );
	}

	m_OnEntityTeleportToMe.FireOutput( pOther, this );

	if ( m_bScaleEntities && pOther->GetBaseAnimating() )
	{
		pOther->GetBaseAnimating()->SetModelScale( pOther->GetBaseAnimating()->GetModelScale() * m_flScale );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBasePortal::CleanupMimicEntities()
{
#ifdef SERVERSIDE_PORTAL_MIMIC
	// Remove any leftover mimic entities
	FOR_EACH_VEC_BACK( m_hMimicEntities, i )
	{
		if ( m_hMimicEntities[i] )
		{
			UTIL_Remove( m_hMimicEntities[i].Get() );
		}
		m_hMimicEntities.Remove( i );
	}
#endif
}
