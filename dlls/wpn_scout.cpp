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

#define SCOUT_WEIGHT             30
#define SCOUT_DEFAULT_GIVE       10
#define SCOUT_MAX_CARRY          AMMO_762NATO_MAX_CARRY
#define SCOUT_MAX_CLIP           10
#define SCOUT_PLAYER_SPEED       260.0
#define SCOUT_PLAYER_SPEED_ZOOM  220.0
#define SCOUT_RELOAD_TIME        2.0

#define SCOUT_PENETRATION        3
#define SCOUT_DAMAGE             75
#define SCOUT_RANGE_MODIFIER     0.96
#define SCOUT_IDLE_TIME          1.8
#define SCOUT_IDLE_INTERVAL      60.0
#define SCOUT_EJECT_BRASSE_TIME  0.56
#define SCOUT_AJUSTED_SPREAD     0.025

enum scout_e
{
    SCOUT_IDLE = 0,
    SCOUT_SHOOT1,
    SCOUT_SHOOT2,
    SCOUT_RELOAD,
    SCOUT_DRAW
};

LINK_ENTITY_TO_CLASS( weapon_scout, CSCOUT );

void CSCOUT::Precache()
{
    PRECACHE_MODEL( "models/v_scout.mdl" );
    PRECACHE_MODEL( "models/w_scout.mdl" );

    PRECACHE_SOUND( "weapons/scout_fire-1.wav"  );
    PRECACHE_SOUND( "weapons/scout_bolt.wav"    );
    PRECACHE_SOUND( "weapons/scout_clipin.wav"  );
    PRECACHE_SOUND( "weapons/scout_clipout.wav" );
    PRECACHE_SOUND( "weapons/zoom.wav"          );

    m_iShell  = m_iShellLate = PRECACHE_MODEL( "models/rshell_big.mdl" );
    m_usSCOUT = PRECACHE_EVENT( 1, "events/scout.sc" );
}

void CSCOUT::Spawn()
{
    Precache();
    m_iId = WEAPON_SCOUT;

    SET_MODEL( ENT( pev ), "models/w_scout.mdl" );

    m_iDefaultAmmo = SCOUT_DEFAULT_GIVE;
    FallInit();
}

int CSCOUT::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "762Nato";
    p->iMaxAmmo1 = SCOUT_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = SCOUT_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 9;
    p->iId       = m_iId = WEAPON_SCOUT;
    p->iWeight   = SCOUT_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CSCOUT::iItemSlot()
{
    return 1;
}

BOOL CSCOUT::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CSCOUT::GetMaxSpeed()
{
    return m_pPlayer->m_iFOV == 90 ? SCOUT_PLAYER_SPEED : SCOUT_PLAYER_SPEED_ZOOM;
}

BOOL CSCOUT::Deploy()
{
    if( DefaultDeploy( "models/v_scout.mdl", "models/p_scout.mdl", SCOUT_DRAW, "rifle", UseDecrement() ) )
    {
        m_flNextPrimaryAttack   = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.25;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;

        return TRUE;
    }

    return FALSE;
}

void CSCOUT::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        SCOUTFire( 0.2, 1.25 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 170 )
    {
        SCOUTFire( 0.075, 1.25 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        SCOUTFire( 0.0, 1.25 );
    }
    else
    {
        SCOUTFire( 0.007, 1.25 );
    }
}

void CSCOUT::SecondaryAttack()
{
    switch( m_pPlayer->m_iFOV )
    {
        case 90 : m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 40;
        case 40 : m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 15;
        case 15 : m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
    }

    /*! @todo: Implements this :
    if( TheBots )
        TheBots->OnEvent( EVENT_WEAPON_ZOOMED, m_pPlayer->pev, NULL ); */

    m_pPlayer->ResetMaxSpeed();

    EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4 );
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CSCOUT::Reload()
{
    if( DefaultReload( SCOUT_MAX_CLIP, SCOUT_RELOAD, SCOUT_RELOAD_TIME ) )
    {
        if( m_pPlayer->m_iFOV != 90 )
        {
            m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 15;
            SecondaryAttack();
        }

        m_pPlayer->SetAnimation( PLAYER_RELOAD );
    }
}

void CSCOUT::SCOUTFire( float flSpread, float flCycleTime, BOOL bUseSemi )
{
    if( m_pPlayer->pev->fov == 90 )
    {
        flSpread += SCOUT_AJUSTED_SPREAD;
    }
    else
    {
        m_pPlayer->m_fResumeZoom = TRUE;
        m_pPlayer->m_iLastZoom   = m_pPlayer->m_iFOV;
        m_pPlayer->m_iFOV        = m_pPlayer->pev->fov = 90;
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

    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
    m_pPlayer->m_flEjectBrass  = gpGlobals->time + SCOUT_EJECT_BRASSE_TIME;

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward,flSpread, 8192.0,
        SCOUT_PENETRATION, BULLET_PLAYER_762MM, SCOUT_DAMAGE, SCOUT_RANGE_MODIFIER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usSCOUT, 0.0, (float *)&g_vecZero, m_pPlayer->pev->angles,
        vecDir.x * 1000, vecDir.y * 1000, m_pPlayer->pev->angles.x * 100, m_pPlayer->pev->angles.y *100, m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ], 0 );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if ( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_pPlayer->pev->punchangle.x -= 2.0;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + SCOUT_IDLE_TIME;
}

void CSCOUT::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    if ( m_iClip )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + SCOUT_IDLE_INTERVAL;
        SendWeaponAnim( SCOUT_IDLE, UseDecrement() );
    }
}