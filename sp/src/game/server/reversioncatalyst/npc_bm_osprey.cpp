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

	DEFINE_KEYFIELD( m_flMaxSpeed,		FIELD_FLOAT, "MaxSpeed" ),
	DEFINE_KEYFIELD( m_flMaxSpeedFiring,	FIELD_FLOAT, "MaxSpeedfiring" ),

	DEFINE_KEYFIELD( m_flAcceleration,		FIELD_FLOAT, "Acceleration" ),

	DEFINE_FIELD( m_vecAngAcceleration, FIELD_VECTOR ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CNPC_BM_Osprey::CNPC_BM_Osprey()
{
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
	SetPoseParameter( m_poseGear, 100.0f );
}

//------------------------------------------------------------------------------
// Purpose: Create our rotor sound
//------------------------------------------------------------------------------
void CNPC_BM_Osprey::InitializeRotorSound( void )
{
	if ( !m_pRotorSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		CPASAttenuationFilter filter( this );

		m_pRotorSound = controller.SoundCreate( filter, entindex(), OSPREY_HOVER_SOUND );
		m_pRotorBlast = controller.SoundCreate( filter, entindex(), OSPREY_ROTOR_BLAST_SOUND );
	}
	else
	{
		Assert(m_pRotorSound);
		Assert(m_pRotorBlast);
	}

	BaseClass::InitializeRotorSound();
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

	// TODO: Unloading soldiers?
	bool bRunFlight = true;
	if ( bRunFlight )
	{
		if( GetFlags() & FL_ONGROUND )
		{
			// This would be really bad.
			SetGroundEntity( NULL );
		}

		// calc desired acceleration
		float dt = 1.0f;

		Vector	accel;
		float	accelRate = OSPREY_ACCEL_RATE;
		float	maxSpeed = GetMaxSpeed();

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

		// calc goal orientation to hit linear accel forces
		float goalPitch = RAD2DEG( asin( DotProduct( forward, goalUp ) ) );
		float goalYaw = UTIL_VecToYaw( m_vecDesiredFaceDir );
		float goalRoll = RAD2DEG( asin( DotProduct( right, goalUp ) ) );

		// clamp goal orientations
		goalPitch = clamp( goalPitch, -45, 60 );
		goalRoll = clamp( goalRoll, -45, 45 );

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
	float finAccelBlend = SimpleSplineRemapVal( finspeed, -60, 60, -1, 1 );
	float curFinAccel = GetPoseParameter( m_poseMiddleRudder );
	curFinAccel = UTIL_Approach( finAccelBlend, curFinAccel, 0.1f );

	float accelPose = EdgeLimitPoseParameter( m_poseMiddleRudder, curFinAccel * 25.0f );
	SetPoseParameter( m_poseMiddleRudder, accelPose );

	// Apply the spin sway to the fins
	float finSwayBlend = SimpleSplineRemapVal( swayspeed, -60, 60, -1, 1 );
	float curFinSway = GetPoseParameter( m_poseRightRudder );
	curFinSway = UTIL_Approach( finSwayBlend, curFinSway, 0.1f );

	float rudderPose = EdgeLimitPoseParameter( m_poseRightRudder, curFinSway * 25.0f );
	float flapsPose = EdgeLimitPoseParameter( m_poseRightWingFlap, curFinSway * 15.0f );
	SetPoseParameter( m_poseRightRudder, rudderPose );
	SetPoseParameter( m_poseLeftRudder, rudderPose );
	SetPoseParameter( m_poseRightWingFlap, flapsPose );
	SetPoseParameter( m_poseLeftWingFlap, -flapsPose );

	if ( bRunFlight )
	{
		// Add in our velocity pulse for this frame
		ApplyAbsVelocityImpulse( vecImpulse );
	}

	//DevMsg("curFinAccel: %f, curFinSway: %f\n", curFinAccel, curFinSway );
}
