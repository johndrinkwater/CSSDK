/**
 *  Copyright 2010,2011 Vincent Herbet, Reuben Morais, Carlos Sola.
 *
 *  This file is part of Counter-Strike SDK.
 *
 *  Counter-Strike SDK is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Counter-Strike SDK is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Counter-Strike SDK. If not, see <http://www.gnu.org/licenses/>.
 */

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

extern int gEvilImpulse101;

void CBasePlayerAmmo::Spawn( void )
{
	pev->movetype	= MOVETYPE_TOSS;
	pev->solid		= SOLID_TRIGGER;

	UTIL_SetSize( pev, Vector( -16, -16, 0 ), Vector( 16, 16, 16 ) );
	UTIL_SetOrigin( pev, pev->origin );

	SetTouch( &CBasePlayerAmmo::DefaultTouch );
}

CBaseEntity* CBasePlayerAmmo::Respawn( void )
{
	pev->effects |= EF_NODRAW;
	SetTouch( NULL );

	UTIL_SetOrigin( pev, g_pGameRules->VecAmmoRespawnSpot( this ) ); // Move to wherever I'm supposed to respawn.

	SetThink( &CBasePlayerAmmo::Materialize );
	pev->nextthink = g_pGameRules->FlAmmoRespawnTime( this );

	return this;
}

void CBasePlayerAmmo::Materialize( void )
{
	if ( pev->effects & EF_NODRAW )
	{
		EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "items/suitchargeok1.wav", VOL_NORM, ATTN_NORM, 0, 150 );

		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch( &CBasePlayerAmmo::DefaultTouch );
}

void CBasePlayerAmmo::DefaultTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() )
	{
		return;
	}

	if( AddAmmo( pOther ) )
	{
		if( g_pGameRules->AmmoShouldRespawn( this ) == GR_AMMO_RESPAWN_YES )
		{
			Respawn();
		}
		else
		{
			SetTouch( NULL );
			SetThink( &CBaseEntity::SUB_Remove );

			pev->nextthink = gpGlobals->time + 0.1;
		}
	}
	else if( gEvilImpulse101 )
	{
		// Evil impulse 101 hack, kill always
		SetTouch( NULL );
		SetThink( &CBaseEntity::SUB_Remove );

		pev->nextthink = gpGlobals->time + 0.1;
	}
}

class C338MagnumAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_9mmclip.mdl" );

		CBasePlayerAmmo::Spawn();
	}

	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_9mmclip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if( pOther->GiveAmmo( AMMO_338MAGNUMCLIP_GIVE, "338Magnum", AMMO_338MAGNUM_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM );
			return TRUE;
		}

		return FALSE;
	}
};

class C357SIGAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_9mmclip.mdl" );

		CBasePlayerAmmo::Spawn();
	}

	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_9mmclip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if( pOther->GiveAmmo( AMMO_357SIGCLIP_GIVE, "357SIG", AMMO_357SIG_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM );
			return TRUE;
		}

		return FALSE;
	}
};

class C45ACPAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_9mmclip.mdl" );

		CBasePlayerAmmo::Spawn();
	}

	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_9mmclip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if( pOther->GiveAmmo( AMMO_45ACPCLIP_GIVE, "45acp", AMMO_45ACP_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM );
			return TRUE;
		}

		return FALSE;
	}
};

class C50AEAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_9mmclip.mdl" );

		CBasePlayerAmmo::Spawn();
	}

	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_9mmclip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if( pOther->GiveAmmo( AMMO_50AECLIP_GIVE, "50AE", AMMO_50AE_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM );
			return TRUE;
		}

		return FALSE;
	}
};

class C556NatoAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_9mmclip.mdl" );

		CBasePlayerAmmo::Spawn();
	}

	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_9mmclip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if( pOther->GiveAmmo( AMMO_556NATOCLIP_GIVE, "556Nato", AMMO_556NATO_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM );
			return TRUE;
		}

		return FALSE;
	}
};

class C556NatoBoxAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_9mmclip.mdl" );

		CBasePlayerAmmo::Spawn();
	}

	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_9mmclip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if( pOther->GiveAmmo( AMMO_556NATOBOXCLIP_GIVE, "556NatoBox", AMMO_556NATOBOX_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM );
			return TRUE;
		}

		return FALSE;
	}
};

class C57MMAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_9mmclip.mdl" );

		CBasePlayerAmmo::Spawn();
	}

	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_9mmclip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if( pOther->GiveAmmo( AMMO_57MMCLIP_GIVE, "57mm", AMMO_57MM_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM );
			return TRUE;
		}

		return FALSE;
	}
};

class C762NatoAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_9mmclip.mdl" );

		CBasePlayerAmmo::Spawn();
	}

	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_9mmclip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if( pOther->GiveAmmo( AMMO_762NATOCLIP_GIVE, "762Nato", AMMO_762NATO_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM );
			return TRUE;
		}

		return FALSE;
	}
};

class C9MMAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache();
		SET_MODEL( ENT( pev ), "models/w_9mmclip.mdl" );

		CBasePlayerAmmo::Spawn();
	}

	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_9mmclip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if( pOther->GiveAmmo( AMMO_9MMCLIP_GIVE, "9mm", AMMO_9MM_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM );
			return TRUE;
		}

		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( ammo_338magnum, C338MagnumAmmo );
LINK_ENTITY_TO_CLASS( ammo_357sig, C357SIGAmmo );
LINK_ENTITY_TO_CLASS( ammo_45acp, C45ACPAmmo );
LINK_ENTITY_TO_CLASS( ammo_50ae, C50AEAmmo );
LINK_ENTITY_TO_CLASS( ammo_556nato, C556NatoAmmo );
LINK_ENTITY_TO_CLASS( ammo_556natobox, C556NatoBoxAmmo );
LINK_ENTITY_TO_CLASS( ammo_57mm, C57MMAmmo );
LINK_ENTITY_TO_CLASS( ammo_762nato, C762NatoAmmo );
LINK_ENTITY_TO_CLASS( ammo_9mm, C9MMAmmo );


