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

#define FAMAS_WEIGHT        75
#define FAMAS_DEFAULT_GIVE  AMMO_556NATOCLIP_GIVE
#define FAMAS_MAX_CARRY     AMMO_556NATO_MAX_CARRY
#define FAMAS_MAX_CLIP      AMMO_556NATOCLIP_GIVE
#define FAMAS_PLAYER_SPEED  240.0
#define FAMAS_RELOAD_TIME   3.3

#define FAMAS_DEFAULT_ACCURACY  0.2
#define FAMAS_ACCURACY_DIVISOR  215
#define FAMAS_ACCURACY_OFFSET   0.3
#define FAMAS_MAX_INACCURACY    1.0
#define FAMAS_PENETRATION       2
#define FAMAS_DAMAGE            30
#define FAMAS_DAMAGE_BURST      34
#define FAMAS_RANGE_MODIFIER    0.96
#define FAMAS_IDLE_TIME         1.1
#define FAMAS_IDLE_INTERVAL     20.0
#define FAMAS_CYCLE_TIME        0.55
#define FAMAS_SPREAD_OFFSET     0.01

enum famas_e 
{
    FAMAS_IDLE = 0,
    FAMAS_RELOAD,
    FAMAS_DRAW,
    FAMAS_SHOOT1,
    FAMAS_SHOOT2,
    FAMAS_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_famas, CFamas );

void CFamas::Precache()
{
    PRECACHE_MODEL("models/v_famas.mdl" );
    PRECACHE_MODEL("models/w_famas.mdl" );
    
    PRECACHE_SOUND( "weapons/famas-1.wav"        );
    PRECACHE_SOUND( "weapons/famas-2.wav"        );
    PRECACHE_SOUND( "weapons/famas_clipout.wav"  );
    PRECACHE_SOUND( "weapons/famas_clipin.wav"   );
    PRECACHE_SOUND( "weapons/famas_boltpull.wav" );
    PRECACHE_SOUND( "weapons/famas_boltslap.wav" );
    PRECACHE_SOUND( "weapons/famas_forearm.wav"  );
    PRECACHE_SOUND( "weapons/famas-burst.wav"    );

    m_iShell  = PRECACHE_MODEL( "models/rshell.mdl" );
    m_usFamas = PRECACHE_EVENT( 1, "events/famas.sc"  );
}

void CFamas::Spawn()
{
    Precache();
    m_iId = WEAPON_FAMAS;

    SET_MODEL( ENT( pev ), "models/w_famas.mdl" );
    
    m_iFamasShotsFired = 0;
    m_flFamasShoot     = 0.0;
    m_iDefaultAmmo     = FAMAS_DEFAULT_GIVE;

    FallInit();
}

int CFamas::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "556Nato";
    p->iMaxAmmo1 = FAMAS_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = FAMAS_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 18;
    p->iId       = m_iId = WEAPON_GLOCK18;
    p->iWeight   = FAMAS_WEIGHT;
    p->iFlags    = 0;
  
    return 1;
}

int CFamas::iItemSlot()
{
    return 1;
}

BOOL CFamas::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CFamas::GetMaxSpeed()
{
    return FAMAS_PLAYER_SPEED;
}

BOOL CFamas::Deploy()
{
    m_iShotsFired      = 0;
    m_iFamasShotsFired = 0;
    m_flFamasShoot     = 0.0;
    m_flAccuracy       = FAMAS_DEFAULT_ACCURACY;

    return DefaultDeploy( "models/v_famas.mdl", "models/p_famas.mdl", FAMAS_DRAW, "carbine", UseDecrement() );
}

void CFamas::PrimaryAttack()
{
    if( m_pPlayer->pev->waterlevel == 3 )
    {
        PlayEmptySound();
        m_flNextPrimaryAttack = gpGlobals->time + 0.15;
        
        return;
    }

    BOOL burstMode = FBitSet( m_fWeaponState, WEAPONSTATE_FAMAS_BURST_MODE );
    
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        FamasFire( m_flAccuracy * 0.3 + 0.03, 0.0825, FALSE, burstMode );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 140 )
    {
        FamasFire( m_flAccuracy * 0.07 + 0.030, 0.0825, FALSE, burstMode );
    }
    else
    {
        FamasFire( m_flAccuracy * 0.02, 0.0825, FALSE, burstMode );
    }
}

void CFamas::SecondaryAttack()
{
    if( FBitSet( m_fWeaponState, WEAPONSTATE_FAMAS_BURST_MODE ) )
    {
        ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "#Switch_To_FullAuto" );
        ClearBits( m_fWeaponState, WEAPONSTATE_FAMAS_BURST_MODE );
    }
    else
    {
        ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "#Switch_To_BurstFire" );
        SetBits( m_fWeaponState, WEAPONSTATE_FAMAS_BURST_MODE );
    }

    m_flNextSecondaryAttack = gpGlobals->time + 0.3;
}

void CFamas::Reload()
{
    if ( m_pPlayer->ammo_556nato <= 0 )
    {
        return;
    }
        
    if( DefaultReload( FAMAS_MAX_CLIP, FAMAS_RELOAD, FAMAS_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );
        
        if( m_pPlayer->m_iFOV != 90 )
        {
            SecondaryAttack();
        }
        
        m_flAccuracy  = 0.0;
        m_iShotsFired = 0;
        m_fDelayFire  = FALSE;
    }
}

void CFamas::FamasFire( float flSpread, float flCycleTime, BOOL fUseAutoAim, BOOL fUseBurst )
{
    if( fUseBurst )
    {
        m_iFamasShotsFired = 0;
        flCycleTime = FAMAS_CYCLE_TIME;
    }
    else
    {
        flSpread += FAMAS_SPREAD_OFFSET;
    }   
    
    m_iShotsFired++;
    m_fDelayFire = TRUE;

    m_flAccuracy = ( m_iShotsFired * m_iShotsFired * m_iShotsFired / FAMAS_ACCURACY_DIVISOR ) + FAMAS_ACCURACY_OFFSET;
    
    if( m_flAccuracy > FAMAS_MAX_INACCURACY )
    {
        m_flAccuracy = FAMAS_MAX_INACCURACY;
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
            TheBots->OnEvent( EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer->pev, NULL );
        */
        return;
    }

    m_iClip--;
    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecDir = m_pPlayer->FireBullets3( 
        m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0,
        FAMAS_PENETRATION, BULLET_PLAYER_556MM, fUseBurst ? FAMAS_DAMAGE_BURST : FAMAS_DAMAGE, FAMAS_RANGE_MODIFIER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed );

    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), 
        m_usFamas, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 
        vecDir.x, vecDir.y, m_pPlayer->pev->punchangle.x * 10000000, m_pPlayer->pev->punchangle.y * 10000000, FALSE, FALSE );
    
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + flCycleTime;
    
    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + FAMAS_IDLE_TIME;

    if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        KickBack( 1, 0.45, 0.275, 0.05, 4, 2.5, 7 );
    }
    else if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        KickBack( 1.25, 0.45, 0.22, 0.18, 5.5, 4, 5 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        KickBack( 0.575, 0.325, 0.2, 0.011, 3.25, 2, 8 );
    }
    else
    {
        KickBack( 0.625, 0.375, 0.25, 0.0125, 3.5, 2.25, 8 );
    }
    
    if( fUseBurst )
    {
        m_iFamasShotsFired++;
        
        m_fBurstSpread = flSpread;
        m_flFamasShoot = gpGlobals->time + 0.05;
    }
}

void CFamas::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + FAMAS_IDLE_INTERVAL;
    SendWeaponAnim( FAMAS_IDLE, UseDecrement() );
}

