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

#define USP_WEIGHT        5
#define USP_DEFAULT_GIVE  30
#define USP_MAX_CARRY     AMMO_57MM_MAX_CARRY
#define USP_MAX_CLIP      12
#define USP_PLAYER_SPEED  250.0
#define USP_RELOAD_TIME   2.7

#define USP_ACCURACY_MIN_DELAY      0.3
#define USP_ACCURACY_OFFSET         0.275
#define USP_MIN_INACCURACY          0.725
#define USP_MAX_INACCURACY          0.92
#define USP_PENETRATION             1
#define USP_DAMAGE                  34
#define USP_DAMAGE_SILENCED         30
#define USP_RANGE_MODIFIER          0.79
#define USP_IDLE_TIME               2.0
#define USP_IDLE_INTERVAL_NOSHIELD  60.0
#define USP_IDLE_INTERVAL           20.0
#define USP_ADJUSTED_CYCLE_TIME     0.075

enum usp_e
{
    USP_IDLE = 0,
    USP_SHOOT1,
    USP_SHOOT2,
    USP_SHOOT3,
    USP_SHOOTLAST,
    USP_RELOAD,
    USP_DRAW,
    USP_ADD_SILENCER,
    USP_IDLE_UNSIL,
    USP_SHOOT1_UNSIL,
    USP_SHOOT2_UNSIL,
    USP_SHOOT3_UNSIL,
    USP_SHOOTLAST_UNSIL,
    USP_RELOAD_UNSIL,
    USP_DRAW_UNSIL,
    USP_DETACH_SILENCER
};

enum shieldgun_e
{
    GUN_IDLE1 = 0,
    GUN_SHOOT1,
    GUN_SHOOT2,
    GUN_SHOOT_EMPTY,
    GUN_RELOAD,
    GUN_DRAW,
    GUN_SHIELD_IDLE,
    GUN_SHIELD_UP,
    GUN_SHIELD_DOWN
};

LINK_ENTITY_TO_CLASS( weapon_usp, CUSP );

void CUSP::Precache()
{
    PRECACHE_MODEL( "models/v_usp.mdl" );
    PRECACHE_MODEL( "models/w_usp.mdl" );
    PRECACHE_MODEL( "models/shield/v_shield_usp.mdl" );

    PRECACHE_SOUND( "weapons/usp1.wav"             );
    PRECACHE_SOUND( "weapons/usp2.wav"             );
    PRECACHE_SOUND( "weapons/usp_unsil-1.wav"      );
    PRECACHE_SOUND( "weapons/usp_clipout.wav"      );
    PRECACHE_SOUND( "weapons/usp_clipin.wav"       );
    PRECACHE_SOUND( "weapons/usp_silenced.wav"     );
    PRECACHE_SOUND( "weapons/usp_silencer_off.wav" );
    PRECACHE_SOUND( "weapons/usp_sliderelease.wav" );
    PRECACHE_SOUND( "weapons/usp_slideback.wav"    );

    m_iShell = PRECACHE_MODEL( "models/pshell.mdl" );
    m_usUSP  = PRECACHE_EVENT( 1, "events/usp.sc"  );
}

void CUSP::Spawn()
{
    Precache();
    m_iId = WEAPON_USP;

    SET_MODEL( ENT( pev ), "models/w_usp.mdl" );

    m_flAccuracy   = USP_MAX_INACCURACY;
    m_iDefaultAmmo = USP_DEFAULT_GIVE;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );

    FallInit();
}

int CUSP::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "45ACP";
    p->iMaxAmmo1 = USP_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = USP_MAX_CLIP;
    p->iSlot     = 1;
    p->iPosition = 4;
    p->iId       = m_iId = WEAPON_USP;
    p->iWeight   = USP_WEIGHT;
    p->iFlags    = 0;

    return 1;
}

int CUSP::iItemSlot()
{
    return 2;
}

BOOL CUSP::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CUSP::GetMaxSpeed()
{
    return m_flWeaponSpeed;
}

BOOL CUSP::IsPistol()
{
    return TRUE;
}

BOOL CUSP::Deploy()
{
    m_flAccuracy    = USP_MAX_INACCURACY;
    m_flWeaponSpeed = USP_PLAYER_SPEED;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );
    ClearBits( m_pPlayer->m_fUserPrefs, USERPREFS_SHIELD_DRAWN );

    if( m_pPlayer->HasShield() )
    {
        return DefaultDeploy( "models/shield/v_shield_usp.mdl", "models/shield/p_shield_usp.mdl", GUN_RELOAD, "shieldgun", UseDecrement() );
    }
    else
    {
        int animDraw = FBitSet( m_fWeaponState, WEAPONSTATE_USP_SILENCED ) ? USP_DRAW : USP_DRAW_UNSIL;
        return DefaultDeploy( "models/v_usp.mdl", "models/p_usp.mdl", animDraw, "onehanded", UseDecrement() );
    }
}

void CUSP::PrimaryAttack()
{
    if( FBitSet( m_fWeaponState, WEAPONSTATE_USP_SILENCED ) )
    {
        if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
        {
            USPFire( ( 1 - m_flAccuracy ) * 1.3, 0.225 );
        }
        else if( m_pPlayer->pev->velocity.Length2D() > 0 )
        {
            USPFire( ( 1 - m_flAccuracy ) * 0.25, 0.225 );
        }
        else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
        {
            USPFire( ( 1 - m_flAccuracy ) * 0.125, 0.225 );
        }
        else
        {
            USPFire( ( 1 - m_flAccuracy ) * 0.15, 0.225 );
        }
    }
    else
    {
        if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
        {
            USPFire( ( 1 - m_flAccuracy ) * 1.2, 0.225 );
        }
        else if( m_pPlayer->pev->velocity.Length2D() > 0 )
        {
            USPFire( ( 1 - m_flAccuracy ) * 0.225, 0.225 );
        }
        else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
        {
            USPFire( ( 1 - m_flAccuracy ) * 0.08, 0.225 );
        }
        else
        {
            USPFire( ( 1 - m_flAccuracy ) * 0.1, 0.225 );
        }
    }
}

void CUSP::SecondaryAttack()
{
    if( ShieldSecondaryFire( GUN_SHIELD_UP, GUN_SHIELD_DOWN ) )
    {
        return;
    }

    int anim;

    if( FBitSet( m_fWeaponState, WEAPONSTATE_USP_SILENCED ) )
    {
        ClearBits( m_fWeaponState, WEAPONSTATE_USP_SILENCED );
        anim = USP_DETACH_SILENCER;
    }
    else
    {
        SetBits( m_fWeaponState, WEAPONSTATE_USP_SILENCED );
        anim = USP_ADD_SILENCER;
    }

    SendWeaponAnim( anim, UseDecrement() );

    strcpy( m_pPlayer->m_szAnimExtention, "onehanded" );

    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.0;
    m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 3.0;
    m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + 3.0;
}

void CUSP::Reload()
{
    if( m_pPlayer->ammo_45acp <= 0 )
    {
        return;
    }

    int reloadAnim;

    if( m_pPlayer->HasShield() )
        reloadAnim = USP_SHOOTLAST;
    else
        reloadAnim = FBitSet( m_fWeaponState, WEAPONSTATE_USP_SILENCED ) ? USP_RELOAD : USP_RELOAD_UNSIL;

    if( DefaultReload( USP_MAX_CLIP, reloadAnim, USP_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );
        m_flAccuracy = USP_MAX_INACCURACY;
    }
}

void CUSP::USPFire( float flSpread, float flCycleTime, BOOL fFireBurst )
{
    m_iShotsFired++;
    flCycleTime -= USP_ADJUSTED_CYCLE_TIME;

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
        m_flAccuracy -= ( USP_ADJUSTED_CYCLE_TIME - ( gpGlobals->time - m_flLastFire ) ) * USP_ACCURACY_OFFSET;

        if( m_flAccuracy > 0.92 )
        {
             m_flAccuracy = 0.92;
        }
        else if( m_flAccuracy < 0.6 )
        {
            m_flAccuracy = 0.6;
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
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;

    SetPlayerShieldAnim();
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
    m_pPlayer->m_iWeaponFlash  = DIM_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    int damageAmount;

    if( FBitSet( m_fWeaponState, WEAPONSTATE_USP_SILENCED ) )
    {
        damageAmount = USP_DAMAGE_SILENCED;
    }
    else
    {
        damageAmount = USP_DAMAGE;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
    }

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 4096.0,
        USP_PENETRATION, BULLET_PLAYER_45ACP, damageAmount, USP_RANGE_MODIFIER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usUSP, 0.0, (float *)&g_vecZero, (float *)&g_vecZero,
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 100, 0, m_iClip ? FALSE : TRUE, FBitSet( m_fWeaponState, WEAPONSTATE_USP_SILENCED ) ? TRUE : FALSE );

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_pPlayer->pev->punchangle.x -= 2.0;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + USP_IDLE_TIME;

    ResetPlayerShieldAnim();
}

void CUSP::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    if( m_pPlayer->HasShield() )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + USP_IDLE_INTERVAL;

        if( FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) )
        {
            SendWeaponAnim( GUN_IDLE1, UseDecrement() );
        }
    }
    else if( m_iClip )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + USP_IDLE_INTERVAL_NOSHIELD;
        SendWeaponAnim( USP_IDLE_UNSIL * !FBitSet( m_fWeaponState, WEAPONSTATE_USP_SILENCED ), UseDecrement() );
    }
}
