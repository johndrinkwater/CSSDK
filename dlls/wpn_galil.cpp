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

#define GALIL_WEIGHT        25
#define GALIL_DEFAULT_GIVE  35
#define GALIL_MAX_CARRY     AMMO_556NATO_MAX_CARRY
#define GALIL_MAX_CLIP      35
#define GALIL_PLAYER_SPEED  240.0
#define GALIL_RELOAD_TIME   2.45

#define GALIL_DEFAULT_ACCURACY  0.2
#define GALIL_ACCURACY_DIVISOR  200
#define GALIL_ACCURACY_OFFSET   0.35
#define GALIL_MAX_INACCURACY    1.25
#define GALIL_PENETRATION       2
#define GALIL_DAMAGE            30
#define GALIL_DAMAGE_BURST      34
#define GALIL_RANGE_MODIFIER    0.98
#define GALIL_IDLE_TIME         1.28
#define GALIL_IDLE_INTERVAL     20.0

enum galil_e
{
    GALIL_IDLE = 0,
    GALIL_RELOAD,
    GALIL_DRAW,
    GALIL_SHOOT1,
    GALIL_SHOOT2,
    GALIL_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_galil, CGalil );

void CGalil::Precache()
{
    PRECACHE_SOUND( "weapons/galil-1.wav"        );
    PRECACHE_SOUND( "weapons/galil-2.wav"        );
    PRECACHE_SOUND( "weapons/galil_clipout.wav"  );
    PRECACHE_SOUND( "weapons/galil_clipin.wav"   );
    PRECACHE_SOUND( "weapons/galil_boltpull.wav" );

    m_iShell  = PRECACHE_MODEL( "models/rshell.mdl" );
    m_usGalil = PRECACHE_EVENT( 1, "events/galil.sc" );
}

void CGalil::Spawn()
{
    Precache();
    m_iId = WEAPON_GALIL;

    SET_MODEL( ENT( pev ), "models/w_galil.mdl" );
    m_iDefaultAmmo = GALIL_DEFAULT_GIVE;

    FallInit();
}

int CGalil::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "556Nato";
    p->iMaxAmmo1 = GALIL_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = GALIL_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 17;
    p->iId       = m_iId = WEAPON_GALIL;
    p->iWeight   = GALIL_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CGalil::iItemSlot()
{
    return 1;
}

BOOL CGalil::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CGalil::GetMaxSpeed()
{
    return GALIL_PLAYER_SPEED;
}

BOOL CGalil::Deploy()
{
    m_flAccuracy  = GALIL_DEFAULT_ACCURACY;
    m_iShotsFired = 0;

    return DefaultDeploy( "models/v_galil.mdl", "models/p_galil.mdl", GALIL_DRAW, "ak47", UseDecrement() );
}

void CGalil::PrimaryAttack()
{
    if( m_pPlayer->pev->waterlevel == 3 )
    {
        PlayEmptySound();
        m_flNextPrimaryAttack = gpGlobals->time + 0.15;

        return;
    }

    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        GalilFire( m_flAccuracy * 0.03 + 0.04, 0.0875 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 140 )
    {
        GalilFire( m_flAccuracy * 0.07 + 0.04, 0.0875 );
    }
    else
    {
        GalilFire( m_flAccuracy * 0.0375, 0.0875 );
    }
}

void CGalil::SecondaryAttack()
{
    // Nothing.
}

void CGalil::Reload()
{
    if( m_pPlayer->ammo_556nato <= 0 )
    {
        return;
    }

    if( DefaultReload( GALIL_MAX_CLIP, GALIL_RELOAD, GALIL_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        m_flAccuracy  = GALIL_DEFAULT_ACCURACY;
        m_iShotsFired = 0;
        m_fDelayFire  = FALSE;
    }
}

void CGalil::GalilFire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    m_iShotsFired++;
    m_fDelayFire = TRUE;

    m_flAccuracy = ( m_iShotsFired * m_iShotsFired * m_iShotsFired / GALIL_ACCURACY_DIVISOR ) + GALIL_ACCURACY_OFFSET;

    if( m_flAccuracy > GALIL_MAX_INACCURACY )
    {
        m_flAccuracy = GALIL_MAX_INACCURACY;
    }

    if( m_iClip <= 0 )
    {
        if( m_fFireOnEmpty )
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
        }

        /*! @todo Implements me :
        if( TheBots )
            TheBots->OnEvent( EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer->pev, NULL ); */

        return;
    }

    m_iClip--;
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0,
        GALIL_PENETRATION, BULLET_PLAYER_556MM, GALIL_DAMAGE, GALIL_RANGE_MODIFIER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usGalil, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 10000000, m_pPlayer->pev->punchangle.y * 10000000, FALSE, FALSE );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GALIL_IDLE_TIME;

    if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        KickBack( 1.0, 0.45, 0.28, 0.045, 3.75, 3, 7 );
    }
    else if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        KickBack( 1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        KickBack( 0.6, 0.3, 0.2, 0.0125, 3.25, 2, 7 );
    }
    else
    {
        KickBack( 0.65, 0.35, 0.25, 0.015, 3.5, 2.25, 7 );
    }
}

void CGalil::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GALIL_IDLE_INTERVAL;
    SendWeaponAnim( GALIL_IDLE, UseDecrement() );
}