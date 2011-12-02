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

#define FIVESEVEN_WEIGHT        5
#define FIVESEVEN_DEFAULT_GIVE  AMMO_57MMCLIP_GIVE
#define FIVESEVEN_MAX_CARRY     AMMO_57MM_MAX_CARRY
#define FIVESEVEN_MAX_CLIP      AMMO_57MMCLIP_GIVE
#define FIVESEVEN_PLAYER_SPEED  250.0
#define FIVESEVEN_RELOAD_TIME   2.7

#define FIVESEVEN_ACCURACY_MIN_DELAY      0.275
#define FIVESEVEN_ACCURACY_OFFSET         0.25
#define FIVESEVEN_MIN_INACCURACY          0.725
#define FIVESEVEN_MAX_INACCURACY          0.92
#define FIVESEVEN_PENETRATION             1
#define FIVESEVEN_DAMAGE                  20
#define FIVESEVEN_RANGE_MODIFIER          0.885
#define FIVESEVEN_IDLE_TIME               2.0
#define FIVESEVEN_IDLE_INTERVAL_NOSHIELD  3.065
#define FIVESEVEN_IDLE_INTERVAL           20.0
#define FIVESEVEN_ADJUSTED_CYCLE_TIME     0.05

enum p228_e
{
    FIVESEVEN_IDLE1 = 0,
    FIVESEVEN_SHOOT1,
    FIVESEVEN_SHOOT2,
    FIVESEVEN_SHOOT_EMPTY,
    FIVESEVEN_RELOAD,
    FIVESEVEN_DRAW,
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

LINK_ENTITY_TO_CLASS( weapon_fiveseven, CFiveSeven );

void CFiveSeven::Precache()
{
    PRECACHE_MODEL( "models/v_fiveseven.mdl" );
    PRECACHE_MODEL( "models/w_fiveseven.mdl" );
    PRECACHE_MODEL( "models/shield/v_shield_fiveseven.mdl" );

    PRECACHE_SOUND( "weapons/fiveseven-1.wav"            );
    PRECACHE_SOUND( "weapons/fiveseven_clipout.wav"      );
    PRECACHE_SOUND( "weapons/fiveseven_clipin.wav"       );
    PRECACHE_SOUND( "weapons/fiveseven_sliderelease.wav" );
    PRECACHE_SOUND( "weapons/fiveseven_slidepull.wav"    );

    m_iShell      = PRECACHE_MODEL( "models/pshell.mdl"      );
    m_usFiveSeven = PRECACHE_EVENT( 1, "events/fiveseven.sc" );
}

void CFiveSeven::Spawn()
{
    Precache();
    m_iId = WEAPON_FIVESEVEN;

    SET_MODEL( ENT( pev ), "models/w_fiveseven.mdl" );

    m_flAccuracy   = FIVESEVEN_MAX_INACCURACY;
    m_iDefaultAmmo = FIVESEVEN_DEFAULT_GIVE;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );

    FallInit();
}

int CFiveSeven::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "57mm";
    p->iMaxAmmo1 = FIVESEVEN_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = FIVESEVEN_MAX_CLIP;
    p->iSlot     = 1;
    p->iPosition = 6;
    p->iId       = m_iId = WEAPON_FIVESEVEN;
    p->iWeight   = FIVESEVEN_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CFiveSeven::iItemSlot()
{
    return 2;
}

BOOL CFiveSeven::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CFiveSeven::GetMaxSpeed()
{
    return m_flWeaponSpeed;
}

BOOL CFiveSeven::IsPistol()
{
    return TRUE;
}

BOOL CFiveSeven::Deploy()
{
    m_flAccuracy    = FIVESEVEN_MAX_INACCURACY;
    m_flWeaponSpeed = FIVESEVEN_PLAYER_SPEED;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );
    ClearBits( m_pPlayer->m_fUserPrefs, USERPREFS_SHIELD_DRAWN );

    if( m_pPlayer->HasShield() )
        return DefaultDeploy( "models/shield/v_shield_fiveseven.mdl", "models/shield/p_shield_fiveseven.mdl", GUN_RELOAD, "shieldgun", UseDecrement() );
    else
        return DefaultDeploy( "models/v_fiveseven.mdl", "models/p_fiveseven.mdl", FIVESEVEN_DRAW, "onehanded", UseDecrement() );
}

void CFiveSeven::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        FiveSevenFire( ( 1 - m_flAccuracy ) * 1.5, 0.2 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        FiveSevenFire( ( 1 - m_flAccuracy ) * 0.255, 0.2 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        FiveSevenFire( ( 1 - m_flAccuracy ) * 0.075, 0.2 );
    }
    else
    {
        FiveSevenFire( ( 1 - m_flAccuracy ) * 0.15, 0.2 );
    }
}

void CFiveSeven::SecondaryAttack()
{
    ShieldSecondaryFire( GUN_SHIELD_UP, GUN_SHIELD_DOWN );
}

void CFiveSeven::Reload()
{
    if( m_pPlayer->ammo_57mm <= 0 )
    {
        return;
    }

    if( DefaultReload( FIVESEVEN_MAX_CLIP, m_pPlayer->HasShield() ? GUN_RELOAD : FIVESEVEN_RELOAD, FIVESEVEN_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );
        m_flAccuracy = FIVESEVEN_MAX_INACCURACY;
    }
}

void CFiveSeven::FiveSevenFire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    m_iShotsFired++;
    flCycleTime -= FIVESEVEN_ADJUSTED_CYCLE_TIME;

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
        m_flAccuracy -= ( FIVESEVEN_ACCURACY_MIN_DELAY - ( gpGlobals->time - m_flLastFire ) ) * FIVESEVEN_ACCURACY_OFFSET;

        if( m_flAccuracy > FIVESEVEN_MAX_INACCURACY )
        {
             m_flAccuracy = FIVESEVEN_MAX_INACCURACY;
        }
        else if( m_flAccuracy < FIVESEVEN_MIN_INACCURACY )
        {
            m_flAccuracy = FIVESEVEN_MIN_INACCURACY;
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

    Vector vecDir = m_pPlayer->FireBullets3(
        m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 4096.0,
        FIVESEVEN_PENETRATION, BULLET_PLAYER_57MM, FIVESEVEN_DAMAGE, FIVESEVEN_RANGE_MODIFIER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(),
        m_usFiveSeven, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, m_pPlayer->pev->punchangle.y * 100,  m_iClip ? FALSE : TRUE, FALSE );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_pPlayer->pev->punchangle.x -= 2.0;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + FIVESEVEN_IDLE_TIME;

    ResetPlayerShieldAnim();
}

void CFiveSeven::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    if( m_pPlayer->HasShield() )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + FIVESEVEN_IDLE_INTERVAL;

        if( FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
        {
            SendWeaponAnim( GUN_IDLE1, UseDecrement() );
        }
    }
    else if( m_iClip )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + FIVESEVEN_IDLE_INTERVAL_NOSHIELD;
        SendWeaponAnim( FIVESEVEN_IDLE1, UseDecrement() );
    }
}