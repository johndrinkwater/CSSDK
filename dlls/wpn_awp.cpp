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

#define AWP_WEIGHT             30
#define AWP_DEFAULT_GIVE       AMMO_338MAGNUMCLIP_GIVE
#define AWP_MAX_CARRY          AMMO_338MAGNUM_MAX_CARRY
#define AWP_MAX_CLIP           AMMO_338MAGNUMCLIP_GIVE
#define AWP_PLAYER_SPEED       210.0
#define AWP_PLAYER_SPEED_ZOOM  150.0
#define AWP_RELOAD_TIME        3.3

#define AWP_PENETRATION        3
#define AWP_DAMAGE             115
#define AWP_RANGE_MODIFIER     0.99
#define AWP_IDLE_TIME          2.0
#define AWP_IDLE_INTERVAL      20.0
#define AWP_EJECT_BRASSE_TIME  0.55
#define AWP_AJUSTED_SPREAD	   0.08

enum awp_e
{
    AWP_IDLE = 0,
    AWP_SHOOT1,
    AWP_SHOOT2,
    AWP_SHOOT3,
    AWP_RELOAD,
    AWP_DRAW
};

LINK_ENTITY_TO_CLASS( weapon_awp, CAWP );

void CAWP::Precache()
{
    PRECACHE_MODEL( "models/v_awp.mdl" );
    PRECACHE_MODEL( "models/w_awp.mdl" );

    PRECACHE_SOUND( "weapons/awp1.wav"        );
    PRECACHE_SOUND( "weapons/boltpull1.wav"   );
    PRECACHE_SOUND( "weapons/boltup.wav"      );
    PRECACHE_SOUND( "weapons/boltdown.wav"    );
    PRECACHE_SOUND( "weapons/zoom.wav"        );
    PRECACHE_SOUND( "weapons/awp_deploy.wav"  );
    PRECACHE_SOUND( "weapons/awp_clipin.wav"  );
    PRECACHE_SOUND( "weapons/awp_clipout.wav" );

    m_iShell = m_iShellLate = PRECACHE_MODEL( "models/rshell_big.mdl" );
    m_usAwp  = PRECACHE_EVENT( 1, "events/awp.sc" );
}

void CAWP::Spawn()
{
    Precache();
    m_iId = WEAPON_AWP;

    SET_MODEL( ENT( pev ), "models/w_awp.mdl" );

    m_iDefaultAmmo = AWP_DEFAULT_GIVE;
    FallInit();
}

int CAWP::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "338Magnum";
    p->iMaxAmmo1 = AWP_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = AWP_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 2;
    p->iId       = m_iId = WEAPON_AWP;
    p->iWeight   = AWP_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CAWP::iItemSlot()
{
    return 1;
}

BOOL CAWP::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CAWP::GetMaxSpeed()
{
    return m_pPlayer->m_iFOV == 90 ? AWP_PLAYER_SPEED : AWP_PLAYER_SPEED_ZOOM;
}

BOOL CAWP::Deploy()
{
    if( DefaultDeploy( "models/v_awp.mdl", "models/p_awp.mdl", AWP_DRAW, "rifle", UseDecrement() ) )
    {
        m_flNextPrimaryAttack   = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.45;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;

		return TRUE;
    }

	return FALSE;
}

void CAWP::PrimaryAttack()
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        AWPFire( 0.85, 1.45 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 140 )
    {
        AWPFire( 0.25, 1.45 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 10 )
    {
        AWPFire( 0.10, 1.45 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        AWPFire( 0.0, 1.45 );
    }
    else
    {
        AWPFire( 0.001, 1.45 );
    }
}

void CAWP::SecondaryAttack()
{
    switch( m_pPlayer->m_iFOV )
    {
        case 90 : m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 40;
        case 40 : m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 10;
        case 10 : m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
    }
    
    /*! @todo: Implements this :
    if( TheBots )
        TheBots->OnEvent( EVENT_WEAPON_ZOOMED, m_pPlayer->pev, NULL ); */
    
    m_pPlayer->ResetMaxSpeed();

    EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/zoom.wav", VOL_NORM, 2.4, 0, ATTN_NORM );
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CAWP::Reload()
{
    if( m_pPlayer->ammo_338Magnum <= 0 )
        return;

    if( DefaultReload( AWP_MAX_CLIP, AWP_RELOAD, AWP_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        if( m_pPlayer->pev->fov != 90.0 )
        {
            m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 10;
            SecondaryAttack();
        }
    }
}

void CAWP::AWPFire( float flSpread, float flCycleTime, BOOL fUseSemi )
{
    if( m_pPlayer->pev->fov == 90 )
    {
        flSpread += AWP_AJUSTED_SPREAD;
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
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = NORMAL_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    m_pPlayer->m_flEjectBrass = gpGlobals->time + AWP_EJECT_BRASSE_TIME;

    Vector vecDir = m_pPlayer->FireBullets3(
        m_pPlayer->GetGunPosition(), gpGlobals->v_forward,
        flSpread, 8192.0, AWP_PENETRATION, BULLET_PLAYER_338MAG, AWP_DAMAGE, AWP_RANGE_MODIFIER, m_pPlayer->pev, 0, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(),
        m_usAwp, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, m_pPlayer->pev->punchangle.x * 100, FALSE, FALSE );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if ( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_pPlayer->pev->punchangle.x -= 2.0;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AWP_IDLE_TIME;
}

void CAWP::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase()  )
    {
        return;
    }

    if( m_iClip )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AWP_IDLE_INTERVAL;
        SendWeaponAnim( AWP_IDLE, UseDecrement() );
    }
}