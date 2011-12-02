/***
 *  Copyright 2010,2011 Vincent Herbet.
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
#include "hltv.h"
#include "gamerules.h"

#define HEGRENADE_WEIGHT        2
#define HEGRENADE_MAX_CARRY     1
#define HEGRENADE_DEFAULT_GIVE  1
#define HEGRENADE_PLAYER_SPEED  250.0

enum hegrenade_e
{
    HEGRENADE_IDLE = 0,
    HEGRENADE_PULLPIN,
    HEGRENADE_THROW,
    HEGRENADE_DEPLOY
};

enum shieldgrenade_e
{
    GRENADE_IDLE = 0,
    GRENADE_PULLPIN,
    GRENADE_THROW,
    GRENADE_DRAW,
    GRENADE_SHIELD_IDLE,
    GRENADE_SHIELD_UP,
    GRENADE_SHIELD_DOWN
};

LINK_ENTITY_TO_CLASS( weapon_hegrenade, CHEGrenade );

void CHEGrenade::Precache()
{
    PRECACHE_MODEL( "models/v_hegrenade.mdl" );
    PRECACHE_MODEL( "models/shield/v_shield_hegrenade.mdl" );

    PRECACHE_SOUND( "weapons/hegrenade-1.wav" );
    PRECACHE_SOUND( "weapons/hegrenade-2.wav" );
    PRECACHE_SOUND( "weapons/he_bounce-1.wav" );
    PRECACHE_SOUND( "weapons/pinpull.wav"     );

    m_usExplo = PRECACHE_EVENT( 1, "events/createexplo.sc" );
}

void CHEGrenade::Spawn()
{
    Precache();
    m_iId = WEAPON_HEGRENADE;

    SET_MODEL( ENT( pev ), "models/w_hegrenade.mdl" );

    pev->dmg = 4.0;

    m_iDefaultAmmo   = HEGRENADE_DEFAULT_GIVE;
    m_flStartThrow   = 0;
    m_flReleaseThrow = -1.0;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );

    FallInit();
}

int CHEGrenade::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "HEGrenade";
    p->iMaxAmmo1 = HEGRENADE_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = WEAPON_NOCLIP;
    p->iSlot     = 3;
    p->iPosition = 1;
    p->iId       = m_iId = WEAPON_HEGRENADE;
    p->iWeight   = HEGRENADE_WEIGHT;
    p->iFlags    = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

    return 1;
}

int CHEGrenade::iItemSlot()
{
    return 4;
}

BOOL CHEGrenade::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CHEGrenade::GetMaxSpeed()
{
    return m_flWeaponSpeed;
}

BOOL CHEGrenade::CanDrop()
{
    return FALSE;
}

BOOL CHEGrenade::CanDeploy()
{
    return m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ];
}

BOOL CHEGrenade::Deploy()
{
    m_flReleaseThrow = -1.0;
    m_flWeaponSpeed  = HEGRENADE_PLAYER_SPEED;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );
    ClearBits( m_pPlayer->m_fUserPrefs, USERPREFS_SHIELD_DRAWN );

    if( m_pPlayer->HasShield() )
        return DefaultDeploy( "models/shield/v_shield_hegrenade.mdl", "models/shield/p_shield_hegrenade.mdl", GRENADE_DRAW, "shieldgren", UseDecrement() );
    else
        return DefaultDeploy( "models/v_hegrenade.mdl", "models/p_hegrenade.mdl", HEGRENADE_DEPLOY, "grenade", UseDecrement() );
}

void CHEGrenade::Holster()
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

    if( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
    {
        ClearBits( m_pPlayer->pev->weapons, 1 << WEAPON_HEGRENADE );
        DestroyItem();
    }

    m_flStartThrow   = 0;
    m_flReleaseThrow = -1.0;
}

void CHEGrenade::PrimaryAttack()
{
    if( FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
    {
        return;
    }

    if( !m_flStartThrow && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] > 0 )
    {
        m_flStartThrow   = gpGlobals->time;
        m_flReleaseThrow = 0;

        SendWeaponAnim( HEGRENADE_PULLPIN );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
    }
}

void CHEGrenade::SecondaryAttack()
{
    ShieldSecondaryFire( GRENADE_SHIELD_UP, GRENADE_SHIELD_DOWN );
}

void CHEGrenade::ShieldSecondaryFire( int animUp, int animDown )
{
    if( !m_pPlayer->HasShield() || m_flStartThrow )
    {
        return;
    }

	if( FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
	{
		ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );
		ClearBits( m_pPlayer->m_fUserPrefs, USERPREFS_SHIELD_DRAWN );

		SendWeaponAnim( animDown, UseDecrement() );
		strcpy( m_pPlayer->m_szAnimExtention, "shieldgren" );

		m_flWeaponSpeed = 250.0;
	}
	else
	{
		SetBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );
		SetBits( m_pPlayer->m_fUserPrefs, USERPREFS_SHIELD_DRAWN );

		SendWeaponAnim( animUp, UseDecrement() );
		strcpy( m_pPlayer->m_szAnimExtention, "shielded" );

		m_flWeaponSpeed = 180.0;
	}

	m_pPlayer->UpdateShieldCrosshair( ~m_fWeaponState & WEAPONSTATE_SHIELD_DRAWN );

    m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 0.3;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
    m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + 0.6;
}

void CHEGrenade::ResetPlayerShieldAnim()
{
	if( m_pPlayer->HasShield() && FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
	{
		strcpy( m_pPlayer->m_szAnimExtention, "shieldgren" );
	}
}

void CHEGrenade::SetPlayerShieldAnim()
{
	if( m_pPlayer->HasShield() )
	{
		if( FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
		{
			strcpy( m_pPlayer->m_szAnimExtention, "shield" );
		}
		else
		{
			strcpy( m_pPlayer->m_szAnimExtention, "shieldgren" );
		}
	}
}

void CHEGrenade::WeaponIdle()
{
    if( !m_flReleaseThrow && m_flStartThrow )
    {
        m_flReleaseThrow = gpGlobals->time;
    }

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    if( m_flStartThrow )
    {
        m_pPlayer->Radio( "%!MRAD_FIREINHOLE", "#Fire_in_the_hole", PITCH_NORM, TRUE );

        Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

        if ( angThrow.x < 0 )
            angThrow.x = -10 + angThrow.x * ( ( 90 - 10 ) / 90.0 );
        else
            angThrow.x = -10 + angThrow.x * ( ( 90 + 10 ) / 90.0 );

        float flVel = ( 90 - angThrow.x ) * 6;

        if ( flVel > 750 )
        {
            flVel = 750;
        }

        UTIL_MakeVectors( angThrow );

        Vector vecSrc   = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16;
        Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

        CGrenade::ShootTimed2( m_pPlayer->pev, vecSrc, vecThrow, 1.5, m_pPlayer->m_iTeam, m_usExplo );

        SendWeaponAnim( HEGRENADE_THROW, UseDecrement() );
        SetPlayerShieldAnim();

        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

        m_flStartThrow = 0;

        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
        m_flTimeWeaponIdle    = UTIL_WeaponTimeBase() + 0.75;

        m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ]--;

        if( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
        {
           m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + 0.5;
           m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
           m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 0.5;
        }

        ResetPlayerShieldAnim();
        return;
    }
    else if( m_flReleaseThrow )
    {
        m_flStartThrow = 0;

        if( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
        {
            SendWeaponAnim( HEGRENADE_DEPLOY, UseDecrement() );

            m_flReleaseThrow  = -1.0;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0, 15.0 );
        }
        else
        {
            RetireWeapon();
        }

        return;
    }

    if( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
    {
        if( m_pPlayer->HasShield() )
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;

            if( !FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
            {
                SendWeaponAnim( GRENADE_SHIELD_IDLE, UseDecrement() );
            }
        }
        else
        {
            SendWeaponAnim( HEGRENADE_IDLE, UseDecrement() );
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0, 15.0 );
        }
    }
}

/*
Offsets
weapon_hegrenade

    30/33    m_flStartThrow
    31/34    m_flReleaseThrow

    78/84    m_usExplo
*/