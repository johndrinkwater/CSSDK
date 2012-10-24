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

#define KNIFE_WEIGHT          0
#define KNIFE_PLAYER_SPEED    250.0
#define KNIFE_IDLE_INTERVAL   20.0
#define KNIFE_DECAL           5
#define KNIFE_BODYHIT_VOLUME  128
#define KNIFE_WALLHIT_VOLUME  512

enum knife_e
{
    KNIFE_IDLE1 = 0,
    KNIFE_SLASH1,
    KNIFE_SLASH2,
    KNIFE_DRAW,
    KNIFE_STAB,
    KNIFE_STAB_MISS,
    KNIFE_MIDSLASH1,
    KNIFE_MIDSLASH2
};

enum shieldknife_e
{
    KNIFE_IDLE = 0,
    KNIFE_SLASH,
    KNIFE_HIT,
    KNIFE_DEPLOY,
    KNIFE_SHIELD_IDLE,
    KNIFE_SHIELD_UP,
    KNIFE_SHIELD_DOWN
};

LINK_ENTITY_TO_CLASS( weapon_knife, CKnife );

void CKnife::Precache()
{
    PRECACHE_MODEL( "models/v_knife.mdl" );
    PRECACHE_MODEL( "models/shield/v_shield_knife.mdl" );
    PRECACHE_MODEL( "models/w_knife.mdl" );

    PRECACHE_SOUND( "weapons/knife_deploy1.wav"  );
    PRECACHE_SOUND( "weapons/knife_hit1.wav"     );
    PRECACHE_SOUND( "weapons/knife_hit2.wav"     );
    PRECACHE_SOUND( "weapons/knife_hit3.wav"     );
    PRECACHE_SOUND( "weapons/knife_hit4.wav"     );
    PRECACHE_SOUND( "weapons/knife_slash1.wav"   );
    PRECACHE_SOUND( "weapons/knife_slash2.wav"   );
    PRECACHE_SOUND( "weapons/knife_stab.wav"     );
    PRECACHE_SOUND( "weapons/knife_hitwall1.wav" );

    m_usKnife = PRECACHE_EVENT( 1, "events/knife.sc" );
}

void CKnife::Spawn()
{
    Precache();
    m_iId = WEAPON_KNIFE;

    SET_MODEL( ENT( pev ), "models/w_knife.mdl" );

    m_iDefaultAmmo = WEAPON_NOCLIP;
    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );

    FallInit();
}

int CKnife::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = NULL;
    p->iMaxAmmo1 = -1;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = WEAPON_NOCLIP;
    p->iSlot     = 2;
    p->iPosition = 1;
    p->iId       = m_iId = WEAPON_KNIFE;
    p->iWeight   = KNIFE_WEIGHT;

    return 1;
}

int CKnife::iItemSlot()
{
    return 3;
}

BOOL CKnife::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CKnife::GetMaxSpeed()
{
    return m_flWeaponSpeed;
}

BOOL CKnife::CanDrop()
{
    return FALSE;
}

BOOL CKnife::Deploy()
{
    EMIT_SOUND( ENT( pev ), CHAN_ITEM, "weapons/knife_deploy1.wav", 0.3, 2.4 );

    m_iSwing        = 0;
    m_flWeaponSpeed = KNIFE_PLAYER_SPEED;

    ClearBits( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN );

    if( m_pPlayer->HasShield() )
        return DefaultDeploy( "models/shield/v_shield_knife.mdl", "models/shield/p_shield_knife.mdl", KNIFE_DRAW, "shieldknife", UseDecrement() );
    else
        return DefaultDeploy( "models/v_knife.mdl", "models/p_knife.mdl", KNIFE_DRAW, "knife", UseDecrement() );
}

void CKnife::Holster()
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
}

void CKnife::SetPlayerShieldAnim()
{
    if( m_pPlayer->HasShield() )
    {
        strcpy( m_pPlayer->m_szAnimExtention, FBitSet( m_fWeaponState, WEAPONSTATE_SHIELD_DRAWN ) ? "knife" : "shieldknife" );
    }
}

void CKnife::WeaponAnimation( int anim )
{
    PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usKnife, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, anim, 2, 3, 4 );
}

void CKnife::PrimaryAttack()
{
    Swing( TRUE );
}

void CKnife::SecondaryAttack()
{
    if( ShieldSecondaryFire( KNIFE_SHIELD_UP, KNIFE_SHIELD_DOWN ) )
    {
        return;
    }

    Stab( TRUE );
    pev->nextthink = UTIL_WeaponTimeBase() + 0.35;
}

void CKnife::Smack()
{
    DecalGunshot( &m_trHit, KNIFE_DECAL );
}

void CKnife::SwingAgain()
{
    Swing( FALSE );
}

BOOL CKnife::Swing( BOOL fFirst )
{
    BOOL fDidHit = FALSE;
    TraceResult tr;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle );

    Vector vecSrc = m_pPlayer->GetGunPosition( );
    Vector vecEnd = vecSrc + gpGlobals->v_forward * 48;

    UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

    if( tr.flFraction >= 1.0 )
    {
        UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &tr );

        if( tr.flFraction < 1.0 )
        {
            CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );

            if( !pHit || pHit->IsBSPModel() )
            {
                FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );
            }

            vecEnd = tr.vecEndPos;
        }
    }

    if( tr.flFraction >= 1.0 )
    {
        if( fFirst )
        {
            if( !m_pPlayer->HasShield() )
            {
                switch( ( ( m_iSwing++ ) % 2 ) + 1 )
                {
                    case 0 : SendWeaponAnim( KNIFE_MIDSLASH1 ); break;
                    case 1 : SendWeaponAnim( KNIFE_MIDSLASH2 ); break;
                }

                m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 0.35;
                m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
            }
            else
            {
                SendWeaponAnim( KNIFE_SLASH2, UseDecrement() );

                m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 1.0;
                m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2;
            }

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;

            if( RANDOM_LONG( 0, 1 ) )
                EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94 );
            else
                EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94 );

            m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
         }
    }
    else
    {
        fDidHit = TRUE;

        if( !m_pPlayer->HasShield() )
        {
            switch( ( ( m_iSwing++ ) % 2 ) + 1 )
            {
                case 0 : SendWeaponAnim( KNIFE_MIDSLASH1 ); break;
                case 1 : SendWeaponAnim( KNIFE_MIDSLASH2 ); break;
            }

            m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 0.4;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
        }
        else
        {
            SendWeaponAnim( KNIFE_SLASH2, UseDecrement() );

            m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 1.0;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2;
        }

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;

        float flVol    = 1.0;
        BOOL hitWorld = TRUE;

        CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

        SetPlayerShieldAnim();
        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

        ClearMultiDamage();
        float damage = ( m_flNextPrimaryAttack + 0.4 < UTIL_WeaponTimeBase() ) ? 20.0 : 15.0;

        pEntity->TraceAttack( m_pPlayer->pev, damage, gpGlobals->v_forward, &tr, DMG_BULLET | DMG_NEVERGIB );
        ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );

        if( pEntity )
        {
            if( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
            {
                switch( RANDOM_LONG( 0, 3 ) )
                {
                    case 0 : EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/knife_hit1.wav", VOL_NORM, ATTN_NORM ); break;
                    case 1 : EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/knife_hit2.wav", VOL_NORM, ATTN_NORM ); break;
                    case 2 : EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/knife_hit3.wav", VOL_NORM, ATTN_NORM ); break;
                    case 3 : EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/knife_hit4.wav", VOL_NORM, ATTN_NORM ); break;
                }

                m_pPlayer->m_iWeaponVolume = 128;

                if( !pEntity->IsAlive() )
                {
                    return TRUE;
                }

                flVol    = 0.1;
                hitWorld = FALSE;
            }
        }

        if( hitWorld )
        {
            TEXTURETYPE_PlaySound( &tr, vecSrc, vecSrc + ( vecEnd - vecSrc ) * 2, KNIFE_DECAL );

            switch( RANDOM_LONG( 0, 1 ) )
            {
            case 0:
                EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG( 0, 3 ) );
                break;
            case 1:
                EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/knife_hitwall2.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG( 0, 3 ) );
                break;
            }
        }

        m_trHit = tr;

        SetThink( &CKnife::Smack );
        pev->nextthink = UTIL_WeaponTimeBase() + 0.2;

        m_pPlayer->m_iWeaponVolume = flVol * 512;
        SetPlayerShieldAnim();
    }

    return FALSE;
}

BOOL CKnife::Stab( BOOL fFirst )
{
    BOOL fDidHit = FALSE;
    TraceResult tr;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle );

    Vector vecSrc = m_pPlayer->GetGunPosition();
    Vector vecEnd = vecSrc + gpGlobals->v_forward * 48;

    UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

    if( tr.flFraction >= 1.0 )
    {
        UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &tr );

        if( tr.flFraction < 1.0 )
        {
            CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );

            if( !pHit || pHit->IsBSPModel() )
            {
                FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );
            }

            vecEnd = tr.vecEndPos;
        }
    }

    if( tr.flFraction >= 1.0 )
    {
        if( fFirst )
        {
            SendWeaponAnim( KNIFE_STAB_MISS, UseDecrement() );

            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
            m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 1.0;

            if ( RANDOM_LONG( 0, 1 ) )
                EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94 );
            else
                EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94 );

            m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
        }
    }
    else
    {
        fDidHit = TRUE;
        SendWeaponAnim( KNIFE_STAB, UseDecrement() );

        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1;
        m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 1.1;

        float flVol   = 1.0;
        BOOL hitWorld = TRUE;

        CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

        float damage = 65.0;

        if( pEntity && pEntity->IsPlayer() )
        {
            Vector2D vec2LOS;
            Vector   vecForward = gpGlobals->v_forward;

            UTIL_MakeVectors( pEntity->pev->angles );

            vec2LOS = vecForward.Make2D();
            vec2LOS = vec2LOS.Normalize();

            if( DotProduct( vec2LOS, gpGlobals->v_forward.Make2D() ) > 0.8 )
            {
                damage *= 3.0;
            }
        }

        UTIL_MakeVectors( m_pPlayer->EyePosition() );
        ClearMultiDamage();

        pEntity->TraceAttack( m_pPlayer->pev, damage, gpGlobals->v_forward, &tr, DMG_BULLET | DMG_NEVERGIB );
        ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );

        if( pEntity )
        {
            if( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
            {
                EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_stab.wav", VOL_NORM, ATTN_NORM );

                m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

                if( !pEntity->IsAlive() )
                {
                    return TRUE;
                }

                flVol     = 0.1;
                hitWorld = FALSE;
            }
        }

        if( hitWorld )
        {
            TEXTURETYPE_PlaySound( &tr, vecSrc, vecSrc + ( vecEnd - vecSrc ) * 2, KNIFE_DECAL );

            switch( RANDOM_LONG( 0, 1 ) )
            {
            case 0:
                EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG( 0, 3 ) );
                break;
            case 1:
                EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/knife_hitwall2.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG( 0, 3 ) );
                break;
            }
        }

        m_trHit = tr;

        SetThink( &CKnife::Smack );
        pev->nextthink = UTIL_WeaponTimeBase() + 0.2;

        m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;
    }

    return FALSE;
}

void CKnife::WeaponIdle()
{
    ResetEmptySound();

    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    if( FBitSet( m_pPlayer->m_fUserPrefs, WEAPONSTATE_SHIELD_DRAWN ) )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + KNIFE_IDLE_INTERVAL;
        SendWeaponAnim( KNIFE_IDLE1, UseDecrement() );
    }
}