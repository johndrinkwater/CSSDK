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

#define M249_WEIGHT        25
#define M249_DEFAULT_GIVE  100
#define M249_MAX_CARRY     AMMO_556NATOBOX_MAX_CARRY
#define M249_MAX_CLIP      100
#define M249_PLAYER_SPEED  220.0
#define M249_RELOAD_TIME   2.45

#define M249_DEFAULT_ACCURACY  0.2
#define M249_ACCURACY_DIVISOR  175
#define M249_ACCURACY_OFFSET   0.4
#define M249_MAX_INACCURACY    0.9
#define M249_PENETRATION       2
#define M249_DAMAGE            32
#define M249_RANGE_MODIFIER    0.97
#define M249_IDLE_TIME         1.6
#define M249_IDLE_INTERVAL     20.0

enum m249_e
{
    M249_IDLE = 0,
    M249_SHOOT1,
    M249_SHOOT2,
    M249_RELOAD,
    M249_DRAW
};

LINK_ENTITY_TO_CLASS( weapon_m249, CM249 );

void CM249::Precache()
{
    PRECACHE_MODEL( "models/v_m249.mdl" );
    PRECACHE_MODEL( "models/w_m249.mdl" );

    PRECACHE_SOUND( "weapons/m249-1.wav"         );
    PRECACHE_SOUND( "weapons/m249-2.wav"         );
    PRECACHE_SOUND( "weapons/m249_boxout.wav"    );
    PRECACHE_SOUND( "weapons/m249_boxin.wav"     );
    PRECACHE_SOUND( "weapons/m249_chain.wav"     );
    PRECACHE_SOUND( "weapons/m249_coverup.wav"   );
    PRECACHE_SOUND( "weapons/m249_coverdown.wav" );

    m_iShell = PRECACHE_MODEL( "models/rshell.mdl" );
    m_usM249 = PRECACHE_EVENT( 1, "events/m249.sc" );
}

void CM249::Spawn()
{
    Precache();
    m_iId = WEAPON_M249;

    SET_MODEL( ENT( pev ), "models/w_m249.mdl" );

    m_iDefaultAmmo = M249_DEFAULT_GIVE;
    m_flAccuracy   = M249_DEFAULT_ACCURACY;
    m_iShotsFired  = 0;

    FallInit();
}

int CM249::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "556NatoBox";
    p->iMaxAmmo1 = M249_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = M249_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 4;
    p->iId       = m_iId = WEAPON_M249;
    p->iWeight   = M249_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CM249::iItemSlot()
{
    return 1;
}

BOOL CM249::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CM249::GetMaxSpeed()
{
    return M249_PLAYER_SPEED;
}

BOOL CM249::Deploy()
{
    m_flAccuracy  = M249_DEFAULT_ACCURACY;
    m_iShotsFired = 0;

    return DefaultDeploy( "models/v_m249.mdl", "models/p_m249.mdl", M249_DRAW, "m249", UseDecrement() );
}

void CM249::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        M249Fire( m_flAccuracy * 0.5 + 0.045, 0.1 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 140 )
    {
        M249Fire( m_flAccuracy * 0.095 + 0.045, 0.1 );
    }
    else
    {
        M249Fire( m_flAccuracy * 0.03, 0.1, FALSE );
    }
}

void CM249::Reload()
{
    if( m_pPlayer->ammo_556natobox <= 0 )
    {
        return;
    }

    if( DefaultReload( M249_MAX_CLIP, M249_RELOAD, M249_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        m_flAccuracy   = M249_DEFAULT_ACCURACY;
        m_iShotsFired  = 0;
        m_fDelayFire   = FALSE;
    }
}

void CM249::M249Fire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    m_iShotsFired++;
    m_fDelayFire = TRUE;

    m_flAccuracy = ( ( m_iShotsFired * m_iShotsFired * m_iShotsFired ) / M249_ACCURACY_DIVISOR ) + M249_ACCURACY_OFFSET;

    if( m_flAccuracy > M249_MAX_INACCURACY )
    {
        m_flAccuracy = M249_MAX_INACCURACY;
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
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = NORMAL_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0,
        M249_PENETRATION, BULLET_PLAYER_556MM, M249_DAMAGE, M249_RANGE_MODIFIER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usM249, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, m_pPlayer->pev->punchangle.y * 100, FALSE, FALSE );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + M249_IDLE_TIME;

    if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        KickBack( 1.8, 0.65, 0.45, 0.125, 5, 3.5, 8 );
    }
    else if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        KickBack( 1.1, 0.5, 0.3, 0.06, 4, 3, 8 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        KickBack( 0.75, 0.325, 0.25, 0.025, 3.5, 2.5, 9 );
    }
    else
    {
        KickBack( 0.8, 0.35, 0.3, 0.03, 3.75, 3, 9 );
    }
}

void CM249::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + M249_IDLE_INTERVAL;
    SendWeaponAnim( M249_IDLE, UseDecrement() );
}