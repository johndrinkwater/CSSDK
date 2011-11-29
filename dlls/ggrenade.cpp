/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "player.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "hltv.h"
#include "gamerules.h"

LINK_ENTITY_TO_CLASS( grenade, CGrenade );

TYPEDESCRIPTION	CGrenade::m_SaveData[] = 
{
	DEFINE_FIELD( CGrenade, m_flNextFreqInterval, FIELD_TIME	),
	DEFINE_FIELD( CGrenade, m_flNextBeep		, FIELD_TIME	),
	DEFINE_FIELD( CGrenade, m_flNextFreq		, FIELD_TIME	),
	DEFINE_FIELD( CGrenade, m_sBeepName			, FIELD_STRING	),
	DEFINE_FIELD( CGrenade, m_flAttenu			, FIELD_FLOAT	),
	DEFINE_FIELD( CGrenade, m_flNextBlink		, FIELD_TIME	),
	DEFINE_FIELD( CGrenade, m_fJustBlew			, FIELD_BOOLEAN ),
	DEFINE_FIELD( CGrenade, m_pentCurBombTarget	, FIELD_ENTITY	),
	DEFINE_FIELD( CGrenade, m_SGSmoke			, FIELD_INTEGER ),
	DEFINE_FIELD( CGrenade, m_fLightSmoke		, FIELD_BOOLEAN ),
	DEFINE_FIELD( CGrenade, m_usEvent			, FIELD_SHORT	),
};

IMPLEMENT_SAVERESTORE( CGrenade, CBaseEntity );


// Grenades flagged with this will be triggered when the owner calls detonateSatchelCharges
#define SF_DETONATE	0x0001

// CSSDK
extern int gmsgScenarioIcon;
extern int gmsgBombPickup;


CGrenade *CGrenade::ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->Spawn();

	pGrenade->pev->gravity = 0.5;	// lower gravity since grenade is aerodynamic and engine doesn't know it.

	UTIL_SetOrigin( pGrenade->pev, vecStart );

	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles (pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	
	// Make monsters afraid of it while in the air
	pGrenade->SetThink( &CGrenade::DangerSoundThink );
	pGrenade->pev->nextthink = gpGlobals->time;
	
	// Tumble in air
	pGrenade->pev->avelocity.x = RANDOM_FLOAT ( -100, -500 );
	
	// Explode on contact
	pGrenade->SetTouch( &CGrenade::ExplodeTouch );

	pGrenade->pev->dmg = gSkillData.plrDmgM203Grenade;

	return pGrenade;
}

void CGrenade::DangerSoundThink( void )
{
	if( !IsInWorld() )
	{
		UTIL_Remove( this );
		return;
	}

	CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * 0.5, pev->velocity.Length( ), 0.2 );
	pev->nextthink = gpGlobals->time + 0.2;

	if( pev->waterlevel != 0 )
	{
		pev->velocity = pev->velocity * 0.5;
	}
}

void CGrenade::TumbleThink( void )
{
	if( !IsInWorld() )
	{
		UTIL_Remove( this );
		return;
	}

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if( pev->dmgtime - 1 < gpGlobals->time )
	{
		CSoundEnt::InsertSound( bits_SOUND_DANGER, pev->origin + pev->velocity * ( pev->dmgtime - gpGlobals->time ), 400, 0.1 );
	}

	if( pev->dmgtime <= gpGlobals->time )
	{
		if( pev->dmg <= 40.0 )
			SetThink( &CGrenade::Detonate );
		else
			SetThink( &CGrenade::Detonate3 );
	}
	else if( pev->waterlevel )
	{
		pev->velocity = pev->velocity * 0.5;
		pev->framerate = 0.2;
	}
}

void CGrenade::BounceTouch( CBaseEntity *pOther )
{
	if( pOther->edict() == pev->owner )
		return;

	if( FClassnameIs( pOther->pev, "func_breakable" ) && pOther->pev->rendermode )
	{	
		pev->velocity = pev->velocity * 0.2;
		return;
	}

	Vector testVelocity;

	testVelocity = pev->velocity; 
	testVelocity.z *= 0.7;

	if ( !m_fRegisteredSound && testVelocity.Length() <= 60 )
	{
		CSoundEnt::InsertSound( bits_SOUND_DANGER, pev->origin, ( pev->dmg * 2.5 ), 0.3 );
		m_fRegisteredSound = TRUE;
	}

	if( pev->flags & FL_ONGROUND )
	{
		pev->velocity = pev->velocity * 0.8;
		pev->sequence = RANDOM_LONG( 1, 1 );
	}
	else
	{
		if( m_iNumBounce < 5)
		{
			BounceSound();
		}
		else if( m_iNumBounce >= 10 )
		{
			pev->groundentity = ENT( 0 );
			pev->flags |= FL_ONGROUND;
			pev->velocity = g_vecZero;
		}

		m_iNumBounce++;
	}

	pev->framerate = pev->velocity.Length() / 200.0;

	if( pev->framerate > 1.0 )		
	{
		pev->framerate = 1;
	}
	else if( pev->framerate < 0.5 )
	{
		pev->framerate = 0;
	}
}

void CGrenade::SlideTouch( CBaseEntity *pOther )
{
	if ( pOther->edict() == pev->owner )
		return;

	if( pev->flags & FL_ONGROUND )
	{
		pev->velocity = pev->velocity * 0.95;
	}
	else
	{
		BounceSound();
	}
}

void CGrenade::BounceSound( void )
{
	if( pev->dmg > 50.0 )
	{
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/he_bounce-1.wav", 0.25, ATTN_NORM );
		return;
	}

	switch( RANDOM_LONG( 0, 2 ) )
	{
		case 0 : EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/grenade_hit1.wav", 0.25, ATTN_NORM );	break;
		case 1 : EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/grenade_hit2.wav", 0.25, ATTN_NORM );	break;
		case 2 : EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/grenade_hit3.wav", 0.25, ATTN_NORM );	break;
	}
}

void CGrenade::UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code )
{
	edict_t *pentFind = NULL;
	edict_t *pentOwner = NULL;

	if( !pevOwner )
		return;

	CBaseEntity	*pOwner = CBaseEntity::Instance( pevOwner );

	pentOwner = pOwner->edict();

	while( !FNullEnt( ( pentFind = FIND_ENTITY_BY_CLASSNAME( pentFind, "grenade" ) ) ) )
	{
		CBaseEntity *pEnt = Instance( pentFind );

		if( pEnt )
		{
			if( FBitSet( pEnt->pev->spawnflags, SF_DETONATE ) && pEnt->pev->owner == pentOwner )
			{
				if( code == SATCHEL_DETONATE )
					pEnt->Use( pOwner, pOwner, USE_ON, 0 );
				else	// SATCHEL_RELEASE
					pEnt->pev->owner = NULL;
			}
		}
	}
}

// CSSDK
void CGrenade::Spawn( void )
{
	m_iNumBounce	= 0;
	m_fPlantedC4	= FALSE;

	pev->movetype	= MOVETYPE_BOUNCE;
	pev->solid		= SOLID_BBOX;

	// TODO: Implements me.
	// 	if( pev->classname )
	// 		this->RemoveEntityHashValue( STRING( pev->classname ) );

	pev->classname	= MAKE_STRING( "grenade" );

	// TODO: Implements me.
	//	this->AddEntityHashValue( STRING( pev->classname ) );

	SET_MODEL( ENT( pev ), "models/grenade.mdl" );
	UTIL_SetSize( pev, Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );

	pev->dmg = 30.0;

	m_fRegisteredSound = FALSE;
}


// ===========
//  FLASHBANG
// ===========

void CGrenade::DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetThink( &CGrenade::Detonate );
	pev->nextthink = gpGlobals->time;
}

CGrenade* CGrenade::ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );

	pGrenade->Spawn();
	UTIL_SetOrigin( pGrenade->pev, vecStart );

	pGrenade->pev->velocity	= vecVelocity;
	pGrenade->pev->angles	= pevOwner->angles;
	pGrenade->pev->owner	= ENT( pevOwner );

	pGrenade->SetTouch( &CGrenade::BounceTouch );
	pGrenade->SetThink( &CGrenade::TumbleThink );

	pGrenade->pev->dmgtime	= gpGlobals->time + time;
	pGrenade->pev->nextthink= gpGlobals->time + 0.1;

	if( time < 0.1 )
	{
		pGrenade->pev->nextthink= gpGlobals->time;
		pGrenade->pev->velocity	= g_vecZero;
	}

	pGrenade->pev->sequence	= RANDOM_LONG( 3, 6 );
	pGrenade->pev->framerate= 1.0;

	pGrenade->m_fJustBlew	= TRUE;

	pGrenade->pev->gravity	= 0.5;
	pGrenade->pev->friction = 0.8;
	pGrenade->pev->dmg		= 35;

	SET_MODEL( ENT( pGrenade->pev ), "models/w_flashbang.mdl" );

	return pGrenade;
}

void CGrenade::Explode( Vector vecSrc, Vector vecAim )
{
	TraceResult tr;
	UTIL_TraceLine ( pev->origin, pev->origin + Vector ( 0, 0, -32 ),  ignore_monsters, ENT(pev), & tr);

	Explode( &tr, DMG_BLAST );
}

void CGrenade::Explode( TraceResult* pTrace, int bitsDamageType )
{
	pev->model		= iStringNull;	// invisible
	pev->solid		= SOLID_NOT;	// intangible
	pev->takedamage = DAMAGE_NO;

	if( pTrace->flFraction != 1.0 )
	{
		pev->origin = pTrace->vecEndPos + ( pTrace->vecPlaneNormal * ( pev->dmg - 24 ) * 0.6 );
	}

	bool isInWater = UTIL_PointContents( pev->origin ) == CONTENTS_WATER;

	CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );

	entvars_t *pevOwner;

	if ( pev->owner )
		pevOwner = VARS( pev->owner );
	else
		pevOwner = NULL;

	pev->owner = NULL; // Can't traceline attack owner if this is set.

// TODO: Implements this.
// 	TheBots->OnEvent( EVENT_FLASHBANG_EXPLODED, pevOwner, NULL );

// TODO: Implements this.
//	RadiusFlash( pev->origin, pev, pevOwner, 4.0 );

	UTIL_DecalTrace( pTrace, RANDOM_FLOAT( 0 , 1 ) < 0.5 ? DECAL_SCORCH1 : DECAL_SCORCH2 );

	switch ( RANDOM_LONG( 0, 2 ) )
	{
		case 0:	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/flashbang-1", 0.55, ATTN_NORM ); break;
		case 1:	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/flashbang-2", 0.55, ATTN_NORM ); break;
	}

	pev->effects |= EF_NODRAW;
	SetThink( &CGrenade::Smoke );

	pev->velocity	= g_vecZero;
	pev->nextthink	= gpGlobals->time + 0.3;

	if( !isInWater )
	{
		int sparkCount = RANDOM_LONG( 0, 3 );

		for( int i = 0; i < sparkCount; i++ )
		{
			Create( "spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL );
		}
	}
}

void CGrenade::Killed( entvars_t *pevAttacker, int iGib )
{
	Detonate();
}

void CGrenade::PreDetonate( void )
{
	CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, 400, 0.3 );

	SetThink( &CGrenade::Detonate );
	pev->nextthink = gpGlobals->time + 1.0;
}

void CGrenade::Detonate( void )
{
	TraceResult tr;
	Vector		vecSpot; // trace starts here!

	vecSpot = pev->origin + Vector ( 0 , 0 , 8 );
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -40 ),  ignore_monsters, ENT( pev ), &tr );

	Explode( &tr, DMG_BLAST );
}

void CGrenade::ExplodeTouch( CBaseEntity *pOther )
{
	TraceResult tr;
	Vector		vecSpot; // trace starts here!

	pev->enemy = pOther->edict();

	vecSpot = pev->origin - pev->velocity.Normalize() * 32;
	UTIL_TraceLine( vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev ), &tr );

	Explode( &tr, DMG_BLAST );
}

void CGrenade::Smoke( void )
{
	if( UTIL_PointContents( pev->origin ) == CONTENTS_WATER )
	{
		UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
	}
	else
	{
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_SMOKE );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_SHORT( g_sModelIndexSmoke );
		WRITE_BYTE( 25 ); // scale * 10
		WRITE_BYTE( 6  ); // framerate
		MESSAGE_END();
	}

	UTIL_Remove( this );
}

// ============
//  HE GRENADE
// ============

CGrenade* CGrenade::ShootTimed2( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, int team, unsigned short usEvent )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	
	pGrenade->Spawn();
	UTIL_SetOrigin( pGrenade->pev, vecStart );

	pGrenade->pev->velocity	= vecVelocity;
	pGrenade->pev->angles	= pevOwner->angles;
	pGrenade->pev->owner	= ENT( pevOwner );

	pGrenade->m_usEvent	= usEvent;

	pGrenade->SetTouch( &CGrenade::BounceTouch );
	pGrenade->SetThink( &CGrenade::TumbleThink );

	pGrenade->pev->dmgtime	= gpGlobals->time + time;
	pGrenade->pev->nextthink= gpGlobals->time + 0.1;

	pGrenade->pev->sequence	= RANDOM_LONG( 3, 6 );
	pGrenade->pev->framerate= 1.0;

	pGrenade->m_fJustBlew	= TRUE;
	pGrenade->m_iTeam		= team;

	pGrenade->pev->gravity	= 0.55;
	pGrenade->pev->friction = 0.70;
	pGrenade->pev->dmg		= 100;
	
	SET_MODEL( ENT( pGrenade->pev ), "models/w_hegrenade.mdl" );

	return pGrenade;
}

void CGrenade::Detonate3( void )
{
	TraceResult tr;
	Vector		vecSpot; // trace starts here!

	vecSpot = pev->origin + Vector ( 0 , 0 , 8 );
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -40 ),  ignore_monsters, ENT( pev ), &tr );

	Explode3( &tr, DMG_GRENADE );
}

void CGrenade::Explode3( TraceResult* pTrace, int bitsDamageType )
{
	pev->model		= iStringNull;	// invisible
	pev->solid		= SOLID_NOT;	// intangible
	pev->takedamage = DAMAGE_NO;

	if( pTrace->flFraction != 1.0 )
	{
		pev->origin = pTrace->vecEndPos + ( pTrace->vecPlaneNormal * ( pev->dmg - 24 ) * 0.6 );
	}

	bool isInWater = UTIL_PointContents( pev->origin ) == CONTENTS_WATER;

	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_EXPLOSION );		
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z + 20.0 );
		WRITE_SHORT( g_sModelIndexFireball3 );
		WRITE_BYTE( 25 ); 
		WRITE_BYTE( 30 ); 
		WRITE_BYTE( 0 ); 
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_EXPLOSION );		
		WRITE_COORD( pev->origin.x + RANDOM_FLOAT( -64, 64 ) );
		WRITE_COORD( pev->origin.y + RANDOM_FLOAT( -64, 64 ) );
		WRITE_COORD( pev->origin.z + RANDOM_FLOAT( 30, 35 ) );
		WRITE_SHORT( g_sModelIndexFireball2 );
		WRITE_BYTE( 30 ); 
		WRITE_BYTE( 30 ); 
		WRITE_BYTE( 0 ); 
	MESSAGE_END();

	CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );

	EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/c4_explode1.wav", VOL_NORM, 0.25 );

	entvars_t *pevOwner;

	if ( pev->owner )
		pevOwner = VARS( pev->owner );
	else
		pevOwner = NULL;

	pev->owner = NULL; // Can't traceline attack owner if this is set.

// 	TODO: Implements this.
// 	TheBots->OnEvent( EVENT_GRENADE_EXPLODED, pevOwner, NULL );

// 	TODO: Fix me.
// 	CBaseMonster::RadiusDamage( pev, pev, pevOwner, pev->dmg, CLASS_NONE, bitsDamageType );

	UTIL_DecalTrace( pTrace, RANDOM_FLOAT( 0 , 1 ) < 0.5 ? DECAL_SCORCH1 : DECAL_SCORCH2 );

	switch ( RANDOM_LONG( 0, 2 ) )
	{
		case 0:	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM ); break;
		case 1:	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM ); break;
		case 2:	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM ); break;
	}

	pev->effects |= EF_NODRAW;
	SetThink( &CGrenade::Smoke3_C );

	pev->velocity	= g_vecZero;
	pev->nextthink	= gpGlobals->time + 0.55;

	if( !isInWater )
	{
		int sparkCount = RANDOM_LONG( 0, 3 );

		for( int i = 0; i < sparkCount; i++ )
		{
			Create( "spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL );
		}
	}
}


// ===============
//  SMOKE GRENADE
// ===============

CGrenade* CGrenade::ShootSmokeGrenade( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, int team, unsigned short usEvent )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );

	pGrenade->Spawn();
	UTIL_SetOrigin( pGrenade->pev, vecStart );

	pGrenade->pev->velocity	= vecVelocity;
	pGrenade->pev->angles	= pevOwner->angles;
	pGrenade->pev->owner	= ENT( pevOwner );

	pGrenade->m_bSGMulti	 = false;
	pGrenade->m_bSGDetonated = false;
	pGrenade->m_usEvent		 = usEvent;

	pGrenade->SetTouch( &CGrenade::BounceTouch );
	pGrenade->SetThink( &CGrenade::SG_TumbleThink );

	pGrenade->pev->dmgtime	= gpGlobals->time + time;
	pGrenade->pev->nextthink= gpGlobals->time + 0.1;

	if( time < 0.1 )
	{
		pGrenade->pev->nextthink= gpGlobals->time;
		pGrenade->pev->velocity	= g_vecZero;
	}

	pGrenade->pev->sequence	= RANDOM_LONG( 3, 6 );
	pGrenade->pev->framerate= 1.0;

	pGrenade->m_fJustBlew	= TRUE;
	pGrenade->m_SGSmoke		= 1;

	pGrenade->pev->gravity	= 0.5;
	pGrenade->pev->friction = 0.8;
	pGrenade->pev->dmg		= 35;

	SET_MODEL( ENT( pGrenade->pev ), "models/w_smokegrenade.mdl" );

	return pGrenade;
}

void CGrenade::SG_TumbleThink( void )
{
	if( !IsInWorld() )
	{
		UTIL_Remove( this );
		return;
	}

	if( pev->flags & FL_ONGROUND )
	{
		pev->velocity = pev->velocity * 0.95;
	}

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if( pev->dmgtime - 1 < gpGlobals->time )
	{
		CSoundEnt::InsertSound( bits_SOUND_DANGER, pev->origin + pev->velocity * ( pev->dmgtime - gpGlobals->time ), 400, 0.1 );
	}

	if( pev->dmgtime <= gpGlobals->time && pev->flags & FL_ONGROUND )
	{
		SetThink( &CGrenade::SG_Detonate );
	}
	else if( pev->waterlevel )
	{
		pev->velocity = pev->velocity * 0.5;
		pev->framerate = 0.2;
	}
}

void CGrenade::SG_Smoke( void )
{
	if( UTIL_PointContents( pev->origin ) == CONTENTS_WATER )
	{
		UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
	}
	else
	{
		UTIL_MakeVectors( pev->angles );

		Vector randomDir = gpGlobals->v_forward * RANDOM_FLOAT( 3, 8 );

		m_fLightSmoke = (int)(( ( m_fLightSmoke * 180 / M_PI ) + 30 )) % 360;

		// TODO: Check me.
		PLAYBACK_EVENT_FULL( 0, 
							NULL, 
							m_usEvent, 
							0.0, 
							pev->origin, 
							m_SGExplosionPos, 
							randomDir.x * cos( 180 / M_PI ) - randomDir.y * cos( 180 / M_PI ), 
							randomDir.x * sin( 180 / M_PI ) + randomDir.y * sin( 180 / M_PI ),
							cos( 180 / M_PI ) * 100.0, 
							4, 
							m_bSGMulti, 
							6 );
	}

	if( m_SGSmoke <= 20 )
	{
		pev->nextthink = gpGlobals->time + 1.0;
		SetThink( &CGrenade::SG_Smoke );

		++m_SGSmoke;
	}
	else
	{
		pev->effects |= EF_NODRAW;
		
		// TODO: Implements this.
		//TheBots->RemoveGrenade( this );

		UTIL_Remove( this );
	}
}

void CGrenade::SG_Detonate( void )
{
	TraceResult tr;
	Vector		vecSpot;

	vecSpot = pev->origin + Vector( 0 , 0 , 8 );
	UTIL_TraceLine( vecSpot, vecSpot + Vector( 0, 0, -40 ), ignore_monsters, ENT( pev ), &tr );

// TODO: Adds support for bots.
// 	TheBots->OnEvent( EVENT_SMOKE_GRENADE_EXPLODE, CBaseEntity::Instance( pev->owner ), NULL );
// 	TheBots->AddGrenade( 9, this );

	edict_t* pEntity = NULL;

	while( ( pEntity = FIND_ENTITY_BY_STRING( pEntity, "classname", "grenade" ) ) != NULL )
	{
		if( !FNullEnt( pEntity ) )
		{
			CBaseEntity* pGrenade = CBaseEntity::Instance( pEntity );

			if( pGrenade && ( pGrenade->pev->origin - pev->origin ).Length() <= 250.0 && pGrenade->pev->dmgtime > gpGlobals->time )
			{
				m_bSGMulti = true; 
			}
		}
	}

	m_bSGDetonated = true;
	PLAYBACK_EVENT_FULL( 0, NULL, m_usEvent, 0.0, pev->origin, (float*)&g_vecZero, 0.0, 0.0, 0, 1, m_bSGMulti, 0 ); 

	m_SGExplosionPos = pev->origin;

	pev->velocity.x = RANDOM_FLOAT( -175, 175 );
	pev->velocity.y = RANDOM_FLOAT( -175, 175 );
	pev->velocity.z = RANDOM_FLOAT( 250, 350 );

	SetThink( &CGrenade::SG_Smoke );
	pev->nextthink = gpGlobals->time + 0.1;
}

void CGrenade::SG_Explode( TraceResult *pTrace, int bitsDamageType )
{
	pev->model		= iStringNull;	// invisible
	pev->solid		= SOLID_NOT;	// intangible
	pev->takedamage = DAMAGE_NO;

	if( pTrace->flFraction != 1.0 )
	{
		pev->origin = pTrace->vecEndPos + ( pTrace->vecPlaneNormal * ( pev->dmg - 24 ) * 0.6 );
	}

	CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );
	pev->owner = NULL;

	UTIL_DecalTrace( pTrace, RANDOM_FLOAT( 0 , 1 ) < 0.5 ? DECAL_SCORCH1 : DECAL_SCORCH2 );

	if( RANDOM_LONG( 0, 1 ) )
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/flashbang-1.wav", 0.55, ATTN_NORM );
	else
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/flashbang-2.wav", 0.55, ATTN_NORM );

	SetThink( &CGrenade::SG_Smoke );
	pev->nextthink = gpGlobals->time + 0.1;

	if( UTIL_PointContents( pev->origin ) != CONTENTS_WATER )
	{
		int sparkCount = RANDOM_LONG( 0, 3 );

		for( int i = 0; i < sparkCount; i++ )
		{
			Create( "spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL );
		}
	}
}


void CGrenade::Smoke3_A( void )
{
	if( UTIL_PointContents( pev->origin ) == CONTENTS_WATER )
	{
		UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
	}
	else
	{
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( pev->origin.x + RANDOM_FLOAT( -128, 128 ) );
			WRITE_COORD( pev->origin.y + RANDOM_FLOAT( -128, 128 ) );
			WRITE_COORD( pev->origin.z + RANDOM_FLOAT( -10, 10 ) );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 15 + RANDOM_FLOAT( 0, 10 ) );	// scale * 10
			WRITE_BYTE( 12  );							// framerate
		MESSAGE_END();
	}

	UTIL_Remove( this );
}

void CGrenade::Smoke3_B( void )
{
	if( UTIL_PointContents( pev->origin ) == CONTENTS_WATER )
	{
		UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
	}
	else
	{
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( pev->origin.x + RANDOM_FLOAT( -128, 128 ) );
			WRITE_COORD( pev->origin.y + RANDOM_FLOAT( -128, 128 ) );
			WRITE_COORD( pev->origin.z + RANDOM_FLOAT( -10, 10 ) );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 15 + RANDOM_FLOAT( 0, 10 ) );	// scale * 10
			WRITE_BYTE( 10 );							// framerate
		MESSAGE_END();
	}

	pev->nextthink = gpGlobals->time + 0.15;
	SetThink( &CGrenade::Smoke3_A );
}

void CGrenade::Smoke3_C( void )
{
	if( UTIL_PointContents( pev->origin ) == CONTENTS_WATER )
	{
		UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
	}
	else
	{
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z - 5.0 );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 35 + RANDOM_FLOAT( 0, 10 ) );	// scale * 10
			WRITE_BYTE( 5  );							// framerate
		MESSAGE_END();
	}

	UTIL_Remove( this );
}

// ==========
//	C4 BOMB
// ==========

CGrenade* CGrenade::ShootSatchelCharge( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );

	pGrenade->pev->movetype	= MOVETYPE_BOUNCE;
	pGrenade->pev->solid	= SOLID_BBOX;

	// TODO: Implements me.
	// 	if( pev->classname )
	// 		this->RemoveEntityHashValue( STRING( pev->classname ) );

	pGrenade->pev->classname = MAKE_STRING( "grenade" );

	// TODO: Implements me.
	//	this->AddEntityHashValue( STRING( pev->classname ) );
	//	

	SET_MODEL( ENT( pGrenade->pev ), "models/w_c4.mdl" );

	UTIL_SetSize( pGrenade->pev, Vector( 3, 6, 8 ), Vector( -3, -6, 0 ) );
	UTIL_SetOrigin( pGrenade->pev, vecStart );

	pGrenade->pev->dmg		= 100;
	pGrenade->pev->angles	= vecVelocity;
	pGrenade->pev->velocity = g_vecZero;
	pGrenade->pev->owner	= ENT( pevOwner );

	pGrenade->SetThink( &CGrenade::C4Think );
	pGrenade->SetTouch( &CGrenade::C4Touch );

	pGrenade->pev->spawnflags = SF_DETONATE;
	pGrenade->pev->nextthink  = gpGlobals->time + 0.1;

	pGrenade->m_flC4Blow			= gpGlobals->time + g_pGameRules->m_flC4DetonateDelay;
	pGrenade->m_flNextFreqInterval	= g_pGameRules->m_flC4DetonateDelay / 4;
	pGrenade->m_flNextFreq			= gpGlobals->time;
	pGrenade->m_flNextBeep			= gpGlobals->time + 0.5;
	pGrenade->m_flNextBlink			= gpGlobals->time + 2.0;

	pGrenade->m_iC4Beep		= 0;
	pGrenade->m_flAttenu	= 0;
	pGrenade->m_sBeepName	= NULL;

	pGrenade->m_flNextDefuseTime = 0;

	pGrenade->m_fPlantedC4	= TRUE;
	pGrenade->m_fStartDefuse= FALSE;
	pGrenade->m_fJustBlew	= FALSE;

	pGrenade->pev->friction = 0.9;

	if( !FNullEnt( pevOwner ) )
	{
		CBasePlayer *pPlayer = (CBasePlayer *)Instance( pevOwner );
		pGrenade->m_pentCurBombTarget = pPlayer->m_pentBombTarget;
	}
	else
		pGrenade->m_pentCurBombTarget = NULL;

	return pGrenade;
}

void CGrenade::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	int barTime;

	if( !m_fPlantedC4 )
	{
		return;
	}

	CBasePlayer* pPlayer = GetClassPtr( (CBasePlayer *)pActivator->pev );

	if( pPlayer->m_iTeam == TEAM_CT )
	{
		if( m_fStartDefuse )
		{
			m_flNextDefuseTime = gpGlobals->time + 0.5;
			return;
		}

		SET_CLIENT_MAXSPEED( ENT( pPlayer->pev ), 1.0 );

		// TODO: Adds support for bots.
		// TheBots->OnEvent( EVENT_BOMB_DEFUSING, pActivator, NULL );

		if( g_pGameRules->IsCareer() )
		{
			// TODO: implements this.
			// TheCareerTasks->HandleEvent( EVENT_BOMB_DEFUSING, NULL, NULL );
		}

		if( pPlayer->m_fHasDefuseKit )
		{
			UTIL_LogPrintf(	"\"%s<%i><%s><CT>\" triggered \"Begin_Bomb_Defuse_With_Kit\"\n", 
							STRING( pPlayer->pev->netname ),
							GETPLAYERAUTHID( pPlayer->edict() ),
							GETPLAYERUSERID( pPlayer->edict() ) );

			ClientPrint( pPlayer->pev, HUD_PRINTCENTER, "#Defusing_Bomb_With_Defuse_Kit" );
			EMIT_SOUND( ENT( pPlayer->pev ), CHAN_ITEM, "weapons/c4_disarm.wav", VOL_NORM, ATTN_NORM );

			pPlayer->m_fBombDefusing = TRUE;

			m_hActivator	= pActivator;
			m_fStartDefuse	= TRUE;

			m_flDefuseCountDown	= gpGlobals->time + 5.0;
			m_flNextDefuseTime	= gpGlobals->time + 0.5;

			barTime = 5;
		}
		else
		{
			UTIL_LogPrintf(	"\"%s<%i><%s><CT>\" triggered \"Begin_Bomb_Defuse_Without_Kit\"\n", 
							STRING( pPlayer->pev->netname ),
							GETPLAYERAUTHID( pPlayer->edict() ),
							GETPLAYERUSERID( pPlayer->edict() ) );

			ClientPrint( pPlayer->pev, HUD_PRINTCENTER, "#Defusing_Bomb_Without_Defuse_Kit" );
			EMIT_SOUND( ENT( pPlayer->pev ), CHAN_ITEM, "weapons/c4_disarm.wav", VOL_NORM, ATTN_NORM );

			pPlayer->m_fBombDefusing = TRUE;

			m_hActivator	= pActivator;
			m_fStartDefuse	= TRUE;

			m_flDefuseCountDown	= gpGlobals->time + 10.0;
			m_flNextDefuseTime	= gpGlobals->time + 0.5;

			barTime = 10;
		}

		pPlayer->SetProgressBarTime( barTime );
	}
}

void CGrenade::Detonate2( void )
{
	TraceResult tr;
	Vector		vecSpot;

	vecSpot = pev->origin + Vector( 0 , 0 , 8 );
	UTIL_TraceLine( vecSpot, vecSpot + Vector( 0, 0, -40 ), ignore_monsters, ENT( pev ), &tr );

	Explode2( &tr, DMG_BLAST );
}

void CGrenade::Smoke2( void )
{
	if( UTIL_PointContents( pev->origin ) == CONTENTS_WATER )
	{
		UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
	}
	else
	{
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 150 );	// scale * 10
			WRITE_BYTE( 8  );	// framerate
		MESSAGE_END();
	}

	UTIL_Remove( this );
}

void CGrenade::Explode2( TraceResult* pTrace, int bitsDamageType )
{
	pev->model		= iStringNull;	// invisible
	pev->solid		= SOLID_NOT;	// intangible
	pev->takedamage = DAMAGE_NO;

	UTIL_ScreenShake( pTrace->vecEndPos, 25.0, 150.0, 1.0, 3000.0 );

	g_pGameRules->m_bTargetBombed = true;

	if( g_pGameRules->IsCareer() )
	{
		// TODO: implements this.
		// TheCareerTasks->LatchRoundEndMessage();
	}

	m_fJustBlew = TRUE;
	g_pGameRules->CheckWinConditions();

	if( pTrace->flFraction != 1.0 )
	{
		pev->origin = pTrace->vecEndPos + ( pTrace->vecPlaneNormal * ( pev->dmg - 24 ) * 0.6 );
	}

	bool isInWater = UTIL_PointContents( pev->origin ) == CONTENTS_WATER;

	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_SPRITE );		
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z - 10.0 );
		WRITE_SHORT( g_sModelIndexFireball3 );
		WRITE_BYTE( ( pev->dmg - 275 ) * 0.6 );
		WRITE_BYTE( 150 ); 
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_SPRITE );		
		WRITE_COORD( pev->origin.x + RANDOM_FLOAT( -512, 512 ) );
		WRITE_COORD( pev->origin.y + RANDOM_FLOAT( -512, 512 ) );
		WRITE_COORD( pev->origin.z + RANDOM_FLOAT( -10, 10 ) );
		WRITE_SHORT( g_sModelIndexFireball2 );
		WRITE_BYTE( ( pev->dmg - 275 ) * 0.6 );
		WRITE_BYTE( 150 ); 
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_SPRITE );		
		WRITE_COORD( pev->origin.x + RANDOM_FLOAT( -512, 512 ) );
		WRITE_COORD( pev->origin.y + RANDOM_FLOAT( -512, 512 ) );
		WRITE_COORD( pev->origin.z + RANDOM_FLOAT( -10, 10 ) );
		WRITE_SHORT( g_sModelIndexFireball3 );
		WRITE_BYTE( ( pev->dmg - 275 ) * 0.6 );
		WRITE_BYTE( 150 ); 
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_SPRITE );		
		WRITE_COORD( pev->origin.x + RANDOM_FLOAT( -512, 512 ) );
		WRITE_COORD( pev->origin.y + RANDOM_FLOAT( -512, 512 ) );
		WRITE_COORD( pev->origin.z + RANDOM_FLOAT( -10, 10 ) );
		WRITE_SHORT( g_sModelIndexFireball );
		WRITE_BYTE( ( pev->dmg - 275 ) * 0.6 );
		WRITE_BYTE( 150 ); 
	MESSAGE_END();

	EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/c4_explode1.wav", VOL_NORM, 0.25 );

	CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );

	entvars_t *pevOwner;

	if ( pev->owner )
		pevOwner = VARS( pev->owner );
	else
		pevOwner = NULL;

	pev->owner = NULL; // Can't traceline attack owner if this is set.

	// TODO: Fix me.
	//RadiusDamage( pev, pevOwner, pev->dmg, g_pGameRules->m_flBombRadius, CLASS_NONE, bitsDamageType );

	if( g_pGameRules->IsCareer() )
	{
		// TODO: implements this.
		// TheCareerTasks->UnlatchRoundEndMessage();
	}

	MESSAGE_BEGIN( MSG_SPEC, SVC_DIRECTOR );
		WRITE_BYTE( 9 );
		WRITE_BYTE( DRC_CMD_EVENT );
		WRITE_SHORT( ENTINDEX( this->edict() ) );
		WRITE_SHORT( NULL );
		WRITE_ENTITY( DRC_FLAG_FINAL | 15 );
	MESSAGE_END();

	UTIL_DecalTrace( pTrace, RANDOM_FLOAT( 0 , 1 ) < 0.5 ? DECAL_SCORCH1 : DECAL_SCORCH2 );

	switch ( RANDOM_LONG( 0, 2 ) )
	{
		case 0:	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM ); break;
		case 1:	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM ); break;
		case 2:	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM ); break;
	}

	pev->effects |= EF_NODRAW;
	SetThink( &CGrenade::Smoke2 );
	
	pev->velocity	= g_vecZero;
	pev->nextthink	= gpGlobals->time + 0.85;

	if( !isInWater )
	{
		int sparkCount = RANDOM_LONG( 0, 3 );

		for( int i = 0; i < sparkCount; i++ )
		{
			Create( "spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL );
		}
	}
}

void CGrenade::C4Think( void )
{
	if( !IsInWorld() )
	{
		UTIL_Remove( this );
		return;
	}

	pev->nextthink = gpGlobals->time + 0.12;

	if( m_flNextFreq <= gpGlobals->time )
	{
		m_flNextFreq = gpGlobals->time + m_flNextFreqInterval;
		m_flNextFreqInterval *= 0.9;

		switch( m_iC4Beep )
		{
			case 0 :
			{
				m_flAttenu = 1.5;
				m_sBeepName = "weapons/c4_beep1.wav";

				if( UTIL_IsGame( "czero" ) )
				{
					MESSAGE_BEGIN( MSG_ALL, gmsgScenarioIcon );
						WRITE_BYTE( 1 );
						WRITE_STRING( "bombticking" );
						WRITE_BYTE( 255 );
						WRITE_SHORT( 140 );
						WRITE_SHORT( 0 );
					MESSAGE_END();
				}

				break;
			}
			case 1 :
			{
				m_flAttenu = 1.0;
				m_sBeepName = "weapons/c4_beep2.wav";

				if( UTIL_IsGame( "czero" ) )
				{
					MESSAGE_BEGIN( MSG_ALL, gmsgScenarioIcon );
						WRITE_BYTE( 1 );
						WRITE_STRING( "bombticking" );
						WRITE_BYTE( 255 );
						WRITE_SHORT( 70 );
						WRITE_SHORT( 0 );
					MESSAGE_END();
				}

				break;
			}
			case 2 :
			{
				m_flAttenu = 0.8;
				m_sBeepName = "weapons/c4_beep3.wav";

				if( UTIL_IsGame( "czero" ) )
				{
					MESSAGE_BEGIN( MSG_ALL, gmsgScenarioIcon );
						WRITE_BYTE( 1 );
						WRITE_STRING( "bombticking" );
						WRITE_BYTE( 255 );
						WRITE_SHORT( 40 );
						WRITE_SHORT( 0 );
					MESSAGE_END();
				}

				break;
			}
			case 3 :
			{
				m_flAttenu = 0.5;
				m_sBeepName = "weapons/c4_beep4.wav";

				if( UTIL_IsGame( "czero" ) )
				{
					MESSAGE_BEGIN( MSG_ALL, gmsgScenarioIcon );
						WRITE_BYTE( 1 );
						WRITE_STRING( "bombticking" );
						WRITE_BYTE( 255 );
						WRITE_SHORT( 30 );
						WRITE_SHORT( 0 );
					MESSAGE_END();
				}

				break;
			}
			case 4 :
			{
				m_flAttenu = 0.2;
				m_sBeepName = "weapons/c4_beep5.wav";

				if( UTIL_IsGame( "czero" ) )
				{
					MESSAGE_BEGIN( MSG_ALL, gmsgScenarioIcon );
						WRITE_BYTE( 1 );
						WRITE_STRING( "bombticking" );
						WRITE_BYTE( 255 );
						WRITE_SHORT( 20 );
						WRITE_SHORT( 0 );
					MESSAGE_END();
				}

				break;
			}		
		}

		++m_iC4Beep;
	}

	if( m_flNextBeep <= gpGlobals->time )
	{
		m_flNextBeep = gpGlobals->time + 1.4;
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, m_sBeepName, VOL_NORM, m_flAttenu );
		
		// TODO: Adds support for bots.
		// TheBots->OnEvent( EVENT_BOMB_BEEP, this, NULL );
	}

	if( m_flNextBlink <= gpGlobals->time )
	{
		m_flNextBlink = gpGlobals->time	+ 2.0;

		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
			WRITE_BYTE( TE_GLOWSPRITE );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z + 5.0 );
			WRITE_SHORT( g_sModelIndexC4Glow );
			WRITE_BYTE( 1 );
			WRITE_BYTE( 3 );
			WRITE_BYTE( 255 );
		MESSAGE_END();
	}

	if( m_flC4Blow <= gpGlobals->time )
	{
		// TODO: Adds support for bots.
		// TheBots->OnEvent( EVENT_BOMB_EXPLODED, NULL, NULL );
		
		MESSAGE_BEGIN( MSG_ALL, gmsgScenarioIcon );
			WRITE_BYTE( 0 );
		MESSAGE_END();
		
		if( m_pentCurBombTarget )
		{
			CBaseEntity *pEntity = CBaseEntity::Instance( m_pentCurBombTarget->pev );
			
			if( pEntity )
			{
				CBaseEntity* pPlayer = CBaseEntity::Instance( pev->owner );

				if( pPlayer )
				{
					pEntity->Use( pPlayer, this, USE_TOGGLE, 0 );
				}
			}
		}
		
		CBasePlayer* pPlayer = (CBasePlayer *)CBaseEntity::Instance( pev->owner );

		if( pPlayer )
		{
			pPlayer->pev->frags += 3;
		}

		MESSAGE_BEGIN( MSG_ALL, gmsgBombPickup );
		MESSAGE_END();

		g_pGameRules->m_fBombDropped = FALSE;

		if( pev->waterlevel )
			UTIL_Remove( this );
		else
			SetThink( &CGrenade::Detonate2 );	
	}

	if( m_fStartDefuse )
	{
		CBasePlayer* pDefuser = (CBasePlayer *)((CBaseEntity *)m_hDefuser);

		if( pDefuser && m_flDefuseCountDown > gpGlobals->time )
		{
			BOOL isOnGround = !!( pDefuser->pev->flags & FL_ONGROUND );

			if( m_flNextDefuseTime < gpGlobals->time || !isOnGround )
			{
				if( !isOnGround )
				{
					ClientPrint( m_hDefuser->pev, HUD_PRINTCENTER, "#C4_Defuse_Must_Be_On_Ground" );
				}

				pDefuser->ResetMaxSpeed();
				pDefuser->SetProgressBarTime( 0 );
				pDefuser->m_fBombDefusing = FALSE;

				m_fStartDefuse = FALSE;
				m_flDefuseCountDown = 0.0;

				// TODO: Adds support for bots.
				// TheBots->OnEvent( EVENT_DEFUSE_ABORTED, NULL, NULL );
			}
		}
		else
		{
			// TODO: Adds support for bots.
			// TheBots->OnEvent( EVENT_BOMB_DEFUSED, pDefuser, NULL );
			
			Broadcast( "BOMBDEF" );

			MESSAGE_BEGIN( MSG_SPEC, SVC_DIRECTOR );
				WRITE_BYTE( 9 );
				WRITE_BYTE( DRC_CMD_EVENT );
				WRITE_SHORT( ENTINDEX( this->edict() ) );
				WRITE_SHORT( NULL );
				WRITE_ENTITY( DRC_FLAG_FINAL | DRC_FLAG_FACEPLAYER | DRC_FLAG_DRAMATIC | 15 );
			MESSAGE_END();

			UTIL_LogPrintf(	"\"%s<%i><%s><CT>\" triggered \"Defused_The_Bomb\"\n", 
							STRING( pDefuser->pev->netname ),
							GETPLAYERAUTHID( pDefuser->edict() ),
							GETPLAYERUSERID( pDefuser->edict() ) );
			
			UTIL_EmitAmbientSound( ENT( pev ), pev->origin, "weapons/c4_beep5.wav", 0, ATTN_NONE, SND_STOP, 0 );
			EMIT_SOUND( ENT( pDefuser->pev ), CHAN_WEAPON, "weapons/c4_disarmed.wav", 0.8, ATTN_NORM );

			UTIL_Remove( this );
			m_fJustBlew = TRUE;

			pDefuser->ResetMaxSpeed();
			pDefuser->m_fBombDefusing = FALSE;

			MESSAGE_BEGIN( MSG_ALL, gmsgScenarioIcon );
				WRITE_BYTE( 0 );
			MESSAGE_END();
			
			if( g_pGameRules->IsCareer() )
			{
				// TODO: Adds support for bots.
				//TheCareerTasks->HandleEvents( EVEN_BOMB_DEFUSED, pDefuser, NULL );
			}

			g_pGameRules->m_bBombDefused = TRUE;
			g_pGameRules->CheckWinConditions();

			pDefuser->pev->frags += 3;

			MESSAGE_BEGIN( MSG_ALL, gmsgBombPickup );
			MESSAGE_END();

			g_pGameRules->m_fBombDropped = FALSE;
			m_fStartDefuse = FALSE;			
		}
	}
}

void CGrenade::C4Touch( CBaseEntity* pOther ) 
{
	// Nothing.
}