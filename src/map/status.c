// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/cbasetypes.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/malloc.h"
#include "../common/utils.h"
#include "../common/ers.h"
#include "../common/strlib.h"

#include "map.h"
#include "path.h"
#include "pc.h"
#include "pet.h"
#include "npc.h"
#include "mob.h"
#include "clif.h"
#include "guild.h"
#include "skill.h"
#include "itemdb.h"
#include "battle.h"
#include "chrif.h"
#include "skill.h"
#include "status.h"
#include "script.h"
#include "unit.h"
#include "homunculus.h"
#include "mercenary.h"
#include "elemental.h"
#include "vending.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>

struct status_interface status_s;

/**
* Returns the status change associated with a skill.
* @param skill The skill to look up
* @return The status registered for this skill
**/
sc_type status_skill2sc(int skill_id) {
	int idx;
	if( (idx = skill->get_index(skill_id)) == 0 ) {
		ShowError("status_skill2sc: Unsupported skill id %d\n", skill_id);
		return SC_NONE;
	}
	return status->Skill2SCTable[idx];
}

/**
* Returns the FIRST skill (in order of definition in initChangeTables) to use a given status change.
* Utilized for various duration lookups. Use with caution!
* @param sc The status to look up
* @return A skill associated with the status
**/
int status_sc2skill(sc_type sc)
{
	if( sc < 0 || sc >= SC_MAX ) {
		ShowError("status_sc2skill: Unsupported status change id %d\n", sc);
		return 0;
	}

	return status->SkillChangeTable[sc];
}

/**
* Returns the status calculation flag associated with a given status change.
* @param sc The status to look up
* @return The scb_flag registered for this status (see enum scb_flag)
**/
unsigned int status_sc2scb_flag(sc_type sc)
{
	if( sc < 0 || sc >= SC_MAX ) {
		ShowError("status_sc2scb_flag: Unsupported status change id %d\n", sc);
		return SCB_NONE;
	}

	return status->ChangeFlagTable[sc];
}

/**
* Returns the bl types which require a status change packet to be sent for a given client status identifier.
* @param type The client-side status identifier to look up (see enum si_type)
* @return The bl types relevant to the type (see enum bl_type)
**/
int status_type2relevant_bl_types(int type)
{
	if( type < 0 || type >= SI_MAX ) {
		ShowError("status_type2relevant_bl_types: Unsupported type %d\n", type);
		return BL_NUL;
	}

	return status->RelevantBLTypes[type];
}

static void set_sc(uint16 skill_id, sc_type sc, int icon, unsigned int flag) {
	uint16 idx;
	if( (idx = skill->get_index(skill_id)) == 0 ) {
		ShowError("set_sc: Unsupported skill id %d\n", skill_id);
		return;
	}
	if( sc < 0 || sc >= SC_MAX ) {
		ShowError("set_sc: Unsupported status change id %d\n", sc);
		return;
	}

	if( status->SkillChangeTable[sc] == 0 )
		status->SkillChangeTable[sc] = skill_id;
	if( status->IconChangeTable[sc] == SI_BLANK )
		status->IconChangeTable[sc] = icon;
	status->ChangeFlagTable[sc] |= flag;

	if( status->Skill2SCTable[idx] == SC_NONE )
		status->Skill2SCTable[idx] = sc;
}

void initChangeTables(void) {
#define add_sc(skill,sc) set_sc((skill),(sc),SI_BLANK,SCB_NONE)
// indicates that the status displays a visual effect for the affected unit, and should be sent to the client for all supported units
#define set_sc_with_vfx(skill, sc, icon, flag) do { set_sc((skill), (sc), (icon), (flag)); if((icon) < SI_MAX) status->RelevantBLTypes[(icon)] |= BL_SCEFFECT; } while(0)

	int i;

	for (i = 0; i < SC_MAX; i++)
		status->IconChangeTable[i] = SI_BLANK;

	for (i = 0; i < MAX_SKILL; i++)
		status->Skill2SCTable[i] = SC_NONE;

	for (i = 0; i < SI_MAX; i++)
		status->RelevantBLTypes[i] = BL_PC;

	memset(status->SkillChangeTable, 0, sizeof(status->SkillChangeTable));
	memset(status->ChangeFlagTable, 0, sizeof(status->ChangeFlagTable));
	memset(status->DisplayType, 0, sizeof(status->DisplayType));

	//First we define the skill for common ailments. These are used in skill_additional_effect through sc cards. [Skotlex]
	set_sc( NPC_PETRIFYATTACK , SC_STONE     , SI_BLANK    , SCB_DEF_ELE|SCB_DEF|SCB_MDEF );
	set_sc( NPC_WIDEFREEZE    , SC_FREEZE    , SI_BLANK    , SCB_DEF_ELE|SCB_DEF|SCB_MDEF );
	set_sc( NPC_STUNATTACK    , SC_STUN      , SI_BLANK    , SCB_NONE );
	set_sc( NPC_SLEEPATTACK   , SC_SLEEP     , SI_BLANK    , SCB_NONE );
	set_sc( NPC_POISON        , SC_POISON    , SI_BLANK    , SCB_DEF2|SCB_REGEN );
	set_sc( NPC_CURSEATTACK   , SC_CURSE     , SI_BLANK    , SCB_LUK|SCB_BATK|SCB_WATK|SCB_SPEED );
	set_sc( NPC_SILENCEATTACK , SC_SILENCE   , SI_BLANK    , SCB_NONE );
	set_sc( NPC_WIDECONFUSE   , SC_CONFUSION , SI_BLANK    , SCB_NONE );
	set_sc( NPC_BLINDATTACK   , SC_BLIND     , SI_BLANK    , SCB_HIT|SCB_FLEE );
	set_sc( NPC_BLEEDING      , SC_BLOODING  , SI_BLOODING , SCB_REGEN );
	set_sc( NPC_POISON        , SC_DPOISON   , SI_BLANK    , SCB_DEF2|SCB_REGEN );

	//The main status definitions
	add_sc( SM_BASH              , SC_STUN            );
	set_sc( SM_PROVOKE           , SC_PROVOKE         , SI_PROVOKE         , SCB_DEF|SCB_DEF2|SCB_BATK|SCB_WATK );
	add_sc( SM_MAGNUM            , SC_SUB_WEAPONPROPERTY    );
	set_sc( SM_ENDURE            , SC_ENDURE          , SI_ENDURE          , SCB_MDEF|SCB_DSPD );
	add_sc( MG_SIGHT             , SC_SIGHT           );
	add_sc( MG_SAFETYWALL        , SC_SAFETYWALL      );
	add_sc( MG_FROSTDIVER        , SC_FREEZE          );
	add_sc( MG_STONECURSE        , SC_STONE           );
	add_sc( AL_RUWACH            , SC_RUWACH          );
	add_sc( AL_PNEUMA            , SC_PNEUMA          );
	set_sc( AL_INCAGI            , SC_INC_AGI         , SI_INC_AGI         , SCB_AGI|SCB_SPEED );
	set_sc( AL_DECAGI            , SC_DEC_AGI         , SI_DEC_AGI         , SCB_AGI|SCB_SPEED );
	set_sc( AL_CRUCIS            , SC_CRUCIS          , SI_CRUCIS          , SCB_DEF );
	set_sc( AL_ANGELUS           , SC_ANGELUS         , SI_ANGELUS         , SCB_DEF2 );
	set_sc( AL_BLESSING          , SC_BLESSING        , SI_BLESSING        , SCB_STR|SCB_INT|SCB_DEX );
	set_sc( AC_CONCENTRATION     , SC_CONCENTRATION   , SI_CONCENTRATION   , SCB_AGI|SCB_DEX );
	set_sc( TF_HIDING            , SC_HIDING          , SI_HIDING          , SCB_SPEED );
	add_sc( TF_POISON            , SC_POISON          );
	set_sc( KN_TWOHANDQUICKEN    , SC_TWOHANDQUICKEN  , SI_TWOHANDQUICKEN  , SCB_ASPD );
	add_sc( KN_AUTOCOUNTER       , SC_AUTOCOUNTER     );
	set_sc( PR_IMPOSITIO         , SC_IMPOSITIO       , SI_IMPOSITIO       , 
#ifdef RENEWAL
		SCB_NONE );
#else
		SCB_WATK );
#endif
	set_sc( PR_SUFFRAGIUM        , SC_SUFFRAGIUM      , SI_SUFFRAGIUM      , SCB_NONE );
	set_sc( PR_ASPERSIO          , SC_ASPERSIO        , SI_ASPERSIO        , SCB_ATK_ELE );
	set_sc( PR_BENEDICTIO        , SC_BENEDICTIO      , SI_BENEDICTIO      , SCB_DEF_ELE );
	set_sc( PR_SLOWPOISON        , SC_SLOWPOISON      , SI_SLOWPOISON      , SCB_REGEN );
	set_sc( PR_KYRIE             , SC_KYRIE           , SI_KYRIE           , SCB_NONE );
	set_sc( PR_MAGNIFICAT        , SC_MAGNIFICAT      , SI_MAGNIFICAT      , SCB_REGEN );
	set_sc( PR_GLORIA            , SC_GLORIA          , SI_GLORIA          , SCB_LUK );
	add_sc( PR_LEXDIVINA         , SC_SILENCE         );
	set_sc( PR_LEXAETERNA        , SC_LEXAETERNA      , SI_LEXAETERNA      , SCB_NONE );
	add_sc( WZ_METEOR            , SC_STUN            );
	add_sc( WZ_VERMILION         , SC_BLIND           );
	add_sc( WZ_FROSTNOVA         , SC_FREEZE          );
	add_sc( WZ_STORMGUST         , SC_FREEZE          );
	set_sc( WZ_QUAGMIRE          , SC_QUAGMIRE        , SI_QUAGMIRE     , SCB_AGI|SCB_DEX|SCB_ASPD|SCB_SPEED );
	set_sc( BS_ADRENALINE        , SC_ADRENALINE      , SI_ADRENALINE   , SCB_ASPD );
	set_sc( BS_WEAPONPERFECT     , SC_WEAPONPERFECT   , SI_WEAPONPERFECT, SCB_NONE );
	set_sc( BS_OVERTHRUST        , SC_OVERTHRUST      , SI_OVERTHRUST   , SCB_NONE );
	set_sc( BS_MAXIMIZE          , SC_MAXIMIZEPOWER   , SI_MAXIMIZE     , SCB_REGEN );
	add_sc( HT_LANDMINE          , SC_STUN            ); 
	set_sc( HT_ANKLESNARE        , SC_ANKLESNARE      , SI_ANKLESNARE   , SCB_NONE );
	add_sc( HT_SANDMAN           , SC_SLEEP           );
	add_sc( HT_FLASHER           , SC_BLIND           );
	add_sc( HT_FREEZINGTRAP      , SC_FREEZE          );
	set_sc( AS_CLOAKING          , SC_CLOAKING        , SI_CLOAKING     , SCB_CRI|SCB_SPEED );
	add_sc( AS_SONICBLOW         , SC_STUN            );
	set_sc( AS_ENCHANTPOISON     , SC_ENCHANTPOISON   , SI_ENCHANTPOISON, SCB_ATK_ELE );
	set_sc( AS_POISONREACT       , SC_POISONREACT     , SI_POISONREACT  , SCB_NONE );
	add_sc( AS_VENOMDUST         , SC_POISON          );
	add_sc( AS_SPLASHER          , SC_SPLASHER        );
	set_sc( NV_TRICKDEAD         , SC_TRICKDEAD    , SI_TRICKDEAD       , SCB_REGEN );
	set_sc( SM_AUTOBERSERK       , SC_AUTOBERSERK  , SI_AUTOBERSERK     , SCB_NONE );
	add_sc( TF_SPRINKLESAND      , SC_BLIND           );
	add_sc( TF_THROWSTONE        , SC_STUN            );
	set_sc( MC_LOUD              , SC_SHOUT        , SI_SHOUT           , SCB_STR );
	set_sc( MG_ENERGYCOAT        , SC_ENERGYCOAT   , SI_ENERGYCOAT      , SCB_NONE );
	set_sc( NPC_EMOTION          , SC_MODECHANGE   , SI_BLANK           , SCB_MODE );
	add_sc( NPC_EMOTION_ON       , SC_MODECHANGE   );
	set_sc( NPC_ATTRICHANGE      , SC_ARMOR_PROPERTY , SI_ARMOR_PROPERTY  , SCB_DEF_ELE );
	add_sc( NPC_CHANGEWATER      , SC_ARMOR_PROPERTY );
	add_sc( NPC_CHANGEGROUND     , SC_ARMOR_PROPERTY );
	add_sc( NPC_CHANGEFIRE       , SC_ARMOR_PROPERTY );
	add_sc( NPC_CHANGEWIND       , SC_ARMOR_PROPERTY );
	add_sc( NPC_CHANGEPOISON     , SC_ARMOR_PROPERTY );
	add_sc( NPC_CHANGEHOLY       , SC_ARMOR_PROPERTY );
	add_sc( NPC_CHANGEDARKNESS   , SC_ARMOR_PROPERTY );
	add_sc( NPC_CHANGETELEKINESIS, SC_ARMOR_PROPERTY );
	add_sc( NPC_POISON           , SC_POISON          );
	add_sc( NPC_BLINDATTACK      , SC_BLIND           );
	add_sc( NPC_SILENCEATTACK    , SC_SILENCE         );
	add_sc( NPC_STUNATTACK       , SC_STUN            );
	add_sc( NPC_PETRIFYATTACK    , SC_STONE           );
	add_sc( NPC_CURSEATTACK      , SC_CURSE           );
	add_sc( NPC_SLEEPATTACK      , SC_SLEEP           );
	add_sc( NPC_MAGICALATTACK    , SC_MAGICALATTACK   );
	set_sc( NPC_KEEPING          , SC_KEEPING         , SI_BLANK           , SCB_DEF );
	add_sc( NPC_DARKBLESSING     , SC_COMA            );
	set_sc( NPC_BARRIER          , SC_BARRIER         , SI_BLANK           , SCB_MDEF|SCB_DEF );
	add_sc( NPC_DEFENDER         , SC_ARMOR           );
	add_sc( NPC_LICK             , SC_STUN            );
	set_sc( NPC_HALLUCINATION    , SC_ILLUSION		  , SI_ILLUSION        , SCB_NONE );
	add_sc( NPC_REBIRTH          , SC_REBIRTH         );
	add_sc( RG_RAID              , SC_STUN            );
#ifdef RENEWAL
	add_sc( RG_RAID              , SC_RAID            );
	add_sc( RG_BACKSTAP          , SC_STUN            );
#endif
	set_sc( RG_STRIPWEAPON       , SC_NOEQUIPWEAPON   , SI_NOEQUIPWEAPON      , SCB_WATK );
	set_sc( RG_STRIPSHIELD       , SC_NOEQUIPSHIELD   , SI_NOEQUIPSHIELD      , SCB_DEF );
	set_sc( RG_STRIPARMOR        , SC_NOEQUIPARMOR    , SI_NOEQUIPARMOR       , SCB_VIT );
	set_sc( RG_STRIPHELM         , SC_NOEQUIPHELM     , SI_NOEQUIPHELM        , SCB_INT );
	add_sc( AM_ACIDTERROR        , SC_BLOODING        );
	set_sc( AM_CP_WEAPON         , SC_PROTECTWEAPON   , SI_PROTECTWEAPON      , SCB_NONE );
	set_sc( AM_CP_SHIELD         , SC_PROTECTSHIELD   , SI_PROTECTSHIELD      , SCB_NONE );
	set_sc( AM_CP_ARMOR          , SC_PROTECTARMOR    , SI_PROTECTARMOR       , SCB_NONE );
	set_sc( AM_CP_HELM           , SC_PROTECTHELM     , SI_PROTECTHELM        , SCB_NONE );
	set_sc( CR_AUTOGUARD         , SC_AUTOGUARD       , SI_AUTOGUARD          , SCB_NONE );
	add_sc( CR_SHIELDCHARGE      , SC_STUN            );
	set_sc( CR_REFLECTSHIELD     , SC_REFLECTSHIELD   , SI_REFLECTSHIELD      , SCB_NONE );
	add_sc( CR_HOLYCROSS         , SC_BLIND           );
	add_sc( CR_GRANDCROSS        , SC_BLIND           );
	add_sc( CR_DEVOTION          , SC_DEVOTION        );
	set_sc( CR_PROVIDENCE        , SC_PROVIDENCE      , SI_PROVIDENCE      , SCB_ALL );
	set_sc( CR_DEFENDER          , SC_DEFENDER        , SI_DEFENDER        , SCB_SPEED|SCB_ASPD );
	set_sc( CR_SPEARQUICKEN      , SC_SPEARQUICKEN    , SI_SPEARQUICKEN    , SCB_ASPD|SCB_CRI|SCB_FLEE );
	set_sc( MO_STEELBODY         , SC_STEELBODY       , SI_STEELBODY       , SCB_DEF|SCB_MDEF|SCB_ASPD|SCB_SPEED );
	add_sc( MO_BLADESTOP         , SC_BLADESTOP_WAIT  );
	add_sc( MO_BLADESTOP         , SC_BLADESTOP       );
	set_sc( MO_EXPLOSIONSPIRITS  , SC_EXPLOSIONSPIRITS, SI_EXPLOSIONSPIRITS, SCB_CRI|SCB_REGEN );
	set_sc( MO_EXTREMITYFIST     , SC_EXTREMITYFIST   , SI_BLANK           , SCB_REGEN );
#ifdef RENEWAL
	set_sc( MO_EXTREMITYFIST     , SC_EXTREMITYFIST2  , SI_EXTREMITYFIST   , SCB_NONE );
#endif
	add_sc( SA_MAGICROD          , SC_MAGICROD        );
	set_sc( SA_AUTOSPELL         , SC_AUTOSPELL       , SI_AUTOSPELL       , SCB_NONE );
	set_sc( SA_FLAMELAUNCHER     , SC_PROPERTYFIRE    , SI_PROPERTYFIRE    , SCB_ATK_ELE );
	set_sc( SA_FROSTWEAPON       , SC_PROPERTYWATER   , SI_PROPERTYWATER   , SCB_ATK_ELE );
	set_sc( SA_LIGHTNINGLOADER   , SC_PROPERTYWIND    , SI_PROPERTYWIND    , SCB_ATK_ELE );
	set_sc( SA_SEISMICWEAPON     , SC_PROPERTYGROUND  , SI_PROPERTYGROUND  , SCB_ATK_ELE );
	set_sc( SA_VOLCANO           , SC_VOLCANO         , SI_GROUNDMAGIC       , SCB_WATK );
	set_sc( SA_DELUGE            , SC_DELUGE          , SI_GROUNDMAGIC       , SCB_MAXHP );
	set_sc( SA_VIOLENTGALE       , SC_VIOLENTGALE     , SI_GROUNDMAGIC       , SCB_FLEE );
	add_sc( SA_REVERSEORCISH     , SC_ORCISH          );
	add_sc( SA_COMA              , SC_COMA            );
	set_sc( BD_ENCORE            , SC_DANCING         , SI_BLANK           , SCB_SPEED|SCB_REGEN );
	add_sc( BD_RICHMANKIM        , SC_RICHMANKIM      );
	set_sc( BD_ETERNALCHAOS      , SC_ETERNALCHAOS    , SI_BLANK           , SCB_DEF2 );
	set_sc( BD_DRUMBATTLEFIELD   , SC_DRUMBATTLE      , SI_BLANK           , SCB_WATK|SCB_DEF );
	set_sc( BD_RINGNIBELUNGEN    , SC_NIBELUNGEN      , SI_BLANK           , SCB_WATK );
	add_sc( BD_ROKISWEIL         , SC_ROKISWEIL       );
	add_sc( BD_INTOABYSS         , SC_INTOABYSS       );
	set_sc( BD_SIEGFRIED         , SC_SIEGFRIED       , SI_BLANK           , SCB_ALL );
	add_sc( BA_FROSTJOKER        , SC_FREEZE          );
	set_sc( BA_WHISTLE           , SC_WHISTLE         , SI_BLANK           , SCB_FLEE|SCB_FLEE2 );
	set_sc( BA_ASSASSINCROSS     , SC_ASSNCROS        , SI_BLANK           , SCB_ASPD );
	add_sc( BA_POEMBRAGI         , SC_POEMBRAGI       );
	set_sc( BA_APPLEIDUN         , SC_APPLEIDUN       , SI_BLANK           , SCB_MAXHP );
	add_sc( DC_SCREAM            , SC_STUN            );
	set_sc( DC_HUMMING           , SC_HUMMING         , SI_BLANK           , SCB_HIT );
	set_sc( DC_DONTFORGETME      , SC_DONTFORGETME    , SI_BLANK           , SCB_SPEED|SCB_ASPD );
	set_sc( DC_FORTUNEKISS       , SC_FORTUNE         , SI_BLANK           , SCB_CRI );
	set_sc( DC_SERVICEFORYOU     , SC_SERVICEFORYOU   , SI_BLANK           , SCB_ALL );
	add_sc( NPC_DARKCROSS        , SC_BLIND           );
	add_sc( NPC_GRANDDARKNESS    , SC_BLIND           );
	set_sc( NPC_STOP             , SC_STOP            , SI_STOP            , SCB_NONE );
	set_sc( NPC_WEAPONBRAKER     , SC_BROKENWEAPON    , SI_BROKENWEAPON    , SCB_NONE );
	set_sc( NPC_ARMORBRAKE       , SC_BROKENARMOR     , SI_BROKENARMOR     , SCB_NONE );
	set_sc( NPC_CHANGEUNDEAD     , SC_PROPERTYUNDEAD  , SI_PROPERTYUNDEAD          , SCB_DEF_ELE );
	set_sc( NPC_POWERUP          , SC_INCHITRATE      , SI_BLANK           , SCB_HIT );
	set_sc( NPC_AGIUP            , SC_INCFLEERATE     , SI_BLANK           , SCB_FLEE );
	add_sc( NPC_INVISIBLE        , SC_CLOAKING        );
	set_sc( LK_AURABLADE         , SC_AURABLADE       , SI_AURABLADE       , SCB_NONE );
	set_sc( LK_PARRYING          , SC_PARRYING        , SI_PARRYING        , SCB_NONE );
#ifndef RENEWAL
	set_sc( LK_CONCENTRATION     , SC_LKCONCENTRATION , SI_LKCONCENTRATION   , SCB_BATK|SCB_WATK|SCB_HIT|SCB_DEF|SCB_DEF2);
#else
	set_sc( LK_CONCENTRATION     , SC_LKCONCENTRATION , SI_LKCONCENTRATION   , SCB_HIT|SCB_DEF);
#endif
	set_sc( LK_TENSIONRELAX      , SC_TENSIONRELAX    , SI_TENSIONRELAX    , SCB_REGEN );
	set_sc( LK_BERSERK           , SC_BERSERK         , SI_BERSERK         , SCB_DEF|SCB_DEF2|SCB_MDEF|SCB_MDEF2|SCB_FLEE|SCB_SPEED|SCB_ASPD|SCB_MAXHP|SCB_REGEN );
	set_sc( HP_ASSUMPTIO         , SC_ASSUMPTIO       , SI_ASSUMPTIO       , SCB_NONE );
	add_sc( HP_BASILICA          , SC_BASILICA        );
	set_sc( HW_MAGICPOWER        , SC_MAGICPOWER      , SI_MAGICPOWER      , SCB_MATK );
	add_sc( PA_SACRIFICE         , SC_SACRIFICE       );
	set_sc( PA_GOSPEL            , SC_GOSPEL          , SI_BLANK           , SCB_SPEED|SCB_ASPD );
	add_sc( PA_GOSPEL            , SC_SCRESIST        );
	add_sc( CH_TIGERFIST         , SC_STOP            );
	set_sc( ASC_EDP              , SC_EDP             , SI_EDP             , SCB_NONE );
	set_sc( SN_SIGHT             , SC_TRUESIGHT       , SI_TRUESIGHT       , SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK|SCB_CRI|SCB_HIT );
	set_sc( SN_WINDWALK          , SC_WINDWALK        , SI_WINDWALK        , SCB_FLEE|SCB_SPEED );
	set_sc( WS_MELTDOWN          , SC_MELTDOWN        , SI_MELTDOWN        , SCB_NONE );
	set_sc( WS_CARTBOOST         , SC_CARTBOOST       , SI_CARTBOOST       , SCB_SPEED );
	set_sc( ST_CHASEWALK         , SC_CHASEWALK       , SI_BLANK           , SCB_SPEED );
	set_sc( ST_REJECTSWORD       , SC_SWORDREJECT     , SI_SWORDREJECT     , SCB_NONE );
	add_sc( ST_REJECTSWORD       , SC_AUTOCOUNTER     );
	set_sc( CG_MARIONETTE        , SC_MARIONETTE_MASTER      , SI_MARIONETTE_MASTER      , SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK );
	set_sc( CG_MARIONETTE        , SC_MARIONETTE     , SI_MARIONETTE     , SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK );
	add_sc( LK_SPIRALPIERCE      , SC_STOP            );
	add_sc( LK_HEADCRUSH         , SC_BLOODING        );
	set_sc( LK_JOINTBEAT         , SC_JOINTBEAT       , SI_JOINTBEAT       , SCB_BATK|SCB_DEF2|SCB_SPEED|SCB_ASPD );
	add_sc( HW_NAPALMVULCAN      , SC_CURSE           );
	set_sc( PF_MINDBREAKER       , SC_MINDBREAKER     , SI_BLANK           , SCB_MATK|SCB_MDEF2 );
	add_sc( PF_MEMORIZE          , SC_MEMORIZE        );
	add_sc( PF_FOGWALL           , SC_FOGWALL         );
	set_sc( PF_SPIDERWEB         , SC_SPIDERWEB       , SI_BLANK           , SCB_FLEE );
	set_sc( WE_BABY              , SC_BABY            , SI_PROTECTEXP      , SCB_NONE );
	set_sc( TK_RUN               , SC_RUN             , SI_RUN             , SCB_SPEED|SCB_DSPD );
	set_sc( TK_RUN               , SC_STRUP           , SI_STRUP           , SCB_STR );
	set_sc( TK_READYSTORM        , SC_STORMKICK_READY      , SI_STORMKICK_ON       , SCB_NONE );
	set_sc( TK_READYDOWN         , SC_DOWNKICK_READY       , SI_DOWNKICK_ON        , SCB_NONE );
	add_sc( TK_DOWNKICK          , SC_STUN            );
	set_sc( TK_READYTURN         , SC_TURNKICK_READY       , SI_TURNKICK_ON        , SCB_NONE );
	set_sc( TK_READYCOUNTER      , SC_COUNTERKICK_READY    , SI_COUNTER_ON         , SCB_NONE );
	set_sc( TK_DODGE             , SC_DODGE_READY          , SI_DODGE_ON           , SCB_NONE );
	set_sc( TK_SPTIME            , SC_EARTHSCROLL     , SI_EARTHSCROLL     , SCB_NONE );
	add_sc( TK_SEVENWIND         , SC_TK_SEVENWIND );
	set_sc( TK_SEVENWIND         , SC_PROPERTYTELEKINESIS     , SI_PROPERTYTELEKINESIS     , SCB_ATK_ELE );
	set_sc( TK_SEVENWIND         , SC_PROPERTYDARK    , SI_PROPERTYDARK    , SCB_ATK_ELE );
	set_sc( SG_SUN_WARM          , SC_WARM            , SI_SG_SUN_WARM      , SCB_NONE );
	add_sc( SG_MOON_WARM         , SC_WARM            );
	add_sc( SG_STAR_WARM         , SC_WARM            );
	set_sc( SG_SUN_COMFORT       , SC_SUN_COMFORT     , SI_SUN_COMFORT     , SCB_DEF2 );
	set_sc( SG_MOON_COMFORT      , SC_MOON_COMFORT    , SI_MOON_COMFORT    , SCB_FLEE );
	set_sc( SG_STAR_COMFORT      , SC_STAR_COMFORT    , SI_STAR_COMFORT    , SCB_ASPD );
	add_sc( SG_FRIEND            , SC_SKILLRATE_UP    );
	set_sc( SG_KNOWLEDGE         , SC_KNOWLEDGE       , SI_BLANK           , SCB_ALL );
	set_sc( SG_FUSION            , SC_FUSION          , SI_BLANK           , SCB_SPEED );
	set_sc( BS_ADRENALINE2       , SC_ADRENALINE2     , SI_ADRENALINE2     , SCB_ASPD );
	set_sc( SL_KAIZEL            , SC_KAIZEL          , SI_KAIZEL          , SCB_NONE );
	set_sc( SL_KAAHI             , SC_KAAHI           , SI_KAAHI           , SCB_NONE );
	set_sc( SL_KAUPE             , SC_KAUPE           , SI_KAUPE           , SCB_NONE );
	set_sc( SL_KAITE             , SC_KAITE           , SI_KAITE           , SCB_NONE );
	add_sc( SL_STUN              , SC_STUN            );
	set_sc( SL_SWOO              , SC_SWOO            , SI_BLANK           , SCB_SPEED );
	set_sc( SL_SKE               , SC_SKE             , SI_BLANK           , SCB_BATK|SCB_WATK|SCB_DEF|SCB_DEF2 );
	set_sc( SL_SKA               , SC_SKA             , SI_BLANK           , SCB_DEF|SCB_MDEF|SCB_ASPD );
	set_sc( SL_SMA               , SC_SMA_READY       , SI_SMA_READY       , SCB_NONE );
	set_sc( SM_SELFPROVOKE       , SC_PROVOKE         , SI_PROVOKE         , SCB_DEF|SCB_DEF2|SCB_BATK|SCB_WATK );
	set_sc( ST_PRESERVE          , SC_PRESERVE        , SI_PRESERVE        , SCB_NONE );
	set_sc( PF_DOUBLECASTING     , SC_DOUBLECASTING   , SI_DOUBLECASTING   , SCB_NONE );
	set_sc( HW_GRAVITATION       , SC_GRAVITATION     , SI_BLANK           , SCB_ASPD );
	add_sc( WS_CARTTERMINATION   , SC_STUN            );
	set_sc( WS_OVERTHRUSTMAX     , SC_OVERTHRUSTMAX   , SI_OVERTHRUSTMAX   , SCB_NONE );
	set_sc( CG_LONGINGFREEDOM    , SC_LONGING         , SI_BLANK           , SCB_SPEED|SCB_ASPD );
	add_sc( CG_HERMODE           , SC_HERMODE         );
	set_sc( CG_TAROTCARD         , SC_TAROTCARD       , SI_TAROTCARD       , SCB_NONE );
	set_sc( ITEM_ENCHANTARMS     , SC_ENCHANTARMS     , SI_BLANK           , SCB_ATK_ELE );
	set_sc( SL_HIGH              , SC_SOULLINK        , SI_SOULLINK        , SCB_ALL );
	set_sc( KN_ONEHAND           , SC_ONEHANDQUICKEN  , SI_ONEHANDQUICKEN  , SCB_ASPD );
	set_sc( GS_FLING             , SC_FLING           , SI_BLANK           , SCB_DEF|SCB_DEF2 );
	add_sc( GS_CRACKER           , SC_STUN            );
	add_sc( GS_DISARM            , SC_NOEQUIPWEAPON     );
	add_sc( GS_PIERCINGSHOT      , SC_BLOODING        );
	set_sc( GS_MADNESSCANCEL     , SC_GS_MADNESSCANCEL   , SI_GS_MADNESSCANCEL   , SCB_ASPD
#ifndef RENEWAL
		|SCB_BATK );
#else
		);
#endif
	set_sc( GS_ADJUSTMENT        , SC_GS_ADJUSTMENT      , SI_GS_ADJUSTMENT      , SCB_HIT|SCB_FLEE );
	set_sc( GS_INCREASING        , SC_GS_ACCURACY        , SI_GS_ACCURACY        , SCB_AGI|SCB_DEX|SCB_HIT );
	set_sc( GS_GATLINGFEVER      , SC_GS_GATLINGFEVER    , SI_GS_GATLINGFEVER    , SCB_FLEE|SCB_SPEED|SCB_ASPD
#ifndef RENEWAL
		|SCB_BATK );
#else
		);
#endif
	set_sc( NJ_TATAMIGAESHI      , SC_NJ_TATAMIGAESHI    , SI_BLANK              , SCB_NONE );
	set_sc( NJ_SUITON            , SC_NJ_SUITON          , SI_NJ_SUITON          , SCB_AGI|SCB_SPEED );
	add_sc( NJ_HYOUSYOURAKU      , SC_FREEZE          );
	set_sc( NJ_NEN               , SC_NJ_NEN             , SI_NJ_NEN             , SCB_STR|SCB_INT );
	set_sc( NJ_UTSUSEMI          , SC_NJ_UTSUSEMI        , SI_NJ_UTSUSEMI        , SCB_NONE );
	set_sc( NJ_BUNSINJYUTSU      , SC_NJ_BUNSINJYUTSU    , SI_NJ_BUNSINJYUTSU    , SCB_DYE );

	add_sc( NPC_ICEBREATH        , SC_FREEZE          );
	add_sc( NPC_ACIDBREATH       , SC_POISON          );
	add_sc( NPC_HELLJUDGEMENT    , SC_CURSE           );
	add_sc( NPC_WIDESILENCE      , SC_SILENCE         );
	add_sc( NPC_WIDEFREEZE       , SC_FREEZE          );
	add_sc( NPC_WIDEBLEEDING     , SC_BLOODING        );
	add_sc( NPC_WIDESTONE        , SC_STONE           );
	add_sc( NPC_WIDECONFUSE      , SC_CONFUSION       );
	add_sc( NPC_WIDESLEEP        , SC_SLEEP           );
	add_sc( NPC_WIDESIGHT        , SC_SIGHT           );
	add_sc( NPC_EVILLAND         , SC_BLIND           );
	add_sc( NPC_MAGICMIRROR      , SC_MAGICMIRROR     );
	set_sc( NPC_SLOWCAST         , SC_SLOWCAST        , SI_SLOWCAST        , SCB_NONE );
	set_sc( NPC_CRITICALWOUND    , SC_CRITICALWOUND   , SI_CRITICALWOUND   , SCB_NONE );
	set_sc( NPC_STONESKIN        , SC_STONESKIN     , SI_BLANK           , SCB_DEF|SCB_MDEF );
	add_sc( NPC_ANTIMAGIC        , SC_STONESKIN     );
	add_sc( NPC_WIDECURSE        , SC_CURSE           );
	add_sc( NPC_WIDESTUN         , SC_STUN            );

	set_sc( NPC_HELLPOWER        , SC_HELLPOWER       , SI_HELLPOWER       , SCB_NONE );
	set_sc( NPC_WIDEHELLDIGNITY  , SC_HELLPOWER       , SI_HELLPOWER       , SCB_NONE );
	set_sc( NPC_INVINCIBLE       , SC_INVINCIBLE      , SI_INVINCIBLE      , SCB_SPEED );
	set_sc( NPC_INVINCIBLEOFF    , SC_INVINCIBLEOFF   , SI_BLANK           , SCB_SPEED );

	set_sc( CASH_BLESSING        , SC_BLESSING        , SI_BLESSING        , SCB_STR|SCB_INT|SCB_DEX );
	set_sc( CASH_INCAGI          , SC_INC_AGI         , SI_INC_AGI         , SCB_AGI|SCB_SPEED );
	set_sc( CASH_ASSUMPTIO       , SC_ASSUMPTIO       , SI_ASSUMPTIO       , SCB_NONE );

	set_sc( ALL_PARTYFLEE        , SC_PARTYFLEE       , SI_PARTYFLEE       , SCB_NONE );
	set_sc( ALL_ODINS_POWER      , SC_ODINS_POWER     , SI_ODINS_POWER     , SCB_WATK | SCB_MATK | SCB_MDEF | SCB_DEF);

	set_sc( CR_SHRINK            , SC_CR_SHRINK       , SI_CR_SHRINK       , SCB_NONE );
	set_sc( RG_CLOSECONFINE      , SC_RG_CCONFINE_S   , SI_RG_CCONFINE_S   , SCB_NONE );
	set_sc( RG_CLOSECONFINE      , SC_RG_CCONFINE_M   , SI_RG_CCONFINE_M   , SCB_FLEE );
	set_sc( WZ_SIGHTBLASTER      , SC_WZ_SIGHTBLASTER , SI_WZ_SIGHTBLASTER , SCB_NONE );
	set_sc( DC_WINKCHARM         , SC_DC_WINKCHARM    , SI_DC_WINKCHARM    , SCB_NONE );
	add_sc( MO_BALKYOUNG         , SC_STUN            );
	add_sc( SA_ELEMENTWATER      , SC_ARMOR_PROPERTY );
	add_sc( SA_ELEMENTFIRE       , SC_ARMOR_PROPERTY );
	add_sc( SA_ELEMENTGROUND     , SC_ARMOR_PROPERTY );
	add_sc( SA_ELEMENTWIND       , SC_ARMOR_PROPERTY );

	set_sc( HLIF_AVOID           , SC_HLIF_AVOID           , SI_BLANK           , SCB_SPEED );
	set_sc( HLIF_CHANGE          , SC_HLIF_CHANGE          , SI_BLANK           , SCB_VIT|SCB_INT );
	set_sc( HFLI_FLEET           , SC_HLIF_FLEET           , SI_BLANK           , SCB_ASPD|SCB_BATK|SCB_WATK );
	set_sc( HFLI_SPEED           , SC_HLIF_SPEED           , SI_BLANK           , SCB_FLEE );
	set_sc( HAMI_DEFENCE         , SC_HAMI_DEFENCE         , SI_BLANK           , SCB_DEF );
	set_sc( HAMI_BLOODLUST       , SC_HAMI_BLOODLUST       , SI_BLANK           , SCB_BATK|SCB_WATK );

	// Homunculus S
	add_sc(MH_STAHL_HORN, SC_STUN);
	set_sc(MH_ANGRIFFS_MODUS, SC_ANGRIFFS_MODUS, SI_ANGRIFFS_MODUS, SCB_BATK | SCB_DEF | SCB_FLEE | SCB_MAXHP);
	set_sc(MH_GOLDENE_FERSE, SC_GOLDENE_FERSE, SI_GOLDENE_FERSE,  SCB_ASPD|SCB_MAXHP);
	add_sc( MH_STEINWAND, SC_SAFETYWALL );
	add_sc(MH_ERASER_CUTTER, SC_ERASER_CUTTER);
	set_sc(MH_OVERED_BOOST, SC_OVERED_BOOST, SI_BLANK, SCB_FLEE|SCB_ASPD);
	add_sc(MH_LIGHT_OF_REGENE, SC_LIGHT_OF_REGENE);
	set_sc(MH_VOLCANIC_ASH, SC_VOLCANIC_ASH, SI_VOLCANIC_ASH, SCB_DEF|SCB_DEF2|SCB_HIT|SCB_BATK|SCB_FLEE);
	set_sc(MH_GRANITIC_ARMOR, SC_GRANITIC_ARMOR, SI_GRANITIC_ARMOR, SCB_NONE);
	set_sc(MH_MAGMA_FLOW, SC_MAGMA_FLOW, SI_MAGMA_FLOW, SCB_NONE);
	set_sc(MH_PYROCLASTIC, SC_PYROCLASTIC, SI_PYROCLASTIC, SCB_BATK|SCB_ATK_ELE);
	add_sc(MH_LAVA_SLIDE, SC_BURNING);
	set_sc(MH_NEEDLE_OF_PARALYZE, SC_NEEDLE_OF_PARALYZE, SI_NEEDLE_OF_PARALYZE, SCB_DEF2);
	add_sc(MH_POISON_MIST, SC_BLIND);
	set_sc(MH_PAIN_KILLER, SC_PAIN_KILLER, SI_PAIN_KILLER, SCB_ASPD);

	add_sc(MH_STYLE_CHANGE		 , SC_STYLE_CHANGE);
	set_sc( MH_TINDER_BREAKER      , SC_RG_CCONFINE_S   , SI_RG_CCONFINE_S   , SCB_NONE );
	set_sc( MH_TINDER_BREAKER      , SC_RG_CCONFINE_M    , SI_RG_CCONFINE_M    , SCB_FLEE );


	add_sc( MER_CRASH            , SC_STUN            );
	set_sc( MER_PROVOKE          , SC_PROVOKE         , SI_PROVOKE         , SCB_DEF|SCB_DEF2|SCB_BATK|SCB_WATK );
	add_sc( MS_MAGNUM            , SC_SUB_WEAPONPROPERTY    );
	add_sc( MER_SIGHT            , SC_SIGHT           );
	set_sc( MER_DECAGI           , SC_DEC_AGI         , SI_DEC_AGI         , SCB_AGI|SCB_SPEED );
	set_sc( MER_MAGNIFICAT       , SC_MAGNIFICAT      , SI_MAGNIFICAT      , SCB_REGEN );
	add_sc( MER_LEXDIVINA        , SC_SILENCE         );
	add_sc( MA_LANDMINE          , SC_STUN            );
	add_sc( MA_SANDMAN           , SC_SLEEP           );
	add_sc( MA_FREEZINGTRAP      , SC_FREEZE          );
	set_sc( MER_AUTOBERSERK      , SC_AUTOBERSERK     , SI_AUTOBERSERK     , SCB_NONE );
	set_sc( ML_AUTOGUARD         , SC_AUTOGUARD       , SI_AUTOGUARD       , SCB_NONE );
	set_sc( MS_REFLECTSHIELD     , SC_REFLECTSHIELD   , SI_REFLECTSHIELD   , SCB_NONE );
	set_sc( ML_DEFENDER          , SC_DEFENDER        , SI_DEFENDER        , SCB_SPEED|SCB_ASPD );
	set_sc( MS_PARRYING          , SC_PARRYING        , SI_PARRYING        , SCB_NONE );
	set_sc( MS_BERSERK           , SC_BERSERK         , SI_BERSERK         , SCB_DEF|SCB_DEF2|SCB_MDEF|SCB_MDEF2|SCB_FLEE|SCB_SPEED|SCB_ASPD|SCB_MAXHP|SCB_REGEN );
	add_sc( ML_SPIRALPIERCE      , SC_STOP            );
	set_sc( MER_QUICKEN          , SC_MER_QUICKEN    , SI_BLANK           , SCB_ASPD );
	add_sc( ML_DEVOTION          , SC_DEVOTION        );
	set_sc( MER_KYRIE            , SC_KYRIE           , SI_KYRIE           , SCB_NONE );
	set_sc( MER_BLESSING         , SC_BLESSING        , SI_BLESSING        , SCB_STR|SCB_INT|SCB_DEX );
	set_sc( MER_INCAGI           , SC_INC_AGI         , SI_INC_AGI         , SCB_AGI|SCB_SPEED );

	set_sc( GD_LEADERSHIP        , SC_LEADERSHIP      , SI_BLANK           , SCB_STR );
	set_sc( GD_GLORYWOUNDS       , SC_GLORYWOUNDS     , SI_BLANK           , SCB_VIT );
	set_sc( GD_SOULCOLD          , SC_SOULCOLD        , SI_BLANK           , SCB_AGI );
	set_sc( GD_HAWKEYES          , SC_HAWKEYES        , SI_BLANK           , SCB_DEX );

	set_sc( GD_BATTLEORDER       , SC_GDSKILL_BATTLEORDER    , SI_BLANK           , SCB_STR|SCB_INT|SCB_DEX );
	set_sc( GD_REGENERATION      , SC_GDSKILL_REGENERATION    , SI_BLANK           , SCB_REGEN );

	/**
	* Rune Knight
	**/
	set_sc( RK_ENCHANTBLADE      , SC_ENCHANTBLADE      , SI_ENCHANTBLADE      , SCB_NONE );
	set_sc( RK_DRAGONHOWLING     , SC_FEAR              , SI_BLANK             , SCB_FLEE|SCB_HIT );
	set_sc( RK_DEATHBOUND        , SC_DEATHBOUND        , SI_DEATHBOUND        , SCB_NONE );
	set_sc( RK_WINDCUTTER        , SC_FEAR              , SI_BLANK             , SCB_FLEE|SCB_HIT );
	add_sc( RK_DRAGONBREATH      , SC_BURNING );
	set_sc( RK_MILLENNIUMSHIELD  , SC_MILLENNIUMSHIELD  , SI_BLANK			   , SCB_NONE );
	set_sc( RK_REFRESH           , SC_REFRESH           , SI_REFRESH           , SCB_NONE );
	set_sc( RK_GIANTGROWTH       , SC_GIANTGROWTH       , SI_GIANTGROWTH       , SCB_STR );
	set_sc( RK_STONEHARDSKIN     , SC_STONEHARDSKIN     , SI_STONEHARDSKIN     , SCB_NONE );
	set_sc( RK_VITALITYACTIVATION, SC_VITALITYACTIVATION, SI_VITALITYACTIVATION, SCB_REGEN );
	set_sc( RK_FIGHTINGSPIRIT    , SC_FIGHTINGSPIRIT    , SI_FIGHTINGSPIRIT    , SCB_WATK|SCB_ASPD );
	set_sc( RK_ABUNDANCE         , SC_ABUNDANCE         , SI_ABUNDANCE         , SCB_NONE );
	set_sc( RK_CRUSHSTRIKE		 , SC_CRUSHSTRIKE		, SI_CRUSHSTRIKE	   , SCB_NONE );
	add_sc( RK_DRAGONBREATH_WATER, SC_FROSTMISTY );
	/**
	* GC Guillotine Cross
	**/
	set_sc_with_vfx( GC_VENOMIMPRESS      , SC_VENOMIMPRESS     , SI_VENOMIMPRESS     , SCB_NONE );
	set_sc( GC_POISONINGWEAPON   , SC_POISONINGWEAPON  , SI_POISONINGWEAPON  , SCB_NONE );
	set_sc( GC_WEAPONBLOCKING    , SC_WEAPONBLOCKING   , SI_WEAPONBLOCKING   , SCB_NONE );
	set_sc( GC_CLOAKINGEXCEED    , SC_CLOAKINGEXCEED   , SI_CLOAKINGEXCEED   , SCB_SPEED );
	set_sc( GC_HALLUCINATIONWALK , SC_HALLUCINATIONWALK, SI_HALLUCINATIONWALK, SCB_FLEE );
	set_sc( GC_ROLLINGCUTTER     , SC_ROLLINGCUTTER    , SI_ROLLINGCUTTER    , SCB_NONE );
	set_sc_with_vfx( GC_DARKCROW          , SC_DARKCROW         , SI_DARKCROW         , SCB_NONE );
	/**
	* Arch Bishop
	**/
	set_sc( AB_ADORAMUS          , SC_ADORAMUS        , SI_ADORAMUS        , SCB_AGI|SCB_SPEED );
	add_sc( AB_CLEMENTIA         , SC_BLESSING );
	add_sc( AB_CANTO             , SC_INC_AGI );
	set_sc( AB_EPICLESIS         , SC_EPICLESIS       , SI_EPICLESIS       , SCB_MAXHP );
	add_sc( AB_PRAEFATIO         , SC_KYRIE );
	set_sc_with_vfx( AB_ORATIO            , SC_ORATIO          , SI_ORATIO          , SCB_NONE );
	set_sc( AB_LAUDAAGNUS        , SC_LAUDAAGNUS      , SI_LAUDAAGNUS      , SCB_VIT );
	set_sc( AB_LAUDARAMUS        , SC_LAUDARAMUS      , SI_LAUDARAMUS      , SCB_LUK );
	set_sc( AB_RENOVATIO         , SC_RENOVATIO       , SI_RENOVATIO       , SCB_REGEN );
	set_sc( AB_EXPIATIO          , SC_EXPIATIO        , SI_EXPIATIO        , SCB_ATK_ELE );
	set_sc( AB_DUPLELIGHT        , SC_DUPLELIGHT      , SI_DUPLELIGHT      , SCB_NONE );
	set_sc( AB_SECRAMENT         , SC_SECRAMENT       , SI_SECRAMENT       , SCB_NONE );
	set_sc( AB_OFFERTORIUM       , SC_OFFERTORIUM     , SI_OFFERTORIUM     , SCB_NONE );
	/**
	* Warlock
	**/
	add_sc( WL_WHITEIMPRISON     , SC_WHITEIMPRISON );
	set_sc_with_vfx( WL_FROSTMISTY        , SC_FROSTMISTY        , SI_FROSTMISTY      , SCB_ASPD|SCB_SPEED|SCB_DEF );
	set_sc( WL_MARSHOFABYSS      , SC_MARSHOFABYSS    , SI_MARSHOFABYSS    , SCB_SPEED|SCB_FLEE|SCB_AGI|SCB_DEX );
	set_sc(WL_RECOGNIZEDSPELL   , SC_RECOGNIZEDSPELL , SI_RECOGNIZEDSPELL , SCB_MATK);
	set_sc( WL_STASIS            , SC_STASIS          , SI_STASIS          , SCB_NONE );
	set_sc( WL_TELEKINESIS_INTENSE, SC_TELEKINESIS_INTENSE     , SI_TELEKINESIS_INTENSE , SCB_MATK );
	/**
	* Ranger
	**/
	set_sc( RA_FEARBREEZE        , SC_FEARBREEZE      , SI_FEARBREEZE      , SCB_NONE );
	set_sc( RA_ELECTRICSHOCKER   , SC_ELECTRICSHOCKER , SI_ELECTRICSHOCKER , SCB_NONE );
	set_sc( RA_WUGDASH           , SC_WUGDASH         , SI_WUGDASH         , SCB_SPEED );
	set_sc( RA_CAMOUFLAGE        , SC_CAMOUFLAGE      , SI_CAMOUFLAGE      , SCB_SPEED );
	add_sc( RA_MAGENTATRAP       , SC_ARMOR_PROPERTY );
	add_sc( RA_COBALTTRAP        , SC_ARMOR_PROPERTY );
	add_sc( RA_MAIZETRAP         , SC_ARMOR_PROPERTY );
	add_sc( RA_VERDURETRAP       , SC_ARMOR_PROPERTY );
	add_sc( RA_FIRINGTRAP        , SC_BURNING );
	add_sc( RA_ICEBOUNDTRAP      , SC_FROSTMISTY );
	set_sc( RA_UNLIMIT           , SC_UNLIMIT         , SI_UNLIMIT         , SCB_NONE );
	/**
	* Mechanic
	**/
	set_sc( NC_ACCELERATION      , SC_ACCELERATION    , SI_ACCELERATION    , SCB_SPEED );
	set_sc( NC_HOVERING          , SC_HOVERING        , SI_HOVERING        , SCB_SPEED );
	set_sc( NC_SHAPESHIFT        , SC_SHAPESHIFT      , SI_SHAPESHIFT      , SCB_DEF_ELE );
	set_sc( NC_INFRAREDSCAN      , SC_INFRAREDSCAN    , SI_INFRAREDSCAN    , SCB_FLEE );
	set_sc( NC_ANALYZE           , SC_ANALYZE         , SI_ANALYZE         , SCB_DEF|SCB_DEF2|SCB_MDEF|SCB_MDEF2 );
	set_sc( NC_MAGNETICFIELD     , SC_MAGNETICFIELD   , SI_MAGNETICFIELD   , SCB_NONE );
	set_sc( NC_NEUTRALBARRIER    , SC_NEUTRALBARRIER  , SI_NEUTRALBARRIER  , SCB_DEF|SCB_MDEF );
	set_sc( NC_STEALTHFIELD      , SC_STEALTHFIELD    , SI_STEALTHFIELD    , SCB_NONE );
	/**
	* Royal Guard
	**/
	set_sc( LG_REFLECTDAMAGE     , SC_LG_REFLECTDAMAGE   , SI_LG_REFLECTDAMAGE, SCB_NONE );
	set_sc( LG_FORCEOFVANGUARD   , SC_FORCEOFVANGUARD , SI_FORCEOFVANGUARD , SCB_MAXHP );
	set_sc( LG_EXEEDBREAK        , SC_EXEEDBREAK      , SI_EXEEDBREAK      , SCB_NONE );
	set_sc( LG_PRESTIGE          , SC_PRESTIGE        , SI_PRESTIGE        , SCB_DEF );
	set_sc( LG_BANDING           , SC_BANDING         , SI_BANDING         , SCB_DEF2|SCB_WATK );// Renewal: atk2 & def2
	set_sc( LG_PIETY             , SC_BENEDICTIO      , SI_BENEDICTIO      , SCB_DEF_ELE );
	set_sc( LG_EARTHDRIVE        , SC_EARTHDRIVE      , SI_EARTHDRIVE      , SCB_DEF|SCB_ASPD );
	set_sc( LG_INSPIRATION       , SC_INSPIRATION     , SI_INSPIRATION     , SCB_MAXHP|SCB_WATK|SCB_HIT|SCB_VIT|SCB_AGI|SCB_STR|SCB_DEX|SCB_INT|SCB_LUK);
	set_sc( LG_KINGS_GRACE       , SC_KINGS_GRACE     , SI_KINGS_GRACE     , SCB_NONE );
	/**
	* Shadow Chaser
	**/
	set_sc( SC_REPRODUCE         , SC__REPRODUCE      , SI_REPRODUCE       , SCB_NONE );
	set_sc( SC_AUTOSHADOWSPELL   , SC__AUTOSHADOWSPELL, SI_AUTOSHADOWSPELL , SCB_NONE );
	set_sc( SC_SHADOWFORM        , SC__SHADOWFORM     , SI_SHADOWFORM      , SCB_NONE );
	set_sc( SC_BODYPAINT         , SC__BODYPAINT      , SI_BODYPAINT       , SCB_ASPD );
	set_sc( SC_INVISIBILITY      , SC__INVISIBILITY   , SI_INVISIBILITY    , SCB_ASPD|SCB_CRI|SCB_ATK_ELE );
	set_sc( SC_DEADLYINFECT      , SC__DEADLYINFECT   , SI_DEADLYINFECT    , SCB_NONE );
	set_sc( SC_ENERVATION        , SC__ENERVATION     , SI_ENERVATION      , SCB_BATK  );
	set_sc( SC_GROOMY            , SC__GROOMY         , SI_GROOMY          , SCB_ASPD|SCB_HIT|SCB_SPEED );
	set_sc( SC_IGNORANCE         , SC__IGNORANCE      , SI_IGNORANCE       , SCB_NONE );
	set_sc( SC_LAZINESS          , SC__LAZINESS       , SI_LAZINESS        , SCB_FLEE );
	set_sc( SC_UNLUCKY           , SC__UNLUCKY        , SI_UNLUCKY         , SCB_CRI|SCB_FLEE2 );
	set_sc( SC_WEAKNESS          , SC__WEAKNESS       , SI_WEAKNESS        , SCB_FLEE2|SCB_MAXHP );
	set_sc( SC_STRIPACCESSARY    , SC__STRIPACCESSARY , SI_STRIPACCESSARY  , SCB_DEX|SCB_INT|SCB_LUK );
	set_sc_with_vfx( SC_MANHOLE           , SC__MANHOLE        , SI_MANHOLE         , SCB_NONE );
	add_sc( SC_CHAOSPANIC        , SC__CHAOS );
	add_sc( SC_MAELSTROM         , SC__MAELSTROM );
	add_sc( SC_BLOODYLUST        , SC_BERSERK );

	/**
	* Sura
	**/
	add_sc( SR_DRAGONCOMBO           , SC_STUN            );
	add_sc( SR_EARTHSHAKER           , SC_STUN            );
	set_sc( SR_CRESCENTELBOW         , SC_CRESCENTELBOW      , SI_CRESCENTELBOW         , SCB_NONE );
	set_sc_with_vfx( SR_CURSEDCIRCLE          , SC_CURSEDCIRCLE_TARGET, SI_CURSEDCIRCLE_TARGET   , SCB_NONE );
	set_sc( SR_LIGHTNINGWALK         , SC_LIGHTNINGWALK      , SI_LIGHTNINGWALK         , SCB_NONE );
	set_sc( SR_RAISINGDRAGON         , SC_RAISINGDRAGON      , SI_RAISINGDRAGON         , SCB_REGEN|SCB_MAXHP|SCB_MAXSP );
	set_sc( SR_GENTLETOUCH_ENERGYGAIN, SC_GENTLETOUCH_ENERGYGAIN      , SI_GENTLETOUCH_ENERGYGAIN, SCB_NONE );
	set_sc( SR_GENTLETOUCH_CHANGE    , SC_GENTLETOUCH_CHANGE          , SI_GENTLETOUCH_CHANGE    , SCB_ASPD|SCB_MDEF|SCB_MAXHP );
	set_sc( SR_GENTLETOUCH_REVITALIZE, SC_GENTLETOUCH_REVITALIZE      , SI_GENTLETOUCH_REVITALIZE, SCB_MAXHP|SCB_REGEN );
	/**
	* Wanderer / Minstrel
	**/
	set_sc( WA_SWING_DANCE            , SC_SWING                  , SI_SWINGDANCE           , SCB_SPEED|SCB_ASPD );
	set_sc( WA_SYMPHONY_OF_LOVER      , SC_SYMPHONY_LOVE          , SI_SYMPHONYOFLOVERS     , SCB_MDEF );
	set_sc( WA_MOONLIT_SERENADE       , SC_MOONLIT_SERENADE       , SI_MOONLITSERENADE      , SCB_MATK );
	set_sc( MI_RUSH_WINDMILL          , SC_RUSH_WINDMILL          , SI_RUSHWINDMILL         , SCB_WATK  );
	set_sc( MI_ECHOSONG               , SC_ECHOSONG               , SI_ECHOSONG             , SCB_DEF2  );
	set_sc( MI_HARMONIZE              , SC_HARMONIZE              , SI_HARMONIZE            , SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK );
	set_sc( WM_POEMOFNETHERWORLD      , SC_STOP                   , SI_NETHERWORLD          , SCB_NONE );
	set_sc_with_vfx( WM_VOICEOFSIREN        , SC_SIREN            , SI_SIREN                , SCB_NONE );
	set_sc_with_vfx( WM_LULLABY_DEEPSLEEP   , SC_DEEP_SLEEP       , SI_DEEPSLEEP            , SCB_NONE );
	set_sc( WM_SIRCLEOFNATURE         , SC_SIRCLEOFNATURE         , SI_SIRCLEOFNATURE       , SCB_NONE );
	set_sc( WM_GLOOMYDAY              , SC_GLOOMYDAY              , SI_GLOOMYDAY            , SCB_FLEE|SCB_ASPD );
	set_sc( WM_SONG_OF_MANA           , SC_SONG_OF_MANA           , SI_SONG_OF_MANA         , SCB_NONE );
	set_sc( WM_DANCE_WITH_WUG         , SC_DANCE_WITH_WUG         , SI_DANCEWITHWUG         , SCB_ASPD );
	set_sc( WM_SATURDAY_NIGHT_FEVER   , SC_SATURDAY_NIGHT_FEVER   , SI_SATURDAYNIGHTFEVER   , SCB_BATK|SCB_DEF|SCB_FLEE|SCB_REGEN );
	set_sc( WM_LERADS_DEW             , SC_LERADS_DEW             , SI_LERADSDEW            , SCB_MAXHP );
	set_sc( WM_MELODYOFSINK           , SC_MELODYOFSINK           , SI_MELODYOFSINK         , SCB_INT );
	set_sc( WM_BEYOND_OF_WARCRY       , SC_BEYOND_OF_WARCRY       , SI_WARCRYOFBEYOND       , SCB_STR|SCB_CRI|SCB_MAXHP );
	set_sc( WM_UNLIMITED_HUMMING_VOICE, SC_UNLIMITED_HUMMING_VOICE, SI_UNLIMITEDHUMMINGVOICE, SCB_NONE );
	set_sc( WM_FRIGG_SONG             , SC_FRIGG_SONG             , SI_FRIGG_SONG           , SCB_MAXHP );

	/**
	* Sorcerer
	**/
	set_sc( SO_FIREWALK          , SC_PROPERTYWALK    , SI_PROPERTYWALK    , SCB_NONE );
	set_sc( SO_ELECTRICWALK      , SC_PROPERTYWALK    , SI_PROPERTYWALK    , SCB_NONE );
	set_sc( SO_SPELLFIST         , SC_SPELLFIST       , SI_SPELLFIST       , SCB_NONE );
	set_sc_with_vfx( SO_DIAMONDDUST       , SC_COLD      , SI_COLD   , SCB_NONE ); // it does show the snow icon on mobs but doesn't affect it.
	set_sc( SO_CLOUD_KILL        , SC_POISON          , SI_CLOUDKILL       , SCB_NONE );
	set_sc( SO_STRIKING          , SC_STRIKING        , SI_STRIKING        , SCB_WATK|SCB_CRI );
	set_sc( SO_WARMER            , SC_WARMER          , SI_WARMER          , SCB_NONE );
	set_sc( SO_VACUUM_EXTREME    , SC_VACUUM_EXTREME  , SI_VACUUM_EXTREME  , SCB_NONE );
	set_sc( SO_ARRULLO           , SC_DEEP_SLEEP       , SI_DEEPSLEEP       , SCB_NONE );
	set_sc( SO_FIRE_INSIGNIA     , SC_FIRE_INSIGNIA   , SI_FIRE_INSIGNIA   , SCB_MATK | SCB_BATK | SCB_WATK | SCB_ATK_ELE | SCB_REGEN );
	set_sc( SO_WATER_INSIGNIA    , SC_WATER_INSIGNIA  , SI_WATER_INSIGNIA  , SCB_WATK | SCB_ATK_ELE | SCB_REGEN );
	set_sc( SO_WIND_INSIGNIA     , SC_WIND_INSIGNIA   , SI_WIND_INSIGNIA   , SCB_WATK | SCB_ATK_ELE | SCB_REGEN );
	set_sc( SO_EARTH_INSIGNIA    , SC_EARTH_INSIGNIA  , SI_EARTH_INSIGNIA  , SCB_MDEF|SCB_DEF|SCB_MAXHP|SCB_MAXSP|SCB_WATK | SCB_ATK_ELE | SCB_REGEN );
	add_sc( SO_ELEMENTAL_SHIELD  , SC_SAFETYWALL );
	/**
	* Genetic
	**/
	set_sc( GN_CARTBOOST                  , SC_GN_CARTBOOST, SI_CARTSBOOST                 , SCB_SPEED );
	set_sc( GN_THORNS_TRAP                , SC_THORNS_TRAP  , SI_THORNTRAP                  , SCB_NONE );
	set_sc_with_vfx( GN_BLOOD_SUCKER      , SC_BLOOD_SUCKER , SI_BLOODSUCKER                , SCB_NONE );
	set_sc( GN_WALLOFTHORN                , SC_STOP        , SI_BLANK                      , SCB_NONE );
	set_sc( GN_FIRE_EXPANSION_SMOKE_POWDER, SC_FIRE_EXPANSION_SMOKE_POWDER , SI_FIRE_EXPANSION_SMOKE_POWDER, SCB_NONE );
	set_sc( GN_FIRE_EXPANSION_TEAR_GAS    , SC_FIRE_EXPANSION_TEAR_GAS     , SI_FIRE_EXPANSION_TEAR_GAS    , SCB_NONE );
	set_sc( GN_MANDRAGORA                 , SC_MANDRAGORA  , SI_MANDRAGORA                 , SCB_INT );

	// Elemental Spirit summoner's 'side' status changes.
	set_sc( EL_CIRCLE_OF_FIRE  , SC_CIRCLE_OF_FIRE_OPTION, SI_CIRCLE_OF_FIRE_OPTION, SCB_NONE );
	set_sc( EL_FIRE_CLOAK      , SC_FIRE_CLOAK_OPTION    , SI_FIRE_CLOAK_OPTION    , SCB_ALL );
	set_sc( EL_WATER_SCREEN    , SC_WATER_SCREEN_OPTION  , SI_WATER_SCREEN_OPTION  , SCB_NONE );
	set_sc( EL_WATER_DROP      , SC_WATER_DROP_OPTION    , SI_WATER_DROP_OPTION    , SCB_ALL );
	set_sc( EL_WATER_BARRIER   , SC_WATER_BARRIER        , SI_WATER_BARRIER        , SCB_MDEF|SCB_WATK|SCB_MATK|SCB_FLEE );
	set_sc( EL_WIND_STEP       , SC_WIND_STEP_OPTION     , SI_WIND_STEP_OPTION     , SCB_SPEED|SCB_FLEE );
	set_sc( EL_WIND_CURTAIN    , SC_WIND_CURTAIN_OPTION  , SI_WIND_CURTAIN_OPTION  , SCB_ALL );
	set_sc( EL_ZEPHYR          , SC_ZEPHYR               , SI_ZEPHYR               , SCB_FLEE );
	set_sc( EL_SOLID_SKIN      , SC_SOLID_SKIN_OPTION    , SI_SOLID_SKIN_OPTION    , SCB_DEF|SCB_MAXHP );
	set_sc( EL_STONE_SHIELD    , SC_STONE_SHIELD_OPTION  , SI_STONE_SHIELD_OPTION  , SCB_ALL );
	set_sc( EL_POWER_OF_GAIA   , SC_POWER_OF_GAIA        , SI_POWER_OF_GAIA        , SCB_MAXHP|SCB_DEF|SCB_SPEED );
	set_sc( EL_PYROTECHNIC     , SC_PYROTECHNIC_OPTION   , SI_PYROTECHNIC_OPTION   , SCB_WATK );
	set_sc( EL_HEATER          , SC_HEATER_OPTION        , SI_HEATER_OPTION        , SCB_WATK );
	set_sc( EL_TROPIC          , SC_TROPIC_OPTION        , SI_TROPIC_OPTION        , SCB_WATK );
	set_sc( EL_AQUAPLAY        , SC_AQUAPLAY_OPTION      , SI_AQUAPLAY_OPTION      , SCB_MATK );
	set_sc( EL_COOLER          , SC_COOLER_OPTION        , SI_COOLER_OPTION        , SCB_MATK );
	set_sc( EL_CHILLY_AIR      , SC_CHILLY_AIR_OPTION    , SI_CHILLY_AIR_OPTION    , SCB_MATK );
	set_sc( EL_GUST            , SC_GUST_OPTION          , SI_GUST_OPTION          , SCB_ASPD );
	set_sc( EL_BLAST           , SC_BLAST_OPTION         , SI_BLAST_OPTION         , SCB_ASPD );
	set_sc( EL_WILD_STORM      , SC_WILD_STORM_OPTION    , SI_WILD_STORM_OPTION    , SCB_ASPD );
	set_sc( EL_PETROLOGY       , SC_PETROLOGY_OPTION     , SI_PETROLOGY_OPTION     , SCB_MAXHP );
	set_sc( EL_CURSED_SOIL     , SC_CURSED_SOIL_OPTION   , SI_CURSED_SOIL_OPTION   , SCB_MAXHP );
	set_sc( EL_UPHEAVAL        , SC_UPHEAVAL_OPTION      , SI_UPHEAVAL_OPTION      , SCB_MAXHP );
	set_sc( EL_TIDAL_WEAPON    , SC_TIDAL_WEAPON_OPTION  , SI_TIDAL_WEAPON_OPTION  , SCB_ALL );
	set_sc( EL_ROCK_CRUSHER    , SC_ROCK_CRUSHER         , SI_ROCK_CRUSHER         , SCB_DEF );
	set_sc( EL_ROCK_CRUSHER_ATK, SC_ROCK_CRUSHER_ATK     , SI_ROCK_CRUSHER_ATK     , SCB_SPEED );

	add_sc( KO_YAMIKUMO			, SC_HIDING		  );
	set_sc_with_vfx( KO_JYUMONJIKIRI		, SC_KO_JYUMONJIKIRI		 , SI_KO_JYUMONJIKIRI	   , SCB_NONE );
	add_sc( KO_MAKIBISHI	    , SC_STUN		  );
	set_sc( KO_MEIKYOUSISUI		, SC_MEIKYOUSISUI		 , SI_MEIKYOUSISUI		   , SCB_NONE );
	set_sc( KO_KYOUGAKU			, SC_KYOUGAKU			 , SI_KYOUGAKU			   , SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK );
	add_sc( KO_JYUSATSU			, SC_CURSE		  );
	set_sc( KO_ZENKAI			, SC_ZENKAI				 , SI_ZENKAI			   , SCB_NONE );
	set_sc( KO_IZAYOI			, SC_IZAYOI				 , SI_IZAYOI			   , SCB_MATK );
	set_sc( KG_KYOMU			, SC_KYOMU				 , SI_KYOMU				   , SCB_NONE );
	set_sc( KG_KAGEMUSYA		, SC_KAGEMUSYA			 , SI_KAGEMUSYA			   , SCB_NONE );
	set_sc( KG_KAGEHUMI			, SC_KG_KAGEHUMI		 , SI_KG_KAGEHUMI		   , SCB_NONE );
	set_sc( OB_ZANGETSU			, SC_ZANGETSU			 , SI_ZANGETSU			   , SCB_MATK|SCB_BATK );
	set_sc_with_vfx( OB_AKAITSUKI		, SC_AKAITSUKI			 , SI_AKAITSUKI			   , SCB_NONE );
	set_sc( OB_OBOROGENSOU		, SC_GENSOU				 , SI_GENSOU			   , SCB_NONE );

	set_sc( ALL_FULL_THROTTLE   , SC_FULL_THROTTLE       , SI_FULL_THROTTLE        , SCB_SPEED|SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK );

	add_sc( ALL_REVERSEORCISH   , SC_ORCISH          );
	set_sc( ALL_ANGEL_PROTECT	, SC_ANGEL_PROTECT		 , SI_ANGEL_PROTECT		   , SCB_REGEN );

	add_sc( NPC_WIDEHEALTHFEAR   , SC_FEAR           );
	add_sc( NPC_WIDEBODYBURNNING , SC_BURNING        );
	add_sc( NPC_WIDEFROSTMISTY   , SC_FROSTMISTY     );
	add_sc( NPC_WIDECOLD         , SC_COLD           );
	add_sc( NPC_WIDE_DEEP_SLEEP  , SC_DEEP_SLEEP     );
	add_sc( NPC_WIDESIREN        , SC_SIREN          );
	
	set_sc_with_vfx( GN_ILLUSIONDOPING   , SC_ILLUSIONDOPING    , SI_ILLUSIONDOPING     , SCB_HIT );

	// Storing the target job rather than simply SC_SOULLINK simplifies code later on.
	status->Skill2SCTable[SL_ALCHEMIST]   = (sc_type)MAPID_ALCHEMIST,
		status->Skill2SCTable[SL_MONK]        = (sc_type)MAPID_MONK,
		status->Skill2SCTable[SL_STAR]        = (sc_type)MAPID_STAR_GLADIATOR,
		status->Skill2SCTable[SL_SAGE]        = (sc_type)MAPID_SAGE,
		status->Skill2SCTable[SL_CRUSADER]    = (sc_type)MAPID_CRUSADER,
		status->Skill2SCTable[SL_SUPERNOVICE] = (sc_type)MAPID_SUPER_NOVICE,
		status->Skill2SCTable[SL_KNIGHT]      = (sc_type)MAPID_KNIGHT,
		status->Skill2SCTable[SL_WIZARD]      = (sc_type)MAPID_WIZARD,
		status->Skill2SCTable[SL_PRIEST]      = (sc_type)MAPID_PRIEST,
		status->Skill2SCTable[SL_BARDDANCER]  = (sc_type)MAPID_BARDDANCER,
		status->Skill2SCTable[SL_ROGUE]       = (sc_type)MAPID_ROGUE,
		status->Skill2SCTable[SL_ASSASIN]     = (sc_type)MAPID_ASSASSIN,
		status->Skill2SCTable[SL_BLACKSMITH]  = (sc_type)MAPID_BLACKSMITH,
		status->Skill2SCTable[SL_HUNTER]      = (sc_type)MAPID_HUNTER,
		status->Skill2SCTable[SL_SOULLINKER]  = (sc_type)MAPID_SOUL_LINKER,

	//Status that don't have a skill associated.
	status->IconChangeTable[SC_WEIGHTOVER50] = SI_WEIGHTOVER50;
	status->IconChangeTable[SC_WEIGHTOVER90] = SI_WEIGHTOVER90;
	status->IconChangeTable[SC_ATTHASTE_POTION1] = SI_ATTHASTE_POTION1;
	status->IconChangeTable[SC_ATTHASTE_POTION2] = SI_ATTHASTE_POTION2;
	status->IconChangeTable[SC_ATTHASTE_POTION3] = SI_ATTHASTE_POTION3;
	status->IconChangeTable[SC_ATTHASTE_INFINITY] = SI_ATTHASTE_INFINITY;
	status->IconChangeTable[SC_MOVHASTE_HORSE] = SI_MOVHASTE_HORSE;
	status->IconChangeTable[SC_MOVHASTE_INFINITY] = SI_MOVHASTE_INFINITY;
	status->IconChangeTable[SC_CHASEWALK2] = SI_INCSTR;
	status->IconChangeTable[SC_MIRACLE] = SI_SOULLINK;
	status->IconChangeTable[SC_CLAIRVOYANCE] = SI_CLAIRVOYANCE;
	status->IconChangeTable[SC_FOOD_STR] = SI_FOOD_STR;
	status->IconChangeTable[SC_FOOD_AGI] = SI_FOOD_AGI;
	status->IconChangeTable[SC_FOOD_VIT] = SI_FOOD_VIT;
	status->IconChangeTable[SC_FOOD_INT] = SI_FOOD_INT;
	status->IconChangeTable[SC_FOOD_DEX] = SI_FOOD_DEX;
	status->IconChangeTable[SC_FOOD_LUK] = SI_FOOD_LUK;
	status->IconChangeTable[SC_FOOD_BASICAVOIDANCE]= SI_FOOD_BASICAVOIDANCE;
	status->IconChangeTable[SC_FOOD_BASICHIT] = SI_FOOD_BASICHIT;
	status->IconChangeTable[SC_MANU_ATK] = SI_MANU_ATK;
	status->IconChangeTable[SC_MANU_DEF] = SI_MANU_DEF;
	status->IconChangeTable[SC_SPL_ATK] = SI_SPL_ATK;
	status->IconChangeTable[SC_SPL_DEF] = SI_SPL_DEF;
	status->IconChangeTable[SC_MANU_MATK] = SI_MANU_MATK;
	status->IconChangeTable[SC_SPL_MATK] = SI_SPL_MATK;
	status->IconChangeTable[SC_PLUSATTACKPOWER] = SI_PLUSATTACKPOWER;
	status->IconChangeTable[SC_PLUSMAGICPOWER] = SI_PLUSMAGICPOWER;
	//Cash Items
	status->IconChangeTable[SC_FOOD_STR_CASH] = SI_FOOD_STR_CASH;
	status->IconChangeTable[SC_FOOD_AGI_CASH] = SI_FOOD_AGI_CASH;
	status->IconChangeTable[SC_FOOD_VIT_CASH] = SI_FOOD_VIT_CASH;
	status->IconChangeTable[SC_FOOD_DEX_CASH] = SI_FOOD_DEX_CASH;
	status->IconChangeTable[SC_FOOD_INT_CASH] = SI_FOOD_INT_CASH;
	status->IconChangeTable[SC_FOOD_LUK_CASH] = SI_FOOD_LUK_CASH;
	status->IconChangeTable[SC_CASH_PLUSEXP] = SI_CASH_PLUSEXP;
	status->IconChangeTable[SC_CASH_RECEIVEITEM] = SI_CASH_RECEIVEITEM;
	status->IconChangeTable[SC_CASH_PLUSONLYJOBEXP] = SI_CASH_PLUSONLYJOBEXP;
	status->IconChangeTable[SC_CASH_DEATHPENALTY] = SI_CASH_DEATHPENALTY;
	status->IconChangeTable[SC_CASH_BOSS_ALARM] = SI_CASH_BOSS_ALARM;
	status->IconChangeTable[SC_PROTECT_DEF] = SI_PROTECT_DEF;
	status->IconChangeTable[SC_PROTECT_MDEF] = SI_PROTECT_MDEF;
	status->IconChangeTable[SC_CRITICALPERCENT] = SI_CRITICALPERCENT;
	status->IconChangeTable[SC_PLUSAVOIDVALUE] = SI_PLUSAVOIDVALUE;
	status->IconChangeTable[SC_HEALPLUS] = SI_HEALPLUS;
	status->IconChangeTable[SC_S_LIFEPOTION] = SI_S_LIFEPOTION;
	status->IconChangeTable[SC_L_LIFEPOTION] = SI_L_LIFEPOTION;
	status->IconChangeTable[SC_ATKER_BLOOD] = SI_ATKER_BLOOD;
	status->IconChangeTable[SC_TARGET_BLOOD] = SI_TARGET_BLOOD;
	// Mercenary Bonus Effects
	status->IconChangeTable[SC_MER_FLEE] = SI_MER_FLEE;
	status->IconChangeTable[SC_MER_ATK] = SI_MER_ATK;
	status->IconChangeTable[SC_MER_HP] = SI_MER_HP;
	status->IconChangeTable[SC_MER_SP] = SI_MER_SP;
	status->IconChangeTable[SC_MER_HIT] = SI_MER_HIT;
	// Warlock Spheres
	status->IconChangeTable[SC_SUMMON1] = SI_SPHERE_1;
	status->IconChangeTable[SC_SUMMON2] = SI_SPHERE_2;
	status->IconChangeTable[SC_SUMMON3] = SI_SPHERE_3;
	status->IconChangeTable[SC_SUMMON4] = SI_SPHERE_4;
	status->IconChangeTable[SC_SUMMON5] = SI_SPHERE_5;
	// Warlock Preserved spells
	status->IconChangeTable[SC_SPELLBOOK1] = SI_SPELLBOOK1;
	status->IconChangeTable[SC_SPELLBOOK2] = SI_SPELLBOOK2;
	status->IconChangeTable[SC_SPELLBOOK3] = SI_SPELLBOOK3;
	status->IconChangeTable[SC_SPELLBOOK4] = SI_SPELLBOOK4;
	status->IconChangeTable[SC_SPELLBOOK5] = SI_SPELLBOOK5;
	status->IconChangeTable[SC_SPELLBOOK6] = SI_SPELLBOOK6;
	status->IconChangeTable[SC_SPELLBOOK7] = SI_SPELLBOOK7;

	status->IconChangeTable[SC_NEUTRALBARRIER_MASTER] = SI_NEUTRALBARRIER_MASTER;
	status->IconChangeTable[SC_STEALTHFIELD_MASTER] = SI_STEALTHFIELD_MASTER;
	status->IconChangeTable[SC_OVERHEAT] = SI_OVERHEAT;
	status->IconChangeTable[SC_OVERHEAT_LIMITPOINT] = SI_OVERHEAT_LIMITPOINT;

	status->IconChangeTable[SC_HALLUCINATIONWALK_POSTDELAY] = SI_HALLUCINATIONWALK_POSTDELAY;
	status->IconChangeTable[SC_TOXIN] = SI_TOXIN;
	status->IconChangeTable[SC_PARALYSE] = SI_PARALYSE;
	status->IconChangeTable[SC_VENOMBLEED] = SI_VENOMBLEED;
	status->IconChangeTable[SC_MAGICMUSHROOM] = SI_MAGICMUSHROOM;
	status->IconChangeTable[SC_DEATHHURT] = SI_DEATHHURT;
	status->IconChangeTable[SC_PYREXIA] = SI_PYREXIA;
	status->IconChangeTable[SC_OBLIVIONCURSE] = SI_OBLIVIONCURSE;
	status->IconChangeTable[SC_LEECHESEND] = SI_LEECHESEND;

	status->IconChangeTable[SC_SHIELDSPELL_DEF] = SI_SHIELDSPELL_DEF;
	status->IconChangeTable[SC_SHIELDSPELL_MDEF] = SI_SHIELDSPELL_MDEF;
	status->IconChangeTable[SC_SHIELDSPELL_REF] = SI_SHIELDSPELL_REF;
	status->IconChangeTable[SC_BANDING_DEFENCE] = SI_BANDING_DEFENCE;

	status->IconChangeTable[SC_CURSEDCIRCLE_ATKER] = SI_CURSEDCIRCLE_ATKER;

	status->IconChangeTable[SC_STOMACHACHE] = SI_STOMACHACHE;
	status->IconChangeTable[SC_MYSTERIOUS_POWDER] = SI_MYSTERIOUS_POWDER;
	status->IconChangeTable[SC_MELON_BOMB] = SI_MELON_BOMB;
	status->IconChangeTable[SC_BANANA_BOMB] = SI_BANANA_BOMB;
	status->IconChangeTable[SC_BANANA_BOMB_SITDOWN_POSTDELAY] = SI_BANANA_BOMB_SITDOWN_POSTDELAY;

	//Genetics New Food Items Status Icons
	status->IconChangeTable[SC_SAVAGE_STEAK] = SI_SAVAGE_STEAK;
	status->IconChangeTable[SC_COCKTAIL_WARG_BLOOD] = SI_COCKTAIL_WARG_BLOOD;
	status->IconChangeTable[SC_MINOR_BBQ] = SI_MINOR_BBQ;
	status->IconChangeTable[SC_SIROMA_ICE_TEA] = SI_SIROMA_ICE_TEA;
	status->IconChangeTable[SC_DROCERA_HERB_STEAMED] = SI_DROCERA_HERB_STEAMED;
	status->IconChangeTable[SC_PUTTI_TAILS_NOODLES] = SI_PUTTI_TAILS_NOODLES;

	status->IconChangeTable[SC_BOOST500] |= SI_BOOST500;
	status->IconChangeTable[SC_FULL_SWING_K] |= SI_FULL_SWING_K;
	status->IconChangeTable[SC_MANA_PLUS] |= SI_MANA_PLUS;
	status->IconChangeTable[SC_MUSTLE_M] |= SI_MUSTLE_M;
	status->IconChangeTable[SC_LIFE_FORCE_F] |= SI_LIFE_FORCE_F;
	status->IconChangeTable[SC_EXTRACT_WHITE_POTION_Z] |= SI_EXTRACT_WHITE_POTION_Z;
	status->IconChangeTable[SC_VITATA_500] |= SI_VITATA_500;
	status->IconChangeTable[SC_EXTRACT_SALAMINE_JUICE] |= SI_EXTRACT_SALAMINE_JUICE;

	// Elemental Spirit's 'side' status change icons.
	status->IconChangeTable[SC_CIRCLE_OF_FIRE] = SI_CIRCLE_OF_FIRE;
	status->IconChangeTable[SC_FIRE_CLOAK] = SI_FIRE_CLOAK;
	status->IconChangeTable[SC_WATER_SCREEN] = SI_WATER_SCREEN;
	status->IconChangeTable[SC_WATER_DROP] = SI_WATER_DROP;
	status->IconChangeTable[SC_WIND_STEP] = SI_WIND_STEP;
	status->IconChangeTable[SC_WIND_CURTAIN] = SI_WIND_CURTAIN;
	status->IconChangeTable[SC_SOLID_SKIN] = SI_SOLID_SKIN;
	status->IconChangeTable[SC_STONE_SHIELD] = SI_STONE_SHIELD;
	status->IconChangeTable[SC_PYROTECHNIC] = SI_PYROTECHNIC;
	status->IconChangeTable[SC_HEATER] = SI_HEATER;
	status->IconChangeTable[SC_TROPIC] = SI_TROPIC;
	status->IconChangeTable[SC_AQUAPLAY] = SI_AQUAPLAY;
	status->IconChangeTable[SC_COOLER] = SI_COOLER;
	status->IconChangeTable[SC_CHILLY_AIR] = SI_CHILLY_AIR;
	status->IconChangeTable[SC_GUST] = SI_GUST;
	status->IconChangeTable[SC_BLAST] = SI_BLAST;
	status->IconChangeTable[SC_WILD_STORM] = SI_WILD_STORM;
	status->IconChangeTable[SC_PETROLOGY] = SI_PETROLOGY;
	status->IconChangeTable[SC_CURSED_SOIL] = SI_CURSED_SOIL;
	status->IconChangeTable[SC_UPHEAVAL] = SI_UPHEAVAL;
	status->IconChangeTable[SC_PUSH_CART] = SI_ON_PUSH_CART;
	status->IconChangeTable[SC_REBOUND] = SI_REBOUND;
	status->IconChangeTable[SC_ALL_RIDING] = SI_ALL_RIDING;
	status->IconChangeTable[SC_MONSTER_TRANSFORM] = SI_MONSTER_TRANSFORM;
	status->IconChangeTable[SC_MOONSTAR] = SI_MOONSTAR;
	status->IconChangeTable[SC_SUPER_STAR] = SI_SUPER_STAR;
	status->IconChangeTable[SC_STRANGELIGHTS] = SI_STRANGELIGHTS;
	status->IconChangeTable[SC_DECORATION_OF_MUSIC] = SI_DECORATION_OF_MUSIC;

	//Other SC which are not necessarily associated to skills.
	status->ChangeFlagTable[SC_ATTHASTE_POTION1] = SCB_ASPD;
	status->ChangeFlagTable[SC_ATTHASTE_POTION2] = SCB_ASPD;
	status->ChangeFlagTable[SC_ATTHASTE_POTION3] = SCB_ASPD;
	status->ChangeFlagTable[SC_ATTHASTE_INFINITY] = SCB_ASPD;
	status->ChangeFlagTable[SC_MOVHASTE_HORSE] = SCB_SPEED;
	status->ChangeFlagTable[SC_MOVHASTE_INFINITY] = SCB_SPEED;
	status->ChangeFlagTable[SC_PLUSATTACKPOWER] = SCB_BATK;
	status->ChangeFlagTable[SC_PLUSMAGICPOWER] = SCB_MATK;
	status->ChangeFlagTable[SC_INCALLSTATUS] |= SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK;
	status->ChangeFlagTable[SC_CHASEWALK2] |= SCB_STR;
	status->ChangeFlagTable[SC_INCAGI] |= SCB_AGI;
	status->ChangeFlagTable[SC_INCVIT] |= SCB_VIT;
	status->ChangeFlagTable[SC_INCINT] |= SCB_INT;
	status->ChangeFlagTable[SC_INCDEX] |= SCB_DEX;
	status->ChangeFlagTable[SC_INCLUK] |= SCB_LUK;
	status->ChangeFlagTable[SC_INCHIT] |= SCB_HIT;
	status->ChangeFlagTable[SC_INCHITRATE] |= SCB_HIT;
	status->ChangeFlagTable[SC_INCFLEE] |= SCB_FLEE;
	status->ChangeFlagTable[SC_INCFLEERATE] |= SCB_FLEE;
	status->ChangeFlagTable[SC_CRITICALPERCENT] |= SCB_CRI;
	status->ChangeFlagTable[SC_INCASPDRATE] |= SCB_ASPD;
	status->ChangeFlagTable[SC_PLUSAVOIDVALUE] |= SCB_FLEE2;
	status->ChangeFlagTable[SC_INCMHPRATE] |= SCB_MAXHP;
	status->ChangeFlagTable[SC_INCMSPRATE] |= SCB_MAXSP;
	status->ChangeFlagTable[SC_INCMHP] |= SCB_MAXHP;
	status->ChangeFlagTable[SC_INCMSP] |= SCB_MAXSP;
	status->ChangeFlagTable[SC_INCATKRATE] |= SCB_BATK|SCB_WATK;
	status->ChangeFlagTable[SC_INCMATKRATE] |= SCB_MATK;
	status->ChangeFlagTable[SC_INCDEFRATE] |= SCB_DEF;
	status->ChangeFlagTable[SC_FOOD_STR] |= SCB_STR;
	status->ChangeFlagTable[SC_FOOD_AGI] |= SCB_AGI;
	status->ChangeFlagTable[SC_FOOD_VIT] |= SCB_VIT;
	status->ChangeFlagTable[SC_FOOD_INT] |= SCB_INT;
	status->ChangeFlagTable[SC_FOOD_DEX] |= SCB_DEX;
	status->ChangeFlagTable[SC_FOOD_LUK] |= SCB_LUK;
	status->ChangeFlagTable[SC_FOOD_BASICHIT] |= SCB_HIT;
	status->ChangeFlagTable[SC_FOOD_BASICAVOIDANCE] |= SCB_FLEE;
	status->ChangeFlagTable[SC_BATKFOOD] |= SCB_BATK;
	status->ChangeFlagTable[SC_WATKFOOD] |= SCB_WATK;
	status->ChangeFlagTable[SC_MATKFOOD] |= SCB_MATK;
	status->ChangeFlagTable[SC_ARMORPROPERTY] |= SCB_ALL;
	status->ChangeFlagTable[SC_ARMOR_RESIST] |= SCB_ALL;
	status->ChangeFlagTable[SC_ATKER_BLOOD] |= SCB_ALL;
	status->ChangeFlagTable[SC_WALKSPEED] |= SCB_SPEED;
	status->ChangeFlagTable[SC_ITEMSCRIPT] |= SCB_ALL;
	// Cash Items
	status->ChangeFlagTable[SC_FOOD_STR_CASH] = SCB_STR;
	status->ChangeFlagTable[SC_FOOD_AGI_CASH] = SCB_AGI;
	status->ChangeFlagTable[SC_FOOD_VIT_CASH] = SCB_VIT;
	status->ChangeFlagTable[SC_FOOD_DEX_CASH] = SCB_DEX;
	status->ChangeFlagTable[SC_FOOD_INT_CASH] = SCB_INT;
	status->ChangeFlagTable[SC_FOOD_LUK_CASH] = SCB_LUK;
	// Mercenary Bonus Effects
	status->ChangeFlagTable[SC_MER_FLEE] |= SCB_FLEE;
	status->ChangeFlagTable[SC_MER_ATK] |= SCB_WATK;
	status->ChangeFlagTable[SC_MER_HP] |= SCB_MAXHP;
	status->ChangeFlagTable[SC_MER_SP] |= SCB_MAXSP;
	status->ChangeFlagTable[SC_MER_HIT] |= SCB_HIT;
	// Guillotine Cross Poison Effects
	status->ChangeFlagTable[SC_PARALYSE] |= SCB_FLEE|SCB_SPEED|SCB_ASPD;
	status->ChangeFlagTable[SC_VENOMBLEED] |= SCB_MAXHP;
	status->ChangeFlagTable[SC_MAGICMUSHROOM] |= SCB_REGEN;
	status->ChangeFlagTable[SC_DEATHHURT] |= SCB_REGEN;
	status->ChangeFlagTable[SC_PYREXIA] |= SCB_HIT|SCB_FLEE;
	status->ChangeFlagTable[SC_OBLIVIONCURSE] |= SCB_REGEN;
	// RG status
	status->ChangeFlagTable[SC_SHIELDSPELL_DEF] |= SCB_WATK;
	status->ChangeFlagTable[SC_SHIELDSPELL_REF] |= SCB_DEF;
	// Meca status
	status->ChangeFlagTable[SC_STEALTHFIELD_MASTER] |= SCB_SPEED;

	status->ChangeFlagTable[SC_SAVAGE_STEAK] |= SCB_STR;
	status->ChangeFlagTable[SC_COCKTAIL_WARG_BLOOD] |= SCB_INT;
	status->ChangeFlagTable[SC_MINOR_BBQ] |= SCB_VIT;
	status->ChangeFlagTable[SC_SIROMA_ICE_TEA] |= SCB_DEX;
	status->ChangeFlagTable[SC_DROCERA_HERB_STEAMED] |= SCB_AGI;
	status->ChangeFlagTable[SC_PUTTI_TAILS_NOODLES] |= SCB_LUK;
	status->ChangeFlagTable[SC_BOOST500] |= SCB_ASPD;
	status->ChangeFlagTable[SC_FULL_SWING_K] |= SCB_BATK;
	status->ChangeFlagTable[SC_MANA_PLUS] |= SCB_MATK;
	status->ChangeFlagTable[SC_MUSTLE_M] |= SCB_MAXHP;
	status->ChangeFlagTable[SC_LIFE_FORCE_F] |= SCB_MAXSP;
	status->ChangeFlagTable[SC_EXTRACT_WHITE_POTION_Z] |= SCB_REGEN;
	status->ChangeFlagTable[SC_VITATA_500] |= SCB_REGEN;
	status->ChangeFlagTable[SC_EXTRACT_SALAMINE_JUICE] |= SCB_ASPD;
	status->ChangeFlagTable[SC_REBOUND] |= SCB_SPEED|SCB_REGEN;

	status->ChangeFlagTable[SC_ALL_RIDING] = SCB_SPEED;
	status->ChangeFlagTable[SC_WEDDING] = SCB_SPEED;

	status->ChangeFlagTable[SC_MTF_ASPD] = SCB_ASPD|SCB_HIT;
	status->ChangeFlagTable[SC_MTF_MATK] = SCB_MATK;
	status->ChangeFlagTable[SC_MTF_MLEATKED] |= SCB_ALL;

	status->ChangeFlagTable[SC_MOONSTAR] |= SCB_NONE;
	status->ChangeFlagTable[SC_SUPER_STAR] |= SCB_NONE;
	status->ChangeFlagTable[SC_STRANGELIGHTS] |= SCB_NONE;
	status->ChangeFlagTable[SC_DECORATION_OF_MUSIC] |= SCB_NONE;

	/* status->DisplayType Table [Ind/Hercules] */
	status->DisplayType[SC_ALL_RIDING]		= true;
	status->DisplayType[SC_PUSH_CART]			= true;
	status->DisplayType[SC_SUMMON1]			= true;
	status->DisplayType[SC_SUMMON2]			= true;
	status->DisplayType[SC_SUMMON3]			= true;
	status->DisplayType[SC_SUMMON4]			= true;
	status->DisplayType[SC_SUMMON5]			= true;
	status->DisplayType[SC_CAMOUFLAGE]		= true;
	status->DisplayType[SC_DUPLELIGHT]		= true;
	status->DisplayType[SC_ORATIO]			= true;
	status->DisplayType[SC_FROSTMISTY]		= true;
	status->DisplayType[SC_VENOMIMPRESS]		= true;
	status->DisplayType[SC_HALLUCINATIONWALK]	= true;
	status->DisplayType[SC_ROLLINGCUTTER]		= true;
	status->DisplayType[SC_BANDING]			= true;
	status->DisplayType[SC_COLD]		= true;
	status->DisplayType[SC_DEEP_SLEEP]		= true;
	status->DisplayType[SC_CURSEDCIRCLE_ATKER]= true;
	status->DisplayType[SC_CURSEDCIRCLE_TARGET]= true;
	status->DisplayType[SC_BLOOD_SUCKER]		= true;
	status->DisplayType[SC__SHADOWFORM]		= true;
	status->DisplayType[SC_MONSTER_TRANSFORM] = true;
	status->DisplayType[SC_MOONSTAR]		= true;
	status->DisplayType[SC_SUPER_STAR]		= true;
	status->DisplayType[SC_STRANGELIGHTS]		= true;
	status->DisplayType[SC_DECORATION_OF_MUSIC]		= true;

#ifdef RENEWAL_EDP
	// renewal EDP increases your weapon atk
	status->ChangeFlagTable[SC_EDP] |= SCB_WATK;
#endif

	if( !battle_config.display_hallucination ) //Disable Hallucination.
		status->IconChangeTable[SC_ILLUSION] = SI_BLANK;
#undef add_sc
#undef set_sc_with_vfx
}

void initDummyData(void)
{
	memset(&status->dummy, 0, sizeof(status->dummy));
	status->dummy.hp =
		status->dummy.max_hp =
		status->dummy.max_sp =
		status->dummy.str =
		status->dummy.agi =
		status->dummy.vit =
		status->dummy.int_ =
		status->dummy.dex =
		status->dummy.luk =
		status->dummy.hit = 1;
	status->dummy.speed = 2000;
	status->dummy.adelay = 4000;
	status->dummy.amotion = 2000;
	status->dummy.dmotion = 2000;
	status->dummy.ele_lv = 1; //Min elemental level.
	status->dummy.mode = MD_CANMOVE;
}


//For copying a status_data structure from b to a, without overwriting current Hp and Sp
static inline void status_cpy(struct status_data* a, const struct status_data* b)
{
	memcpy((void*)&a->max_hp, (const void*)&b->max_hp, sizeof(struct status_data)-(sizeof(a->hp)+sizeof(a->sp)));
}

//Sets HP to given value. Flag is the flag passed to status->heal in case
//final value is higher than current (use 2 to make a healing effect display
//on players) It will always succeed (overrides Berserk block), but it can't kill.
int status_set_hp(struct block_list *bl, unsigned int hp, int flag) {
	struct status_data *st;
	if (hp < 1) return 0;
	st = status->get_status_data(bl);
	if (st == &status->dummy)
		return 0;

	if (hp > st->max_hp) hp = st->max_hp;
	if (hp == st->hp) return 0;
	if (hp > st->hp)
		return status->heal(bl, hp - st->hp, 0, 1|flag);
	return status_zap(bl, st->hp - hp, 0);
}

//Sets SP to given value. Flag is the flag passed to status->heal in case
//final value is higher than current (use 2 to make a healing effect display
//on players)
int status_set_sp(struct block_list *bl, unsigned int sp, int flag) {
	struct status_data *st;

	st = status->get_status_data(bl);
	if (st == &status->dummy)
		return 0;

	if (sp > st->max_sp) sp = st->max_sp;
	if (sp == st->sp) return 0;
	if (sp > st->sp)
		return status->heal(bl, 0, sp - st->sp, 1|flag);
	return status_zap(bl, 0, st->sp - sp);
}

int status_charge(struct block_list* bl, int64 hp, int64 sp) {
	if(!(bl->type&BL_CONSUME))
		return (int)(hp+sp); //Assume all was charged so there are no 'not enough' fails.
	return status->damage(NULL, bl, hp, sp, 0, 3);
}

//Inflicts damage on the target with the according walkdelay.
//If flag&1, damage is passive and does not triggers cancelling status changes.
//If flag&2, fail if target does not has enough to substract.
//If flag&4, if killed, mob must not give exp/loot.
//flag will be set to &8 when damaging sp of a dead character
int status_damage(struct block_list *src,struct block_list *target,int64 in_hp, int64 in_sp, int walkdelay, int flag) {
	struct status_data *st;
	struct status_change *sc;
	int hp,sp;

	/* here onwards we consider it a 32-type, the client does not support higher and from here onwards the value doesn't get thru percentage modifiers */
	hp = (int)cap_value(in_hp,INT_MIN,INT_MAX);
	sp = (int)cap_value(in_sp,INT_MIN,INT_MAX);
	
	if(sp && !(target->type&BL_CONSUME))
		sp = 0; //Not a valid SP target.

	if (hp < 0) { //Assume absorbed damage.
		status->heal(target, -hp, 0, 1);
		hp = 0;
	}

	if (sp < 0) {
		status->heal(target, 0, -sp, 1);
		sp = 0;
	}

	if (target->type == BL_SKILL)
		return skill->unit_ondamaged((struct skill_unit *)target, src, hp, timer->gettick());

	st = status->get_status_data(target);
	if( st == &status->dummy )
		return 0;

	if ((unsigned int)hp >= st->hp) {
		if (flag&2) return 0;
		hp = st->hp;
	}

	if ((unsigned int)sp > st->sp) {
		if (flag&2) return 0;
		sp = st->sp;
	}

	if (!hp && !sp)
		return 0;

	if( !st->hp )
		flag |= 8;

	// Let through. battle.c/skill.c have the whole logic of when it's possible or
	// not to hurt someone (and this check breaks pet catching) [Skotlex]
	//	if (!target->prev && !(flag&2))
	//		return 0; //Cannot damage a bl not on a map, except when "charging" hp/sp

	sc = status->get_sc(target);
	if( hp && battle_config.invincible_nodamage && src && sc && sc->data[SC_INVINCIBLE] && !sc->data[SC_INVINCIBLEOFF] )
		hp = 1;

	if( hp && !(flag&1) ) {
		if( sc ) {
			struct status_change_entry *sce;
			
#ifdef DEVOTION_REFLECT_DAMAGE
			if(src && (sce = sc->data[SC_DEVOTION])) {
				struct block_list *d_bl = map->id2bl(sce->val1);
				
				if(d_bl &&((d_bl->type == BL_MER && ((TBL_MER *)d_bl)->master && ((TBL_MER *)d_bl)->master->bl.id == target->id)
						   || (d_bl->type == BL_PC && ((TBL_PC *)d_bl)->devotion[sce->val2] == target->id)) && check_distance_bl(target, d_bl, sce->val3)) {
					clif->damage(d_bl, d_bl, 0, 0, hp, 0, 0, 0);
					status_fix_damage(NULL, d_bl, hp, 0);
					return 0;
				}
				status_change_end(target, SC_DEVOTION, INVALID_TIMER);
			}
#endif
			
			if (sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
				status_change_end(target, SC_STONE, INVALID_TIMER);
			status_change_end(target, SC_FREEZE, INVALID_TIMER);
			status_change_end(target, SC_SLEEP, INVALID_TIMER);
			status_change_end(target, SC_DC_WINKCHARM, INVALID_TIMER);
			status_change_end(target, SC_CONFUSION, INVALID_TIMER);
			status_change_end(target, SC_TRICKDEAD, INVALID_TIMER);
			status_change_end(target, SC_HIDING, INVALID_TIMER);
			status_change_end(target, SC_CLOAKING, INVALID_TIMER);
			status_change_end(target, SC_CHASEWALK, INVALID_TIMER);
			status_change_end(target, SC_CAMOUFLAGE, INVALID_TIMER);
			if ((sce=sc->data[SC_ENDURE]) && !sce->val4 && !sc->data[SC_LKCONCENTRATION]) {
				//Endure count is only reduced by non-players on non-gvg maps.
				//val4 signals infinite endure. [Skotlex]
				if (src && src->type != BL_PC && !map_flag_gvg2(target->m) && !map->list[target->m].flag.battleground && --(sce->val2) < 0)
					status_change_end(target, SC_ENDURE, INVALID_TIMER);
			}
			if ((sce=sc->data[SC_GRAVITATION]) && sce->val3 == BCT_SELF) {
				struct skill_unit_group* sg = skill->id2group(sce->val4);
				if (sg) {
					skill->del_unitgroup(sg, ALC_MARK);
					sce->val4 = 0;
					status_change_end(target, SC_GRAVITATION, INVALID_TIMER);
				}
			}
			if(sc->data[SC_DANCING] && (unsigned int)hp > st->max_hp>>2)
				status_change_end(target, SC_DANCING, INVALID_TIMER);
			if(sc->data[SC_CLOAKINGEXCEED] && --(sc->data[SC_CLOAKINGEXCEED]->val2) <= 0)
				status_change_end(target, SC_CLOAKINGEXCEED, INVALID_TIMER);
			if(sc->data[SC_KAGEMUSYA] && --(sc->data[SC_KAGEMUSYA]->val3) <= 0)
				status_change_end(target, SC_KAGEMUSYA, INVALID_TIMER);
		}
		unit->skillcastcancel(target, 2);
	}

	st->hp-= hp;
	st->sp-= sp;

	if (sc && hp && st->hp) {
		if (sc->data[SC_AUTOBERSERK] &&
			(!sc->data[SC_PROVOKE] || !sc->data[SC_PROVOKE]->val2) &&
			st->hp < st->max_hp>>2)
			sc_start4(src,target,SC_PROVOKE,100,10,1,0,0,0);
		if (sc->data[SC_BERSERK] && st->hp <= 100)
			status_change_end(target, SC_BERSERK, INVALID_TIMER);
		if( sc->data[SC_RAISINGDRAGON] && st->hp <= 1000 )
			status_change_end(target, SC_RAISINGDRAGON, INVALID_TIMER);
		if (sc->data[SC_SATURDAY_NIGHT_FEVER] && st->hp <= 100)
			status_change_end(target, SC_SATURDAY_NIGHT_FEVER, INVALID_TIMER);
	}

	switch (target->type) {
	case BL_PC:  pc->damage((TBL_PC*)target,src,hp,sp); break;
	case BL_MOB: mob->damage((TBL_MOB*)target, src, hp); break;
	case BL_HOM: homun->damaged((TBL_HOM*)target); break;
	case BL_MER: mercenary->heal((TBL_MER*)target,hp,sp); break;
	case BL_ELEM: elemental->heal((TBL_ELEM*)target,hp,sp); break;
	}

	if( src && target->type == BL_PC && (((TBL_PC*)target)->disguise) > 0 ) {// stop walking when attacked in disguise to prevent walk-delay bug
		unit->stop_walking( target, 1 );
	}

	if( st->hp || (flag&8) )
	{	//Still lives or has been dead before this damage.
		if (walkdelay)
			unit->set_walkdelay(target, timer->gettick(), walkdelay, 0);
		return (int)(hp+sp);
	}

	st->hp = 1; //To let the dead function cast skills and all that.
	//NOTE: These dead functions should return: [Skotlex]
	//0: Death cancelled, auto-revived.
	//Non-zero: Standard death. Clear status, cancel move/attack, etc
	//&2: Also remove object from map.
	//&4: Also delete object from memory.
	switch (target->type) {
	case BL_PC:  flag = pc->dead((TBL_PC*)target,src); break;
	case BL_MOB: flag = mob->dead((TBL_MOB*)target, src, flag&4?3:0); break;
	case BL_HOM: flag = homun->dead((TBL_HOM*)target); break;
	case BL_MER: flag = mercenary->dead((TBL_MER*)target); break;
	case BL_ELEM: flag = elemental->dead((TBL_ELEM*)target); break;
	default:	//Unhandled case, do nothing to object.
		flag = 0;
		break;
	}

	if(!flag) //Death cancelled.
		return (int)(hp+sp);

	//Normal death
	st->hp = 0;
	if (battle_config.clear_unit_ondeath &&
		battle_config.clear_unit_ondeath&target->type)
		skill->clear_unitgroup(target);

	if(target->type&BL_REGEN) {
		//Reset regen ticks.
		struct regen_data *regen = status->get_regen_data(target);
		if (regen) {
			memset(&regen->tick, 0, sizeof(regen->tick));
			if (regen->sregen)
				memset(&regen->sregen->tick, 0, sizeof(regen->sregen->tick));
			if (regen->ssregen)
				memset(&regen->ssregen->tick, 0, sizeof(regen->ssregen->tick));
		}
	}

	if( sc && sc->data[SC_KAIZEL] && !map_flag_gvg2(target->m) ) {
		//flag&8 = disable Kaizel
		int time = skill->get_time2(SL_KAIZEL,sc->data[SC_KAIZEL]->val1);
		//Look for Osiris Card's bonus effect on the character and revive 100% or revive normally
		if ( target->type == BL_PC && BL_CAST(BL_PC,target)->special_state.restart_full_recover )
			status->revive(target, 100, 100);
		else
			status->revive(target, sc->data[SC_KAIZEL]->val2, 0);
		status->change_clear(target,0);
		clif->skill_nodamage(target,target,ALL_RESURRECTION,1,1);
		sc_start(target,target,status->skill2sc(PR_KYRIE),100,10,time);

		if( target->type == BL_MOB )
			((TBL_MOB*)target)->state.rebirth = 1;

		return (int)(hp+sp);
	}
	if(target->type == BL_PC) {
		TBL_PC *sd = BL_CAST(BL_PC,target);
		TBL_HOM *hd = sd->hd;
		if(hd && hd->sc.data[SC_LIGHT_OF_REGENE]){
			clif->skillcasting(&hd->bl, hd->bl.id, target->id, 0,0, MH_LIGHT_OF_REGENE, skill->get_ele(MH_LIGHT_OF_REGENE, 1), 10); //just to display usage
			clif->skill_nodamage(&sd->bl, target, ALL_RESURRECTION, 1, status->revive(&sd->bl,10*hd->sc.data[SC_LIGHT_OF_REGENE]->val1,0));
			status_change_end(&sd->hd->bl,SC_LIGHT_OF_REGENE,INVALID_TIMER);
			return (int)(hp + sp);
		}
	}
	if (target->type == BL_MOB && sc && sc->data[SC_REBIRTH] && !((TBL_MOB*) target)->state.rebirth) {
		// Ensure the monster has not already rebirthed before doing so.
		status->revive(target, sc->data[SC_REBIRTH]->val2, 0);
		status->change_clear(target,0);
		((TBL_MOB*)target)->state.rebirth = 1;

		return (int)(hp+sp);
	}

	status->change_clear(target,0);

	if(flag&4) //Delete from memory. (also invokes map removal code)
		unit->free(target,CLR_DEAD);
	else if(flag&2) //remove from map
		unit->remove_map(target,CLR_DEAD,ALC_MARK);
	else { //Some death states that would normally be handled by unit_remove_map
		unit->stop_attack(target);
		unit->stop_walking(target,1);
		unit->skillcastcancel(target,0);
		clif->clearunit_area(target,CLR_DEAD);
		skill->unit_move(target,timer->gettick(),4);
		skill->cleartimerskill(target);
	}

	return (int)(hp+sp);
}

//Heals a character. If flag&1, this is forced healing (otherwise stuff like Berserk can block it)
//If flag&2, when the player is healed, show the HP/SP heal effect.
int status_heal(struct block_list *bl,int64 in_hp,int64 in_sp, int flag) {
	struct status_data *st;
	struct status_change *sc;
	int hp,sp;

	st = status->get_status_data(bl);

	if (st == &status->dummy || !st->hp)
		return 0;

	/* here onwards we consider it a 32-type, the client does not support higher and from here onwards the value doesn't get thru percentage modifiers */
	hp = (int)cap_value(in_hp,INT_MIN,INT_MAX);
	sp = (int)cap_value(in_sp,INT_MIN,INT_MAX);
	
	sc = status->get_sc(bl);
	if (sc && !sc->count)
		sc = NULL;

	if (hp < 0) {
		if (hp == INT_MIN) hp++; //-INT_MIN == INT_MIN in some architectures!
		status->damage(NULL, bl, -hp, 0, 0, 1);
		hp = 0;
	}

	if(hp) {
		if( sc && sc->data[SC_BERSERK] ) {
			if( flag&1 )
				flag &= ~2;
			else
				hp = 0;
		}

		if((unsigned int)hp > st->max_hp - st->hp)
			hp = st->max_hp - st->hp;
	}

	if(sp < 0) {
		if (sp==INT_MIN) sp++;
		status->damage(NULL, bl, 0, -sp, 0, 1);
		sp = 0;
	}

	if(sp) {
		if((unsigned int)sp > st->max_sp - st->sp)
			sp = st->max_sp - st->sp;
	}

	if(!sp && !hp) return 0;

	st->hp+= hp;
	st->sp+= sp;

	if( hp && sc
	    && sc->data[SC_AUTOBERSERK]
	    && sc->data[SC_PROVOKE]
	    && sc->data[SC_PROVOKE]->val2==1
	    && st->hp>=st->max_hp>>2
	 ) //End auto berserk.
		status_change_end(bl, SC_PROVOKE, INVALID_TIMER);

	// send hp update to client
	switch(bl->type) {
		case BL_PC:  pc->heal((TBL_PC*)bl,hp,sp,flag&2?1:0); break;
		case BL_MOB: mob->heal((TBL_MOB*)bl,hp); break;
		case BL_HOM: homun->healed((TBL_HOM*)bl); break;
		case BL_MER: mercenary->heal((TBL_MER*)bl,hp,sp); break;
		case BL_ELEM: elemental->heal((TBL_ELEM*)bl,hp,sp); break;
	}

	return (int)(hp+sp);
}

//Does percentual non-flinching damage/heal. If mob is killed this way,
//no exp/drops will be awarded if there is no src (or src is target)
//If rates are > 0, percent is of current HP/SP
//If rates are < 0, percent is of max HP/SP
//If !flag, this is heal, otherwise it is damage.
//Furthermore, if flag==2, then the target must not die from the substraction.
int status_percent_change(struct block_list *src,struct block_list *target,signed char hp_rate, signed char sp_rate, int flag) {
	struct status_data *st;
	unsigned int hp = 0, sp = 0;

	st = status->get_status_data(target);

	if (hp_rate > 100)
		hp_rate = 100;
	else if (hp_rate < -100)
		hp_rate = -100;
	if (hp_rate > 0)
		hp = APPLY_RATE(st->hp, hp_rate);
	else
		hp = APPLY_RATE(st->max_hp, -hp_rate);
	if (hp_rate && !hp)
		hp = 1;

	if (flag == 2 && hp >= st->hp)
		hp = st->hp-1; //Must not kill target.

	if (sp_rate > 100)
		sp_rate = 100;
	else if (sp_rate < -100)
		sp_rate = -100;
	if (sp_rate > 0)
		sp = APPLY_RATE(st->sp, sp_rate);
	else
		sp = APPLY_RATE(st->max_sp, -sp_rate);
	if (sp_rate && !sp)
		sp = 1;

	//Ugly check in case damage dealt is too much for the received args of
	//status->heal / status->damage. [Skotlex]
	if (hp > INT_MAX) {
		hp -= INT_MAX;
		if (flag)
			status->damage(src, target, INT_MAX, 0, 0, (!src||src==target?5:1));
		else
			status->heal(target, INT_MAX, 0, 0);
	}
	if (sp > INT_MAX) {
		sp -= INT_MAX;
		if (flag)
			status->damage(src, target, 0, INT_MAX, 0, (!src||src==target?5:1));
		else
			status->heal(target, 0, INT_MAX, 0);
	}
	if (flag)
		return status->damage(src, target, hp, sp, 0, (!src||src==target?5:1));
	return status->heal(target, hp, sp, 0);
}

int status_revive(struct block_list *bl, unsigned char per_hp, unsigned char per_sp) {
	struct status_data *st;
	unsigned int hp, sp;
	if (!status->isdead(bl)) return 0;

	st = status->get_status_data(bl);
	if (st == &status->dummy)
		return 0; //Invalid target.

	hp = APPLY_RATE(st->max_hp, per_hp);
	sp = APPLY_RATE(st->max_sp, per_sp);

	if(hp > st->max_hp - st->hp)
		hp = st->max_hp - st->hp;
	else if (per_hp && !hp)
		hp = 1;

	if(sp > st->max_sp - st->sp)
		sp = st->max_sp - st->sp;
	else if (per_sp && !sp)
		sp = 1;

	st->hp += hp;
	st->sp += sp;

	if (bl->prev) //Animation only if character is already on a map.
		clif->resurrection(bl, 1);
	
	switch (bl->type) {
		case BL_PC:  pc->revive((TBL_PC*)bl, hp, sp); break;
		case BL_MOB: mob->revive((TBL_MOB*)bl, hp); break;
		case BL_HOM: homun->revive((TBL_HOM*)bl, hp, sp); break;
	}
	
	return 1;
}

int status_fixed_revive(struct block_list *bl, unsigned int per_hp, unsigned int per_sp) {
	struct status_data *st;
	unsigned int hp, sp;
	if (!status->isdead(bl)) return 0;

	st = status->get_status_data(bl);
	if (st == &status->dummy)
		return 0; //Invalid target.

	hp = per_hp;
	sp = per_sp;

	if(hp > st->max_hp - st->hp)
		hp = st->max_hp - st->hp;
	else if (per_hp && !hp)
		hp = 1;
		
	if(sp > st->max_sp - st->sp)
		sp = st->max_sp - st->sp;
	else if (per_sp && !sp)
		sp = 1;

	st->hp += hp;
	st->sp += sp;

	if (bl->prev) //Animation only if character is already on a map.
		clif->resurrection(bl, 1);
	switch (bl->type) {
		case BL_PC:  pc->revive((TBL_PC*)bl, hp, sp); break;
		case BL_MOB: mob->revive((TBL_MOB*)bl, hp); break;
		case BL_HOM: homun->revive((TBL_HOM*)bl, hp, sp); break;
	}
	return 1;
}

/*==========================================
* Checks whether the src can use the skill on the target,
* taking into account status/option of both source/target. [Skotlex]
* flag:
* 	0 - Trying to use skill on target.
* 	1 - Cast bar is done.
* 	2 - Skill already pulled off, check is due to ground-based skills or splash-damage ones.
* src MAY be null to indicate we shouldn't check it, this is a ground-based skill attack.
* target MAY Be null, in which case the checks are only to see
* whether the source can cast or not the skill on the ground.
*------------------------------------------*/
int status_check_skilluse(struct block_list *src, struct block_list *target, uint16 skill_id, int flag) {
	struct status_data *st;
	struct status_change *sc=NULL, *tsc;
	int hide_flag;

	st = src ? status->get_status_data(src) : &status->dummy;

	if (src && src->type != BL_PC && status->isdead(src))
		return 0;

	if (!skill_id) { //Normal attack checks.
		if (!(st->mode&MD_CANATTACK))
			return 0; //This mode is only needed for melee attacking.
		//Dead state is not checked for skills as some skills can be used
		//on dead characters, said checks are left to skill.c [Skotlex]
		if (target && status->isdead(target))
			return 0;
		if( src && (sc = status->get_sc(src)) && sc->data[SC_COLD] && src->type != BL_MOB)
			return 0;
	}

	if( skill_id ) {

		if( src && !(src->type == BL_PC && ((TBL_PC*)src)->skillitem)) { // Items that cast skills using 'itemskill' will not be handled by map_zone_db.
			int i;

			for(i = 0; i < map->list[src->m].zone->disabled_skills_count; i++) {
				if( skill_id == map->list[src->m].zone->disabled_skills[i]->nameid && (map->list[src->m].zone->disabled_skills[i]->type&src->type) ) {
					if( src->type == BL_PC )
						clif->msg((TBL_PC*)src, SKILL_CANT_USE_AREA); // This skill cannot be used within this area
					else if( src->type == BL_MOB && map->list[src->m].zone->disabled_skills[i]->subtype != MZS_NONE ) {
						if( (st->mode&MD_BOSS) && !(map->list[src->m].zone->disabled_skills[i]->subtype&MZS_BOSS) )
							break;
					}
					return 0;
				}
			}
		}

		switch( skill_id ) {
			case PA_PRESSURE:
				if( flag && target ) {
					//Gloria Avoids pretty much everything....
					tsc = status->get_sc(target);
					if(tsc && tsc->option&OPTION_HIDE)
						return 0;
				}
				break;
			case GN_WALLOFTHORN:
				if( target && status->isdead(target) )
					return 0;
				break;
			case AL_TELEPORT:
				//Should fail when used on top of Land Protector [Skotlex]
				if (src && map->getcell(src->m, src->x, src->y, CELL_CHKLANDPROTECTOR)
					&& !(st->mode&MD_BOSS)
					&& (src->type != BL_PC || ((TBL_PC*)src)->skillitem != skill_id))
					return 0;
				break;
			default:
				break;
		}
	}

	if ( src ) sc = status->get_sc(src);

	if( sc && sc->count ) {

		if (skill_id != RK_REFRESH && sc->opt1 >0 && !(sc->opt1 == OPT1_CRYSTALIZE && src->type == BL_MOB) && sc->opt1 != OPT1_BURNING && skill_id != SR_GENTLETOUCH_CURE) { //Stuned/Frozen/etc
			if (flag != 1) //Can't cast, casted stuff can't damage.
				return 0;
			if (!(skill->get_inf(skill_id)&INF_GROUND_SKILL))
				return 0; //Targetted spells can't come off.
		}

		if (
			(sc->data[SC_TRICKDEAD] && skill_id != NV_TRICKDEAD)
			|| (sc->data[SC_AUTOCOUNTER] && !flag)
			|| (sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_SELF && skill_id != PA_GOSPEL)
			|| (sc->data[SC_GRAVITATION] && sc->data[SC_GRAVITATION]->val3 == BCT_SELF && flag != 2)
			)
			return 0;

		if (sc->data[SC_DC_WINKCHARM] && target && !flag) { //Prevents skill usage
			if( unit->bl2ud(src) && (unit->bl2ud(src))->walktimer == INVALID_TIMER )
				unit->walktobl(src, map->id2bl(sc->data[SC_DC_WINKCHARM]->val2), 3, 1);
			clif->emotion(src, E_LV);
			return 0;
		}

		if (sc->data[SC_BLADESTOP]) {
			switch (sc->data[SC_BLADESTOP]->val1)
			{
			case 5: if (skill_id == MO_EXTREMITYFIST) break;
			case 4: if (skill_id == MO_CHAINCOMBO) break;
			case 3: if (skill_id == MO_INVESTIGATE) break;
			case 2: if (skill_id == MO_FINGEROFFENSIVE) break;
			default: return 0;
			}
		}

		if (sc->data[SC_DANCING] && flag!=2) {
			if( src->type == BL_PC && skill_id >= WA_SWING_DANCE && skill_id <= WM_UNLIMITED_HUMMING_VOICE )
			{ // Lvl 5 Lesson or higher allow you use 3rd job skills while dancing.v
				if( pc->checkskill((TBL_PC*)src,WM_LESSON) < 5 )
					return 0;
			} else if(sc->data[SC_LONGING]) { //Allow everything except dancing/re-dancing. [Skotlex]
				if (skill_id == BD_ENCORE ||
					skill->get_inf2(skill_id)&(INF2_SONG_DANCE|INF2_ENSEMBLE_SKILL)
					)
					return 0;
			} else {
				switch (skill_id) {
				case BD_ADAPTATION:
				case CG_LONGINGFREEDOM:
				case BA_MUSICALSTRIKE:
				case DC_THROWARROW:
					break;
				default:
					return 0;
				}
			}
			if ((sc->data[SC_DANCING]->val1&0xFFFF) == CG_HERMODE && skill_id == BD_ADAPTATION)
				return 0;	//Can't amp out of Wand of Hermode :/ [Skotlex]
		}

		if (skill_id && //Do not block item-casted skills.
			(src->type != BL_PC || ((TBL_PC*)src)->skillitem != skill_id)
			) {	//Skills blocked through status changes...
				if (!flag && ( //Blocked only from using the skill (stuff like autospell may still go through
					sc->data[SC_SILENCE] ||
					sc->data[SC_STEELBODY] ||
					sc->data[SC_BERSERK] ||
					sc->data[SC_OBLIVIONCURSE] ||
					sc->data[SC_WHITEIMPRISON] ||
					sc->data[SC__INVISIBILITY] ||
					(sc->data[SC_COLD] && src->type != BL_MOB) ||
					sc->data[SC__IGNORANCE] ||
					sc->data[SC_DEEP_SLEEP] ||
					sc->data[SC_SATURDAY_NIGHT_FEVER] ||
					sc->data[SC_CURSEDCIRCLE_TARGET] ||
					(sc->data[SC_MARIONETTE_MASTER] && skill_id != CG_MARIONETTE) || //Only skill you can use is marionette again to cancel it
					(sc->data[SC_MARIONETTE] && skill_id == CG_MARIONETTE) || //Cannot use marionette if you are being buffed by another
					(sc->data[SC_STASIS] && skill->block_check(src, SC_STASIS, skill_id)) ||
					(sc->data[SC_KG_KAGEHUMI] && skill->block_check(src, SC_KG_KAGEHUMI, skill_id))
					))
					return 0;

				//Skill blocking.
				if (
					(sc->data[SC_VOLCANO] && skill_id == WZ_ICEWALL) ||
					(sc->data[SC_ROKISWEIL] && skill_id != BD_ADAPTATION) ||
					(sc->data[SC_HERMODE] && skill->get_inf(skill_id) & INF_SUPPORT_SKILL) ||
					(sc->data[SC_NOCHAT] && sc->data[SC_NOCHAT]->val1&MANNER_NOSKILL)
					)
					return 0;

				if( sc->data[SC__MANHOLE] || ((tsc = status->get_sc(target)) && tsc->data[SC__MANHOLE]) ) {
					switch(skill_id) {//##TODO## make this a flag in skill_db?
						// Skills that can be used even under Man Hole effects.
					case SC_SHADOWFORM:
						break;
					default:
						return 0;
					}
				}

		}
	}

	if (sc && sc->option) {
		if (sc->option&OPTION_HIDE) {
			switch (skill_id) { //Usable skills while hiding.
				case TF_HIDING:
				case AS_GRIMTOOTH:
				case RG_BACKSTAP:
				case RG_RAID:
				case NJ_SHADOWJUMP:
				case NJ_KIRIKAGE:
				case KO_YAMIKUMO:
					break;
				default:
					//Non players can use all skills while hidden.
					if (!skill_id || src->type == BL_PC)
						return 0;
			}
		}
		if ( sc->option&OPTION_CHASEWALK ) {
			if ( sc->data[SC__INVISIBILITY] ) {
				if ( skill_id != 0 && skill_id != SC_INVISIBILITY )
					return 0;
			} else if ( skill_id != ST_CHASEWALK )
				return 0;
		}
		if( sc->data[SC_ALL_RIDING] )
			return 0;//New mounts can't attack nor use skills in the client; this check makes it cheat-safe [Ind]
	}

	if (target == NULL || target == src) //No further checking needed.
		return 1;

	tsc = status->get_sc(target);

	if(tsc && tsc->count) {
		/* attacks in invincible are capped to 1 damage and handled in batte.c; allow spell break and eske for sealed shrine GDB when in INVINCIBLE state. */
		if( tsc->data[SC_INVINCIBLE] && !tsc->data[SC_INVINCIBLEOFF] && skill_id && !(skill_id&(SA_SPELLBREAKER|SL_SKE)) )
			return 0;
		if(!skill_id && tsc->data[SC_TRICKDEAD])
			return 0;
		if((skill_id == WZ_STORMGUST || skill_id == WZ_FROSTNOVA || skill_id == NJ_HYOUSYOURAKU)
			&& tsc->data[SC_FREEZE])
			return 0;
		if(skill_id == PR_LEXAETERNA && (tsc->data[SC_FREEZE] || (tsc->data[SC_STONE] && tsc->opt1 == OPT1_STONE)))
			return 0;
		if( ( tsc->data[SC_STEALTHFIELD] || tsc->data[SC_CAMOUFLAGE] ) && !(st->mode&(MD_BOSS|MD_DETECTOR)) && flag == 4 )
			return 0;
	}
	//If targetting, cloak+hide protect you, otherwise only hiding does.
	hide_flag = flag?OPTION_HIDE:(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK);

	//You cannot hide from ground skills.
	if( skill->get_ele(skill_id,1) == ELE_EARTH ) //TODO: Need Skill Lv here :/
		hide_flag &= ~OPTION_HIDE;
	else {
		switch ( skill_id ) {
			case MO_ABSORBSPIRITS: // it works when already casted and target suddenly hides.
				hide_flag &= ~OPTION_HIDE;
				break;
		}
	}

	switch( target->type ) {
		case BL_PC: {
				struct map_session_data *sd = (TBL_PC*) target;
				bool is_boss = (st->mode&MD_BOSS);
				bool is_detect = ((st->mode&MD_DETECTOR)?true:false);//god-knows-why gcc doesn't shut up until this happens
				if (pc_isinvisible(sd))
					return 0;
				if( tsc ) {
					if (tsc->option&hide_flag && !is_boss &&
						((sd->special_state.perfect_hiding || !is_detect) ||
						(tsc->data[SC_CLOAKINGEXCEED] && is_detect)))
						return 0;
					if( tsc->data[SC_CAMOUFLAGE] && !(is_boss || is_detect) && (!skill_id || (flag == 0 && src && src->type != BL_PC)) )
						return 0;
					if( tsc->data[SC_STEALTHFIELD] && !is_boss )
						return 0;
				}
			}
			break;
		case BL_ITEM:	//Allow targetting of items to pick'em up (or in the case of mobs, to loot them).
			//TODO: Would be nice if this could be used to judge whether the player can or not pick up the item it targets. [Skotlex]
			if (st->mode&MD_LOOTER)
				return 1;
			return 0;
		case BL_HOM:
		case BL_MER:
		case BL_ELEM:
			if( target->type == BL_HOM && skill_id && battle_config.hom_setting&0x1 && skill->get_inf(skill_id)&INF_SUPPORT_SKILL && battle->get_master(target) != src )
				return 0; // Can't use support skills on Homunculus (only Master/Self)
			if( target->type == BL_MER && (skill_id == PR_ASPERSIO || (skill_id >= SA_FLAMELAUNCHER && skill_id <= SA_SEISMICWEAPON)) && battle->get_master(target) != src )
				return 0; // Can't use Weapon endow skills on Mercenary (only Master)
			if( skill_id == AM_POTIONPITCHER && ( target->type == BL_MER || target->type == BL_ELEM) )
				return 0; // Can't use Potion Pitcher on Mercenaries
		default:
			//Check for chase-walk/hiding/cloaking opponents.
			if( tsc ) {
				if( tsc->option&hide_flag && !(st->mode&(MD_BOSS|MD_DETECTOR)))
					return 0;
				if( tsc->data[SC_STEALTHFIELD] && !(st->mode&MD_BOSS) )
					return 0;
			}
	}

	return 1;
}

//Checks whether the source can see and chase target.
int status_check_visibility(struct block_list *src, struct block_list *target) {
	int view_range;
	struct status_change *tsc = NULL;
	
	switch (src->type) {
		case BL_MOB:
			view_range = ((TBL_MOB*)src)->min_chase;
			break;
		case BL_PET:
			view_range = ((TBL_PET*)src)->db->range2;
			break;
		default:
			view_range = AREA_SIZE;
	}

	if (src->m != target->m || !check_distance_bl(src, target, view_range))
		return 0;

	if( ( tsc = status->get_sc(target) ) ) {
		struct status_data *st = status->get_status_data(src);

		switch (target->type) { //Check for chase-walk/hiding/cloaking opponents.
			case BL_PC:
				if ( tsc->data[SC_CLOAKINGEXCEED] && !(st->mode&MD_BOSS) )
					return 0;
				if( (tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) || tsc->data[SC_STEALTHFIELD] || tsc->data[SC__INVISIBILITY] || tsc->data[SC_CAMOUFLAGE]) && !(st->mode&MD_BOSS) &&
					( ((TBL_PC*)target)->special_state.perfect_hiding || !(st->mode&MD_DETECTOR) ) )
					return 0;
				break;
			default:
				if( (tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) || tsc->data[SC_CAMOUFLAGE]) && !(st->mode&(MD_BOSS|MD_DETECTOR)) )
					return 0;

		}
	}

	return 1;
}

// Basic ASPD value
int status_base_amotion_pc(struct map_session_data *sd, struct status_data *st) {
	int amotion;
#ifdef RENEWAL_ASPD
	short mod = -1;

	switch( sd->weapontype2 ){ // adjustment for dual weilding
		case W_DAGGER:	mod = 0;	break; // 0, 1, 1
		case W_1HSWORD:
		case W_1HAXE:	mod = 1;
			if( (sd->class_&MAPID_THIRDMASK) == MAPID_GUILLOTINE_CROSS ) // 0, 2, 3
				mod = sd->weapontype2 / W_1HSWORD + W_1HSWORD / sd->weapontype2 ;
	}

	amotion = ( sd->status.weapon < MAX_WEAPON_TYPE && mod < 0 )
		? (status->aspd_base[pc->class2idx(sd->status.class_)][sd->status.weapon]) // single weapon
		: ((status->aspd_base[pc->class2idx(sd->status.class_)][sd->weapontype2] // dual-wield
	+ status->aspd_base[pc->class2idx(sd->status.class_)][sd->weapontype2]) * 6 / 10 + 10 * mod
		- status->aspd_base[pc->class2idx(sd->status.class_)][sd->weapontype2]
	+ status->aspd_base[pc->class2idx(sd->status.class_)][sd->weapontype1]);

	if ( sd->status.shield )
		amotion += ( 2000 - status->aspd_base[pc->class2idx(sd->status.class_)][W_FIST] ) +
		( status->aspd_base[pc->class2idx(sd->status.class_)][MAX_WEAPON_TYPE] - 2000 );

#else
	// base weapon delay
	amotion = (sd->status.weapon < MAX_WEAPON_TYPE)
		? (status->aspd_base[pc->class2idx(sd->status.class_)][sd->status.weapon]) // single weapon
		: (status->aspd_base[pc->class2idx(sd->status.class_)][sd->weapontype1] + status->aspd_base[pc->class2idx(sd->status.class_)][sd->weapontype2])*7/10; // dual-wield

	// percentual delay reduction from stats
	amotion -= amotion * (4*st->agi + st->dex)/1000;
#endif
	
	// raw delay adjustment from bAspd bonus
	amotion += sd->bonus.aspd_add;

	/* angra manyu disregards aspd_base and similar */
	if( sd->equip_index[EQI_HAND_R] >= 0 && sd->status.inventory[sd->equip_index[EQI_HAND_R]].nameid == ITEMID_ANGRA_MANYU )
		return 0;
	
	return amotion;
}

unsigned short status_base_atk(const struct block_list *bl, const struct status_data *st) {
	int flag = 0, str, dex,
#ifdef RENEWAL
		rstr,
#endif
		dstr;


	if(!(bl->type&battle_config.enable_baseatk))
		return 0;

	if (bl->type == BL_PC)
		switch(((TBL_PC*)bl)->status.weapon){
		case W_BOW:
		case W_MUSICAL:
		case W_WHIP:
		case W_REVOLVER:
		case W_RIFLE:
		case W_GATLING:
		case W_SHOTGUN:
		case W_GRENADE:
			flag = 1;
	}
	if (flag) {
#ifdef RENEWAL
		rstr =
#endif
			str = st->dex;
		dex = st->str;
	} else {
#ifdef RENEWAL
		rstr =
#endif
			str = st->str;
		dex = st->dex;
	}
	//Normally only players have base-atk, but homunc have a different batk
	// equation, hinting that perhaps non-players should use this for batk.
	// [Skotlex]
	dstr = str/10;
	str += dstr*dstr;
	if (bl->type == BL_PC)
#ifdef RENEWAL
		str = (int)(rstr + (float)dex/5 + (float)st->luk/3 + (float)((TBL_PC*)bl)->status.base_level/4);
	else if(bl->type == BL_MOB)
		str = rstr + ((TBL_MOB*)bl)->level;
#else
		str+= dex/5 + st->luk/5;
#endif
	return cap_value(str, 0, USHRT_MAX);
}

static inline unsigned short status_base_matk_min(const struct status_data *st){ return st->int_+(st->int_/7)*(st->int_/7); }
static inline unsigned short status_base_matk_max(const struct status_data *st){ return st->int_+(st->int_/5)*(st->int_/5); }

unsigned short status_base_matk(const struct status_data *st, int level) {
#ifdef RENEWAL
	return st->int_+(st->int_/2)+(st->dex/5)+(st->luk/3)+(level/4);
#else
	return 0;
#endif
}

//Fills in the misc data that can be calculated from the other status info (except for level)
void status_calc_misc(struct block_list *bl, struct status_data *st, int level) {
	//Non players get the value set, players need to stack with previous bonuses.
	if( bl->type != BL_PC )
		st->batk =
		st->hit = st->flee =
		st->def2 = st->mdef2 =
		st->cri = st->flee2 = 0;

#ifdef RENEWAL // renewal formulas
	st->matk_min = st->matk_max = bl->type == BL_PC ? status->base_matk(st, level) : level + st->int_;
	st->hit += level + st->dex + (bl->type == BL_PC ? st->luk/3 + 175 : 150); //base level + ( every 1 dex = +1 hit ) + (every 3 luk = +1 hit) + 175
	st->flee += level + st->agi + (bl->type == BL_PC ? st->luk/5 : 0) + 100; //base level + ( every 1 agi = +1 flee ) + (every 5 luk = +1 flee) + 100
	st->def2 += (int)(((float)level + st->vit)/2 + ( bl->type == BL_PC ? ((float)st->agi/5) : 0 )); //base level + (every 2 vit = +1 def) + (every 5 agi = +1 def)
	st->mdef2 += (int)( bl->type == BL_PC ?(st->int_ + ((float)level/4) + ((float)(st->dex+st->vit)/5)):((float)(st->int_ + level)/4)); //(every 4 base level = +1 mdef) + (every 1 int = +1 mdef) + (every 5 dex = +1 mdef) + (every 5 vit = +1 mdef)
#else // not RENEWAL
	st->matk_min = status_base_matk_min(st);
	st->matk_max = status_base_matk_max(st);
	st->hit += level + st->dex;
	st->flee += level + st->agi;
	st->def2 += st->vit;
	st->mdef2 += st->int_ + (st->vit>>1);
#endif // RENEWAL

	if( bl->type&battle_config.enable_critical )
		st->cri += 10 + (st->luk*10/3); //(every 1 luk = +0.3 critical)
	else
		st->cri = 0;

	if (bl->type&battle_config.enable_perfect_flee)
		st->flee2 += st->luk + 10; //(every 10 luk = +1 perfect flee)
	else
		st->flee2 = 0;

	if (st->batk) {
		int temp = st->batk + status->base_atk(bl, st);
		st->batk = cap_value(temp, 0, USHRT_MAX);
	} else
		st->batk = status->base_atk(bl, st);
	if (st->cri)
		switch (bl->type) {
		case BL_MOB:
			if(battle_config.mob_critical_rate != 100)
				st->cri = st->cri*battle_config.mob_critical_rate/100;
			if(!st->cri && battle_config.mob_critical_rate)
				st->cri = 10;
			break;
		case BL_PC:
			//Players don't have a critical adjustment setting as of yet.
			break;
		case BL_MER:
#ifdef RENEWAL
			st->matk_min = st->matk_max = status_base_matk_max(st);
			st->def2 = st->vit + level / 10 + st->vit / 5;
			st->mdef2 = level / 10 + st->int_ / 5;
#endif
			break;
		default:
			if(battle_config.critical_rate != 100)
				st->cri = st->cri*battle_config.critical_rate/100;
			if (!st->cri && battle_config.critical_rate)
				st->cri = 10;
	}
	if(bl->type&BL_REGEN)
		status->calc_regen(bl, st, status->get_regen_data(bl));
}

//Skotlex: Calculates the initial status for the given mob
//first will only be false when the mob leveled up or got a GuardUp level.
int status_calc_mob_(struct mob_data* md, enum e_status_calc_opt opt) {
	struct status_data *mstatus;
	struct block_list *mbl = NULL;
	int flag=0;

	if(opt&SCO_FIRST) { //Set basic level on respawn.
		if (md->level > 0 && md->level <= MAX_LEVEL && md->level != md->db->lv)
			;
		else
			md->level = md->db->lv;
	}

	//Check if we need custom base-status
	if (battle_config.mobs_level_up && md->level > md->db->lv)
		flag|=1;

	if (md->special_state.size)
		flag|=2;

	if (md->guardian_data && md->guardian_data->guardup_lv)
		flag|=4;

	if (battle_config.slaves_inherit_speed && md->master_id)
		flag|=8;

	if (md->master_id && md->special_state.ai>1)
		flag|=16;

	if (!flag)
	{ //No special status required.
		if (md->base_status) {
			aFree(md->base_status);
			md->base_status = NULL;
		}
		if(opt&SCO_FIRST)
			memcpy(&md->status, &md->db->status, sizeof(struct status_data));
		return 0;
	}
	if (!md->base_status)
		md->base_status = (struct status_data*)aCalloc(1, sizeof(struct status_data));

	mstatus = md->base_status;
	memcpy(mstatus, &md->db->status, sizeof(struct status_data));

	if (flag&(8|16))
		mbl = map->id2bl(md->master_id);

	if (flag&8 && mbl) {
		struct status_data *masterstatus = status->get_base_status(mbl);
		if ( masterstatus ) {
			if (battle_config.slaves_inherit_speed&(masterstatus->mode&MD_CANMOVE ? 1 : 2))
				mstatus->speed = masterstatus->speed;
			if (mstatus->speed < 2) /* minimum for the unit to function properly */
				mstatus->speed = 2;
		}
	}

	if (flag&16 && mbl) {
		//Max HP setting from Summon Flora/marine Sphere
		struct unit_data *ud = unit->bl2ud(mbl);
		//Remove special AI when this is used by regular mobs.
		if (mbl->type == BL_MOB && !((TBL_MOB*)mbl)->special_state.ai)
			md->special_state.ai = 0;
		if (ud) {
			// different levels of HP according to skill level
			if (ud->skill_id == AM_SPHEREMINE) {
				mstatus->max_hp = 2000 + 400*ud->skill_lv;
			} else if(ud->skill_id == KO_ZANZOU) {
				mstatus->max_hp = 3000 + 3000 * ud->skill_lv + status_get_max_sp(battle->get_master(mbl));
			} else { //AM_CANNIBALIZE
				mstatus->max_hp = 1500 + 200*ud->skill_lv + 10*status->get_lv(mbl);
				mstatus->mode|= MD_CANATTACK|MD_AGGRESSIVE;
			}
			mstatus->hp = mstatus->max_hp;
			if( ud->skill_id == NC_SILVERSNIPER )
				mstatus->rhw.atk = mstatus->rhw.atk2 = 200 * ud->skill_lv;
		}
	}

	if (flag&1) {
		// increase from mobs leveling up [Valaris]
		int diff = md->level - md->db->lv;
		mstatus->str+= diff;
		mstatus->agi+= diff;
		mstatus->vit+= diff;
		mstatus->int_+= diff;
		mstatus->dex+= diff;
		mstatus->luk+= diff;
		mstatus->max_hp += diff*mstatus->vit;
		mstatus->max_sp += diff*mstatus->int_;
		mstatus->hp = mstatus->max_hp;
		mstatus->sp = mstatus->max_sp;
		mstatus->speed -= cap_value(diff, 0, mstatus->speed - 10);
	}


	if (flag&2 && battle_config.mob_size_influence) {
		// change for sized monsters [Valaris]
		if (md->special_state.size==SZ_SMALL) {
			mstatus->max_hp>>=1;
			mstatus->max_sp>>=1;
			if (!mstatus->max_hp) mstatus->max_hp = 1;
			if (!mstatus->max_sp) mstatus->max_sp = 1;
			mstatus->hp=mstatus->max_hp;
			mstatus->sp=mstatus->max_sp;
			mstatus->str>>=1;
			mstatus->agi>>=1;
			mstatus->vit>>=1;
			mstatus->int_>>=1;
			mstatus->dex>>=1;
			mstatus->luk>>=1;
			if (!mstatus->str) mstatus->str = 1;
			if (!mstatus->agi) mstatus->agi = 1;
			if (!mstatus->vit) mstatus->vit = 1;
			if (!mstatus->int_) mstatus->int_ = 1;
			if (!mstatus->dex) mstatus->dex = 1;
			if (!mstatus->luk) mstatus->luk = 1;
		} else if (md->special_state.size==SZ_BIG) {
			mstatus->max_hp<<=1;
			mstatus->max_sp<<=1;
			mstatus->hp=mstatus->max_hp;
			mstatus->sp=mstatus->max_sp;
			mstatus->str<<=1;
			mstatus->agi<<=1;
			mstatus->vit<<=1;
			mstatus->int_<<=1;
			mstatus->dex<<=1;
			mstatus->luk<<=1;
		}
	}

	status->calc_misc(&md->bl, mstatus, md->level);

	if(flag&4)
	{	// Strengthen Guardians - custom value +10% / lv
		struct guild_castle *gc;
		gc=guild->mapname2gc(map->list[md->bl.m].name);
		if (!gc)
			ShowError("status_calc_mob: No castle set at map %s\n", map->list[md->bl.m].name);
		else
			if(gc->castle_id < 24 || md->class_ == MOBID_EMPERIUM) {
#ifdef RENEWAL
				mstatus->max_hp += 50 * gc->defense;
				mstatus->max_sp += 70 * gc->defense;
#else
				mstatus->max_hp += 1000 * gc->defense;
				mstatus->max_sp += 200 * gc->defense;
#endif
				mstatus->hp = mstatus->max_hp;
				mstatus->sp = mstatus->max_sp;
				mstatus->def += (gc->defense+2)/3;
				mstatus->mdef += (gc->defense+2)/3;
			}
			if(md->class_ != MOBID_EMPERIUM) {
				mstatus->batk += mstatus->batk * 10*md->guardian_data->guardup_lv/100;
				mstatus->rhw.atk += mstatus->rhw.atk * 10*md->guardian_data->guardup_lv/100;
				mstatus->rhw.atk2 += mstatus->rhw.atk2 * 10*md->guardian_data->guardup_lv/100;
				mstatus->aspd_rate -= 100*md->guardian_data->guardup_lv;
			}
	}

	if( opt&SCO_FIRST ) //Initial battle status
		memcpy(&md->status, mstatus, sizeof(struct status_data));

	return 1;
}

//Skotlex: Calculates the stats of the given pet.
int status_calc_pet_(struct pet_data *pd, enum e_status_calc_opt opt)
{
	nullpo_ret(pd);

	if (opt&SCO_FIRST) {
		memcpy(&pd->status, &pd->db->status, sizeof(struct status_data));
		pd->status.mode = MD_CANMOVE; // pets discard all modes, except walking
		pd->status.speed = pd->petDB->speed;

		if(battle_config.pet_attack_support || battle_config.pet_damage_support)
		{// attack support requires the pet to be able to attack
			pd->status.mode|= MD_CANATTACK;
		}
	}

	if (battle_config.pet_lv_rate && pd->msd) {
		struct map_session_data *sd = pd->msd;
		int lv;

		lv =sd->status.base_level*battle_config.pet_lv_rate/100;
		if (lv < 0)
			lv = 1;
		if (lv != pd->pet.level || opt&SCO_FIRST) {
			struct status_data *bstat = &pd->db->status, *pstatus = &pd->status;
			pd->pet.level = lv;
			if (! (opt&SCO_FIRST) ) //Lv Up animation
				clif->misceffect(&pd->bl, 0);
			pstatus->rhw.atk = (bstat->rhw.atk*lv)/pd->db->lv;
			pstatus->rhw.atk2 = (bstat->rhw.atk2*lv)/pd->db->lv;
			pstatus->str = (bstat->str*lv)/pd->db->lv;
			pstatus->agi = (bstat->agi*lv)/pd->db->lv;
			pstatus->vit = (bstat->vit*lv)/pd->db->lv;
			pstatus->int_ = (bstat->int_*lv)/pd->db->lv;
			pstatus->dex = (bstat->dex*lv)/pd->db->lv;
			pstatus->luk = (bstat->luk*lv)/pd->db->lv;

			pstatus->rhw.atk = cap_value(pstatus->rhw.atk, 1, battle_config.pet_max_atk1);
			pstatus->rhw.atk2 = cap_value(pstatus->rhw.atk2, 2, battle_config.pet_max_atk2);
			pstatus->str = cap_value(pstatus->str,1,battle_config.pet_max_stats);
			pstatus->agi = cap_value(pstatus->agi,1,battle_config.pet_max_stats);
			pstatus->vit = cap_value(pstatus->vit,1,battle_config.pet_max_stats);
			pstatus->int_= cap_value(pstatus->int_,1,battle_config.pet_max_stats);
			pstatus->dex = cap_value(pstatus->dex,1,battle_config.pet_max_stats);
			pstatus->luk = cap_value(pstatus->luk,1,battle_config.pet_max_stats);

			status->calc_misc(&pd->bl, &pd->status, lv);

			if (! (opt&SCO_FIRST) )	//Not done the first time because the pet is not visible yet
				clif->send_petstatus(sd);
		}
	} else if ( opt&SCO_FIRST ) {
		status->calc_misc(&pd->bl, &pd->status, pd->db->lv);
		if (!battle_config.pet_lv_rate && pd->pet.level != pd->db->lv)
			pd->pet.level = pd->db->lv;
	}

	//Support rate modifier (1000 = 100%)
	pd->rate_fix = 1000*(pd->pet.intimate - battle_config.pet_support_min_friendly)/(1000- battle_config.pet_support_min_friendly) +500;
	if(battle_config.pet_support_rate != 100)
		pd->rate_fix = pd->rate_fix*battle_config.pet_support_rate/100;

	return 1;
}

/// Helper function for status_base_pc_maxhp(), used to pre-calculate the hp_sigma_val[] array
void status_calc_sigma(void)
{
	int i,j;

	for(i = 0; i < CLASS_COUNT; i++)
	{
		unsigned int k = 0;
		status->hp_sigma_val[i][0] = status->hp_sigma_val[i][1] = 0;
		for(j = 2; j <= MAX_LEVEL; j++)
		{
			k += (status->hp_coefficient[i]*j + 50) / 100;
			status->hp_sigma_val[i][j] = k;
			if (k >= INT_MAX)
				break; //Overflow protection. [Skotlex]
		}
		for(; j <= MAX_LEVEL; j++)
			status->hp_sigma_val[i][j] = INT_MAX;
	}
}

/// Calculates base MaxHP value according to class and base level
/// The recursive equation used to calculate level bonus is (using integer operations)
///    f(0) = 35 | f(x+1) = f(x) + A + (x + B)*C/D
/// which reduces to something close to
///    f(x) = 35 + x*(A + B*C/D) + sum(i=2..x){ i*C/D }
unsigned int status_base_pc_maxhp(struct map_session_data *sd, struct status_data *st) {
	uint64 val = pc->class2idx(sd->status.class_);
	val = 35 + sd->status.base_level*(int64)status->hp_coefficient2[val]/100 + status->hp_sigma_val[val][sd->status.base_level];

	if((sd->class_&MAPID_UPPERMASK) == MAPID_NINJA || (sd->class_&MAPID_UPPERMASK) == MAPID_GUNSLINGER || (sd->class_&MAPID_UPPERMASK) == MAPID_REBELLION)
		val += 100; //Since their HP can't be approximated well enough without this.
	if((sd->class_&MAPID_UPPERMASK) == MAPID_TAEKWON && sd->status.base_level >= 90 && pc->famerank(sd->status.char_id, MAPID_TAEKWON))
		val *= 3; //Triple max HP for top ranking Taekwons over level 90.
	if((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->status.base_level >= 99)
		val += 2000; //Supernovice lvl99 hp bonus.

	val += val * st->vit/100; // +1% per each point of VIT

	if (sd->class_&JOBL_UPPER)
		val += val * 25/100; //Trans classes get a 25% hp bonus
	else if (sd->class_&JOBL_BABY)
		val -= val * 30/100; //Baby classes get a 30% hp penalty
	return (unsigned int)cap_value(val,0,UINT_MAX);
}

unsigned int status_base_pc_maxsp(struct map_session_data* sd, struct status_data *st) {
	uint64 val;

	val = 10 + sd->status.base_level*(int64)status->sp_coefficient[pc->class2idx(sd->status.class_)]/100;
	val += val * st->int_/100;

	if (sd->class_&JOBL_UPPER)
		val += val * 25/100;
	else if (sd->class_&JOBL_BABY)
		val -= val * 30/100;
	if ((sd->class_&MAPID_UPPERMASK) == MAPID_TAEKWON && sd->status.base_level >= 90 && pc->famerank(sd->status.char_id, MAPID_TAEKWON))
		val *= 3; //Triple max SP for top ranking Taekwons over level 90.

	return (unsigned int)cap_value(val,0,UINT_MAX);
}

//Calculates player data from scratch without counting SC adjustments.
//Should be invoked whenever players raise stats, learn passive skills or change equipment.
int status_calc_pc_(struct map_session_data* sd, enum e_status_calc_opt opt) {
	static int calculating = 0; //Check for recursive call preemption. [Skotlex]
	struct status_data *bstatus; // pointer to the player's base status
	const struct status_change *sc = &sd->sc;
	struct s_skill b_skill[MAX_SKILL]; // previous skill tree
	int b_weight, b_max_weight, b_cart_weight_max, // previous weight
		i, k, index, skill_lv,refinedef=0;
	int64 i64;

	if (++calculating > 10) //Too many recursive calls!
		return -1;

	// remember player-specific values that are currently being shown to the client (for refresh purposes)
	memcpy(b_skill, &sd->status.skill, sizeof(b_skill));
	b_weight = sd->weight;
	b_max_weight = sd->max_weight;
	b_cart_weight_max = sd->cart_weight_max;

	pc->calc_skilltree(sd);	// SkillTree calculation

	sd->max_weight = status->max_weight_base[pc->class2idx(sd->status.class_)]+sd->status.str*300;

	if(opt&SCO_FIRST) {
		//Load Hp/SP from char-received data.
		sd->battle_status.hp = sd->status.hp;
		sd->battle_status.sp = sd->status.sp;
		sd->regen.sregen = &sd->sregen;
		sd->regen.ssregen = &sd->ssregen;
		sd->weight=0;
		for(i=0;i<MAX_INVENTORY;i++){
			if(sd->status.inventory[i].nameid==0 || sd->inventory_data[i] == NULL)
				continue;
			sd->weight += sd->inventory_data[i]->weight*sd->status.inventory[i].amount;
		}
		sd->cart_weight=0;
		sd->cart_num=0;
		for(i=0;i<MAX_CART;i++){
			if(sd->status.cart[i].nameid==0)
				continue;
			sd->cart_weight+=itemdb_weight(sd->status.cart[i].nameid)*sd->status.cart[i].amount;
			sd->cart_num++;
		}
	}

	bstatus = &sd->base_status;
	// these are not zeroed. [zzo]
	sd->hprate=100;
	sd->sprate=100;
	sd->castrate=100;
	sd->delayrate=100;
	sd->dsprate=100;
	sd->hprecov_rate = 100;
	sd->sprecov_rate = 100;
	sd->matk_rate = 100;
	sd->critical_rate = sd->hit_rate = sd->flee_rate = sd->flee2_rate = 100;
	sd->def_rate = sd->def2_rate = sd->mdef_rate = sd->mdef2_rate = 100;
	sd->regen.state.block = 0;

	// zeroed arrays, order follows the order in pc.h.
	// add new arrays to the end of zeroed area in pc.h (see comments) and size here. [zzo]
	memset (sd->param_bonus, 0, sizeof(sd->param_bonus)
		+ sizeof(sd->param_equip)
		+ sizeof(sd->subele)
		+ sizeof(sd->subrace)
		+ sizeof(sd->subrace2)
		+ sizeof(sd->subsize)
		+ sizeof(sd->reseff)
		+ sizeof(sd->weapon_coma_ele)
		+ sizeof(sd->weapon_coma_race)
		+ sizeof(sd->weapon_atk)
		+ sizeof(sd->weapon_atk_rate)
		+ sizeof(sd->arrow_addele)
		+ sizeof(sd->arrow_addrace)
		+ sizeof(sd->arrow_addsize)
		+ sizeof(sd->magic_addele)
		+ sizeof(sd->magic_addrace)
		+ sizeof(sd->magic_addsize)
		+ sizeof(sd->magic_atk_ele)
		+ sizeof(sd->critaddrace)
		+ sizeof(sd->expaddrace)
		+ sizeof(sd->ignore_mdef)
		+ sizeof(sd->ignore_def)
		+ sizeof(sd->sp_gain_race)
		+ sizeof(sd->sp_gain_race_attack)
		+ sizeof(sd->hp_gain_race_attack)
		);

	memset (&sd->right_weapon.overrefine, 0, sizeof(sd->right_weapon) - sizeof(sd->right_weapon.atkmods));
	memset (&sd->left_weapon.overrefine, 0, sizeof(sd->left_weapon) - sizeof(sd->left_weapon.atkmods));

	if (sd->special_state.intravision && !sd->sc.data[SC_CLAIRVOYANCE]) //Clear intravision as long as nothing else is using it
		clif->sc_end(&sd->bl,sd->bl.id,SELF,SI_CLAIRVOYANCE);

	memset(&sd->special_state,0,sizeof(sd->special_state));
	
	if (!sd->state.permanent_speed) {
		memset(&bstatus->max_hp, 0, sizeof(struct status_data)-(sizeof(bstatus->hp)+sizeof(bstatus->sp)));
		bstatus->speed = DEFAULT_WALK_SPEED;
	} else {
		int pSpeed = bstatus->speed;
		memset(&bstatus->max_hp, 0, sizeof(struct status_data)-(sizeof(bstatus->hp)+sizeof(bstatus->sp)));
		bstatus->speed = pSpeed;
	}
	
	//FIXME: Most of these stuff should be calculated once, but how do I fix the memset above to do that? [Skotlex]
	//Give them all modes except these (useful for clones)
	bstatus->mode = MD_MASK&~(MD_BOSS|MD_PLANT|MD_DETECTOR|MD_ANGRY|MD_TARGETWEAK);

	bstatus->size = (sd->class_&JOBL_BABY)?SZ_MEDIUM:SZ_SMALL;
	if (battle_config.character_size && (pc_isriding(sd) || pc_isridingdragon(sd)) ) { //[Lupus]
		if (sd->class_&JOBL_BABY) {
			if (battle_config.character_size&SZ_BIG)
				bstatus->size++;
		} else
			if(battle_config.character_size&SZ_SMALL)
				bstatus->size++;
	}
	bstatus->aspd_rate = 1000;
	bstatus->ele_lv = 1;
	bstatus->race = RC_DEMIHUMAN;

	//zero up structures...
	memset(&sd->autospell,0,sizeof(sd->autospell)
		+ sizeof(sd->autospell2)
		+ sizeof(sd->autospell3)
		+ sizeof(sd->addeff)
		+ sizeof(sd->addeff2)
		+ sizeof(sd->addeff3)
		+ sizeof(sd->skillatk)
		+ sizeof(sd->skillusesprate)
		+ sizeof(sd->skillusesp)
		+ sizeof(sd->skillheal)
		+ sizeof(sd->skillheal2)
		+ sizeof(sd->hp_loss)
		+ sizeof(sd->sp_loss)
		+ sizeof(sd->hp_regen)
		+ sizeof(sd->sp_regen)
		+ sizeof(sd->skillblown)
		+ sizeof(sd->skillcast)
		+ sizeof(sd->add_def)
		+ sizeof(sd->add_mdef)
		+ sizeof(sd->add_mdmg)
		+ sizeof(sd->add_drop)
		+ sizeof(sd->itemhealrate)
		+ sizeof(sd->subele2)
		+ sizeof(sd->skillcooldown)
		+ sizeof(sd->skillfixcast)
		+ sizeof(sd->skillvarcast)
		+ sizeof(sd->skillfixcastrate)
		);

	memset (&sd->bonus, 0,sizeof(sd->bonus));

	// Autobonus
	pc->delautobonus(sd,sd->autobonus,ARRAYLENGTH(sd->autobonus),true);
	pc->delautobonus(sd,sd->autobonus2,ARRAYLENGTH(sd->autobonus2),true);
	pc->delautobonus(sd,sd->autobonus3,ARRAYLENGTH(sd->autobonus3),true);

	// Parse equipment.
	for(i=0;i<EQI_MAX;i++) {
		status->current_equip_item_index = index = sd->equip_index[i]; //We pass INDEX to status->current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
		if(index < 0)
			continue;
		if(i == EQI_AMMO) continue;/* ammo has special handler down there */
		if(i == EQI_HAND_R && sd->equip_index[EQI_HAND_L] == index)
			continue;
		if(i == EQI_HEAD_MID && sd->equip_index[EQI_HEAD_LOW] == index)
			continue;
		if(i == EQI_HEAD_TOP && (sd->equip_index[EQI_HEAD_MID] == index || sd->equip_index[EQI_HEAD_LOW] == index))
			continue;
		if(i == EQI_COSTUME_MID && sd->equip_index[EQI_COSTUME_LOW] == index)
			continue;
		if(i == EQI_COSTUME_TOP && (sd->equip_index[EQI_COSTUME_MID] == index || sd->equip_index[EQI_COSTUME_LOW] == index))
			continue;
		if(!sd->inventory_data[index])
			continue;

		for(k = 0; k < map->list[sd->bl.m].zone->disabled_items_count; k++) {
			if( map->list[sd->bl.m].zone->disabled_items[k] == sd->inventory_data[index]->nameid ) {
				break;
			}
		}

		if( k < map->list[sd->bl.m].zone->disabled_items_count )
			continue;

		bstatus->def += sd->inventory_data[index]->def;

		if(opt&SCO_FIRST && sd->inventory_data[index]->equip_script)
		{	//Execute equip-script on login
			script->run(sd->inventory_data[index]->equip_script,0,sd->bl.id,0);
			if (!calculating)
				return 1;
		}

		// sanitize the refine level in case someone decreased the value inbetween
		if (sd->status.inventory[index].refine > MAX_REFINE)
			sd->status.inventory[index].refine = MAX_REFINE;

		if(sd->inventory_data[index]->type == IT_WEAPON) {
			int r,wlv = sd->inventory_data[index]->wlv;
			struct weapon_data *wd;
			struct weapon_atk *wa;
			if (wlv >= REFINE_TYPE_MAX)
				wlv = REFINE_TYPE_MAX - 1;
			if(i == EQI_HAND_L && sd->status.inventory[index].equip == EQP_HAND_L) {
				wd = &sd->left_weapon; // Left-hand weapon
				wa = &bstatus->lhw;
			} else {
				wd = &sd->right_weapon;
				wa = &bstatus->rhw;
			}
			wa->atk += sd->inventory_data[index]->atk;
			if ( (r = sd->status.inventory[index].refine) )
				wa->atk2 = status->refine_info[wlv].bonus[r-1] / 100;

#ifdef RENEWAL
			wa->matk += sd->inventory_data[index]->matk;
			wa->wlv = wlv;
			if( r && sd->weapontype1 != W_BOW ) // renewal magic attack refine bonus
				wa->matk += status->refine_info[wlv].bonus[r-1] / 100;
#endif

			//Overrefine bonus.
			if (r)
				wd->overrefine = status->refine_info[wlv].randombonus_max[r-1] / 100;

			wa->range += sd->inventory_data[index]->range;
			if(sd->inventory_data[index]->script) {
				if (wd == &sd->left_weapon) {
					sd->state.lr_flag = 1;
					script->run(sd->inventory_data[index]->script,0,sd->bl.id,0);
					sd->state.lr_flag = 0;
				} else
					script->run(sd->inventory_data[index]->script,0,sd->bl.id,0);
				if (!calculating) //Abort, script->run retriggered this. [Skotlex]
					return 1;
			}

			if(sd->status.inventory[index].card[0]==CARD0_FORGE)
			{	// Forged weapon
				wd->star += (sd->status.inventory[index].card[1]>>8);
				if(wd->star >= 15) wd->star = 40; // 3 Star Crumbs now give +40 dmg
				if(pc->famerank(MakeDWord(sd->status.inventory[index].card[2],sd->status.inventory[index].card[3]) ,MAPID_BLACKSMITH))
					wd->star += 10;

				if (!wa->ele) //Do not overwrite element from previous bonuses.
					wa->ele = (sd->status.inventory[index].card[1]&0x0f);
			}
		}
		else if(sd->inventory_data[index]->type == IT_ARMOR) {
			int r;
			if ( (r = sd->status.inventory[index].refine) )
				refinedef += status->refine_info[REFINE_TYPE_ARMOR].bonus[r-1];
			if(sd->inventory_data[index]->script) {
				if( i == EQI_HAND_L ) //Shield
					sd->state.lr_flag = 3;
				script->run(sd->inventory_data[index]->script,0,sd->bl.id,0);
				if( i == EQI_HAND_L ) //Shield
					sd->state.lr_flag = 0;
				if (!calculating) //Abort, script->run retriggered this. [Skotlex]
					return 1;
			}
		}
	}

	if(sd->equip_index[EQI_AMMO] >= 0){
		index = sd->equip_index[EQI_AMMO];
		if(sd->inventory_data[index]){		// Arrows
			sd->bonus.arrow_atk += sd->inventory_data[index]->atk;
			sd->state.lr_flag = 2;
			if( !itemdb_is_GNthrowable(sd->inventory_data[index]->nameid) ) //don't run scripts on throwable items
				script->run(sd->inventory_data[index]->script,0,sd->bl.id,0);
			sd->state.lr_flag = 0;
			if (!calculating) //Abort, script->run retriggered status_calc_pc. [Skotlex]
				return 1;
		}
	}

	/* we've got combos to process */
	for( i = 0; i < sd->combo_count; i++ ) {
		struct item_combo *combo = itemdb->id2combo(sd->combos[i].id);
		unsigned char j;
		
		/**
		 * ensure combo usage is allowed at this location
		 **/
		for(j = 0; j < combo->count; j++) {
			for(k = 0; k < map->list[sd->bl.m].zone->disabled_items_count; k++) {
				if( map->list[sd->bl.m].zone->disabled_items[k] == combo->nameid[j] ) {
					break;
				}
			}
			if( k != map->list[sd->bl.m].zone->disabled_items_count )
				break;
		}
		
		if( j != combo->count )
			continue;
		
		script->run(sd->combos[i].bonus,0,sd->bl.id,0);
		if (!calculating) //Abort, script->run retriggered this.
			return 1;
	}

	//Store equipment script bonuses
	memcpy(sd->param_equip,sd->param_bonus,sizeof(sd->param_equip));
	memset(sd->param_bonus, 0, sizeof(sd->param_bonus));

	bstatus->def += (refinedef+50)/100;

	//Parse Cards
	for(i=0;i<EQI_MAX;i++) {
		status->current_equip_item_index = index = sd->equip_index[i]; //We pass INDEX to status->current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
		if(index < 0)
			continue;
		if(i == EQI_AMMO) continue;/* ammo doesn't have cards */
		if(i == EQI_HAND_R && sd->equip_index[EQI_HAND_L] == index)
			continue;
		if(i == EQI_HEAD_MID && sd->equip_index[EQI_HEAD_LOW] == index)
			continue;
		if(i == EQI_HEAD_TOP && (sd->equip_index[EQI_HEAD_MID] == index || sd->equip_index[EQI_HEAD_LOW] == index))
			continue;

		if(sd->inventory_data[index]) {
			int j,c;
			struct item_data *data;

			//Card script execution.
			if(itemdb_isspecial(sd->status.inventory[index].card[0]))
				continue;
			for(j=0;j<MAX_SLOTS;j++) {
				// Uses MAX_SLOTS to support Soul Bound system [Inkfish]
				status->current_equip_card_id= c= sd->status.inventory[index].card[j];
				if(!c)
					continue;
				data = itemdb->exists(c);
				if(!data)
					continue;

				for(k = 0; k < map->list[sd->bl.m].zone->disabled_items_count; k++) {
					if( map->list[sd->bl.m].zone->disabled_items[k] == data->nameid ) {
						break;
					}
				}

				if( k < map->list[sd->bl.m].zone->disabled_items_count )
					continue;

				if(opt&SCO_FIRST && data->equip_script) {//Execute equip-script on login
					script->run(data->equip_script,0,sd->bl.id,0);
					if (!calculating)
						return 1;
				}

				if(!data->script)
					continue;

				if(i == EQI_HAND_L && sd->status.inventory[index].equip == EQP_HAND_L) { //Left hand status.
					sd->state.lr_flag = 1;
					script->run(data->script,0,sd->bl.id,0);
					sd->state.lr_flag = 0;
				} else
					script->run(data->script,0,sd->bl.id,0);
				if (!calculating) //Abort, script->run his function. [Skotlex]
					return 1;
			}
		}
	}

	if( sc->count && sc->data[SC_ITEMSCRIPT] ) {
		struct item_data *data = itemdb->exists(sc->data[SC_ITEMSCRIPT]->val1);
		if( data && data->script )
			script->run(data->script,0,sd->bl.id,0);
	}

	if( sd->pd ) { // Pet Bonus
		struct pet_data *pd = sd->pd;
		if( pd && pd->petDB && pd->petDB->equip_script && pd->pet.intimate >= battle_config.pet_equip_min_friendly )
			script->run(pd->petDB->equip_script,0,sd->bl.id,0);
		if( pd && pd->pet.intimate > 0 && (!battle_config.pet_equip_required || pd->pet.equip > 0) && pd->state.skillbonus == 1 && pd->bonus )
			pc->bonus(sd,pd->bonus->type, pd->bonus->val);
	}

	//param_bonus now holds card bonuses.
	if(bstatus->rhw.range < 1) bstatus->rhw.range = 1;
	if(bstatus->lhw.range < 1) bstatus->lhw.range = 1;
	if(bstatus->rhw.range < bstatus->lhw.range)
		bstatus->rhw.range = bstatus->lhw.range;

	sd->bonus.double_rate += sd->bonus.double_add_rate;
	sd->bonus.perfect_hit += sd->bonus.perfect_hit_add;
	sd->bonus.splash_range += sd->bonus.splash_add_range;

	// Damage modifiers from weapon type
	sd->right_weapon.atkmods[0] = status->atkmods[0][sd->weapontype1];
	sd->right_weapon.atkmods[1] = status->atkmods[1][sd->weapontype1];
	sd->right_weapon.atkmods[2] = status->atkmods[2][sd->weapontype1];
	sd->left_weapon.atkmods[0] = status->atkmods[0][sd->weapontype2];
	sd->left_weapon.atkmods[1] = status->atkmods[1][sd->weapontype2];
	sd->left_weapon.atkmods[2] = status->atkmods[2][sd->weapontype2];

	if( (pc_isriding(sd) || pc_isridingdragon(sd)) &&
		(sd->status.weapon==W_1HSPEAR || sd->status.weapon==W_2HSPEAR))
	{	//When Riding with spear, damage modifier to mid-class becomes
		//same as versus large size.
		sd->right_weapon.atkmods[1] = sd->right_weapon.atkmods[2];
		sd->left_weapon.atkmods[1] = sd->left_weapon.atkmods[2];
	}

	// ----- STATS CALCULATION -----

	// Job bonuses
	index = pc->class2idx(sd->status.class_);
	for(i=0;i<(int)sd->status.job_level && i<MAX_LEVEL;i++){
		if(!status->job_bonus[index][i])
			continue;
		switch(status->job_bonus[index][i]) {
			case 1: bstatus->str++; break;
			case 2: bstatus->agi++; break;
			case 3: bstatus->vit++; break;
			case 4: bstatus->int_++; break;
			case 5: bstatus->dex++; break;
			case 6: bstatus->luk++; break;
		}
	}

	// If a Super Novice has never died and is at least joblv 70, he gets all stats +10
	if((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->die_counter == 0 && sd->status.job_level >= 70) {
		bstatus->str += 10;
		bstatus->agi += 10;
		bstatus->vit += 10;
		bstatus->int_+= 10;
		bstatus->dex += 10;
		bstatus->luk += 10;
	}

	// Absolute modifiers from passive skills
	if(pc->checkskill(sd,BS_HILTBINDING)>0)
		bstatus->str++;
	if((skill_lv=pc->checkskill(sd,SA_DRAGONOLOGY))>0)
		bstatus->int_ += (skill_lv+1)/2; // +1 INT / 2 lv
	if((skill_lv=pc->checkskill(sd,AC_OWL))>0)
		bstatus->dex += skill_lv;
	if((skill_lv = pc->checkskill(sd,RA_RESEARCHTRAP))>0)
		bstatus->int_ += skill_lv;

	// Bonuses from cards and equipment as well as base stat, remember to avoid overflows.
	i = bstatus->str + sd->status.str + sd->param_bonus[0] + sd->param_equip[0];
	bstatus->str = cap_value(i,0,USHRT_MAX);
	i = bstatus->agi + sd->status.agi + sd->param_bonus[1] + sd->param_equip[1];
	bstatus->agi = cap_value(i,0,USHRT_MAX);
	i = bstatus->vit + sd->status.vit + sd->param_bonus[2] + sd->param_equip[2];
	bstatus->vit = cap_value(i,0,USHRT_MAX);
	i = bstatus->int_+ sd->status.int_+ sd->param_bonus[3] + sd->param_equip[3];
	bstatus->int_ = cap_value(i,0,USHRT_MAX);
	i = bstatus->dex + sd->status.dex + sd->param_bonus[4] + sd->param_equip[4];
	bstatus->dex = cap_value(i,0,USHRT_MAX);
	i = bstatus->luk + sd->status.luk + sd->param_bonus[5] + sd->param_equip[5];
	bstatus->luk = cap_value(i,0,USHRT_MAX);

	// ------ BASE ATTACK CALCULATION ------

	// Base batk value is set on status->calc_misc
	// weapon-type bonus (FIXME: Why is the weapon_atk bonus applied to base attack?)
	if (sd->status.weapon < MAX_WEAPON_TYPE && sd->weapon_atk[sd->status.weapon])
		bstatus->batk += sd->weapon_atk[sd->status.weapon];
	// Absolute modifiers from passive skills
#ifndef RENEWAL
	if((skill_lv=pc->checkskill(sd,BS_HILTBINDING))>0) // it doesn't work in RE.
		bstatus->batk += 4;
#endif

	// ----- HP MAX CALCULATION -----

	// Basic MaxHP value
	//We hold the standard Max HP here to make it faster to recalculate on vit changes.
	sd->status.max_hp = status->base_pc_maxhp(sd,bstatus);
	//This is done to handle underflows from negative Max HP bonuses
	i64 = sd->status.max_hp + (int)bstatus->max_hp;
	bstatus->max_hp = (unsigned int)cap_value(i64, 0, INT_MAX);

	// Absolute modifiers from passive skills
	if((skill_lv=pc->checkskill(sd,CR_TRUST))>0)
		bstatus->max_hp += skill_lv*200;

	// Apply relative modifiers from equipment
	if(sd->hprate < 0)
		sd->hprate = 0;
	if(sd->hprate!=100)
		bstatus->max_hp = APPLY_RATE(bstatus->max_hp, sd->hprate);
	if(battle_config.hp_rate != 100)
		bstatus->max_hp = APPLY_RATE(bstatus->max_hp, battle_config.hp_rate);

	if(bstatus->max_hp > (unsigned int)battle_config.max_hp)
		bstatus->max_hp = battle_config.max_hp;
	else if(!bstatus->max_hp)
		bstatus->max_hp = 1;

	// ----- SP MAX CALCULATION -----

	// Basic MaxSP value
	sd->status.max_sp = status->base_pc_maxsp(sd,bstatus);
	//This is done to handle underflows from negative Max SP bonuses
	i64 = sd->status.max_sp + (int)bstatus->max_sp;
	bstatus->max_sp = (unsigned int)cap_value(i64, 0, INT_MAX);

	// Absolute modifiers from passive skills
	if((skill_lv=pc->checkskill(sd,SL_KAINA))>0)
		bstatus->max_sp += 30*skill_lv;
	if((skill_lv=pc->checkskill(sd,HP_MEDITATIO))>0)
		bstatus->max_sp += (int64)bstatus->max_sp * skill_lv/100;
	if((skill_lv=pc->checkskill(sd,HW_SOULDRAIN))>0)
		bstatus->max_sp += (int64)bstatus->max_sp * 2*skill_lv/100;
	if( (skill_lv = pc->checkskill(sd,RA_RESEARCHTRAP)) > 0 )
		bstatus->max_sp += 200 + 20 * skill_lv;
	if( (skill_lv = pc->checkskill(sd,WM_LESSON)) > 0 )
		bstatus->max_sp += 30 * skill_lv;


	// Apply relative modifiers from equipment
	if(sd->sprate < 0)
		sd->sprate = 0;
	if(sd->sprate!=100)
		bstatus->max_sp = APPLY_RATE(bstatus->max_sp, sd->sprate);
	if(battle_config.sp_rate != 100)
		bstatus->max_sp = APPLY_RATE(bstatus->max_sp, battle_config.sp_rate);

	if(bstatus->max_sp > (unsigned int)battle_config.max_sp)
		bstatus->max_sp = battle_config.max_sp;
	else if(!bstatus->max_sp)
		bstatus->max_sp = 1;

	// ----- RESPAWN HP/SP -----
	//
	//Calc respawn hp and store it on base_status
	if (sd->special_state.restart_full_recover)
	{
		bstatus->hp = bstatus->max_hp;
		bstatus->sp = bstatus->max_sp;
	} else {
		if((sd->class_&MAPID_BASEMASK) == MAPID_NOVICE && !(sd->class_&JOBL_2)
			&& battle_config.restart_hp_rate < 50)
			bstatus->hp = bstatus->max_hp>>1;
		else
			bstatus->hp = APPLY_RATE(bstatus->max_hp, battle_config.restart_hp_rate);
		if(!bstatus->hp)
			bstatus->hp = 1;

		bstatus->sp = APPLY_RATE(bstatus->max_sp, battle_config.restart_sp_rate);

		if( !bstatus->sp ) /* the minimum for the respawn setting is SP:1 */
			bstatus->sp = 1;
	}

	// ----- MISC CALCULATION -----
	status->calc_misc(&sd->bl, bstatus, sd->status.base_level);

	//Equipment modifiers for misc settings
	if(sd->matk_rate < 0)
		sd->matk_rate = 0;

	if(sd->matk_rate != 100){
		bstatus->matk_max = bstatus->matk_max * sd->matk_rate/100;
		bstatus->matk_min = bstatus->matk_min * sd->matk_rate/100;
	}

	if(sd->hit_rate < 0)
		sd->hit_rate = 0;
	if(sd->hit_rate != 100)
		bstatus->hit = bstatus->hit * sd->hit_rate/100;

	if(sd->flee_rate < 0)
		sd->flee_rate = 0;
	if(sd->flee_rate != 100)
		bstatus->flee = bstatus->flee * sd->flee_rate/100;

	if(sd->def2_rate < 0)
		sd->def2_rate = 0;
	if(sd->def2_rate != 100)
		bstatus->def2 = bstatus->def2 * sd->def2_rate/100;

	if(sd->mdef2_rate < 0)
		sd->mdef2_rate = 0;
	if(sd->mdef2_rate != 100)
		bstatus->mdef2 = bstatus->mdef2 * sd->mdef2_rate/100;

	if(sd->critical_rate < 0)
		sd->critical_rate = 0;
	if(sd->critical_rate != 100)
		bstatus->cri = bstatus->cri * sd->critical_rate/100;

	if(sd->flee2_rate < 0)
		sd->flee2_rate = 0;
	if(sd->flee2_rate != 100)
		bstatus->flee2 = bstatus->flee2 * sd->flee2_rate/100;

	// ----- HIT CALCULATION -----

	// Absolute modifiers from passive skills
#ifndef RENEWAL
	if((skill_lv=pc->checkskill(sd,BS_WEAPONRESEARCH))>0) // is this correct in pre? there is already hitrate bonus in battle.c
		bstatus->hit += skill_lv*2;
#endif
	if((skill_lv=pc->checkskill(sd,AC_VULTURE))>0) {
#ifndef RENEWAL
		bstatus->hit += skill_lv;
#endif
		if(sd->status.weapon == W_BOW)
			bstatus->rhw.range += skill_lv;
	}
	if(sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE) {
		if((skill_lv=pc->checkskill(sd,GS_SINGLEACTION))>0)
			bstatus->hit += 2*skill_lv;
		if((skill_lv=pc->checkskill(sd,GS_SNAKEEYE))>0) {
			bstatus->hit += skill_lv;
			bstatus->rhw.range += skill_lv;
		}
	}
	if( (sd->status.weapon == W_1HAXE || sd->status.weapon == W_2HAXE) && (skill_lv = pc->checkskill(sd,NC_TRAININGAXE)) > 0 )
		bstatus->hit += 3*skill_lv;
	if((sd->status.weapon == W_MACE || sd->status.weapon == W_2HMACE) && ((skill_lv = pc->checkskill(sd,NC_TRAININGAXE)) > 0))
		bstatus->hit += 2*skill_lv;

	// ----- FLEE CALCULATION -----

	// Absolute modifiers from passive skills
	if((skill_lv=pc->checkskill(sd,TF_MISS))>0)
		bstatus->flee += skill_lv*(sd->class_&JOBL_2 && (sd->class_&MAPID_BASEMASK) == MAPID_THIEF? 4 : 3);
	if((skill_lv=pc->checkskill(sd,MO_DODGE))>0)
		bstatus->flee += (skill_lv*3)>>1;
	// ----- EQUIPMENT-DEF CALCULATION -----

	// Apply relative modifiers from equipment
	if(sd->def_rate < 0)
		sd->def_rate = 0;
	if(sd->def_rate != 100) {
		i =  bstatus->def * sd->def_rate/100;
		bstatus->def = cap_value(i, DEFTYPE_MIN, DEFTYPE_MAX);
	}

	if( pc_ismadogear(sd) && (skill_lv = pc->checkskill(sd,NC_MAINFRAME)) > 0 )
		bstatus->def += 20 + 20 * skill_lv;

#ifndef RENEWAL
	if (!battle_config.weapon_defense_type && bstatus->def > battle_config.max_def) {
		bstatus->def2 += battle_config.over_def_bonus*(bstatus->def -battle_config.max_def);
		bstatus->def = (unsigned char)battle_config.max_def;
	}
#endif

	// ----- EQUIPMENT-MDEF CALCULATION -----

	// Apply relative modifiers from equipment
	if(sd->mdef_rate < 0)
		sd->mdef_rate = 0;
	if(sd->mdef_rate != 100) {
		i =  bstatus->mdef * sd->mdef_rate/100;
		bstatus->mdef = cap_value(i, DEFTYPE_MIN, DEFTYPE_MAX);
	}

#ifndef RENEWAL
	if (!battle_config.magic_defense_type && bstatus->mdef > battle_config.max_def) {
		bstatus->mdef2 += battle_config.over_def_bonus*(bstatus->mdef -battle_config.max_def);
		bstatus->mdef = (signed char)battle_config.max_def;
	}
#endif

	// ----- ASPD CALCULATION -----
	// Unlike other stats, ASPD rate modifiers from skills/SCs/items/etc are first all added together, then the final modifier is applied

	// Basic ASPD value
	i = status->base_amotion_pc(sd,bstatus);
	bstatus->amotion = cap_value(i,((sd->class_&JOBL_THIRD) ? battle_config.max_third_aspd : battle_config.max_aspd),2000);

	// Relative modifiers from passive skills
#ifndef RENEWAL_ASPD
	if((skill_lv=pc->checkskill(sd,SA_ADVANCEDBOOK))>0 && sd->status.weapon == W_BOOK)
		bstatus->aspd_rate -= 5*skill_lv;
	if((skill_lv = pc->checkskill(sd,SG_DEVIL)) > 0 && !pc->nextjobexp(sd))
		bstatus->aspd_rate -= 30*skill_lv;
	if((skill_lv=pc->checkskill(sd,GS_SINGLEACTION))>0 &&
		(sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE))
		bstatus->aspd_rate -= ((skill_lv+1)/2) * 10;
	if(pc_isriding(sd))
		bstatus->aspd_rate += 500-100*pc->checkskill(sd,KN_CAVALIERMASTERY);
	else if(pc_isridingdragon(sd))
		bstatus->aspd_rate += 250-50*pc->checkskill(sd,RK_DRAGONTRAINING);
#else // needs more info
	if((skill_lv=pc->checkskill(sd,SA_ADVANCEDBOOK))>0 && sd->status.weapon == W_BOOK)
		bstatus->aspd_rate += 5*skill_lv;
	if((skill_lv = pc->checkskill(sd,SG_DEVIL)) > 0 && !pc->nextjobexp(sd))
		bstatus->aspd_rate += 30*skill_lv;
	if((skill_lv=pc->checkskill(sd,GS_SINGLEACTION))>0 &&
		(sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE))
		bstatus->aspd_rate += ((skill_lv+1)/2) * 10;
	if(pc_isriding(sd))
		bstatus->aspd_rate -= 500-100*pc->checkskill(sd,KN_CAVALIERMASTERY);
	else if(pc_isridingdragon(sd))
		bstatus->aspd_rate -= 250-50*pc->checkskill(sd,RK_DRAGONTRAINING);
#endif
	bstatus->adelay = 2*bstatus->amotion;


	// ----- DMOTION -----
	//
	i =  800-bstatus->agi*4;
	bstatus->dmotion = cap_value(i, 400, 800);
	if(battle_config.pc_damage_delay_rate != 100)
		bstatus->dmotion = bstatus->dmotion*battle_config.pc_damage_delay_rate/100;

	// ----- MISC CALCULATIONS -----

	// Weight
	if((skill_lv=pc->checkskill(sd,MC_INCCARRY))>0)
		sd->max_weight += 2000*skill_lv;
	if(pc_isriding(sd) && pc->checkskill(sd,KN_RIDING)>0)
		sd->max_weight += 10000;
	else if(pc_isridingdragon(sd))
		sd->max_weight += 5000+2000*pc->checkskill(sd,RK_DRAGONTRAINING);
	if(sc->data[SC_KNOWLEDGE])
		sd->max_weight += sd->max_weight*sc->data[SC_KNOWLEDGE]->val1/10;
	if((skill_lv=pc->checkskill(sd,ALL_INCCARRY))>0)
		sd->max_weight += 2000*skill_lv;

	sd->cart_weight_max = battle_config.max_cart_weight + (pc->checkskill(sd, GN_REMODELING_CART)*5000);

	if (pc->checkskill(sd,SM_MOVINGRECOVERY)>0)
		sd->regen.state.walk = 1;
	else
		sd->regen.state.walk = 0;

	// Skill SP cost
	if((skill_lv=pc->checkskill(sd,HP_MANARECHARGE))>0 )
		sd->dsprate -= 4*skill_lv;

	if(sc->data[SC_SERVICEFORYOU])
		sd->dsprate -= sc->data[SC_SERVICEFORYOU]->val3;

	if(sc->data[SC_ATKER_BLOOD])
		sd->dsprate -= sc->data[SC_ATKER_BLOOD]->val1;

	//Underflow protections.
	if(sd->dsprate < 0)
		sd->dsprate = 0;
	if(sd->castrate < 0)
		sd->castrate = 0;
	if(sd->delayrate < 0)
		sd->delayrate = 0;
	if(sd->hprecov_rate < 0)
		sd->hprecov_rate = 0;
	if(sd->sprecov_rate < 0)
		sd->sprecov_rate = 0;

	// Anti-element and anti-race
	if((skill_lv=pc->checkskill(sd,CR_TRUST))>0)
		sd->subele[ELE_HOLY] += skill_lv*5;
	if((skill_lv=pc->checkskill(sd,BS_SKINTEMPER))>0) {
		sd->subele[ELE_NEUTRAL] += skill_lv;
		sd->subele[ELE_FIRE] += skill_lv*4;
	}
	if((skill_lv=pc->checkskill(sd,NC_RESEARCHFE))>0) {
		sd->subele[ELE_EARTH] += skill_lv*10;
		sd->subele[ELE_FIRE] += skill_lv*10;
	}
	if((skill_lv=pc->checkskill(sd,SA_DRAGONOLOGY))>0 ) {
#ifdef RENEWAL
		skill_lv = skill_lv*2;
#else
		skill_lv = skill_lv*4;
#endif
		sd->right_weapon.addrace[RC_DRAGON]+=skill_lv;
		sd->left_weapon.addrace[RC_DRAGON]+=skill_lv;
		sd->magic_addrace[RC_DRAGON]+=skill_lv;
		sd->subrace[RC_DRAGON]+=skill_lv;
	}

	if( (skill_lv = pc->checkskill(sd, AB_EUCHARISTICA)) > 0 ) {
		sd->right_weapon.addrace[RC_DEMON] += skill_lv;
		sd->right_weapon.addele[ELE_DARK] += skill_lv;
		sd->left_weapon.addrace[RC_DEMON] += skill_lv;
		sd->left_weapon.addele[ELE_DARK] += skill_lv;
		sd->magic_addrace[RC_DEMON] += skill_lv;
		sd->magic_addele[ELE_DARK] += skill_lv;
		sd->subrace[RC_DEMON] += skill_lv;
		sd->subele[ELE_DARK] += skill_lv;
	}

	if(sc->count) {
		if(sc->data[SC_CONCENTRATION]) { //Update the card-bonus data
			sc->data[SC_CONCENTRATION]->val3 = sd->param_bonus[1]; //Agi
			sc->data[SC_CONCENTRATION]->val4 = sd->param_bonus[4]; //Dex
		}
		if(sc->data[SC_SIEGFRIED]){
			i = sc->data[SC_SIEGFRIED]->val2;
			sd->subele[ELE_WATER] += i;
			sd->subele[ELE_EARTH] += i;
			sd->subele[ELE_FIRE] += i;
			sd->subele[ELE_WIND] += i;
			sd->subele[ELE_POISON] += i;
			sd->subele[ELE_HOLY] += i;
			sd->subele[ELE_DARK] += i;
			sd->subele[ELE_GHOST] += i;
			sd->subele[ELE_UNDEAD] += i;
		}
		if(sc->data[SC_PROVIDENCE]){
			sd->subele[ELE_HOLY] += sc->data[SC_PROVIDENCE]->val2;
			sd->subrace[RC_DEMON] += sc->data[SC_PROVIDENCE]->val2;
		}
		if(sc->data[SC_ARMORPROPERTY]) {	//This status change should grant card-type elemental resist.
			sd->subele[ELE_WATER] += sc->data[SC_ARMORPROPERTY]->val1;
			sd->subele[ELE_EARTH] += sc->data[SC_ARMORPROPERTY]->val2;
			sd->subele[ELE_FIRE] += sc->data[SC_ARMORPROPERTY]->val3;
			sd->subele[ELE_WIND] += sc->data[SC_ARMORPROPERTY]->val4;
		}
		if(sc->data[SC_ARMOR_RESIST]) { // Undead Scroll
			sd->subele[ELE_WATER] += sc->data[SC_ARMOR_RESIST]->val1;
			sd->subele[ELE_EARTH] += sc->data[SC_ARMOR_RESIST]->val2;
			sd->subele[ELE_FIRE] += sc->data[SC_ARMOR_RESIST]->val3;
			sd->subele[ELE_WIND] += sc->data[SC_ARMOR_RESIST]->val4;
		}
		if( sc->data[SC_FIRE_CLOAK_OPTION] ) {
			i = sc->data[SC_FIRE_CLOAK_OPTION]->val2;
			sd->subele[ELE_FIRE] += i;
			sd->subele[ELE_WATER] -= i;
		}
		if( sc->data[SC_WATER_DROP_OPTION] ) {
			i = sc->data[SC_WATER_DROP_OPTION]->val2;
			sd->subele[ELE_WATER] += i;
			sd->subele[ELE_WIND] -= i;
		}
		if( sc->data[SC_WIND_CURTAIN_OPTION] ) {
			i = sc->data[SC_WIND_CURTAIN_OPTION]->val2;
			sd->subele[ELE_WIND] += i;
			sd->subele[ELE_EARTH] -= i;
		}
		if( sc->data[SC_STONE_SHIELD_OPTION] ) {
			i = sc->data[SC_STONE_SHIELD_OPTION]->val2;
			sd->subele[ELE_EARTH] += i;
			sd->subele[ELE_FIRE] -= i;
		}
		if( sc->data[SC_MTF_MLEATKED] )
			sd->subele[ELE_NEUTRAL] += 2;
		if( sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 3 )
			sd->magic_addele[ELE_FIRE] += 25;
		if( sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 3 )
			sd->magic_addele[ELE_WATER] += 25;
		if( sc->data[SC_WIND_INSIGNIA] && sc->data[SC_WIND_INSIGNIA]->val1 == 3 )
			sd->magic_addele[ELE_WIND] += 25;
		if( sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 3 )
			sd->magic_addele[ELE_EARTH] += 25;
	}
	status_cpy(&sd->battle_status, bstatus);

	// ----- CLIENT-SIDE REFRESH -----
	if(!sd->bl.prev) {
		//Will update on LoadEndAck
		calculating = 0;
		return 0;
	}
	if(memcmp(b_skill,sd->status.skill,sizeof(sd->status.skill)))
		clif->skillinfoblock(sd);
	if(b_weight != sd->weight)
		clif->updatestatus(sd,SP_WEIGHT);
	if(b_max_weight != sd->max_weight) {
		clif->updatestatus(sd,SP_MAXWEIGHT);
		pc->updateweightstatus(sd);
	}
	if( b_cart_weight_max != sd->cart_weight_max ) {
		clif->updatestatus(sd,SP_CARTINFO);
	}

	calculating = 0;

	return 0;
}

int status_calc_mercenary_(struct mercenary_data *md, enum e_status_calc_opt opt) {
	struct status_data *mstatus = &md->base_status;
	struct s_mercenary *merc = &md->mercenary;

	if( opt&SCO_FIRST ) {
		memcpy(mstatus, &md->db->status, sizeof(struct status_data));
		mstatus->mode = MD_CANMOVE|MD_CANATTACK;
		mstatus->hp = mstatus->max_hp;
		mstatus->sp = mstatus->max_sp;
		md->battle_status.hp = merc->hp;
		md->battle_status.sp = merc->sp;
	}

	status->calc_misc(&md->bl, mstatus, md->db->lv);
	status_cpy(&md->battle_status, mstatus);

	return 0;
}

int status_calc_homunculus_(struct homun_data *hd, enum e_status_calc_opt opt) {
	struct status_data *hstatus = &hd->base_status;
	struct s_homunculus *hom = &hd->homunculus;
	int skill_lv;
	int amotion;

	hstatus->str = hom->str / 10;
	hstatus->agi = hom->agi / 10;
	hstatus->vit = hom->vit / 10;
	hstatus->dex = hom->dex / 10;
	hstatus->int_ = hom->int_ / 10;
	hstatus->luk = hom->luk / 10;

	if ( opt&SCO_FIRST ) { //[orn]
		const struct s_homunculus_db *db = hd->homunculusDB;
		hstatus->def_ele =  db->element;
		hstatus->ele_lv = 1;
		hstatus->race = db->race;
		hstatus->size = (hom->class_ == db->evo_class)?db->evo_size:db->base_size;
		hstatus->rhw.range = 1 + hstatus->size;
		hstatus->mode = MD_CANMOVE|MD_CANATTACK;
		hstatus->speed = DEFAULT_WALK_SPEED;
		if (battle_config.hom_setting&0x8 && hd->master)
			hstatus->speed = status->get_speed(&hd->master->bl);

		hstatus->hp = 1;
		hstatus->sp = 1;
	}
	skill_lv = hom->level/10 + hstatus->vit/5;
	hstatus->def = cap_value(skill_lv, 0, 99);

	skill_lv = hom->level/10 + hstatus->int_/5;
	hstatus->mdef = cap_value(skill_lv, 0, 99);

	hstatus->max_hp = hom->max_hp;
	hstatus->max_sp = hom->max_sp;

	homun->calc_skilltree(hd, 0);

	if((skill_lv=homun->checkskill(hd,HAMI_SKIN)) > 0)
		hstatus->def +=	skill_lv * 4;

	if((skill_lv = homun->checkskill(hd,HVAN_INSTRUCT)) > 0) {
		hstatus->int_ += 1 +skill_lv/2 +skill_lv/4 +skill_lv/5;
		hstatus->str  += 1 +skill_lv/3 +skill_lv/3 +skill_lv/4;
	}

	if((skill_lv=homun->checkskill(hd,HAMI_SKIN)) > 0)
		hstatus->max_hp += skill_lv * 2 * hstatus->max_hp / 100;

	if((skill_lv = homun->checkskill(hd,HLIF_BRAIN)) > 0)
		hstatus->max_sp += (1 +skill_lv/2 -skill_lv/4 +skill_lv/5) * hstatus->max_sp / 100;

	if ( opt&SCO_FIRST ) {
		hd->battle_status.hp = hom->hp;
		hd->battle_status.sp = hom->sp;
	}

	hstatus->rhw.atk = hstatus->dex;
	hstatus->rhw.atk2 = hstatus->str + hom->level;

	hstatus->aspd_rate = 1000;

	amotion = (1000 -4*hstatus->agi -hstatus->dex) * hd->homunculusDB->baseASPD/1000;
	hstatus->amotion = cap_value(amotion,battle_config.max_aspd,2000);
	hstatus->adelay = hstatus->amotion; //It seems adelay = amotion for Homunculus.

	status->calc_misc(&hd->bl, hstatus, hom->level);

#ifdef RENEWAL
	hstatus->matk_max = hstatus->matk_min;
#endif

	status_cpy(&hd->battle_status, hstatus);
	return 1;
}

int status_calc_elemental_(struct elemental_data *ed, enum e_status_calc_opt opt) {
	struct status_data *estatus = &ed->base_status;
	struct s_elemental *ele = &ed->elemental;
	struct map_session_data *sd = ed->master;

	if( !sd )
		return 0;

	if( opt&SCO_FIRST ) {
		memcpy(estatus, &ed->db->status, sizeof(struct status_data));
		if( !ele->mode )
			estatus->mode = EL_MODE_PASSIVE;
		else
			estatus->mode = ele->mode;

		status->calc_misc(&ed->bl, estatus, 0);

		estatus->max_hp = ele->max_hp;
		estatus->max_sp = ele->max_sp;
		estatus->hp = ele->hp;
		estatus->sp = ele->sp;
		estatus->rhw.atk = ele->atk;
		estatus->rhw.atk2 = ele->atk2;

		estatus->matk_min += ele->matk;
		estatus->def += ele->def;
		estatus->mdef += ele->mdef;
		estatus->flee = ele->flee;
		estatus->hit = ele->hit;

		memcpy(&ed->battle_status,estatus,sizeof(struct status_data));
	} else {
		status->calc_misc(&ed->bl, estatus, 0);
		status_cpy(&ed->battle_status, estatus);
	}

	return 0;
}

int status_calc_npc_(struct npc_data *nd, enum e_status_calc_opt opt) {
	struct status_data *nstatus = &nd->status;

	if (!nd)
		return 0;

	if ( opt&SCO_FIRST ) {
		nstatus->hp = 1;
		nstatus->sp = 1;
		nstatus->max_hp = 1;
		nstatus->max_sp = 1;

		nstatus->def_ele = ELE_NEUTRAL;
		nstatus->ele_lv = 1;
		nstatus->race = RC_DEMIHUMAN;
		nstatus->size = nd->size;
		nstatus->rhw.range = 1 + nstatus->size;
		nstatus->mode = (MD_CANMOVE|MD_CANATTACK);
		nstatus->speed = nd->speed;
	}

	nstatus->str = nd->stat_point;
	nstatus->agi = nd->stat_point;
	nstatus->vit = nd->stat_point;
	nstatus->int_= nd->stat_point;
	nstatus->dex = nd->stat_point;
	nstatus->luk = nd->stat_point;

	status->calc_misc(&nd->bl, nstatus, nd->level);
	status_cpy(&nd->status, nstatus);

	return 0;
}


//Calculates base regen values.
void status_calc_regen(struct block_list *bl, struct status_data *st, struct regen_data *regen) {
	struct map_session_data *sd;
	int val, skill_lv, reg_flag;

	if( !(bl->type&BL_REGEN) || !regen )
		return;

	sd = BL_CAST(BL_PC,bl);
	val = 1 + (st->vit/5) + (st->max_hp/200);

	if( sd && sd->hprecov_rate != 100 )
		val = val*sd->hprecov_rate/100;

	reg_flag = bl->type == BL_PC ? 0 : 1;

	regen->hp = cap_value(val, reg_flag, SHRT_MAX);

	val = 1 + (st->int_/6) + (st->max_sp/100);
	if( st->int_ >= 120 )
		val += ((st->int_-120)>>1) + 4;

	if( sd && sd->sprecov_rate != 100 )
		val = val*sd->sprecov_rate/100;

	regen->sp = cap_value(val, reg_flag, SHRT_MAX);

	if( sd ) {
		struct regen_data_sub *sregen;
		if( (skill_lv=pc->checkskill(sd,HP_MEDITATIO)) > 0 ) {
			val = regen->sp*(100+3*skill_lv)/100;
			regen->sp = cap_value(val, 1, SHRT_MAX);
		}
		//Only players have skill/sitting skill regen for now.
		sregen = regen->sregen;

		val = 0;
		if( (skill_lv=pc->checkskill(sd,SM_RECOVERY)) > 0 )
			val += skill_lv*5 + skill_lv*st->max_hp/500;
		sregen->hp = cap_value(val, 0, SHRT_MAX);

		val = 0;
		if( (skill_lv=pc->checkskill(sd,MG_SRECOVERY)) > 0 )
			val += skill_lv*3 + skill_lv*st->max_sp/500;
		if( (skill_lv=pc->checkskill(sd,NJ_NINPOU)) > 0 )
			val += skill_lv*3 + skill_lv*st->max_sp/500;
		if( (skill_lv=pc->checkskill(sd,WM_LESSON)) > 0 )
			val += skill_lv*3 + skill_lv*st->max_sp/500;

		sregen->sp = cap_value(val, 0, SHRT_MAX);

		// Skill-related recovery (only when sit)
		sregen = regen->ssregen;

		val = 0;
		if( (skill_lv=pc->checkskill(sd,MO_SPIRITSRECOVERY)) > 0 )
			val += skill_lv*4 + skill_lv*st->max_hp/500;

		if( (skill_lv=pc->checkskill(sd,TK_HPTIME)) > 0 && sd->state.rest )
			val += skill_lv*30 + skill_lv*st->max_hp/500;
		sregen->hp = cap_value(val, 0, SHRT_MAX);

		val = 0;
		if( (skill_lv=pc->checkskill(sd,TK_SPTIME)) > 0 && sd->state.rest ) {
			val += skill_lv*3 + skill_lv*st->max_sp/500;
			if ((skill_lv=pc->checkskill(sd,SL_KAINA)) > 0) //Power up Enjoyable Rest
				val += (30+10*skill_lv)*val/100;
		}
		if( (skill_lv=pc->checkskill(sd,MO_SPIRITSRECOVERY)) > 0 )
			val += skill_lv*2 + skill_lv*st->max_sp/500;
		sregen->sp = cap_value(val, 0, SHRT_MAX);
	}

	if( bl->type == BL_HOM ) {
		struct homun_data *hd = (TBL_HOM*)bl;
		if( (skill_lv = homun->checkskill(hd,HAMI_SKIN)) > 0 ) {
			val = regen->hp*(100+5*skill_lv)/100;
			regen->hp = cap_value(val, 1, SHRT_MAX);
		}
		if( (skill_lv = homun->checkskill(hd,HLIF_BRAIN)) > 0 ) {
			val = regen->sp*(100+3*skill_lv)/100;
			regen->sp = cap_value(val, 1, SHRT_MAX);
		}
	} else if( bl->type == BL_MER ) {
		val = (st->max_hp * st->vit / 10000 + 1) * 6;
		regen->hp = cap_value(val, 1, SHRT_MAX);

		val = (st->max_sp * (st->int_ + 10) / 750) + 1;
		regen->sp = cap_value(val, 1, SHRT_MAX);
	} else if( bl->type == BL_ELEM ) {
		val = (st->max_hp * st->vit / 10000 + 1) * 6;
		regen->hp = cap_value(val, 1, SHRT_MAX);

		val = (st->max_sp * (st->int_ + 10) / 750) + 1;
		regen->sp = cap_value(val, 1, SHRT_MAX);
	}
}

//Calculates SC related regen rates.
void status_calc_regen_rate(struct block_list *bl, struct regen_data *regen, struct status_change *sc)
{
	if (!(bl->type&BL_REGEN) || !regen)
		return;

	regen->flag = RGN_HP|RGN_SP;
	if(regen->sregen)
	{
		if (regen->sregen->hp)
			regen->flag|=RGN_SHP;

		if (regen->sregen->sp)
			regen->flag|=RGN_SSP;
		regen->sregen->rate.hp = regen->sregen->rate.sp = 1;
	}
	if (regen->ssregen)
	{
		if (regen->ssregen->hp)
			regen->flag|=RGN_SHP;

		if (regen->ssregen->sp)
			regen->flag|=RGN_SSP;
		regen->ssregen->rate.hp = regen->ssregen->rate.sp = 1;
	}
	regen->rate.hp = regen->rate.sp = 1;

	if (!sc || !sc->count)
		return;

	if ((sc->data[SC_POISON] && !sc->data[SC_SLOWPOISON])
	 || (sc->data[SC_DPOISON] && !sc->data[SC_SLOWPOISON])
	 || sc->data[SC_BERSERK]
	 || sc->data[SC_TRICKDEAD]
	 || sc->data[SC_BLOODING]
	 || sc->data[SC_MAGICMUSHROOM]
	 || sc->data[SC_RAISINGDRAGON]
	 || sc->data[SC_SATURDAY_NIGHT_FEVER]
	)	//No regen
		regen->flag = 0;

	if ( sc->data[SC_DANCING] || sc->data[SC_OBLIVIONCURSE] || sc->data[SC_MAXIMIZEPOWER] || sc->data[SC_REBOUND]
	   || ( bl->type == BL_PC && (((TBL_PC*)bl)->class_&MAPID_UPPERMASK) == MAPID_MONK
	      && (sc->data[SC_EXTREMITYFIST] || (sc->data[SC_EXPLOSIONSPIRITS] && (!sc->data[SC_SOULLINK] || sc->data[SC_SOULLINK]->val2 != SL_MONK)))
	      )
	) {
		regen->flag &=~RGN_SP; //No natural SP regen
	}

	if (sc->data[SC_TENSIONRELAX]) {
		regen->rate.hp += 2;
		if (regen->sregen)
			regen->sregen->rate.hp += 3;
	}
	if (sc->data[SC_MAGNIFICAT]) {
		regen->rate.hp += 1;
		regen->rate.sp += 1;
	}
	if (sc->data[SC_GDSKILL_REGENERATION]) {
		const struct status_change_entry *sce = sc->data[SC_GDSKILL_REGENERATION];
		if (!sce->val4) {
			regen->rate.hp += sce->val2;
			regen->rate.sp += sce->val3;
		} else
			regen->flag&=~sce->val4; //Remove regen as specified by val4
	}
	if(sc->data[SC_GENTLETOUCH_REVITALIZE]) {
		regen->hp = cap_value(regen->hp*sc->data[SC_GENTLETOUCH_REVITALIZE]->val3/100, 1, SHRT_MAX);
		regen->state.walk= 1;
	}
	if ((sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 1) //if insignia lvl 1
		|| (sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 1)
		|| (sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 1)
		|| (sc->data[SC_WIND_INSIGNIA] && sc->data[SC_WIND_INSIGNIA]->val1 == 1))
		regen->rate.hp *= 2;
	if( sc->data[SC_VITALITYACTIVATION] )
		regen->flag &=~RGN_SP;
	if(sc->data[SC_EXTRACT_WHITE_POTION_Z])
		regen->rate.hp += regen->rate.hp * sc->data[SC_EXTRACT_WHITE_POTION_Z]->val1/100;
	if(sc->data[SC_VITATA_500])
		regen->rate.sp += regen->rate.sp * sc->data[SC_VITATA_500]->val1/100;
}
/// Recalculates parts of an object's battle status according to the specified flags.
/// @param flag bitfield of values from enum scb_flag
void status_calc_bl_main(struct block_list *bl, /*enum scb_flag*/int flag) {
	const struct status_data *bst = status->get_base_status(bl);
	struct status_data *st = status->get_status_data(bl);
	struct status_change *sc = status->get_sc(bl);
	TBL_PC *sd = BL_CAST(BL_PC,bl);
	int temp;

	if (!bst || !st)
		return;

	if((!(bl->type&BL_REGEN)) && (!sc || !sc->count)) { //No difference.
		status_cpy(st, bst);
		return;
	}

	if(flag&SCB_STR) {
		st->str = status->calc_str(bl, sc, bst->str);
		flag|=SCB_BATK;
		if( bl->type&BL_HOM )
			flag |= SCB_WATK;
	}

	if(flag&SCB_AGI) {
		st->agi = status->calc_agi(bl, sc, bst->agi);
		flag|=SCB_FLEE
#ifdef RENEWAL
			|SCB_DEF2
#endif
			;
		if( bl->type&(BL_PC|BL_HOM) )
			flag |= SCB_ASPD|SCB_DSPD;
	}

	if(flag&SCB_VIT) {
		st->vit = status->calc_vit(bl, sc, bst->vit);
		flag|=SCB_DEF2|SCB_MDEF2;
		if( bl->type&(BL_PC|BL_HOM|BL_MER|BL_ELEM) )
			flag |= SCB_MAXHP;
		if( bl->type&BL_HOM )
			flag |= SCB_DEF;
	}

	if(flag&SCB_INT) {
		st->int_ = status->calc_int(bl, sc, bst->int_);
		flag|=SCB_MATK|SCB_MDEF2;
		if( bl->type&(BL_PC|BL_HOM|BL_MER|BL_ELEM) )
			flag |= SCB_MAXSP;
		if( bl->type&BL_HOM )
			flag |= SCB_MDEF;
	}

	if(flag&SCB_DEX) {
		st->dex = status->calc_dex(bl, sc, bst->dex);
		flag|=SCB_BATK|SCB_HIT
#ifdef RENEWAL
			|SCB_MATK|SCB_MDEF2
#endif
			;
		if( bl->type&(BL_PC|BL_HOM) )
			flag |= SCB_ASPD;
		if( bl->type&BL_HOM )
			flag |= SCB_WATK;
	}

	if(flag&SCB_LUK) {
		st->luk = status->calc_luk(bl, sc, bst->luk);
		flag|=SCB_BATK|SCB_CRI|SCB_FLEE2
#ifdef RENEWAL
			|SCB_MATK|SCB_HIT|SCB_FLEE
#endif
			;
	}

	if(flag&SCB_BATK && bst->batk) {
		st->batk = status->base_atk(bl,st);
		temp = bst->batk - status->base_atk(bl,bst);
		if (temp) {
			temp += st->batk;
			st->batk = cap_value(temp, 0, USHRT_MAX);
		}
		st->batk = status->calc_batk(bl, sc, st->batk, true);
	}

	if(flag&SCB_WATK) {		
		st->rhw.atk = status->calc_watk(bl, sc, bst->rhw.atk, true);
		if (!sd) //Should not affect weapon refine bonus
			st->rhw.atk2 = status->calc_watk(bl, sc, bst->rhw.atk2, true);

		if(bst->lhw.atk) {
			if (sd) {
				sd->state.lr_flag = 1;
				st->lhw.atk = status->calc_watk(bl, sc, bst->lhw.atk, true);
				sd->state.lr_flag = 0;
			} else {
				st->lhw.atk = status->calc_watk(bl, sc, bst->lhw.atk, true);
				st->lhw.atk2 = status->calc_watk(bl, sc, bst->lhw.atk2, true);
			}
		}

		if( bl->type&BL_HOM ) {
			st->rhw.atk += (st->dex - bst->dex);
			st->rhw.atk2 += (st->str - bst->str);
			if( st->rhw.atk2 < st->rhw.atk )
				st->rhw.atk2 = st->rhw.atk;
		}
	}

	if(flag&SCB_HIT) {
		if (st->dex == bst->dex
#ifdef RENEWAL
			&& st->luk == bst->luk
#endif
			)
			st->hit = status->calc_hit(bl, sc, bst->hit, true);
		else
			st->hit = status->calc_hit(bl, sc, bst->hit + (st->dex - bst->dex)
#ifdef RENEWAL
			+ (st->luk/3 - bst->luk/3)
#endif
			, true);
	}

	if(flag&SCB_FLEE) {
		if (st->agi == bst->agi
#ifdef RENEWAL
			&& st->luk == bst->luk
#endif
			)
			st->flee = status->calc_flee(bl, sc, bst->flee, true);
		else
			st->flee = status->calc_flee(bl, sc, bst->flee +(st->agi - bst->agi)
#ifdef RENEWAL
			+ (st->luk/5 - bst->luk/5)
#endif
			, true);
	}

	if(flag&SCB_DEF) {
		st->def = status->calc_def(bl, sc, bst->def, true);

		if( bl->type&BL_HOM )
			st->def += (st->vit/5 - bst->vit/5);
	}

	if(flag&SCB_DEF2) {
		if (st->vit == bst->vit
#ifdef RENEWAL
			&& st->agi == bst->agi
#endif
			)
			st->def2 = status->calc_def2(bl, sc, bst->def2, true);
		else
			st->def2 = status->calc_def2(bl, sc, bst->def2
#ifdef RENEWAL
			+ (int)( ((float)st->vit/2 - (float)bst->vit/2) + ((float)st->agi/5 - (float)bst->agi/5) )
#else
			+ (st->vit - bst->vit)
#endif
			, true);
	}

	if(flag&SCB_MDEF) {
		st->mdef = status->calc_mdef(bl, sc, bst->mdef, true);

		if( bl->type&BL_HOM )
			st->mdef += (st->int_/5 - bst->int_/5);
	}

	if(flag&SCB_MDEF2) {
		if (st->int_ == bst->int_ && st->vit == bst->vit
#ifdef RENEWAL
			&& st->dex == bst->dex
#endif
			)
			st->mdef2 = status->calc_mdef2(bl, sc, bst->mdef2, true);
		else
			st->mdef2 = status->calc_mdef2(bl, sc, bst->mdef2 +(st->int_ - bst->int_)
#ifdef RENEWAL
			+ (int)( ((float)st->dex/5 - (float)bst->dex/5) + ((float)st->vit/5 - (float)bst->vit/5) )
#else
			+ ((st->vit - bst->vit)>>1)
#endif
			, true);
	}

	if(flag&SCB_SPEED) {
		struct unit_data *ud = unit->bl2ud(bl);

		st->speed = status->calc_speed(bl, sc, bst->speed);

		//Re-walk to adjust speed (we do not check if walktimer != INVALID_TIMER
		//because if you step on something while walking, the moment this
		//piece of code triggers the walk-timer is set on INVALID_TIMER) [Skotlex]
		if (ud)
			ud->state.change_walk_target = ud->state.speed_changed = 1;

		if( bl->type&BL_PC && !(sd && sd->state.permanent_speed) && st->speed < battle_config.max_walk_speed )
			st->speed = battle_config.max_walk_speed;

		if( bl->type&BL_HOM && battle_config.hom_setting&0x8 && ((TBL_HOM*)bl)->master)
			st->speed = status->get_speed(&((TBL_HOM*)bl)->master->bl);
	}

	if(flag&SCB_CRI && bst->cri) {
		if (st->luk == bst->luk)
			st->cri = status->calc_critical(bl, sc, bst->cri, true);
		else
			st->cri = status->calc_critical(bl, sc, bst->cri + 3*(st->luk - bst->luk), true);
		/**
		* after status_calc_critical so the bonus is applied despite if you have or not a sc bugreport:5240
		**/
		if( bl->type == BL_PC && ((TBL_PC*)bl)->status.weapon == W_KATAR )
			st->cri <<= 1;

	}

	if(flag&SCB_FLEE2 && bst->flee2) {
		if (st->luk == bst->luk)
			st->flee2 = status->calc_flee2(bl, sc, bst->flee2, true);
		else
			st->flee2 = status->calc_flee2(bl, sc, bst->flee2 +(st->luk - bst->luk), true);
	}

	if(flag&SCB_ATK_ELE) {
		st->rhw.ele = status->calc_attack_element(bl, sc, bst->rhw.ele);
		if (sd) sd->state.lr_flag = 1;
		st->lhw.ele = status->calc_attack_element(bl, sc, bst->lhw.ele);
		if (sd) sd->state.lr_flag = 0;
	}

	if(flag&SCB_DEF_ELE) {
		st->def_ele = status->calc_element(bl, sc, bst->def_ele);
		st->ele_lv = status->calc_element_lv(bl, sc, bst->ele_lv);
	}

	if(flag&SCB_MODE) {
		st->mode = status->calc_mode(bl, sc, bst->mode);
		//Since mode changed, reset their state.
		if (!(st->mode&MD_CANATTACK))
			unit->stop_attack(bl);
		if (!(st->mode&MD_CANMOVE))
			unit->stop_walking(bl,1);
	}

	// No status changes alter these yet.
	//	if(flag&SCB_SIZE)
	// if(flag&SCB_RACE)
	// if(flag&SCB_RANGE)

	if(flag&SCB_MAXHP) {
		if( bl->type&BL_PC ) {
			st->max_hp = status->base_pc_maxhp(sd,st);
			st->max_hp += bst->max_hp - sd->status.max_hp;

			st->max_hp = status->calc_maxhp(bl, sc, st->max_hp);

			if( st->max_hp > (unsigned int)battle_config.max_hp )
				st->max_hp = (unsigned int)battle_config.max_hp;
		} else {
			st->max_hp = status->calc_maxhp(bl, sc, bst->max_hp);
		}

		if( st->hp > st->max_hp ) {
			//FIXME: Should perhaps a status_zap should be issued?
			st->hp = st->max_hp;
			if( sd ) clif->updatestatus(sd,SP_HP);
		}
	}

	if(flag&SCB_MAXSP) {
		if( bl->type&BL_PC ) {
			st->max_sp = status->base_pc_maxsp(sd,st);
			st->max_sp += bst->max_sp - sd->status.max_sp;

			st->max_sp = status->calc_maxsp(&sd->bl, &sd->sc, st->max_sp);

			if( st->max_sp > (unsigned int)battle_config.max_sp )
				st->max_sp = (unsigned int)battle_config.max_sp;
		} else {
			st->max_sp = status->calc_maxsp(bl, sc, bst->max_sp);
		}

		if( st->sp > st->max_sp ) {
			st->sp = st->max_sp;
			if( sd ) clif->updatestatus(sd,SP_SP);
		}
	}

	if(flag&SCB_MATK) {
		status->get_matk(bl, 0);
	}

	if(flag&SCB_ASPD) {
		int amotion;
		if( bl->type&BL_PC ) {
			amotion = status->base_amotion_pc(sd,st);
#ifndef RENEWAL_ASPD
			st->aspd_rate = status->calc_aspd_rate(bl, sc, bst->aspd_rate);

			if(st->aspd_rate != 1000)
				amotion = amotion*st->aspd_rate/1000;
#else
			// aspd = baseaspd + floor(sqrt((agi^2/2) + (dex^2/5))/4 + (potskillbonus*agi/200))
			amotion -= (int)(sqrt( (pow(st->agi, 2) / 2) + (pow(st->dex, 2) / 5) ) / 4 + ((float)status->calc_aspd(bl, sc, 1) * st->agi / 200)) * 10;

			if( (status->calc_aspd(bl, sc, 2) + st->aspd_rate2) != 0 ) // RE ASPD percertage modifier
				amotion -= (( amotion - ((sd->class_&JOBL_THIRD) ? battle_config.max_third_aspd : battle_config.max_aspd) )
				* (status->calc_aspd(bl, sc, 2) + st->aspd_rate2) / 10 + 5) / 10;

			if(st->aspd_rate != 1000) // absolute percentage modifier
				amotion = ( 200 - (200-amotion/10) * st->aspd_rate / 1000 ) * 10;
#endif
			amotion = status->calc_fix_aspd(bl, sc, amotion);
			st->amotion = cap_value(amotion,((sd->class_&JOBL_THIRD) ? battle_config.max_third_aspd : battle_config.max_aspd),2000);

			st->adelay = 2*st->amotion;
		} else if( bl->type&BL_HOM ) {
			amotion = (1000 - 4*st->agi - st->dex) * ((TBL_HOM*)bl)->homunculusDB->baseASPD/1000;
			st->aspd_rate = status->calc_aspd_rate(bl, sc, bst->aspd_rate);

			if(st->aspd_rate != 1000)
				amotion = amotion*st->aspd_rate/1000;

			amotion = status->calc_fix_aspd(bl, sc, amotion);
			st->amotion = cap_value(amotion,battle_config.max_aspd,2000);

			st->adelay = st->amotion;
		} else { // mercenary and mobs
			amotion = bst->amotion;
			st->aspd_rate = status->calc_aspd_rate(bl, sc, bst->aspd_rate);

			if(st->aspd_rate != 1000)
				amotion = amotion*st->aspd_rate/1000;

			amotion = status->calc_fix_aspd(bl, sc, amotion);
			st->amotion = cap_value(amotion, battle_config.monster_max_aspd, 2000);

			temp = bst->adelay*st->aspd_rate/1000;
			st->adelay = cap_value(temp, battle_config.monster_max_aspd*2, 4000);
		}
	}

	if(flag&SCB_DSPD) {
		int dmotion;
		if( bl->type&BL_PC ) {
			if (bst->agi == st->agi)
				st->dmotion = status->calc_dmotion(bl, sc, bst->dmotion);
			else {
				dmotion = 800-st->agi*4;
				st->dmotion = cap_value(dmotion, 400, 800);
				if(battle_config.pc_damage_delay_rate != 100)
					st->dmotion = st->dmotion*battle_config.pc_damage_delay_rate/100;
				//It's safe to ignore bst->dmotion since no bonus affects it.
				st->dmotion = status->calc_dmotion(bl, sc, st->dmotion);
			}
		} else if( bl->type&BL_HOM ) {
			dmotion = 800-st->agi*4;
			st->dmotion = cap_value(dmotion, 400, 800);
			st->dmotion = status->calc_dmotion(bl, sc, bst->dmotion);
		} else { // mercenary and mobs
			st->dmotion = status->calc_dmotion(bl, sc, bst->dmotion);
		}
	}

	if(flag&(SCB_VIT|SCB_MAXHP|SCB_INT|SCB_MAXSP) && bl->type&BL_REGEN)
		status->calc_regen(bl, st, status->get_regen_data(bl));

	if(flag&SCB_REGEN && bl->type&BL_REGEN)
		status->calc_regen_rate(bl, status->get_regen_data(bl), sc);
}
/// Recalculates parts of an object's base status and battle status according to the specified flags.
/// Also sends updates to the client wherever applicable.
/// @param flag bitfield of values from enum scb_flag
/// @param first if true, will cause status_calc_* functions to run their base status initialization code
void status_calc_bl_(struct block_list *bl, enum scb_flag flag, enum e_status_calc_opt opt) {
	struct status_data bst; // previous battle status
	struct status_data *st; // pointer to current battle status

	if( bl->type == BL_PC && ((TBL_PC*)bl)->delayed_damage != 0 ) {
		if( opt&SCO_FORCE )
			((TBL_PC*)bl)->state.hold_recalc = 0;/* clear and move on */
		else {
			((TBL_PC*)bl)->state.hold_recalc = 1;/* flag and stop */
			return;
		}
	}
	
	// remember previous values
	st = status->get_status_data(bl);
	memcpy(&bst, st, sizeof(struct status_data));

	if( flag&SCB_BASE ) {// calculate the object's base status too
		switch( bl->type ) {
			case BL_PC:   status->calc_pc_(BL_CAST(BL_PC,bl), opt);          break;
			case BL_MOB:  status->calc_mob_(BL_CAST(BL_MOB,bl), opt);        break;
			case BL_PET:  status->calc_pet_(BL_CAST(BL_PET,bl), opt);        break;
			case BL_HOM:  status->calc_homunculus_(BL_CAST(BL_HOM,bl), opt); break;
			case BL_MER:  status->calc_mercenary_(BL_CAST(BL_MER,bl), opt);  break;
			case BL_ELEM: status->calc_elemental_(BL_CAST(BL_ELEM,bl), opt); break;
			case BL_NPC:  status->calc_npc_(BL_CAST(BL_NPC,bl), opt);        break;
		}
	}

	if( bl->type == BL_PET )
		return; // pets are not affected by statuses

	if( opt&SCO_FIRST && bl->type == BL_MOB )
		return; // assume there will be no statuses active

	status->calc_bl_main(bl, flag);

	if( opt&SCO_FIRST && bl->type == BL_HOM )
		return; // client update handled by caller

	// compare against new values and send client updates
	if( bl->type == BL_PC ) {
		TBL_PC* sd = BL_CAST(BL_PC, bl);
		if(bst.str != st->str)
			clif->updatestatus(sd,SP_STR);
		if(bst.agi != st->agi)
			clif->updatestatus(sd,SP_AGI);
		if(bst.vit != st->vit)
			clif->updatestatus(sd,SP_VIT);
		if(bst.int_ != st->int_)
			clif->updatestatus(sd,SP_INT);
		if(bst.dex != st->dex)
			clif->updatestatus(sd,SP_DEX);
		if(bst.luk != st->luk)
			clif->updatestatus(sd,SP_LUK);
		if(bst.hit != st->hit)
			clif->updatestatus(sd,SP_HIT);
		if(bst.flee != st->flee)
			clif->updatestatus(sd,SP_FLEE1);
		if(bst.amotion != st->amotion)
			clif->updatestatus(sd,SP_ASPD);
		if(bst.speed != st->speed)
			clif->updatestatus(sd,SP_SPEED);

		if(bst.batk != st->batk
#ifndef RENEWAL
		   || bst.rhw.atk != st->rhw.atk || bst.lhw.atk != st->lhw.atk
#endif
		)
			clif->updatestatus(sd,SP_ATK1);

		if(bst.def != st->def) {
			clif->updatestatus(sd,SP_DEF1);
#ifdef RENEWAL
			clif->updatestatus(sd,SP_DEF2);
#endif
		}

		if(bst.rhw.atk2 != st->rhw.atk2 || bst.lhw.atk2 != st->lhw.atk2
#ifdef RENEWAL
			|| bst.rhw.atk != st->rhw.atk || bst.lhw.atk != st->lhw.atk
#endif
			)
			clif->updatestatus(sd,SP_ATK2);

		if(bst.def2 != st->def2){
			clif->updatestatus(sd,SP_DEF2);
#ifdef RENEWAL
			clif->updatestatus(sd,SP_DEF1);
#endif
		}
		if(bst.flee2 != st->flee2)
			clif->updatestatus(sd,SP_FLEE2);
		if(bst.cri != st->cri)
			clif->updatestatus(sd,SP_CRITICAL);
#ifndef RENEWAL
		if(bst.matk_max != st->matk_max)
			clif->updatestatus(sd,SP_MATK1);
		if(bst.matk_min != st->matk_min)
			clif->updatestatus(sd,SP_MATK2);
#else
		if(bst.matk_max != st->matk_max || bst.matk_min != st->matk_min){
			clif->updatestatus(sd,SP_MATK2);
			clif->updatestatus(sd,SP_MATK1);
		}
#endif
		if(bst.mdef != st->mdef) {
			clif->updatestatus(sd,SP_MDEF1);
#ifdef RENEWAL
			clif->updatestatus(sd,SP_MDEF2);
#endif
		}
		if(bst.mdef2 != st->mdef2) {
			clif->updatestatus(sd,SP_MDEF2);
#ifdef RENEWAL
			clif->updatestatus(sd,SP_MDEF1);
#endif
		}
		if(bst.rhw.range != st->rhw.range)
			clif->updatestatus(sd,SP_ATTACKRANGE);
		if(bst.max_hp != st->max_hp)
			clif->updatestatus(sd,SP_MAXHP);
		if(bst.max_sp != st->max_sp)
			clif->updatestatus(sd,SP_MAXSP);
		if(bst.hp != st->hp)
			clif->updatestatus(sd,SP_HP);
		if(bst.sp != st->sp)
			clif->updatestatus(sd,SP_SP);
#ifdef RENEWAL
		if(bst.equip_atk != st->equip_atk)
			clif->updatestatus(sd,SP_ATK2);
#endif
	} else if( bl->type == BL_HOM ) {
		TBL_HOM* hd = BL_CAST(BL_HOM, bl);
		if( hd->master && memcmp(&bst, st, sizeof(struct status_data)) != 0 )
			clif->hominfo(hd->master,hd,0);
	} else if( bl->type == BL_MER ) {
		TBL_MER* md = BL_CAST(BL_MER, bl);
		if( bst.rhw.atk != st->rhw.atk || bst.rhw.atk2 != st->rhw.atk2 )
			clif->mercenary_updatestatus(md->master, SP_ATK1);
		if( bst.matk_max != st->matk_max )
			clif->mercenary_updatestatus(md->master, SP_MATK1);
		if( bst.hit != st->hit )
			clif->mercenary_updatestatus(md->master, SP_HIT);
		if( bst.cri != st->cri )
			clif->mercenary_updatestatus(md->master, SP_CRITICAL);
		if( bst.def != st->def )
			clif->mercenary_updatestatus(md->master, SP_DEF1);
		if( bst.mdef != st->mdef )
			clif->mercenary_updatestatus(md->master, SP_MDEF1);
		if( bst.flee != st->flee )
			clif->mercenary_updatestatus(md->master, SP_MERCFLEE);
		if( bst.amotion != st->amotion )
			clif->mercenary_updatestatus(md->master, SP_ASPD);
		if( bst.max_hp != st->max_hp )
			clif->mercenary_updatestatus(md->master, SP_MAXHP);
		if( bst.max_sp != st->max_sp )
			clif->mercenary_updatestatus(md->master, SP_MAXSP);
		if( bst.hp != st->hp )
			clif->mercenary_updatestatus(md->master, SP_HP);
		if( bst.sp != st->sp )
			clif->mercenary_updatestatus(md->master, SP_SP);
	} else if( bl->type == BL_ELEM ) {
		TBL_ELEM* ed = BL_CAST(BL_ELEM, bl);
		if( bst.max_hp != st->max_hp )
			clif->elemental_updatestatus(ed->master, SP_MAXHP);
		if( bst.max_sp != st->max_sp )
			clif->elemental_updatestatus(ed->master, SP_MAXSP);
		if( bst.hp != st->hp )
			clif->elemental_updatestatus(ed->master, SP_HP);
		if( bst.sp != st->sp )
			clif->mercenary_updatestatus(ed->master, SP_SP);
	}
}

/*==========================================
* Apply shared stat mods from status changes [DracoRPG]
*------------------------------------------*/
unsigned short status_calc_str(struct block_list *bl, struct status_change *sc, int str)
{
	if(!sc || !sc->count)
		return cap_value(str,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		str -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(str,0,USHRT_MAX);
	}
	if(sc->data[SC_BEYOND_OF_WARCRY])
		str += sc->data[SC_BEYOND_OF_WARCRY]->val3;
	if(sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_HIGH && str < 50)
		return 50;
	if(sc->data[SC_INCALLSTATUS])
		str += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_CHASEWALK2])
		str += sc->data[SC_CHASEWALK2]->val1;
	if(sc->data[SC_FOOD_STR])
		str += sc->data[SC_FOOD_STR]->val1;
	if(sc->data[SC_FOOD_STR_CASH])
		str += sc->data[SC_FOOD_STR_CASH]->val1;
	if(sc->data[SC_GDSKILL_BATTLEORDER])
		str += 5;
	if(sc->data[SC_LEADERSHIP])
		str += sc->data[SC_LEADERSHIP]->val1;
	if(sc->data[SC_SHOUT])
		str += 4;
	if(sc->data[SC_TRUESIGHT])
		str += 5;
	if(sc->data[SC_STRUP])
		str += 10;
	if(sc->data[SC_NJ_NEN])
		str += sc->data[SC_NJ_NEN]->val1;
	if(sc->data[SC_BLESSING]){
		if(sc->data[SC_BLESSING]->val2)
			str += sc->data[SC_BLESSING]->val2;
		else
			str >>= 1;
	}
	if(sc->data[SC_MARIONETTE_MASTER])
		str -= ((sc->data[SC_MARIONETTE_MASTER]->val3)>>16)&0xFF;
	if(sc->data[SC_MARIONETTE])
		str += ((sc->data[SC_MARIONETTE]->val3)>>16)&0xFF;
	if(sc->data[SC_GIANTGROWTH])
		str += 30;
	if(sc->data[SC_SAVAGE_STEAK])
		str += sc->data[SC_SAVAGE_STEAK]->val1;
	if(sc->data[SC_INSPIRATION])
		str += sc->data[SC_INSPIRATION]->val3;
	if(sc->data[SC_STOMACHACHE])
		str -= sc->data[SC_STOMACHACHE]->val1;
	if(sc->data[SC_KYOUGAKU])
		str -= sc->data[SC_KYOUGAKU]->val2;
	if(sc->data[SC_FULL_THROTTLE])
		str += str * 20 / 100;

	return (unsigned short)cap_value(str,0,USHRT_MAX);
}

unsigned short status_calc_agi(struct block_list *bl, struct status_change *sc, int agi)
{
	if(!sc || !sc->count)
		return cap_value(agi,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		agi -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(agi,0,USHRT_MAX);
	}
	if(sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_HIGH && agi < 50)
		return 50;
	if(sc->data[SC_CONCENTRATION] && !sc->data[SC_QUAGMIRE])
		agi += (agi-sc->data[SC_CONCENTRATION]->val3)*sc->data[SC_CONCENTRATION]->val2/100;
	if(sc->data[SC_INCALLSTATUS])
		agi += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCAGI])
		agi += sc->data[SC_INCAGI]->val1;
	if(sc->data[SC_FOOD_AGI])
		agi += sc->data[SC_FOOD_AGI]->val1;
	if(sc->data[SC_FOOD_AGI_CASH])
		agi += sc->data[SC_FOOD_AGI_CASH]->val1;
	if(sc->data[SC_SOULCOLD])
		agi += sc->data[SC_SOULCOLD]->val1;
	if(sc->data[SC_TRUESIGHT])
		agi += 5;
	if(sc->data[SC_INC_AGI])
		agi += sc->data[SC_INC_AGI]->val2;
	if(sc->data[SC_GS_ACCURACY])
		agi += 4;	// added based on skill updates [Reddozen]
	if(sc->data[SC_DEC_AGI])
		agi -= sc->data[SC_DEC_AGI]->val2;
	if(sc->data[SC_QUAGMIRE])
		agi -= sc->data[SC_QUAGMIRE]->val2;
	if(sc->data[SC_NJ_SUITON] && sc->data[SC_NJ_SUITON]->val3)
		agi -= sc->data[SC_NJ_SUITON]->val2;
	if(sc->data[SC_MARIONETTE_MASTER])
		agi -= ((sc->data[SC_MARIONETTE_MASTER]->val3)>>8)&0xFF;
	if(sc->data[SC_MARIONETTE])
		agi += ((sc->data[SC_MARIONETTE]->val3)>>8)&0xFF;
	if(sc->data[SC_ADORAMUS])
		agi -= sc->data[SC_ADORAMUS]->val2;
	if(sc->data[SC_DROCERA_HERB_STEAMED])
		agi += sc->data[SC_DROCERA_HERB_STEAMED]->val1;
	if(sc->data[SC_INSPIRATION])
		agi += sc->data[SC_INSPIRATION]->val3;
	if(sc->data[SC_STOMACHACHE])
		agi -= sc->data[SC_STOMACHACHE]->val1;
	if(sc->data[SC_KYOUGAKU])
		agi -= sc->data[SC_KYOUGAKU]->val2;

	if(sc->data[SC_MARSHOFABYSS])
		agi -= agi * sc->data[SC_MARSHOFABYSS]->val2 / 100;
	if(sc->data[SC_FULL_THROTTLE])
		agi += agi * 20 / 100;

	return (unsigned short)cap_value(agi,0,USHRT_MAX);
}

unsigned short status_calc_vit(struct block_list *bl, struct status_change *sc, int vit)
{
	if(!sc || !sc->count)
		return cap_value(vit,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		vit -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(vit,0,USHRT_MAX);
	}
	if(sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_HIGH && vit < 50)
		return 50;
	if(sc->data[SC_INCALLSTATUS])
		vit += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCVIT])
		vit += sc->data[SC_INCVIT]->val1;
	if(sc->data[SC_FOOD_VIT])
		vit += sc->data[SC_FOOD_VIT]->val1;
	if(sc->data[SC_FOOD_VIT_CASH])
		vit += sc->data[SC_FOOD_VIT_CASH]->val1;
	if(sc->data[SC_HLIF_CHANGE])
		vit += sc->data[SC_HLIF_CHANGE]->val2;
	if(sc->data[SC_GLORYWOUNDS])
		vit += sc->data[SC_GLORYWOUNDS]->val1;
	if(sc->data[SC_TRUESIGHT])
		vit += 5;
	if(sc->data[SC_MARIONETTE_MASTER])
		vit -= sc->data[SC_MARIONETTE_MASTER]->val3&0xFF;
	if(sc->data[SC_MARIONETTE])
		vit += sc->data[SC_MARIONETTE]->val3&0xFF;
	if(sc->data[SC_LAUDAAGNUS])
		vit += 4 + sc->data[SC_LAUDAAGNUS]->val1;
	if(sc->data[SC_MINOR_BBQ])
		vit += sc->data[SC_MINOR_BBQ]->val1;
	if(sc->data[SC_INSPIRATION])
		vit += sc->data[SC_INSPIRATION]->val3;
	if(sc->data[SC_STOMACHACHE])
		vit -= sc->data[SC_STOMACHACHE]->val1;
	if(sc->data[SC_KYOUGAKU])
		vit -= sc->data[SC_KYOUGAKU]->val2;

	if(sc->data[SC_NOEQUIPARMOR])
		vit -= vit * sc->data[SC_NOEQUIPARMOR]->val2/100;
	if(sc->data[SC_FULL_THROTTLE])
		vit += vit * 20 / 100;

	return (unsigned short)cap_value(vit,0,USHRT_MAX);
}

unsigned short status_calc_int(struct block_list *bl, struct status_change *sc, int int_)
{
	if(!sc || !sc->count)
		return cap_value(int_,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		int_ -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(int_,0,USHRT_MAX);
	}
	if(sc->data[SC_MELODYOFSINK])
		int_ -= sc->data[SC_MELODYOFSINK]->val3;
	if(sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_HIGH && int_ < 50)
		return 50;
	if(sc->data[SC_INCALLSTATUS])
		int_ += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCINT])
		int_ += sc->data[SC_INCINT]->val1;
	if(sc->data[SC_FOOD_INT])
		int_ += sc->data[SC_FOOD_INT]->val1;
	if(sc->data[SC_FOOD_INT_CASH])
		int_ += sc->data[SC_FOOD_INT_CASH]->val1;
	if(sc->data[SC_HLIF_CHANGE])
		int_ += sc->data[SC_HLIF_CHANGE]->val3;
	if(sc->data[SC_GDSKILL_BATTLEORDER])
		int_ += 5;
	if(sc->data[SC_TRUESIGHT])
		int_ += 5;
	if(sc->data[SC_BLESSING]){
		if (sc->data[SC_BLESSING]->val2)
			int_ += sc->data[SC_BLESSING]->val2;
		else
			int_ >>= 1;
	}
	if(sc->data[SC_NJ_NEN])
		int_ += sc->data[SC_NJ_NEN]->val1;
	if(sc->data[SC_MARIONETTE_MASTER])
		int_ -= ((sc->data[SC_MARIONETTE_MASTER]->val4)>>16)&0xFF;
	if(sc->data[SC_MARIONETTE])
		int_ += ((sc->data[SC_MARIONETTE]->val4)>>16)&0xFF;
	if(sc->data[SC_MANDRAGORA])
		int_ -= 4 * sc->data[SC_MANDRAGORA]->val1;
	if(sc->data[SC_COCKTAIL_WARG_BLOOD])
		int_ += sc->data[SC_COCKTAIL_WARG_BLOOD]->val1;
	if(sc->data[SC_INSPIRATION])
		int_ += sc->data[SC_INSPIRATION]->val3;
	if(sc->data[SC_STOMACHACHE])
		int_ -= sc->data[SC_STOMACHACHE]->val1;
	if(sc->data[SC_KYOUGAKU])
		int_ -= sc->data[SC_KYOUGAKU]->val2;

	if(bl->type != BL_PC){
		if(sc->data[SC_NOEQUIPHELM])
			int_ -= int_ * sc->data[SC_NOEQUIPHELM]->val2/100;
		if(sc->data[SC__STRIPACCESSARY])
			int_ -= int_ * sc->data[SC__STRIPACCESSARY]->val2 / 100;
	}
	if(sc->data[SC_FULL_THROTTLE])
		int_ += int_ * 20 / 100;

	return (unsigned short)cap_value(int_,0,USHRT_MAX);
}

unsigned short status_calc_dex(struct block_list *bl, struct status_change *sc, int dex)
{
	if(!sc || !sc->count)
		return cap_value(dex,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		dex -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(dex,0,USHRT_MAX);
	}
	if(sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_HIGH && dex < 50)
		return 50;
	if(sc->data[SC_CONCENTRATION] && !sc->data[SC_QUAGMIRE])
		dex += (dex-sc->data[SC_CONCENTRATION]->val4)*sc->data[SC_CONCENTRATION]->val2/100;
	if(sc->data[SC_INCALLSTATUS])
		dex += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCDEX])
		dex += sc->data[SC_INCDEX]->val1;
	if(sc->data[SC_FOOD_DEX])
		dex += sc->data[SC_FOOD_DEX]->val1;
	if(sc->data[SC_FOOD_DEX_CASH])
		dex += sc->data[SC_FOOD_DEX_CASH]->val1;
	if(sc->data[SC_GDSKILL_BATTLEORDER])
		dex += 5;
	if(sc->data[SC_HAWKEYES])
		dex += sc->data[SC_HAWKEYES]->val1;
	if(sc->data[SC_TRUESIGHT])
		dex += 5;
	if(sc->data[SC_QUAGMIRE])
		dex -= sc->data[SC_QUAGMIRE]->val2;
	if(sc->data[SC_BLESSING]){
		if (sc->data[SC_BLESSING]->val2)
			dex += sc->data[SC_BLESSING]->val2;
		else
			dex >>= 1;
	}
	if(sc->data[SC_GS_ACCURACY])
		dex += 4;	// added based on skill updates [Reddozen]
	if(sc->data[SC_MARIONETTE_MASTER])
		dex -= ((sc->data[SC_MARIONETTE_MASTER]->val4)>>8)&0xFF;
	if(sc->data[SC_MARIONETTE])
		dex += ((sc->data[SC_MARIONETTE]->val4)>>8)&0xFF;
	if(sc->data[SC_SIROMA_ICE_TEA])
		dex += sc->data[SC_SIROMA_ICE_TEA]->val1;
	if(sc->data[SC_INSPIRATION])
		dex += sc->data[SC_INSPIRATION]->val3;
	if(sc->data[SC_STOMACHACHE])
		dex -= sc->data[SC_STOMACHACHE]->val1;
	if(sc->data[SC_KYOUGAKU])
		dex -= sc->data[SC_KYOUGAKU]->val2;

	if(sc->data[SC_MARSHOFABYSS])
		dex -= dex * sc->data[SC_MARSHOFABYSS]->val2 / 100;
	if(sc->data[SC__STRIPACCESSARY] && bl->type != BL_PC)
		dex -= dex * sc->data[SC__STRIPACCESSARY]->val2 / 100;
	if(sc->data[SC_FULL_THROTTLE])
		dex += dex * 20 / 100;

	return (unsigned short)cap_value(dex,0,USHRT_MAX);
}

unsigned short status_calc_luk(struct block_list *bl, struct status_change *sc, int luk)
{
	if(!sc || !sc->count)
		return cap_value(luk,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		luk -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(luk,0,USHRT_MAX);
	}
	if(sc->data[SC_CURSE])
		return 0;
	if(sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_HIGH && luk < 50)
		return 50;
	if(sc->data[SC_INCALLSTATUS])
		luk += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCLUK])
		luk += sc->data[SC_INCLUK]->val1;
	if(sc->data[SC_FOOD_LUK])
		luk += sc->data[SC_FOOD_LUK]->val1;
	if(sc->data[SC_FOOD_LUK_CASH])
		luk += sc->data[SC_FOOD_LUK_CASH]->val1;
	if(sc->data[SC_TRUESIGHT])
		luk += 5;
	if(sc->data[SC_GLORIA])
		luk += 30;
	if(sc->data[SC_MARIONETTE_MASTER])
		luk -= sc->data[SC_MARIONETTE_MASTER]->val4&0xFF;
	if(sc->data[SC_MARIONETTE])
		luk += sc->data[SC_MARIONETTE]->val4&0xFF;
	if(sc->data[SC_PUTTI_TAILS_NOODLES])
		luk += sc->data[SC_PUTTI_TAILS_NOODLES]->val1;
	if(sc->data[SC_INSPIRATION])
		luk += sc->data[SC_INSPIRATION]->val3;
	if(sc->data[SC_STOMACHACHE])
		luk -= sc->data[SC_STOMACHACHE]->val1;
	if(sc->data[SC_KYOUGAKU])
		luk -= sc->data[SC_KYOUGAKU]->val2;
	if(sc->data[SC_LAUDARAMUS])
		luk += 4 + sc->data[SC_LAUDARAMUS]->val1;

	if(sc->data[SC__STRIPACCESSARY] && bl->type != BL_PC)
		luk -= luk * sc->data[SC__STRIPACCESSARY]->val2 / 100;
	if(sc->data[SC_BANANA_BOMB])
		luk -= luk * sc->data[SC_BANANA_BOMB]->val1 / 100;
	if(sc->data[SC_FULL_THROTTLE])
		luk += luk * 20 / 100;

	return (unsigned short)cap_value(luk,0,USHRT_MAX);
}
unsigned short status_calc_batk(struct block_list *bl, struct status_change *sc, int batk, bool viewable)
{
	if(!sc || !sc->count)
		return cap_value(batk,0,USHRT_MAX);

	if( !viewable ){
		/* some statuses that are hidden in the status window */
		if(sc->data[SC_PLUSATTACKPOWER])
			batk += sc->data[SC_PLUSATTACKPOWER]->val1;
		return (unsigned short)cap_value(batk,0,USHRT_MAX);
	}
#ifndef RENEWAL
	if(sc->data[SC_PLUSATTACKPOWER])
		batk += sc->data[SC_PLUSATTACKPOWER]->val1;
	if(sc->data[SC_GS_MADNESSCANCEL])
		batk += 100;
	if(sc->data[SC_GS_GATLINGFEVER])
		batk += sc->data[SC_GS_GATLINGFEVER]->val3;
#endif
	if(sc->data[SC_BATKFOOD])
		batk += sc->data[SC_BATKFOOD]->val1;
	if(sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 2)
		batk += 50;
	if(bl->type == BL_ELEM
		&& ((sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 1)
		|| (sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 1)
		|| (sc->data[SC_WIND_INSIGNIA] && sc->data[SC_WIND_INSIGNIA]->val1 == 1)
		|| (sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 1))
		)
		batk += batk / 5;
	if(sc->data[SC_FULL_SWING_K])
		batk += sc->data[SC_FULL_SWING_K]->val1;
	if(sc->data[SC_VOLCANIC_ASH] && (bl->type==BL_MOB)){
		if(status_get_element(bl) == ELE_WATER) //water type
			batk /= 2;
	}
	if(sc->data[SC_PYROCLASTIC])
		batk += sc->data[SC_PYROCLASTIC]->val2;
	if (sc->data[SC_ANGRIFFS_MODUS])
		batk += sc->data[SC_ANGRIFFS_MODUS]->val2;

	if(sc->data[SC_INCATKRATE])
		batk += batk * sc->data[SC_INCATKRATE]->val1/100;
	if(sc->data[SC_PROVOKE])
		batk += batk * sc->data[SC_PROVOKE]->val3/100;
#ifndef RENEWAL
	if(sc->data[SC_LKCONCENTRATION])
		batk += batk * sc->data[SC_LKCONCENTRATION]->val2/100;
#endif
	if(sc->data[SC_SKE])
		batk += batk * 3;
	if(sc->data[SC_HAMI_BLOODLUST])
		batk += batk * sc->data[SC_HAMI_BLOODLUST]->val2/100;
	if(sc->data[SC_JOINTBEAT] && sc->data[SC_JOINTBEAT]->val2&BREAK_WAIST)
		batk -= batk * 25/100;
	if(sc->data[SC_CURSE])
		batk -= batk * 25/100;
	if( sc->data[SC_ZANGETSU] )
		batk += sc->data[SC_ZANGETSU]->val2;
	//Curse shouldn't effect on this?  <- Curse OR Bleeding??
	//	if(sc->data[SC_BLOODING])
	//		batk -= batk * 25/100;
	if(sc->data[SC_HLIF_FLEET])
		batk += batk * sc->data[SC_HLIF_FLEET]->val3/100;
	if(sc->data[SC__ENERVATION])
		batk -= batk * sc->data[SC__ENERVATION]->val2 / 100;
	if(sc->data[SC_SATURDAY_NIGHT_FEVER])
		batk += 100 * sc->data[SC_SATURDAY_NIGHT_FEVER]->val1;

	return (unsigned short)cap_value(batk,0,USHRT_MAX);
}

unsigned short status_calc_watk(struct block_list *bl, struct status_change *sc, int watk, bool viewable)
{
	if(!sc || !sc->count)
		return cap_value(watk,0,USHRT_MAX);

	if( !viewable ){
		/* some statuses that are hidden in the status window */
		if(sc->data[SC_STRIKING])
			watk += sc->data[SC_STRIKING]->val2;
		if(sc->data[SC_GENTLETOUCH_CHANGE] && sc->data[SC_GENTLETOUCH_CHANGE]->val2)
			watk += sc->data[SC_GENTLETOUCH_CHANGE]->val2;
		return (unsigned short)cap_value(watk,0,USHRT_MAX);
	}
#ifndef RENEWAL
	if(sc->data[SC_IMPOSITIO])
		watk += sc->data[SC_IMPOSITIO]->val2;
	if(sc->data[SC_DRUMBATTLE])
		watk += sc->data[SC_DRUMBATTLE]->val2;
#endif
	if(sc->data[SC_WATKFOOD])
		watk += sc->data[SC_WATKFOOD]->val1;
	if(sc->data[SC_VOLCANO])
		watk += sc->data[SC_VOLCANO]->val2;
	if(sc->data[SC_MER_ATK])
		watk += sc->data[SC_MER_ATK]->val2;
	if(sc->data[SC_FIGHTINGSPIRIT])
		watk += sc->data[SC_FIGHTINGSPIRIT]->val1;
	if(sc->data[SC_SHIELDSPELL_DEF] && sc->data[SC_SHIELDSPELL_DEF]->val1 == 3)
		watk += sc->data[SC_SHIELDSPELL_DEF]->val2;
	if(sc->data[SC_INSPIRATION])
		watk += sc->data[SC_INSPIRATION]->val2;
	if( sc->data[SC_BANDING] && sc->data[SC_BANDING]->val2 > 0 )
		watk += (10 + 10 * sc->data[SC_BANDING]->val1) * (sc->data[SC_BANDING]->val2);
	if( sc->data[SC_TROPIC_OPTION] )
		watk += sc->data[SC_TROPIC_OPTION]->val2;
	if( sc->data[SC_HEATER_OPTION] )
		watk += sc->data[SC_HEATER_OPTION]->val2;
	if( sc->data[SC_WATER_BARRIER] )
		watk -= sc->data[SC_WATER_BARRIER]->val3;
	if( sc->data[SC_PYROTECHNIC_OPTION] )
		watk += sc->data[SC_PYROTECHNIC_OPTION]->val2;

#ifndef RENEWAL
	if(sc->data[SC_NIBELUNGEN]) {
		if (bl->type != BL_PC)
			watk += sc->data[SC_NIBELUNGEN]->val2;
		else {
			TBL_PC *sd = (TBL_PC*)bl;
			int index = sd->equip_index[sd->state.lr_flag?EQI_HAND_L:EQI_HAND_R];
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->wlv == 4)
				watk += sc->data[SC_NIBELUNGEN]->val2;
		}
	}
	if(sc->data[SC_STRIKING])
		watk += sc->data[SC_STRIKING]->val2;
	if(sc->data[SC_GENTLETOUCH_CHANGE] && sc->data[SC_GENTLETOUCH_CHANGE]->val2)
		watk += sc->data[SC_GENTLETOUCH_CHANGE]->val2;
	if(sc->data[SC_LKCONCENTRATION])
		watk += watk * sc->data[SC_LKCONCENTRATION]->val2/100;
#endif
	if(sc->data[SC_INCATKRATE])
		watk += watk * sc->data[SC_INCATKRATE]->val1/100;
	if(sc->data[SC_PROVOKE])
		watk += watk * sc->data[SC_PROVOKE]->val3/100;
	if(sc->data[SC_SKE])
		watk += watk * 3;
	if(sc->data[SC_HLIF_FLEET])
		watk += watk * sc->data[SC_HLIF_FLEET]->val3/100;
	if(sc->data[SC_CURSE])
		watk -= watk * 25/100;
	if(sc->data[SC_NOEQUIPWEAPON] && bl->type != BL_PC)
		watk -= watk * sc->data[SC_NOEQUIPWEAPON]->val2/100;
	if(sc->data[SC__ENERVATION])
		watk -= watk * sc->data[SC__ENERVATION]->val2 / 100;
	if(sc->data[SC_RUSH_WINDMILL])
		watk += sc->data[SC_RUSH_WINDMILL]->val2;
	if (sc->data[SC_ODINS_POWER])
		watk += 40 + 30 * sc->data[SC_ODINS_POWER]->val1;
	if((sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 2)
		|| (sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 2)
		|| (sc->data[SC_WIND_INSIGNIA] && sc->data[SC_WIND_INSIGNIA]->val1 == 2)
		|| (sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 2)
		)
		watk += watk / 10;
	if( sc && sc->data[SC_TIDAL_WEAPON] )
		watk += watk * sc->data[SC_TIDAL_WEAPON]->val2 / 100;
	if(sc->data[SC_ANGRIFFS_MODUS])
		watk += watk * sc->data[SC_ANGRIFFS_MODUS]->val2/100;

	return (unsigned short)cap_value(watk,0,USHRT_MAX);
}
unsigned short status_calc_ematk(struct block_list *bl, struct status_change *sc, int matk) {
#ifdef RENEWAL

	if (!sc || !sc->count)
		return cap_value(matk,0,USHRT_MAX);
	if (sc->data[SC_PLUSMAGICPOWER])
		matk += sc->data[SC_PLUSMAGICPOWER]->val1;
	if (sc->data[SC_MATKFOOD])
		matk += sc->data[SC_MATKFOOD]->val1;
	if(sc->data[SC_MANA_PLUS])
		matk += sc->data[SC_MANA_PLUS]->val1;
	if(sc->data[SC_AQUAPLAY_OPTION])
		matk += sc->data[SC_AQUAPLAY_OPTION]->val2;
	if(sc->data[SC_CHILLY_AIR_OPTION])
		matk += sc->data[SC_CHILLY_AIR_OPTION]->val2;
	if(sc->data[SC_COOLER_OPTION])
		matk += sc->data[SC_COOLER_OPTION]->val2;
	if(sc->data[SC_WATER_BARRIER])
		matk -= sc->data[SC_WATER_BARRIER]->val3;
	if(sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 3)
		matk += 50;
	if(sc->data[SC_ODINS_POWER])
		matk += 40 + 30 * sc->data[SC_ODINS_POWER]->val1; //70 lvl1, 100lvl2
	if(sc->data[SC_IZAYOI])
		matk += 25 * sc->data[SC_IZAYOI]->val1;
	return (unsigned short)cap_value(matk,0,USHRT_MAX);
#else
	return 0;
#endif
}
unsigned short status_calc_matk(struct block_list *bl, struct status_change *sc, int matk, bool viewable)
{
	if(!sc || !sc->count)
		return cap_value(matk,0,USHRT_MAX);

	if( !viewable ){
		/* some statuses that are hidden in the status window */
		return (unsigned short)cap_value(matk,0,USHRT_MAX);
	}

#ifndef RENEWAL
	// take note fixed value first before % modifiers
	if (sc->data[SC_PLUSMAGICPOWER])
		matk += sc->data[SC_PLUSMAGICPOWER]->val1;
	if (sc->data[SC_MATKFOOD])
		matk += sc->data[SC_MATKFOOD]->val1;
	if (sc->data[SC_MANA_PLUS])
		matk += sc->data[SC_MANA_PLUS]->val1;
	if (sc->data[SC_AQUAPLAY_OPTION])
		matk += sc->data[SC_AQUAPLAY_OPTION]->val2;
	if (sc->data[SC_CHILLY_AIR_OPTION])
		matk += sc->data[SC_CHILLY_AIR_OPTION]->val2;
	if(sc->data[SC_COOLER_OPTION])
		matk += sc->data[SC_COOLER_OPTION]->val2;
	if (sc->data[SC_WATER_BARRIER])
		matk -= sc->data[SC_WATER_BARRIER]->val3;
	if (sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 3)
		matk += 50;
	if (sc->data[SC_ODINS_POWER])
		matk += 40 + 30 * sc->data[SC_ODINS_POWER]->val1; //70 lvl1, 100lvl2
	if (sc->data[SC_IZAYOI])
		matk += 25 * sc->data[SC_IZAYOI]->val1;
#endif
	if( sc->data[SC_ZANGETSU] )
		matk += sc->data[SC_ZANGETSU]->val3;
	if (sc->data[SC_MAGICPOWER] && sc->data[SC_MAGICPOWER]->val4)
		matk += matk * sc->data[SC_MAGICPOWER]->val3/100;
	if (sc->data[SC_MINDBREAKER])
		matk += matk * sc->data[SC_MINDBREAKER]->val2/100;
	if (sc->data[SC_INCMATKRATE])
		matk += matk * sc->data[SC_INCMATKRATE]->val1/100;
	if (sc->data[SC_MOONLIT_SERENADE])
		matk += matk * sc->data[SC_MOONLIT_SERENADE]->val2/100;
	if (sc->data[SC_MTF_MATK])
		matk += matk * 25 / 100;

	return (unsigned short)cap_value(matk,0,USHRT_MAX);
}

signed short status_calc_critical(struct block_list *bl, struct status_change *sc, int critical, bool viewable) {

	if(!sc || !sc->count)
		return cap_value(critical,10,SHRT_MAX);

	if( !viewable ){
		/* some statuses that are hidden in the status window */
		return (short)cap_value(critical,10,SHRT_MAX);
	}

	if (sc->data[SC_CRITICALPERCENT])
		critical += sc->data[SC_CRITICALPERCENT]->val2;
	if (sc->data[SC_EXPLOSIONSPIRITS])
		critical += sc->data[SC_EXPLOSIONSPIRITS]->val2;
	if (sc->data[SC_FORTUNE])
		critical += sc->data[SC_FORTUNE]->val2;
	if (sc->data[SC_TRUESIGHT])
		critical += sc->data[SC_TRUESIGHT]->val2;
	if(sc->data[SC_CLOAKING])
		critical += critical;
	if(sc->data[SC_STRIKING])
		critical += sc->data[SC_STRIKING]->val1;
#ifdef RENEWAL
	if (sc->data[SC_SPEARQUICKEN])
		critical += 3*sc->data[SC_SPEARQUICKEN]->val1*10;
#endif

	if(sc->data[SC__INVISIBILITY])
		critical += sc->data[SC__INVISIBILITY]->val3;
	if(sc->data[SC__UNLUCKY])
		critical -= critical * sc->data[SC__UNLUCKY]->val2 / 100;
	if(sc->data[SC_BEYOND_OF_WARCRY])
		critical += 10 * sc->data[SC_BEYOND_OF_WARCRY]->val3;

	return (short)cap_value(critical,10,SHRT_MAX);
}

signed short status_calc_hit(struct block_list *bl, struct status_change *sc, int hit, bool viewable)
{

	if(!sc || !sc->count)
		return cap_value(hit,1,SHRT_MAX);

	if( !viewable ){
		/* some statuses that are hidden in the status window */
		if(sc->data[SC_MTF_ASPD])
			hit += 5;
		return (short)cap_value(hit,1,SHRT_MAX);
	}

	if(sc->data[SC_INCHIT])
		hit += sc->data[SC_INCHIT]->val1;
	if(sc->data[SC_FOOD_BASICHIT])
		hit += sc->data[SC_FOOD_BASICHIT]->val1;
	if(sc->data[SC_TRUESIGHT])
		hit += sc->data[SC_TRUESIGHT]->val3;
	if(sc->data[SC_HUMMING])
		hit += sc->data[SC_HUMMING]->val2;
	if(sc->data[SC_LKCONCENTRATION])
		hit += sc->data[SC_LKCONCENTRATION]->val3;
	if(sc->data[SC_INSPIRATION])
		hit += 5 * sc->data[SC_INSPIRATION]->val1 + 25;
	if(sc->data[SC_GS_ADJUSTMENT])
		hit -= 30;
	if(sc->data[SC_GS_ACCURACY])
		hit += 20; // RockmanEXE; changed based on updated [Reddozen]
	if(sc->data[SC_MER_HIT])
		hit += sc->data[SC_MER_HIT]->val2;

	if(sc->data[SC_INCHITRATE])
		hit += hit * sc->data[SC_INCHITRATE]->val1/100;
	if(sc->data[SC_BLIND])
		hit -= hit * 25/100;
	if(sc->data[SC__GROOMY])
		hit -= hit * sc->data[SC__GROOMY]->val3 / 100;
	if(sc->data[SC_FEAR])
		hit -= hit * 20 / 100;
	if (sc->data[SC_VOLCANIC_ASH])
		hit /= 2;
	if(sc->data[SC_ILLUSIONDOPING])
		hit -= hit * (5 + sc->data[SC_ILLUSIONDOPING]->val1) / 100; //custom
	
	return (short)cap_value(hit,1,SHRT_MAX);
}

signed short status_calc_flee(struct block_list *bl, struct status_change *sc, int flee, bool viewable) {
	if( bl->type == BL_PC ) {
		if( map_flag_gvg2(bl->m) )
			flee -= flee * battle_config.gvg_flee_penalty/100;
		else if( map->list[bl->m].flag.battleground )
			flee -= flee * battle_config.bg_flee_penalty/100;
	}

	if(!sc || !sc->count)
		return cap_value(flee,1,SHRT_MAX);

	if( !viewable ){
		/* some statuses that are hidden in the status window */
		return (short)cap_value(flee,1,SHRT_MAX);
	}

	if(sc->data[SC_INCFLEE])
		flee += sc->data[SC_INCFLEE]->val1;
	if(sc->data[SC_FOOD_BASICAVOIDANCE])
		flee += sc->data[SC_FOOD_BASICAVOIDANCE]->val1;
	if(sc->data[SC_WHISTLE])
		flee += sc->data[SC_WHISTLE]->val2;
	if(sc->data[SC_WINDWALK])
		flee += sc->data[SC_WINDWALK]->val2;
	if(sc->data[SC_VIOLENTGALE])
		flee += sc->data[SC_VIOLENTGALE]->val2;
	if(sc->data[SC_MOON_COMFORT]) //SG skill [Komurka]
		flee += sc->data[SC_MOON_COMFORT]->val2;
	if(sc->data[SC_RG_CCONFINE_M])
		flee += 10;
	if (sc->data[SC_ANGRIFFS_MODUS])
		flee -= sc->data[SC_ANGRIFFS_MODUS]->val3;
	if (sc->data[SC_OVERED_BOOST])
		flee = max(flee,sc->data[SC_OVERED_BOOST]->val2);
	if(sc->data[SC_GS_ADJUSTMENT])
		flee += 30;
	if(sc->data[SC_HLIF_SPEED])
		flee += 10 + sc->data[SC_HLIF_SPEED]->val1 * 10;
	if(sc->data[SC_GS_GATLINGFEVER])
		flee -= sc->data[SC_GS_GATLINGFEVER]->val4;
	if(sc->data[SC_PARTYFLEE])
		flee += sc->data[SC_PARTYFLEE]->val1 * 10;
	if(sc->data[SC_MER_FLEE])
		flee += sc->data[SC_MER_FLEE]->val2;
	if( sc->data[SC_HALLUCINATIONWALK] )
		flee += sc->data[SC_HALLUCINATIONWALK]->val2;
	if( sc->data[SC_WATER_BARRIER] )
		flee -= sc->data[SC_WATER_BARRIER]->val3;
#ifdef RENEWAL
	if( sc->data[SC_SPEARQUICKEN] )
		flee += 2 * sc->data[SC_SPEARQUICKEN]->val1;
#endif

	if(sc->data[SC_INCFLEERATE])
		flee += flee * sc->data[SC_INCFLEERATE]->val1/100;
	if(sc->data[SC_SPIDERWEB] && sc->data[SC_SPIDERWEB]->val1)
		flee -= flee * 50/100;
	if (sc->data[SC_BERSERK])
		flee -= flee * 50/100;
	if(sc->data[SC_BLIND])
		flee -= flee * 25/100;
	if(sc->data[SC_FEAR])
		flee -= flee * 20 / 100;
	if(sc->data[SC_PARALYSE])
		flee -= flee / 10; // 10% Flee reduction
	if(sc->data[SC_INFRAREDSCAN])
		flee -= flee * 30 / 100;
	if( sc->data[SC__LAZINESS] )
		flee -= flee * sc->data[SC__LAZINESS]->val3 / 100;
	if( sc->data[SC_GLOOMYDAY] )
		flee -= flee * ( 20 + 5 * sc->data[SC_GLOOMYDAY]->val1 ) / 100;
	if( sc->data[SC_SATURDAY_NIGHT_FEVER] )
		flee -= flee * (40 + 10 * sc->data[SC_SATURDAY_NIGHT_FEVER]->val1) / 100;
	if( sc->data[SC_WIND_STEP_OPTION] )
		flee += flee * sc->data[SC_WIND_STEP_OPTION]->val2 / 100;
	if( sc->data[SC_ZEPHYR] )
		flee += sc->data[SC_ZEPHYR]->val2;
	if(sc->data[SC_VOLCANIC_ASH] && (bl->type==BL_MOB)){ //mob
		if(status_get_element(bl) == ELE_WATER) //water type
			flee /= 2;
	}

	return (short)cap_value(flee,1,SHRT_MAX);
}

signed short status_calc_flee2(struct block_list *bl, struct status_change *sc, int flee2, bool viewable)
{
	if(!sc || !sc->count)
		return cap_value(flee2,10,SHRT_MAX);

	if( !viewable ){
		/* some statuses that are hidden in the status window */
		return (short)cap_value(flee2,10,SHRT_MAX);
	}

	if(sc->data[SC_PLUSAVOIDVALUE])
		flee2 += sc->data[SC_PLUSAVOIDVALUE]->val2;
	if(sc->data[SC_WHISTLE])
		flee2 += sc->data[SC_WHISTLE]->val3*10;
	if(sc->data[SC__UNLUCKY])
		flee2 -= flee2 * sc->data[SC__UNLUCKY]->val2 / 100;

	return (short)cap_value(flee2,10,SHRT_MAX);
}
defType status_calc_def(struct block_list *bl, struct status_change *sc, int def, bool viewable) {

	if(!sc || !sc->count)
		return (defType)cap_value(def,DEFTYPE_MIN,DEFTYPE_MAX);

	if( !viewable ){
		/* some statuses that are hidden in the status window */
		if( sc && sc->data[SC_CAMOUFLAGE] )
			def -= def * 5 * (10-sc->data[SC_CAMOUFLAGE]->val4) / 100;
		if( sc && sc->data[SC_GENTLETOUCH_REVITALIZE] && sc->data[SC_GENTLETOUCH_REVITALIZE]->val4 )
			def += 2 * sc->data[SC_GENTLETOUCH_REVITALIZE]->val4;
		if( sc->data[SC_FORCEOFVANGUARD] )
			def += def * 2 * sc->data[SC_FORCEOFVANGUARD]->val1 / 100;
		return (defType)cap_value(def,DEFTYPE_MIN,DEFTYPE_MAX);
	}

	if (sc->data[SC_BERSERK])
		return 0;
	if(sc->data[SC_SKA])
		return sc->data[SC_SKA]->val3;
	if(sc->data[SC_BARRIER])
		return 100;
	if(sc->data[SC_KEEPING])
		return 90;
#ifndef RENEWAL // does not provide 90 DEF in renewal mode
	if(sc->data[SC_STEELBODY])
		return 90;
#endif

	if(sc->data[SC_STONESKIN])
		def += sc->data[SC_STONESKIN]->val2;
	if(sc->data[SC_DRUMBATTLE])
		def += sc->data[SC_DRUMBATTLE]->val3;
	if(sc->data[SC_HAMI_DEFENCE])	//[orn]
		def += sc->data[SC_HAMI_DEFENCE]->val2 ;
	if(sc->data[SC_INCDEFRATE])
		def += def * sc->data[SC_INCDEFRATE]->val1/100;
	if(sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 2)
		def += 50;
	if(sc->data[SC_ODINS_POWER])
		def -= 20;
	if( sc->data[SC_ANGRIFFS_MODUS] )
		def -= 30 + 20 * sc->data[SC_ANGRIFFS_MODUS]->val1;
	if(sc->data[SC_STONEHARDSKIN])
		def += sc->data[SC_STONEHARDSKIN]->val1;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		def >>=1;
	if(sc->data[SC_FREEZE])
		def >>=1;
	if(sc->data[SC_CRUCIS])
		def -= def * sc->data[SC_CRUCIS]->val2/100;
	if(sc->data[SC_LKCONCENTRATION])
		def -= def * sc->data[SC_LKCONCENTRATION]->val4/100;
	if(sc->data[SC_SKE])
		def >>=1;
	if(sc->data[SC_PROVOKE] && bl->type != BL_PC) // Provoke doesn't alter player defense->
		def -= def * sc->data[SC_PROVOKE]->val4/100;
	if(sc->data[SC_NOEQUIPSHIELD]  && bl->type != BL_PC)
		def -= def * sc->data[SC_NOEQUIPSHIELD]->val2/100;
	if (sc->data[SC_FLING])
		def -= def * (sc->data[SC_FLING]->val2)/100;
	if( sc->data[SC_ANALYZE] )
		def -= def * ( 14 * sc->data[SC_ANALYZE]->val1 ) / 100;
	if(sc->data[SC_SATURDAY_NIGHT_FEVER])
		def -= def * (10 + 10 * sc->data[SC_SATURDAY_NIGHT_FEVER]->val1) / 100;
	if(sc->data[SC_EARTHDRIVE])
		def -= def * 25 / 100;
	if(sc->data[SC_SOLID_SKIN_OPTION])
		def += def * sc->data[SC_SOLID_SKIN_OPTION]->val2 / 100;
	if( sc->data[SC_ROCK_CRUSHER] )
		def -= def * sc->data[SC_ROCK_CRUSHER]->val2 / 100;
	if( sc->data[SC_POWER_OF_GAIA] )
		def += def * sc->data[SC_POWER_OF_GAIA]->val2 / 100;
	if( sc->data[SC_NEUTRALBARRIER] )
		def += def * (5 * sc->data[SC_NEUTRALBARRIER]->val1 + 10) / 100;
	if( sc->data[SC_SHIELDSPELL_REF] && sc->data[SC_SHIELDSPELL_REF]->val1 == 2 )
		def += sc->data[SC_SHIELDSPELL_REF]->val2;
	if( sc->data[SC_PRESTIGE] )
		def += sc->data[SC_PRESTIGE]->val1;
	if( sc->data[SC_FROSTMISTY] )
		def -= def * 10 / 100;
	if(sc->data[SC_VOLCANIC_ASH] && (bl->type==BL_MOB)){
		if(status_get_race(bl)==RC_PLANT)
			def /= 2;
	}

	return (defType)cap_value(def,DEFTYPE_MIN,DEFTYPE_MAX);
}

signed short status_calc_def2(struct block_list *bl, struct status_change *sc, int def2, bool viewable)
{
	if(!sc || !sc->count)
#ifdef RENEWAL
		return (short)cap_value(def2,SHRT_MIN,SHRT_MAX);
#else
		return (short)cap_value(def2,1,SHRT_MAX);
#endif

	if( !viewable ){
		/* some statuses that are hidden in the status window */
#ifdef RENEWAL
		if( sc && sc->data[SC_ASSUMPTIO] )
			def2 <<= 1;
#endif				
		if( sc && sc->data[SC_CAMOUFLAGE] )
			def2 -= def2 * 5 * (10-sc->data[SC_CAMOUFLAGE]->val4) / 100;
#ifdef RENEWAL
		return (short)cap_value(def2,SHRT_MIN,SHRT_MAX);
#else
		return (short)cap_value(def2,1,SHRT_MAX);
#endif
	}

	if (sc->data[SC_BERSERK])
		return 0;
	if(sc->data[SC_ETERNALCHAOS])
		return 0;
	if(sc->data[SC_SUN_COMFORT])
		def2 += sc->data[SC_SUN_COMFORT]->val2;
	if( sc->data[SC_BANDING] && sc->data[SC_BANDING]->val2 > 0 )
		def2 += (5 + sc->data[SC_BANDING]->val1) * (sc->data[SC_BANDING]->val2);
	if(sc->data[SC_ANGELUS])
#ifdef RENEWAL //in renewal only the VIT stat bonus is boosted by angelus
		def2 += status_get_vit(bl) / 2 * sc->data[SC_ANGELUS]->val2/100;
#else
		def2 += def2 * sc->data[SC_ANGELUS]->val2/100;
	if(sc->data[SC_LKCONCENTRATION])
		def2 -= def2 * sc->data[SC_LKCONCENTRATION]->val4/100;
#endif
	if(sc->data[SC_POISON])
		def2 -= def2 * 25/100;
	if(sc->data[SC_DPOISON])
		def2 -= def2 * 25/100;
	if(sc->data[SC_SKE])
		def2 -= def2 * 50/100;
	if(sc->data[SC_PROVOKE])
		def2 -= def2 * sc->data[SC_PROVOKE]->val4/100;
	if(sc->data[SC_JOINTBEAT])
		def2 -= def2 * ( sc->data[SC_JOINTBEAT]->val2&BREAK_SHOULDER ? 50 : 0 ) / 100
		+ def2 * ( sc->data[SC_JOINTBEAT]->val2&BREAK_WAIST ? 25 : 0 ) / 100;
	if(sc->data[SC_FLING])
		def2 -= def2 * (sc->data[SC_FLING]->val3)/100;
	if(sc->data[SC_ANALYZE])
		def2 -= def2 * ( 14 * sc->data[SC_ANALYZE]->val1 ) / 100;
	if( sc->data[SC_ECHOSONG] )
		def2 += def2 * sc->data[SC_ECHOSONG]->val3/100;
	if(sc->data[SC_VOLCANIC_ASH] && (bl->type==BL_MOB)){
		if(status_get_race(bl)==RC_PLANT)
			def2 /= 2;
	}
	if (sc->data[SC_NEEDLE_OF_PARALYZE])
		def2 -= def2 * sc->data[SC_NEEDLE_OF_PARALYZE]->val2 / 100;

#ifdef RENEWAL
	return (short)cap_value(def2,SHRT_MIN,SHRT_MAX);
#else
	return (short)cap_value(def2,1,SHRT_MAX);
#endif
}


defType status_calc_mdef(struct block_list *bl, struct status_change *sc, int mdef, bool viewable) {

	if(!sc || !sc->count)
		return (defType)cap_value(mdef,DEFTYPE_MIN,DEFTYPE_MAX);

	if( !viewable ){
		/* some statuses that are hidden in the status window */
		return (defType)cap_value(mdef,DEFTYPE_MIN,DEFTYPE_MAX);
	}

	if (sc->data[SC_BERSERK])
		return 0;
	if(sc->data[SC_BARRIER])
		return 100;

#ifndef RENEWAL // no longer provides 90 MDEF in renewal mode
	if(sc->data[SC_STEELBODY])
		return 90;
#endif

	if(sc->data[SC_STONESKIN])
		mdef += sc->data[SC_STONESKIN]->val3;
	if(sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 3)
		mdef += 50;
	if(sc->data[SC_ENDURE])// It has been confirmed that eddga card grants 1 MDEF, not 0, not 10, but 1.
		mdef += (sc->data[SC_ENDURE]->val4 == 0) ? sc->data[SC_ENDURE]->val1 : 1;
	if(sc->data[SC_STONEHARDSKIN])// Final MDEF increase divided by 10 since were using classic (pre-renewal) mechanics. [Rytech]
		mdef += sc->data[SC_STONEHARDSKIN]->val1;
	if(sc->data[SC_WATER_BARRIER])
		mdef += sc->data[SC_WATER_BARRIER]->val2;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		mdef += 25*mdef/100;
	if(sc->data[SC_FREEZE])
		mdef += 25*mdef/100;
	if(sc->data[SC_ANALYZE])
		mdef -= mdef * ( 14 * sc->data[SC_ANALYZE]->val1 ) / 100;
	if(sc->data[SC_SYMPHONY_LOVE])
		mdef += mdef * sc->data[SC_SYMPHONY_LOVE]->val2 / 100;
	if(sc->data[SC_GENTLETOUCH_CHANGE] && sc->data[SC_GENTLETOUCH_CHANGE]->val4)
		mdef -= mdef * sc->data[SC_GENTLETOUCH_CHANGE]->val4 / 100;
	if(sc->data[SC_NEUTRALBARRIER] )
		mdef += mdef * (5 * sc->data[SC_NEUTRALBARRIER]->val1 + 10) / 100;
	if (sc->data[SC_ODINS_POWER])
		mdef -= 20;
	if(sc->data[SC_BURNING])
		mdef -= mdef *25 / 100;

	return (defType)cap_value(mdef,DEFTYPE_MIN,DEFTYPE_MAX);
}

signed short status_calc_mdef2(struct block_list *bl, struct status_change *sc, int mdef2, bool viewable)
{
	if(!sc || !sc->count)
#ifdef RENEWAL
		return (short)cap_value(mdef2,SHRT_MIN,SHRT_MAX);
#else
		return (short)cap_value(mdef2,1,SHRT_MAX);
#endif

	if( !viewable ){
		/* some statuses that are hidden in the status window */
#ifdef RENEWAL
		if(sc && sc->data[SC_ASSUMPTIO])
			mdef2 <<= 1;
		return (short)cap_value(mdef2,SHRT_MIN,SHRT_MAX);
#else
		return (short)cap_value(mdef2,1,SHRT_MAX);
#endif
	}

	if (sc->data[SC_BERSERK])
		return 0;
	if(sc->data[SC_SKA])
		return 90;
	if(sc->data[SC_MINDBREAKER])
		mdef2 -= mdef2 * sc->data[SC_MINDBREAKER]->val3/100;
	if(sc->data[SC_ANALYZE])
		mdef2 -= mdef2 * ( 14 * sc->data[SC_ANALYZE]->val1 ) / 100;

#ifdef RENEWAL
	return (short)cap_value(mdef2,SHRT_MIN,SHRT_MAX);
#else
	return (short)cap_value(mdef2,1,SHRT_MAX);
#endif
}

unsigned short status_calc_speed(struct block_list *bl, struct status_change *sc, int speed)
{
	TBL_PC* sd = BL_CAST(BL_PC, bl);
	int speed_rate;

	if( sc == NULL || ( sd && sd->state.permanent_speed ) )
		return (unsigned short)cap_value(speed,MIN_WALK_SPEED,MAX_WALK_SPEED);

	if( sd && sd->ud.skilltimer != INVALID_TIMER && (pc->checkskill(sd,SA_FREECAST) > 0 || sd->ud.skill_id == LG_EXEEDBREAK) )
	{
		if( sd->ud.skill_id == LG_EXEEDBREAK )
			speed_rate = 160 - 10 * sd->ud.skill_lv;
		else
			speed_rate = 175 - 5 * pc->checkskill(sd,SA_FREECAST);
	}
	else
	{
		speed_rate = 100;

		//GetMoveHasteValue2()
		{
			int val = 0;

			if( sc->data[SC_FUSION] )
				val = 25;
			else if( sd ) {
				if( pc_isriding(sd) || sd->sc.option&(OPTION_DRAGON) || sd->sc.data[SC_ALL_RIDING] )
					val = 25;//Same bonus
				else if( pc_isridingwug(sd) )
					val = 15 + 5 * pc->checkskill(sd, RA_WUGRIDER);
				else if( pc_ismadogear(sd) ) {
					val = (- 10 * (5 - pc->checkskill(sd,NC_MADOLICENCE)));
					if( sc->data[SC_ACCELERATION] )
						val += 25;
				}
			}

			speed_rate -= val;
		}

		//GetMoveSlowValue()
		{
			int val = 0;

			if( sd && sc->data[SC_HIDING] && pc->checkskill(sd,RG_TUNNELDRIVE) > 0 )
				val = 120 - 6 * pc->checkskill(sd,RG_TUNNELDRIVE);
			else
				if( sd && sc->data[SC_CHASEWALK] && sc->data[SC_CHASEWALK]->val3 < 0 )
					val = sc->data[SC_CHASEWALK]->val3;
				else
				{
					// Longing for Freedom cancels song/dance penalty
					if( sc->data[SC_LONGING] )
						val = max( val, 50 - 10 * sc->data[SC_LONGING]->val1 );
					else
						if( sd && sc->data[SC_DANCING] )
							val = max( val, 500 - (40 + 10 * (sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_BARDDANCER)) * pc->checkskill(sd,(sd->status.sex?BA_MUSICALLESSON:DC_DANCINGLESSON)) );

					if( sc->data[SC_DEC_AGI] )
						val = max( val, 25 );
					if( sc->data[SC_QUAGMIRE] || sc->data[SC_HALLUCINATIONWALK_POSTDELAY] )
						val = max( val, 50 );
					if( sc->data[SC_DONTFORGETME] )
						val = max( val, sc->data[SC_DONTFORGETME]->val3 );
					if( sc->data[SC_CURSE] )
						val = max( val, 300 );
					if( sc->data[SC_CHASEWALK] )
						val = max( val, sc->data[SC_CHASEWALK]->val3 );
					if( sc->data[SC_WEDDING] )
						val = max( val, 100 );
					if( sc->data[SC_JOINTBEAT] && sc->data[SC_JOINTBEAT]->val2&(BREAK_ANKLE|BREAK_KNEE) )
						val = max( val, (sc->data[SC_JOINTBEAT]->val2&BREAK_ANKLE ? 50 : 0) + (sc->data[SC_JOINTBEAT]->val2&BREAK_KNEE ? 30 : 0) );
					if( sc->data[SC_CLOAKING] && (sc->data[SC_CLOAKING]->val4&1) == 0 )
						val = max( val, sc->data[SC_CLOAKING]->val1 < 3 ? 300 : 30 - 3 * sc->data[SC_CLOAKING]->val1 );
					if( sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_ENEMY )
						val = max( val, 75 );
					if( sc->data[SC_SLOWDOWN] ) // Slow Potion
						val = max( val, 100 );
					if( sc->data[SC_GS_GATLINGFEVER] )
						val = max( val, 100 );
					if( sc->data[SC_NJ_SUITON] )
						val = max( val, sc->data[SC_NJ_SUITON]->val3 );
					if( sc->data[SC_SWOO] )
						val = max( val, 300 );
					if( sc->data[SC_FROSTMISTY] )
						val = max( val, 50 );
					if( sc->data[SC_CAMOUFLAGE] && (sc->data[SC_CAMOUFLAGE]->val3&1) == 0 )
						val = max( val, sc->data[SC_CAMOUFLAGE]->val1 < 3 ? 0 : 25 * (5 - sc->data[SC_CAMOUFLAGE]->val1) );
					if( sc->data[SC__GROOMY] )
						val = max( val, sc->data[SC__GROOMY]->val2);
					if( sc->data[SC_GLOOMYDAY] )
						val = max( val, sc->data[SC_GLOOMYDAY]->val3 );	// Should be 50 (-50% speed)
					if( sc->data[SC_STEALTHFIELD_MASTER] )
						val = max( val, 30 );
					if( sc->data[SC_BANDING_DEFENCE] )
						val = max( val, sc->data[SC_BANDING_DEFENCE]->val1 );//+90% walking speed.
					if( sc->data[SC_ROCK_CRUSHER_ATK] )
						val = max( val, sc->data[SC_ROCK_CRUSHER_ATK]->val2 );
					if( sc->data[SC_POWER_OF_GAIA] )
						val = max( val, sc->data[SC_POWER_OF_GAIA]->val2 );
					if( sc->data[SC_MELON_BOMB] )
						val = max( val, sc->data[SC_MELON_BOMB]->val1 );

					if( sc->data[SC_MARSHOFABYSS] ) // It stacks to other statuses so always put this at the end.
						val = max( 50, val + 10 * sc->data[SC_MARSHOFABYSS]->val1 );

					if( sd && sd->bonus.speed_rate + sd->bonus.speed_add_rate > 0 ) // permanent item-based speedup
						val = max( val, sd->bonus.speed_rate + sd->bonus.speed_add_rate );
				}

				speed_rate += val;
		}

		//GetMoveHasteValue1()
		{
			int val = 0;

			if( sc->data[SC_MOVHASTE_INFINITY] ) //FIXME: used both by NPC_AGIUP and Speed Potion script
				val = max( val, 50 );
			if( sc->data[SC_INC_AGI] )
				val = max( val, 25 );
			if( sc->data[SC_WINDWALK] )
				val = max( val, 2 * sc->data[SC_WINDWALK]->val1 );
			if( sc->data[SC_CARTBOOST] )
				val = max( val, 20 );
			if( sd && (sd->class_&MAPID_UPPERMASK) == MAPID_ASSASSIN && pc->checkskill(sd,TF_MISS) > 0 )
				val = max( val, 1 * pc->checkskill(sd,TF_MISS) );
			if( sc->data[SC_CLOAKING] && (sc->data[SC_CLOAKING]->val4&1) == 1 )
				val = max( val, sc->data[SC_CLOAKING]->val1 >= 10 ? 25 : 3 * sc->data[SC_CLOAKING]->val1 - 3 );
			if (sc->data[SC_BERSERK])
				val = max( val, 25 );
			if( sc->data[SC_RUN] )
				val = max( val, 55 );
			if( sc->data[SC_HLIF_AVOID] )
				val = max( val, 10 * sc->data[SC_HLIF_AVOID]->val1 );
			if( sc->data[SC_INVINCIBLE] && !sc->data[SC_INVINCIBLEOFF] )
				val = max( val, 75 );
			if( sc->data[SC_CLOAKINGEXCEED] )
				val = max( val, sc->data[SC_CLOAKINGEXCEED]->val3);
			if( sc->data[SC_HOVERING] )
				val = max( val, 10 );
			if( sc->data[SC_GN_CARTBOOST] )
				val = max( val, sc->data[SC_GN_CARTBOOST]->val2 );
			if( sc->data[SC_SWING] )
				val = max( val, sc->data[SC_SWING]->val3 );
			if( sc->data[SC_WIND_STEP_OPTION] )
				val = max( val, sc->data[SC_WIND_STEP_OPTION]->val2 );
			if( sc->data[SC_FULL_THROTTLE] )
				val = max( val, 30);
			//FIXME: official items use a single bonus for this [ultramage]
			if( sc->data[SC_MOVHASTE_HORSE] ) // temporary item-based speedup
				val = max( val, 25 );
			if( sd && sd->bonus.speed_rate + sd->bonus.speed_add_rate < 0 ) // permanent item-based speedup
				val = max( val, -(sd->bonus.speed_rate + sd->bonus.speed_add_rate) );

			speed_rate -= val;
		}

		if( speed_rate < 40 )
			speed_rate = 40;
	}

	//GetSpeed()
	{
		if( sd && pc_iscarton(sd) )
			speed += speed * (50 - 5 * pc->checkskill(sd,MC_PUSHCART)) / 100;
		if( sc->data[SC_PARALYSE] )
			speed += speed * 50 / 100;
		if( sc->data[SC_REBOUND] )
			speed += max(speed, 100);
		if( speed_rate != 100 )
			speed = speed * speed_rate / 100;
		if( sc->data[SC_STEELBODY] )
			speed = 200;
		if( sc->data[SC_DEFENDER] )
			speed = max(speed, 200);
		if( sc->data[SC_WALKSPEED] && sc->data[SC_WALKSPEED]->val1 > 0 ) // ChangeSpeed
			speed = speed * 100 / sc->data[SC_WALKSPEED]->val1;

	}

	return (unsigned short)cap_value(speed,MIN_WALK_SPEED,MAX_WALK_SPEED);
}

// flag&1 - fixed value [malufett]
// flag&2 - percentage value
short status_calc_aspd(struct block_list *bl, struct status_change *sc, short flag) {
#ifdef RENEWAL_ASPD
	int i, pots = 0, skills1 = 0, skills2 = 0;

	if(!sc || !sc->count)
		return 0;

	if(sc->data[i=SC_ATTHASTE_INFINITY] ||
		sc->data[i=SC_ATTHASTE_POTION3] ||
		sc->data[i=SC_ATTHASTE_POTION2] ||
		sc->data[i=SC_ATTHASTE_POTION1])
		pots += sc->data[i]->val1;

	if( !sc->data[SC_QUAGMIRE] ){
		if(sc->data[SC_TWOHANDQUICKEN] && skills1 < 7)
			skills1 = 7;

		if(sc->data[SC_ONEHANDQUICKEN] && skills1 < 7) skills1 = 7;

		if(sc->data[SC_MER_QUICKEN] && skills1 < 7) // needs more info
			skills1 = 7;

		if(sc->data[SC_ADRENALINE2] && skills1 < 6)
			skills1 = 6;

		if(sc->data[SC_ADRENALINE] && skills1 < 7)
			skills1 = 7;

		if(sc->data[SC_SPEARQUICKEN] && skills1 < 7)
			skills1 = 7;

		if(sc->data[SC_HLIF_FLEET] && skills1 < 5)
			skills1 = 5;
	}

	if((sc->data[SC_BERSERK]) &&	skills1 < 15)
		skills1 = 15;
	else if(sc->data[SC_GS_MADNESSCANCEL] && skills1 < 20)
		skills1 = 20;

	if(sc->data[SC_DONTFORGETME])
		skills2 -= sc->data[SC_DONTFORGETME]->val2;
	if(sc->data[SC_LONGING])
		skills2 -= sc->data[SC_LONGING]->val2;
	if(sc->data[SC_STEELBODY])
		skills2 -= 25;
	if(sc->data[SC_SKA])
		skills2 -= 25;
	if(sc->data[SC_DEFENDER])
		skills2 -= sc->data[SC_DEFENDER]->val4 / 10;
	if(sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_ENEMY) // needs more info
		skills2 -= 25;
	if(sc->data[SC_GRAVITATION])
		skills2 -= sc->data[SC_GRAVITATION]->val2; // needs more info
	if(sc->data[SC_JOINTBEAT]) { // needs more info
		if( sc->data[SC_JOINTBEAT]->val2&BREAK_WRIST )
			skills2 -= 25;
		if( sc->data[SC_JOINTBEAT]->val2&BREAK_KNEE )
			skills2 -= 10;
	}
	if( sc->data[SC_FROSTMISTY] )
		skills2 -= 15;
	if( sc->data[SC_HALLUCINATIONWALK_POSTDELAY] )
		skills2 -= 50;
	if( sc->data[SC_PARALYSE] )
		skills2 -= 10;
	if( sc->data[SC__BODYPAINT] )
		skills2 -= sc->data[SC__BODYPAINT]->val1;
	if( sc->data[SC__INVISIBILITY] )
		skills2 -= sc->data[SC__INVISIBILITY]->val2 ;
	if( sc->data[SC__GROOMY] )
		skills2 -= sc->data[SC__GROOMY]->val2;
	if( sc->data[SC_GLOOMYDAY] )
		skills2 -= ( 15 + 5 * sc->data[SC_GLOOMYDAY]->val1 );
	if( sc->data[SC_EARTHDRIVE] )
		skills2 -= 25;
	if( sc->data[SC_MELON_BOMB] )
		skills2 -= sc->data[SC_MELON_BOMB]->val1;

	if( sc->data[SC_SWING] )
		skills2 += sc->data[SC_SWING]->val3;
	if( sc->data[SC_DANCE_WITH_WUG] )
		skills2 += sc->data[SC_DANCE_WITH_WUG]->val3;
	if( sc->data[SC_GENTLETOUCH_CHANGE] )
		skills2 += sc->data[SC_GENTLETOUCH_CHANGE]->val3;
	if( sc->data[SC_BOOST500] )
		skills2 += sc->data[SC_BOOST500]->val1;
	if( sc->data[SC_EXTRACT_SALAMINE_JUICE] )
		skills2 += sc->data[SC_EXTRACT_SALAMINE_JUICE]->val1;
	if( sc->data[SC_INCASPDRATE] )
		skills2 += sc->data[SC_INCASPDRATE]->val1;
	if( sc->data[SC_GS_GATLINGFEVER] )
		skills2 += sc->data[SC_GS_GATLINGFEVER]->val1;
	if( sc->data[SC_STAR_COMFORT] )
		skills2 += 3 * sc->data[SC_STAR_COMFORT]->val1;
	if( sc->data[SC_ASSNCROS] && !skills1){
		if (bl->type!=BL_PC)
			skills2 += sc->data[SC_ASSNCROS]->val2;
		else
			switch(((TBL_PC*)bl)->status.weapon)
		{
			case W_BOW:
			case W_REVOLVER:
			case W_RIFLE:
			case W_GATLING:
			case W_SHOTGUN:
			case W_GRENADE:
				break;
			default:
				skills2 += sc->data[SC_ASSNCROS]->val2;
		}
	}
	return ( flag&1? (skills1 + pots) : skills2 );
#else
	return 0;
#endif
}

short status_calc_fix_aspd(struct block_list *bl, struct status_change *sc, int aspd) {
	if (!sc || !sc->count)
		return cap_value(aspd, 0, 2000);

	if (!sc->data[SC_QUAGMIRE]) {
		if (sc->data[SC_OVERED_BOOST])
			aspd = 2000 - sc->data[SC_OVERED_BOOST]->val3*10;
	}

	if ((sc->data[SC_GUST_OPTION] || sc->data[SC_BLAST_OPTION]
	|| sc->data[SC_WILD_STORM_OPTION]))
		aspd -= 50; // +5 ASPD
	if( sc && sc->data[SC_FIGHTINGSPIRIT] && sc->data[SC_FIGHTINGSPIRIT]->val2 )
		aspd -= (bl->type==BL_PC?pc->checkskill((TBL_PC *)bl, RK_RUNEMASTERY):10) / 10 * 40;
	if( sc && sc->data[SC_MTF_ASPD] )
		aspd -= 10;

	return cap_value(aspd, 0, 2000); // will be recap for proper bl anyway
}

/// Calculates an object's ASPD modifier (alters the base amotion value).
/// Note that the scale of aspd_rate is 1000 = 100%.
short status_calc_aspd_rate(struct block_list *bl, struct status_change *sc, int aspd_rate)
{
	int i;

	if(!sc || !sc->count)
		return cap_value(aspd_rate,0,SHRT_MAX);

	if( !sc->data[SC_QUAGMIRE] ){
		int max = 0;
		if(sc->data[SC_STAR_COMFORT])
			max = sc->data[SC_STAR_COMFORT]->val2;

		if(sc->data[SC_TWOHANDQUICKEN] &&
			max < sc->data[SC_TWOHANDQUICKEN]->val2)
			max = sc->data[SC_TWOHANDQUICKEN]->val2;

		if(sc->data[SC_ONEHANDQUICKEN] &&
			max < sc->data[SC_ONEHANDQUICKEN]->val2)
			max = sc->data[SC_ONEHANDQUICKEN]->val2;

		if(sc->data[SC_MER_QUICKEN] &&
			max < sc->data[SC_MER_QUICKEN]->val2)
			max = sc->data[SC_MER_QUICKEN]->val2;

		if(sc->data[SC_ADRENALINE2] &&
			max < sc->data[SC_ADRENALINE2]->val3)
			max = sc->data[SC_ADRENALINE2]->val3;

		if(sc->data[SC_ADRENALINE] &&
			max < sc->data[SC_ADRENALINE]->val3)
			max = sc->data[SC_ADRENALINE]->val3;

		if(sc->data[SC_SPEARQUICKEN] &&
			max < sc->data[SC_SPEARQUICKEN]->val2)
			max = sc->data[SC_SPEARQUICKEN]->val2;

		if(sc->data[SC_GS_GATLINGFEVER] &&
			max < sc->data[SC_GS_GATLINGFEVER]->val2)
			max = sc->data[SC_GS_GATLINGFEVER]->val2;

		if(sc->data[SC_HLIF_FLEET] &&
			max < sc->data[SC_HLIF_FLEET]->val2)
			max = sc->data[SC_HLIF_FLEET]->val2;

		if(sc->data[SC_ASSNCROS] &&
			max < sc->data[SC_ASSNCROS]->val2)
		{
			if (bl->type!=BL_PC)
				max = sc->data[SC_ASSNCROS]->val2;
			else
				switch(((TBL_PC*)bl)->status.weapon)
			{
				case W_BOW:
				case W_REVOLVER:
				case W_RIFLE:
				case W_GATLING:
				case W_SHOTGUN:
				case W_GRENADE:
					break;
				default:
					max = sc->data[SC_ASSNCROS]->val2;
			}
		}
		aspd_rate -= max;

		if(sc->data[SC_BERSERK])
			aspd_rate -= 300;
		else if(sc->data[SC_GS_MADNESSCANCEL])
			aspd_rate -= 200;
	}

	if( sc->data[i=SC_ATTHASTE_INFINITY] ||
		sc->data[i=SC_ATTHASTE_POTION3] ||
		sc->data[i=SC_ATTHASTE_POTION2] ||
		sc->data[i=SC_ATTHASTE_POTION1] )
		aspd_rate -= sc->data[i]->val2;

	if(sc->data[SC_DONTFORGETME])
		aspd_rate += 10 * sc->data[SC_DONTFORGETME]->val2;
	if(sc->data[SC_LONGING])
		aspd_rate += sc->data[SC_LONGING]->val2;
	if(sc->data[SC_STEELBODY])
		aspd_rate += 250;
	if(sc->data[SC_SKA])
		aspd_rate += 250;
	if(sc->data[SC_DEFENDER])
		aspd_rate += sc->data[SC_DEFENDER]->val4;
	if(sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_ENEMY)
		aspd_rate += 250;
	if(sc->data[SC_GRAVITATION])
		aspd_rate += sc->data[SC_GRAVITATION]->val2;
	if(sc->data[SC_JOINTBEAT]) {
		if( sc->data[SC_JOINTBEAT]->val2&BREAK_WRIST )
			aspd_rate += 250;
		if( sc->data[SC_JOINTBEAT]->val2&BREAK_KNEE )
			aspd_rate += 100;
	}
	if( sc->data[SC_FROSTMISTY] )
		aspd_rate += 150;
	if( sc->data[SC_HALLUCINATIONWALK_POSTDELAY] )
		aspd_rate += 500;
	if( sc->data[SC_FIGHTINGSPIRIT] && sc->data[SC_FIGHTINGSPIRIT]->val2 )
		aspd_rate -= sc->data[SC_FIGHTINGSPIRIT]->val2;
	if( sc->data[SC_PARALYSE] )
		aspd_rate += 100;
	if( sc->data[SC__BODYPAINT] )
		aspd_rate += 10 * 5 * sc->data[SC__BODYPAINT]->val1;
	if( sc->data[SC__INVISIBILITY] )
		aspd_rate += sc->data[SC__INVISIBILITY]->val2 * 10 ;
	if( sc->data[SC__GROOMY] )
		aspd_rate += sc->data[SC__GROOMY]->val2 * 10;
	if( sc->data[SC_SWING] )
		aspd_rate -= sc->data[SC_SWING]->val3 * 10;
	if( sc->data[SC_DANCE_WITH_WUG] )
		aspd_rate -= sc->data[SC_DANCE_WITH_WUG]->val3 * 10;
	if( sc->data[SC_GLOOMYDAY] )
		aspd_rate += ( 15 + 5 * sc->data[SC_GLOOMYDAY]->val1 );
	if( sc->data[SC_EARTHDRIVE] )
		aspd_rate += 250;
	if( sc->data[SC_GENTLETOUCH_CHANGE] )
		aspd_rate -= sc->data[SC_GENTLETOUCH_CHANGE]->val3 * 10;
	if( sc->data[SC_MELON_BOMB] )
		aspd_rate += sc->data[SC_MELON_BOMB]->val1 * 10;
	if( sc->data[SC_BOOST500] )
		aspd_rate -= sc->data[SC_BOOST500]->val1 *10;
	if( sc->data[SC_EXTRACT_SALAMINE_JUICE] )
		aspd_rate -= sc->data[SC_EXTRACT_SALAMINE_JUICE]->val1 * 10;
	if( sc->data[SC_INCASPDRATE] )
		aspd_rate -= sc->data[SC_INCASPDRATE]->val1 * 10;
	if( sc->data[SC_PAIN_KILLER])
		aspd_rate += sc->data[SC_PAIN_KILLER]->val2 * 10;
	if( sc->data[SC_GOLDENE_FERSE])
		aspd_rate -= sc->data[SC_GOLDENE_FERSE]->val3 * 10;

	return (short)cap_value(aspd_rate,0,SHRT_MAX);
}

unsigned short status_calc_dmotion(struct block_list *bl, struct status_change *sc, int dmotion) {
	// It has been confirmed on official servers that MvP mobs have no dmotion even without endure
	if( bl->type == BL_MOB && (((TBL_MOB*)bl)->status.mode&MD_BOSS) )
		return 0;

	if( !sc || !sc->count || map_flag_gvg2(bl->m) || map->list[bl->m].flag.battleground )
		return cap_value(dmotion,0,USHRT_MAX);

	if( sc->data[SC_ENDURE] )
		return 0;
	if( sc->data[SC_RUN] || sc->data[SC_WUGDASH] )
		return 0;

	return (unsigned short)cap_value(dmotion,0,USHRT_MAX);
}

unsigned int status_calc_maxhp(struct block_list *bl, struct status_change *sc, uint64 maxhp)
{
	if(!sc || !sc->count)
		return (unsigned int)cap_value(maxhp,1,UINT_MAX);

	if(sc->data[SC_INCMHPRATE])
		maxhp += maxhp * sc->data[SC_INCMHPRATE]->val1/100;
	if(sc->data[SC_INCMHP])
		maxhp += (sc->data[SC_INCMHP]->val1);
	if(sc->data[SC_APPLEIDUN])
		maxhp += maxhp * sc->data[SC_APPLEIDUN]->val2/100;
	if(sc->data[SC_DELUGE])
		maxhp += maxhp * sc->data[SC_DELUGE]->val2/100;
	if(sc->data[SC_BERSERK])
		maxhp += maxhp * 2;
	if(sc->data[SC_MARIONETTE_MASTER])
		maxhp -= 1000;
	if(sc->data[SC_SOLID_SKIN_OPTION])
		maxhp += 2000;// Fix amount.
	if(sc->data[SC_POWER_OF_GAIA])
		maxhp += 3000;
	if(sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 2)
		maxhp += 500;

	if(sc->data[SC_MER_HP])
		maxhp += maxhp * sc->data[SC_MER_HP]->val2/100;

	if(sc->data[SC_EPICLESIS])
		maxhp += maxhp * 5 * sc->data[SC_EPICLESIS]->val1 / 100;
	if(sc->data[SC_VENOMBLEED])
		maxhp -= maxhp * 15 / 100;
	if(sc->data[SC__WEAKNESS])
		maxhp -= maxhp * sc->data[SC__WEAKNESS]->val2 / 100;
	if(sc->data[SC_LERADS_DEW])
		maxhp += sc->data[SC_LERADS_DEW]->val3;
	if(sc->data[SC_BEYOND_OF_WARCRY])
		maxhp -= maxhp * sc->data[SC_BEYOND_OF_WARCRY]->val4 / 100;
	if(sc->data[SC_FORCEOFVANGUARD])
		maxhp += maxhp * 3 * sc->data[SC_FORCEOFVANGUARD]->val1 / 100;
	if(sc->data[SC_INSPIRATION])
		maxhp += maxhp * 5 * sc->data[SC_INSPIRATION]->val1 / 100 + 600 * sc->data[SC_INSPIRATION]->val1;
	if(sc->data[SC_RAISINGDRAGON])
		maxhp += maxhp * (2 + sc->data[SC_RAISINGDRAGON]->val1) / 100;
	if(sc->data[SC_GENTLETOUCH_CHANGE]) // Max HP decrease: [Skill Level x 4] %
		maxhp -= maxhp * (4 * sc->data[SC_GENTLETOUCH_CHANGE]->val1) / 100;
	if(sc->data[SC_GENTLETOUCH_REVITALIZE])// Max HP increase: [Skill Level x 2] %
		maxhp += maxhp * (2 * sc->data[SC_GENTLETOUCH_REVITALIZE]->val1) / 100;
	if(sc->data[SC_MUSTLE_M])
		maxhp += maxhp * sc->data[SC_MUSTLE_M]->val1/100;
	if(sc->data[SC_MYSTERIOUS_POWDER])
		maxhp -= sc->data[SC_MYSTERIOUS_POWDER]->val1 / 100;
	if(sc->data[SC_PETROLOGY_OPTION])
		maxhp += maxhp * sc->data[SC_PETROLOGY_OPTION]->val2 / 100;
	if(sc->data[SC_CURSED_SOIL_OPTION])
		maxhp += maxhp * sc->data[SC_CURSED_SOIL_OPTION]->val2 / 100;
	if(sc->data[SC_UPHEAVAL_OPTION])
		maxhp += maxhp * sc->data[SC_UPHEAVAL_OPTION]->val3 / 100;
	if (sc->data[SC_ANGRIFFS_MODUS])
		maxhp += maxhp * 5 * sc->data[SC_ANGRIFFS_MODUS]->val1 /100;
	if (sc->data[SC_GOLDENE_FERSE])
		maxhp += maxhp * sc->data[SC_GOLDENE_FERSE]->val2 / 100;
	if(sc->data[SC_FRIGG_SONG])
		maxhp += maxhp * sc->data[SC_FRIGG_SONG]->val2 / 100;

	return (unsigned int)cap_value(maxhp,1,UINT_MAX);
}

unsigned int status_calc_maxsp(struct block_list *bl, struct status_change *sc, unsigned int maxsp)
{
	if(!sc || !sc->count)
		return cap_value(maxsp,1,UINT_MAX);

	if(sc->data[SC_INCMSPRATE])
		maxsp += maxsp * sc->data[SC_INCMSPRATE]->val1/100;
	if(sc->data[SC_INCMSP])
		maxsp += (sc->data[SC_INCMSP]->val1);
	if(sc->data[SC_SERVICEFORYOU])
		maxsp += maxsp * sc->data[SC_SERVICEFORYOU]->val2/100;
	if(sc->data[SC_MER_SP])
		maxsp += maxsp * sc->data[SC_MER_SP]->val2/100;
	if(sc->data[SC_RAISINGDRAGON])
		maxsp += maxsp * (2 + sc->data[SC_RAISINGDRAGON]->val1) / 100;
	if(sc->data[SC_LIFE_FORCE_F])
		maxsp += maxsp * sc->data[SC_LIFE_FORCE_F]->val1/100;
	if(sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 3)
		maxsp += 50;

	return cap_value(maxsp,1,UINT_MAX);
}

unsigned char status_calc_element(struct block_list *bl, struct status_change *sc, int element)
{
	if(!sc || !sc->count)
		return element;

	if(sc->data[SC_FREEZE])
		return ELE_WATER;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		return ELE_EARTH;
	if(sc->data[SC_BENEDICTIO])
		return ELE_HOLY;
	if(sc->data[SC_PROPERTYUNDEAD])
		return ELE_UNDEAD;
	if(sc->data[SC_ARMOR_PROPERTY])
		return sc->data[SC_ARMOR_PROPERTY]->val2;
	if(sc->data[SC_SHAPESHIFT])
		return sc->data[SC_SHAPESHIFT]->val2;

	return (unsigned char)cap_value(element,0,UCHAR_MAX);
}

unsigned char status_calc_element_lv(struct block_list *bl, struct status_change *sc, int lv)
{
	if(!sc || !sc->count)
		return lv;

	if(sc->data[SC_FREEZE])
		return 1;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		return 1;
	if(sc->data[SC_BENEDICTIO])
		return 1;
	if(sc->data[SC_PROPERTYUNDEAD])
		return 1;
	if(sc->data[SC_ARMOR_PROPERTY])
		return sc->data[SC_ARMOR_PROPERTY]->val1;
	if(sc->data[SC_SHAPESHIFT])
		return 1;
	if(sc->data[SC__INVISIBILITY])
		return 1;

	return (unsigned char)cap_value(lv,1,4);
}


unsigned char status_calc_attack_element(struct block_list *bl, struct status_change *sc, int element)
{
	if(!sc || !sc->count)
		return element;
	if(sc->data[SC_ENCHANTARMS])
		return sc->data[SC_ENCHANTARMS]->val2;
	if(sc->data[SC_PROPERTYWATER]
	|| (sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 2) )
		return ELE_WATER;
	if(sc->data[SC_PROPERTYGROUND]
	|| (sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 2) )
		return ELE_EARTH;
	if(sc->data[SC_PROPERTYFIRE]
	|| (sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 2) )
		return ELE_FIRE;
	if(sc->data[SC_PROPERTYWIND]
	|| (sc->data[SC_WIND_INSIGNIA] && sc->data[SC_WIND_INSIGNIA]->val1 == 2) )
		return ELE_WIND;
	if(sc->data[SC_ENCHANTPOISON])
		return ELE_POISON;
	if(sc->data[SC_ASPERSIO])
		return ELE_HOLY;
	if(sc->data[SC_PROPERTYDARK])
		return ELE_DARK;
	if(sc->data[SC_PROPERTYTELEKINESIS] || sc->data[SC__INVISIBILITY])
		return ELE_GHOST;
	if(sc->data[SC_TIDAL_WEAPON_OPTION] || sc->data[SC_TIDAL_WEAPON] )
		return ELE_WATER;
	if(sc->data[SC_PYROCLASTIC])
		return ELE_FIRE;
	return (unsigned char)cap_value(element,0,UCHAR_MAX);
}

unsigned short status_calc_mode(struct block_list *bl, struct status_change *sc, int mode)
{
	if(!sc || !sc->count)
		return mode;
	if(sc->data[SC_MODECHANGE]) {
		if (sc->data[SC_MODECHANGE]->val2)
			mode = sc->data[SC_MODECHANGE]->val2; //Set mode
		if (sc->data[SC_MODECHANGE]->val3)
			mode|= sc->data[SC_MODECHANGE]->val3; //Add mode
		if (sc->data[SC_MODECHANGE]->val4)
			mode&=~sc->data[SC_MODECHANGE]->val4; //Del mode
	}
	return cap_value(mode,0,USHRT_MAX);
}

const char* status_get_name(struct block_list *bl) {
	nullpo_ret(bl);
	switch (bl->type) {
		case BL_PC:  return ((TBL_PC *)bl)->fakename[0] != '\0' ? ((TBL_PC*)bl)->fakename : ((TBL_PC*)bl)->status.name;
		case BL_MOB: return ((TBL_MOB*)bl)->name;
		case BL_PET: return ((TBL_PET*)bl)->pet.name;
		case BL_HOM: return ((TBL_HOM*)bl)->homunculus.name;
		case BL_NPC: return ((TBL_NPC*)bl)->name;
	}
	return "Unknown";
}

/*==========================================
* Get the class of the current bl
* return
*	0 = fail
*	class_id = success
*------------------------------------------*/
int status_get_class(struct block_list *bl) {
	nullpo_ret(bl);
	switch( bl->type ) {
		case BL_PC:  return ((TBL_PC*)bl)->status.class_;
		case BL_MOB: return ((TBL_MOB*)bl)->vd->class_; //Class used on all code should be the view class of the mob.
		case BL_PET: return ((TBL_PET*)bl)->pet.class_;
		case BL_HOM: return ((TBL_HOM*)bl)->homunculus.class_;
		case BL_MER: return ((TBL_MER*)bl)->mercenary.class_;
		case BL_NPC: return ((TBL_NPC*)bl)->class_;
		case BL_ELEM: return ((TBL_ELEM*)bl)->elemental.class_;
	}
	return 0;
}
/*==========================================
* Get the base level of the current bl
* return
*	1 = fail
*	level = success
*------------------------------------------*/
int status_get_lv(struct block_list *bl) {
	nullpo_ret(bl);
	switch (bl->type) {
		case BL_PC:  return ((TBL_PC*)bl)->status.base_level;
		case BL_MOB: return ((TBL_MOB*)bl)->level;
		case BL_PET: return ((TBL_PET*)bl)->pet.level;
		case BL_HOM: return ((TBL_HOM*)bl)->homunculus.level;
		case BL_MER: return ((TBL_MER*)bl)->db->lv;
		case BL_ELEM: return ((TBL_ELEM*)bl)->db->lv;
		case BL_NPC: return ((TBL_NPC*)bl)->level;
	}
	return 1;
}

struct regen_data *status_get_regen_data(struct block_list *bl)
{
	nullpo_retr(NULL, bl);
	switch (bl->type) {
		case BL_PC:  return &((TBL_PC*)bl)->regen;
		case BL_HOM: return &((TBL_HOM*)bl)->regen;
		case BL_MER: return &((TBL_MER*)bl)->regen;
		case BL_ELEM: return &((TBL_ELEM*)bl)->regen;
		default:
			return NULL;
	}
}

struct status_data *status_get_status_data(struct block_list *bl)
{
	nullpo_retr(&status->dummy, bl);

	switch (bl->type) {
		case BL_PC:  return &((TBL_PC*)bl)->battle_status;
		case BL_MOB: return &((TBL_MOB*)bl)->status;
		case BL_PET: return &((TBL_PET*)bl)->status;
		case BL_HOM: return &((TBL_HOM*)bl)->battle_status;
		case BL_MER: return &((TBL_MER*)bl)->battle_status;
		case BL_ELEM: return &((TBL_ELEM*)bl)->battle_status;
		case BL_NPC:  return ((mob->db_checkid(((TBL_NPC*)bl)->class_) == 0) ? &((TBL_NPC*)bl)->status : &status->dummy);
		default:
			return &status->dummy;
	}
}

struct status_data *status_get_base_status(struct block_list *bl)
{
	nullpo_retr(NULL, bl);
	switch (bl->type) {
		case BL_PC:  return &((TBL_PC*)bl)->base_status;
		case BL_MOB: return ((TBL_MOB*)bl)->base_status ? ((TBL_MOB*)bl)->base_status : &((TBL_MOB*)bl)->db->status;
		case BL_PET: return &((TBL_PET*)bl)->db->status;
		case BL_HOM: return &((TBL_HOM*)bl)->base_status;
		case BL_MER: return &((TBL_MER*)bl)->base_status;
		case BL_ELEM: return &((TBL_ELEM*)bl)->base_status;
		case BL_NPC:  return ((mob->db_checkid(((TBL_NPC*)bl)->class_) == 0) ? &((TBL_NPC*)bl)->status : NULL); 
		default:
			return NULL;
	}
}
defType status_get_def(struct block_list *bl) {
	struct unit_data *ud;
	struct status_data *st = status->get_status_data(bl);
	int def = st ? st->def : 0;
	ud = unit->bl2ud(bl);
	if (ud && ud->skilltimer != INVALID_TIMER)
		def -= def * skill->get_castdef(ud->skill_id)/100;

	return cap_value(def, DEFTYPE_MIN, DEFTYPE_MAX);
}

unsigned short status_get_speed(struct block_list *bl) {
	if(bl->type==BL_NPC)//Only BL with speed data but no status_data [Skotlex] 
		return ((struct npc_data *)bl)->speed; 
	return status->get_status_data(bl)->speed;
}

int status_get_party_id(struct block_list *bl) {
	nullpo_ret(bl);
	switch (bl->type) {
	case BL_PC:
		return ((TBL_PC*)bl)->status.party_id;
	case BL_PET:
		if (((TBL_PET*)bl)->msd)
			return ((TBL_PET*)bl)->msd->status.party_id;
		break;
	case BL_MOB: {
		struct mob_data *md=(TBL_MOB*)bl;
		if( md->master_id > 0 ) {
			struct map_session_data *msd;
			if (md->special_state.ai && (msd = map->id2sd(md->master_id)) != NULL)
				return msd->status.party_id;
			return -md->master_id;
		}
				 }
				 break;
	case BL_HOM:
		if (((TBL_HOM*)bl)->master)
			return ((TBL_HOM*)bl)->master->status.party_id;
		break;
	case BL_MER:
		if (((TBL_MER*)bl)->master)
			return ((TBL_MER*)bl)->master->status.party_id;
		break;
	case BL_SKILL:
		return ((TBL_SKILL*)bl)->group->party_id;
	case BL_ELEM:
		if (((TBL_ELEM*)bl)->master)
			return ((TBL_ELEM*)bl)->master->status.party_id;
		break;
	}
	return 0;
}

int status_get_guild_id(struct block_list *bl) {
	nullpo_ret(bl);
	switch (bl->type) {
	case BL_PC:
		return ((TBL_PC*)bl)->status.guild_id;
	case BL_PET:
		if (((TBL_PET*)bl)->msd)
			return ((TBL_PET*)bl)->msd->status.guild_id;
		break;
	case BL_MOB: {
		struct map_session_data *msd;
		struct mob_data *md = (struct mob_data *)bl;
		if (md->guardian_data)	//Guardian's guild [Skotlex]
			return md->guardian_data->guild_id;
		if (md->special_state.ai && (msd = map->id2sd(md->master_id)) != NULL)
			return msd->status.guild_id; //Alchemist's mobs [Skotlex]
				 }
				 break;
	case BL_HOM:
		if (((TBL_HOM*)bl)->master)
			return ((TBL_HOM*)bl)->master->status.guild_id;
		break;
	case BL_MER:
		if (((TBL_MER*)bl)->master)
			return ((TBL_MER*)bl)->master->status.guild_id;
		break;
	case BL_NPC:
		if (((TBL_NPC*)bl)->subtype == SCRIPT)
			return ((TBL_NPC*)bl)->u.scr.guild_id;
		break;
	case BL_SKILL:
		return ((TBL_SKILL*)bl)->group->guild_id;
	case BL_ELEM:
		if (((TBL_ELEM*)bl)->master)
			return ((TBL_ELEM*)bl)->master->status.guild_id;
		break;
	}
	return 0;
}

int status_get_emblem_id(struct block_list *bl) {
	nullpo_ret(bl);
	switch (bl->type) {
	case BL_PC:
		return ((TBL_PC*)bl)->guild_emblem_id;
	case BL_PET:
		if (((TBL_PET*)bl)->msd)
			return ((TBL_PET*)bl)->msd->guild_emblem_id;
		break;
	case BL_MOB: {
		struct map_session_data *msd;
		struct mob_data *md = (struct mob_data *)bl;
		if (md->guardian_data)	//Guardian's guild [Skotlex]
			return md->guardian_data->emblem_id;
		if (md->special_state.ai && (msd = map->id2sd(md->master_id)) != NULL)
			return msd->guild_emblem_id; //Alchemist's mobs [Skotlex]
				 }
				 break;
	case BL_HOM:
		if (((TBL_HOM*)bl)->master)
			return ((TBL_HOM*)bl)->master->guild_emblem_id;
		break;
	case BL_MER:
		if (((TBL_MER*)bl)->master)
			return ((TBL_MER*)bl)->master->guild_emblem_id;
		break;
	case BL_NPC:
		if (((TBL_NPC*)bl)->subtype == SCRIPT && ((TBL_NPC*)bl)->u.scr.guild_id > 0) {
			struct guild *g = guild->search(((TBL_NPC*)bl)->u.scr.guild_id);
			if (g)
				return g->emblem_id;
		}
		break;
	case BL_ELEM:
		if (((TBL_ELEM*)bl)->master)
			return ((TBL_ELEM*)bl)->master->guild_emblem_id;
		break;
	}
	return 0;
}

int status_get_mexp(struct block_list *bl)
{
	nullpo_ret(bl);
	if(bl->type==BL_MOB)
		return ((struct mob_data *)bl)->db->mexp;
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->db->mexp;
	return 0;
}
int status_get_race2(struct block_list *bl)
{
	nullpo_ret(bl);
	if(bl->type == BL_MOB)
		return ((struct mob_data *)bl)->db->race2;
	if(bl->type==BL_PET)
		return ((struct pet_data *)bl)->db->race2;
	return 0;
}

int status_isdead(struct block_list *bl) {
	nullpo_ret(bl);
	return status->get_status_data(bl)->hp == 0;
}

int status_isimmune(struct block_list *bl) {
	struct status_change *sc = status->get_sc(bl);
	if (sc && sc->data[SC_HERMODE])
		return 100;

	if (bl->type == BL_PC &&
		((TBL_PC*)bl)->special_state.no_magic_damage >= battle_config.gtb_sc_immunity)
		return ((TBL_PC*)bl)->special_state.no_magic_damage;
	return 0;
}

struct view_data* status_get_viewdata(struct block_list *bl)
{
	nullpo_retr(NULL, bl);
	switch (bl->type) {
		case BL_PC:  return &((TBL_PC*)bl)->vd;
		case BL_MOB: return ((TBL_MOB*)bl)->vd;
		case BL_PET: return &((TBL_PET*)bl)->vd;
		case BL_NPC: return ((TBL_NPC*)bl)->vd;
		case BL_HOM: return ((TBL_HOM*)bl)->vd;
		case BL_MER: return ((TBL_MER*)bl)->vd;
		case BL_ELEM: return ((TBL_ELEM*)bl)->vd;
	}
	return NULL;
}

void status_set_viewdata(struct block_list *bl, int class_)
{
	struct view_data* vd;
	nullpo_retv(bl);
	if (mob->db_checkid(class_) || mob->is_clone(class_))
		vd = mob->get_viewdata(class_);
	else if (npcdb_checkid(class_) || (bl->type == BL_NPC && class_ == WARP_CLASS))
		vd = npc->get_viewdata(class_);
	else if (homdb_checkid(class_))
		vd = homun->get_viewdata(class_);
	else if (mercenary->class(class_))
		vd = mercenary->get_viewdata(class_);
	else if (elemental->class(class_))
		vd = elemental->get_viewdata(class_);
	else
		vd = NULL;

	switch (bl->type) {
	case BL_PC:
		{
			TBL_PC* sd = (TBL_PC*)bl;
			if (pcdb_checkid(class_)) {
				if (sd->sc.option&OPTION_RIDING) {
					switch (class_) {	//Adapt class to a Mounted one.
						case JOB_KNIGHT:
							class_ = JOB_KNIGHT2;
							break;
						case JOB_CRUSADER:
							class_ = JOB_CRUSADER2;
							break;
						case JOB_LORD_KNIGHT:
							class_ = JOB_LORD_KNIGHT2;
							break;
						case JOB_PALADIN:
							class_ = JOB_PALADIN2;
							break;
						case JOB_BABY_KNIGHT:
							class_ = JOB_BABY_KNIGHT2;
							break;
						case JOB_BABY_CRUSADER:
							class_ = JOB_BABY_CRUSADER2;
							break;
					}
				}
				sd->vd.class_ = class_;
				clif->get_weapon_view(sd, &sd->vd.weapon, &sd->vd.shield);
				sd->vd.head_top = sd->status.head_top;
				sd->vd.head_mid = sd->status.head_mid;
				sd->vd.head_bottom = sd->status.head_bottom;
				sd->vd.hair_style = cap_value(sd->status.hair,0,battle_config.max_hair_style);
				sd->vd.hair_color = cap_value(sd->status.hair_color,0,battle_config.max_hair_color);
				sd->vd.cloth_color = cap_value(sd->status.clothes_color,0,battle_config.max_cloth_color);
				sd->vd.robe = sd->status.robe;
				sd->vd.sex = sd->status.sex;

				if ( sd->vd.cloth_color ) {
					if( sd->sc.option&OPTION_WEDDING && battle_config.wedding_ignorepalette )
						sd->vd.cloth_color = 0;
					if( sd->sc.option&OPTION_XMAS && battle_config.xmas_ignorepalette )
						sd->vd.cloth_color = 0;
					if( sd->sc.option&OPTION_SUMMER && battle_config.summer_ignorepalette )
						sd->vd.cloth_color = 0;
					if( sd->sc.option&OPTION_HANBOK && battle_config.hanbok_ignorepalette )
						sd->vd.cloth_color = 0;
					if( sd->sc.option&OPTION_OKTOBERFEST /* TODO: config? */ )
						sd->vd.cloth_color = 0;
				}
			} else if (vd)
				memcpy(&sd->vd, vd, sizeof(struct view_data));
			else
				ShowError("status_set_viewdata (PC): No view data for class %d\n", class_);
		}
		break;
	case BL_MOB:
		{
			TBL_MOB* md = (TBL_MOB*)bl;
			if (vd)
				md->vd = vd;
			else
				ShowError("status_set_viewdata (MOB): No view data for class %d\n", class_);
		}
		break;
	case BL_PET:
		{
			TBL_PET* pd = (TBL_PET*)bl;
			if (vd) {
				memcpy(&pd->vd, vd, sizeof(struct view_data));
				if (!pcdb_checkid(vd->class_)) {
					pd->vd.hair_style = battle_config.pet_hair_style;
					if(pd->pet.equip) {
						pd->vd.head_bottom = itemdb_viewid(pd->pet.equip);
						if (!pd->vd.head_bottom)
							pd->vd.head_bottom = pd->pet.equip;
					}
				}
			} else
				ShowError("status_set_viewdata (PET): No view data for class %d\n", class_);
		}
		break;
	case BL_NPC:
		{
			TBL_NPC* nd = (TBL_NPC*)bl;
			if (vd)
				nd->vd = vd;
			else
				ShowError("status_set_viewdata (NPC): No view data for class %d\n", class_);
		}
		break;
	case BL_HOM:		//[blackhole89]
		{
			struct homun_data *hd = (struct homun_data*)bl;
			if (vd)
				hd->vd = vd;
			else
				ShowError("status_set_viewdata (HOMUNCULUS): No view data for class %d\n", class_);
		}
		break;
	case BL_MER:
		{
			struct mercenary_data *md = (struct mercenary_data*)bl;
			if (vd)
				md->vd = vd;
			else
				ShowError("status_set_viewdata (MERCENARY): No view data for class %d\n", class_);
		}
		break;
	case BL_ELEM:
		{
			struct elemental_data *ed = (struct elemental_data*)bl;
			if (vd)
				ed->vd = vd;
			else
				ShowError("status_set_viewdata (ELEMENTAL): No view data for class %d\n", class_);
		}
		break;
	}
}

/// Returns the status_change data of bl or NULL if it doesn't exist.
struct status_change *status_get_sc(struct block_list *bl) {
	if( bl ) {
		switch (bl->type) {
		case BL_PC:  return &((TBL_PC*)bl)->sc;
		case BL_MOB: return &((TBL_MOB*)bl)->sc;
		case BL_NPC: return NULL;
		case BL_HOM: return &((TBL_HOM*)bl)->sc;
		case BL_MER: return &((TBL_MER*)bl)->sc;
		case BL_ELEM: return &((TBL_ELEM*)bl)->sc;
		}
	}
	return NULL;
}

void status_change_init(struct block_list *bl) {
	struct status_change *sc = status->get_sc(bl);
	nullpo_retv(sc);
	memset(sc, 0, sizeof (struct status_change));
}

//Applies SC defense to a given status change.
//Returns the adjusted duration based on flag values.
//the flag values are the same as in status->change_start.
int status_get_sc_def(struct block_list *src, struct block_list *bl, enum sc_type type, int rate, int tick, int flag) {
	//Percentual resistance: 10000 = 100% Resist
	//Example: 50% -> sc_def=5000 -> 25%; 5000ms -> tick_def=5000 -> 2500ms
	int sc_def = 0, tick_def = -1; //-1 = use sc_def
	//Linear resistance substracted from rate and tick after percentual resistance was applied
	//Example: 25% -> sc_def2=2000 -> 5%; 2500ms -> tick_def2=2000 -> 500ms
	int sc_def2 = 0, tick_def2 = -1; //-1 = use sc_def2 (pre-re only)

	struct status_data *st;
	struct status_change *sc;
	struct map_session_data *sd;

	nullpo_ret(bl);
	
	if(!src)
		return tick ? tick : 1; // If no source, it can't be resisted (NPC given)

/// Returns the 'bl's level, capped to 'cap'
#define SCDEF_LVL_CAP(bl, cap) ( (bl) ? (status->get_lv(bl) > (cap) ? (cap) : status->get_lv(bl)) : 0 )
/// Renewal level modifier.
/// In renewal, returns the difference between the levels of 'bl' and 'src', both capped to 'maxlv', multiplied by 'factor'
/// In pre-renewal, returns zero.
#ifdef RENEWAL
#define SCDEF_LVL_DIFF(bl, src, maxlv, factor) ( ( SCDEF_LVL_CAP((bl), (maxlv)) - SCDEF_LVL_CAP((src), (maxlv)) ) * (factor) )
#else
#define SCDEF_LVL_DIFF(bl, src, maxlv, factor) 0
#endif

	//Status that are blocked by Golden Thief Bug card or Wand of Hermod
	if (status->isimmune(bl))
		switch (type) {
		case SC_DEC_AGI:
		case SC_SILENCE:
		case SC_COMA:
		case SC_INC_AGI:
		case SC_BLESSING:
		case SC_SLOWPOISON:
		case SC_IMPOSITIO:
		case SC_LEXAETERNA:
		case SC_SUFFRAGIUM:
		case SC_BENEDICTIO:
		case SC_PROVIDENCE:
		case SC_KYRIE:
		case SC_ASSUMPTIO:
		case SC_ANGELUS:
		case SC_MAGNIFICAT:
		case SC_GLORIA:
		case SC_WINDWALK:
		case SC_MAGICROD:
		case SC_ILLUSION:
		case SC_STONE:
		case SC_QUAGMIRE:
		case SC_NJ_SUITON:
		case SC_SWING:
		case SC__ENERVATION:
		case SC__GROOMY:
		case SC__IGNORANCE:
		case SC__LAZINESS:
		case SC__UNLUCKY:
		case SC__WEAKNESS:
			return 0;
		}

	sd = BL_CAST(BL_PC,bl);
	st = status->get_status_data(bl);
	sc = status->get_sc(bl);
	if( sc && !sc->count )
		sc = NULL;

	if (sc && sc->data[SC_KINGS_GRACE]) {
		// Protects against status effects
		switch (type) {
			case SC_POISON:
			case SC_BLIND:
			case SC_FREEZE:
			case SC_STONE:
			case SC_STUN:
			case SC_SLEEP:
			case SC_BLOODING:
			case SC_CURSE:
			case SC_CONFUSION:
			case SC_ILLUSION:
			case SC_SILENCE:
			case SC_BURNING:
			case SC_COLD:
			case SC_FROSTMISTY:
			case SC_DEEP_SLEEP:
			case SC_FEAR:
			case SC_MANDRAGORA:
			case SC__CHAOS:
				return 0;
		}
	}

	switch (type) {
	case SC_STUN:
		sc_def = st->vit*100;
		sc_def2 = st->luk*10 + SCDEF_LVL_DIFF(bl, src, 99, 10);
#ifdef RENEWAL
		tick_def2 = st->luk*10;
#endif
		break;
	case SC_POISON:
	case SC_DPOISON:
		sc_def = st->vit*100;
		sc_def2 = st->luk*10 + SCDEF_LVL_DIFF(bl, src, 99, 10);
#ifdef RENEWAL
		if (sd) {
			//For players: 60000 - 450*vit - 100*luk
			tick_def = st->vit*75;
			tick_def2 = st->luk*100;
		} else {
			//For monsters: 30000 - 200*vit
			tick>>=1;
			tick_def = (st->vit*200)/3;
		}
#endif
		break;
	case SC_SILENCE:
#ifdef RENEWAL
		sc_def = st->int_*100;
		sc_def2 = (st->vit + st->luk) * 5 + SCDEF_LVL_DIFF(bl, src, 99, 10);
		tick_def2 = st->luk * 10;
#else
		sc_def = st->vit*100;
		sc_def2 = st->luk*10 + SCDEF_LVL_DIFF(bl, src, 99, 10);
#endif
		break;
	case SC_BLOODING:
#ifdef RENEWAL
		sc_def = st->agi*100;
		tick_def2 = st->luk*10;
#else
		sc_def = st->vit*100;
#endif
		sc_def2 = st->luk*10 + SCDEF_LVL_DIFF(bl, src, 99, 10);
		break;
	case SC_SLEEP:
		sc_def = st->int_*100;
		sc_def2 = st->luk*10 + SCDEF_LVL_DIFF(bl, src, 99, 10);
#ifdef RENEWAL
		tick_def2 = st->luk*10;
#endif
		break;
	case SC_DEEP_SLEEP:
		sc_def = st->int_*50;
		tick_def = 0; // Linear reduction instead
		tick_def2 = st->int_ * 50 + SCDEF_LVL_CAP(bl, 150) * 50; // kRO balance update lists this formula
		break;
	case SC_DEC_AGI:
	case SC_ADORAMUS:
		if (sd) tick >>= 1; //Half duration for players.
		sc_def = st->mdef*100;
#ifndef RENEWAL
		sc_def2 = st->luk*10;
		tick_def2 = 0; //No duration reduction
#endif
		tick_def = 0; //No duration reduction
		break;
	case SC_STONE:
		sc_def = st->mdef*100;
		sc_def2 = st->luk*10 + SCDEF_LVL_DIFF(bl, src, 99, 10);
		tick_def = 0; //No duration reduction
#ifndef RENEWAL
		tick_def2 = 0; //No duration reduction
#endif
		break;
	case SC_FREEZE:
		sc_def = st->mdef*100;
		sc_def2 = st->luk*10 + SCDEF_LVL_DIFF(bl, src, 99, 10);
		tick_def = 0; //No duration reduction
#ifdef RENEWAL
		tick_def2 = status_get_luk(src) * -10; //Caster can increase final duration with luk
#else
		tick_def2 = 0; //No duration reduction
#endif
		break;
	case SC_CURSE:
		// Special property: immunity when luk is zero
		if (st->luk == 0)
			return 0;
#ifndef RENEWAL
		// Special property: immunity when luk is greater than level
		if (st->luk > status->get_lv(bl))
			return 0;
#endif
		sc_def = st->luk*100;
		sc_def2 = st->luk*10 + SCDEF_LVL_DIFF(NULL, src, 99, 10); // Curse only has a level penalty and no resistance
		tick_def = st->vit*100;
#ifdef RENEWAL
		tick_def2 = st->luk*10;
#endif
		break;
	case SC_BLIND:
		sc_def = (st->vit + st->int_)*50;
		sc_def2 = st->luk*10 + SCDEF_LVL_DIFF(bl, src, 99, 10);
#ifdef RENEWAL
		tick_def2 = st->luk*10;
#endif
		break;
	case SC_CONFUSION:
		sc_def = (st->str + st->int_)*50;
		sc_def2 = st->luk*10 + SCDEF_LVL_DIFF(bl, src, 99, 10);
#ifdef RENEWAL
		sc_def2 = -sc_def2; // Reversed sc_def2
		tick_def2 = st->luk*10;
#endif
		break;
	case SC_ANKLESNARE:
		if(st->mode&MD_BOSS) // Lasts 5 times less on bosses
			tick /= 5;
		sc_def = st->agi*50;
		break;
	case SC_MAGICMIRROR:
	case SC_STONESKIN:
		if (sd) //Duration greatly reduced for players.
			tick /= 15;
		sc_def2 = st->vit*25 + st->agi*10 + SCDEF_LVL_CAP(bl, 99) * 20; // Linear Reduction of Rate
		tick_def2 = 0; //No duration reduction
		break;
	case SC_MARSHOFABYSS:
		//5 second (Fixed) + 25 second - {( INT + LUK ) / 20 second }
		tick_def2 = (st->int_ + st->luk)*50;
		break;
	case SC_STASIS:
		//5 second (fixed) + { Stasis Skill level * 5 - (Target's VIT + DEX) / 20 }
		tick_def2 = (st->vit + st->dex)*50;
		break;
	case SC_WHITEIMPRISON:
		if( tick == 5000 ) // 100% on caster
			break;
		if (bl->type == BL_PC)
			tick_def2 = st->vit*25 + st->agi*10 + SCDEF_LVL_CAP(bl, 150) * 20;
		else
			tick_def2 = (st->vit + st->luk)*50;
		break;
	case SC_BURNING:
		tick_def2 = 75*st->luk + 125*st->agi;
		break;
	case SC_FROSTMISTY:
		tick_def2 = (st->vit + st->dex)*50;
		break;
	case SC_OBLIVIONCURSE: // 100% - (100 - 0.8 x INT)
		sc_def = st->int_*80;
	case SC_TOXIN:
	case SC_PARALYSE:
	case SC_VENOMBLEED:
	case SC_MAGICMUSHROOM:
	case SC_DEATHHURT:
	case SC_PYREXIA:
	case SC_LEECHESEND:
		tick_def2 = (st->vit + st->luk) * 500;
		break;
	case SC_WUGBITE: // {(Base Success chance) - (Target's AGI / 4)}
		sc_def2 = st->agi*25;
		break;
	case SC_ELECTRICSHOCKER:
		tick_def2 = (st->vit + st->agi) * 70;
		break;
	case SC_COLD:
		tick_def2 = st->vit*100 + status->get_lv(bl)*20;
		break;
	case SC_VACUUM_EXTREME:
		tick_def2 = st->str*50;
		break;
	case SC_MANDRAGORA:
		sc_def = (st->vit + st->luk)*20;
		break;
	case SC_SIREN:
		tick_def2 = (status->get_lv(bl) * 100) + ((bl->type == BL_PC)?((TBL_PC*)bl)->status.job_level : 0);
		break;
	case SC_KYOUGAKU:
		tick_def2 = st->int_ * 50;
		break;
	case SC_NEEDLE_OF_PARALYZE:
		tick_def2 = (st->vit + st->luk) * 50;
		break;
	default:
		//Effect that cannot be reduced? Likely a buff.
		if (!(rnd()%10000 < rate))
			return 0;
		return tick ? tick : 1;
	}

	if (sd) {

		if (battle_config.pc_sc_def_rate != 100) {
			sc_def = sc_def*battle_config.pc_sc_def_rate/100;
			sc_def2 = sc_def2*battle_config.pc_sc_def_rate/100;
		}

		sc_def = min(sc_def, battle_config.pc_max_sc_def*100);
		sc_def2 = min(sc_def2, battle_config.pc_max_sc_def*100);

		if (tick_def > 0 && battle_config.pc_sc_def_rate != 100) {
			tick_def = tick_def*battle_config.pc_sc_def_rate/100;
			tick_def2 = tick_def2*battle_config.pc_sc_def_rate/100;
		}
	} else {

		if (battle_config.mob_sc_def_rate != 100) {
			sc_def = sc_def*battle_config.mob_sc_def_rate/100;
			sc_def2 = sc_def2*battle_config.mob_sc_def_rate/100;
		}

		sc_def = min(sc_def, battle_config.mob_max_sc_def*100);
		sc_def2 = min(sc_def2, battle_config.mob_max_sc_def*100);

		if (tick_def > 0 && battle_config.mob_sc_def_rate != 100) {
			tick_def = tick_def*battle_config.mob_sc_def_rate/100;
			tick_def2 = tick_def2*battle_config.mob_sc_def_rate/100;
		}
	}

	if (sc) {
		if (sc->data[SC_SCRESIST])
			sc_def += sc->data[SC_SCRESIST]->val1*100; //Status resist
		else if (sc->data[SC_SIEGFRIED])
			sc_def += sc->data[SC_SIEGFRIED]->val3*100; //Status resistance.
	}

	//When tick def not set, reduction is the same for both.
	if(tick_def == -1)
		tick_def = sc_def;
	if(tick_def2 == -1) {
#ifdef RENEWAL
		tick_def2 = 0;
#else
		tick_def2 = sc_def2;
#endif
	}

	//Natural resistance
	if (!(flag&8)) {
		rate -= rate*sc_def/10000;
		rate -= sc_def2;

		//Minimum chances
		switch (type) {
		case SC_OBLIVIONCURSE:
			rate = max(rate,500); //Minimum of 5%
			break;
		case SC_WUGBITE:
			rate = max(rate,5000); //Minimum of 50%
			break;
		}

		//Item resistance (only applies to rate%)
		if(sd && SC_COMMON_MIN <= type && type <= SC_COMMON_MAX)
		{
			if( sd->reseff[type-SC_COMMON_MIN] > 0 )
				rate -= rate*sd->reseff[type-SC_COMMON_MIN]/10000;
			if( sd->sc.data[SC_TARGET_BLOOD] )
				rate -= rate*sd->sc.data[SC_TARGET_BLOOD]->val1/100;
		}

		//Aegis accuracy
		if(rate > 0 && rate%10 != 0) rate += (10 - rate%10);
	}

	if (!(rnd()%10000 < rate))
		return 0;

	//Even if a status change doesn't have a duration, it should still trigger
	if (tick < 1) return 1;

	//Rate reduction
	if (flag&2)
		return tick;

	tick -= tick*tick_def/10000;
	tick -= tick_def2;

	//Minimum durations
	switch (type) {
	case SC_ANKLESNARE:
	case SC_BURNING:
	case SC_MARSHOFABYSS:
	case SC_STASIS:
	case SC_DEEP_SLEEP:
		tick = max(tick, 5000); //Minimum duration 5s
		break;
	case SC_FROSTMISTY:
		tick = max(tick, 6000);
		break;
	default:
		//Skills need to trigger even if the duration is reduced below 1ms
		tick = max(tick, 1);
		break;
	}

	return tick;
#undef SCDEF_LVL_CAP
#undef SCDEF_LVL_DIFF
}
/* [Ind/Hercules] fast-checkin sc-display array */
void status_display_add(struct map_session_data *sd, enum sc_type type, int dval1, int dval2, int dval3) {
	struct sc_display_entry *entry;
	int i;
	
	for( i = 0; i < sd->sc_display_count; i++ ) {
		if( sd->sc_display[i]->type == type )
			break;
	}
	
	if( i != sd->sc_display_count ) {
		sd->sc_display[i]->val1 = dval1;
		sd->sc_display[i]->val2 = dval2;
		sd->sc_display[i]->val3 = dval3;
		return;
	}
	
	entry = ers_alloc(pc->sc_display_ers, struct sc_display_entry);

	entry->type = type;
	entry->val1 = dval1;
	entry->val2 = dval2;
	entry->val3 = dval3;

	RECREATE(sd->sc_display, struct sc_display_entry *, ++sd->sc_display_count);
	sd->sc_display[ sd->sc_display_count - 1 ] = entry;
}
void status_display_remove(struct map_session_data *sd, enum sc_type type) {
	int i;

	for( i = 0; i < sd->sc_display_count; i++ ) {
		if( sd->sc_display[i]->type == type )
			break;
	}

	if( i != sd->sc_display_count ) {
		int cursor;

		ers_free(pc->sc_display_ers, sd->sc_display[i]);
		sd->sc_display[i] = NULL;

		/* the all-mighty compact-o-matic */
		for( i = 0, cursor = 0; i < sd->sc_display_count; i++ ) {
			if( sd->sc_display[i] == NULL )
				continue;

			if( i != cursor ) {
				sd->sc_display[cursor] = sd->sc_display[i];
			}

			cursor++;
		}

		if( !(sd->sc_display_count = cursor) ) {
			aFree(sd->sc_display);
			sd->sc_display = NULL;
		}
	}
}
/*==========================================
* Starts a status change.
* 'type' = type, 'val1~4' depend on the type.
* 'rate' = base success rate. 10000 = 100%
* 'tick' is base duration
* 'flag':
* &1: Cannot be avoided (it has to start)
* &2: Tick should not be reduced (by vit, luk, lv, etc)
* &4: sc_data loaded, no value has to be altered.
* &8: rate should not be reduced
*------------------------------------------*/
int status_change_start(struct block_list *src, struct block_list *bl, enum sc_type type, int rate, int val1, int val2, int val3, int val4, int tick, int flag) {
	struct map_session_data *sd = NULL;
	struct status_change* sc;
	struct status_change_entry* sce;
	struct status_data *st;
	struct view_data *vd;
	int opt_flag, calc_flag, undead_flag, val_flag = 0, tick_time = 0;

	nullpo_ret(bl);
	sc = status->get_sc(bl);
	st = status->get_status_data(bl);

	if( type <= SC_NONE || type >= SC_MAX ) {
		ShowError("status_change_start: invalid status change (%d)!\n", type);
		return 0;
	}

	if( !sc )
		return 0; //Unable to receive status changes

	if( status->isdead(bl) && type != SC_NOCHAT ) // SC_NOCHAT should work even on dead characters
		return 0;

	if( bl->type == BL_MOB) {
		struct mob_data *md = BL_CAST(BL_MOB,bl);
		if(md && (md->class_ == MOBID_EMPERIUM || mob_is_battleground(md)) && type != SC_SAFETYWALL && type != SC_PNEUMA)
			return 0; //Emperium/BG Monsters can't be afflicted by status changes
		//if(md && mob_is_gvg(md) && status->sc2scb_flag(type)&SCB_MAXHP)
		//	return 0; //prevent status addinh hp to gvg mob (like bloodylust=hp*3 etc...
	}

	if( sc->data[SC_REFRESH] ) {
		if( type >= SC_COMMON_MIN && type <= SC_COMMON_MAX) // Confirmed.
			return 0; // Immune to status ailements
		switch( type ) {
			case SC_DEEP_SLEEP:
			case SC__CHAOS:
			case SC_BURNING:
			case SC_STUN:
			case SC_SLEEP:
			case SC_CURSE:
			case SC_STONE:
			case SC_POISON:
			case SC_BLIND:
			case SC_SILENCE:
			case SC_BLOODING:
			case SC_FREEZE:
			case SC_FROSTMISTY:
			case SC_COLD:
			case SC_TOXIN:
			case SC_PARALYSE:
			case SC_VENOMBLEED:
			case SC_MAGICMUSHROOM:
			case SC_DEATHHURT:
			case SC_PYREXIA:
			case SC_OBLIVIONCURSE:
			case SC_MARSHOFABYSS:
			case SC_MANDRAGORA:
				return 0;
		}
	} else if( sc->data[SC_INSPIRATION] ) {
		if( type >= SC_COMMON_MIN && type <= SC_COMMON_MAX )
			return 0; // Immune to status ailements
		switch( type ) {
			case SC_POISON:
			case SC_BLIND:
			case SC_STUN:
			case SC_SILENCE:
			case SC__CHAOS:
			case SC_STONE:
			case SC_SLEEP:
			case SC_BLOODING:
			case SC_CURSE:
			case SC_BURNING:
			case SC_FROSTMISTY:
			case SC_FREEZE:
			case SC_COLD:
			case SC_FEAR:
			case SC_TOXIN:
			case SC_PARALYSE:
			case SC_VENOMBLEED:
			case SC_MAGICMUSHROOM:
			case SC_DEATHHURT:
			case SC_PYREXIA:
			case SC_OBLIVIONCURSE:
			case SC_LEECHESEND:
			case SC_DEEP_SLEEP:
			case SC_SATURDAY_NIGHT_FEVER:
			case SC__BODYPAINT:
			case SC__ENERVATION:
			case SC__GROOMY:
			case SC__IGNORANCE:
			case SC__LAZINESS:
			case SC__UNLUCKY:
			case SC__WEAKNESS:
				return 0;
		}
	}

	sd = BL_CAST(BL_PC, bl);

	//Adjust tick according to status resistances
	if( !(flag&(1|4)) ) {
		tick = status->get_sc_def(src, bl, type, rate, tick, flag);
		if( !tick ) return 0;
	}

	undead_flag = battle->check_undead(st->race,st->def_ele);
	//Check for inmunities / sc fails
	switch (type) {
		case SC_DRUMBATTLE:
		case SC_NIBELUNGEN:
		case SC_INTOABYSS:
		case SC_SIEGFRIED:
			if( sd && !sd->status.party_id )
				return 0;
			break;
		case SC_ANGRIFFS_MODUS:
		case SC_GOLDENE_FERSE:
			if ((type==SC_GOLDENE_FERSE && sc->data[SC_ANGRIFFS_MODUS])
				|| (type==SC_ANGRIFFS_MODUS && sc->data[SC_GOLDENE_FERSE])
				)
				return 0;
		case SC_VACUUM_EXTREME:
			if(sc->data[SC_HALLUCINATIONWALK] || sc->data[SC_HOVERING])
				return 0;
			break;
		case SC_STONE:
			if(sc->data[SC_POWER_OF_GAIA])
				return 0;
		case SC_FREEZE:
			//Undead are immune to Freeze/Stone
			if (undead_flag && !(flag&1))
				return 0;
		case SC_SLEEP:
		case SC_STUN:
		case SC_FROSTMISTY:
		case SC_COLD:
			if (sc->opt1)
				return 0; //Cannot override other opt1 status changes. [Skotlex]
			if((type == SC_FREEZE || type == SC_FROSTMISTY || type == SC_COLD) && sc->data[SC_WARMER])
				return 0; //Immune to Frozen and Freezing status if under Warmer status. [Jobbie]
			break;

			//There all like berserk, do not everlap each other
		case SC_BERSERK:
			if( sc->data[SC__BLOODYLUST] )
				return 0;
			break;

		case SC_BURNING:
			if(sc->opt1 || sc->data[SC_FROSTMISTY])
				return 0;
			break;

		case SC_CRUCIS:
			//Only affects demons and undead element (but not players)
			if((!undead_flag && st->race!=RC_DEMON) || bl->type == BL_PC)
				return 0;
			break;
		case SC_LEXAETERNA:
			if( (sc->data[SC_STONE] && sc->opt1 == OPT1_STONE) || sc->data[SC_FREEZE] )
				return 0;
			break;
		case SC_KYRIE:
			if (bl->type == BL_MOB)
				return 0;
			break;
		case SC_OVERTHRUST:
			if (sc->data[SC_OVERTHRUSTMAX])
				return 0; //Overthrust can't take effect if under Max Overthrust. [Skotlex]
		case SC_OVERTHRUSTMAX:
			if( sc->option&OPTION_MADOGEAR )
				return 0;//Overthrust and Overthrust Max cannot be used on Mado Gear [Ind]
			break;
		case SC_ADRENALINE:
			if(sd && !pc_check_weapontype(sd,skill->get_weapontype(BS_ADRENALINE)))
				return 0;
			if (sc->data[SC_QUAGMIRE] ||
				sc->data[SC_DEC_AGI] ||
				sc->option&OPTION_MADOGEAR //Adrenaline doesn't affect Mado Gear [Ind]
				)
				return 0;
			break;
		case SC_ADRENALINE2:
			if(sd && !pc_check_weapontype(sd,skill->get_weapontype(BS_ADRENALINE2)))
				return 0;
			if (sc->data[SC_QUAGMIRE] ||
				sc->data[SC_DEC_AGI]
			)
				return 0;
			break;
		case SC_MAGNIFICAT:
			if( sc->data[SC_OFFERTORIUM] || sc->option&OPTION_MADOGEAR ) //Mado is immune to magnificat
				return 0;
			break;
		case SC_ONEHANDQUICKEN:
		case SC_MER_QUICKEN:
		case SC_TWOHANDQUICKEN:
			if(sc->data[SC_DEC_AGI])
				return 0;

		case SC_CONCENTRATION:
		case SC_SPEARQUICKEN:
		case SC_TRUESIGHT:
		case SC_WINDWALK:
		case SC_CARTBOOST:
		case SC_ASSNCROS:
			if(sc->option&OPTION_MADOGEAR)
				return 0;//Mado is immune to wind walk, cart boost, etc (others above) [Ind]
		case SC_INC_AGI:	
			if (sc->data[SC_QUAGMIRE])
				return 0;		
			break;
		case SC_CLOAKING:
			//Avoid cloaking with no wall and low skill level. [Skotlex]
			//Due to the cloaking card, we have to check the wall versus to known
			//skill level rather than the used one. [Skotlex]
			//if (sd && val1 < 3 && skill_check_cloaking(bl,NULL))
			if( sd && pc->checkskill(sd, AS_CLOAKING) < 3 && !skill->check_cloaking(bl,NULL) )
				return 0;
			break;
		case SC_MODECHANGE:
		{
			int mode;
			struct status_data *bst = status->get_base_status(bl);
			if (!bst) return 0;
			if (sc->data[type]) {
				//Pile up with previous values.
				if(!val2) val2 = sc->data[type]->val2;
				val3 |= sc->data[type]->val3;
				val4 |= sc->data[type]->val4;
			}
			mode = val2 ? val2 : bst->mode; //Base mode
			if (val4) mode&=~val4; //Del mode
			if (val3) mode|= val3; //Add mode
			if (mode == bst->mode) { //No change.
				if (sc->data[type]) //Abort previous status
					return status_change_end(bl, type, INVALID_TIMER);
				return 0;
			}
		}
			break;
			//Strip skills, need to divest something or it fails.
		case SC_NOEQUIPWEAPON:
			if (sd && !(flag&4)) { //apply sc anyway if loading saved sc_data
				int i;
				opt_flag = 0; //Reuse to check success condition.
				if(sd->bonus.unstripable_equip&EQP_WEAPON)
					return 0;

				i = sd->equip_index[EQI_HAND_R];
				if (i>=0 && sd->inventory_data[i] && sd->inventory_data[i]->type == IT_WEAPON) {
					opt_flag|=2;
					pc->unequipitem(sd,i,3);
				}
				if (!opt_flag) return 0;
			}
			if (tick == 1) return 1; //Minimal duration: Only strip without causing the SC
			break;
		case SC_NOEQUIPSHIELD:
			if( val2 == 1 ) val2 = 0; //GX effect. Do not take shield off..
			else
				if (sd && !(flag&4)) {
					int i;
					if(sd->bonus.unstripable_equip&EQP_SHIELD)
						return 0;
					i = sd->equip_index[EQI_HAND_L];
					if ( i < 0 || !sd->inventory_data[i] || sd->inventory_data[i]->type != IT_ARMOR )
						return 0;
					pc->unequipitem(sd,i,3);
				}
				if (tick == 1) return 1; //Minimal duration: Only strip without causing the SC
				break;
		case SC_NOEQUIPARMOR:
			if (sd && !(flag&4)) {
				int i;
				if(sd->bonus.unstripable_equip&EQP_ARMOR)
					return 0;
				i = sd->equip_index[EQI_ARMOR];
				if ( i < 0 || !sd->inventory_data[i] )
					return 0;
				pc->unequipitem(sd,i,3);
			}
			if (tick == 1) return 1; //Minimal duration: Only strip without causing the SC
			break;
		case SC_NOEQUIPHELM:
			if (sd && !(flag&4)) {
				int i;
				if(sd->bonus.unstripable_equip&EQP_HELM)
					return 0;
				i = sd->equip_index[EQI_HEAD_TOP];
				if ( i < 0 || !sd->inventory_data[i] )
					return 0;
				pc->unequipitem(sd,i,3);
			}
			if (tick == 1) return 1; //Minimal duration: Only strip without causing the SC
			break;
		case SC_MER_FLEE:
		case SC_MER_ATK:
		case SC_MER_HP:
		case SC_MER_SP:
		case SC_MER_HIT:
			if( bl->type != BL_MER )
				return 0; // Stats only for Mercenaries
			break;
		case SC_FOOD_STR:
			if (sc->data[SC_FOOD_STR_CASH] && sc->data[SC_FOOD_STR_CASH]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_AGI:
			if (sc->data[SC_FOOD_AGI_CASH] && sc->data[SC_FOOD_AGI_CASH]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_VIT:
			if (sc->data[SC_FOOD_VIT_CASH] && sc->data[SC_FOOD_VIT_CASH]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_INT:
			if (sc->data[SC_FOOD_INT_CASH] && sc->data[SC_FOOD_INT_CASH]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_DEX:
			if (sc->data[SC_FOOD_DEX_CASH] && sc->data[SC_FOOD_DEX_CASH]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_LUK:
			if (sc->data[SC_FOOD_LUK_CASH] && sc->data[SC_FOOD_LUK_CASH]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_STR_CASH:
			if (sc->data[SC_FOOD_STR] && sc->data[SC_FOOD_STR]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_AGI_CASH:
			if (sc->data[SC_FOOD_AGI] && sc->data[SC_FOOD_AGI]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_VIT_CASH:
			if (sc->data[SC_FOOD_VIT] && sc->data[SC_FOOD_VIT]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_INT_CASH:
			if (sc->data[SC_FOOD_INT] && sc->data[SC_FOOD_INT]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_DEX_CASH:
			if (sc->data[SC_FOOD_DEX] && sc->data[SC_FOOD_DEX]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_LUK_CASH:
			if (sc->data[SC_FOOD_LUK] && sc->data[SC_FOOD_LUK]->val1 > val1)
				return 0;
			break;
		case SC_CAMOUFLAGE:
			if( sd && pc->checkskill(sd, RA_CAMOUFLAGE) < 3 && !skill->check_camouflage(bl,NULL) )
				return 0;
			break;
		case SC__STRIPACCESSARY:
			if( sd ) {
				int i = -1;
				if( !(sd->bonus.unstripable_equip&EQP_ACC_L) ) {
					i = sd->equip_index[EQI_ACC_L];
					if( i >= 0 && sd->inventory_data[i] && sd->inventory_data[i]->type == IT_ARMOR )
						pc->unequipitem(sd,i,3); //L-Accessory
				} if( !(sd->bonus.unstripable_equip&EQP_ACC_R) ) {
					i = sd->equip_index[EQI_ACC_R];
					if( i >= 0 && sd->inventory_data[i] && sd->inventory_data[i]->type == IT_ARMOR )
						pc->unequipitem(sd,i,3); //R-Accessory
				}
				if( i < 0 )
					return 0;
			}
			if (tick == 1) return 1; //Minimal duration: Only strip without causing the SC
			break;
		case SC_TOXIN:
		case SC_PARALYSE:
		case SC_VENOMBLEED:
		case SC_MAGICMUSHROOM:
		case SC_DEATHHURT:
		case SC_PYREXIA:
		case SC_OBLIVIONCURSE:
		case SC_LEECHESEND:
			{ // it doesn't stack or even renewed
				int i = SC_TOXIN;
				for(; i<= SC_LEECHESEND; i++)
					if(sc->data[i])	return 0;
			}
			break;
		case SC_MAGNETICFIELD:
			if(sc->data[SC_HOVERING])
				return 0;
			break;
		case SC_OFFERTORIUM:
			if (sc->data[SC_MAGNIFICAT])
				return 0;
			break;
	}

	//Check for BOSS resistances
	if(st->mode&MD_BOSS && !(flag&1)) {
		if (type>=SC_COMMON_MIN && type <= SC_COMMON_MAX)
			return 0;
		switch (type) {
			case SC_BLESSING:
			case SC_DEC_AGI:
			case SC_PROVOKE:
			case SC_COMA:
			case SC_GRAVITATION:
			case SC_NJ_SUITON:
			case SC_RICHMANKIM:
			case SC_ROKISWEIL:
			case SC_FOGWALL:
			case SC_FROSTMISTY:
			case SC_BURNING: 
			case SC_MARSHOFABYSS:
			case SC_ADORAMUS:
			case SC_NEEDLE_OF_PARALYZE:
			case SC_DEEP_SLEEP:
			case SC_COLD:

				// Exploit prevention - kRO Fix
			case SC_PYREXIA:
			case SC_DEATHHURT:
			case SC_TOXIN:
			case SC_PARALYSE:
			case SC_VENOMBLEED:
			case SC_MAGICMUSHROOM:
			case SC_OBLIVIONCURSE:
			case SC_LEECHESEND:

				// Ranger Effects
			case SC_WUGBITE:
			case SC_ELECTRICSHOCKER:
			case SC_MAGNETICFIELD:

				// Masquerades
			case SC__ENERVATION:
			case SC__GROOMY:
			case SC__LAZINESS:
			case SC__UNLUCKY:
			case SC__WEAKNESS:
			case SC__IGNORANCE:

			// Other Effects
			case SC_VACUUM_EXTREME:

				return 0;
		}
	}

	//Before overlapping fail, one must check for status cured.
	switch (type) {
		case SC_BLESSING:
			//TO-DO Blessing and Agi up should do 1 damage against players on Undead Status, even on PvM
			//but cannot be plagiarized (this requires aegis investigation on packets and official behavior) [Brainstorm]
			if ((!undead_flag && st->race!=RC_DEMON) || bl->type == BL_PC) {
				status_change_end(bl, SC_CURSE, INVALID_TIMER);
				if (sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
					status_change_end(bl, SC_STONE, INVALID_TIMER);
			}
			break;
		case SC_INC_AGI:
			status_change_end(bl, SC_DEC_AGI, INVALID_TIMER);
			break;
		case SC_QUAGMIRE:
			status_change_end(bl, SC_CONCENTRATION, INVALID_TIMER);
			status_change_end(bl, SC_TRUESIGHT, INVALID_TIMER);
			status_change_end(bl, SC_WINDWALK, INVALID_TIMER);
			//Also blocks the ones below...
		case SC_DEC_AGI:
		case SC_ADORAMUS:
			status_change_end(bl, SC_CARTBOOST, INVALID_TIMER);
			//Also blocks the ones below...
		case SC_DONTFORGETME:
			status_change_end(bl, SC_INC_AGI, INVALID_TIMER);
			status_change_end(bl, SC_ADRENALINE, INVALID_TIMER);
			status_change_end(bl, SC_ADRENALINE2, INVALID_TIMER);
			status_change_end(bl, SC_SPEARQUICKEN, INVALID_TIMER);
			status_change_end(bl, SC_TWOHANDQUICKEN, INVALID_TIMER);
			status_change_end(bl, SC_ONEHANDQUICKEN, INVALID_TIMER);
			status_change_end(bl, SC_MER_QUICKEN, INVALID_TIMER);
			status_change_end(bl, SC_ACCELERATION, INVALID_TIMER);
			break;
		case SC_ONEHANDQUICKEN:
			//Removes the Aspd potion effect, as reported by Vicious. [Skotlex]
			status_change_end(bl, SC_ATTHASTE_POTION1, INVALID_TIMER);
			status_change_end(bl, SC_ATTHASTE_POTION2, INVALID_TIMER);
			status_change_end(bl, SC_ATTHASTE_POTION3, INVALID_TIMER);
			status_change_end(bl, SC_ATTHASTE_INFINITY, INVALID_TIMER);
			break;
		case SC_OVERTHRUSTMAX:
			//Cancels Normal Overthrust. [Skotlex]
			status_change_end(bl, SC_OVERTHRUST, INVALID_TIMER);
			break;
		case SC_KYRIE:
			//Cancels Assumptio
			status_change_end(bl, SC_ASSUMPTIO, INVALID_TIMER);
			break;
		case SC_DELUGE:
			if (sc->data[SC_FOGWALL] && sc->data[SC_BLIND])
				status_change_end(bl, SC_BLIND, INVALID_TIMER);
			break;
		case SC_SILENCE:
			if (sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_SELF)
				status_change_end(bl, SC_GOSPEL, INVALID_TIMER);
			break;
		case SC_HIDING:
			status_change_end(bl, SC_RG_CCONFINE_M, INVALID_TIMER);
			status_change_end(bl, SC_RG_CCONFINE_S, INVALID_TIMER);
			break;
		case SC_BERSERK:
			if( val3 == SC__BLOODYLUST )
				break;
			if(battle_config.berserk_cancels_buffs) {
				status_change_end(bl, SC_ONEHANDQUICKEN, INVALID_TIMER);
				status_change_end(bl, SC_TWOHANDQUICKEN, INVALID_TIMER);
				status_change_end(bl, SC_LKCONCENTRATION, INVALID_TIMER);
				status_change_end(bl, SC_PARRYING, INVALID_TIMER);
				status_change_end(bl, SC_AURABLADE, INVALID_TIMER);
				status_change_end(bl, SC_MER_QUICKEN, INVALID_TIMER);
			}
	#ifdef RENEWAL
			else {
				status_change_end(bl, SC_TWOHANDQUICKEN, INVALID_TIMER);
			}
	#endif
			break;
		case SC_ASSUMPTIO:
			status_change_end(bl, SC_KYRIE, INVALID_TIMER);
			status_change_end(bl, SC_KAITE, INVALID_TIMER);
			break;
		case SC_KAITE:
			status_change_end(bl, SC_ASSUMPTIO, INVALID_TIMER);
			break;
		case SC_CARTBOOST:
			if(sc->data[SC_DEC_AGI] || sc->data[SC_ADORAMUS])
			{	//Cancel Decrease Agi, but take no further effect [Skotlex]
				status_change_end(bl, SC_DEC_AGI, INVALID_TIMER);
				status_change_end(bl, SC_ADORAMUS, INVALID_TIMER);
				return 0;
			}
			break;
		case SC_FUSION:
			status_change_end(bl, SC_SOULLINK, INVALID_TIMER);
			break;
		case SC_GS_ADJUSTMENT:
			status_change_end(bl, SC_GS_MADNESSCANCEL, INVALID_TIMER);
			break;
		case SC_GS_MADNESSCANCEL:
			status_change_end(bl, SC_GS_ADJUSTMENT, INVALID_TIMER);
			break;
			//NPC_CHANGEUNDEAD will debuff Blessing and Agi Up
		case SC_PROPERTYUNDEAD:
			status_change_end(bl, SC_BLESSING, INVALID_TIMER);
			status_change_end(bl, SC_INC_AGI, INVALID_TIMER);
			break;
		case SC_FOOD_STR:
			status_change_end(bl, SC_FOOD_STR_CASH, INVALID_TIMER);
			break;
		case SC_FOOD_AGI:
			status_change_end(bl, SC_FOOD_AGI_CASH, INVALID_TIMER);
			break;
		case SC_FOOD_VIT:
			status_change_end(bl, SC_FOOD_VIT_CASH, INVALID_TIMER);
			break;
		case SC_FOOD_INT:
			status_change_end(bl, SC_FOOD_INT_CASH, INVALID_TIMER);
			break;
		case SC_FOOD_DEX:
			status_change_end(bl, SC_FOOD_DEX_CASH, INVALID_TIMER);
			break;
		case SC_FOOD_LUK:
			status_change_end(bl, SC_FOOD_LUK_CASH, INVALID_TIMER);
			break;
		case SC_FOOD_STR_CASH:
			status_change_end(bl, SC_FOOD_STR, INVALID_TIMER);
			break;
		case SC_FOOD_AGI_CASH:
			status_change_end(bl, SC_FOOD_AGI, INVALID_TIMER);
			break;
		case SC_FOOD_VIT_CASH:
			status_change_end(bl, SC_FOOD_VIT, INVALID_TIMER);
			break;
		case SC_FOOD_INT_CASH:
			status_change_end(bl, SC_FOOD_INT, INVALID_TIMER);
			break;
		case SC_FOOD_DEX_CASH:
			status_change_end(bl, SC_FOOD_DEX, INVALID_TIMER);
			break;
		case SC_FOOD_LUK_CASH:
			status_change_end(bl, SC_FOOD_LUK, INVALID_TIMER);
			break;
		case SC_ENDURE:
			if( val4 )
				status_change_end(bl, SC_LKCONCENTRATION, INVALID_TIMER);
			break;
		case SC_FIGHTINGSPIRIT:
			status_change_end(bl, type, INVALID_TIMER); // Remove previous one.
			break;
		case SC_MARSHOFABYSS:
			status_change_end(bl, SC_INCAGI, INVALID_TIMER);
			status_change_end(bl, SC_WINDWALK, INVALID_TIMER);
			status_change_end(bl, SC_ATTHASTE_POTION1, INVALID_TIMER);
			status_change_end(bl, SC_ATTHASTE_POTION2, INVALID_TIMER);
			status_change_end(bl, SC_ATTHASTE_POTION3, INVALID_TIMER);
			status_change_end(bl, SC_ATTHASTE_INFINITY, INVALID_TIMER);
			break;
		//Group A Status (doesn't overlap)
		case SC_SWING:
		case SC_SYMPHONY_LOVE:
		case SC_MOONLIT_SERENADE:
		case SC_RUSH_WINDMILL:
		case SC_ECHOSONG:
		case SC_HARMONIZE:
		case SC_FRIGG_SONG:
			if (type != SC_SWING) status_change_end(bl, SC_SWING, INVALID_TIMER);
			if (type != SC_SYMPHONY_LOVE) status_change_end(bl, SC_SYMPHONY_LOVE, INVALID_TIMER);
			if (type != SC_MOONLIT_SERENADE) status_change_end(bl, SC_MOONLIT_SERENADE, INVALID_TIMER);
			if (type != SC_RUSH_WINDMILL) status_change_end(bl, SC_RUSH_WINDMILL, INVALID_TIMER);
			if (type != SC_ECHOSONG) status_change_end(bl, SC_ECHOSONG, INVALID_TIMER);
			if (type != SC_HARMONIZE) status_change_end(bl, SC_HARMONIZE, INVALID_TIMER);
			if (type != SC_FRIGG_SONG) status_change_end(bl, SC_FRIGG_SONG, INVALID_TIMER);
			break;
		//Group B Status
		case SC_SIREN:
		case SC_DEEP_SLEEP:
		case SC_SIRCLEOFNATURE:
		case SC_LERADS_DEW:
		case SC_MELODYOFSINK:
		case SC_BEYOND_OF_WARCRY:
		case SC_UNLIMITED_HUMMING_VOICE:
		case SC_GLOOMYDAY:
		case SC_SONG_OF_MANA:
		case SC_DANCE_WITH_WUG:
			if (type != SC_SIREN) status_change_end(bl, SC_SIREN, INVALID_TIMER);
			if (type != SC_DEEP_SLEEP) status_change_end(bl, SC_DEEP_SLEEP, INVALID_TIMER);
			if (type != SC_SIRCLEOFNATURE) status_change_end(bl, SC_SIRCLEOFNATURE, INVALID_TIMER);
			if (type != SC_LERADS_DEW) status_change_end(bl, SC_LERADS_DEW, INVALID_TIMER);
			if (type != SC_MELODYOFSINK) status_change_end(bl, SC_MELODYOFSINK, INVALID_TIMER);
			if (type != SC_BEYOND_OF_WARCRY) status_change_end(bl, SC_BEYOND_OF_WARCRY, INVALID_TIMER);
			if (type != SC_UNLIMITED_HUMMING_VOICE) status_change_end(bl, SC_UNLIMITED_HUMMING_VOICE, INVALID_TIMER);
			if (type != SC_GLOOMYDAY) status_change_end(bl, SC_GLOOMYDAY, INVALID_TIMER);
			if (type != SC_SONG_OF_MANA) status_change_end(bl, SC_SONG_OF_MANA, INVALID_TIMER);
			if (type != SC_DANCE_WITH_WUG) status_change_end(bl, SC_DANCE_WITH_WUG, INVALID_TIMER);
			break;
		case SC_REFLECTSHIELD:
			status_change_end(bl, SC_LG_REFLECTDAMAGE, INVALID_TIMER);
			break;
		case SC_LG_REFLECTDAMAGE:
			status_change_end(bl, SC_REFLECTSHIELD, INVALID_TIMER);
			break;
		case SC_SHIELDSPELL_DEF:
		case SC_SHIELDSPELL_MDEF:
		case SC_SHIELDSPELL_REF:
			status_change_end(bl, SC_MAGNIFICAT, INVALID_TIMER);
			status_change_end(bl, SC_SHIELDSPELL_DEF, INVALID_TIMER);
			status_change_end(bl, SC_SHIELDSPELL_MDEF, INVALID_TIMER);
			status_change_end(bl, SC_SHIELDSPELL_REF, INVALID_TIMER);
			break;
		case SC_GENTLETOUCH_ENERGYGAIN:
		case SC_GENTLETOUCH_CHANGE:
		case SC_GENTLETOUCH_REVITALIZE:
			if( type != SC_GENTLETOUCH_REVITALIZE )
				status_change_end(bl, SC_GENTLETOUCH_REVITALIZE, INVALID_TIMER);
			if( type != SC_GENTLETOUCH_ENERGYGAIN )
				status_change_end(bl, SC_GENTLETOUCH_ENERGYGAIN, INVALID_TIMER);
			if( type != SC_GENTLETOUCH_CHANGE )
				status_change_end(bl, SC_GENTLETOUCH_CHANGE, INVALID_TIMER);
			break;
		case SC_INVINCIBLE:
			status_change_end(bl, SC_INVINCIBLEOFF, INVALID_TIMER);
			break;
		case SC_INVINCIBLEOFF:
			status_change_end(bl, SC_INVINCIBLE, INVALID_TIMER);
			break;
		case SC_MAGICPOWER:
			status_change_end(bl, type, INVALID_TIMER);
			break;
	}

	//Check for overlapping fails
	if( (sce = sc->data[type]) ) {
		switch( type ) {
			case SC_MER_FLEE:
			case SC_MER_ATK:
			case SC_MER_HP:
			case SC_MER_SP:
			case SC_MER_HIT:
				if( sce->val1 > val1 )
					val1 = sce->val1;
				break;
			case SC_ADRENALINE:
			case SC_ADRENALINE2:
			case SC_WEAPONPERFECT:
			case SC_OVERTHRUST:
				if (sce->val2 > val2)
					return 0;
				break;
			case SC_S_LIFEPOTION:
			case SC_L_LIFEPOTION:
			case SC_CASH_BOSS_ALARM:
			case SC_STUN:
			case SC_SLEEP:
			case SC_POISON:
			case SC_CURSE:
			case SC_SILENCE:
			case SC_CONFUSION:
			case SC_BLIND:
			case SC_BLOODING:
			case SC_DPOISON:
			case SC_RG_CCONFINE_S: //Can't be re-closed in.
			case SC_MARIONETTE_MASTER:
			case SC_MARIONETTE:
			case SC_NOCHAT:
			case SC_HLIF_CHANGE: //Otherwise your Hp/Sp would get refilled while still within effect of the last invocation.
			case SC_ABUNDANCE:
			case SC_TOXIN:
			case SC_PARALYSE:
			case SC_VENOMBLEED:
			case SC_MAGICMUSHROOM:
			case SC_DEATHHURT:
			case SC_PYREXIA:
			case SC_OBLIVIONCURSE:
			case SC_LEECHESEND:
			case SC__INVISIBILITY:
			case SC__ENERVATION:
			case SC__GROOMY:
			case SC__IGNORANCE:
			case SC__LAZINESS:
			case SC__WEAKNESS:
			case SC__UNLUCKY:
			case SC__CHAOS:
				return 0;
			case SC_COMBOATTACK:
			case SC_DANCING:
			case SC_DEVOTION:
			case SC_ATTHASTE_POTION1:
			case SC_ATTHASTE_POTION2:
			case SC_ATTHASTE_POTION3:
			case SC_ATTHASTE_INFINITY:
			case SC_PLUSATTACKPOWER:
			case SC_PLUSMAGICPOWER:
			case SC_ENCHANTARMS:
			case SC_ARMORPROPERTY:
			case SC_ARMOR_RESIST:
				break;
			case SC_GOSPEL:
				//Must not override a casting gospel char.
				if(sce->val4 == BCT_SELF)
					return 0;
				if(sce->val1 > val1)
					return 1;
				break;
			case SC_ENDURE:
				if(sce->val4 && !val4)
					return 1; //Don't let you override infinite endure.
				if(sce->val1 > val1)
					return 1;
				break;
			case SC_KAAHI:
				//Kaahi overwrites previous level regardless of existing level.
				//Delete timer if it exists.
				if (sce->val4 != INVALID_TIMER) {
					timer->delete(sce->val4,status->kaahi_heal_timer);
					sce->val4 = INVALID_TIMER;
				}
				break;
			case SC_JAILED:
				//When a player is already jailed, do not edit the jail data.
				val2 = sce->val2;
				val3 = sce->val3;
				val4 = sce->val4;
				break;
			case SC_LERADS_DEW:
				if (sc && sc->data[SC_BERSERK])
					return 0;
			case SC_SHAPESHIFT:
			case SC_PROPERTYWALK:
				break;
			case SC_LEADERSHIP:
			case SC_GLORYWOUNDS:
			case SC_SOULCOLD:
			case SC_HAWKEYES:
				if( sce->val4 && !val4 )//you cannot override master guild aura
					return 0;
				break;
			case SC_JOINTBEAT:
				val2 |= sce->val2; // stackable ailments
			default:
				if(sce->val1 > val1)
					return 1; //Return true to not mess up skill animations. [Skotlex]
		}
	}

	vd = status->get_viewdata(bl);
	calc_flag = status->ChangeFlagTable[type];
	if(!(flag&4)) { //&4 - Do not parse val settings when loading SCs
		switch(type) {
			case SC_ADORAMUS:
				sc_start(src,bl,SC_BLIND,100,val1,skill->get_time(status->sc2skill(type),val1));
				// Fall through to SC_INC_AGI
			case SC_DEC_AGI:
			case SC_INC_AGI:
				val2 = 2 + val1; //Agi change
				break;
			case SC_ENDURE:
				val2 = 7; // Hit-count [Celest]
				if( !(flag&1) && (bl->type&(BL_PC|BL_MER)) && !map_flag_gvg(bl->m) && !map->list[bl->m].flag.battleground && !val4 ) {
					struct map_session_data *tsd;
					if( sd ) {
						int i;
						for( i = 0; i < 5; i++ ) {
							if( sd->devotion[i] && (tsd = map->id2sd(sd->devotion[i])) )
								status->change_start(bl, &tsd->bl, type, 10000, val1, val2, val3, val4, tick, 1);
						}
					} else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
						status->change_start(bl, &tsd->bl, type, 10000, val1, val2, val3, val4, tick, 1);
				}
				//val4 signals infinite endure (if val4 == 2 it is infinite endure from Berserk)
				if( val4 )
					tick = -1;
				break;
			case SC_AUTOBERSERK:
				if (st->hp < st->max_hp>>2 &&
					(!sc->data[SC_PROVOKE] || sc->data[SC_PROVOKE]->val2==0))
					sc_start4(src,bl,SC_PROVOKE,100,10,1,0,0,60000);
				tick = -1;
				break;
			case SC_CRUCIS:
				val2 = 10 + 4*val1; //Def reduction
				tick = -1;
				clif->emotion(bl,E_SWT);
				break;
			case SC_MAXIMIZEPOWER:
				tick_time = val2 = tick>0?tick:60000;
				tick = -1; // duration sent to the client should be infinite
				break;
			case SC_EDP:	// [Celest]
				val2 = val1 + 2; //Chance to Poison enemies.
				val3 = 50*(val1+1); //Damage increase (+50 +50*lv%)
	#ifdef RENEWAL_EDP
				val4 = 100 * ((val1 + 1)/2 + 2);
	#endif
				if( sd )//[Ind] - iROwiki says each level increases its duration by 3 seconds
					tick += pc->checkskill(sd,GC_RESEARCHNEWPOISON)*3000;
				break;
			case SC_POISONREACT:
				val2=(val1+1)/2 + val1/10; // Number of counters [Skotlex]
				val3=50; // + 5*val1; //Chance to counter. [Skotlex]
				break;
			case SC_MAGICROD:
				val2 = val1*20; //SP gained
				break;
			case SC_KYRIE:
				val2 = APPLY_RATE(st->max_hp, (val1 * 2 + 10)); //%Max HP to absorb
				// val4 holds current about of party memebers when casting AB_PRAEFATIO,
				// as Praefatio's barrier has more health and blocks more hits than Kyrie Elesion.
				if( val4 < 1 ) //== PR_KYRIE
					val3 = (val1 / 2 + 5); // Hits
				else { //== AB_PRAEFATIO
					val2 += val4 * 2; //Increase barrier strength per party member.
					val3 = 6 + val1;
				}
				if( sd )
					val1 = min(val1,pc->checkskill(sd,PR_KYRIE)); // use skill level to determine barrier health.
				break;
			case SC_MAGICPOWER:
				//val1: Skill lv
				val2 = 1; //Lasts 1 invocation
				val3 = 5*val1; //Matk% increase
				val4 = 0; // 0 = ready to be used, 1 = activated and running
				break;
			case SC_SACRIFICE:
				val2 = 5; //Lasts 5 hits
				tick = -1;
				break;
			case SC_ENCHANTPOISON:
				val2= 250+50*val1;	//Poisoning Chance (2.5+0.5%) in 1/10000 rate
			case SC_ASPERSIO:
			case SC_PROPERTYFIRE:
			case SC_PROPERTYWATER:
			case SC_PROPERTYWIND:
			case SC_PROPERTYGROUND:
			case SC_PROPERTYDARK:
			case SC_PROPERTYTELEKINESIS:
				skill->enchant_elemental_end(bl,type);
				break;
			case SC_ARMOR_PROPERTY:
				// val1 : Element Lvl (if called by skill lvl 1, takes random value between 1 and 4)
				// val2 : Element (When no element, random one is picked)
				// val3 : 0 = called by skill 1 = called by script (fixed level)
				if( !val2 ) val2 = rnd()%ELE_MAX;

				if( val1 == 1 && val3 == 0 )
					val1 = 1 + rnd()%4;
				else if( val1 > 4 )
					val1 = 4; // Max Level
				val3 = 0; // Not need to keep this info.
				break;
			case SC_PROVIDENCE:
				val2=val1*5; //Race/Ele resist
				break;
			case SC_REFLECTSHIELD:
				val2=10+val1*3; // %Dmg reflected
				if( !(flag&1) && (bl->type&(BL_PC|BL_MER)) ) {
					struct map_session_data *tsd;
					if( sd ) {
						int i;
						for( i = 0; i < 5; i++ ) {
							if( sd->devotion[i] && (tsd = map->id2sd(sd->devotion[i])) )
								status->change_start(bl, &tsd->bl, type, 10000, val1, val2, 0, 0, tick, 1);
						}
					} else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
						status->change_start(bl, &tsd->bl, type, 10000, val1, val2, 0, 0, tick, 1);
				}
				break;
			case SC_NOEQUIPWEAPON:
				if (!sd) //Watk reduction
					val2 = 25;
				break;
			case SC_NOEQUIPSHIELD:
				if (!sd) //Def reduction
					val2 = 15;
				break;
			case SC_NOEQUIPARMOR:
				if (!sd) //Vit reduction
					val2 = 40;
				break;
			case SC_NOEQUIPHELM:
				if (!sd) //Int reduction
					val2 = 40;
				break;
			case SC_AUTOSPELL:
				//Val1 Skill LV of Autospell
				//Val2 Skill ID to cast
				//Val3 Max Lv to cast
				val4 = 5 + val1*2; //Chance of casting
				break;
			case SC_VOLCANO:
				val2 = val1*10; //Watk increase
	#ifndef RENEWAL
				if (st->def_ele != ELE_FIRE)
					val2 = 0;
	#endif
				break;
			case SC_VIOLENTGALE:
				val2 = val1*3; //Flee increase
	#ifndef RENEWAL
				if (st->def_ele != ELE_WIND)
					val2 = 0;
	#endif
				break;
			case SC_DELUGE:
				val2 = skill->deluge_eff[val1-1]; //HP increase
	#ifndef RENEWAL
				if(st->def_ele != ELE_WATER)
					val2 = 0;
	#endif
				break;
			case SC_NJ_SUITON:
				if (!val2 || (sd && (sd->class_&MAPID_BASEMASK) == MAPID_NINJA)) {
					//No penalties.
					val2 = 0; //Agi penalty
					val3 = 0; //Walk speed penalty
					break;
				}
				val3 = 50;
				val2 = 3*((val1+1)/3);
				if (val1 > 4) val2--;
				break;
			case SC_ONEHANDQUICKEN:
			case SC_TWOHANDQUICKEN:
				val2 = 300;
				if (val1 > 10) //For boss casted skills [Skotlex]
					val2 += 20*(val1-10);
				break;
			case SC_MER_QUICKEN:
				val2 = 300;
				break;
	#ifndef RENEWAL_ASPD
			case SC_SPEARQUICKEN:
				val2 = 200+10*val1;
				break;
	#endif
			case SC_DANCING:
				//val1 : Skill ID + LV
				//val2 : Skill Group of the Dance.
				//val3 : Brings the skill_lv (merged into val1 here)
				//val4 : Partner
				if (val1 == CG_MOONLIT)
					clif->status_change(bl,SI_MOON,1,tick,0, 0, 0);
				val1|= (val3<<16);
				val3 = tick/1000; //Tick duration
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_LONGING:
	#ifdef RENEWAL
				val2 = 50 + 10 * val1;
	#else
				val2 = 500-100*val1; //Aspd penalty.
	#endif
				break;
			case SC_EXPLOSIONSPIRITS:
				val2 = 75 + 25*val1; //Cri bonus
				break;

			case SC_ATTHASTE_POTION1:
			case SC_ATTHASTE_POTION2:
			case SC_ATTHASTE_POTION3:
			case SC_ATTHASTE_INFINITY:
				val2 = 50*(2+type-SC_ATTHASTE_POTION1);
				break;

			case SC_WEDDING:
			case SC_XMAS:
			case SC_SUMMER:
			case SC_HANBOK:
			case SC_OKTOBERFEST:
				if (!vd) return 0;
				//Store previous values as they could be removed.
				unit->stop_attack(bl);
				break;
			case SC_NOCHAT:
				// [GodLesZ] FIXME: is this correct? a hardcoded interval of 60sec? what about configuration ?_?
				tick = 60000;
				val1 = battle_config.manner_system; //Mute filters.
				if (sd)
				{
					clif->changestatus(sd,SP_MANNER,sd->status.manner);
					clif->updatestatus(sd,SP_MANNER);
				}
				break;

			case SC_STONE:
				val3 = tick/1000; //Petrified HP-damage iterations.
				if(val3 < 1) val3 = 1;
				tick = val4; //Petrifying time.
				if(val4 > 500) // not with WL_SIENNAEXECRATE
					tick = max(tick, 1000); //Min time
				calc_flag = 0; //Actual status changes take effect on petrified state.
				break;

			case SC_DPOISON:
				//Lose 10/15% of your life as long as it doesn't brings life below 25%
				if (st->hp > st->max_hp>>2) {
					int diff = st->max_hp*(bl->type==BL_PC?10:15)/100;
					if (st->hp - diff < st->max_hp>>2)
						diff = st->hp - (st->max_hp>>2);
					if( val2 && bl->type == BL_MOB ) {
						struct block_list* src2 = map->id2bl(val2);
						if( src2 )
							mob->log_damage((TBL_MOB*)bl,src2,diff);
					}
					status_zap(bl, diff, 0);
				}
				// fall through
			case SC_POISON:
				val3 = tick/1000; //Damage iterations
				if(val3 < 1) val3 = 1;
				tick_time = 1000; // [GodLesZ] tick time
				//val4: HP damage
				if (bl->type == BL_PC)
					val4 = (type == SC_DPOISON) ? 3 + st->max_hp/50 : 3 + st->max_hp*3/200;
				else
					val4 = (type == SC_DPOISON) ? 3 + st->max_hp/100 : 3 + st->max_hp/200;

				break;
			case SC_CONFUSION:
				clif->emotion(bl,E_WHAT);
				break;
			case SC_BLOODING:
				val4 = tick/10000;
				if (!val4) val4 = 1;
				tick_time = 10000; // [GodLesZ] tick time
				break;
			case SC_S_LIFEPOTION:
			case SC_L_LIFEPOTION:
				if( val1 == 0 ) return 0;
				// val1 = heal percent/amout
				// val2 = seconds between heals
				// val4 = total of heals
				if( val2 < 1 ) val2 = 1;
				if( (val4 = tick/(val2 * 1000)) < 1 )
					val4 = 1;
				tick_time = val2 * 1000; // [GodLesZ] tick time
				break;
			case SC_CASH_BOSS_ALARM:
				if( sd != NULL ) {
					struct mob_data *boss_md = map->getmob_boss(bl->m); // Search for Boss on this Map
					if( boss_md == NULL || boss_md->bl.prev == NULL ) {
						// No MVP on this map - MVP is dead
						clif->bossmapinfo(sd->fd, boss_md, 1);
						return 0; // No need to start SC
					}
					val1 = boss_md->bl.id;
					if( (val4 = tick/1000) < 1 )
						val4 = 1;
					tick_time = 1000; // [GodLesZ] tick time
				}
				break;
			case SC_HIDING:
				val2 = tick/1000;
				tick_time = 1000; // [GodLesZ] tick time
				val3 = 0; // unused, previously speed adjustment
				val4 = val1+3; //Seconds before SP substraction happen.
				break;
			case SC_CHASEWALK:
				val2 = tick>0?tick:10000; //Interval at which SP is drained.
				val3 = 35 - 5 * val1; //Speed adjustment.
				if (sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_ROGUE)
					val3 -= 40;
				val4 = 10+val1*2; //SP cost.
				if (map_flag_gvg(bl->m) || map->list[bl->m].flag.battleground) val4 *= 5;
				break;
			case SC_CLOAKING:
				if (!sd) //Monsters should be able to walk with no penalties. [Skotlex]
					val1 = 10;
				tick_time = val2 = tick>0?tick:60000; //SP consumption rate.
				tick = -1; // duration sent to the client should be infinite
				val3 = 0; // unused, previously walk speed adjustment
				//val4&1 signals the presence of a wall.
				//val4&2 makes cloak not end on normal attacks [Skotlex]
				//val4&4 makes cloak not end on using skills
				if (bl->type == BL_PC || (bl->type == BL_MOB && ((TBL_MOB*)bl)->special_state.clone) )	//Standard cloaking.
					val4 |= battle_config.pc_cloak_check_type&7;
				else
					val4 |= battle_config.monster_cloak_check_type&7;
				break;
			case SC_SIGHT:			/* splash status */
			case SC_RUWACH:
			case SC_WZ_SIGHTBLASTER:
				val3 = skill->get_splash(val2, val1); //Val2 should bring the skill-id.
				val2 = tick/250;
				tick_time = 10; // [GodLesZ] tick time
				break;

				//Permanent effects.
			case SC_LEXAETERNA:
			case SC_MODECHANGE:
			case SC_WEIGHTOVER50:
			case SC_WEIGHTOVER90:
			case SC_BROKENWEAPON:
			case SC_BROKENARMOR:
			case SC_STORMKICK_READY:
			case SC_DOWNKICK_READY:
			case SC_COUNTERKICK_READY:
			case SC_TURNKICK_READY:
			case SC_DODGE_READY:
			case SC_PUSH_CART:
			case SC_ALL_RIDING:
				tick = -1;
				break;

			case SC_AUTOGUARD:
				if( !(flag&1) ) {
					struct map_session_data *tsd;
					int i,t;
					for( i = val2 = 0; i < val1; i++) {
						t = 5-(i>>1);
						val2 += (t < 0)? 1:t;
					}

					if( bl->type&(BL_PC|BL_MER) ) {
						if( sd ) {
							for( i = 0; i < 5; i++ ) {
								if( sd->devotion[i] && (tsd = map->id2sd(sd->devotion[i])) )
									status->change_start(bl, &tsd->bl, type, 10000, val1, val2, 0, 0, tick, 1);
							}
						}
						else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
							status->change_start(bl, &tsd->bl, type, 10000, val1, val2, 0, 0, tick, 1);
					}
				}
				break;

			case SC_DEFENDER:
				if (!(flag&1)) {
					val2 = 5 + 15*val1; //Damage reduction
					val3 = 0; // unused, previously speed adjustment
					val4 = 250 - 50*val1; //Aspd adjustment

					if (sd) {
						struct map_session_data *tsd;
						int i;
						for (i = 0; i < 5; i++) {
							//See if there are devoted characters, and pass the status to them. [Skotlex]
							if (sd->devotion[i] && (tsd = map->id2sd(sd->devotion[i])))
								status->change_start(bl, &tsd->bl,type,10000,val1,5+val1*5,val3,val4,tick,1);
						}
					}
				}
				break;

			case SC_TENSIONRELAX:
				if (sd) {
					pc_setsit(sd);
					clif->sitting(&sd->bl);
				}
				val2 = 12; //SP cost
				val4 = 10000; //Decrease at 10secs intervals.
				val3 = tick/val4;
				tick = -1; // duration sent to the client should be infinite
				tick_time = val4; // [GodLesZ] tick time
				break;
			case SC_PARRYING:
				val2 = 20 + val1*3; //Block Chance
				break;

			case SC_WINDWALK:
				val2 = (val1+1)/2; // Flee bonus is 1/1/2/2/3/3/4/4/5/5
				break;

			case SC_JOINTBEAT:
				if( val2&BREAK_NECK )
					sc_start2(src,bl,SC_BLOODING,100,val1,val3,skill->get_time2(status->sc2skill(type),val1));
				break;

			case SC_BERSERK:
				if( val3 == SC__BLOODYLUST )
					sc_start(src,bl,(sc_type)val3,100,val1,tick);
				if( !val3 && !(!sc->data[SC_ENDURE] || !sc->data[SC_ENDURE]->val4) )
					sc_start4(src, bl, SC_ENDURE, 100,10,0,0,2, tick);
				//HP healing is performing after the calc_status call.
				//Val2 holds HP penalty
				if (!val4) val4 = skill->get_time2(status->sc2skill(type),val1);
				if (!val4) val4 = 10000; //Val4 holds damage interval
				val3 = tick/val4; //val3 holds skill duration
				tick_time = val4; // [GodLesZ] tick time
				break;

			case SC_GOSPEL:
				if(val4 == BCT_SELF) {
					// self effect
					val2 = tick/10000;
					tick_time = 10000; // [GodLesZ] tick time
					status->change_clear_buffs(bl,3); //Remove buffs/debuffs
				}
				break;

			case SC_MARIONETTE_MASTER:
			{
				int stat;

				val3 = 0;
				val4 = 0;
				stat = ( sd ? sd->status.str : status->get_base_status(bl)->str ) / 2; val3 |= cap_value(stat,0,0xFF)<<16;
				stat = ( sd ? sd->status.agi : status->get_base_status(bl)->agi ) / 2; val3 |= cap_value(stat,0,0xFF)<<8;
				stat = ( sd ? sd->status.vit : status->get_base_status(bl)->vit ) / 2; val3 |= cap_value(stat,0,0xFF);
				stat = ( sd ? sd->status.int_: status->get_base_status(bl)->int_) / 2; val4 |= cap_value(stat,0,0xFF)<<16;
				stat = ( sd ? sd->status.dex : status->get_base_status(bl)->dex ) / 2; val4 |= cap_value(stat,0,0xFF)<<8;
				stat = ( sd ? sd->status.luk : status->get_base_status(bl)->luk ) / 2; val4 |= cap_value(stat,0,0xFF);
			}
				break;
			case SC_MARIONETTE:
			{
				int stat,max_stat;
				// fetch caster information
				struct block_list *pbl = map->id2bl(val1);
				struct status_change *psc = pbl ? status->get_sc(pbl) : NULL;
				struct status_change_entry *psce = psc ? psc->data[SC_MARIONETTE_MASTER] : NULL;
				// fetch target's stats
				struct status_data* tst = status->get_status_data(bl); // battle status

				if (!psce)
					return 0;

				val3 = 0;
				val4 = 0;
				max_stat = battle_config.max_parameter; //Cap to 99 (default)
				stat = (psce->val3 >>16)&0xFF; stat = min(stat, max_stat - tst->str ); val3 |= cap_value(stat,0,0xFF)<<16;
				stat = (psce->val3 >> 8)&0xFF; stat = min(stat, max_stat - tst->agi ); val3 |= cap_value(stat,0,0xFF)<<8;
				stat = (psce->val3 >> 0)&0xFF; stat = min(stat, max_stat - tst->vit ); val3 |= cap_value(stat,0,0xFF);
				stat = (psce->val4 >>16)&0xFF; stat = min(stat, max_stat - tst->int_); val4 |= cap_value(stat,0,0xFF)<<16;
				stat = (psce->val4 >> 8)&0xFF; stat = min(stat, max_stat - tst->dex ); val4 |= cap_value(stat,0,0xFF)<<8;
				stat = (psce->val4 >> 0)&0xFF; stat = min(stat, max_stat - tst->luk ); val4 |= cap_value(stat,0,0xFF);
			}
				break;
			case SC_SWORDREJECT:
				val2 = 15*val1; //Reflect chance
				val3 = 3; //Reflections
				tick = -1;
				break;

			case SC_MEMORIZE:
				val2 = 5; //Memorized casts.
				tick = -1;
				break;

			case SC_GRAVITATION:
				val2 = 50*val1; //aspd reduction
				break;

			case SC_GDSKILL_REGENERATION:
				if (val1 == 1)
					val2 = 2;
				else
					val2 = val1; //HP Regerenation rate: 200% 200% 300%
				val3 = val1; //SP Regeneration Rate: 100% 200% 300%
				//if val4 comes set, this blocks regen rather than increase it.
				break;

			case SC_DEVOTION:
			{
				struct block_list *d_bl;
				struct status_change *d_sc;

				if( (d_bl = map->id2bl(val1)) && (d_sc = status->get_sc(d_bl)) && d_sc->count ) {
					// Inherits Status From Source
					const enum sc_type types[] = { SC_AUTOGUARD, SC_DEFENDER, SC_REFLECTSHIELD, SC_ENDURE };
					enum sc_type type2;
					int i = (map_flag_gvg(bl->m) || map->list[bl->m].flag.battleground)?2:3;
					while( i >= 0 ) {
						type2 = types[i];
						if( d_sc->data[type2] )
							sc_start(bl, bl, type2, 100, d_sc->data[type2]->val1, skill->get_time(status->sc2skill(type2),d_sc->data[type2]->val1));
						i--;
					}
				}
				break;
			}

			case SC_COMA: //Coma. Sends a char to 1HP. If val2, do not zap sp
				if( val3 && bl->type == BL_MOB ) {
					struct block_list* src2 = map->id2bl(val3);
					if( src2 )
						mob->log_damage((TBL_MOB*)bl,src2,st->hp - 1);
				}
				status_zap(bl, st->hp-1, val2 ? 0 : st->sp);
				return 1;
				break;
			case SC_RG_CCONFINE_S:
			{
				struct block_list *src2 = val2 ? map->id2bl(val2) : NULL;
				struct status_change *sc2 = src ? status->get_sc(src2) : NULL;
				struct status_change_entry *sce2 = sc2 ? sc2->data[SC_RG_CCONFINE_M] : NULL;
				if (src2 && sc2) {
					if (!sce2) //Start lock on caster.
						sc_start4(src,src2,SC_RG_CCONFINE_M,100,val1,1,0,0,tick+1000);
					else { //Increase count of locked enemies and refresh time.
						(sce2->val2)++;
						timer->delete(sce2->timer, status->change_timer);
						sce2->timer = timer->add(timer->gettick()+tick+1000, status->change_timer, src2->id, SC_RG_CCONFINE_M);
					}
				} else //Status failed.
					return 0;
			}
				break;
			case SC_KAITE:
				val2 = 1+val1/5; //Number of bounces: 1 + skill_lv/5
				break;
			case SC_KAUPE:
				switch (val1) {
				case 3: //33*3 + 1 -> 100%
					val2++;
				case 1:
				case 2: //33, 66%
					val2 += 33*val1;
					val3 = 1; //Dodge 1 attack total.
					break;
				default: //Custom. For high level mob usage, higher level means more blocks. [Skotlex]
					val2 = 100;
					val3 = val1-2;
					break;
				}
				break;

			case SC_COMBOATTACK: {
				//val1: Skill ID
				//val2: When given, target (for autotargetting skills)
				//val3: When set, this combo time should NOT delay attack/movement
				//val3: TK: Last used kick
				//val4: TK: Combo time
				struct unit_data *ud = unit->bl2ud(bl);
				if (ud && !val3) {
					tick += 300 * battle_config.combo_delay_rate/100;
					ud->attackabletime = timer->gettick()+tick;
					unit->set_walkdelay(bl, timer->gettick(), tick, 1);
				}
				val3 = 0;
				val4 = tick;
								 }
								 break;
			case SC_EARTHSCROLL:
				val2 = 11-val1; //Chance to consume: 11-skill_lv%
				break;
			case SC_RUN:
			{
				//Store time at which you started running.
				int64 currenttick = timer->gettick();
				// Note: this int64 value is stored in two separate int32 variables (FIXME)
				val3 = (int)(currenttick&0x00000000ffffffffLL);
				val4 = (int)((currenttick&0xffffffff00000000LL)>>32);
			}
				tick = -1;
				break;
			case SC_KAAHI:
				val2 = 200*val1; //HP heal
				val3 = 5*val1; //SP cost
				val4 = INVALID_TIMER;	//Kaahi Timer.
				break;
			case SC_BLESSING:
				if ((!undead_flag && st->race!=RC_DEMON) || bl->type == BL_PC)
					val2 = val1;
				else
					val2 = 0; //0 -> Half stat.
				break;
			case SC_TRICKDEAD:
				if (vd) vd->dead_sit = 1;
				tick = -1;
				break;
			case SC_CONCENTRATION:
				val2 = 2 + val1;
				if (sd) { //Store the card-bonus data that should not count in the %
					val3 = sd->param_bonus[1]; //Agi
					val4 = sd->param_bonus[4]; //Dex
				} else {
					val3 = val4 = 0;
				}
				break;
			case SC_OVERTHRUSTMAX:
				val2 = 20*val1; //Power increase
				break;
			case SC_OVERTHRUST:
				//val2 holds if it was casted on self, or is bonus received from others
				val3 = 5*val1; //Power increase
				if(sd && pc->checkskill(sd,BS_HILTBINDING)>0)
					tick += tick / 10;
				break;
			case SC_ADRENALINE2:
			case SC_ADRENALINE:
				val3 = (val2) ? 300 : 200; // aspd increase
			case SC_WEAPONPERFECT:
				if(sd && pc->checkskill(sd,BS_HILTBINDING)>0)
					tick += tick / 10;
				break;
			case SC_LKCONCENTRATION:
				val2 = 5*val1; //Batk/Watk Increase
				val3 = 10*val1; //Hit Increase
				val4 = 5*val1; //Def reduction
				sc_start(src, bl, SC_ENDURE, 100, 1, tick); //Endure effect
				break;
			case SC_ANGELUS:
				val2 = 5*val1; //def increase
				break;
			case SC_IMPOSITIO:
				val2 = 5*val1; //watk increase
				break;
			case SC_MELTDOWN:
				val2 = 100*val1; //Chance to break weapon
				val3 = 70*val1; //Change to break armor
				break;
			case SC_TRUESIGHT:
				val2 = 10*val1; //Critical increase
				val3 = 3*val1; //Hit increase
				break;
			case SC_SUN_COMFORT:
				val2 = (status->get_lv(bl) + st->dex + st->luk)/2; //def increase
				break;
			case SC_MOON_COMFORT:
				val2 = (status->get_lv(bl) + st->dex + st->luk)/10; //flee increase
				break;
			case SC_STAR_COMFORT:
				val2 = (status->get_lv(bl) + st->dex + st->luk); //Aspd increase
				break;
			case SC_QUAGMIRE:
				val2 = (sd?5:10)*val1; //Agi/Dex decrease.
				break;

				// gs_something1 [Vicious]
			case SC_GS_GATLINGFEVER:
				val2 = 20*val1; //Aspd increase
				val4 = 5*val1; //Flee decrease
	#ifndef RENEWAL
				val3 = 20+10*val1; //Batk increase
	#endif
				break;

			case SC_FLING:
				if (bl->type == BL_PC)
					val2 = 0; //No armor reduction to players.
				else
					val2 = 5*val1; //Def reduction
				val3 = 5*val1; //Def2 reduction
				break;
			case SC_PROVOKE:
				//val2 signals autoprovoke.
				val3 = 2+3*val1; //Atk increase
				val4 = 5+5*val1; //Def reduction.
				break;
			case SC_HLIF_AVOID:
				//val2 = 10*val1; //Speed change rate.
				break;
			case SC_HAMI_DEFENCE:
				val2 = 2*val1; //Def bonus
				break;
			case SC_HAMI_BLOODLUST:
				val2 = 20+10*val1; //Atk rate change.
				val3 = 3*val1; //Leech chance
				val4 = 20; //Leech percent
				break;
			case SC_HLIF_FLEET:
				val2 = 30*val1; //Aspd change
				val3 = 5+5*val1; //bAtk/wAtk rate change
				break;
			case SC_MINDBREAKER:
				val2 = 20*val1; //matk increase.
				val3 = 12*val1; //mdef2 reduction.
				break;
			case SC_SKA:
				val2 = tick/1000;
				val3 = rnd()%100; //Def changes randomly every second...
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_JAILED:
				//Val1 is duration in minutes. Use INT_MAX to specify 'unlimited' time.
				tick = val1>0?1000:250;
				if (sd)
				{
					if (sd->mapindex != val2)
					{
						int pos = (bl->x&0xFFFF)|(bl->y<<16); /// Current Coordinates
						int map_index = sd->mapindex; /// Current Map
						//1. Place in Jail (val2 -> Jail Map, val3 -> x, val4 -> y
						pc->setpos(sd,(unsigned short)val2,val3,val4, CLR_TELEPORT);
						//2. Set restore point (val3 -> return map, val4 return coords
						val3 = map_index;
						val4 = pos;
					} else if (!val3 || val3 == sd->mapindex) { //Use save point.
						val3 = sd->status.save_point.map;
						val4 = (sd->status.save_point.x&0xFFFF)
							|(sd->status.save_point.y<<16);
					}
				}
				break;
			case SC_NJ_UTSUSEMI:
				val2=(val1+1)/2; // number of hits blocked
				val3=skill->get_blewcount(NJ_UTSUSEMI, val1); //knockback value.
				break;
			case SC_NJ_BUNSINJYUTSU:
				val2=(val1+1)/2; // number of hits blocked
				break;
			case SC_HLIF_CHANGE:
				val2= 30*val1; //Vit increase
				val3= 20*val1; //Int increase
				break;
			case SC_SWOO:
				if(st->mode&MD_BOSS)
					tick /= 5; //TODO: Reduce skill's duration. But for how long?
				break;
			case SC_SPIDERWEB:
				if( bl->type == BL_PC )
					tick /= 2;
				break;
			case SC_ARMOR:
				//NPC_DEFENDER:
				val2 = 80; //Damage reduction
				//Attack requirements to be blocked:
				val3 = BF_LONG; //Range
				val4 = BF_WEAPON|BF_MISC; //Type
				break;
			case SC_ENCHANTARMS:
				//end previous enchants
				skill->enchant_elemental_end(bl,type);
				//Make sure the received element is valid.
				if (val2 >= ELE_MAX)
					val2 = val2%ELE_MAX;
				else if (val2 < 0)
					val2 = rnd()%ELE_MAX;
				break;
			case SC_CRITICALWOUND:
				val2 = 20*val1; //Heal effectiveness decrease
				break;
			case SC_MAGICMIRROR:
			case SC_SLOWCAST:
				val2 = 20*val1; //Magic reflection/cast rate
				break;

			case SC_STONESKIN:
				if (val2 == NPC_ANTIMAGIC)
				{	//Boost mdef
					val2 =-20;
					val3 = 20;
				} else { //Boost def
					val2 = 20;
					val3 =-20;
				}
				val2*=val1; //20% per level
				val3*=val1;
				break;
			case SC_CASH_PLUSEXP:
			case SC_CASH_PLUSONLYJOBEXP:
				if (val1 < 0)
					val1 = 0;
				break;
			case SC_PLUSAVOIDVALUE:
			case SC_CRITICALPERCENT:
				val2 = val1*10; //Actual boost (since 100% = 1000)
				break;
			case SC_SUFFRAGIUM:
				val2 = 15 * val1; //Speed cast decrease
				break;
			case SC_HEALPLUS:
				if (val1 < 1)
					val1 = 1;
				break;
			case SC_ILLUSION:
				val2 = 5+val1; //Factor by which displayed damage is increased by
				break;
			case SC_DOUBLECASTING:
				val2 = 30+10*val1; //Trigger rate
				break;
			case SC_KAIZEL:
				val2 = 10*val1; //% of life to be revived with
				break;
				// case SC_ARMORPROPERTY:
				// case SC_ARMOR_RESIST:
				// Mod your resistance against elements:
				// val1 = water | val2 = earth | val3 = fire | val4 = wind
				// break;
				//case ????:
				//Place here SCs that have no SCB_* data, no skill associated, no ICON
				//associated, and yet are not wrong/unknown. [Skotlex]
				//break;

			case SC_MER_FLEE:
			case SC_MER_ATK:
			case SC_MER_HIT:
				val2 = 15 * val1;
				break;
			case SC_MER_HP:
			case SC_MER_SP:
				val2 = 5 * val1;
				break;
			case SC_REBIRTH:
				val2 = 20*val1; //% of life to be revived with
				break;

			case SC_MANU_DEF:
			case SC_MANU_ATK:
			case SC_MANU_MATK:
				val2 = 1; // Manuk group
				break;
			case SC_SPL_DEF:
			case SC_SPL_ATK:
			case SC_SPL_MATK:
				val2 = 2; // Splendide group
				break;
				/**
				* General
				**/
			case SC_FEAR:
				val2 = 2;
				val4 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_BURNING:
				val4 = tick / 3000; // Total Ticks to Burn!!
				tick_time = 3000; // [GodLesZ] tick time
				break;
				/**
				* Rune Knight
				**/
			case SC_DEATHBOUND:
				val2 = 500 + 100 * val1;
				break;
			case SC_STONEHARDSKIN:
				if( sd )
					val1 = sd->status.job_level * pc->checkskill(sd, RK_RUNEMASTERY) / 4; //DEF/MDEF Increase
				break;
			case SC_ABUNDANCE:
				val4 = tick / 10000;
				tick_time = 10000; // [GodLesZ] tick time
				break;
				/**
				* Arch Bishop
				**/
			case SC_RENOVATIO:
				val4 = tick / 5000;
				tick_time = 5000;
				break;
			case SC_SECRAMENT:
				val2 = 10 * val1;
				break;
			case SC_VENOMIMPRESS:
				val2 = 10 * val1;
				break;
			case SC_WEAPONBLOCKING:
				val2 = 10 + 2 * val1; // Chance
				val4 = tick / 5000;
				tick_time = 5000; // [GodLesZ] tick time
				break;
			case SC_TOXIN:
				val4 = tick / 10000;
				tick_time = 10000; // [GodLesZ] tick time
				break;
			case SC_MAGICMUSHROOM:
				val4 = tick / 4000;
				tick_time = 4000; // [GodLesZ] tick time
				break;
			case SC_PYREXIA:
				status->change_start(src, bl,SC_BLIND,10000,val1,0,0,0,30000,11); // Blind status that last for 30 seconds
				val4 = tick / 3000;
				tick_time = 3000; // [GodLesZ] tick time
				break;
			case SC_LEECHESEND:
				val4 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_OBLIVIONCURSE:
				val4 = tick / 3000;
				tick_time = 3000; // [GodLesZ] tick time
				break;
			case SC_CLOAKINGEXCEED:
				val2 = ( val1 + 1 ) / 2; // Hits
				val3 = 90 + val1 * 10; // Walk speed
				if (bl->type == BL_PC)
					val4 |= battle_config.pc_cloak_check_type&7;
				else
					val4 |= battle_config.monster_cloak_check_type&7;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_HALLUCINATIONWALK:
				val2 = 50 * val1; // Evasion rate of physical attacks. Flee
				val3 = 10 * val1; // Evasion rate of magical attacks.
				break;
			case SC_WHITEIMPRISON:
				status_change_end(bl, SC_BURNING, INVALID_TIMER);
				status_change_end(bl, SC_FROSTMISTY, INVALID_TIMER);
				status_change_end(bl, SC_FREEZE, INVALID_TIMER);
				status_change_end(bl, SC_STONE, INVALID_TIMER);
				break;
			case SC_MARSHOFABYSS:
				val2 = 6 * val1;
				if( sd ) // half on players
					val2 >>= 1;
				break;
			case SC_FROSTMISTY:
				status_change_end(bl, SC_BURNING, INVALID_TIMER);
				break;
			case SC_READING_SB:
				// val2 = sp reduction per second
				tick_time = 10000; // [GodLesZ] tick time
				break;
			case SC_SUMMON1:
			case SC_SUMMON2:
			case SC_SUMMON3:
			case SC_SUMMON4:
			case SC_SUMMON5:
				val4 = tick / 1000;
				if( val4 < 1 )
					val4 = 1;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_SHAPESHIFT:
				switch( val1 )
				{
				case 1: val2 = ELE_FIRE; break;
				case 2: val2 = ELE_EARTH; break;
				case 3: val2 = ELE_WIND; break;
				case 4: val2 = ELE_WATER; break;
				}
				break;
			case SC_STEALTHFIELD_MASTER:
				val4 = tick / 1000;
				tick_time = 2000 + (1000 * val1);
				break;
			case SC_ELECTRICSHOCKER:
			case SC_COLD:
			case SC_MEIKYOUSISUI:
				val4 = tick / 1000;
				if( val4 < 1 )
					val4 = 1;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_CAMOUFLAGE:
				val4 = tick/1000;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_WUGDASH:
			{
				//Store time at which you started running.
				int64 currenttick = timer->gettick();
				// Note: this int64 value is stored in two separate int32 variables (FIXME)
				val3 = (int)(currenttick&0x00000000ffffffffLL);
				val4 = (int)((currenttick&0xffffffff00000000LL)>>32);
			}
				tick = -1;
				break;
			case SC__REPRODUCE:
				val4 = tick / 1000;
				tick_time = 1000;
				break;
			case SC__SHADOWFORM: {
				struct map_session_data * s_sd = map->id2sd(val2);
				if( s_sd )
					s_sd->shadowform_id = bl->id;
				val4 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
								 }
				break;
			case SC__STRIPACCESSARY:
				if (!sd)
					val2 = 20;
				break;
			case SC__INVISIBILITY:
				val2 = 50 - 10 * val1; // ASPD
				val3 = 200 * val1; // CRITICAL
				val4 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC__ENERVATION:
				val2 = 20 + 10 * val1; // ATK Reduction
				if( sd ) pc->delspiritball(sd,sd->spiritball,0);
				break;
			case SC__GROOMY:
				val2 = 20 + 10 * val1; //ASPD. Need to confirm if Movement Speed reduction is the same. [Jobbie]
				val3 = 20 * val1; //HIT
				if( sd ) { // Removes Animals
					if( pc_isriding(sd) ) pc->setriding(sd, 0);
					if( pc_isridingdragon(sd) ) pc->setoption(sd, sd->sc.option&~OPTION_DRAGON);
					if( pc_iswug(sd) ) pc->setoption(sd, sd->sc.option&~OPTION_WUG);
					if( pc_isridingwug(sd) ) pc->setoption(sd, sd->sc.option&~OPTION_WUGRIDER);
					if( pc_isfalcon(sd) ) pc->setoption(sd, sd->sc.option&~OPTION_FALCON);
					if( sd->status.pet_id > 0 ) pet->menu(sd, 3);
					if( homun_alive(sd->hd) ) homun->vaporize(sd,HOM_ST_REST);
					if( sd->md ) mercenary->delete(sd->md,3);
				}
				break;
			case SC__LAZINESS:
				val2 = 10 + 10 * val1; // Cast reduction
				val3 = 10 * val1; // Flee Reduction
				break;
			case SC__UNLUCKY:
				val2 = 10 * val1; // Crit and Flee2 Reduction
				break;
			case SC__WEAKNESS:
				val2 = 10 * val1;
				// bypasses coating protection and MADO
				sc_start(src, bl,SC_NOEQUIPWEAPON,100,val1,tick);
				sc_start(src, bl,SC_NOEQUIPSHIELD,100,val1,tick);
				break;
			case SC_GN_CARTBOOST:
				if( val1 < 3 )
					val2 = 50;
				else if( val1 < 5 )
					val2 = 75;
				else
					val2 = 100;
				break;
			case SC_PROPERTYWALK:
				val3 = 0;
				break;
			case SC_WARMER:
				status_change_end(bl, SC_FREEZE, INVALID_TIMER);
				status_change_end(bl, SC_FROSTMISTY, INVALID_TIMER);
				status_change_end(bl, SC_COLD, INVALID_TIMER);
				break;
			case SC_STRIKING:
				val1 = 6 - val1;//spcost = 6 - level (lvl1:5 ... lvl 5: 1)
				val4 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_BLOOD_SUCKER:
			{
				struct block_list *src2 = map->id2bl(val2);
				val3 = 1;
				if(src2)
					val3 = 200 + 100 * val1 + status_get_int(src2);
				val4 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
			}
				break;
			case SC_SWING:
				val3 = 5 * val1 + val2;//Movement Speed And ASPD Increase
				break;
			case SC_SYMPHONY_LOVE:
				val2 = 12 * val1 + val2 + sd->status.job_level / 4;//MDEF Increase In %
			case SC_MOONLIT_SERENADE:
			case SC_RUSH_WINDMILL:
				val2 = 6 * val1 + val2 + sd->status.job_level / 5;
				break;
			case SC_ECHOSONG:
				val3 = 6 * val1 + val2 + sd->status.job_level / 4;//DEF Increase In %
				break;
			case SC_HARMONIZE:
				val2 = 5 + 5 * val1;
				break;
			case SC_SIREN:
				val4 = tick / 2000;
				tick_time = 2000; // [GodLesZ] tick time
				break;
			case SC_DEEP_SLEEP:
				val4 = tick / 2000;
				tick_time = 2000; // [GodLesZ] tick time
				break;
			case SC_SIRCLEOFNATURE:
				val2 = 40 * val1;//HP recovery
				val3 = 4 * val1;//SP drain
				val4 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_SONG_OF_MANA:
				val3 = 10 + 5 * val2;
				val4 = tick/5000;
				tick_time = 5000; // [GodLesZ] tick time
				break;
			case SC_SATURDAY_NIGHT_FEVER:
				/*val2 = 12000 - 2000 * val1;//HP/SP Drain Timer
				if ( val2 < 1000 )
					val2 = 1000;//Added to prevent val3 from dividing by 0 when using level 6 or higher through commands. [Rytech]
				val3 = tick/val2;*/
				val3 = tick / 3000;
				tick_time = 3000;// [GodLesZ] tick time
				break;
			case SC_GLOOMYDAY:
				if ( !val2 ) {
					val2 = (val4 > 0 ? max(15, rnd()%(val4*5)) : 0) + val1 * 10;
				}
				if ( rnd()%10000 < val1*100 ) { // 1% per SkillLv chance
					if ( !val3 )
						val3 = 50;
					if( sd ) {
						if( pc_isriding(sd) ) pc->setriding(sd, 0);
						if( pc_isridingdragon(sd) ) pc->setoption(sd, sd->sc.option&~OPTION_DRAGON);
					}
				}
				break;
			case SC_SITDOWN_FORCE:
			case SC_BANANA_BOMB_SITDOWN_POSTDELAY:
				if( sd && !pc_issit(sd) )
				{
					pc_setsit(sd);
					skill->sit(sd,1);
					clif->sitting(bl);
				}
				break;
			case SC_DANCE_WITH_WUG:
				val3 = 5 + 5 * val2;//ASPD Increase
				val4 = 20 + 10 * val2;//Fixed Cast Time Reduction
				break;
			case SC_LERADS_DEW:
				val3 = 200 * val1 + 300 * val2;//MaxHP Increase
				break;
			case SC_MELODYOFSINK:
				val3 = val1 * (2 + val2);//INT Reduction. Formula Includes Caster And 2nd Performer.
				val4 = tick/1000;
				tick_time = 1000;
				break;
			case SC_BEYOND_OF_WARCRY:
				val3 = val1 * (2 + val2);//STR And Crit Reduction. Formula Includes Caster And 2nd Performer.
				val4 = 4 * val1 + 4 * val2;//MaxHP Reduction
				break;
			case SC_UNLIMITED_HUMMING_VOICE:
				{
					struct unit_data *ud = unit->bl2ud(bl);
					if( ud == NULL ) return 0;
					ud->state.skillcastcancel = 0;
					val3 = 15 - (3 * val2);//Increased SP Cost.
				}
				break;
			case SC_LG_REFLECTDAMAGE:
				val2 = 15 + 5 * val1;
				val3 = 25 + 5 * val1; //Number of Reflects
				val4 = tick/10000;
				tick_time = 10000; // [GodLesZ] tick time
				break;
			case SC_FORCEOFVANGUARD:
				val2 = 8 + 12 * val1; // Chance
				val3 = 5 + 2 * val1; // Max rage counters
				tick = -1; //endless duration in the client
				break;
			case SC_EXEEDBREAK:
				if( sd ){
					short index = sd->equip_index[EQI_HAND_R];
					val1 = 15 * (sd->status.job_level + val1 * 10);
					if( index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_WEAPON )
						val1 += (sd->inventory_data[index]->weight / 10 * sd->inventory_data[index]->wlv) * status->get_lv(bl) / 100;
				}
				break;
			case SC_PRESTIGE:
				val2 = (st->int_ + st->luk) * val1 / 20;// Chance to evade magic damage.
				val2 = val2 * status->get_lv(bl) / 200;
				val2 += val1;
				val1 *= 15; // Defence added
				if( sd )
					val1 += 10 * pc->checkskill(sd,CR_DEFENDER);
				val1 *= status->get_lv(bl) / 100;
				break;
			case SC_BANDING:
				tick_time = 5000; // [GodLesZ] tick time
				break;
			case SC_MAGNETICFIELD:
				val3 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_INSPIRATION:
				if( sd ) {
					val2 = 40 * val1 + 3 * sd->status.job_level;// ATK bonus
					val3 = sd->status.base_level / 10 + sd->status.job_level / 5;// All stat bonus
				}
				val4 = tick / 5000;
				tick_time = 5000; // [GodLesZ] tick time
				status->change_clear_buffs(bl,3); //Remove buffs/debuffs
				break;
			case SC_LIGHTNINGWALK: //  [(Job Level / 2) + (40 + 5 * Skill Level)] %
				val1 = (sd?sd->status.job_level:2)/2 + 40 + 5 * val1;
				break;
			case SC_RAISINGDRAGON:
				val3 = tick / 5000;
				tick_time = 5000; // [GodLesZ] tick time
				break;
			case SC_GENTLETOUCH_CHANGE:
			{// take note there is no def increase as skill desc says. [malufett]
				struct block_list * src2;
				val3 = st->agi * val1 / 60; // ASPD increase: [(Target AGI x Skill Level) / 60] %
				if( (src2 = map->id2bl(val2)) ){
					val4 = ( 200/status_get_int(src2) ) * val1;// MDEF decrease: MDEF [(200 / Caster INT) x Skill Level]
					val2 = ( status_get_dex(src2)/4 + status_get_str(src2)/2 ) * val1 / 5; // ATK increase: ATK [{(Caster DEX / 4) + (Caster STR / 2)} x Skill Level / 5]
				}
			}
				break;
			case SC_GENTLETOUCH_REVITALIZE:
			{// take note there is no vit,aspd,speed increase as skill desc says. [malufett]
				struct block_list * src2;
				val3 = val1 * 30 + 150; // Natural HP recovery increase: [(Skill Level x 30) + 50] %
				if( (src2 = map->id2bl(val2)) ) // the stat def is not shown in the status window and it is process differently
					val4 = ( status_get_vit(src2)/4 ) * val1; // STAT DEF increase: [(Caster VIT / 4) x Skill Level]
			}
				break;
			case SC_PYROTECHNIC_OPTION:
				val2 = 60;
				break;
			case SC_HEATER_OPTION:
				val2 = 120; // Watk. TODO: Renewal (Atk2)
				val3 = (sd ? sd->status.job_level : 0); // % Increase damage.
				val4 = 3;	// Change into fire element.
				break;
			case SC_TROPIC_OPTION:
				val2 = 180; // Watk. TODO: Renewal (Atk2)
				val3 = MG_FIREBOLT;
				break;
			case SC_AQUAPLAY_OPTION:
				val2 = 40;
				break;
			case SC_COOLER_OPTION:
				val2 = 80;	// Bonus Matk
				val3 = (sd ? sd->status.job_level : 0); // % Freezing chance
				val4 = 1;	// Change into water elemet
				break;
			case SC_CHILLY_AIR_OPTION:
				val2 = 120; // Matk. TODO: Renewal (Matk1)
				val3 = MG_COLDBOLT;
				break;
			case SC_WIND_STEP_OPTION:
				val2 = 50;	// % Increase speed and flee.
				break;
			case SC_BLAST_OPTION:
				val2 = (sd ? sd->status.job_level : 0); // % Increase damage
				val3 = ELE_WIND;
				break;
			case SC_WILD_STORM_OPTION:
				val2 = MG_LIGHTNINGBOLT;
				break;
			case SC_PETROLOGY_OPTION:
				val2 = 5;
				val3 = 50;
				break;
			case SC_SOLID_SKIN_OPTION:
				val2 = 33; // % Increase DEF
				break;
			case SC_CURSED_SOIL_OPTION:
				val2 = 10;
				val3 = (sd ? sd->status.job_level : 0); // % Increase Damage
				val4 = 2;
				break;
			case SC_UPHEAVAL_OPTION:
				val2 = WZ_EARTHSPIKE;
				val3 = 15; // Bonus MaxHP
				break;
			case SC_CIRCLE_OF_FIRE_OPTION:
				val2 = 300;
				break;
			case SC_FIRE_CLOAK_OPTION:
			case SC_WATER_DROP_OPTION:
			case SC_WIND_CURTAIN_OPTION:
			case SC_STONE_SHIELD_OPTION:
				val2 = 100;	// Elemental modifier.
				break;
			case SC_CIRCLE_OF_FIRE:
			case SC_FIRE_CLOAK:
			case SC_WATER_DROP:
			case SC_WATER_SCREEN:
			case SC_WIND_CURTAIN:
			case SC_WIND_STEP:
			case SC_STONE_SHIELD:
			case SC_SOLID_SKIN:
				val2 = 10;
				tick_time = 2000; // [GodLesZ] tick time
				break;
			case SC_WATER_BARRIER:
				val2 = 40;	// Increasement. Mdef1 ???
				val3 = 30;	// Reductions. Atk2, Flee1, Matk1 ????
				break;
			case SC_ZEPHYR:
				val2 = 25;	// Flee.
				break;
			case SC_TIDAL_WEAPON:
				val2 = 20; // Increase Elemental's attack.
				break;
			case SC_ROCK_CRUSHER:
			case SC_ROCK_CRUSHER_ATK:
			case SC_POWER_OF_GAIA:
				val2 = 33;
				break;
			case SC_MELON_BOMB:
			case SC_BANANA_BOMB:
				val1 = 15;
				break;
			case SC_STOMACHACHE:
				val2 = 8;	// SP consume.
				val4 = tick / 10000;
				tick_time = 10000; // [GodLesZ] tick time
				break;
			case SC_KYOUGAKU:
				val2 = 2*val1 + rand()%(3 * val1);
				clif->status_change(bl, SI_ACTIVE_MONSTER_TRANSFORM, 1, 0, 1002, 0, 0); // Poring in disguise
				break;
			case SC_KAGEMUSYA:
				val3 = val1 * 2;
			case SC_IZAYOI:
				val2 = tick/1000;
				tick_time = 1000;
				break;
			case SC_ZANGETSU:
				val2 = val4 = status->get_lv(bl) / 3 + 20 * val1;
				val3 = status->get_lv(bl) / 2 + 30 * val1;
				val2 = (!(status_get_hp(bl)%2) ? val2 : -val3);
				val3 = (!(status_get_sp(bl)%2) ? val4 : -val3);
				break;
			case SC_GENSOU:

#define PER( a, lvl ) do { \
	int temp__ = (a); \
	if( temp__ <= 15 ) (lvl) = 1; \
	else if( temp__ <= 30 ) (lvl) = 2; \
	else if( temp__ <= 50 ) (lvl) = 3; \
	else if( temp__ <= 75 ) (lvl) = 4; \
	else (lvl) = 5; \
} while(0)

			{
				int hp = status_get_hp(bl), sp = status_get_sp(bl), lv = 5;

				if( rand()%100 > (25 + 10 * val1) - status_get_int(bl) / 2)
					return 0;

				PER( 100 / (status_get_max_hp(bl) / hp), lv );
				status->heal(bl, (!(hp%2) ? (6-lv) *4 / 100 : -(lv*4) / 100), 0, 1);

				PER( 100 / (status_get_max_sp(bl) / sp), lv );
				status->heal(bl, 0,(!(sp%2) ? (6-lv) *3 / 100 : -(lv*3) / 100), 1);
			}
#undef PER
				break;
			case SC_ANGRIFFS_MODUS:
				val2 = 50 + 20 * val1; //atk bonus
				val3 = 40 + 20 * val1; // Flee reduction.
				val4 = tick/1000; // hp/sp reduction timer
				tick_time = 1000;
				break;
			case SC_NEUTRALBARRIER:
				tick_time = tick;
				tick = -1;
				break;
			case SC_GOLDENE_FERSE:
				val2 = 10 + 10*val1; //max hp bonus
				val3 = 6 + 4 * val1; // Aspd Bonus
				val4 = 2 + 2 * val1; // Chance of holy attack
				break;
			case SC_OVERED_BOOST:
				val2 = 300 + 40*val1; //flee bonus
				val3 = 179 + 2*val1; //aspd bonus
				break;
			case SC_GRANITIC_ARMOR:
				val2 = 2*val1; //dmg reduction
				val3 = 6*val1; //dmg on status end
				break;
			case SC_MAGMA_FLOW:
				val2 = 3*val1; //activation chance
				break;
			case SC_PYROCLASTIC:
				val2 += 10*val1; //atk bonus
				break;
			case SC_NEEDLE_OF_PARALYZE: //[Lighta] need real info
				val2 = 2*val1; //def reduction
				val3 = 500*val1; //varcast augmentation
				break;
			case SC_PAIN_KILLER: //[Lighta] need real info
				val2 = 2*val1; //aspd reduction %
				val3 = 2*val1; //dmg reduction %
				if(sc->data[SC_NEEDLE_OF_PARALYZE])
					sc_start(src, bl, SC_ENDURE, 100, val1, tick); //start endure for same duration
				break;
			case SC_STYLE_CHANGE: //[Lighta] need real info
				tick = -1;
				if(val2 == MH_MD_FIGHTING) val2 = MH_MD_GRAPPLING;
				else val2 = MH_MD_FIGHTING;
				break;
			case SC_FULL_THROTTLE:
				status_percent_heal(bl,100,0);
				val2 = 7 - val1;
				tick_time = 1000;
				val4 = tick / tick_time;
				break;
			case SC_KINGS_GRACE:
				val2 = 3 + val1;
				tick_time = 1000;
				val4 = tick / tick_time;
				break;
			case SC_TELEKINESIS_INTENSE:
				val2 = 10 * val1;
				val3 = 40 * val1;
				break;
			case SC_OFFERTORIUM:
				val2 = 30 * val1;
				break;
			case SC_FRIGG_SONG:
				val2 = 5 * val1;
				val3 = (20 * val1) + 80;
				tick_time = 1000;
				val4 = tick / tick_time;
				break;
			case SC_DARKCROW:
				val2 = 30 * val1;
				break;
			case SC_MONSTER_TRANSFORM:
				if( !mob->db_checkid(val1) )
					val1 = 1002; // default poring
				break;
			default:
				if( calc_flag == SCB_NONE && status->SkillChangeTable[type] == 0 && status->IconChangeTable[type] == 0 )
				{	//Status change with no calc, no icon, and no skill associated...?
					ShowError("UnknownStatusChange [%d]\n", type);
					return 0;
				}
		}
	} else { //Special considerations when loading SC data.
		switch( type ) {
			case SC_WEDDING:
			case SC_XMAS:
			case SC_SUMMER:
			case SC_HANBOK:
			case SC_OKTOBERFEST:
				if( !vd ) break;
				clif->changelook(bl,LOOK_BASE,vd->class_);
				clif->changelook(bl,LOOK_WEAPON,0);
				clif->changelook(bl,LOOK_SHIELD,0);
				clif->changelook(bl,LOOK_CLOTHES_COLOR,vd->cloth_color);
				break;
			case SC_KAAHI:
				val4 = INVALID_TIMER;
				break;
			case SC_KYOUGAKU:
				clif->status_change(bl, SI_ACTIVE_MONSTER_TRANSFORM, 1, 0, 1002, 0, 0); // Poring in disguise
				break;
		}
	}

	/* values that must be set regardless of flag&4 e.g. val_flag */
	switch(type) {
		case SC_FIGHTINGSPIRIT:
			val_flag |= 1|2;
			break;
		case SC_VENOMIMPRESS:
			val_flag |= 1|2;
			break;
		case SC_POISONINGWEAPON:
			val_flag |= 1|2|4;
			break;
		case SC_WEAPONBLOCKING:
			val_flag |= 1|2;
			break;
		case SC_ROLLINGCUTTER:
			val_flag |= 1;
			break;
		case SC_CLOAKINGEXCEED:
			val_flag |= 1|2|4;
			break;
		case SC_HALLUCINATIONWALK:
			val_flag |= 1|2|4;
			break;
		case SC_SUMMON1:
		case SC_SUMMON2:
		case SC_SUMMON3:
		case SC_SUMMON4:
		case SC_SUMMON5:
			val_flag |= 1;
			break;
		case SC__SHADOWFORM:
			val_flag |= 1|2|4;
			break;
		case SC__INVISIBILITY:
			val_flag |= 1|2;
			break;
		case SC__ENERVATION:
			val_flag |= 1|2;
			break;
		case SC__GROOMY:
			val_flag |= 1|2|4;
			break;
		case SC__LAZINESS:
			val_flag |= 1|2|4;
			break;
		case SC__UNLUCKY:
			val_flag |= 1|2|4;
			break;
		case SC__WEAKNESS:
			val_flag |= 1|2;
			break;
		case SC_PROPERTYWALK:
			val_flag |= 1|2;
			break;
		case SC_FORCEOFVANGUARD:
			val_flag |= 1|2|4;
			break;
		case SC_PRESTIGE:
			val_flag |= 1|2;
			break;
		case SC_BANDING:
			val_flag |= 1;
			break;
		case SC_SHIELDSPELL_DEF:
		case SC_SHIELDSPELL_MDEF:
		case SC_SHIELDSPELL_REF:
			val_flag |= 1|2;
			break;
		case SC_SPELLFIST:
		case SC_CURSEDCIRCLE_ATKER:
			val_flag |= 1|2|4;
			break;
		case SC_CRESCENTELBOW:
			val_flag |= 1|2;
			break;
		case SC_LIGHTNINGWALK:
			val_flag |= 1;
			break;
		case SC_PYROTECHNIC_OPTION:
			val_flag |= 1|2|4;
			break;
		case SC_HEATER_OPTION:
			val_flag |= 1|2|4;
			break;
		case SC_AQUAPLAY_OPTION:
			val_flag |= 1|2|4;
			break;
		case SC_COOLER_OPTION:
			val_flag |= 1|2|4;
			break;
		case SC_CHILLY_AIR_OPTION:
			val_flag |= 1|2;
			break;
		case SC_GUST_OPTION:
			val_flag |= 1|2;
			break;
		case SC_BLAST_OPTION:
			val_flag |= 1|2|4;
			break;
		case SC_WILD_STORM_OPTION:
			val_flag |= 1|2;
			break;
		case SC_PETROLOGY_OPTION:
			val_flag |= 1|2|4;
			break;
		case SC_CURSED_SOIL_OPTION:
			val_flag |= 1|2|4;
			break;
		case SC_UPHEAVAL_OPTION:
			val_flag |= 1|2;
			break;
		case SC_CIRCLE_OF_FIRE_OPTION:
			val_flag |= 1|2;
			break;
		case SC_WATER_BARRIER:
			val_flag |= 1|2|4;
			break;
		case SC_CASH_PLUSEXP:
		case SC_CASH_PLUSONLYJOBEXP:
		case SC_MONSTER_TRANSFORM:
		case SC_CASH_RECEIVEITEM:
			val_flag |= 1;
			break;
	}
	
	/* [Ind/Hercules] */
	if( sd && status->DisplayType[type] ) {
		int dval1 = 0, dval2 = 0, dval3 = 0;
		switch( type ) {
			case SC_ALL_RIDING:
				dval1 = 1;
				break;
			default: /* all others: just copy val1 */
				dval1 = val1;
				break;
		}
		status->display_add(sd,type,dval1,dval2,dval3);
	}

	//Those that make you stop attacking/walking....
	switch (type) {
		case SC_VACUUM_EXTREME:
			if(!map_flag_gvg(bl->m))
				unit->stop_walking(bl,1);
			break;
		case SC_FREEZE:
		case SC_STUN:
		case SC_SLEEP:
		case SC_STONE:
		case SC_DEEP_SLEEP:
			if (sd && pc_issit(sd)) //Avoid sprite sync problems.
				pc->setstand(sd);
		case SC_TRICKDEAD:
			status_change_end(bl, SC_DANCING, INVALID_TIMER);
			// Cancel cast when get status [LuzZza]
			if (battle_config.sc_castcancel&bl->type)
				unit->skillcastcancel(bl, 0);
		case SC_WHITEIMPRISON:
			unit->stop_attack(bl);
		case SC_STOP:
		case SC_CONFUSION:
		case SC_RG_CCONFINE_M:
		case SC_RG_CCONFINE_S:
		case SC_SPIDERWEB:
		case SC_ELECTRICSHOCKER:
		case SC_WUGBITE:
		case SC_THORNS_TRAP:
		case SC__MANHOLE:
		case SC__CHAOS:
		case SC_COLD:
		case SC_CURSEDCIRCLE_ATKER:
		case SC_CURSEDCIRCLE_TARGET:
		case SC_FEAR:
		case SC_MEIKYOUSISUI:
		case SC_KYOUGAKU:
		case SC_NEEDLE_OF_PARALYZE:
		case SC_DEATHBOUND:
			unit->stop_walking(bl,1);
			break;
		case SC_ANKLESNARE:
			if( battle_config.skill_trap_type || !map_flag_gvg(bl->m) )
				unit->stop_walking(bl,1);
			break;
		case SC_HIDING:
		case SC_CLOAKING:
		case SC_CLOAKINGEXCEED:
		case SC_CHASEWALK:
		case SC_WEIGHTOVER90:
		case SC_CAMOUFLAGE:
		case SC_SIREN:
			unit->stop_attack(bl);
			break;
		case SC_SILENCE:
			if (battle_config.sc_castcancel&bl->type)
				unit->skillcastcancel(bl, 0);
			break;
			/* */
		case SC_ITEMSCRIPT:
			if( sd ) {
				switch( val1 ) {
					//case ITEMID_PHREEONI_CARD:
					//case ITEMID_GHOSTRING_CARD:
					case ITEMID_TAO_GUNKA_CARD:
						clif->status_change(bl,SI_MVPCARD_TAOGUNKA,1,tick,0,0,0);
						break;
					case ITEMID_MISTRESS_CARD:
						clif->status_change(bl,SI_MVPCARD_MISTRESS,1,tick,0,0,0);
						break;
					case ITEMID_ORC_HERO_CARD:
						clif->status_change(bl,SI_MVPCARD_ORCHERO,1,tick,0,0,0);
						break;
					case ITEMID_ORC_LOAD_CARD:
						clif->status_change(bl,SI_MVPCARD_ORCLORD,1,tick,0,0,0);
						break;
				}
			}
			break;
	}

	// Set option as needed.
	opt_flag = 1;
	switch(type) {
		//OPT1
		case SC_STONE:  sc->opt1 = OPT1_STONEWAIT; break;
		case SC_FREEZE: sc->opt1 = OPT1_FREEZE;    break;
		case SC_STUN:   sc->opt1 = OPT1_STUN;      break;
		case SC_SLEEP:   sc->opt1 = OPT1_SLEEP;		break;
		case SC_BURNING:		sc->opt1 = OPT1_BURNING;	break; // Burning need this to be showed correctly. [pakpil]
		case SC_WHITEIMPRISON:  sc->opt1 = OPT1_IMPRISON;	break;
		case SC_COLD:		sc->opt1 = OPT1_CRYSTALIZE;	break;
		//OPT2
		case SC_POISON:       sc->opt2 |= OPT2_POISON;       break;
		case SC_CURSE:        sc->opt2 |= OPT2_CURSE;        break;
		case SC_SILENCE:      sc->opt2 |= OPT2_SILENCE;      break;

		case SC_CRUCIS:
		case SC__CHAOS:
			sc->opt2 |= OPT2_SIGNUMCRUCIS;
			break;

		case SC_BLIND:        sc->opt2 |= OPT2_BLIND;        break;
		case SC_ANGELUS:      sc->opt2 |= OPT2_ANGELUS;      break;
		case SC_BLOODING:     sc->opt2 |= OPT2_BLEEDING;     break;
		case SC_DPOISON:      sc->opt2 |= OPT2_DPOISON;      break;
		//OPT3
		case SC_TWOHANDQUICKEN:
		case SC_ONEHANDQUICKEN:
		case SC_SPEARQUICKEN:
		case SC_LKCONCENTRATION:
		case SC_MER_QUICKEN:
			sc->opt3 |= OPT3_QUICKEN;
			opt_flag = 0;
			break;
		case SC_OVERTHRUSTMAX:
		case SC_OVERTHRUST:
		case SC_SWOO:	//Why does it shares the same opt as Overthrust? Perhaps we'll never know...
			sc->opt3 |= OPT3_OVERTHRUST;
			opt_flag = 0;
			break;
		case SC_ENERGYCOAT:
		case SC_SKE:
			sc->opt3 |= OPT3_ENERGYCOAT;
			opt_flag = 0;
			break;
		case SC_INCATKRATE:
			//Simulate Explosion Spirits effect for NPC_POWERUP [Skotlex]
			if (bl->type != BL_MOB) {
				opt_flag = 0;
				break;
			}
		case SC_EXPLOSIONSPIRITS:
			sc->opt3 |= OPT3_EXPLOSIONSPIRITS;
			opt_flag = 0;
			break;
		case SC_STEELBODY:
		case SC_SKA:
			sc->opt3 |= OPT3_STEELBODY;
			opt_flag = 0;
			break;
		case SC_BLADESTOP:
			sc->opt3 |= OPT3_BLADESTOP;
			opt_flag = 0;
			break;
		case SC_AURABLADE:
			sc->opt3 |= OPT3_AURABLADE;
			opt_flag = 0;
			break;
		case SC_BERSERK:
			opt_flag = 0;
			sc->opt3 |= OPT3_BERSERK;
			break;
//		case ???: // doesn't seem to do anything
//			sc->opt3 |= OPT3_LIGHTBLADE;
//			opt_flag = 0;
//			break;
		case SC_DANCING:
			if ((val1&0xFFFF) == CG_MOONLIT)
				sc->opt3 |= OPT3_MOONLIT;
			opt_flag = 0;
			break;
		case SC_MARIONETTE_MASTER:
		case SC_MARIONETTE:
			sc->opt3 |= OPT3_MARIONETTE;
			opt_flag = 0;
			break;
		case SC_ASSUMPTIO:
			sc->opt3 |= OPT3_ASSUMPTIO;
			opt_flag = 0;
			break;
		case SC_WARM: //SG skills [Komurka]
			sc->opt3 |= OPT3_WARM;
			opt_flag = 0;
			break;
		case SC_KAITE:
			sc->opt3 |= OPT3_KAITE;
			opt_flag = 0;
			break;
		case SC_NJ_BUNSINJYUTSU:
			sc->opt3 |= OPT3_BUNSIN;
			opt_flag = 0;
			break;
		case SC_SOULLINK:
			sc->opt3 |= OPT3_SOULLINK;
			opt_flag = 0;
			break;
		case SC_PROPERTYUNDEAD:
			sc->opt3 |= OPT3_UNDEAD;
			opt_flag = 0;
			break;
//		case ???: // from DA_CONTRACT (looks like biolab mobs aura)
//			sc->opt3 |= OPT3_CONTRACT;
//			opt_flag = 0;
//			break;
		//OPTION
		case SC_HIDING:
			sc->option |= OPTION_HIDE;
			opt_flag = 2;
			break;
		case SC_CLOAKING:
		case SC_CLOAKINGEXCEED:
			sc->option |= OPTION_CLOAK;
			opt_flag = 2;
			break;
		case SC__INVISIBILITY:
		case SC_CHASEWALK:
			sc->option |= OPTION_CHASEWALK|OPTION_CLOAK;
			opt_flag = 2;
			break;
		case SC_SIGHT:
			sc->option |= OPTION_SIGHT;
			break;
		case SC_RUWACH:
			sc->option |= OPTION_RUWACH;
			break;
		case SC_WEDDING:
			sc->option |= OPTION_WEDDING;
			opt_flag |= 0x4;
			break;
		case SC_XMAS:
			sc->option |= OPTION_XMAS;
			opt_flag |= 0x4;
			break;
		case SC_SUMMER:
			sc->option |= OPTION_SUMMER;
			opt_flag |= 0x4;
			break;
		case SC_HANBOK:
			sc->option |= OPTION_HANBOK;
			opt_flag |= 0x4;
			break;
		case SC_ORCISH:
			sc->option |= OPTION_ORCISH;
			break;
		case SC_FUSION:
			sc->option |= OPTION_FLYING;
			break;
		case SC_OKTOBERFEST:
			sc->option |= OPTION_OKTOBERFEST;
			opt_flag |= 0x4;
			break;
		default:
			opt_flag = 0;
	}

	//On Aegis, when turning on a status change, first goes the option packet, then the sc packet.
	if(opt_flag) {			
		clif->changeoption(bl);
		if( sd && opt_flag&0x4 ) {
			clif->changelook(bl,LOOK_BASE,vd->class_);
			clif->changelook(bl,LOOK_WEAPON,0);
			clif->changelook(bl,LOOK_SHIELD,0);
			clif->changelook(bl,LOOK_CLOTHES_COLOR,vd->cloth_color);
		}
	}
	if (calc_flag&SCB_DYE) {	//Reset DYE color
		if (vd && vd->cloth_color) {
			val4 = vd->cloth_color;
			clif->changelook(bl,LOOK_CLOTHES_COLOR,0);
		}
		calc_flag&=~SCB_DYE;
	}

	if( !(flag&4 && status->DisplayType[type]) )
		clif->status_change(bl,status->IconChangeTable[type],1,tick,(val_flag&1)?val1:1,(val_flag&2)?val2:0,(val_flag&4)?val3:0);

	/**
	* used as temporary storage for scs with interval ticks, so that the actual duration is sent to the client first.
	**/
	if( tick_time )
		tick = tick_time;

	//Don't trust the previous sce assignment, in case the SC ended somewhere between there and here.
	if((sce=sc->data[type])) {// reuse old sc
		if( sce->timer != INVALID_TIMER )
			timer->delete(sce->timer, status->change_timer);
	} else {// new sc
		++(sc->count);
		sce = sc->data[type] = ers_alloc(status->data_ers, struct status_change_entry);
	}
	sce->val1 = val1;
	sce->val2 = val2;
	sce->val3 = val3;
	sce->val4 = val4;
	if (tick >= 0)
		sce->timer = timer->add(timer->gettick() + tick, status->change_timer, bl->id, type);
	else {
		sce->timer = INVALID_TIMER; //Infinite duration
		if( sd )
			chrif->save_scdata_single(sd->status.account_id,sd->status.char_id,type,sce);
	}

	if (calc_flag)
		status_calc_bl(bl,calc_flag);

	if(sd && sd->pd)
		pet->sc_check(sd, type); //Skotlex: Pet Status Effect Healing

	switch (type) {
		case SC_BERSERK:
			if (!(sce->val2)) { //don't heal if already set
				status->heal(bl, st->max_hp, 0, 1); //Do not use percent_heal as this healing must override BERSERK's block.
				status->set_sp(bl, 0, 0); //Damage all SP
			}
			sce->val2 = 5 * st->max_hp / 100;
			break;
		case SC_HLIF_CHANGE:
			status_percent_heal(bl, 100, 100);
			break;
		case SC_RUN:
			{
				struct unit_data *ud = unit->bl2ud(bl);
				if( ud )
					ud->state.running = unit->run(bl);
			}
			break;
		case SC_CASH_BOSS_ALARM:
			clif->bossmapinfo(sd->fd, map->id2boss(sce->val1), 0); // First Message
			break;
		case SC_MER_HP:
			status_percent_heal(bl, 100, 0); // Recover Full HP
			break;
		case SC_MER_SP:
			status_percent_heal(bl, 0, 100); // Recover Full SP
			break;
			/**
			* Ranger
			**/
		case SC_WUGDASH:
			{
				struct unit_data *ud = unit->bl2ud(bl);
				if( ud )
					ud->state.running = unit->wugdash(bl, sd);
			}
			break;
		case SC_COMBOATTACK:
			switch (sce->val1) {
			case TK_STORMKICK:
				clif->skill_nodamage(bl,bl,TK_READYSTORM,1,1);
				break;
			case TK_DOWNKICK:
				clif->skill_nodamage(bl,bl,TK_READYDOWN,1,1);
				break;
			case TK_TURNKICK:
				clif->skill_nodamage(bl,bl,TK_READYTURN,1,1);
				break;
			case TK_COUNTER:
				clif->skill_nodamage(bl,bl,TK_READYCOUNTER,1,1);
				break;
			case MO_COMBOFINISH:
			case CH_TIGERFIST:
			case CH_CHAINCRUSH:
				if (sd)
					clif->skillinfo(sd,MO_EXTREMITYFIST, INF_SELF_SKILL);
				break;
			case TK_JUMPKICK:
				if (sd)
					clif->skillinfo(sd,TK_JUMPKICK, INF_SELF_SKILL);
				break;
			case MO_TRIPLEATTACK:
				if (sd && pc->checkskill(sd, SR_DRAGONCOMBO) > 0)
					clif->skillinfo(sd,SR_DRAGONCOMBO, INF_SELF_SKILL);
				break;
			case SR_FALLENEMPIRE:
				if (sd){
					clif->skillinfo(sd,SR_GATEOFHELL, INF_SELF_SKILL);
					clif->skillinfo(sd,SR_TIGERCANNON, INF_SELF_SKILL);
				}
				break;
			}
			break;
			case SC_RAISINGDRAGON:
				sce->val2 = st->max_hp / 100;// Officially tested its 1%hp drain. [Jobbie]
			break;
	}

	if( opt_flag&2 && sd && sd->touching_id )
		npc->touchnext_areanpc(sd,false); // run OnTouch_ on next char in range

	return 1;
}
/*==========================================
* Ending all status except those listed.
* @TODO maybe usefull for dispel instead reseting a liste there.
* type:
* 0 - PC killed -> Place here statuses that do not dispel on death.
* 1 - If for some reason status_change_end decides to still keep the status when quitting.
* 2 - Do clif
* 3 - Do not remove some permanent/time-independent effects
*------------------------------------------*/
int status_change_clear(struct block_list* bl, int type) {
	struct status_change* sc;
	int i;

	sc = status->get_sc(bl);

	if (!sc || !sc->count)
		return 0;

	for(i = 0; i < SC_MAX; i++) {
		if(!sc->data[i])
			continue;

		if(type == 0){
			if( status->get_sc_type(i)&SC_NO_REM_DEATH ) {
				switch (i) {
					case SC_ARMOR_PROPERTY://Only when its Holy or Dark that it doesn't dispell on death
						if( sc->data[i]->val2 != ELE_HOLY && sc->data[i]->val2 != ELE_DARK )
							break;
					default:
						continue;
				}
			}
		}
		if( type == 3 && status->get_sc_type(i)&SC_NO_CLEAR )
			continue;

		status_change_end(bl, (sc_type)i, INVALID_TIMER);

		if( type == 1 && sc->data[i] ) {
			//If for some reason status_change_end decides to still keep the status when quitting. [Skotlex]
			(sc->count)--;
			if (sc->data[i]->timer != INVALID_TIMER)
				timer->delete(sc->data[i]->timer, status->change_timer);
			ers_free(status->data_ers, sc->data[i]);
			sc->data[i] = NULL;
		}
	}

	sc->opt1 = 0;
	sc->opt2 = 0;
	sc->opt3 = 0;
	sc->bs_counter = 0;
	sc->fv_counter = 0;
#ifndef RENEWAL
	sc->sg_counter = 0;
#endif

	if( type == 0 || type == 2 )
		clif->changeoption(bl);

	return 1;
}

/*==========================================
* Special condition we want to effectuate, check before ending a status.
*------------------------------------------*/
int status_change_end_(struct block_list* bl, enum sc_type type, int tid, const char* file, int line) {
	struct map_session_data *sd;
	struct status_change *sc;
	struct status_change_entry *sce;
	struct status_data *st;
	struct view_data *vd;
	int opt_flag=0, calc_flag;
#ifdef ANTI_MAYAP_CHEAT
	bool invisible = false;
#endif
	
	nullpo_ret(bl);

	sc = status->get_sc(bl);
	st = status->get_status_data(bl);

	if(type < 0 || type >= SC_MAX || !sc || !(sce = sc->data[type]))
		return 0;

	sd = BL_CAST(BL_PC,bl);

	if (sce->timer != tid && tid != INVALID_TIMER)
		return 0;

	if( sd && sce->timer == INVALID_TIMER )
		chrif->del_scdata_single(sd->status.account_id,sd->status.char_id,type);
	
	if (tid == INVALID_TIMER) {
		if (type == SC_ENDURE && sce->val4)
			//Do not end infinite endure.
				return 0;
		if (sce->timer != INVALID_TIMER) //Could be a SC with infinite duration
			timer->delete(sce->timer,status->change_timer);
		if (sc->opt1)
			switch (type) {
				//"Ugly workaround"  [Skotlex]
				//delays status change ending so that a skill that sets opt1 fails to
				//trigger when it also removed one
				case SC_STONE:
					sce->val3 = 0; //Petrify time counter.
				case SC_FREEZE:
				case SC_STUN:
				case SC_SLEEP:
					if (sce->val1) {
						//Removing the 'level' shouldn't affect anything in the code
						//since these SC are not affected by it, and it lets us know
						//if we have already delayed this attack or not.
						sce->val1 = 0;
						sce->timer = timer->add(timer->gettick()+10, status->change_timer, bl->id, type);
						return 1;
					}
		}
	}

	(sc->count)--;

	sc->data[type] = NULL;

	if( sd && status->DisplayType[type] ) {
		status->display_remove(sd,type);
	}

#ifdef ANTI_MAYAP_CHEAT
	if (sc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_INVISIBLE))
		invisible = true;
#endif

	vd = status->get_viewdata(bl);
	calc_flag = status->ChangeFlagTable[type];
	switch(type) {
		case SC_GRANITIC_ARMOR:
		{
			int damage = st->max_hp*sce->val3/100;
			if(st->hp < damage) //to not kill him
				damage = st->hp-1;
			status->damage(NULL, bl, damage,0,0,1);
		}
			break;
		case SC_PYROCLASTIC:
			if(bl->type == BL_PC)
				skill->break_equip(bl,EQP_WEAPON,10000,BCT_SELF);
			break;
		case SC_RUN:
			{
				struct unit_data *ud = unit->bl2ud(bl);
				bool begin_spurt = true;
				// Note: this int64 value is stored in two separate int32 variables (FIXME)
				int64 starttick  = (int64)sce->val3&0x00000000ffffffffLL;
					  starttick |= ((int64)sce->val4<<32)&0xffffffff00000000LL;

				if (ud) {
					if(!ud->state.running)
						begin_spurt = false;
					ud->state.running = 0;
					if (ud->walktimer != INVALID_TIMER)
						unit->stop_walking(bl,1);
				}
				if (begin_spurt && sce->val1 >= 7
				 && DIFF_TICK(timer->gettick(), starttick) <= 1000
				 && (!sd || (sd->weapontype1 == 0 && sd->weapontype2 == 0))
				)
					sc_start(bl, bl,SC_STRUP,100,sce->val1,skill->get_time2(status->sc2skill(type), sce->val1));
			}
			break;
		case SC_AUTOBERSERK:
			if (sc->data[SC_PROVOKE] && sc->data[SC_PROVOKE]->val2 == 1)
				status_change_end(bl, SC_PROVOKE, INVALID_TIMER);
			break;

		case SC_ENDURE:
		case SC_DEFENDER:
		case SC_REFLECTSHIELD:
		case SC_AUTOGUARD:
			{
				struct map_session_data *tsd;
				if( bl->type == BL_PC ) {
					// Clear Status from others
					int i;
					for( i = 0; i < 5; i++ ) {
						if( sd->devotion[i] && (tsd = map->id2sd(sd->devotion[i])) && tsd->sc.data[type] )
							status_change_end(&tsd->bl, type, INVALID_TIMER);
					}
				} else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag ) {
					// Clear Status from Master
					tsd = ((TBL_MER*)bl)->master;
					if( tsd && tsd->sc.data[type] )
						status_change_end(&tsd->bl, type, INVALID_TIMER);
				}
			}
			break;
		case SC_DEVOTION:
			{
				struct block_list *d_bl = map->id2bl(sce->val1);
				if( d_bl ) {
					if( d_bl->type == BL_PC )
						((TBL_PC*)d_bl)->devotion[sce->val2] = 0;
					else if( d_bl->type == BL_MER )
						((TBL_MER*)d_bl)->devotion_flag = 0;
					clif->devotion(d_bl, NULL);
				}

				status_change_end(bl, SC_AUTOGUARD, INVALID_TIMER);
				status_change_end(bl, SC_DEFENDER, INVALID_TIMER);
				status_change_end(bl, SC_REFLECTSHIELD, INVALID_TIMER);
				status_change_end(bl, SC_ENDURE, INVALID_TIMER);
			}
			break;

		case SC_BLADESTOP:
			if(sce->val4) {
				int target_id = sce->val4;
				struct block_list *tbl = map->id2bl(target_id);
				struct status_change *tsc = status->get_sc(tbl);
				sce->val4 = 0;
				if(tbl && tsc && tsc->data[SC_BLADESTOP]) {
					tsc->data[SC_BLADESTOP]->val4 = 0;
					status_change_end(tbl, SC_BLADESTOP, INVALID_TIMER);
				}
				clif->bladestop(bl, target_id, 0);
			}
			break;
		case SC_DANCING:
			{
				const char* prevfile = "<unknown>";
				int prevline = 0;
				struct map_session_data *dsd;
				struct status_change_entry *dsc;
				struct skill_unit_group *group;

				if( sd )
				{
					if( sd->delunit_prevfile )
					{// initially this is NULL, when a character logs in
						prevfile = sd->delunit_prevfile;
						prevline = sd->delunit_prevline;
					}
					else
					{
						prevfile = "<none>";
					}
					sd->delunit_prevfile = file;
					sd->delunit_prevline = line;
				}

				if(sce->val4 && sce->val4 != BCT_SELF && (dsd=map->id2sd(sce->val4)))
				{// end status on partner as well
					dsc = dsd->sc.data[SC_DANCING];
					if(dsc) {

						//This will prevent recursive loops.
						dsc->val2 = dsc->val4 = 0;

						status_change_end(&dsd->bl, SC_DANCING, INVALID_TIMER);
					}
				}

				if(sce->val2)
				{// erase associated land skill
					group = skill->id2group(sce->val2);

					if( group == NULL )
					{
						ShowDebug("status_change_end: SC_DANCING is missing skill unit group (val1=%d, val2=%d, val3=%d, val4=%d, timer=%d, tid=%d, char_id=%d, map=%s, x=%d, y=%d, prev=%s:%d, from=%s:%d). Please report this! (#3504)\n",
							sce->val1, sce->val2, sce->val3, sce->val4, sce->timer, tid,
							sd ? sd->status.char_id : 0,
							mapindex_id2name(map_id2index(bl->m)), bl->x, bl->y,
							prevfile, prevline,
							file, line);
					}

					sce->val2 = 0;
					skill->del_unitgroup(group,ALC_MARK);
				}

				if((sce->val1&0xFFFF) == CG_MOONLIT)
					clif->sc_end(bl,bl->id,AREA,SI_MOON);

				status_change_end(bl, SC_LONGING, INVALID_TIMER);
			}
			break;
		case SC_NOCHAT:
			if (sd && sd->status.manner < 0 && tid != INVALID_TIMER)
				sd->status.manner = 0;
			if (sd && tid == INVALID_TIMER)
			{
				clif->changestatus(sd,SP_MANNER,sd->status.manner);
				clif->updatestatus(sd,SP_MANNER);
			}
			break;
		case SC_SPLASHER:
			{
				struct block_list *src=map->id2bl(sce->val3);
				if(src && tid != INVALID_TIMER)
					skill->castend_damage_id(src, bl, sce->val2, sce->val1, timer->gettick(), SD_LEVEL );
			}
			break;
		case SC_RG_CCONFINE_S:
			{
				struct block_list *src = sce->val2 ? map->id2bl(sce->val2) : NULL;
				struct status_change *sc2 = src ? status->get_sc(src) : NULL;
				if (src && sc2 && sc2->data[SC_RG_CCONFINE_M]) {
					//If status was already ended, do nothing.
					//Decrease count
					if (--(sc2->data[SC_RG_CCONFINE_M]->val1) <= 0) //No more holds, free him up.
						status_change_end(src, SC_RG_CCONFINE_M, INVALID_TIMER);
				}
			}
		case SC_RG_CCONFINE_M:
			if (sce->val2 > 0) {
				//Caster has been unlocked... nearby chars need to be unlocked.
				int range = 1
					+skill->get_range2(bl, status->sc2skill(type), sce->val1)
					+skill->get_range2(bl, TF_BACKSLIDING, 1); //Since most people use this to escape the hold....
				map->foreachinarea(status->change_timer_sub,
					bl->m, bl->x-range, bl->y-range, bl->x+range,bl->y+range,BL_CHAR,bl,sce,type,timer->gettick());
			}
			break;
		case SC_COMBOATTACK:
			if( sd )
				switch (sce->val1) {
				case MO_COMBOFINISH:
				case CH_TIGERFIST:
				case CH_CHAINCRUSH:
					clif->skillinfo(sd, MO_EXTREMITYFIST, 0);
					break;
				case TK_JUMPKICK:
					clif->skillinfo(sd, TK_JUMPKICK, 0);
					break;
				case MO_TRIPLEATTACK:
					if (pc->checkskill(sd, SR_DRAGONCOMBO) > 0)
						clif->skillinfo(sd, SR_DRAGONCOMBO, 0);
					break;
				case SR_FALLENEMPIRE:
					clif->skillinfo(sd, SR_GATEOFHELL, 0);
					clif->skillinfo(sd, SR_TIGERCANNON, 0);
					break;
			}
			break;

		case SC_MARIONETTE_MASTER:
		case SC_MARIONETTE: /// Marionette target
			if (sce->val1) {
				// check for partner and end their marionette status as well
				enum sc_type type2 = (type == SC_MARIONETTE_MASTER) ? SC_MARIONETTE : SC_MARIONETTE_MASTER;
				struct block_list *pbl = map->id2bl(sce->val1);
				struct status_change* sc2 = pbl ? status->get_sc(pbl) : NULL;

				if (sc2 && sc2->data[type2])
				{
					sc2->data[type2]->val1 = 0;
					status_change_end(pbl, type2, INVALID_TIMER);
				}
			}
			break;

		case SC_BERSERK:
			if(st->hp > 200 && sc && sc->data[SC__BLOODYLUST]) {
				status_percent_heal(bl, 100, 0);
				status_change_end(bl, SC__BLOODYLUST, INVALID_TIMER);
			} else if(st->hp > 100 && sce->val2) //If val2 is removed, no HP penalty (dispelled?) [Skotlex]
				status->set_hp(bl, 100, 0);
			if(sc->data[SC_ENDURE] && sc->data[SC_ENDURE]->val4 == 2) {
				sc->data[SC_ENDURE]->val4 = 0;
				status_change_end(bl, SC_ENDURE, INVALID_TIMER);
			}
			sc_start4(bl, bl, SC_GDSKILL_REGENERATION, 100, 10,0,0,(RGN_HP|RGN_SP), skill->get_time(LK_BERSERK, sce->val1));
			if( type == SC_SATURDAY_NIGHT_FEVER ) //Sit down force of Saturday Night Fever has the duration of only 3 seconds.
				sc_start(bl,bl,SC_SITDOWN_FORCE,100,sce->val1,skill->get_time2(WM_SATURDAY_NIGHT_FEVER,sce->val1));
			break;
		case SC_GOSPEL:
			if (sce->val3) { //Clear the group.
				struct skill_unit_group* group = skill->id2group(sce->val3);
				sce->val3 = 0;
				skill->del_unitgroup(group,ALC_MARK);
			}
			break;
		case SC_HERMODE:
			if(sce->val3 == BCT_SELF)
				skill->clear_unitgroup(bl);
			break;
		case SC_BASILICA: //Clear the skill area. [Skotlex]
			skill->clear_unitgroup(bl);
			break;
		case SC_TRICKDEAD:
			if (vd) vd->dead_sit = 0;
			break;
		case SC_WARM:
		case SC__MANHOLE:
			if (sce->val4) { //Clear the group.
				struct skill_unit_group* group = skill->id2group(sce->val4);
				sce->val4 = 0;
				if( group ) /* might have been cleared before status ended, e.g. land protector */
					skill->del_unitgroup(group,ALC_MARK);
			}
			break;
		case SC_KAAHI:
			//Delete timer if it exists.
			if (sce->val4 != INVALID_TIMER)
				timer->delete(sce->val4,status->kaahi_heal_timer);
			break;
		case SC_JAILED:
			if(tid == INVALID_TIMER)
				break;
			//natural expiration.
			if(sd && sd->mapindex == sce->val2)
				pc->setpos(sd,(unsigned short)sce->val3,sce->val4&0xFFFF, sce->val4>>16, CLR_TELEPORT);
			break; //guess hes not in jail :P
		case SC_HLIF_CHANGE:
			if (tid == INVALID_TIMER)
				break;
			// "lose almost all their HP and SP" on natural expiration.
			status->set_hp(bl, 10, 0);
			status->set_sp(bl, 10, 0);
			break;
		case SC_AUTOTRADE:
			if (tid == INVALID_TIMER)
				break;
			// Note: vending/buying is closed by unit_remove_map, no
			// need to do it here.
			map->quit(sd);
			// Because map->quit calls status_change_end with tid -1
			// from here it's not neccesary to continue
			return 1;
			break;
		case SC_STOP:
			if( sce->val2 ) {
				struct block_list *tbl = map->id2bl(sce->val2);
				struct status_change *tsc = NULL;
				sce->val2 = 0;
				if( tbl && (tsc = status->get_sc(tbl)) && tsc->data[SC_STOP] && tsc->data[SC_STOP]->val2 == bl->id )
					status_change_end(tbl, SC_STOP, INVALID_TIMER);
			}
			break;
		case SC_LKCONCENTRATION:
			status_change_end(bl, SC_ENDURE, INVALID_TIMER);
			break;
			/**
			* 3rd Stuff
			**/
		case SC_MILLENNIUMSHIELD:
			clif->millenniumshield(bl,0);
			break;
		case SC_HALLUCINATIONWALK:
			sc_start(bl,bl,SC_HALLUCINATIONWALK_POSTDELAY,100,sce->val1,skill->get_time2(GC_HALLUCINATIONWALK,sce->val1));
			break;
		case SC_WHITEIMPRISON:
			{
				struct block_list* src = map->id2bl(sce->val2);
				if( tid == -1 || !src)
					break; // Terminated by Damage
				status_fix_damage(src,bl,400*sce->val1,clif->damage(bl,bl,0,0,400*sce->val1,0,0,0));
			}
			break;
		case SC_WUGDASH:
			{
				struct unit_data *ud = unit->bl2ud(bl);
				if (ud) {
					ud->state.running = 0;
					if (ud->walktimer != -1)
						unit->stop_walking(bl,1);
				}
			}
			break;
		case SC_ADORAMUS:
			status_change_end(bl, SC_BLIND, INVALID_TIMER);
			break;
		case SC__SHADOWFORM:
			{
				struct map_session_data *s_sd = map->id2sd(sce->val2);
				if( !s_sd )
					break;
				s_sd->shadowform_id = 0;
			}
			break;
		case SC_SITDOWN_FORCE:
			if( sd && pc_issit(sd) ) {
				pc->setstand(sd);
				clif->standing(bl);
			}
			break;
		case SC_NEUTRALBARRIER_MASTER:
		case SC_STEALTHFIELD_MASTER:
			if( sce->val2 ) {
				struct skill_unit_group* group = skill->id2group(sce->val2);
				sce->val2 = 0;
				if( group ) /* might have been cleared before status ended, e.g. land protector */
					skill->del_unitgroup(group,ALC_MARK);
			}
			break;
		case SC_BANDING:
			if(sce->val4) {
				struct skill_unit_group *group = skill->id2group(sce->val4);
				sce->val4 = 0;
				if( group ) /* might have been cleared before status ended, e.g. land protector */
					skill->del_unitgroup(group,ALC_MARK);
			}
			break;
		case SC_CURSEDCIRCLE_ATKER:
			if( sce->val2 ) // used the default area size cause there is a chance the caster could knock back and can't clear the target.
				map->foreachinrange(status->change_timer_sub, bl, battle_config.area_size,BL_CHAR, bl, sce, SC_CURSEDCIRCLE_TARGET, timer->gettick());
			break;
		case SC_RAISINGDRAGON:
			if( sd && sce->val2 && !pc_isdead(sd) ) {
				int i;
				i = min(sd->spiritball,5);
				pc->delspiritball(sd, sd->spiritball, 0);
				status_change_end(bl, SC_EXPLOSIONSPIRITS, INVALID_TIMER);
				while( i > 0 ) {
					pc->addspiritball(sd, skill->get_time(MO_CALLSPIRITS, pc->checkskill(sd,MO_CALLSPIRITS)), 5);
					--i;
				}
			}
			break;
		case SC_CURSEDCIRCLE_TARGET:
		{
			struct block_list *src = map->id2bl(sce->val2);
			struct status_change *ssc = status->get_sc(src);
			if( ssc && ssc->data[SC_CURSEDCIRCLE_ATKER] && --(ssc->data[SC_CURSEDCIRCLE_ATKER]->val2) == 0 ){
				status_change_end(src, SC_CURSEDCIRCLE_ATKER, INVALID_TIMER);
				clif->bladestop(bl, sce->val2, 0);
			}
		}
			break;
		case SC_BLOOD_SUCKER:
			if( sce->val2 ){
				struct block_list *src = map->id2bl(sce->val2);
				if(src) {
					struct status_change *ssc = status->get_sc(src);
					ssc->bs_counter--;
				}
			}
			break;
		case SC_KYOUGAKU:
			clif->sc_end(&sd->bl,sd->bl.id,AREA,SI_ACTIVE_MONSTER_TRANSFORM);
			break;
		case SC_CLAIRVOYANCE:
			calc_flag = SCB_ALL;/* required for overlapping */
			break;
		case SC_FULL_THROTTLE:
			sc_start(bl,bl,SC_REBOUND,100,sce->val1,skill->get_time2(ALL_FULL_THROTTLE,sce->val1));
			break;
		case SC_MONSTER_TRANSFORM:
			if( sce->val2 )
				status_change_end(bl, (sc_type)sce->val2, INVALID_TIMER);
			break;
		case SC_ITEMSCRIPT:
			if( sd ) {
				switch( sce->val1 ) {
				//case ITEMID_PHREEONI_CARD:
				//case ITEMID_GHOSTRING_CARD:
				case ITEMID_TAO_GUNKA_CARD:
					clif->sc_end(&sd->bl, sd->bl.id, SELF, SI_MVPCARD_TAOGUNKA);
					break;
				case ITEMID_MISTRESS_CARD:
					clif->sc_end(&sd->bl, sd->bl.id, SELF, SI_MVPCARD_MISTRESS);
					break;
				case ITEMID_ORC_HERO_CARD:
					clif->sc_end(&sd->bl, sd->bl.id, SELF, SI_MVPCARD_ORCHERO);
					break;
				case ITEMID_ORC_LOAD_CARD:
					clif->sc_end(&sd->bl, sd->bl.id, SELF, SI_MVPCARD_ORCLORD);
					break;
				}
			}
			break;
	}

	opt_flag = 1;
	switch(type) {
		case SC_STONE:
		case SC_FREEZE:
		case SC_STUN:
		case SC_SLEEP:
		case SC_BURNING:
		case SC_WHITEIMPRISON:
		case SC_COLD:
			sc->opt1 = 0;
			break;

		case SC_POISON:
		case SC_CURSE:
		case SC_SILENCE:
		case SC_BLIND:
			sc->opt2 &= ~(1<<(type-SC_POISON));
			break;
		case SC_DPOISON:
			sc->opt2 &= ~OPT2_DPOISON;
			break;
		case SC_CRUCIS:
		case SC__CHAOS:
			sc->opt2 &= ~OPT2_SIGNUMCRUCIS;
			break;

		case SC_HIDING:
			sc->option &= ~OPTION_HIDE;
			opt_flag|= 2|4; //Check for warp trigger + AoE trigger
			break;
		case SC_CLOAKING:
		case SC_CLOAKINGEXCEED:
			sc->option &= ~OPTION_CLOAK;
		case SC_CAMOUFLAGE:
			opt_flag|= 2;
			break;
		case SC__INVISIBILITY:
		case SC_CHASEWALK:
			sc->option &= ~(OPTION_CHASEWALK|OPTION_CLOAK);
			opt_flag|= 2;
			break;
		case SC_SIGHT:
			sc->option &= ~OPTION_SIGHT;
			break;
		case SC_WEDDING:
			sc->option &= ~OPTION_WEDDING;
			opt_flag |= 0x4;
			break;
		case SC_XMAS:
			sc->option &= ~OPTION_XMAS;
			opt_flag |= 0x4;
			break;
		case SC_SUMMER:
			sc->option &= ~OPTION_SUMMER;
			opt_flag |= 0x4;
			break;
		case SC_HANBOK:
			sc->option &= ~OPTION_HANBOK;
			opt_flag |= 0x4;
			break;
		case SC_OKTOBERFEST:
			sc->option &= ~OPTION_OKTOBERFEST;
			opt_flag |= 0x4;
			break;
		case SC_ORCISH:
			sc->option &= ~OPTION_ORCISH;
			break;
		case SC_RUWACH:
			sc->option &= ~OPTION_RUWACH;
			break;
		case SC_FUSION:
			sc->option &= ~OPTION_FLYING;
			break;
			//opt3
		case SC_TWOHANDQUICKEN:
		case SC_ONEHANDQUICKEN:
		case SC_SPEARQUICKEN:
		case SC_CONCENTRATION:
		case SC_MER_QUICKEN:
			sc->opt3 &= ~OPT3_QUICKEN;
			opt_flag = 0;
			break;
		case SC_OVERTHRUST:
		case SC_OVERTHRUSTMAX:
		case SC_SWOO:
			sc->opt3 &= ~OPT3_OVERTHRUST;
			if( type == SC_SWOO )
				opt_flag = 8;
			else
				opt_flag = 0;
			break;
		case SC_ENERGYCOAT:
		case SC_SKE:
			sc->opt3 &= ~OPT3_ENERGYCOAT;
			opt_flag = 0;
			break;
		case SC_INCATKRATE: //Simulated Explosion spirits effect.
			if (bl->type != BL_MOB)
			{
				opt_flag = 0;
				break;
			}
		case SC_EXPLOSIONSPIRITS:
			sc->opt3 &= ~OPT3_EXPLOSIONSPIRITS;
			opt_flag = 0;
			break;
		case SC_STEELBODY:
		case SC_SKA:
			sc->opt3 &= ~OPT3_STEELBODY;
			opt_flag = 0;
			break;
		case SC_BLADESTOP:
			sc->opt3 &= ~OPT3_BLADESTOP;
			opt_flag = 0;
			break;
		case SC_AURABLADE:
			sc->opt3 &= ~OPT3_AURABLADE;
			opt_flag = 0;
			break;
		case SC_BERSERK:
			opt_flag = 0;
			sc->opt3 &= ~OPT3_BERSERK;
			break;
			//	case ???: // doesn't seem to do anything
			//		sc->opt3 &= ~OPT3_LIGHTBLADE;
			//		opt_flag = 0;
			//		break;
		case SC_DANCING:
			if ((sce->val1&0xFFFF) == CG_MOONLIT)
				sc->opt3 &= ~OPT3_MOONLIT;
			opt_flag = 0;
			break;
		case SC_MARIONETTE:
		case SC_MARIONETTE_MASTER:
			sc->opt3 &= ~OPT3_MARIONETTE;
			opt_flag = 0;
			break;
		case SC_ASSUMPTIO:
			sc->opt3 &= ~OPT3_ASSUMPTIO;
			opt_flag = 0;
			break;
		case SC_WARM: //SG skills [Komurka]
			sc->opt3 &= ~OPT3_WARM;
			opt_flag = 0;
			break;
		case SC_KAITE:
			sc->opt3 &= ~OPT3_KAITE;
			opt_flag = 0;
			break;
		case SC_NJ_BUNSINJYUTSU:
			sc->opt3 &= ~OPT3_BUNSIN;
			opt_flag = 0;
			break;
		case SC_SOULLINK:
			sc->opt3 &= ~OPT3_SOULLINK;
			opt_flag = 0;
			break;
		case SC_PROPERTYUNDEAD:
			sc->opt3 &= ~OPT3_UNDEAD;
			opt_flag = 0;
			break;
			//	case ???: // from DA_CONTRACT (looks like biolab mobs aura)
			//		sc->opt3 &= ~OPT3_CONTRACT;
			//		opt_flag = 0;
			//		break;
		default:
			opt_flag = 0;
	}

#ifdef ANTI_MAYAP_CHEAT
	if (invisible && !(sc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_INVISIBLE))) {
		clif->fixpos(bl);
	}
#endif

	if (calc_flag&SCB_DYE) { //Restore DYE color
		if (vd && !vd->cloth_color && sce->val4)
			clif->changelook(bl,LOOK_CLOTHES_COLOR,sce->val4);
		calc_flag&=~SCB_DYE;
	}

	//On Aegis, when turning off a status change, first goes the sc packet, then the option packet.
	clif->sc_end(bl,bl->id,AREA,status->IconChangeTable[type]);

	if( opt_flag&8 ) //bugreport:681
		clif->changeoption2(bl);
	else if(opt_flag) {
		clif->changeoption(bl);
		if( sd && opt_flag&0x4 ) {
			clif->changelook(bl,LOOK_BASE,sd->vd.class_);
			clif->get_weapon_view(sd, &sd->vd.weapon, &sd->vd.shield);
			clif->changelook(bl,LOOK_WEAPON,sd->vd.weapon);
			clif->changelook(bl,LOOK_SHIELD,sd->vd.shield);
			clif->changelook(bl,LOOK_CLOTHES_COLOR,cap_value(sd->status.clothes_color,0,battle_config.max_cloth_color));
		}
	}

	if (calc_flag)
		status_calc_bl(bl,calc_flag);

	if(opt_flag&4) //Out of hiding, invoke on place.
		skill->unit_move(bl,timer->gettick(),1);

	if(opt_flag&2 && sd && map->getcell(bl->m,bl->x,bl->y,CELL_CHKNPC))
		npc->touch_areanpc(sd,bl->m,bl->x,bl->y); //Trigger on-touch event.

	ers_free(status->data_ers, sce);
	return 1;
}

int kaahi_heal_timer(int tid, int64 tick, int id, intptr_t data) {
	struct block_list *bl;
	struct status_change *sc;
	struct status_change_entry *sce;
	struct status_data *st;
	int hp;

	if(!( (bl=map->id2bl(id))
	   && (sc=status->get_sc(bl))
	   && (sce=sc->data[SC_KAAHI])
	))
		return 0;

	if(sce->val4 != tid) {
		ShowError("kaahi_heal_timer: Timer mismatch: %d != %d\n", tid, sce->val4);
		sce->val4 = INVALID_TIMER;
		return 0;
	}

	st=status->get_status_data(bl);
	if(!status->charge(bl, 0, sce->val3)) {
		sce->val4 = INVALID_TIMER;
		return 0;
	}

	hp = st->max_hp - st->hp;
	if (hp > sce->val2)
		hp = sce->val2;
	if (hp)
		status->heal(bl, hp, 0, 2);
	sce->val4 = INVALID_TIMER;
	return 1;
}

/*==========================================
* For recusive status, like for each 5s we drop sp etc.
* Reseting the end timer.
*------------------------------------------*/
int status_change_timer(int tid, int64 tick, int id, intptr_t data) {
	enum sc_type type = (sc_type)data;
	struct block_list *bl;
	struct map_session_data *sd;
	struct status_data *st;
	struct status_change *sc;
	struct status_change_entry *sce;

	bl = map->id2bl(id);
	if(!bl) {
		ShowDebug("status_change_timer: Null pointer id: %d data: %d\n", id, data);
		return 0;
	}
	sc = status->get_sc(bl);
	st = status->get_status_data(bl);

	if(!(sc && (sce = sc->data[type])))
	{
		ShowDebug("status_change_timer: Null pointer id: %d data: %d bl-type: %d\n", id, data, bl->type);
		return 0;
	}

	if( sce->timer != tid )
	{
		ShowError("status_change_timer: Mismatch for type %d: %d != %d (bl id %d)\n",type,tid,sce->timer, bl->id);
		return 0;
	}

	sd = BL_CAST(BL_PC, bl);

	// set the next timer of the sce (don't assume the status still exists)
#define sc_timer_next(t,f,i,d) do { \
	if( (sce=sc->data[type]) ) \
		sce->timer = timer->add((t),(f),(i),(d)); \
	else \
		ShowError("status_change_timer: Unexpected NULL status change id: %d data: %d\n", id, data); \
} while(0)

	switch(type) {
		case SC_MAXIMIZEPOWER:
		case SC_CLOAKING:
			if(!status->charge(bl, 0, 1))
				break; //Not enough SP to continue.
			sc_timer_next(sce->val2+tick, status->change_timer, bl->id, data);
			return 0;

		case SC_CHASEWALK:
			if(!status->charge(bl, 0, sce->val4))
				break; //Not enough SP to continue.

			if (!sc->data[SC_CHASEWALK2]) {
				sc_start(bl,bl, SC_CHASEWALK2,100,1<<(sce->val1-1),
						 (sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_ROGUE?10:1) //SL bonus -> x10 duration
						 * skill->get_time2(status->sc2skill(type),sce->val1));
			}
			sc_timer_next(sce->val2+tick, status->change_timer, bl->id, data);
			return 0;
			break;

		case SC_SKA:
			if(--(sce->val2)>0) {
				sce->val3 = rnd()%100; //Random defense.
				sc_timer_next(1000+tick, status->change_timer,bl->id, data);
				return 0;
			}
			break;

		case SC_HIDING:
			if(--(sce->val2)>0) {
				if(sce->val2 % sce->val4 == 0 && !status->charge(bl, 0, 1))
					break; //Fail if it's time to substract SP and there isn't.

				sc_timer_next(1000+tick, status->change_timer,bl->id, data);
				return 0;
			}
			break;

		case SC_SIGHT:
		case SC_RUWACH:
		case SC_WZ_SIGHTBLASTER:
			if(type == SC_WZ_SIGHTBLASTER)
				map->foreachinrange(status->change_timer_sub, bl, sce->val3, BL_CHAR|BL_SKILL, bl, sce, type, tick);
			else
				map->foreachinrange(status->change_timer_sub, bl, sce->val3, BL_CHAR, bl, sce, type, tick);

			if( --(sce->val2)>0 ){
				sce->val4 += 250; // use for Shadow Form 2 seconds checking.
				sc_timer_next(250+tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_PROVOKE:
			if(sce->val2) { //Auto-provoke (it is ended in status->heal)
				sc_timer_next(1000*60+tick, status->change_timer, bl->id, data );
				return 0;
			}
			break;

		case SC_STONE:
			if(sc->opt1 == OPT1_STONEWAIT && sce->val3) {
				sce->val4 = 0;
				unit->stop_walking(bl,1);
				unit->stop_attack(bl);
				sc->opt1 = OPT1_STONE;
				clif->changeoption(bl);
				sc_timer_next(1000+tick, status->change_timer, bl->id, data );
				status_calc_bl(bl, status->ChangeFlagTable[type]);
				return 0;
			}
			if(--(sce->val3) > 0) {
				if(++(sce->val4)%5 == 0 && st->hp > st->max_hp/4)
					status_percent_damage(NULL, bl, 1, 0, false);
				sc_timer_next(1000+tick, status->change_timer, bl->id, data );
				return 0;
			}
			break;

		case SC_POISON:
			if(st->hp <= max(st->max_hp>>2, sce->val4)) //Stop damaging after 25% HP left.
				break;
		case SC_DPOISON:
			if (--(sce->val3) > 0) {
				if (!sc->data[SC_SLOWPOISON]) {
					if( sce->val2 && bl->type == BL_MOB ) {
						struct block_list* src = map->id2bl(sce->val2);
						if( src )
							mob->log_damage((TBL_MOB*)bl,src,sce->val4);
					}
					map->freeblock_lock();
					status_zap(bl, sce->val4, 0);
					if (sc->data[type]) { // Check if the status still last ( can be dead since then ).
						sc_timer_next(1000 + tick, status->change_timer, bl->id, data );
					}
					map->freeblock_unlock();
				}
				return 0;
			}
			break;

		case SC_TENSIONRELAX:
			if(st->max_hp > st->hp && --(sce->val3) > 0) {
				sc_timer_next(sce->val4+tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_KNOWLEDGE:
			if (!sd) break;
			if(bl->m == sd->feel_map[0].m ||
				bl->m == sd->feel_map[1].m ||
				bl->m == sd->feel_map[2].m)
			{	//Timeout will be handled by pc->setpos
				sce->timer = INVALID_TIMER;
				return 0;
			}
			break;

		case SC_BLOODING:
			if (--(sce->val4) >= 0) {
				int hp =  rnd()%600 + 200;
				struct block_list* src = map->id2bl(sce->val2);
				if( src && bl && bl->type == BL_MOB ) {
					mob->log_damage((TBL_MOB*)bl,src,sd||hp<st->hp?hp:st->hp-1);
				}
				map->freeblock_lock();
				status_fix_damage(src, bl, sd||hp<st->hp?hp:st->hp-1, 1);
				if( sc->data[type] ) {
					if( st->hp == 1 ) {
						map->freeblock_unlock();
						break;
					}
					sc_timer_next(10000 + tick, status->change_timer, bl->id, data);
				}
				map->freeblock_unlock();
				return 0;
			}
			break;

		case SC_S_LIFEPOTION:
		case SC_L_LIFEPOTION:
			if( sd && --(sce->val4) >= 0 ) {
				// val1 < 0 = per max% | val1 > 0 = exact amount
				int hp = 0;
				if( st->hp < st->max_hp )
					hp = (sce->val1 < 0) ? (int)(sd->status.max_hp * -1 * sce->val1 / 100.) : sce->val1 ;
				status->heal(bl, hp, 0, 2);
				sc_timer_next((sce->val2 * 1000) + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_CASH_BOSS_ALARM:
			if( sd && --(sce->val4) >= 0 ) {
				struct mob_data *boss_md = map->id2boss(sce->val1);
				if( boss_md && sd->bl.m == boss_md->bl.m ) {
					clif->bossmapinfo(sd->fd, boss_md, 1); // Update X - Y on minimap
					if (boss_md->bl.prev != NULL) {
						sc_timer_next(5000 + tick, status->change_timer, bl->id, data);
						return 0;
					}
				}
			}
			break;

		case SC_DANCING: //SP consumption by time of dancing skills
			{
				int s = 0;
				int sp = 1;
				if (--sce->val3 <= 0)
					break;
				switch(sce->val1&0xFFFF){
				case BD_RICHMANKIM:
				case BD_DRUMBATTLEFIELD:
				case BD_RINGNIBELUNGEN:
				case BD_SIEGFRIED:
				case BA_DISSONANCE:
				case BA_ASSASSINCROSS:
				case DC_UGLYDANCE:
					s=3;
					break;
				case BD_LULLABY:
				case BD_ETERNALCHAOS:
				case BD_ROKISWEIL:
				case DC_FORTUNEKISS:
					s=4;
					break;
				case CG_HERMODE:
				case BD_INTOABYSS:
				case BA_WHISTLE:
				case DC_HUMMING:
				case BA_POEMBRAGI:
				case DC_SERVICEFORYOU:
					s=5;
					break;
				case BA_APPLEIDUN:
	#ifdef RENEWAL
					s=5;
	#else
					s=6;
	#endif
					break;
				case CG_MOONLIT:
					//Moonlit's cost is 4sp*skill_lv [Skotlex]
					sp= 4*(sce->val1>>16);
					//Upkeep is also every 10 secs.
				case DC_DONTFORGETME:
					s=10;
					break;
				}
				if( s != 0 && sce->val3 % s == 0 ) {
					if (sc->data[SC_LONGING])
						sp*= 3;
					if (!status->charge(bl, 0, sp))
						break;
				}
				sc_timer_next(1000+tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;
		case SC_BERSERK:
			// 5% every 10 seconds [DracoRPG]
			if( --( sce->val3 ) > 0 && status->charge(bl, sce->val2, 0) && st->hp > 100 ) {
				sc_timer_next(sce->val4+tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_NOCHAT:
			if(sd) {
				sd->status.manner++;
				clif->changestatus(sd,SP_MANNER,sd->status.manner);
				clif->updatestatus(sd,SP_MANNER);
				if (sd->status.manner < 0) {
					//Every 60 seconds your manner goes up by 1 until it gets back to 0.
					sc_timer_next(60000+tick, status->change_timer, bl->id, data);
					return 0;
				}
			}
			break;

		case SC_SPLASHER:
			// custom Venom Splasher countdown timer
			//if (sce->val4 % 1000 == 0) {
			//	char counter[10];
			//	snprintf (counter, 10, "%d", sce->val4/1000);
			//	clif->message(bl, counter);
			//}
			if((sce->val4 -= 500) > 0) {
				sc_timer_next(500 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_MARIONETTE_MASTER:
		case SC_MARIONETTE:
			{
				struct block_list *pbl = map->id2bl(sce->val1);
				if( pbl && check_distance_bl(bl, pbl, 7) ) {
					sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
					return 0;
				}
			}
			break;

		case SC_GOSPEL:
			if(sce->val4 == BCT_SELF && --(sce->val2) > 0) {
				int hp, sp;
				hp = (sce->val1 > 5) ? 45 : 30;
				sp = (sce->val1 > 5) ? 35 : 20;
				if(!status->charge(bl, hp, sp))
					break;
				sc_timer_next(10000+tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_JAILED:
			if(sce->val1 == INT_MAX || --(sce->val1) > 0) {
				sc_timer_next(60000+tick, status->change_timer, bl->id,data);
				return 0;
			}
			break;

		case SC_BLIND:
			if(sc->data[SC_FOGWALL]) {
				//Blind lasts forever while you are standing on the fog.
				sc_timer_next(5000+tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;
		case SC_ABUNDANCE:
			if(--(sce->val4) > 0) {
				status->heal(bl,0,60,0);
				sc_timer_next(10000+tick, status->change_timer, bl->id, data);
			}
			break;

		case SC_PYREXIA:
			if( --(sce->val4) > 0 ) {
				map->freeblock_lock();
				clif->damage(bl,bl,status_get_amotion(bl),status_get_dmotion(bl)+500,100,0,0,0);
				status_fix_damage(NULL,bl,100,0);
				if( sc->data[type] ) {
					sc_timer_next(3000+tick,status->change_timer,bl->id,data);
				}
				map->freeblock_unlock();
				return 0;
			}
			break;

		case SC_LEECHESEND:
			if( --(sce->val4) > 0 ) {
				int damage = st->max_hp/100; // {Target VIT x (New Poison Research Skill Level - 3)} + (Target HP/100)
				damage += st->vit * (sce->val1 - 3);
				unit->skillcastcancel(bl,2);
				map->freeblock_lock();
				status->damage(bl, bl, damage, 0, clif->damage(bl,bl,status_get_amotion(bl),status_get_dmotion(bl)+500,damage,1,0,0), 1);
				if( sc->data[type] ) {
					sc_timer_next(1000 + tick, status->change_timer, bl->id, data );
				}
				map->freeblock_unlock();
				return 0;
			}
			break;

		case SC_MAGICMUSHROOM:
			if( --(sce->val4) > 0 ) {
				bool flag = 0;
				int damage = st->max_hp * 3 / 100;
				if( st->hp <= damage )
					damage = st->hp - 1; // Cannot Kill

				if( damage > 0 ) { // 3% Damage each 4 seconds
					map->freeblock_lock();
					status_zap(bl,damage,0);
					flag = !sc->data[type]; // Killed? Should not
					map->freeblock_unlock();
				}

				if( !flag ) { // Random Skill Cast
					if (sd && !pc_issit(sd)) { //can't cast if sit
						int mushroom_skill_id = 0, i;
						unit->stop_attack(bl);
						unit->skillcastcancel(bl,0);
						do {
							i = rnd() % MAX_SKILL_MAGICMUSHROOM_DB;
							mushroom_skill_id = skill->magicmushroom_db[i].skill_id;
						}
						while( mushroom_skill_id == 0 );

						switch( skill->get_casttype(mushroom_skill_id) ) { // Magic Mushroom skills are buffs or area damage
							case CAST_GROUND:
								skill->castend_pos2(bl,bl->x,bl->y,mushroom_skill_id,1,tick,0);
								break;
							case CAST_NODAMAGE:
								skill->castend_nodamage_id(bl,bl,mushroom_skill_id,1,tick,0);
								break;
							case CAST_DAMAGE:
								skill->castend_damage_id(bl,bl,mushroom_skill_id,1,tick,0);
								break;
						}
					}

					clif->emotion(bl,E_HEH);
					sc_timer_next(4000+tick,status->change_timer,bl->id,data);
				}
				return 0;
			}
			break;

		case SC_TOXIN:
			if( --(sce->val4) > 0 ) {
				//Damage is every 10 seconds including 3%sp drain.
				map->freeblock_lock();
				clif->damage(bl,bl,status_get_amotion(bl),1,1,0,0,0);
				status->damage(NULL, bl, 1, st->max_sp * 3 / 100, 0, 0); //cancel dmg only if cancelable
				if( sc->data[type] ) {
					sc_timer_next(10000 + tick, status->change_timer, bl->id, data );
				}
				map->freeblock_unlock();
				return 0;
			}
			break;

		case SC_OBLIVIONCURSE:
			if( --(sce->val4) > 0 ) {
				clif->emotion(bl,E_WHAT);
				sc_timer_next(3000 + tick, status->change_timer, bl->id, data );
				return 0;
			}
			break;

		case SC_WEAPONBLOCKING:
			if( --(sce->val4) > 0 ) {
				if( !status->charge(bl,0,3) )
					break;
				sc_timer_next(5000+tick,status->change_timer,bl->id,data);
				return 0;
			}
			break;

		case SC_CLOAKINGEXCEED:
			if(!status->charge(bl,0,10-sce->val1))
				break;
			sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
			return 0;

		case SC_RENOVATIO:
			if( --(sce->val4) > 0 ) {
				int heal = st->max_hp * 3 / 100;
				if( sc && sc->data[SC_AKAITSUKI] && heal )
					heal = ~heal + 1;
				status->heal(bl, heal, 0, 2);
				sc_timer_next(5000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_BURNING:
			if( --(sce->val4) > 0 ) {
				struct block_list *src = map->id2bl(sce->val3);
				int damage = 1000 + 3 * status_get_max_hp(bl) / 100; // Deals fixed (1000 + 3%*MaxHP)

				map->freeblock_lock();
				clif->damage(bl,bl,0,0,damage,1,9,0); //damage is like endure effect with no walk delay
				status->damage(src, bl, damage, 0, 0, 1);

				if( sc->data[type]){ // Target still lives. [LimitLine]
					sc_timer_next(3000 + tick, status->change_timer, bl->id, data);
				}
				map->freeblock_unlock();
				return 0;
			}
			break;

		case SC_FEAR:
			if( --(sce->val4) > 0 ) {
				if( sce->val2 > 0 )
					sce->val2--;
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_SUMMON1:
		case SC_SUMMON2:
		case SC_SUMMON3:
		case SC_SUMMON4:
		case SC_SUMMON5:
			if( --(sce->val4) > 0 ) {
				if( !status->charge(bl, 0, 1) )
					break;
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_READING_SB:
			if( !status->charge(bl, 0, sce->val2) ) {
				int i;
				for(i = SC_SPELLBOOK1; i <= SC_SPELLBOOK7; i++) // Also remove stored spell as well.
					status_change_end(bl, (sc_type)i, INVALID_TIMER);
				break;
			}
			sc_timer_next(10000 + tick, status->change_timer, bl->id, data);
			return 0;

		case SC_ELECTRICSHOCKER:
			if( --(sce->val4) > 0 ) {
				status->charge(bl, 0, st->max_sp / 100 * sce->val1 );
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_CAMOUFLAGE:
			if(--(sce->val4) > 0) {
				status->charge(bl,0,7 - sce->val1);
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC__REPRODUCE:
			if( --(sce->val4) >= 0 ) {
				if( !status_charge(bl, 0, 9 - (1 + sce->val1) / 2) )
					break;
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC__SHADOWFORM:
			if( --(sce->val4) > 0 ) {
				if( !status->charge(bl, 0, sce->val1 - (sce->val1 - 1)) )
					break;
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC__INVISIBILITY:
			if( --(sce->val4) >= 0 ) {
				if( !status->charge(bl, 0, status_get_max_sp(bl) * (12 - sce->val1*2) / 100) )
					break;
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_STRIKING:
			if( --(sce->val4) > 0 ) {
				if( !status->charge(bl,0, sce->val1 ) )
					break;
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;
		case SC_BLOOD_SUCKER:
			if( --(sce->val4) > 0 ) {
				struct block_list *src = map->id2bl(sce->val2);
				int damage;
				if( !src || (src && (status->isdead(src) || src->m != bl->m || distance_bl(src, bl) >= 12)) )
					break;
				map->freeblock_lock();
				damage =  sce->val3;
				status->damage(src, bl, damage, 0, clif->damage(bl,bl,st->amotion,st->dmotion+200,damage,1,0,0), 1);
				unit->skillcastcancel(bl,1);
				if ( sc->data[type] ) {
					sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				}
				map->freeblock_unlock();
				status->heal(src, damage*(5 + 5 * sce->val1)/100, 0, 0); // 5 + 5% per level
				return 0;
			}
			break;

		case SC_SIREN:
			if( --(sce->val4) > 0 ) {
				clif->emotion(bl,E_LV);
				sc_timer_next(2000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_DEEP_SLEEP:
			if( --(sce->val4) >= 0 )
			{// Recovers 3% of the player's MaxHP/MaxSP every 2 seconds.
				status->heal(bl, st->max_hp * 3 / 100, st->max_sp * 3 / 100, 2);
				sc_timer_next(2000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_SIRCLEOFNATURE:
			if( --(sce->val4) >= 0 ) {
				if( !status_charge(bl,0,sce->val3) )
					break;
				status->heal(bl, sce->val2, 0, 1);
				sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_SONG_OF_MANA:
			if( --(sce->val4) >= 0 ) {
				status->heal(bl,0,sce->val3,3);
				sc_timer_next(5000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;


		case SC_SATURDAY_NIGHT_FEVER:
			if( --(sce->val3) >= 0 ) {
				if( !status_charge(bl, st->max_hp * 1 / 100, st->max_sp * 1 / 100) )
				break;
				sc_timer_next(3000+tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_MELODYOFSINK:
			if( --(sce->val4) >= 0 ) {
				status_charge(bl, 0, st->max_sp * ( 2 * sce->val1 + 2 * sce->val2 ) / 100);
				sc_timer_next(1000+tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_COLD:
			if( --(sce->val4) > 0 ) {
				// Drains 2% of HP and 1% of SP every seconds.
				if( bl->type != BL_MOB) // doesn't work on mobs
					status->charge(bl, st->max_hp * 2 / 100, st->max_sp / 100);
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_FORCEOFVANGUARD:
			if( !status->charge(bl, 0, (24 - 4 * sce->val1)) )
				break;
			sc_timer_next(10000 + tick, status->change_timer, bl->id, data);
			return 0;

		case SC_BANDING:
			if( status->charge(bl, 0, 7 - sce->val1) ) {
				if( sd ) pc->banding(sd, sce->val1);
				sc_timer_next(5000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_LG_REFLECTDAMAGE:
			if( --(sce->val4) >= 0 ) {
				if( !status->charge(bl,0,10) )
					break;
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_OVERHEAT_LIMITPOINT:
			if( --(sce->val1) > 0 ) { // Cooling
				sc_timer_next(30000 + tick, status->change_timer, bl->id, data);
			}
			break;

		case SC_OVERHEAT:
			{
				int damage = st->max_hp / 100; // Suggestion 1% each second
				if( damage >= st->hp ) damage = st->hp - 1; // Do not kill, just keep you with 1 hp minimum
				map->freeblock_lock();
				status_fix_damage(NULL,bl,damage,clif->damage(bl,bl,0,0,damage,0,0,0));
				if( sc->data[type] ) {
					sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				}
				map->freeblock_unlock();
			}
			break;

		case SC_MAGNETICFIELD:
			{
				if( --(sce->val3) <= 0 )
					break; // Time out
				if( sce->val2 == bl->id ) {
					if( !status->charge(bl,0,50) )
						break; // No more SP status should end, and in the next second will end for the other affected players
				} else {
					struct block_list *src = map->id2bl(sce->val2);
					struct status_change *ssc;
					if( !src || (ssc = status->get_sc(src)) == NULL || !ssc->data[SC_MAGNETICFIELD] )
						break; // Source no more under Magnetic Field
				}
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
			}
			break;

		case SC_STEALTHFIELD_MASTER:
			if(--(sce->val4) >= 0) {
				// 1% SP Upkeep Cost
				int sp = st->max_sp / 100;
			
				if( st->sp <= sp )
					status_change_end(bl,SC_STEALTHFIELD_MASTER,INVALID_TIMER);

				if( !status_charge(bl,0,sp) )
					break;

				if( !sc->data[SC_STEALTHFIELD_MASTER] )
					break;

				sc_timer_next((2000 + 1000 * sce->val1)+tick,status->change_timer,bl->id, data);
				return 0;
			}
			break;

		case SC_INSPIRATION:
			if(--(sce->val4) >= 0) {
				int hp = st->max_hp * (35 - 5 * sce->val1) / 1000;
				int sp = st->max_sp * (45 - 5 * sce->val1) / 1000;

				if( !status->charge(bl,hp,sp) ) break;

				sc_timer_next(5000+tick,status->change_timer,bl->id, data);
				return 0;
			}
			break;

		case SC_RAISINGDRAGON:
			// 1% every 5 seconds [Jobbie]
			if( --(sce->val3)>0 && status->charge(bl, sce->val2, 0) ) {
				if( !sc->data[type] ) return 0;
				sc_timer_next(5000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;

		case SC_CIRCLE_OF_FIRE:
		case SC_FIRE_CLOAK:
		case SC_WATER_DROP:
		case SC_WATER_SCREEN:
		case SC_WIND_CURTAIN:
		case SC_WIND_STEP:
		case SC_STONE_SHIELD:
		case SC_SOLID_SKIN:
			if( !status->charge(bl,0,sce->val2) ){
				struct block_list *s_bl = battle->get_master(bl);
				if( s_bl )
					status_change_end(s_bl,type+1,INVALID_TIMER);
				status_change_end(bl,type,INVALID_TIMER);
				break;
			}
			sc_timer_next(2000 + tick, status->change_timer, bl->id, data);
			return 0;

		case SC_STOMACHACHE:
			if( --(sce->val4) > 0 ) {
				status->charge(bl,0,sce->val2); // Reduce 8 every 10 seconds.
				if( sd && !pc_issit(sd) ) {     // Force to sit every 10 seconds.
					pc_stop_walking(sd,1|4);
					pc_stop_attack(sd);
					pc_setsit(sd);
					clif->sitting(bl);
				}
				sc_timer_next(10000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;
		case SC_LEADERSHIP:
		case SC_GLORYWOUNDS:
		case SC_SOULCOLD:
		case SC_HAWKEYES:
			/* they only end by status_change_end */
			sc_timer_next(600000 + tick, status->change_timer, bl->id, data);
			return 0;
		case SC_MEIKYOUSISUI:
			if( --(sce->val4) > 0 ) {
				status->heal(bl, st->max_hp * (sce->val1+1) / 100, st->max_sp * sce->val1 / 100, 0);
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;
		case SC_IZAYOI:
		case SC_KAGEMUSYA:
			if( --(sce->val2) > 0 ) {
				if(!status->charge(bl, 0, 1)) break;
				sc_timer_next(1000+tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;
		case SC_ANGRIFFS_MODUS:
			if(--(sce->val4) > 0) { //drain hp/sp
				if( !status->charge(bl,100,20) ) break;
				sc_timer_next(1000+tick,status->change_timer,bl->id, data);
				return 0;
			}
			break;
		case SC_FULL_THROTTLE:
			if( --(sce->val4) > 0 ) {
				status_percent_damage(bl, bl, sce->val2, 0, false);
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;
		case SC_KINGS_GRACE:
			if( --(sce->val4) > 0 ) {
				status_percent_heal(bl, sce->val2, 0);
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;
		case SC_FRIGG_SONG:
			if( --(sce->val4) > 0 ) {
				status->heal(bl, sce->val3, 0, 0);
				sc_timer_next(1000 + tick, status->change_timer, bl->id, data);
				return 0;
			}
			break;
	}

	// default for all non-handled control paths is to end the status
	return status_change_end( bl,type,tid );
#undef sc_timer_next
}

/*==========================================
* Foreach iteration of repetitive status
*------------------------------------------*/
int status_change_timer_sub(struct block_list* bl, va_list ap) {
	struct status_change* tsc;

	struct block_list* src = va_arg(ap,struct block_list*);
	struct status_change_entry* sce = va_arg(ap,struct status_change_entry*);
	enum sc_type type = (sc_type)va_arg(ap,int); //gcc: enum args get promoted to int
	int64 tick = va_arg(ap, int64);

	if (status->isdead(bl))
		return 0;

	tsc = status->get_sc(bl);

	switch( type ) {
		case SC_SIGHT: /* Reveal hidden ennemy on 3*3 range */
			if( tsc && tsc->data[SC__SHADOWFORM] && (sce && sce->val4 >0 && sce->val4%2000 == 0) && // for every 2 seconds do the checking
				rnd()%100 < 100-tsc->data[SC__SHADOWFORM]->val1*10 ) // [100 - (Skill Level x 10)] %
				status_change_end(bl, SC__SHADOWFORM, INVALID_TIMER);
		case SC_CONCENTRATION:
			status_change_end(bl, SC_HIDING, INVALID_TIMER);
			status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
			status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
			status_change_end(bl, SC_CAMOUFLAGE, INVALID_TIMER);
			break;
		case SC_RUWACH: /* Reveal hidden target and deal little dammages if ennemy */
			if (tsc && (tsc->data[SC_HIDING] || tsc->data[SC_CLOAKING] ||
				tsc->data[SC_CAMOUFLAGE] || tsc->data[SC_CLOAKINGEXCEED])) {
					status_change_end(bl, SC_HIDING, INVALID_TIMER);
					status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
					status_change_end(bl, SC_CAMOUFLAGE, INVALID_TIMER);
					status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
					if(battle->check_target( src, bl, BCT_ENEMY ) > 0)
						skill->attack(BF_MAGIC,src,src,bl,AL_RUWACH,1,tick,0);
			}
			if( tsc && tsc->data[SC__SHADOWFORM] && (sce && sce->val4 >0 && sce->val4%2000 == 0) && // for every 2 seconds do the checking
				rnd()%100 < 100-tsc->data[SC__SHADOWFORM]->val1*10 ) // [100 - (Skill Level x 10)] %
				status_change_end(bl, SC__SHADOWFORM, INVALID_TIMER);
			break;
		case SC_WZ_SIGHTBLASTER:
			if (battle->check_target( src, bl, BCT_ENEMY ) > 0
			 && status->check_skilluse(src, bl, WZ_SIGHTBLASTER, 2)
			) {
				if (sce && !(bl->type&BL_SKILL) //The hit is not counted if it's against a trap
						&& skill->attack(BF_MAGIC,src,src,bl,WZ_SIGHTBLASTER,1,tick,0)
				){
						sce->val2 = 0; //This signals it to end.
				}
			}
			break;
		case SC_RG_CCONFINE_M:
			//Lock char has released the hold on everyone...
			if (tsc && tsc->data[SC_RG_CCONFINE_S] && tsc->data[SC_RG_CCONFINE_S]->val2 == src->id) {
				tsc->data[SC_RG_CCONFINE_S]->val2 = 0;
				status_change_end(bl, SC_RG_CCONFINE_S, INVALID_TIMER);
			}
			break;
		case SC_CURSEDCIRCLE_TARGET:
			if( tsc && tsc->data[SC_CURSEDCIRCLE_TARGET] && tsc->data[SC_CURSEDCIRCLE_TARGET]->val2 == src->id ) {
				clif->bladestop(bl, tsc->data[SC_CURSEDCIRCLE_TARGET]->val2, 0);
				status_change_end(bl, type, INVALID_TIMER);
			}
			break;
	}
	return 0;
}

int status_get_total_def(struct block_list *src) { return status->get_status_data(src)->def2 + (short)status->get_def(src); }
int status_get_total_mdef(struct block_list *src) { return status->get_status_data(src)->mdef2 + (short)status_get_mdef(src); }
int status_get_weapon_atk(struct block_list *bl, struct weapon_atk *watk, int flag) {
#ifdef RENEWAL
	int min = 0, max = 0, dstr;
	float strdex_bonus, variance;
	struct status_change *sc = status->get_sc(bl);

	if ( bl->type == BL_PC && watk->atk ){
		if ( flag&2 )
			dstr = status_get_dex(bl);
		else
			dstr = status_get_str(bl);

		variance = 5.0f * watk->atk *  watk->wlv / 100.0f;
		strdex_bonus = watk->atk * dstr / 200.0f;

		min = (int)(watk->atk - variance + strdex_bonus) + watk->atk2;
		max = (int)(watk->atk + variance + strdex_bonus) + watk->atk2;
	}else if( watk->atk ){
		min = watk->atk * 80 / 100;
		max = watk->atk * 120 / 100;
	}

	if( !(flag&1) ){
		if( max > min )
			max = min + rnd()%(max - min);
		else
			max = min;
	}

	if( bl->type == BL_PC && ((TBL_PC*)bl)->right_weapon.overrefine > 0 && !(flag&2) )
		max += rnd()%((TBL_PC*)bl)->right_weapon.overrefine + 1;

	max = status->calc_watk(bl, sc, max, false);

	return max;
#else
	return 0;
#endif
}

#define GETRANDMATK(st) do {\
	if( (st)->matk_max > (st)->matk_min ) \
		return (st)->matk_min + rnd()%((st)->matk_max - (st)->matk_min); \
	else \
		return (st)->matk_min; \
	} while(0)

/*==========================================
*  flag [malufett]
*	0 - update matk values
*	1 - get matk w/o SC bonuses
*	2 - get modified matk
*	3 - get matk w/o eatk & SC bonuses
*------------------------------------------*/
int status_get_matk(struct block_list *bl, int flag) {
	struct status_data *st;
	struct status_change *sc;
	struct map_session_data *sd;

	if( bl == NULL )
		return 1;

	st = status->get_status_data(bl);
	sc = status->get_sc(bl);
	sd = BL_CAST(BL_PC, bl);

	if( flag == 2 ) // just get matk
		GETRANDMATK(st);

#ifdef RENEWAL
	/**
	 * RE MATK Formula (from irowiki:http://irowiki.org/wiki/MATK)
	 * MATK = (sMATK + wMATK + eMATK) * Multiplicative Modifiers
	 **/
	st->matk_min = status->base_matk(st, status->get_lv(bl));

	//  Any +MATK you get from skills and cards, including cards in weapon, is added here.
	if( sd && sd->bonus.ematk > 0 && flag != 3 )
		st->matk_min += sd->bonus.ematk;
	if( flag != 3 )
		st->matk_min = status->calc_ematk(bl, sc, st->matk_min);

	st->matk_max = st->matk_min;

	//This is the only portion in MATK that varies depending on the weapon level and refinement rate.
	if( bl->type&BL_PC && (st->rhw.matk + st->lhw.matk) > 0 ) {
		int wMatk = st->rhw.matk + st->lhw.matk; // Left and right matk stacks
		int variance = wMatk * st->rhw.wlv / 10; // Only use right hand weapon level
		st->matk_min += wMatk - variance;
		st->matk_max += wMatk + variance;
	} else if( bl->type&BL_MOB ) {
		st->matk_min = st->matk_max = status_get_int(bl) + status->get_lv(bl);
		st->matk_min += 70 * ((TBL_MOB*)bl)->status.rhw.atk2 / 100;
		st->matk_max += 130 * ((TBL_MOB*)bl)->status.rhw.atk2 / 100;
	}
#else // not RENEWAL
	st->matk_min = status_base_matk_min(st) + (sd?sd->bonus.ematk:0);
	st->matk_max = status_base_matk_max(st) + (sd?sd->bonus.ematk:0);
#endif
	if (sd && sd->matk_rate != 100) {
		st->matk_max = st->matk_max * sd->matk_rate/100;
		st->matk_min = st->matk_min * sd->matk_rate/100;
	}

	if ((bl->type&BL_HOM && battle_config.hom_setting&0x20)  //Hom Min Matk is always the same as Max Matk
		|| (sc && sc->data[SC_RECOGNIZEDSPELL]))
		st->matk_min = st->matk_max;

#ifdef RENEWAL
	if( sd && sd->right_weapon.overrefine > 0){
		st->matk_min++;
		st->matk_max += sd->right_weapon.overrefine - 1;
	}
#endif

	if( flag ) // get unmodified from sc matk
		GETRANDMATK(st);

	st->matk_min = status->calc_matk(bl, sc, st->matk_min, true);
	st->matk_max = status->calc_matk(bl, sc, st->matk_max, true);

	return 0;
}

/*==========================================
* Clears buffs/debuffs of a character.
* type&1 -> buffs, type&2 -> debuffs
* type&4 -> especific debuffs(implemented with refresh)
*------------------------------------------*/
int status_change_clear_buffs (struct block_list* bl, int type) {
	int i;
	struct status_change *sc= status->get_sc(bl);

	if (!sc || !sc->count)
		return 0;

	if (type&6) //Debuffs
		for (i = SC_COMMON_MIN; i <= SC_COMMON_MAX; i++)
			status_change_end(bl, (sc_type)i, INVALID_TIMER);

	for( i = SC_COMMON_MAX+1; i < SC_MAX; i++ ) {
		if( !sc->data[i] || !status->get_sc_type(i) )
			continue;

		if( type&3 && !(status->get_sc_type(i)&SC_BUFF) && !(status->get_sc_type(i)&SC_DEBUFF) )
			continue;

		if( type < 3 ) {
			if( type&1 && !(status->get_sc_type(i)&SC_BUFF) )
				continue;
			if( type&2 && !(status->get_sc_type(i)&SC_DEBUFF) )
				continue;
		}
		switch (i) {
		case SC_DEEP_SLEEP:
		case SC_FROSTMISTY:
		case SC_COLD:
		case SC_TOXIN:
		case SC_PARALYSE:
		case SC_VENOMBLEED:
		case SC_MAGICMUSHROOM:
		case SC_DEATHHURT:
		case SC_PYREXIA:
		case SC_OBLIVIONCURSE:
		case SC_LEECHESEND:
		case SC_MARSHOFABYSS:
		case SC_MANDRAGORA:
			if(!(type&4))
				continue;
			break;
		case SC_BERSERK:
			if(type&4)
				continue;
			sc->data[i]->val2 = 0;
			break;
		default:
			if(type&4)
				continue;

		}
		status_change_end(bl, (sc_type)i, INVALID_TIMER);
	}
	return 0;
}

int status_change_spread( struct block_list *src, struct block_list *bl ) {
	int i, flag = 0;
	struct status_change *sc = status->get_sc(src);
	int64 tick;
	struct status_change_data data;

	if( !sc || !sc->count )
		return 0;

	tick = timer->gettick();

	for( i = SC_COMMON_MIN; i < SC_MAX; i++ ) {
		if( !sc->data[i] || i == SC_COMMON_MAX )
			continue;

		switch( i ) {
			//Debuffs that can be spreaded.
			// NOTE: We'll add/delte SCs when we are able to confirm it.
		case SC_CURSE:
		case SC_SILENCE:
		case SC_CONFUSION:
		case SC_BLIND:
		case SC_NOCHAT:
		case SC_ILLUSION:
		case SC_CRUCIS:
		case SC_DEC_AGI:
		case SC_SLOWDOWN:
		case SC_MINDBREAKER:
		case SC_DC_WINKCHARM:
		case SC_STOP:
		case SC_ORCISH:
			//case SC_NOEQUIPWEAPON://Omg I got infected and had the urge to strip myself physically.
			//case SC_NOEQUIPSHIELD://No this is stupid and shouldnt be spreadable at all.
			//case SC_NOEQUIPARMOR:// Disabled until I can confirm if it does or not. [Rytech]
			//case SC_NOEQUIPHELM:
			//case SC__STRIPACCESSARY:
		case SC_WUGBITE:
		case SC_FROSTMISTY:
		case SC_VENOMBLEED:
		case SC_DEATHHURT:
		case SC_PARALYSE:
			if( sc->data[i]->timer != INVALID_TIMER ) {
				const struct TimerData *td = timer->get(sc->data[i]->timer);
				if (td == NULL || td->func != status->change_timer || DIFF_TICK(td->tick,tick) < 0)
					continue;
				data.tick = DIFF_TICK32(td->tick,tick);
			} else
				data.tick = INVALID_TIMER;
			break;
			// Special cases
		case SC_POISON:
		case SC_DPOISON:
			data.tick = sc->data[i]->val3 * 1000;
			break;
		case SC_FEAR:
		case SC_LEECHESEND:
			data.tick = sc->data[i]->val4 * 1000;
			break;
		case SC_BURNING:
			data.tick = sc->data[i]->val4 * 2000;
			break;
		case SC_PYREXIA:
		case SC_OBLIVIONCURSE:
			data.tick = sc->data[i]->val4 * 3000;
			break;
		case SC_MAGICMUSHROOM:
			data.tick = sc->data[i]->val4 * 4000;
			break;
		case SC_TOXIN:
		case SC_BLOODING:
			data.tick = sc->data[i]->val4 * 10000;
			break;
		default:
			continue;
		}
		if( i ) {
			data.val1 = sc->data[i]->val1;
			data.val2 = sc->data[i]->val2;
			data.val3 = sc->data[i]->val3;
			data.val4 = sc->data[i]->val4;
			status->change_start(src,bl,(sc_type)i,10000,data.val1,data.val2,data.val3,data.val4,data.tick,1|2|8);
			flag = 1;
		}
	}

	return flag;
}

//Natural regen related stuff.
int status_natural_heal(struct block_list* bl, va_list args) {
	struct regen_data *regen;
	struct status_data *st;
	struct status_change *sc;
	struct unit_data *ud;
	struct view_data *vd = NULL;
	struct regen_data_sub *sregen;
	struct map_session_data *sd;
	int val,rate,bonus = 0,flag;

	regen = status->get_regen_data(bl);
	if (!regen) return 0;
	st = status->get_status_data(bl);
	sc = status->get_sc(bl);
	if (sc && !sc->count)
		sc = NULL;
	sd = BL_CAST(BL_PC,bl);

	flag = regen->flag;
	if (flag&RGN_HP && (st->hp >= st->max_hp || regen->state.block&1))
		flag&=~(RGN_HP|RGN_SHP);
	if (flag&RGN_SP && (st->sp >= st->max_sp || regen->state.block&2))
		flag&=~(RGN_SP|RGN_SSP);

	if (flag
	  && (status->isdead(bl)
	    || (sc && (sc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) || sc->data[SC__INVISIBILITY]))
	     )
	)
		flag=0;

	if (sd) {
		if (sd->hp_loss.value || sd->sp_loss.value)
			pc->bleeding(sd, status->natural_heal_diff_tick);
		if (sd->hp_regen.value || sd->sp_regen.value)
			pc->regen(sd, status->natural_heal_diff_tick);
	}

	if (flag&(RGN_SHP|RGN_SSP)
	 && regen->ssregen
	 && (vd = status->get_viewdata(bl))
	 && vd->dead_sit == 2
	) {
		//Apply sitting regen bonus.
		sregen = regen->ssregen;
		if(flag&(RGN_SHP)) {
			//Sitting HP regen
			val = status->natural_heal_diff_tick * sregen->rate.hp;
			if (regen->state.overweight)
				val>>=1; //Half as fast when overweight.
			sregen->tick.hp += val;
			while(sregen->tick.hp >= (unsigned int)battle_config.natural_heal_skill_interval) {
				sregen->tick.hp -= battle_config.natural_heal_skill_interval;
				if(status->heal(bl, sregen->hp, 0, 3) < sregen->hp) {
					//Full
					flag&=~(RGN_HP|RGN_SHP);
					break;
				}
			}
		}
		if(flag&(RGN_SSP)) {
			//Sitting SP regen
			val = status->natural_heal_diff_tick * sregen->rate.sp;
			if (regen->state.overweight)
				val>>=1; //Half as fast when overweight.
			sregen->tick.sp += val;
			while(sregen->tick.sp >= (unsigned int)battle_config.natural_heal_skill_interval) {
				sregen->tick.sp -= battle_config.natural_heal_skill_interval;
				if(status->heal(bl, 0, sregen->sp, 3) < sregen->sp) {
					//Full
					flag&=~(RGN_SP|RGN_SSP);
					break;
				}
			}
		}
	}

	if (flag && regen->state.overweight)
		flag=0;

	ud = unit->bl2ud(bl);

	if (flag&(RGN_HP|RGN_SHP|RGN_SSP) && ud && ud->walktimer != INVALID_TIMER) {
		flag&=~(RGN_SHP|RGN_SSP);
		if(!regen->state.walk)
			flag&=~RGN_HP;
	}

	if (!flag)
		return 0;

	if (flag&(RGN_HP|RGN_SP)) {
		if(!vd) vd = status->get_viewdata(bl);
		if(vd && vd->dead_sit == 2)
			bonus++;
		if(regen->state.gc)
			bonus++;
	}

	//Natural Hp regen
	if (flag&RGN_HP) {
		rate = status->natural_heal_diff_tick*(regen->rate.hp+bonus);
		if (ud && ud->walktimer != INVALID_TIMER)
			rate/=2;
		// Homun HP regen fix (they should regen as if they were sitting (twice as fast)
		if(bl->type==BL_HOM) rate *=2;

		regen->tick.hp += rate;

		if(regen->tick.hp >= (unsigned int)battle_config.natural_healhp_interval) {
			val = 0;
			do {
				val += regen->hp;
				regen->tick.hp -= battle_config.natural_healhp_interval;
			} while(regen->tick.hp >= (unsigned int)battle_config.natural_healhp_interval);
			if (status->heal(bl, val, 0, 1) < val)
				flag&=~RGN_SHP; //full.
		}
	}

	//Natural SP regen
	if(flag&RGN_SP) {
		rate = status->natural_heal_diff_tick*(regen->rate.sp+bonus);
		// Homun SP regen fix (they should regen as if they were sitting (twice as fast)
		if(bl->type==BL_HOM) rate *=2;

		regen->tick.sp += rate;

		if(regen->tick.sp >= (unsigned int)battle_config.natural_healsp_interval) {
			val = 0;
			do {
				val += regen->sp;
				regen->tick.sp -= battle_config.natural_healsp_interval;
			} while(regen->tick.sp >= (unsigned int)battle_config.natural_healsp_interval);
			if (status->heal(bl, 0, val, 1) < val)
				flag&=~RGN_SSP; //full.
		}
	}

	if (!regen->sregen)
		return flag;

	//Skill regen
	sregen = regen->sregen;

	if(flag&RGN_SHP) {
		//Skill HP regen
		sregen->tick.hp += status->natural_heal_diff_tick * sregen->rate.hp;

		while(sregen->tick.hp >= (unsigned int)battle_config.natural_heal_skill_interval) {
			sregen->tick.hp -= battle_config.natural_heal_skill_interval;
			if(status->heal(bl, sregen->hp, 0, 3) < sregen->hp)
				break; //Full
		}
	}
	if(flag&RGN_SSP) {
		//Skill SP regen
		sregen->tick.sp += status->natural_heal_diff_tick * sregen->rate.sp;
		while(sregen->tick.sp >= (unsigned int)battle_config.natural_heal_skill_interval) {
			val = sregen->sp;
			if (sd && sd->state.doridori) {
				val*=2;
				sd->state.doridori = 0;
				if ((rate = pc->checkskill(sd,TK_SPTIME)))
					sc_start(bl,bl,status->skill2sc(TK_SPTIME),
					         100,rate,skill->get_time(TK_SPTIME, rate));
				if ((sd->class_&MAPID_UPPERMASK) == MAPID_STAR_GLADIATOR
				 &&rnd()%10000 < battle_config.sg_angel_skill_ratio
				) {
					//Angel of the Sun/Moon/Star
					clif->feel_hate_reset(sd);
					pc->resethate(sd);
					pc->resetfeel(sd);
				}
			}
			sregen->tick.sp -= battle_config.natural_heal_skill_interval;
			if(status->heal(bl, 0, val, 3) < val)
				break; //Full
		}
	}
	return flag;
}

//Natural heal main timer.
int status_natural_heal_timer(int tid, int64 tick, int id, intptr_t data) {
	// This difference is always positive and lower than UINT_MAX (~24 days)
	status->natural_heal_diff_tick = (unsigned int)cap_value(DIFF_TICK(tick,status->natural_heal_prev_tick), 0, UINT_MAX);
	map->foreachregen(status->natural_heal);
	status->natural_heal_prev_tick = tick;
	return 0;
}

/**
* Get the chance to upgrade a piece of equipment.
* @param wlv The weapon type of the item to refine (see see enum refine_type)
* @param refine The target refine level
* @return The chance to refine the item, in percent (0~100)
**/
int status_get_refine_chance(enum refine_type wlv, int refine) {

	if ( refine < 0 || refine >= MAX_REFINE)
	return 0;

	return status->refine_info[wlv].chance[refine];
}

int status_get_sc_type(sc_type type) {

	if( type <= SC_NONE || type >= SC_MAX )
		return 0;

	return status->sc_conf[type];
}

/*------------------------------------------
* DB reading.
* job_db1.txt    - weight, hp, sp, aspd
* job_db2.txt    - job level stat bonuses
* size_fix.txt   - size adjustment table for weapons
* refine_db.txt  - refining data table
*------------------------------------------*/
bool status_readdb_job1(char* fields[], int columns, int current)
{// Job-specific values (weight, HP, SP, ASPD)
	int idx, class_;
	unsigned int i;

	class_ = atoi(fields[0]);

	if(!pcdb_checkid(class_))
	{
		ShowWarning("status_readdb_job1: Invalid job class %d specified.\n", class_);
		return false;
	}
	idx = pc->class2idx(class_);

	status->max_weight_base[idx] = atoi(fields[1]);
	status->hp_coefficient[idx]  = atoi(fields[2]);
	status->hp_coefficient2[idx] = atoi(fields[3]);
	status->sp_coefficient[idx]  = atoi(fields[4]);
#ifdef RENEWAL_ASPD
	for(i = 0; i <= MAX_WEAPON_TYPE; i++)
#else
	for(i = 0; i < MAX_WEAPON_TYPE; i++)
#endif
	{
		status->aspd_base[idx][i] = atoi(fields[i+5]);
	}
	return true;
}

bool status_readdb_job2(char* fields[], int columns, int current)
{
	int idx, class_, i;

	class_ = atoi(fields[0]);

	if(!pcdb_checkid(class_))
	{
		ShowWarning("status_readdb_job2: Invalid job class %d specified.\n", class_);
		return false;
	}
	idx = pc->class2idx(class_);

	for(i = 1; i < columns; i++)
	{
		status->job_bonus[idx][i-1] = atoi(fields[i]);
	}
	return true;
}

bool status_readdb_sizefix(char* fields[], int columns, int current)
{
	unsigned int i;

	for(i = 0; i < MAX_WEAPON_TYPE; i++)
	{
		status->atkmods[current][i] = atoi(fields[i]);
	}
	return true;
}

bool status_readdb_refine(char* fields[], int columns, int current)
{
	int i, bonus_per_level, random_bonus, random_bonus_start_level;

	current = atoi(fields[0]);

	if (current < 0 || current >= REFINE_TYPE_MAX)
		return false;

	bonus_per_level = atoi(fields[1]);
	random_bonus_start_level = atoi(fields[2]);
	random_bonus = atoi(fields[3]);

	for(i = 0; i < MAX_REFINE; i++)
	{
		char* delim;

		if (!(delim = strchr(fields[4+i], ':')))
			return false;

		*delim = '\0';

		status->refine_info[current].chance[i] = atoi(fields[4+i]);

		if (i >= random_bonus_start_level - 1)
			status->refine_info[current].randombonus_max[i] = random_bonus * (i - random_bonus_start_level + 2);

		status->refine_info[current].bonus[i] = bonus_per_level + atoi(delim+1);
		if (i > 0)
			status->refine_info[current].bonus[i] += status->refine_info[current].bonus[i-1];
	}
	return true;
}

bool status_readdb_scconfig(char* fields[], int columns, int current) {
	int val = 0;
	char* type = fields[0];

	if( !script->get_constant(type, &val) ){
		ShowWarning("status_readdb_sc_conf: Invalid status type %s specified.\n", type);
		return false;
	}

	status->sc_conf[val] = (int)strtol(fields[1], NULL, 0);

	return true;
}
/**
 * Read status db
 * job1.txt
 * job2.txt
 * size_fixe.txt
 * refine_db.txt
 **/
int status_readdb(void)
{
	int i, j;

	// initialize databases to default
	//
	if( runflag == MAPSERVER_ST_RUNNING ) {//not necessary during boot
		// reset job_db1.txt data
		memset(status->max_weight_base, 0, sizeof(status->max_weight_base));
		memset(status->hp_coefficient, 0, sizeof(status->hp_coefficient));
		memset(status->hp_coefficient2, 0, sizeof(status->hp_coefficient2));
		memset(status->sp_coefficient, 0, sizeof(status->sp_coefficient));
		memset(status->aspd_base, 0, sizeof(status->aspd_base));
		// reset job_db2.txt data
		memset(status->job_bonus,0,sizeof(status->job_bonus)); // Job-specific stats bonus
	}
	// size_fix.txt
	for(i = 0; i < ARRAYLENGTH(status->atkmods); i++)
		for(j = 0; j < MAX_WEAPON_TYPE; j++)
			status->atkmods[i][j] = 100;

	// refine_db.txt
	for(i=0;i<ARRAYLENGTH(status->refine_info);i++) {
		for(j=0;j<MAX_REFINE; j++) {
			status->refine_info[i].chance[j] = 100;
			status->refine_info[i].bonus[j] = 0;
			status->refine_info[i].randombonus_max[j] = 0;
		}
	}

	// read databases
	//


#ifdef RENEWAL_ASPD
	sv->readdb(map->db_path, "re/job_db1.txt",      ',', 6+MAX_WEAPON_TYPE, 6+MAX_WEAPON_TYPE, -1,                       status->readdb_job1);
#else
	sv->readdb(map->db_path, "pre-re/job_db1.txt",  ',', 5+MAX_WEAPON_TYPE, 5+MAX_WEAPON_TYPE, -1,                       status->readdb_job1);
#endif
	sv->readdb(map->db_path, "job_db2.txt",         ',', 1,                 1+MAX_LEVEL,       -1,                       status->readdb_job2);
	sv->readdb(map->db_path, "size_fix.txt",        ',', MAX_WEAPON_TYPE,   MAX_WEAPON_TYPE,   ARRAYLENGTH(status->atkmods),     status->readdb_sizefix);
	sv->readdb(map->db_path, DBPATH"refine_db.txt", ',', 4+MAX_REFINE,      4+MAX_REFINE,      ARRAYLENGTH(status->refine_info), status->readdb_refine);
	sv->readdb(map->db_path, "sc_config.txt",       ',', 2,                 2,                 SC_MAX,                   status->readdb_scconfig);

	return 0;
}

/*==========================================
* Status db init and destroy.
*------------------------------------------*/
int do_init_status(bool minimal) {
	if (minimal)
		return 0;

	timer->add_func_list(status->change_timer,"status_change_timer");
	timer->add_func_list(status->kaahi_heal_timer,"status_kaahi_heal_timer");
	timer->add_func_list(status->natural_heal_timer,"status_natural_heal_timer");
	status->initChangeTables();
	status->initDummyData();
	status->readdb();
	status->calc_sigma();
	status->natural_heal_prev_tick = timer->gettick();
	status->data_ers = ers_new(sizeof(struct status_change_entry),"status.c::data_ers",ERS_OPT_NONE);
	timer->add_interval(status->natural_heal_prev_tick + NATURAL_HEAL_INTERVAL, status->natural_heal_timer, 0, 0, NATURAL_HEAL_INTERVAL);
	return 0;
}
void do_final_status(void) {
	ers_destroy(status->data_ers);
}

/*=====================================
* Default Functions : status.h 
* Generated by HerculesInterfaceMaker
* created by Susu
*-------------------------------------*/
void status_defaults(void) {
	status = &status_s;

	/* vars */
	//we need it for new cards 15 Feb 2005, to check if the combo cards are insrerted into the CURRENT weapon only
	//to avoid cards exploits
	status->current_equip_item_index = 0; //Contains inventory index of an equipped item. To pass it into the EQUP_SCRIPT [Lupus]
	status->current_equip_card_id = 0;    //To prevent card-stacking (from jA) [Skotlex]

	memset(status->max_weight_base,0,sizeof(status->max_weight_base)
		   + sizeof(status->hp_coefficient)
		   + sizeof(status->hp_coefficient2)
		   + sizeof(status->hp_sigma_val)
		   + sizeof(status->sp_coefficient)
		   + sizeof(status->aspd_base)
		   + sizeof(status->Skill2SCTable)
		   + sizeof(status->IconChangeTable)
		   + sizeof(status->ChangeFlagTable)
		   + sizeof(status->SkillChangeTable)
		   + sizeof(status->RelevantBLTypes)
		   + sizeof(status->DisplayType)
		   + sizeof(status->refine_info)
		   + sizeof(status->atkmods)
		   + sizeof(status->job_bonus)
		   + sizeof(status->sc_conf)
		   );

	status->data_ers = NULL;
	memset(&status->dummy, 0, sizeof(status->dummy));
	status->natural_heal_prev_tick = 0;
	status->natural_heal_diff_tick = 0;
	/* funcs */
	status->get_refine_chance = status_get_refine_chance;
	// for looking up associated data
	status->skill2sc = status_skill2sc;
	status->sc2skill = status_sc2skill;
	status->sc2scb_flag = status_sc2scb_flag;
	status->type2relevant_bl_types = status_type2relevant_bl_types;
	status->get_sc_type = status_get_sc_type;

	status->damage = status_damage;
	//Define for standard HP/SP skill-related cost triggers (mobs require no HP/SP to use skills)
	status->charge = status_charge;
	status->percent_change = status_percent_change;
	//Used to set the hp/sp of an object to an absolute value (can't kill)
	status->set_hp = status_set_hp;
	status->set_sp = status_set_sp;
	status->heal = status_heal;
	status->revive = status_revive;
	status->fixed_revive = status_fixed_revive;
	status->get_regen_data = status_get_regen_data;
	status->get_status_data = status_get_status_data;
	status->get_base_status = status_get_base_status;
	status->get_name = status_get_name;
	status->get_class = status_get_class;
	status->get_lv = status_get_lv;
	status->get_def = status_get_def;
	status->get_speed = status_get_speed;
	status->calc_attack_element = status_calc_attack_element;
	status->get_party_id = status_get_party_id;
	status->get_guild_id = status_get_guild_id;
	status->get_emblem_id = status_get_emblem_id;
	status->get_mexp = status_get_mexp;
	status->get_race2 = status_get_race2;

	status->get_viewdata = status_get_viewdata;
	status->set_viewdata = status_set_viewdata;
	status->change_init = status_change_init;
	status->get_sc = status_get_sc;

	status->isdead = status_isdead;
	status->isimmune = status_isimmune;

	status->get_sc_def = status_get_sc_def;

	status->change_start = status_change_start;
	status->change_end_ = status_change_end_;
	status->kaahi_heal_timer = kaahi_heal_timer;
	status->change_timer = status_change_timer;
	status->change_timer_sub = status_change_timer_sub;
	status->change_clear = status_change_clear;
	status->change_clear_buffs = status_change_clear_buffs;

	status->calc_bl_ = status_calc_bl_;
	status->calc_mob_ = status_calc_mob_;
	status->calc_pet_ = status_calc_pet_;
	status->calc_pc_ = status_calc_pc_;
	status->calc_homunculus_ = status_calc_homunculus_;
	status->calc_mercenary_ = status_calc_mercenary_;
	status->calc_elemental_ = status_calc_elemental_;

	status->calc_misc = status_calc_misc;
	status->calc_regen = status_calc_regen;
	status->calc_regen_rate = status_calc_regen_rate;

	status->check_skilluse = status_check_skilluse; // [Skotlex]
	status->check_visibility = status_check_visibility; //[Skotlex]

	status->change_spread = status_change_spread;

	status->calc_def = status_calc_def;
	status->calc_def2 = status_calc_def2;
	status->calc_mdef = status_calc_mdef;
	status->calc_mdef2 = status_calc_mdef2;
	status->calc_batk = status_calc_batk;
	status->base_matk = status_base_matk;
	status->get_weapon_atk = status_get_weapon_atk;
	status->get_total_mdef = status_get_total_mdef;
	status->get_total_def = status_get_total_def;

	status->get_matk = status_get_matk;

	status->readdb = status_readdb;
	status->init = do_init_status;
	status->final = do_final_status;
	
	status->initChangeTables = initChangeTables;
	status->initDummyData = initDummyData;
	status->base_amotion_pc = status_base_amotion_pc;
	status->base_atk = status_base_atk;
	status->calc_sigma = status_calc_sigma;
	status->base_pc_maxhp = status_base_pc_maxhp;
	status->base_pc_maxsp = status_base_pc_maxsp;
	status->calc_npc_ = status_calc_npc_;
	status->calc_str = status_calc_str;
	status->calc_agi = status_calc_agi;
	status->calc_vit = status_calc_vit;
	status->calc_int = status_calc_int;
	status->calc_dex = status_calc_dex;
	status->calc_luk = status_calc_luk;
	status->calc_watk = status_calc_watk;
	status->calc_matk = status_calc_matk;
	status->calc_hit = status_calc_hit;
	status->calc_critical = status_calc_critical;
	status->calc_flee = status_calc_flee;
	status->calc_flee2 = status_calc_flee2;
	status->calc_speed = status_calc_speed;
	status->calc_aspd_rate = status_calc_aspd_rate;
	status->calc_dmotion = status_calc_dmotion;
	status->calc_aspd = status_calc_aspd;
	status->calc_fix_aspd = status_calc_fix_aspd;
	status->calc_maxhp = status_calc_maxhp;
	status->calc_maxsp = status_calc_maxsp;
	status->calc_element = status_calc_element;
	status->calc_element_lv = status_calc_element_lv;
	status->calc_mode = status_calc_mode;
	status->calc_ematk = status_calc_ematk;
	status->calc_bl_main = status_calc_bl_main;
	status->display_add = status_display_add;
	status->display_remove = status_display_remove;
	status->natural_heal = status_natural_heal;
	status->natural_heal_timer = status_natural_heal_timer;
	status->readdb_job1 = status_readdb_job1;
	status->readdb_job2 = status_readdb_job2;
	status->readdb_sizefix = status_readdb_sizefix;
	status->readdb_refine = status_readdb_refine;
	status->readdb_scconfig = status_readdb_scconfig;
}
