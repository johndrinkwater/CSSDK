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

extern int gmsgBombDrop;

#define C4_WEIGHT        30
#define C4_DEFAULT_GIVE  1
#define C4_MAX_CARRY     1
#define C4_PLAYER_SPEED  250.0

enum c4_e 
{
    C4_IDLE1 = 0,
    C4_DRAW,
    C4_DROP,
    C4_PRESS_BUTTON
};

LINK_ENTITY_TO_CLASS( weapon_c4, CC4 );

void CC4::KeyValue( KeyValueData_s *pkvd )
{
    pkvd->fHandled = FALSE;
    
    if( FStrEq( pkvd->szKeyName, "detonatedelay" ) )
    {
        pev->speed = atof( pkvd->szValue );
        pkvd->fHandled = TRUE;
    }
    else if( FStrEq( pkvd->szKeyName, "detonatetarget" ) )
    {
        pev->noise1 = ALLOC_STRING( pkvd->szValue );
        pkvd->fHandled = TRUE;
    }
    else if( FStrEq( pkvd->szKeyName, "defusetarget" ) )
    {
        pev->target = ALLOC_STRING( pkvd->szValue );
        pkvd->fHandled = TRUE;
    }
}

void CC4::Spawn()
{
    SET_MODEL( ENT( pev ), "models/w_backpack.mdl" );
    
    pev->frame     = 0;
    pev->body      = 3;
    pev->sequence  = 0;
    pev->framerate = 0;
    
    m_iId          = WEAPON_C4;
    m_iDefaultAmmo = C4_DEFAULT_GIVE;

    m_fStartedArming       = FALSE;
    m_fBombPlacedAnimation = FALSE;
    
    if( pev->targetname )
    {
        SetBits( pev->effects, EF_NODRAW );
        DROP_TO_FLOOR( ENT( pev ) );
    }
    else
    {
        FallInit(); 

        SetThink( &CBasePlayerItem::FallThink );
        pev->nextthink = UTIL_WeaponTimeBase() + 0.1;
    }
}

void CC4::Precache()
{
    PRECACHE_MODEL( "models/v_c4.mdl" );
    PRECACHE_MODEL( "models/w_backpack.mdl" );
    
    PRECACHE_SOUND( "weapons/c4_click.wav" );
}

int CC4::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "C4";
    p->iMaxAmmo1 = C4_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = WEAPON_NOCLIP;
    p->iSlot     = 4;
    p->iPosition = 3;
    p->iId       = m_iId = WEAPON_C4;
    p->iWeight   = C4_WEIGHT;
    p->iFlags    = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;;

    return 1;
}
    
int CC4::iItemSlot()
{
    return 5;
}

BOOL CC4::UseDecrement()
{
#if defined( CLIENT_WEAPONS )
    return TRUE;
#else
    return FALSE;
#endif
}

float CC4::GetMaxSpeed()
{
    return C4_PLAYER_SPEED;
}

BOOL CC4::Deploy()
{
    pev->body = 0;

    m_fStartedArming       = FALSE;
    m_fBombPlacedAnimation = FALSE;

    if( m_pPlayer->HasShield() )
    {
        m_fUnknown80 = TRUE;
        m_pPlayer->pev->gamestate = 1;
    }
    
    return DefaultDeploy( "models/v_c4.mdl", "models/p_c4.mdl", C4_DRAW, "c4", UseDecrement() );
}

void CC4::Holster()
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
    m_fStartedArming = FALSE;
    
    if( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
    {
        ClearBits( m_pPlayer->pev->weapons, 1 << WEAPON_C4 );
        DestroyItem();
    }
    
    if( m_fUnknown80 )
    {
        m_pPlayer->pev->gamestate = 0;
        m_fUnknown80 = FALSE;
    }
}

void CC4::PrimaryAttack()
{
    if( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        return;
    }
    
    BOOL onGround   = FBitSet( m_pPlayer->pev->flags, FL_ONGROUND );
    BOOL inBombZone = FBitSet( m_pPlayer->m_fMapZone, MAPZONE_BUY );
    
    if( !m_fStartedArming )
    {
        if( inBombZone && onGround )
        {
            m_fStartedArming = TRUE;
            m_flArmedTime     = gpGlobals->time + 3.0;
            
            SendWeaponAnim( C4_PRESS_BUTTON, UseDecrement() );
            SET_CLIENT_MAXSPEED( ENT( m_pPlayer->pev ), 1.0 );
            
            m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
            m_pPlayer->SetProgressBarTime( 3 );  
        }
        else
        {
            ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, inBombZone ? "#C4_Plant_At_Bomb_Spot" : "#C4_Plant_Must_Be_On_Ground" );
            m_flNextPrimaryAttack = gpGlobals->time + 1.0;
        }
       
        return;
    }
    else
    {
        if( !onGround || !inBombZone )
        {
            ClientPrint( m_pPlayer->pev, print_center, inBombZone ? "#C4_Plant_Must_Be_On_Ground" : "#C4_Arming_Cancelled" );
            
            m_fStartedArming      = FALSE;
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5;
            
            m_pPlayer->ResetMaxSpeed();
            m_pPlayer->SetProgressBarTime( 0 );
            m_pPlayer->SetAnimation( PLAYER_HOLDBOMB );
            
            SendWeaponAnim( m_fBombPlacedAnimation ? C4_DRAW : C4_IDLE1, UseDecrement() ? 1: 0 );
            return;
        }
        else if( gpGlobals->time < m_flArmedTime && m_flArmedTime - 0.75 <= gpGlobals->time && !m_fBombPlacedAnimation )
        {
            m_fBombPlacedAnimation = TRUE;

            SendWeaponAnim( C4_DROP, UseDecrement() );
            m_pPlayer->SetAnimation( PLAYER_HOLDBOMB );
        }
    }    
        
    if( m_fStartedArming )
    {
        m_fStartedArming = FALSE;
        m_flArmedTime = 0.0;
        
        if( inBombZone )
        {
            Broadcast( "BOMBPL" );
            
            m_pPlayer->m_fHasDefuseKit = FALSE;

            CGrenade *pBomb = CGrenade::ShootSatchelCharge( m_pPlayer->pev, m_pPlayer->pev->origin, 0 );
            
            MESSAGE_BEGIN( MSG_SPEC, SVC_DIRECTOR );
                WRITE_BYTE( 9 );    
                WRITE_BYTE( DRC_CMD_EVENT ); 
                WRITE_SHORT( ENTINDEX( m_pPlayer->edict() ) );
                WRITE_SHORT( 0 );
                WRITE_LONG( 11 | DRC_FLAG_FACEPLAYER );  
            MESSAGE_END();

            MESSAGE_BEGIN( MSG_ALL, gmsgBombDrop );
                WRITE_COORD( pBomb->pev->origin.x );
                WRITE_COORD( pBomb->pev->origin.y );
                WRITE_COORD( pBomb->pev->origin.z );
                WRITE_BYTE( 1 );
            MESSAGE_END();

            UTIL_ClientPrintAll( HUD_PRINTCENTER, "#Bomb_Planted" );

            /*! @todo: Implements this :
            TheBots->OnEvent( EVENT_BOMB_PLANTED, m_pPlayer->pev, pBomb->pev ); */

            if ( g_pGameRules->IsCareer() && !m_pPlayer->IsBot() )
            {
                /*! @todo: Implements this :
                TheCareerTasks->HandleEvent( EVENT_BOMB_PLANTED, m_pPlayer->pev, NULL ); */
            }

            UTIL_LogPrintf( "\"%s<%i><%s><TERRORIST>\" triggered \"Planted_The_Bomb\"\n",
                            STRING( m_pPlayer->pev->netname ),
                            GETPLAYERUSERID( m_pPlayer->edict() ),
                            GETPLAYERAUTHID( m_pPlayer->edict() ) );

            EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/c4_plant.wav", VOL_NORM, ATTN_NORM );
                
            m_pPlayer->pev->body = 0;
            m_pPlayer->SetBombIcon( 0 );
            m_pPlayer->ResetMaxSpeed();

            if( !--m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
            {
                RetireWeapon();
            }
        }
        else
        {
            ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "#C4_Activated_At_Bomb_Spot" );
            
            m_pPlayer->SetAnimation( PLAYER_HOLDBOMB );
            m_pPlayer->ResetMaxSpeed();

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
            return;
        }
    }

    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3;
    m_flTimeWeaponIdle    = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0, 15.0 );
}

void CC4::WeaponIdle()
{
    if( m_fStartedArming )
    {
        m_fStartedArming = FALSE;
        
        m_pPlayer->ResetMaxSpeed();
        m_pPlayer->SetProgressBarTime( 0 );

        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;

        SendWeaponAnim( m_fBombPlacedAnimation ? C4_DRAW : C4_IDLE1, UseDecrement() );
    }

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }
    
    if( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
    {
        SendWeaponAnim( C4_IDLE1, UseDecrement() );
        SendWeaponAnim( C4_DRAW , UseDecrement() );
    }
    else
    {
        RetireWeapon();
    }
}

void CC4::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
    if ( m_pPlayer )
    {
        return;
    }

    CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );

    if ( pPlayer )
    {
        BOOL temp = FBitSet( m_pPlayer->m_fMapZone, MAPZONE_BUY );
        ClearBits( m_pPlayer->m_fMapZone, MAPZONE_BUY );

        if( pev->speed && g_pGameRules )
        {
            g_pGameRules->m_flC4DetonateDelay = pev->speed;
        }

        EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/c4_plant.wav", VOL_NORM, ATTN_NORM );
        CGrenade::ShootSatchelCharge( pPlayer->pev, pev->origin, Vector( 0, 0, 0 ) );

        CGrenade* pGrenade= NULL;

        while( ( pGrenade = (CGrenade *)UTIL_FindEntityByClassname( pGrenade, "grenade" ) ) )
        {
            if( pGrenade->m_fPlantedC4 && pGrenade->m_flNextFreq != gpGlobals->time )
            {
                pGrenade->pev->target = pev->target;
                pGrenade->pev->noise1 = pev->noise1;

                break;
            }
        }

        SetBits( m_pPlayer->m_fMapZone, temp );
        SUB_Remove();
    }
}