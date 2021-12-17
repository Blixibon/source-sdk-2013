//==============================================================================
//
// Purpose: Osprey helicopter created from scratch based on HL2 NPCs.
// 
//==============================================================================

#include "cbase.h"
#include "npc_bm_osprey.h"
#include "soundenvelope.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( npc_bm_osprey, CNPC_BM_Osprey );

BEGIN_DATADESC( CNPC_BM_Osprey )

	DEFINE_FIELD( m_vecAngAcceleration, FIELD_VECTOR ),
	
	DEFINE_FIELD( m_bPlaneMode, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flPlaneTransitionStartTime, FIELD_TIME ),
	DEFINE_FIELD( m_flPlaneTransitionEndTime, FIELD_TIME ),

	DEFINE_FIELD( m_soldiersToDrop, FIELD_INTEGER ),
	DEFINE_FIELD( m_iDropState, FIELD_INTEGER ),
	DEFINE_FIELD( m_iLandState, FIELD_INTEGER ),

	DEFINE_FIELD( m_flTimeTakeOff, FIELD_TIME ),
	DEFINE_FIELD( m_flNextTroopSpawnAttempt, FIELD_TIME ),
	DEFINE_FIELD( m_flDropDelay, FIELD_TIME ),
	DEFINE_FIELD( m_flTimeNextAttack, FIELD_TIME ),
	DEFINE_FIELD( m_flLastTime, FIELD_TIME ),

	DEFINE_INPUTFUNC( FIELD_FLOAT, "EnterPlaneMode", InputEnterPlaneMode ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "ExitPlaneMode", InputExitPlaneMode ),

	DEFINE_INPUTFUNC( FIELD_VOID, "BeginRappellingGrunts", InputBeginRappellingGrunts ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RemoveGrunts", InputRemoveGrunts ),
	DEFINE_INPUTFUNC( FIELD_VOID, "WaitHereTillReady", InputWaitHereTillReady ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RappelToTarget", InputRappelToTarget ),
	DEFINE_INPUTFUNC( FIELD_VOID, "KillRappelingGrunts", InputKillRappelingGrunts ),

	DEFINE_OUTPUT( m_OnReadyToMoveDeployZone, "OnReadyToMoveDeployZone" ),
	DEFINE_OUTPUT( m_OnReadyToRetreat, "OnReadyToRetreat" ),
	DEFINE_OUTPUT( m_OnSpawnNPC, "OnSpawnNPC" ),

	DEFINE_SOUNDPATCH( m_pNearRotorSound ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CNPC_BM_Osprey::CNPC_BM_Osprey()
{
	m_flMaxSpeed = OSPREY_MAX_SPEED;
	m_flMaxSpeedFiring = OSPREY_MAX_SPEED;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CNPC_BM_Osprey::~CNPC_BM_Osprey()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_BM_Osprey::Precache( void )
{
	if (GetModelName() == NULL_STRING)
		SetModelName( AllocPooledString( OSPREY_MODEL ) );

	PrecacheModel( STRING(GetModelName()) );

	PrecacheScriptSound( OSPREY_HOVER_SOUND );
	PrecacheScriptSound( OSPREY_HOVER_SOUND_FAR );
	PrecacheScriptSound( OSPREY_ROTOR_BLAST_SOUND );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_BM_Osprey::Spawn( void )
{
	if (GetModelName() == NULL_STRING)
		SetModelName( AllocPooledString( OSPREY_MODEL ) );

	SetModel( STRING(GetModelName()) );

	BaseClass::Spawn();

	CreateVPhysics();

	SetActivity( ACT_IDLE );
	SetPoseParameter( m_poseSideDoor, 100.0f );
	SetPoseParameter( m_poseGear, 100.0f );
}

//------------------------------------------------------------------------------
// Purpose: Create our rotor sound
//------------------------------------------------------------------------------
void CNPC_BM_Osprey::InitializeRotorSound( void )
{
	CSoundEnvelopeController& controller = CSoundEnvelopeController::GetController();

	if ( !m_pRotorSound )
	{
		CPASAttenuationFilter filter( this );

		m_pRotorSound = controller.SoundCreate( filter, entindex(), OSPREY_HOVER_SOUND_FAR );
		m_pNearRotorSound = controller.SoundCreate( filter, entindex(), OSPREY_HOVER_SOUND );
		m_pRotorBlast = controller.SoundCreate( filter, entindex(), OSPREY_ROTOR_BLAST_SOUND );
	}
	else
	{
		Assert(m_pRotorSound);
		Assert(m_pNearRotorSound);
		Assert(m_pRotorBlast);
	}
	
	if ( m_pNearRotorSound )
	{
		controller.Play( m_pNearRotorSound, 0.0, 100 );
	}

	BaseClass::InitializeRotorSound();
}

//------------------------------------------------------------------------------
// Updates the rotor wash volume
//------------------------------------------------------------------------------
void CNPC_BM_Osprey::UpdateRotorWashVolume()
{
	BaseClass::UpdateRotorWashVolume();

	if ( m_pNearRotorSound )
	{
		CSoundEnvelopeController& controller = CSoundEnvelopeController::GetController();
		float flVolDelta = GetRotorVolume() - controller.SoundGetVolume( m_pNearRotorSound );
		if (flVolDelta)
		{
			// We can change from 0 to 1 in 3 seconds. 
			// Figure out how many seconds flVolDelta will take.
			float flRampTime = fabs( flVolDelta ) * 3.0f;
			controller.SoundChangeVolume( m_pNearRotorSound, GetRotorVolume(), flRampTime );
		}
	}
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_BM_Osprey::UpdateRotorSoundPitch( int iPitch )
{
	BaseClass::UpdateRotorSoundPitch( iPitch );

	if ( m_pNearRotorSound )
	{
		CSoundEnvelopeController& controller = CSoundEnvelopeController::GetController();
		controller.SoundChangePitch( m_pNearRotorSound, iPitch , 0.1 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_BM_Osprey::StopLoopingSounds()
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	if ( m_pNearRotorSound )
	{
		controller.SoundDestroy( m_pNearRotorSound );
		m_pNearRotorSound = NULL;
	}

	BaseClass::StopLoopingSounds();
}

//-----------------------------------------------------------------------------
// Purpose: Cache whatever pose parameters we intend to use
//-----------------------------------------------------------------------------
void CNPC_BM_Osprey::PopulatePoseParameters( void )
{
	m_poseRightRudder = LookupPoseParameter("right_back_rudder");
	m_poseMiddleRudder = LookupPoseParameter("middle_back_rudder");
	m_poseLeftRudder = LookupPoseParameter("left_back_rudder");
	m_poseLeftWingFlap = LookupPoseParameter("left_wingflap");
	m_poseRightWingFlap = LookupPoseParameter("right_wingflap");
	m_poseLeftNacelle = LookupPoseParameter("left_nacelle");
	m_poseRightNacelle = LookupPoseParameter("right_nacelle");
	m_poseSideDoor = LookupPoseParameter("pose_sidedoor");
	m_poseBackDoor = LookupPoseParameter("pose_backdoor");
	m_poseGear = LookupPoseParameter("pose_gear");

	BaseClass::PopulatePoseParameters();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_BM_Osprey::InputEnterPlaneMode( inputdata_t &inputdata )
{
	if (m_bPlaneMode)
		return;

	m_bPlaneMode = true;
	m_flPlaneTransitionStartTime = gpGlobals->curtime;
	m_flPlaneTransitionEndTime = inputdata.value.Float() > 0.0f ? gpGlobals->curtime + inputdata.value.Float() : gpGlobals->curtime + OSPREY_PLANE_DEFAULT_TRANSITION_TIME;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_BM_Osprey::InputExitPlaneMode( inputdata_t &inputdata )
{
	if (!m_bPlaneMode)
		return;

	m_bPlaneMode = false;
	m_flPlaneTransitionStartTime = gpGlobals->curtime;
	m_flPlaneTransitionEndTime = inputdata.value.Float() > 0.0f ? gpGlobals->curtime + inputdata.value.Float() : gpGlobals->curtime + OSPREY_PLANE_DEFAULT_TRANSITION_TIME;
}

//------------------------------------------------------------------------------
// Purpose : 
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_BM_Osprey::Flight( void )
{
	Vector forward, right, up;
	GetVectors( &forward, &right, &up );

	float finspeed = 0;
	float swayspeed = 0;
	Vector vecImpulse = vec3_origin;

	// Osprey plane stuff
	// 
	// 1.0 = Plane mode, 0.0 = Hover mode
	float flPlaneTransition = m_bPlaneMode ? 1.0f : 0.0f;
	bool bInTransition = m_flPlaneTransitionEndTime > gpGlobals->curtime;
	
	// Only run the flight model in some flight states
	bool bRunFlight = ( GetLandingState() == LANDING_NO || 
							GetLandingState() == LANDING_LEVEL_OUT || 
							GetLandingState() == LANDING_LIFTOFF ||
							GetLandingState() == LANDING_SWOOPING ||
							GetLandingState() == LANDING_DESCEND ||
							GetLandingState() == LANDING_HOVER_LEVEL_OUT ||
							GetLandingState() == LANDING_HOVER_DESCEND );

	if ( bRunFlight )
	{
		if( GetFlags() & FL_ONGROUND )
		{
			// This would be really bad.
			SetGroundEntity( NULL );
		}

		if (bInTransition)
		{
			float flTransitionTime = m_flPlaneTransitionEndTime - m_flPlaneTransitionStartTime;
			float flTransitionProgress;
			if (m_bPlaneMode)
				flTransitionProgress = gpGlobals->curtime - m_flPlaneTransitionStartTime;
			else
				flTransitionProgress = m_flPlaneTransitionEndTime - gpGlobals->curtime;

			flPlaneTransition = SimpleSplineRemapVal( flTransitionProgress, 0.0f, flTransitionTime, 0.0f, 1.0f );
			Msg( "Osprey transition: %f\n", flPlaneTransition );
		}

		// calc desired acceleration
		float dt = 1.0f;

		Vector	accel;
		float	accelRate = OSPREY_ACCEL_RATE;
		float	maxSpeed = GetMaxSpeed();

		if (bInTransition)
		{
			// Lerp between the regular acceleration and plane mode acceleration
			accelRate = Lerp( flPlaneTransition, OSPREY_ACCEL_RATE, OSPREY_PLANE_ACCEL_RATE );
			maxSpeed = Lerp( flPlaneTransition, maxSpeed, OSPREY_PLANE_MAX_SPEED );
		}
		else if (m_bPlaneMode)
		{
			// Plane mode has higher acceleration
			accelRate = OSPREY_PLANE_ACCEL_RATE;
			maxSpeed = OSPREY_PLANE_MAX_SPEED;
		}

		if ( m_lifeState == LIFE_DYING )
		{
			accelRate *= 5.0;
			maxSpeed *= 5.0;
		}

		float flCurrentSpeed = GetAbsVelocity().Length();
		float flDist = MIN( flCurrentSpeed + accelRate, maxSpeed );

		Vector deltaPos;
		//if ( GetLandingState() == LANDING_SWOOPING )
		//{
		//	// Move directly to the target point
		//	deltaPos = GetDesiredPosition();
		//}
		//else
		{
			ComputeActualTargetPosition( flDist, dt, 0.0f, &deltaPos );
		}
		deltaPos -= GetAbsOrigin();

		//NDebugOverlay::Line( GetAbsOrigin(), GetAbsOrigin() + deltaPos, 0, 255, 0, true, 0.1f );

		// calc goal linear accel to hit deltaPos in dt time.
		accel.x = 2.0 * (deltaPos.x - GetAbsVelocity().x * dt) / (dt * dt);
		accel.y = 2.0 * (deltaPos.y - GetAbsVelocity().y * dt) / (dt * dt);
		accel.z = 2.0 * (deltaPos.z - GetAbsVelocity().z * dt + 0.5 * 384 * dt * dt) / (dt * dt);
		
		float flDistFromPath = 0.0f;
		Vector vecPoint, vecDelta;
		if ( IsOnPathTrack() /*&& GetLandingState() == LANDING_NO*/ )
		{
			// Also, add in a little force to get us closer to our current line segment if we can
			ClosestPointToCurrentPath( &vecPoint );
			VectorSubtract( vecPoint, GetAbsOrigin(), vecDelta );
 			flDistFromPath = VectorNormalize( vecDelta );
			if ( flDistFromPath > 200 )
			{
				// Strongly constrain to an n unit pipe around the current path
				// by damping out all impulse forces that would push us further from the pipe
				float flAmount = (flDistFromPath - 200) / 200.0f;
				flAmount = clamp( flAmount, 0, 1 );
				VectorMA( accel, flAmount * 200.0f, vecDelta, accel );
			}
		}

		// don't fall faster than 0.2G or climb faster than 2G
		accel.z = clamp( accel.z, 384 * 0.2, 384 * 2.0 );

		Vector goalUp = accel;
		VectorNormalize( goalUp );

		float goalPitch;
		float goalYaw;
		float goalRoll;

		// calc goal orientation to hit linear accel forces
		// TODO: Plane mode
		if (m_bPlaneMode)
		{
			// Plane mode has different yaw and pitch changes
			goalPitch = accel.z * 0.05f;
			goalYaw = UTIL_VecToYaw( GetGoalOrientation() );
			goalRoll = RAD2DEG( asin( DotProduct( right, goalUp ) ) );
		}
		else
		{
			goalPitch = RAD2DEG( asin( DotProduct( forward, goalUp ) ) );
			goalYaw = UTIL_VecToYaw( m_vecDesiredFaceDir );
			goalRoll = RAD2DEG( asin( DotProduct( right, goalUp ) ) );
		}

		// clamp goal orientations
		goalPitch = clamp( goalPitch, -25, 25 );
		goalRoll = clamp( goalRoll, -25, 25 );

		// calc angular accel needed to hit goal pitch in dt time.
		dt = 0.6;
		QAngle goalAngAccel;
		goalAngAccel.x = 2.0 * (AngleDiff( goalPitch, AngleNormalize( GetLocalAngles().x ) ) - GetLocalAngularVelocity().x * dt) / (dt * dt);
		goalAngAccel.y = 2.0 * (AngleDiff( goalYaw, AngleNormalize( GetLocalAngles().y ) ) - GetLocalAngularVelocity().y * dt) / (dt * dt);
		goalAngAccel.z = 2.0 * (AngleDiff( goalRoll, AngleNormalize( GetLocalAngles().z ) ) - GetLocalAngularVelocity().z * dt) / (dt * dt);

		goalAngAccel.x = clamp( goalAngAccel.x, -300, 300 );
		//goalAngAccel.y = clamp( goalAngAccel.y, -60, 60 );
		goalAngAccel.y = clamp( goalAngAccel.y, -120, 120 );
		goalAngAccel.z = clamp( goalAngAccel.z, -300, 300 );

		// limit angular accel changes to simulate mechanical response times
		dt = 0.1;
		QAngle angAccelAccel;
		angAccelAccel.x = (goalAngAccel.x - m_vecAngAcceleration.x) / dt;
		angAccelAccel.y = (goalAngAccel.y - m_vecAngAcceleration.y) / dt;
		angAccelAccel.z = (goalAngAccel.z - m_vecAngAcceleration.z) / dt;

		angAccelAccel.x = clamp( angAccelAccel.x, -1000, 1000 );
		angAccelAccel.y = clamp( angAccelAccel.y, -1000, 1000 );
		angAccelAccel.z = clamp( angAccelAccel.z, -1000, 1000 );

		m_vecAngAcceleration += angAccelAccel * 0.1;

		// DevMsg( "pitch %6.1f (%6.1f:%6.1f)  ", goalPitch, GetLocalAngles().x, m_vecAngVelocity.x );
		// DevMsg( "roll %6.1f (%6.1f:%6.1f) : ", goalRoll, GetLocalAngles().z, m_vecAngVelocity.z );
		// DevMsg( "%6.1f %6.1f %6.1f  :  ", goalAngAccel.x, goalAngAccel.y, goalAngAccel.z );
		// DevMsg( "%6.0f %6.0f %6.0f\n", angAccelAccel.x, angAccelAccel.y, angAccelAccel.z );

		ApplySidewaysDrag( right );
		ApplyGeneralDrag();
		
		QAngle angVel = GetLocalAngularVelocity();
		angVel += m_vecAngAcceleration * 0.1;

		//angVel.y = clamp( angVel.y, -60, 60 );
		//angVel.y = clamp( angVel.y, -120, 120 );
		angVel.y = clamp( angVel.y, -120, 120 );

		SetLocalAngularVelocity( angVel );

		m_flForce = m_flForce * 0.8 + (accel.z + fabs( accel.x ) * 0.1 + fabs( accel.y ) * 0.1) * 0.1 * 0.2;

		vecImpulse = m_flForce * up;
		
		if ( m_lifeState == LIFE_DYING )
		{
			vecImpulse.z = -38.4;  // 64ft/sec
		}
		else
		{
			vecImpulse.z -= 38.4;  // 32ft/sec
		}

		// Find our current velocity
		Vector vecVelDir = GetAbsVelocity();

		VectorNormalize( vecVelDir );

		if ( flDistFromPath > 100 )
		{
			// Strongly constrain to an n unit pipe around the current path
			// by damping out all impulse forces that would push us further from the pipe
			float flDot = DotProduct( vecImpulse, vecDelta );
			if ( flDot < 0.0f )
			{
				VectorMA( vecImpulse, -flDot * 0.1f, vecDelta, vecImpulse );
			}

			// Also apply an extra impulse to compensate for the current velocity
			flDot = DotProduct( vecVelDir, vecDelta );
			if ( flDot < 0.0f )
			{
				VectorMA( vecImpulse, -flDot * 0.1f, vecDelta, vecImpulse );
			}
		}
		
		// Find our acceleration direction
		Vector	vecAccelDir = vecImpulse;
		VectorNormalize( vecAccelDir );

		// Level out our plane of movement
		vecAccelDir.z	= 0.0f;
		vecVelDir.z		= 0.0f;
		forward.z		= 0.0f;
		right.z			= 0.0f;

		// Find out how "fast" we're moving in relation to facing and acceleration
		finspeed = m_flForce * DotProduct( vecVelDir, vecAccelDir );
		swayspeed = m_flForce * DotProduct( vecVelDir, right );
	}

	/*
	// If we're landing, deliberately tuck in the back end
	if ( GetLandingState() == LANDING_DESCEND || GetLandingState() == LANDING_TOUCHDOWN || 
		 GetLandingState() == LANDING_UNLOADING || GetLandingState() == LANDING_UNLOADED || IsHovering() )
	{
		finspeed = -60;
	}
	*/

	// Apply the acceleration blend to the fins
	float finAccelBlend = SimpleSplineRemapVal( finspeed, -60, 60, -25, 25 );
	float curFinAccel = GetPoseParameter( m_poseMiddleRudder );
	curFinAccel = UTIL_Approach( finAccelBlend, curFinAccel, 2.5f );

	float accelPose = EdgeLimitPoseParameter( m_poseMiddleRudder, curFinAccel );
	SetPoseParameter( m_poseMiddleRudder, accelPose );

	// Apply the spin sway to the fins
	float finSwayBlend = SimpleSplineRemapVal( swayspeed, -60, 60, -25, 25 );
	float curFinSway = GetPoseParameter( m_poseRightRudder );
	curFinSway = UTIL_Approach( finSwayBlend, curFinSway, 2.5f );

	float rudderPose = EdgeLimitPoseParameter( m_poseRightRudder, curFinSway );
	float flapsPose = EdgeLimitPoseParameter( m_poseRightWingFlap, curFinSway * 0.6f );
	SetPoseParameter( m_poseRightRudder, rudderPose );
	SetPoseParameter( m_poseLeftRudder, rudderPose );
	SetPoseParameter( m_poseRightWingFlap, flapsPose );
	SetPoseParameter( m_poseLeftWingFlap, -flapsPose );

	// Pose the nacelles
	float nacellePose = EdgeLimitPoseParameter( m_poseRightNacelle, flPlaneTransition * 90.0f );
	SetPoseParameter( m_poseRightNacelle, nacellePose );
	SetPoseParameter( m_poseLeftNacelle, nacellePose );

	//Msg( "fin accel: %f, fin sway: %f\n", curFinAccel, curFinSway );

	if ( bRunFlight )
	{
		// Add in our velocity pulse for this frame
		ApplyAbsVelocityImpulse( vecImpulse );
	}

	//DevMsg("curFinAccel: %f, curFinSway: %f\n", curFinAccel, curFinSway );
}

//------------------------------------------------------------------------------
// Purpose : Spawn the next NPC in our template list
//------------------------------------------------------------------------------
void CNPC_BM_Osprey::SpawnTroop( void )
{
#if 0
	// Are we fully unloaded? If so, take off. Otherwise, tell the next troop to exit.
	if ( m_iCurrentTroopExiting >= m_soldiersToDrop || m_sNPCTemplateData[m_iCurrentTroopExiting] == NULL_STRING )
	{
		// We're done, take off.
		m_flTimeTakeOff = gpGlobals->curtime + 0.5;
		return;
	}

	m_hLastTroopToLeave = NULL;

	// Not time to try again yet?
	if ( m_flNextTroopSpawnAttempt > gpGlobals->curtime )
		return;

	// HACK: This is a nasty piece of work. We want to make sure the deploy end is clear, and has enough
	// room with our deploying NPC, but we don't want to create the NPC unless it's clear, and we don't
	// know how much room he needs without spawning him. 
	// So, because we know that we only ever spawn combine soldiers at the moment, we'll just use their hull.
	// HACK: Add some bloat because the endpoint isn't perfectly aligned with NPC end origin
	Vector vecNPCMins = NAI_Hull::Mins( HULL_HUMAN ) - Vector(4,4,4);
	Vector vecNPCMaxs = NAI_Hull::Maxs( HULL_HUMAN ) + Vector(4,4,4);

	// Scare NPCs away from our deploy endpoint to keep them away
	Vector vecDeployEndPoint;
	QAngle vecDeployEndAngles;
	GetAttachment( m_iAttachmentTroopDeploy, vecDeployEndPoint, vecDeployEndAngles );

	// Make sure there are no NPCs on the spot
	trace_t tr;
	CTraceFilterOnlyNPCsAndPlayer filter( this, COLLISION_GROUP_NONE );
	AI_TraceHull( vecDeployEndPoint, vecDeployEndPoint, vecNPCMins, vecNPCMaxs, MASK_SOLID, &filter, &tr );
	if ( tr.m_pEnt )
	{
		if ( g_debug_dropship.GetInt() == 2 )
		{
			NDebugOverlay::Box( vecDeployEndPoint, vecNPCMins, vecNPCMaxs, 255,0,0, 64, 0.5 );
		}

		m_flNextTroopSpawnAttempt = gpGlobals->curtime + 1;
		return;
	}

	if ( g_debug_dropship.GetInt() == 2 )
	{
		NDebugOverlay::Box( vecDeployEndPoint, vecNPCMins, vecNPCMaxs, 0,255,0, 64, 0.5 );
	}

	// Get the spawn point inside the container
	Vector vecSpawnOrigin;
	QAngle vecSpawnAngles;
	m_hContainer->GetAttachment( m_iAttachmentDeployStart, vecSpawnOrigin, vecSpawnAngles );

	// Spawn the templated NPC
	CBaseEntity *pEntity = NULL;
	MapEntity_ParseEntity( pEntity, STRING(m_sNPCTemplateData[m_iCurrentTroopExiting]), NULL );

	// Increment troop count
	m_iCurrentTroopExiting++;

	if ( !pEntity )
	{
		Warning("Dropship could not create template NPC\n" );
		return;
	}
	CAI_BaseNPC	*pNPC = pEntity->MyNPCPointer();
	Assert( pNPC );

	// Spawn an entity blocker.
	CBaseEntity *pBlocker = CEntityBlocker::Create( vecDeployEndPoint, vecNPCMins, vecNPCMaxs, pNPC, true );
	g_EventQueue.AddEvent( pBlocker, "Kill", 2.5, this, this );
	if ( g_debug_dropship.GetInt() == 2 )
	{
		NDebugOverlay::Box( vecDeployEndPoint, vecNPCMins, vecNPCMaxs, 255, 255, 255, 64, 2.5 );
	}

	// Ensure our NPCs are standing upright
	vecSpawnAngles[PITCH] = vecSpawnAngles[ROLL] = 0;

	// Move it to the container spawnpoint
	pNPC->SetAbsOrigin( vecSpawnOrigin );
	pNPC->SetAbsAngles( vecSpawnAngles );
	DispatchSpawn( pNPC );
	pNPC->m_NPCState = NPC_STATE_IDLE;
	pNPC->Activate();

	// Spawn a scripted sequence entity to make the NPC run out of the dropship
	CAI_ScriptedSequence *pSequence = (CAI_ScriptedSequence*)CreateEntityByName( "scripted_sequence" );
	pSequence->KeyValue( "m_iszEntity", STRING(pNPC->GetEntityName()) );
	pSequence->KeyValue( "m_iszPlay", "Dropship_Deploy" );
	pSequence->KeyValue( "m_fMoveTo", "4" );	// CINE_MOVETO_TELEPORT
	pSequence->KeyValue( "OnEndSequence", UTIL_VarArgs("%s,NPCFinishDustoff,%s,0,-1", STRING(GetEntityName()), STRING(pNPC->GetEntityName())) );
	pSequence->SetAbsOrigin( vecSpawnOrigin );
	pSequence->SetAbsAngles( vecSpawnAngles );
	pSequence->AddSpawnFlags( SF_SCRIPT_NOINTERRUPT | SF_SCRIPT_HIGH_PRIORITY | SF_SCRIPT_OVERRIDESTATE );
	pSequence->Spawn();
	pSequence->Activate();
	variant_t emptyVariant;
	pSequence->AcceptInput( "BeginSequence", this, this, emptyVariant, 0 );

	m_hLastTroopToLeave = pNPC;

	m_OnSpawnNPC.Set( pNPC, pNPC, this );
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_BM_Osprey::InputBeginRappellingGrunts( inputdata_t& inputdata )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_BM_Osprey::InputRemoveGrunts( inputdata_t& inputdata )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_BM_Osprey::InputWaitHereTillReady( inputdata_t& inputdata )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_BM_Osprey::InputRappelToTarget( inputdata_t& inputdata )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_BM_Osprey::InputKillRappelingGrunts( inputdata_t& inputdata )
{
}
