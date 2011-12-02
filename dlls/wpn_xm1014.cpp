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

#define XM1014_WEIGHT        20
#define XM1014_DEFAULT_GIVE  7
#define XM1014_MAX_CARRY     32
#define XM1014_MAX_CLIP      7
#define XM1014_PLAYER_SPEED  240.0
#define XM1014_NUM_SHOTS	 9

#define VECTOR_CONE_XM1014	Vector( 0.0725, 0.0725, 0.0 )

enum xm1014_e
{
    XM1014_IDLE1 = 0,
    XM1014_SHOOT1,
    XM1014_SHOOT2,
    XM1014_INSERT,
    XM1014_AFTER_RELOAD,
    XM1014_START_RELOAD,
    XM1014_DRAW
};

LINK_ENTITY_TO_CLASS( weapon_xm1014, CXM1014 );

void CXM1014::Precache()
{
    PRECACHE_MODEL( "models/v_xm1014.mdl" );
    PRECACHE_MODEL( "models/w_xm1014.mdl" );

    PRECACHE_SOUND( "weapons/xm1014-1.wav" );
    PRECACHE_SOUND( "weapons/reload1.wav"  );
    PRECACHE_SOUND( "weapons/reload3.wav"  );

    m_iShell = m_iShellLate = PRECACHE_MODEL( "models/shotgunshell.mdl" );
    m_usXM1014 = PRECACHE_EVENT( 1, "events/xm1014.sc" );
}

void CXM1014::Spawn()
{
    Precache();
    m_iId = WEAPON_XM1014;

    SET_MODEL( ENT( pev ), "models/w_xm1014.mdl" );

    m_iDefaultAmmo = XM1014_DEFAULT_GIVE;
    FallInit();
}

int CXM1014::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "buckshot";
    p->iMaxAmmo1 = XM1014_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = XM1014_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 12;
    p->iId       = m_iId = WEAPON_XM1014;
    p->iWeight   = XM1014_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CXM1014::iItemSlot()
{
    return 1;
}

BOOL CXM1014::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CXM1014::GetMaxSpeed()
{
    return XM1014_PLAYER_SPEED;
}

BOOL CXM1014::Deploy()
{
    return DefaultDeploy( "models/v_xm1014.mdl", "models/p_xm1014.mdl", XM1014_DRAW, "m249", UseDecrement() );
}

void CXM1014::PrimaryAttack()
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

        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
        return;
    }

    m_iClip--;
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    m_pPlayer->FireBullets( 6, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, VECTOR_CONE_XM1014, 3048.0, 4, 0, 0, 0 );

    //PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usXM1014, 0.0,
    //    (float *)&g_vecZero, (float *)&g_vecZero, m_flUnknow1, m_flUnknow2, 7, m_flUnknow1 * 100, m_iClip ? 0 : 1, 0 ); // !!

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    if( m_iClip )
    {
        m_flPumpTime = UTIL_WeaponTimeBase() + 0.125;
    }

    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.25;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.25;

    if( m_iClip )
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.25;
    else
        m_flTimeWeaponIdle = 0.75;

    m_fInSpecialReload = 0;

    float rand;

    if( m_pPlayer->pev->flags & FL_ONGROUND )
        rand = UTIL_SharedRandomLong( m_pPlayer->random_seed, 5, 3 );
    else
        rand = UTIL_SharedRandomLong( m_pPlayer->random_seed, 10, 7 );

    m_pPlayer->pev->punchangle.x -= rand;
}

void CXM1014::Reload()
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

            SendWeaponAnim( XM1014_START_RELOAD, UseDecrement() );  
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
            
            if( RANDOM_LONG( 0, 1 ) )
                EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/reload1.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG( 0, 0x1f ) );
            else
                EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/reload3.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG( 0, 0x1f ) );

            SendWeaponAnim( XM1014_INSERT, UseDecrement() );

            m_flNextReload     = UTIL_WeaponTimeBase() + 0.3;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.3;
        }
        default :
        {
            m_iClip++;
            m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ]--;
            m_fInSpecialReload = 1;

            m_pPlayer->ammo_buckshot--;
        }
    }
}

void CXM1014::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

    if( m_flPumpTime && m_flPumpTime < gpGlobals->time )
    {
        m_flPumpTime = 0;
    }

    if( m_flTimeWeaponIdle < gpGlobals->time )
    {
        if( m_iClip == 0 && m_fInSpecialReload == 0 && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
        {
            Reload();
        }
        else if( m_fInSpecialReload != 0 )
        {
            if( m_iClip != XM1014_MAX_CLIP && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
            {
                Reload();
            }
            else
            {
                SendWeaponAnim( XM1014_AFTER_RELOAD, UseDecrement() );

                m_fInSpecialReload = 0;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
            }
        }
        else
        {
            SendWeaponAnim( XM1014_IDLE1, UseDecrement() );
        }
    }
}

/*
Offsets
weapon_xm1014

57/61    m_iShellLate
59/63    m_bDelayFire
62/66    m_flAccuracy
64/68    m_iShotsFired
65/69    
66/70
78/82    m_iShell
79/83    m_bUnknown79
80/84    m_usXM1014
*/