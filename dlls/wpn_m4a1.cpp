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

#define M4A1_WEIGHT        25
#define M4A1_DEFAULT_GIVE  AMMO_556NATOCLIP_GIVE
#define M4A1_MAX_CARRY     AMMO_556NATO_MAX_CARRY
#define M4A1_MAX_CLIP      AMMO_556NATOCLIP_GIVE
#define M4A1_PLAYER_SPEED  230.0
#define M4A1_RELOAD_TIME   3.05

#define M4A1_DEFAULT_ACCURACY         0.2
#define M4A1_ACCURACY_DIVISOR         220
#define M4A1_ACCURACY_OFFSET          0.30
#define M4A1_MAX_INACCURACY           1.0
#define M4A1_PENETRATION              2
#define M4A1_DAMAGE                   32
#define M4A1_DAMAGE_SILENCED          33
#define M4A1_RANGE_MODIFIER           0.97
#define M4A1_RANGE_MODIFIER_SILENCED  0.95
#define M4A1_IDLE_TIME                1.5
#define M4A1_IDLE_INTERVAL            20.0

enum m4a1_e
{
    M4A1_IDLE1 = 0,
    M4A1_SHOOT1,
    M4A1_SHOOT2,
    M4A1_SHOOT3,
    M4A1_RELOAD,
    M4A1_DRAW,
    M4A1_ADD_SILENCER,
    M4A1_IDLE_UNSIL,
    M4A1_SHOOT1_UNSIL,
    M4A1_SHOOT2_UNSIL,
    M4A1_SHOOT3_UNSIL,
    M4A1_RELOAD_UNSIL,
    M4A1_DRAW_UNSIL,
    M4A1_DETACH_SILENCER
};

LINK_ENTITY_TO_CLASS( weapon_m4a1, CM4A1 );

void CM4A1::Precache()
{
    PRECACHE_MODEL( "models/v_m4a1.mdl" );
    PRECACHE_MODEL( "models/w_m4a1.mdl" );

    PRECACHE_SOUND( "weapons/m4a1-1.wav"            );
    PRECACHE_SOUND( "weapons/m4a1_unsil-1.wav"      );
    PRECACHE_SOUND( "weapons/m4a1_unsil-2.wav"      );
    PRECACHE_SOUND( "weapons/m4a1_clipin.wav"       );
    PRECACHE_SOUND( "weapons/m4a1_clipout.wav"      );
    PRECACHE_SOUND( "weapons/m4a1_boltpull.wav"     );
    PRECACHE_SOUND( "weapons/m4a1_deploy.wav"       );
    PRECACHE_SOUND( "weapons/m4a1_silencer_on.wav"  );
    PRECACHE_SOUND( "weapons/m4a1_silencer_off.wav" );

    m_iShell = PRECACHE_MODEL( "models/rshell.mdl" );
    m_usM4A1 = PRECACHE_EVENT( 1, "events/m4a1.sc" );
}

void CM4A1::Spawn()
{
    Precache();
    m_iId = WEAPON_M4A1;

    SET_MODEL( ENT( pev ), "models/w_m4a1.mdl" );

    m_iDefaultAmmo = M4A1_DEFAULT_GIVE;
    m_flAccuracy   = M4A1_DEFAULT_ACCURACY;
    m_iShotsFired  = 0;
    m_fDelayFire   = TRUE;

    FallInit();
}

int CM4A1::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "556Nato";
    p->iMaxAmmo1 = M4A1_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = M4A1_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 6;
    p->iId       = m_iId = WEAPON_M4A1;
    p->iWeight   = M4A1_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CM4A1::iItemSlot()
{
    return 1;
}

BOOL CM4A1::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CM4A1::GetMaxSpeed()
{
    return M4A1_PLAYER_SPEED;
}

BOOL CM4A1::Deploy()
{
    m_flAccuracy  = M4A1_DEFAULT_ACCURACY;
    m_iShotsFired = 0;

    int deployAnim = FBitSet( m_fWeaponState, WEAPONSTATE_M4A1_SILENCED ) ? M4A1_DRAW : M4A1_DRAW_UNSIL;

    return DefaultDeploy( "models/v_m4a1.mdl", "models/p_m4a1.mdl", deployAnim, "rifle", UseDecrement() );
}

void CM4A1::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        M4A1Fire( m_flAccuracy * 0.4 + 0.035, 0.0875 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 140 )
    {
        M4A1Fire( m_flAccuracy * 0.07 + 0.035, 0.0875 );
    }
    else if( FBitSet( m_fWeaponState, WEAPONSTATE_M4A1_SILENCED ) )
    {
        M4A1Fire( m_flAccuracy * 0.025, 0.0875 );
    }
    else
    {
        M4A1Fire( m_flAccuracy * 0.02, 0.0875 );
    }
}

void CM4A1::SecondaryAttack()
{
    if( FBitSet( m_fWeaponState, WEAPONSTATE_M4A1_SILENCED ) )
    {
        ClearBits( m_fWeaponState, WEAPONSTATE_M4A1_SILENCED );
        SendWeaponAnim( M4A1_DETACH_SILENCER, UseDecrement() );
    }
    else
    {
        SetBits( m_fWeaponState, WEAPONSTATE_M4A1_SILENCED );
        SendWeaponAnim( M4A1_ADD_SILENCER, UseDecrement() );
    }

    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0;
    m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 2.0;
    m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + 2.0;
}

void CM4A1::Reload()
{
    if( m_pPlayer->ammo_556nato <= 0 )
    {
        return;
    }

    int reloadAnim = FBitSet( m_fWeaponState, WEAPONSTATE_M4A1_SILENCED ) ? M4A1_RELOAD : M4A1_RELOAD_UNSIL;

    if( DefaultReload( M4A1_MAX_CLIP, reloadAnim, M4A1_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        m_flAccuracy   = M4A1_DEFAULT_ACCURACY;
        m_iShotsFired  = 0;
        m_fDelayFire   = FALSE;
    }
}

void CM4A1::M4A1Fire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    m_iShotsFired++;
    m_fDelayFire = TRUE;

    m_flAccuracy = ( m_iShotsFired * m_iShotsFired * m_iShotsFired / M4A1_ACCURACY_DIVISOR ) + M4A1_ACCURACY_OFFSET;

    if( m_flAccuracy > M4A1_MAX_INACCURACY )
    {
        m_flAccuracy = M4A1_MAX_INACCURACY;
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
            TheBots->OnEvent( EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer->pev, NULL );  */

        return;
    }

    m_iClip--;

    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    float flDamage;
    float flRangeModifier;

    if( FBitSet( m_fWeaponState, WEAPONSTATE_M4A1_SILENCED ) )
    {
        flDamage        = M4A1_DAMAGE_SILENCED;
        flRangeModifier = M4A1_RANGE_MODIFIER_SILENCED;
    }
    else
    {
        flDamage        = M4A1_DAMAGE;
        flRangeModifier = M4A1_RANGE_MODIFIER;

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
    }

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0,
        M4A1_PENETRATION, BULLET_PLAYER_556MM, flDamage, flRangeModifier, m_pPlayer->pev, FALSE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usM4A1, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, FBitSet( m_fWeaponState, WEAPONSTATE_M4A1_SILENCED ), FALSE, FALSE );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + M4A1_IDLE_TIME;

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

void CM4A1::WeaponIdle()
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + M4A1_IDLE_INTERVAL;
    SendWeaponAnim( FBitSet( m_fWeaponState, WEAPONSTATE_M4A1_SILENCED ) ? M4A1_IDLE1 : M4A1_IDLE_UNSIL, UseDecrement() );
}