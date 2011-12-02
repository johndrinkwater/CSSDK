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

#define P228_WEIGHT        5
#define P228_DEFAULT_GIVE  AMMO_357SIGCLIP_GIVE
#define P228_MAX_CARRY     AMMO_357SIG_MAX_CARRY
#define P228_MAX_CLIP      AMMO_357SIGCLIP_GIVE
#define P228_PLAYER_SPEED  250.0
#define P228_RELOAD_TIME   2.7

#define P228_ACCURACY_MIN_DELAY      0.325
#define P228_ACCURACY_OFFSET         0.3
#define P228_MIN_INACCURACY          0.60
#define P228_MAX_INACCURACY          0.90
#define P228_PENETRATION             1
#define P228_DAMAGE                  32
#define P228_RANGE_MODIFIER          0.8
#define P228_IDLE_TIME               2.0
#define P228_IDLE_INTERVAL_NOSHIELD  3.065
#define P228_IDLE_INTERVAL           20.0
#define P228_ADJUSTED_CYCLE_TIME     0.05

enum p228_e
{
    P228_IDLE1 = 0,
    P228_SHOOT1,
    P228_SHOOT2,
    P228_SHOOT3,
    P228_SHOOT_EMPTY,
    P228_RELOAD,
    P228_DRAW,
};

enum shieldgun_e
{
    GUN_IDLE1,
    GUN_SHOOT1,
    GUN_SHOOT2,
    GUN_SHOOT_EMPTY,
    GUN_RELOAD,
    GUN_DRAW,
    GUN_SHIELD_IDLE,
    GUN_SHIELD_UP,
    GUN_SHIELD_DOWN
};

LINK_ENTITY_TO_CLASS( weapon_p228, CP228 );

void CP228::Precache()
{
    PRECACHE_MODEL( "models/v_p228.mdl" );
    PRECACHE_MODEL( "models/w_p228.mdl" );
    PRECACHE_MODEL( "models/shield/v_shield_p228.mdl");

    PRECACHE_SOUND( "weapons/p228-1.wav"            );
    PRECACHE_SOUND( "weapons/p228_clipout.wav"      );
    PRECACHE_SOUND( "weapons/p228_clipin.wav"       );
    PRECACHE_SOUND( "weapons/p228_sliderelease.wav" );
    PRECACHE_SOUND( "weapons/p228_slidepull.wav"    );

    m_iShell = PRECACHE_MODEL( "models/pshell.mdl" );
    m_usP228 = PRECACHE_EVENT( 1, "events/p228.sc" );
}

void CP228::Spawn()
{
    Precache();
    m_iId = WEAPON_P228;

    SET_MODEL( ENT( pev ), "models/w_p228.mdl" );

    m_flAccuracy   = P228_MAX_INACCURACY;
    m_iDefaultAmmo = P228_DEFAULT_GIVE;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );

    FallInit();
}

int CP228::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "357SIG";
    p->iMaxAmmo1 = P228_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = P228_MAX_CLIP;
    p->iSlot     = 1;
    p->iPosition = 3;
    p->iId       = m_iId = WEAPON_P228;
    p->iWeight   = P228_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CP228::iItemSlot()
{
    return 2;
}

BOOL CP228::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CP228::GetMaxSpeed()
{
    return m_flWeaponSpeed;
}

BOOL CP228::IsPistol()
{
    return TRUE;
}

BOOL CP228::Deploy()
{
    m_flAccuracy    = P228_MAX_INACCURACY;
    m_flWeaponSpeed = P228_PLAYER_SPEED;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );
    ClearBits( m_pPlayer->m_fUserPrefs, USERPREFS_SHIELD_DRAWN );

    if( m_pPlayer->HasShield() )
    {
        return DefaultDeploy( "models/shield/v_shield_p228.mdl", "models/shield/p_shield_p228.mdl", GUN_RELOAD, "shieldgun", UseDecrement() );
    }
    else
    {
        return DefaultDeploy( "models/v_p228.mdl", "models/p_p228.mdl", P228_DRAW, "onehanded", UseDecrement() );
    }
}

void CP228::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        P228Fire( ( 1 - m_flAccuracy ) * 1.5, 0.2 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        P228Fire( ( 1 - m_flAccuracy ) * 0.255, 0.2 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        P228Fire( ( 1 - m_flAccuracy ) * 0.075, 0.2 );
    }
    else
    {
        P228Fire( ( 1 - m_flAccuracy ) * 0.15, 0.2 );
    }
}

void CP228::SecondaryAttack()
{
    ShieldSecondaryFire( GUN_SHIELD_UP, GUN_SHIELD_DOWN );
}

void CP228::Reload()
{
    if( m_pPlayer->ammo_357sig <= 0 )
    {
        return;
    }

    if( DefaultReload( P228_MAX_CLIP, m_pPlayer->HasShield() ? GUN_RELOAD : P228_RELOAD, P228_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );
        m_flAccuracy = P228_MAX_INACCURACY;
    }
}

void CP228::P228Fire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    m_iShotsFired++;
    flCycleTime -= P228_ADJUSTED_CYCLE_TIME;

    if( m_iShotsFired > 1 )
    {
        return;
    }

    if( !m_flLastFire )
    {
        m_flLastFire = gpGlobals->time;
    }
    else
    {
        m_flAccuracy -= ( P228_ACCURACY_MIN_DELAY - ( gpGlobals->time - m_flLastFire ) ) * P228_ACCURACY_OFFSET;

        if( m_flAccuracy > P228_MAX_INACCURACY )
        {
             m_flAccuracy = P228_MAX_INACCURACY;
        }
        else if( m_flAccuracy < P228_MIN_INACCURACY )
        {
            m_flAccuracy = P228_MIN_INACCURACY;
        }

        m_flLastFire = gpGlobals->time;
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
        TheBots->OnEvent( EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer->pev, NULL );*/

        return;
    }

    m_iClip--;
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    SetPlayerShieldAnim();
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
    m_pPlayer->m_iWeaponFlash  = DIM_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 4096.0,
        P228_PENETRATION, BULLET_PLAYER_357SIG, P228_DAMAGE, P228_RANGE_MODIFIER, m_pPlayer->pev, 1, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usP228, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, m_pPlayer->pev->punchangle.y * 100,  m_iClip ? FALSE : TRUE, FALSE );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_pPlayer->pev->punchangle.x -= 2.0;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P228_IDLE_TIME;

    ResetPlayerShieldAnim();
}

void CP228::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    if( m_pPlayer->HasShield() )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P228_IDLE_INTERVAL;

        if( FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
        {
            SendWeaponAnim( GUN_IDLE1, UseDecrement() );
        }
    }
    else if( m_iClip )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + P228_IDLE_INTERVAL_NOSHIELD;
        SendWeaponAnim( P228_IDLE1, UseDecrement() );
    }
}


