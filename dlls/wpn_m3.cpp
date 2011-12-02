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

#define M3_WEIGHT        20
#define M3_DEFAULT_GIVE  10
#define M3_MAX_CARRY     32
#define M3_MAX_CLIP      8
#define M3_PLAYER_SPEED  230.0
#define M3_NUM_SHOTS	 9

#define VECTOR_CONE_M3	Vector( 0.06750, 0.06850, 0.00 )

enum m3_e
{
    M3_IDLE1 = 0,
    M3_SHOOT1,
    M3_SHOOT2,
    M3_INSERT,
    M3_AFTER_RELOAD,
    M3_START_RELOAD,
    M3_DRAW
};

LINK_ENTITY_TO_CLASS( weapon_m3, CM3 );

void CM3::Precache()
{
    PRECACHE_MODEL( "models/v_m3.mdl" );
    PRECACHE_MODEL( "models/w_m3.mdl" );

    PRECACHE_SOUND( "weapons/m3-1.wav" );
    PRECACHE_SOUND( "weapons/m3_insertshell.wav" );
    PRECACHE_SOUND( "weapons/m3_pump.wav" );
    PRECACHE_SOUND( "weapons/reload1.wav" );
    PRECACHE_SOUND( "weapons/reload3.wav" );

    m_iShell = m_iShellLate = PRECACHE_MODEL( "models/shotgunshell.mdl" );
    m_usM3   = PRECACHE_EVENT( 1, "events/m3.sc" );
}

void CM3::Spawn()
{
    Precache();
    m_iId = WEAPON_M3;

    SET_MODEL( ENT( pev ), "models/w_m3.mdl" );

    m_iDefaultAmmo = M3_DEFAULT_GIVE;
    FallInit();
}

int CM3::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "buckshot";
    p->iMaxAmmo1 = M3_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = M3_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 5;
    p->iId       = m_iId = WEAPON_M3;
    p->iWeight   = M3_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CM3::iItemSlot()
{
    return 1;
}

BOOL CM3::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CM3::GetMaxSpeed()
{
    return M3_PLAYER_SPEED;
}

BOOL CM3::Deploy()
{
    if( DefaultDeploy( "models/v_m3.mdl", "models/p_m3.mdl", M3_DRAW, "rifle", UseDecrement() ) )
    {
        m_flNextPrimaryAttack   = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.25;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;

		return TRUE;
    }

	return FALSE;
}

void CM3::PrimaryAttack()
{
    if( m_pPlayer->pev->waterlevel == 3 )
    {
        PlayEmptySound();

        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
        return;
    }

    if( m_iClip <= 0 )
    {
        Reload();

        if( !m_iClip )
        {
            PlayEmptySound();
        }

        /*! @todo Implements me :
        if( TheBots )
            TheBots->OnEvent( EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer->pev, NULL ); */

        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
        return;
    }

    m_iClip--;
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
    m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.45;

    m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    m_pPlayer->FireBullets( M3_NUM_SHOTS, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, VECTOR_CONE_M3, 3000.0, 4, 0, 0, 0 );
    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usM3, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, 0, 0, 0, 0 );

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    if( m_iClip != 0 )
    {
        m_flPumpTime = gpGlobals->time + 0.5;
    }

    m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 0.875;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.875;

    if( m_iClip != 0 )
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
    else
        m_flTimeWeaponIdle = 0.875;

    m_fInSpecialReload = 0;

    float rand;

    if( m_pPlayer->pev->flags & FL_ONGROUND )
        rand = UTIL_SharedRandomLong( m_pPlayer->random_seed, 6, 4 );
    else
        rand = UTIL_SharedRandomLong( m_pPlayer->random_seed, 11, 8 );

    m_pPlayer->pev->punchangle.x -= rand;
	m_pPlayer->m_flEjectBrass = UTIL_WeaponTimeBase() + 0.45;
}

void CM3::Reload()
{
    if( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 || m_iClip == SHOTGUN_MAX_CLIP )
    {
        return;
    }

    if( m_flNextPrimaryAttack > UTIL_WeaponTimeBase() )
    {
        return;
    }

    switch( m_fInSpecialReload )
    {
        case 0 :
        {
            m_pPlayer->SetAnimation( PLAYER_RELOAD );

            SendWeaponAnim( M3_START_RELOAD, UseDecrement() );
            m_fInSpecialReload = 1;

            m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.55;
            m_flTimeWeaponIdle        = UTIL_WeaponTimeBase() + 0.55;
            m_flNextPrimaryAttack     = UTIL_WeaponTimeBase() + 0.55;
            m_flNextSecondaryAttack   = UTIL_WeaponTimeBase() + 0.55;
        }
        case 1 :
        {
            if( m_flTimeWeaponIdle > gpGlobals->time )
            {
                return;
            }

            m_fInSpecialReload = 2;

            SendWeaponAnim( M3_INSERT, UseDecrement() );

            m_flNextReload     = UTIL_WeaponTimeBase() + 0.45;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.45;
        }
        default :
        {
            m_iClip++;
            m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] -= 1;
            m_fInSpecialReload = 1;

            m_pPlayer->ammo_buckshot--;
        }
    }
}

void CM3::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

    if( m_flPumpTime && m_flPumpTime < gpGlobals->time )
    {
        m_flPumpTime = 0;
    }

    if( m_flTimeWeaponIdle < gpGlobals->time )
    {
        if( !m_iClip && !m_fInSpecialReload && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
        {
            Reload();
        }
        else if( m_fInSpecialReload != 0 )
        {
            if( m_iClip != M3_MAX_CLIP && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
            {
                Reload();
            }
            else
            {
                SendWeaponAnim( M3_AFTER_RELOAD, UseDecrement() );

                m_fInSpecialReload = 0;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
            }
        }
        else
        {
            SendWeaponAnim( M3_IDLE1, UseDecrement() );
        }
    }
}