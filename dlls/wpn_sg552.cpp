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
#include "gamerules.h"

#define SG552_WEIGHT             25
#define SG552_DEFAULT_GIVE       AMMO_556NATOCLIP_GIVE
#define SG552_MAX_CARRY          AMMO_556NATO_MAX_CARRY
#define SG552_MAX_CLIP           AMMO_556NATOCLIP_GIVE
#define SG552_PLAYER_SPEED       235.0
#define SG552_PLAYER_SPEED_ZOOM  200.0
#define SG552_RELOAD_TIME        3.0

#define SG552_ACCURACY_DEFAULT     0.2
#define SG552_ACCURACY_DIVISOR   220
#define SG552_ACCURACY_OFFSET    0.3
#define SG552_MAX_INACCURACY     1.0
#define SG552_PENETRATION        2
#define SG552_DAMAGE             33
#define SG552_RANGE_MODIFIER     0.955
#define SG552_IDLE_TIME          2.0
#define SG552_IDLE_INTERVAL      60.0

enum sg552_e
{
    SG552_IDLE = 0,
    SG552_RELOAD,
    SG552_DRAW,
    SG552_SHOOT1,
    SG552_SHOOT2,
    SG552_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_sg552, CSG552 );

void CSG552::Precache()
{
    PRECACHE_MODEL( "models/v_sg552.mdl"         );
    PRECACHE_MODEL( "models/w_sg552.mdl"         );
    PRECACHE_SOUND( "weapons/sg552-1.wav"        );
    PRECACHE_SOUND( "weapons/sg552-2.wav"        );
    PRECACHE_SOUND( "weapons/sg552_clipout.wav"  );
    PRECACHE_SOUND( "weapons/sg552_clipin.wav"   );
    PRECACHE_SOUND( "weapons/sg552_boltpull.wav" );

    m_iShell  = PRECACHE_MODEL( "models/rshell.mdl" );
    m_usSG552 = PRECACHE_EVENT( 1, "events/sg552.sc" );
}

void CSG552::Spawn()
{
    Precache();
    m_iId = WEAPON_SG552;

    SET_MODEL( ENT( pev ), "models/w_sg552.mdl" );

    m_iDefaultAmmo = SG552_DEFAULT_GIVE;
    m_flAccuracy   = SG552_ACCURACY_DEFAULT;
    m_iShotsFired  = 0;

    FallInit();
}

int CSG552::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "556Nato";
    p->iMaxAmmo1 = SG552_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = SG552_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 10;
    p->iId       = m_iId = WEAPON_SG552;
    p->iWeight   = SG552_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CSG552::iItemSlot()
{
    return 1;
}

BOOL CSG552::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CSG552::GetMaxSpeed()
{
    return m_pPlayer->m_iFOV == 90 ? SG552_PLAYER_SPEED : SG552_PLAYER_SPEED_ZOOM;
}

BOOL CSG552::Deploy()
{
    m_flAccuracy  = SG552_ACCURACY_DEFAULT;
    m_iShotsFired = 0;

    return DefaultDeploy( "models/v_sg552.mdl", "models/p_sg552.mdl", SG552_DRAW, "mp5", UseDecrement() );
}

void CSG552::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        SG552Fire( m_flAccuracy * 0.45 + 0.035, 0.0825 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 140 )
    {
        SG552Fire( m_flAccuracy * 0.075 + 0.035, 0.0825 );
    }
    else if( m_pPlayer->pev->fov == 90 )
    {
        SG552Fire( m_flAccuracy * 0.02, 0.0825 );
    }
    else
    {
        SG552Fire( m_flAccuracy * 0.02, 0.135 );
    }
}

void CSG552::SecondaryAttack()
{
    int newFov;

    if( m_pPlayer->m_iFOV == 90 )
        newFov = 55;
    else if( m_pPlayer->m_iFOV == 55 )
        newFov = 90;
    else
        newFov = 90;

    m_pPlayer->m_iFOV = m_pPlayer->pev->fov = newFov;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CSG552::Reload()
{
    if( m_pPlayer->ammo_556Nato <= 0 )
    {
        return;
    }

    if( DefaultReload( SG552_MAX_CLIP, SG552_RELOAD, SG552_RELOAD_TIME ) )
    {
        if( m_pPlayer->pev->fov != 90 )
        {
            SecondaryAttack();
        }

        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        m_flAccuracy  = SG552_ACCURACY_DEFAULT;
        m_iShotsFired = 0;
        m_fDelayFire  = FALSE;
    }
}

void CSG552::SG552Fire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    m_iShotsFired++;
    m_fDelayFire = TRUE;

    m_flAccuracy = ( m_iShotsFired * m_iShotsFired * m_iShotsFired / SG552_ACCURACY_DIVISOR ) + SG552_ACCURACY_OFFSET;

    if( m_flAccuracy > SG552_MAX_INACCURACY )
    {
        m_flAccuracy = SG552_MAX_INACCURACY;
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
        SG552_PENETRATION, BULLET_PLAYER_556MM, SG552_DAMAGE, SG552_RANGE_MODIFIER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usSG552, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, m_pPlayer->pev->punchangle.y * 100, 5, 0 );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + SG552_IDLE_TIME;

    if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        KickBack( 1, 0.45, 0.28, 0.04, 4.25, 2.5, 7 );
    }
    else if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        KickBack( 1.25, 0.45, 0.22, 0.18, 6, 4, 5 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        KickBack( 0.6, 0.35, 0.2, 0.0125, 3.7, 2, 10 );
    }
    else
    {
        KickBack( 0.625, 0.375, 0.25, 0.0125, 4, 2.25, 9 );
    }
}

void CSG552::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + SG552_IDLE_INTERVAL;
        SendWeaponAnim( SG552_IDLE, UseDecrement() );
    }
}
