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

//Regen related flags.
enum e_regen
{
	RGN_HP  = 0x01,
	RGN_SP  = 0x02,
	RGN_SHP = 0x04,
	RGN_SSP = 0x08,
};

static int max_weight_base[CLASS_COUNT];
static int hp_coefficient[CLASS_COUNT];
static int hp_coefficient2[CLASS_COUNT];
static int hp_sigma_val[CLASS_COUNT][MAX_LEVEL+1];
static int sp_coefficient[CLASS_COUNT];
#ifdef RENEWAL_ASPD
	static int aspd_base[CLASS_COUNT][MAX_WEAPON_TYPE+1];
#else
	static int aspd_base[CLASS_COUNT][MAX_WEAPON_TYPE];	//[blackhole89]
#endif

// bonus values and upgrade chances for refining equipment
static struct {
	int chance[MAX_REFINE]; // success chance
	int bonus[MAX_REFINE]; // cumulative fixed bonus damage
	int randombonus_max[MAX_REFINE]; // cumulative maximum random bonus damage
} refine_info[REFINE_TYPE_MAX];

static int atkmods[3][MAX_WEAPON_TYPE];	//ATK weapon modification for size (size_fix.txt)
static char job_bonus[CLASS_COUNT][MAX_LEVEL];

static struct eri *sc_data_ers; //For sc_data entries
static struct status_data dummy_status;

int current_equip_item_index; //Contains inventory index of an equipped item. To pass it into the EQUP_SCRIPT [Lupus]
int current_equip_card_id; //To prevent card-stacking (from jA) [Skotlex]
//we need it for new cards 15 Feb 2005, to check if the combo cards are insrerted into the CURRENT weapon only
//to avoid cards exploits

/**
 * Returns the status change associated with a skill.
 * @param skill The skill to look up
 * @return The status registered for this skill
 **/
sc_type status_skill2sc(int skill_id) {
	int idx;
	if( (idx = skill->get_index(skill_id)) == 0 ) {
		ShowError("status_skill2sc: Unsupported skill id %d\n", skill);
		return SC_NONE;
	}
	return SkillStatusChangeTable[idx];
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

	return StatusSkillChangeTable[sc];
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

	return StatusChangeFlagTable[sc];
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
		return SI_BLANK;
	}

	return StatusRelevantBLTypes[type];
}

#define add_sc(skill,sc) set_sc(skill,sc,SI_BLANK,SCB_NONE)
// indicates that the status displays a visual effect for the affected unit, and should be sent to the client for all supported units
#define set_sc_with_vfx(skill, sc, icon, flag) set_sc((skill), (sc), (icon), (flag)); if((icon) < SI_MAX) StatusRelevantBLTypes[(icon)] |= BL_SCEFFECT

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

	if( StatusSkillChangeTable[sc] == 0 )
  		StatusSkillChangeTable[sc] = skill_id;
	if( StatusIconChangeTable[sc] == SI_BLANK )
  		StatusIconChangeTable[sc] = icon;
	StatusChangeFlagTable[sc] |= flag;

	if( SkillStatusChangeTable[idx] == SC_NONE )
		SkillStatusChangeTable[idx] = sc;
}

void initChangeTables(void) {
	int i;

	for (i = 0; i < SC_MAX; i++)
		StatusIconChangeTable[i] = SI_BLANK;

	for (i = 0; i < MAX_SKILL; i++)
		SkillStatusChangeTable[i] = SC_NONE;

	for (i = 0; i < SI_MAX; i++)
		StatusRelevantBLTypes[i] = BL_PC;

	memset(StatusSkillChangeTable, 0, sizeof(StatusSkillChangeTable));
	memset(StatusChangeFlagTable, 0, sizeof(StatusChangeFlagTable));
	memset(StatusDisplayType, 0, sizeof(StatusDisplayType));

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
	set_sc( NPC_BLEEDING      , SC_BLEEDING  , SI_BLEEDING , SCB_REGEN );
	set_sc( NPC_POISON        , SC_DPOISON   , SI_BLANK    , SCB_DEF2|SCB_REGEN );

	//The main status definitions
	add_sc( SM_BASH              , SC_STUN            );
	set_sc( SM_PROVOKE           , SC_PROVOKE         , SI_PROVOKE         , SCB_DEF|SCB_DEF2|SCB_BATK|SCB_WATK );
	add_sc( SM_MAGNUM            , SC_WATK_ELEMENT    );
	set_sc( SM_ENDURE            , SC_ENDURE          , SI_ENDURE          , SCB_MDEF|SCB_DSPD );
	add_sc( MG_SIGHT             , SC_SIGHT           );
	add_sc( MG_SAFETYWALL        , SC_SAFETYWALL      );
	add_sc( MG_FROSTDIVER        , SC_FREEZE          );
	add_sc( MG_STONECURSE        , SC_STONE           );
	add_sc( AL_RUWACH            , SC_RUWACH          );
	add_sc( AL_PNEUMA            , SC_PNEUMA          );
	set_sc( AL_INCAGI            , SC_INCREASEAGI     , SI_INCREASEAGI     , SCB_AGI|SCB_SPEED );
	set_sc( AL_DECAGI            , SC_DECREASEAGI     , SI_DECREASEAGI     , SCB_AGI|SCB_SPEED );
	set_sc( AL_CRUCIS            , SC_SIGNUMCRUCIS    , SI_SIGNUMCRUCIS    , SCB_DEF );
	set_sc( AL_ANGELUS           , SC_ANGELUS         , SI_ANGELUS         , SCB_DEF2 );
	set_sc( AL_BLESSING          , SC_BLESSING        , SI_BLESSING        , SCB_STR|SCB_INT|SCB_DEX );
	set_sc( AC_CONCENTRATION     , SC_CONCENTRATE     , SI_CONCENTRATE     , SCB_AGI|SCB_DEX );
	set_sc( TF_HIDING            , SC_HIDING          , SI_HIDING          , SCB_SPEED );
	add_sc( TF_POISON            , SC_POISON          );
	set_sc( KN_TWOHANDQUICKEN    , SC_TWOHANDQUICKEN  , SI_TWOHANDQUICKEN  , SCB_ASPD );
	add_sc( KN_AUTOCOUNTER       , SC_AUTOCOUNTER     );
	set_sc( PR_IMPOSITIO         , SC_IMPOSITIO       , SI_IMPOSITIO       , SCB_WATK );
	set_sc( PR_SUFFRAGIUM        , SC_SUFFRAGIUM      , SI_SUFFRAGIUM      , SCB_NONE );
	set_sc( PR_ASPERSIO          , SC_ASPERSIO        , SI_ASPERSIO        , SCB_ATK_ELE );
	set_sc( PR_BENEDICTIO        , SC_BENEDICTIO      , SI_BENEDICTIO      , SCB_DEF_ELE );
	set_sc( PR_SLOWPOISON        , SC_SLOWPOISON      , SI_SLOWPOISON      , SCB_REGEN );
	set_sc( PR_KYRIE             , SC_KYRIE           , SI_KYRIE           , SCB_NONE );
	set_sc( PR_MAGNIFICAT        , SC_MAGNIFICAT      , SI_MAGNIFICAT      , SCB_REGEN );
	set_sc( PR_GLORIA            , SC_GLORIA          , SI_GLORIA          , SCB_LUK );
	add_sc( PR_LEXDIVINA         , SC_SILENCE         );
	set_sc( PR_LEXAETERNA        , SC_AETERNA         , SI_AETERNA         , SCB_NONE );
	add_sc( WZ_METEOR            , SC_STUN            );
	add_sc( WZ_VERMILION         , SC_BLIND           );
	add_sc( WZ_FROSTNOVA         , SC_FREEZE          );
	add_sc( WZ_STORMGUST         , SC_FREEZE          );
	set_sc( WZ_QUAGMIRE          , SC_QUAGMIRE        , SI_QUAGMIRE        , SCB_AGI|SCB_DEX|SCB_ASPD|SCB_SPEED );
	set_sc( BS_ADRENALINE        , SC_ADRENALINE      , SI_ADRENALINE      , SCB_ASPD );
	set_sc( BS_WEAPONPERFECT     , SC_WEAPONPERFECTION, SI_WEAPONPERFECTION, SCB_NONE );
	set_sc( BS_OVERTHRUST        , SC_OVERTHRUST      , SI_OVERTHRUST      , SCB_NONE );
	set_sc( BS_MAXIMIZE          , SC_MAXIMIZEPOWER   , SI_MAXIMIZEPOWER   , SCB_REGEN );
	add_sc( HT_LANDMINE          , SC_STUN            );
	add_sc( HT_ANKLESNARE        , SC_ANKLE           );
	add_sc( HT_SANDMAN           , SC_SLEEP           );
	add_sc( HT_FLASHER           , SC_BLIND           );
	add_sc( HT_FREEZINGTRAP      , SC_FREEZE          );
	set_sc( AS_CLOAKING          , SC_CLOAKING        , SI_CLOAKING        , SCB_CRI|SCB_SPEED );
	add_sc( AS_SONICBLOW         , SC_STUN            );
	set_sc( AS_ENCHANTPOISON     , SC_ENCPOISON       , SI_ENCPOISON       , SCB_ATK_ELE );
	set_sc( AS_POISONREACT       , SC_POISONREACT     , SI_POISONREACT     , SCB_NONE );
	add_sc( AS_VENOMDUST         , SC_POISON          );
	add_sc( AS_SPLASHER          , SC_SPLASHER        );
	set_sc( NV_TRICKDEAD         , SC_TRICKDEAD       , SI_TRICKDEAD       , SCB_REGEN );
	set_sc( SM_AUTOBERSERK       , SC_AUTOBERSERK     , SI_AUTOBERSERK     , SCB_NONE );
	add_sc( TF_SPRINKLESAND      , SC_BLIND           );
	add_sc( TF_THROWSTONE        , SC_STUN            );
	set_sc( MC_LOUD              , SC_LOUD            , SI_LOUD            , SCB_STR );
	set_sc( MG_ENERGYCOAT        , SC_ENERGYCOAT      , SI_ENERGYCOAT      , SCB_NONE );
	set_sc( NPC_EMOTION          , SC_MODECHANGE      , SI_BLANK           , SCB_MODE );
	add_sc( NPC_EMOTION_ON       , SC_MODECHANGE      );
	set_sc( NPC_ATTRICHANGE      , SC_ELEMENTALCHANGE , SI_ARMOR_PROPERTY  , SCB_DEF_ELE );
	add_sc( NPC_CHANGEWATER      , SC_ELEMENTALCHANGE );
	add_sc( NPC_CHANGEGROUND     , SC_ELEMENTALCHANGE );
	add_sc( NPC_CHANGEFIRE       , SC_ELEMENTALCHANGE );
	add_sc( NPC_CHANGEWIND       , SC_ELEMENTALCHANGE );
	add_sc( NPC_CHANGEPOISON     , SC_ELEMENTALCHANGE );
	add_sc( NPC_CHANGEHOLY       , SC_ELEMENTALCHANGE );
	add_sc( NPC_CHANGEDARKNESS   , SC_ELEMENTALCHANGE );
	add_sc( NPC_CHANGETELEKINESIS, SC_ELEMENTALCHANGE );
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
	set_sc( NPC_HALLUCINATION    , SC_HALLUCINATION   , SI_HALLUCINATION   , SCB_NONE );
	add_sc( NPC_REBIRTH          , SC_REBIRTH         );
	add_sc( RG_RAID              , SC_STUN            );
#ifdef RENEWAL
	add_sc( RG_RAID              , SC_RAID            );
	add_sc( RG_BACKSTAP          , SC_STUN            );
#endif
	set_sc( RG_STRIPWEAPON       , SC_STRIPWEAPON     , SI_STRIPWEAPON     , SCB_WATK );
	set_sc( RG_STRIPSHIELD       , SC_STRIPSHIELD     , SI_STRIPSHIELD     , SCB_DEF );
	set_sc( RG_STRIPARMOR        , SC_STRIPARMOR      , SI_STRIPARMOR      , SCB_VIT );
	set_sc( RG_STRIPHELM         , SC_STRIPHELM       , SI_STRIPHELM       , SCB_INT );
	add_sc( AM_ACIDTERROR        , SC_BLEEDING        );
	set_sc( AM_CP_WEAPON         , SC_CP_WEAPON       , SI_CP_WEAPON       , SCB_NONE );
	set_sc( AM_CP_SHIELD         , SC_CP_SHIELD       , SI_CP_SHIELD       , SCB_NONE );
	set_sc( AM_CP_ARMOR          , SC_CP_ARMOR        , SI_CP_ARMOR        , SCB_NONE );
	set_sc( AM_CP_HELM           , SC_CP_HELM         , SI_CP_HELM         , SCB_NONE );
	set_sc( CR_AUTOGUARD         , SC_AUTOGUARD       , SI_AUTOGUARD       , SCB_NONE );
	add_sc( CR_SHIELDCHARGE      , SC_STUN            );
	set_sc( CR_REFLECTSHIELD     , SC_REFLECTSHIELD   , SI_REFLECTSHIELD   , SCB_NONE );
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
	set_sc( MO_EXTREMITYFIST     , SC_EXTREMITYFIST2   , SI_EXTREMITYFIST   , SCB_NONE );
#endif
	add_sc( SA_MAGICROD          , SC_MAGICROD        );
	set_sc( SA_AUTOSPELL         , SC_AUTOSPELL       , SI_AUTOSPELL       , SCB_NONE );
	set_sc( SA_FLAMELAUNCHER     , SC_FIREWEAPON      , SI_FIREWEAPON      , SCB_ATK_ELE );
	set_sc( SA_FROSTWEAPON       , SC_WATERWEAPON     , SI_WATERWEAPON     , SCB_ATK_ELE );
	set_sc( SA_LIGHTNINGLOADER   , SC_WINDWEAPON      , SI_WINDWEAPON      , SCB_ATK_ELE );
	set_sc( SA_SEISMICWEAPON     , SC_EARTHWEAPON     , SI_EARTHWEAPON     , SCB_ATK_ELE );
	set_sc( SA_VOLCANO           , SC_VOLCANO         , SI_LANDENDOW       , SCB_WATK );
	set_sc( SA_DELUGE            , SC_DELUGE          , SI_LANDENDOW       , SCB_MAXHP );
	set_sc( SA_VIOLENTGALE       , SC_VIOLENTGALE     , SI_LANDENDOW       , SCB_FLEE );
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
	set_sc( DC_SERVICEFORYOU     , SC_SERVICE4U       , SI_BLANK           , SCB_ALL );
	add_sc( NPC_DARKCROSS        , SC_BLIND           );
	add_sc( NPC_GRANDDARKNESS    , SC_BLIND           );
	set_sc( NPC_STOP             , SC_STOP            , SI_STOP            , SCB_NONE );
	set_sc( NPC_WEAPONBRAKER     , SC_BROKENWEAPON    , SI_BROKENWEAPON    , SCB_NONE );
	set_sc( NPC_ARMORBRAKE       , SC_BROKENARMOR     , SI_BROKENARMOR     , SCB_NONE );
	set_sc( NPC_CHANGEUNDEAD     , SC_CHANGEUNDEAD    , SI_UNDEAD          , SCB_DEF_ELE );
	set_sc( NPC_POWERUP          , SC_INCHITRATE      , SI_BLANK           , SCB_HIT );
	set_sc( NPC_AGIUP            , SC_INCFLEERATE     , SI_BLANK           , SCB_FLEE );
	add_sc( NPC_INVISIBLE        , SC_CLOAKING        );
	set_sc( LK_AURABLADE         , SC_AURABLADE       , SI_AURABLADE       , SCB_NONE );
	set_sc( LK_PARRYING          , SC_PARRYING        , SI_PARRYING        , SCB_NONE );
	set_sc( LK_CONCENTRATION     , SC_CONCENTRATION   , SI_CONCENTRATION   , SCB_BATK|SCB_WATK|SCB_HIT|SCB_DEF|SCB_DEF2|SCB_MDEF|SCB_DSPD );
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
	set_sc( ST_REJECTSWORD       , SC_REJECTSWORD     , SI_REJECTSWORD     , SCB_NONE );
	add_sc( ST_REJECTSWORD       , SC_AUTOCOUNTER     );
	set_sc( CG_MARIONETTE        , SC_MARIONETTE      , SI_MARIONETTE      , SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK );
	set_sc( CG_MARIONETTE        , SC_MARIONETTE2     , SI_MARIONETTE2     , SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK );
	add_sc( LK_SPIRALPIERCE      , SC_STOP            );
	add_sc( LK_HEADCRUSH         , SC_BLEEDING        );
	set_sc( LK_JOINTBEAT         , SC_JOINTBEAT       , SI_JOINTBEAT       , SCB_BATK|SCB_DEF2|SCB_SPEED|SCB_ASPD );
	add_sc( HW_NAPALMVULCAN      , SC_CURSE           );
	set_sc( PF_MINDBREAKER       , SC_MINDBREAKER     , SI_BLANK           , SCB_MATK|SCB_MDEF2 );
	add_sc( PF_MEMORIZE          , SC_MEMORIZE        );
	add_sc( PF_FOGWALL           , SC_FOGWALL         );
	set_sc( PF_SPIDERWEB         , SC_SPIDERWEB       , SI_BLANK           , SCB_FLEE );
	set_sc( WE_BABY              , SC_BABY            , SI_BABY            , SCB_NONE );
	set_sc( TK_RUN               , SC_RUN             , SI_RUN             , SCB_SPEED|SCB_DSPD );
	set_sc( TK_RUN               , SC_SPURT           , SI_SPURT           , SCB_STR );
	set_sc( TK_READYSTORM        , SC_READYSTORM      , SI_READYSTORM      , SCB_NONE );
	set_sc( TK_READYDOWN         , SC_READYDOWN       , SI_READYDOWN       , SCB_NONE );
	add_sc( TK_DOWNKICK          , SC_STUN            );
	set_sc( TK_READYTURN         , SC_READYTURN       , SI_READYTURN       , SCB_NONE );
	set_sc( TK_READYCOUNTER      , SC_READYCOUNTER    , SI_READYCOUNTER    , SCB_NONE );
	set_sc( TK_DODGE             , SC_DODGE           , SI_DODGE           , SCB_NONE );
	set_sc( TK_SPTIME            , SC_EARTHSCROLL     , SI_EARTHSCROLL     , SCB_NONE );
	add_sc( TK_SEVENWIND         , SC_SEVENWIND );
	set_sc( TK_SEVENWIND         , SC_GHOSTWEAPON     , SI_GHOSTWEAPON     , SCB_ATK_ELE );
	set_sc( TK_SEVENWIND         , SC_SHADOWWEAPON    , SI_SHADOWWEAPON    , SCB_ATK_ELE );
	set_sc( SG_SUN_WARM          , SC_WARM            , SI_WARM            , SCB_NONE );
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
	set_sc( SL_SMA               , SC_SMA             , SI_SMA             , SCB_NONE );
	set_sc( SM_SELFPROVOKE       , SC_PROVOKE         , SI_PROVOKE         , SCB_DEF|SCB_DEF2|SCB_BATK|SCB_WATK );
	set_sc( ST_PRESERVE          , SC_PRESERVE        , SI_PRESERVE        , SCB_NONE );
	set_sc( PF_DOUBLECASTING     , SC_DOUBLECAST      , SI_DOUBLECAST      , SCB_NONE );
	set_sc( HW_GRAVITATION       , SC_GRAVITATION     , SI_BLANK           , SCB_ASPD );
	add_sc( WS_CARTTERMINATION   , SC_STUN            );
	set_sc( WS_OVERTHRUSTMAX     , SC_MAXOVERTHRUST   , SI_MAXOVERTHRUST   , SCB_NONE );
	set_sc( CG_LONGINGFREEDOM    , SC_LONGING         , SI_BLANK           , SCB_SPEED|SCB_ASPD );
	add_sc( CG_HERMODE           , SC_HERMODE         );
	set_sc( ITEM_ENCHANTARMS     , SC_ENCHANTARMS     , SI_BLANK           , SCB_ATK_ELE );
	set_sc( SL_HIGH              , SC_SPIRIT          , SI_SPIRIT          , SCB_ALL );
	set_sc( KN_ONEHAND           , SC_ONEHAND         , SI_ONEHAND         , SCB_ASPD );
	set_sc( GS_FLING             , SC_FLING           , SI_BLANK           , SCB_DEF|SCB_DEF2 );
	add_sc( GS_CRACKER           , SC_STUN            );
	add_sc( GS_DISARM            , SC_STRIPWEAPON     );
	add_sc( GS_PIERCINGSHOT      , SC_BLEEDING        );
	set_sc( GS_MADNESSCANCEL     , SC_MADNESSCANCEL   , SI_MADNESSCANCEL   , SCB_BATK|SCB_ASPD );
	set_sc( GS_ADJUSTMENT        , SC_ADJUSTMENT      , SI_ADJUSTMENT      , SCB_HIT|SCB_FLEE );
	set_sc( GS_INCREASING        , SC_INCREASING      , SI_ACCURACY        , SCB_AGI|SCB_DEX|SCB_HIT );
	set_sc( GS_GATLINGFEVER      , SC_GATLINGFEVER    , SI_GATLINGFEVER    , SCB_BATK|SCB_FLEE|SCB_SPEED|SCB_ASPD );
	set_sc( NJ_TATAMIGAESHI      , SC_TATAMIGAESHI    , SI_BLANK           , SCB_NONE );
	set_sc( NJ_SUITON            , SC_SUITON          , SI_BLANK           , SCB_AGI|SCB_SPEED );
	add_sc( NJ_HYOUSYOURAKU      , SC_FREEZE          );
	set_sc( NJ_NEN               , SC_NEN             , SI_NEN             , SCB_STR|SCB_INT );
	set_sc( NJ_UTSUSEMI          , SC_UTSUSEMI        , SI_UTSUSEMI        , SCB_NONE );
	set_sc( NJ_BUNSINJYUTSU      , SC_BUNSINJYUTSU    , SI_BUNSINJYUTSU    , SCB_DYE );

	add_sc( NPC_ICEBREATH        , SC_FREEZE          );
	add_sc( NPC_ACIDBREATH       , SC_POISON          );
	add_sc( NPC_HELLJUDGEMENT    , SC_CURSE           );
	add_sc( NPC_WIDESILENCE      , SC_SILENCE         );
	add_sc( NPC_WIDEFREEZE       , SC_FREEZE          );
	add_sc( NPC_WIDEBLEEDING     , SC_BLEEDING        );
	add_sc( NPC_WIDESTONE        , SC_STONE           );
	add_sc( NPC_WIDECONFUSE      , SC_CONFUSION       );
	add_sc( NPC_WIDESLEEP        , SC_SLEEP           );
	add_sc( NPC_WIDESIGHT        , SC_SIGHT           );
	add_sc( NPC_EVILLAND         , SC_BLIND           );
	add_sc( NPC_MAGICMIRROR      , SC_MAGICMIRROR     );
	set_sc( NPC_SLOWCAST         , SC_SLOWCAST        , SI_SLOWCAST        , SCB_NONE );
	set_sc( NPC_CRITICALWOUND    , SC_CRITICALWOUND   , SI_CRITICALWOUND   , SCB_NONE );
	set_sc( NPC_STONESKIN        , SC_ARMORCHANGE     , SI_BLANK           , SCB_DEF|SCB_MDEF );
	add_sc( NPC_ANTIMAGIC        , SC_ARMORCHANGE     );
	add_sc( NPC_WIDECURSE        , SC_CURSE           );
	add_sc( NPC_WIDESTUN         , SC_STUN            );

	set_sc( NPC_HELLPOWER        , SC_HELLPOWER       , SI_HELLPOWER       , SCB_NONE );
	set_sc( NPC_WIDEHELLDIGNITY  , SC_HELLPOWER       , SI_HELLPOWER       , SCB_NONE );
	set_sc( NPC_INVINCIBLE       , SC_INVINCIBLE      , SI_INVINCIBLE      , SCB_SPEED );
	set_sc( NPC_INVINCIBLEOFF    , SC_INVINCIBLEOFF   , SI_BLANK           , SCB_SPEED );

	set_sc( CASH_BLESSING        , SC_BLESSING        , SI_BLESSING        , SCB_STR|SCB_INT|SCB_DEX );
	set_sc( CASH_INCAGI          , SC_INCREASEAGI     , SI_INCREASEAGI     , SCB_AGI|SCB_SPEED );
	set_sc( CASH_ASSUMPTIO       , SC_ASSUMPTIO       , SI_ASSUMPTIO       , SCB_NONE );

	set_sc( ALL_PARTYFLEE        , SC_PARTYFLEE       , SI_PARTYFLEE       , SCB_NONE );
	set_sc( ALL_ODINS_POWER      , SC_ODINS_POWER     , SI_ODINS_POWER     , SCB_MATK|SCB_BATK|SCB_MDEF|SCB_DEF );

	set_sc( CR_SHRINK            , SC_SHRINK          , SI_SHRINK          , SCB_NONE );
	set_sc( RG_CLOSECONFINE      , SC_CLOSECONFINE2   , SI_CLOSECONFINE2   , SCB_NONE );
	set_sc( RG_CLOSECONFINE      , SC_CLOSECONFINE    , SI_CLOSECONFINE    , SCB_FLEE );
	set_sc( WZ_SIGHTBLASTER      , SC_SIGHTBLASTER    , SI_SIGHTBLASTER    , SCB_NONE );
	set_sc( DC_WINKCHARM         , SC_WINKCHARM       , SI_WINKCHARM       , SCB_NONE );
	add_sc( MO_BALKYOUNG         , SC_STUN            );
	add_sc( SA_ELEMENTWATER      , SC_ELEMENTALCHANGE );
	add_sc( SA_ELEMENTFIRE       , SC_ELEMENTALCHANGE );
	add_sc( SA_ELEMENTGROUND     , SC_ELEMENTALCHANGE );
	add_sc( SA_ELEMENTWIND       , SC_ELEMENTALCHANGE );

	set_sc( HLIF_AVOID           , SC_AVOID           , SI_BLANK           , SCB_SPEED );
	set_sc( HLIF_CHANGE          , SC_CHANGE          , SI_BLANK           , SCB_VIT|SCB_INT );
	set_sc( HFLI_FLEET           , SC_FLEET           , SI_BLANK           , SCB_ASPD|SCB_BATK|SCB_WATK );
	set_sc( HFLI_SPEED           , SC_SPEED           , SI_BLANK           , SCB_FLEE );
	set_sc( HAMI_DEFENCE         , SC_DEFENCE         , SI_BLANK           , SCB_DEF );
	set_sc( HAMI_BLOODLUST       , SC_BLOODLUST       , SI_BLANK           , SCB_BATK|SCB_WATK );

	// Homunculus S
	add_sc(MH_STAHL_HORN, SC_STUN);
	set_sc(MH_ANGRIFFS_MODUS, SC_ANGRIFFS_MODUS, SI_ANGRIFFS_MODUS, SCB_BATK | SCB_DEF | SCB_FLEE | SCB_MAXHP);
	set_sc(MH_GOLDENE_FERSE, SC_GOLDENE_FERSE, SI_GOLDENE_FERSE,  SCB_ASPD|SCB_MAXHP);
	add_sc( MH_STEINWAND, SC_SAFETYWALL );
	add_sc(MH_ERASER_CUTTER, SC_ERASER_CUTTER);
	set_sc(MH_OVERED_BOOST, SC_OVERED_BOOST, SI_BLANK, SCB_FLEE|SCB_ASPD);
	add_sc(MH_LIGHT_OF_REGENE, SC_LIGHT_OF_REGENE);
	set_sc(MH_VOLCANIC_ASH, SC_ASH, SI_VOLCANIC_ASH, SCB_DEF|SCB_DEF2|SCB_HIT|SCB_BATK|SCB_FLEE);
	set_sc(MH_GRANITIC_ARMOR, SC_GRANITIC_ARMOR, SI_GRANITIC_ARMOR, SCB_NONE);
	set_sc(MH_MAGMA_FLOW, SC_MAGMA_FLOW, SI_MAGMA_FLOW, SCB_NONE);
	set_sc(MH_PYROCLASTIC, SC_PYROCLASTIC, SI_PYROCLASTIC, SCB_BATK|SCB_ATK_ELE);
	add_sc(MH_LAVA_SLIDE, SC_BURNING);
	set_sc(MH_NEEDLE_OF_PARALYZE, SC_PARALYSIS, SI_NEEDLE_OF_PARALYZE, SCB_DEF2);
	add_sc(MH_POISON_MIST, SC_BLIND);
	set_sc(MH_PAIN_KILLER, SC_PAIN_KILLER, SI_PAIN_KILLER, SCB_ASPD);

	add_sc(MH_STYLE_CHANGE, SC_STYLE_CHANGE);
        set_sc( MH_TINDER_BREAKER      , SC_CLOSECONFINE2   , SI_CLOSECONFINE2   , SCB_NONE );
	set_sc( MH_TINDER_BREAKER      , SC_CLOSECONFINE    , SI_CLOSECONFINE    , SCB_FLEE );


	add_sc( MER_CRASH            , SC_STUN            );
	set_sc( MER_PROVOKE          , SC_PROVOKE         , SI_PROVOKE         , SCB_DEF|SCB_DEF2|SCB_BATK|SCB_WATK );
	add_sc( MS_MAGNUM            , SC_WATK_ELEMENT    );
	add_sc( MER_SIGHT            , SC_SIGHT           );
	set_sc( MER_DECAGI           , SC_DECREASEAGI     , SI_DECREASEAGI     , SCB_AGI|SCB_SPEED );
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
	set_sc( MER_QUICKEN          , SC_MERC_QUICKEN    , SI_BLANK           , SCB_ASPD );
	add_sc( ML_DEVOTION          , SC_DEVOTION        );
	set_sc( MER_KYRIE            , SC_KYRIE           , SI_KYRIE           , SCB_NONE );
	set_sc( MER_BLESSING         , SC_BLESSING        , SI_BLESSING        , SCB_STR|SCB_INT|SCB_DEX );
	set_sc( MER_INCAGI           , SC_INCREASEAGI     , SI_INCREASEAGI     , SCB_AGI|SCB_SPEED );

	set_sc( GD_LEADERSHIP        , SC_LEADERSHIP      , SI_BLANK           , SCB_STR );
	set_sc( GD_GLORYWOUNDS       , SC_GLORYWOUNDS     , SI_BLANK           , SCB_VIT );
	set_sc( GD_SOULCOLD          , SC_SOULCOLD        , SI_BLANK           , SCB_AGI );
	set_sc( GD_HAWKEYES          , SC_HAWKEYES        , SI_BLANK           , SCB_DEX );

	set_sc( GD_BATTLEORDER       , SC_BATTLEORDERS    , SI_BLANK           , SCB_STR|SCB_INT|SCB_DEX );
	set_sc( GD_REGENERATION      , SC_REGENERATION    , SI_BLANK           , SCB_REGEN );

	/**
	 * Rune Knight
	 **/
	set_sc( RK_ENCHANTBLADE      , SC_ENCHANTBLADE      , SI_ENCHANTBLADE      , SCB_NONE );
	set_sc( RK_DRAGONHOWLING     , SC_FEAR              , SI_BLANK             , SCB_FLEE|SCB_HIT );
	set_sc( RK_DEATHBOUND        , SC_DEATHBOUND        , SI_DEATHBOUND        , SCB_NONE );
	set_sc( RK_WINDCUTTER        , SC_FEAR              , SI_BLANK             , SCB_FLEE|SCB_HIT );
	add_sc( RK_DRAGONBREATH      , SC_BURNING           );
	set_sc( RK_MILLENNIUMSHIELD  , SC_MILLENNIUMSHIELD  , SI_REUSE_MILLENNIUMSHIELD  , SCB_NONE );
	set_sc( RK_REFRESH           , SC_REFRESH           , SI_REFRESH           , SCB_NONE );
	set_sc( RK_GIANTGROWTH       , SC_GIANTGROWTH       , SI_GIANTGROWTH       , SCB_STR );
	set_sc( RK_STONEHARDSKIN     , SC_STONEHARDSKIN     , SI_STONEHARDSKIN     , SCB_DEF|SCB_MDEF );
	set_sc( RK_VITALITYACTIVATION, SC_VITALITYACTIVATION, SI_VITALITYACTIVATION, SCB_REGEN );
	set_sc( RK_FIGHTINGSPIRIT    , SC_FIGHTINGSPIRIT    , SI_FIGHTINGSPIRIT    , SCB_WATK|SCB_ASPD );
	set_sc( RK_ABUNDANCE         , SC_ABUNDANCE         , SI_ABUNDANCE         , SCB_NONE );
	set_sc( RK_CRUSHSTRIKE		 , SC_CRUSHSTRIKE		, SI_CRUSHSTRIKE	   , SCB_NONE );
	/**
	 * GC Guillotine Cross
	 **/
	set_sc_with_vfx( GC_VENOMIMPRESS      , SC_VENOMIMPRESS     , SI_VENOMIMPRESS     , SCB_NONE );
	set_sc( GC_POISONINGWEAPON   , SC_POISONINGWEAPON  , SI_POISONINGWEAPON  , SCB_NONE );
	set_sc( GC_WEAPONBLOCKING    , SC_WEAPONBLOCKING   , SI_WEAPONBLOCKING   , SCB_NONE );
	set_sc( GC_CLOAKINGEXCEED    , SC_CLOAKINGEXCEED   , SI_CLOAKINGEXCEED   , SCB_SPEED );
	set_sc( GC_HALLUCINATIONWALK , SC_HALLUCINATIONWALK, SI_HALLUCINATIONWALK, SCB_FLEE );
	set_sc( GC_ROLLINGCUTTER     , SC_ROLLINGCUTTER    , SI_ROLLINGCUTTER    , SCB_NONE );
	/**
	 * Arch Bishop
	 **/
	set_sc( AB_ADORAMUS          , SC_ADORAMUS        , SI_ADORAMUS        , SCB_AGI|SCB_SPEED );
	add_sc( AB_CLEMENTIA         , SC_BLESSING );
	add_sc( AB_CANTO             , SC_INCREASEAGI );
	set_sc( AB_EPICLESIS         , SC_EPICLESIS       , SI_EPICLESIS       , SCB_MAXHP );
	add_sc( AB_PRAEFATIO         , SC_KYRIE );
	set_sc_with_vfx( AB_ORATIO            , SC_ORATIO          , SI_ORATIO          , SCB_NONE );
	set_sc( AB_LAUDAAGNUS        , SC_LAUDAAGNUS      , SI_LAUDAAGNUS      , SCB_VIT );
	set_sc( AB_LAUDARAMUS        , SC_LAUDARAMUS      , SI_LAUDARAMUS      , SCB_LUK );
	set_sc( AB_RENOVATIO         , SC_RENOVATIO       , SI_RENOVATIO       , SCB_REGEN );
	set_sc( AB_EXPIATIO          , SC_EXPIATIO        , SI_EXPIATIO        , SCB_ATK_ELE );
	set_sc( AB_DUPLELIGHT        , SC_DUPLELIGHT      , SI_DUPLELIGHT      , SCB_NONE );
	set_sc( AB_SECRAMENT         , SC_SECRAMENT       , SI_SECRAMENT       , SCB_NONE );
	/**
	 * Warlock
	 **/
	add_sc( WL_WHITEIMPRISON     , SC_WHITEIMPRISON );
	set_sc_with_vfx( WL_FROSTMISTY        , SC_FREEZING        , SI_FROSTMISTY      , SCB_ASPD|SCB_SPEED|SCB_DEF|SCB_DEF2 );
	set_sc( WL_MARSHOFABYSS      , SC_MARSHOFABYSS    , SI_MARSHOFABYSS    , SCB_SPEED|SCB_FLEE|SCB_DEF|SCB_MDEF );
    set_sc(WL_RECOGNIZEDSPELL   , SC_RECOGNIZEDSPELL , SI_RECOGNIZEDSPELL , SCB_MATK);
	set_sc( WL_STASIS            , SC_STASIS          , SI_STASIS          , SCB_NONE );
	/**
	 * Ranger
	 **/
	set_sc( RA_FEARBREEZE        , SC_FEARBREEZE      , SI_FEARBREEZE      , SCB_NONE );
	set_sc( RA_ELECTRICSHOCKER   , SC_ELECTRICSHOCKER , SI_ELECTRICSHOCKER , SCB_NONE );
	set_sc( RA_WUGDASH           , SC_WUGDASH         , SI_WUGDASH         , SCB_SPEED );
	set_sc( RA_CAMOUFLAGE        , SC_CAMOUFLAGE      , SI_CAMOUFLAGE      , SCB_SPEED );
	add_sc( RA_MAGENTATRAP       , SC_ELEMENTALCHANGE );
	add_sc( RA_COBALTTRAP        , SC_ELEMENTALCHANGE );
	add_sc( RA_MAIZETRAP         , SC_ELEMENTALCHANGE );
	add_sc( RA_VERDURETRAP       , SC_ELEMENTALCHANGE );
	add_sc( RA_FIRINGTRAP        , SC_BURNING         );
	set_sc_with_vfx( RA_ICEBOUNDTRAP      , SC_FREEZING        , SI_FROSTMISTY      , SCB_NONE );
	/**
	 * Mechanic
	 **/
	set_sc( NC_ACCELERATION      , SC_ACCELERATION    , SI_ACCELERATION    , SCB_SPEED );
	set_sc( NC_HOVERING          , SC_HOVERING        , SI_HOVERING        , SCB_SPEED );
	set_sc( NC_SHAPESHIFT        , SC_SHAPESHIFT      , SI_SHAPESHIFT      , SCB_DEF_ELE );
	set_sc( NC_INFRAREDSCAN      , SC_INFRAREDSCAN    , SI_INFRAREDSCAN    , SCB_FLEE );
	set_sc( NC_ANALYZE           , SC_ANALYZE         , SI_ANALYZE         , SCB_DEF|SCB_DEF2|SCB_MDEF|SCB_MDEF2 );
	set_sc( NC_MAGNETICFIELD     , SC_MAGNETICFIELD   , SI_MAGNETICFIELD   , SCB_NONE );
	set_sc( NC_NEUTRALBARRIER    , SC_NEUTRALBARRIER  , SI_NEUTRALBARRIER  , SCB_NONE );
	set_sc( NC_STEALTHFIELD      , SC_STEALTHFIELD    , SI_STEALTHFIELD    , SCB_NONE );
	/**
	 * Royal Guard
	 **/
	set_sc( LG_REFLECTDAMAGE     , SC_REFLECTDAMAGE   , SI_LG_REFLECTDAMAGE, SCB_NONE );
	set_sc( LG_FORCEOFVANGUARD   , SC_FORCEOFVANGUARD , SI_FORCEOFVANGUARD , SCB_MAXHP|SCB_DEF );
	set_sc( LG_EXEEDBREAK        , SC_EXEEDBREAK      , SI_EXEEDBREAK      , SCB_NONE );
	set_sc( LG_PRESTIGE          , SC_PRESTIGE        , SI_PRESTIGE        , SCB_DEF );
	set_sc( LG_BANDING           , SC_BANDING         , SI_BANDING         , SCB_DEF2|SCB_WATK );// Renewal: atk2 & def2
	set_sc( LG_PIETY             , SC_BENEDICTIO      , SI_BENEDICTIO      , SCB_DEF_ELE );
	set_sc( LG_EARTHDRIVE        , SC_EARTHDRIVE      , SI_EARTHDRIVE      , SCB_DEF|SCB_ASPD );
	set_sc( LG_INSPIRATION       , SC_INSPIRATION     , SI_INSPIRATION     , SCB_MAXHP|SCB_WATK|SCB_HIT|SCB_VIT|SCB_AGI|SCB_STR|SCB_DEX|SCB_INT|SCB_LUK);
	set_sc( LG_SHIELDSPELL       , SC_SHIELDSPELL_DEF , SI_SHIELDSPELL_DEF , SCB_WATK );
	set_sc( LG_SHIELDSPELL       , SC_SHIELDSPELL_REF , SI_SHIELDSPELL_REF , SCB_DEF );
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
	set_sc( SC_STRIPACCESSARY    , SC__STRIPACCESSORY , SI_STRIPACCESSARY  , SCB_DEX|SCB_INT|SCB_LUK );
	set_sc_with_vfx( SC_MANHOLE           , SC__MANHOLE        , SI_MANHOLE         , SCB_NONE );
	add_sc( SC_CHAOSPANIC        , SC_CONFUSION );
	set_sc_with_vfx( SC_BLOODYLUST        , SC__BLOODYLUST     , SI_BLOODYLUST      , SCB_DEF | SCB_DEF2 | SCB_MDEF | SCB_MDEF2 | SCB_FLEE | SCB_SPEED | SCB_ASPD | SCB_MAXHP | SCB_REGEN );
	/**
	 * Sura
	 **/
	add_sc( SR_DRAGONCOMBO           , SC_STUN            );
	add_sc( SR_EARTHSHAKER           , SC_STUN            );
	set_sc( SR_CRESCENTELBOW         , SC_CRESCENTELBOW      , SI_CRESCENTELBOW         , SCB_NONE );
	set_sc_with_vfx( SR_CURSEDCIRCLE          , SC_CURSEDCIRCLE_TARGET, SI_CURSEDCIRCLE_TARGET   , SCB_NONE );
	set_sc( SR_LIGHTNINGWALK         , SC_LIGHTNINGWALK      , SI_LIGHTNINGWALK         , SCB_NONE );
	set_sc( SR_RAISINGDRAGON         , SC_RAISINGDRAGON      , SI_RAISINGDRAGON         , SCB_REGEN|SCB_MAXHP|SCB_MAXSP );
	set_sc( SR_GENTLETOUCH_ENERGYGAIN, SC_GT_ENERGYGAIN      , SI_GENTLETOUCH_ENERGYGAIN, SCB_NONE );
	set_sc( SR_GENTLETOUCH_CHANGE    , SC_GT_CHANGE          , SI_GENTLETOUCH_CHANGE    , SCB_ASPD|SCB_MDEF|SCB_MAXHP );
	set_sc( SR_GENTLETOUCH_REVITALIZE, SC_GT_REVITALIZE      , SI_GENTLETOUCH_REVITALIZE, SCB_MAXHP|SCB_REGEN );
	/**
	 * Wanderer / Minstrel
	 **/
	set_sc( WA_SWING_DANCE            , SC_SWINGDANCE           , SI_SWINGDANCE           , SCB_SPEED|SCB_ASPD );
	set_sc( WA_SYMPHONY_OF_LOVER      , SC_SYMPHONYOFLOVER      , SI_SYMPHONYOFLOVERS     , SCB_MDEF );
	set_sc( WA_MOONLIT_SERENADE       , SC_MOONLITSERENADE      , SI_MOONLITSERENADE      , SCB_MATK );
	set_sc( MI_RUSH_WINDMILL          , SC_RUSHWINDMILL         , SI_RUSHWINDMILL         , SCB_BATK  );
	set_sc( MI_ECHOSONG               , SC_ECHOSONG             , SI_ECHOSONG             , SCB_DEF2  );
	set_sc( MI_HARMONIZE              , SC_HARMONIZE            , SI_HARMONIZE            , SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK );
	set_sc_with_vfx( WM_POEMOFNETHERWORLD      , SC_NETHERWORLD          , SI_NETHERWORLD          , SCB_NONE );
	set_sc_with_vfx( WM_VOICEOFSIREN           , SC_VOICEOFSIREN         , SI_VOICEOFSIREN         , SCB_NONE );
	set_sc_with_vfx( WM_LULLABY_DEEPSLEEP      , SC_DEEPSLEEP            , SI_DEEPSLEEP            , SCB_NONE );
	set_sc( WM_SIRCLEOFNATURE         , SC_SIRCLEOFNATURE       , SI_SIRCLEOFNATURE       , SCB_NONE );
	set_sc( WM_GLOOMYDAY              , SC_GLOOMYDAY            , SI_GLOOMYDAY            , SCB_FLEE|SCB_ASPD );
	set_sc( WM_SONG_OF_MANA           , SC_SONGOFMANA           , SI_SONGOFMANA           , SCB_NONE );
	set_sc( WM_DANCE_WITH_WUG         , SC_DANCEWITHWUG         , SI_DANCEWITHWUG         , SCB_ASPD );
	set_sc( WM_SATURDAY_NIGHT_FEVER   , SC_SATURDAYNIGHTFEVER   , SI_SATURDAYNIGHTFEVER   , SCB_BATK|SCB_DEF|SCB_FLEE|SCB_REGEN );
	set_sc( WM_LERADS_DEW             , SC_LERADSDEW            , SI_LERADSDEW            , SCB_MAXHP );
	set_sc( WM_MELODYOFSINK           , SC_MELODYOFSINK         , SI_MELODYOFSINK         , SCB_BATK|SCB_MATK );
	set_sc( WM_BEYOND_OF_WARCRY       , SC_BEYONDOFWARCRY       , SI_WARCRYOFBEYOND       , SCB_BATK|SCB_MATK );
	set_sc( WM_UNLIMITED_HUMMING_VOICE, SC_UNLIMITEDHUMMINGVOICE, SI_UNLIMITEDHUMMINGVOICE, SCB_NONE );
	/**
	 * Sorcerer
	 **/
	set_sc( SO_FIREWALK          , SC_PROPERTYWALK    , SI_PROPERTYWALK    , SCB_NONE );
	set_sc( SO_ELECTRICWALK      , SC_PROPERTYWALK    , SI_PROPERTYWALK    , SCB_NONE );
	set_sc( SO_SPELLFIST         , SC_SPELLFIST       , SI_SPELLFIST       , SCB_NONE );
	set_sc_with_vfx( SO_DIAMONDDUST       , SC_CRYSTALIZE      , SI_COLD   , SCB_NONE ); // it does show the snow icon on mobs but doesn't affect it.
	add_sc( SO_CLOUD_KILL		 , SC_POISON );
	set_sc( SO_STRIKING          , SC_STRIKING        , SI_STRIKING        , SCB_WATK|SCB_CRI );
	set_sc( SO_WARMER            , SC_WARMER          , SI_WARMER          , SCB_NONE );
	set_sc( SO_VACUUM_EXTREME    , SC_VACUUM_EXTREME  , SI_VACUUM_EXTREME  , SCB_NONE );
	set_sc( SO_ARRULLO           , SC_DEEPSLEEP       , SI_DEEPSLEEP       , SCB_NONE );
	set_sc( SO_FIRE_INSIGNIA     , SC_FIRE_INSIGNIA   , SI_FIRE_INSIGNIA   , SCB_MATK | SCB_BATK | SCB_WATK | SCB_ATK_ELE | SCB_REGEN );
	set_sc( SO_WATER_INSIGNIA    , SC_WATER_INSIGNIA  , SI_WATER_INSIGNIA  , SCB_WATK | SCB_ATK_ELE | SCB_REGEN );
	set_sc( SO_WIND_INSIGNIA     , SC_WIND_INSIGNIA   , SI_WIND_INSIGNIA   , SCB_WATK | SCB_ATK_ELE | SCB_REGEN );
	set_sc( SO_EARTH_INSIGNIA    , SC_EARTH_INSIGNIA  , SI_EARTH_INSIGNIA  , SCB_MDEF|SCB_DEF|SCB_MAXHP|SCB_MAXSP|SCB_WATK | SCB_ATK_ELE | SCB_REGEN );
	/**
	 * Genetic
	 **/
	set_sc( GN_CARTBOOST                  , SC_GN_CARTBOOST, SI_CARTSBOOST                 , SCB_SPEED );
	set_sc( GN_THORNS_TRAP                , SC_THORNSTRAP  , SI_THORNTRAP                  , SCB_NONE );
	set_sc_with_vfx( GN_BLOOD_SUCKER      , SC_BLOODSUCKER , SI_BLOODSUCKER                , SCB_NONE );
	set_sc( GN_WALLOFTHORN                , SC_STOP        , SI_BLANK                      , SCB_NONE );
	set_sc( GN_FIRE_EXPANSION_SMOKE_POWDER, SC_SMOKEPOWDER , SI_FIRE_EXPANSION_SMOKE_POWDER, SCB_NONE );
	set_sc( GN_FIRE_EXPANSION_TEAR_GAS    , SC_TEARGAS     , SI_FIRE_EXPANSION_TEAR_GAS    , SCB_NONE );
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
	set_sc( EL_CURSED_SOIL     , SC_CURSED_SOIL_OPTION   , SI_CURSED_SOIL_OPTION   , SCB_NONE );
	set_sc( EL_UPHEAVAL        , SC_UPHEAVAL_OPTION      , SI_UPHEAVAL_OPTION      , SCB_NONE );
	set_sc( EL_TIDAL_WEAPON    , SC_TIDAL_WEAPON_OPTION  , SI_TIDAL_WEAPON_OPTION  , SCB_ALL );
	set_sc( EL_ROCK_CRUSHER    , SC_ROCK_CRUSHER         , SI_ROCK_CRUSHER         , SCB_DEF );
	set_sc( EL_ROCK_CRUSHER_ATK, SC_ROCK_CRUSHER_ATK     , SI_ROCK_CRUSHER_ATK     , SCB_SPEED );

	add_sc( KO_YAMIKUMO			, SC_HIDING		  );
	set_sc_with_vfx( KO_JYUMONJIKIRI		, SC_JYUMONJIKIRI		 , SI_KO_JYUMONJIKIRI	   , SCB_NONE );
	add_sc( KO_MAKIBISHI	    , SC_STUN		  );
	set_sc( KO_MEIKYOUSISUI		, SC_MEIKYOUSISUI		 , SI_MEIKYOUSISUI		   , SCB_NONE );
	set_sc( KO_KYOUGAKU			, SC_KYOUGAKU			 , SI_KYOUGAKU			   , SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK );
	add_sc( KO_JYUSATSU			, SC_CURSE		  );
	set_sc( KO_ZENKAI			, SC_ZENKAI				 , SI_ZENKAI			   , SCB_NONE );
	set_sc( KO_IZAYOI			, SC_IZAYOI				 , SI_IZAYOI			   , SCB_MATK );
	set_sc( KG_KYOMU			, SC_KYOMU				 , SI_KYOMU				   , SCB_NONE );
	set_sc( KG_KAGEMUSYA		, SC_KAGEMUSYA			 , SI_KAGEMUSYA			   , SCB_NONE );
	set_sc( KG_KAGEHUMI			, SC_KAGEHUMI			 , SI_KG_KAGEHUMI		   , SCB_NONE );
	set_sc( OB_ZANGETSU			, SC_ZANGETSU			 , SI_ZANGETSU			   , SCB_MATK|SCB_BATK );
	set_sc_with_vfx( OB_AKAITSUKI		, SC_AKAITSUKI			 , SI_AKAITSUKI			   , SCB_NONE );
	set_sc( OB_OBOROGENSOU		, SC_GENSOU				 , SI_GENSOU			   , SCB_NONE );

	// Storing the target job rather than simply SC_SPIRIT simplifies code later on.
	SkillStatusChangeTable[SL_ALCHEMIST]   = (sc_type)MAPID_ALCHEMIST,
	SkillStatusChangeTable[SL_MONK]        = (sc_type)MAPID_MONK,
	SkillStatusChangeTable[SL_STAR]        = (sc_type)MAPID_STAR_GLADIATOR,
	SkillStatusChangeTable[SL_SAGE]        = (sc_type)MAPID_SAGE,
	SkillStatusChangeTable[SL_CRUSADER]    = (sc_type)MAPID_CRUSADER,
	SkillStatusChangeTable[SL_SUPERNOVICE] = (sc_type)MAPID_SUPER_NOVICE,
	SkillStatusChangeTable[SL_KNIGHT]      = (sc_type)MAPID_KNIGHT,
	SkillStatusChangeTable[SL_WIZARD]      = (sc_type)MAPID_WIZARD,
	SkillStatusChangeTable[SL_PRIEST]      = (sc_type)MAPID_PRIEST,
	SkillStatusChangeTable[SL_BARDDANCER]  = (sc_type)MAPID_BARDDANCER,
	SkillStatusChangeTable[SL_ROGUE]       = (sc_type)MAPID_ROGUE,
	SkillStatusChangeTable[SL_ASSASIN]     = (sc_type)MAPID_ASSASSIN,
	SkillStatusChangeTable[SL_BLACKSMITH]  = (sc_type)MAPID_BLACKSMITH,
	SkillStatusChangeTable[SL_HUNTER]      = (sc_type)MAPID_HUNTER,
	SkillStatusChangeTable[SL_SOULLINKER]  = (sc_type)MAPID_SOUL_LINKER,

	//Status that don't have a skill associated.
	StatusIconChangeTable[SC_WEIGHT50] = SI_WEIGHT50;
	StatusIconChangeTable[SC_WEIGHT90] = SI_WEIGHT90;
	StatusIconChangeTable[SC_ASPDPOTION0] = SI_ASPDPOTION0;
	StatusIconChangeTable[SC_ASPDPOTION1] = SI_ASPDPOTION1;
	StatusIconChangeTable[SC_ASPDPOTION2] = SI_ASPDPOTION2;
	StatusIconChangeTable[SC_ASPDPOTION3] = SI_ASPDPOTIONINFINITY;
	StatusIconChangeTable[SC_SPEEDUP0] = SI_MOVHASTE_HORSE;
	StatusIconChangeTable[SC_SPEEDUP1] = SI_SPEEDPOTION1;
	StatusIconChangeTable[SC_INCSTR] = SI_INCSTR;
	StatusIconChangeTable[SC_MIRACLE] = SI_SPIRIT;
	StatusIconChangeTable[SC_INTRAVISION] = SI_INTRAVISION;
	StatusIconChangeTable[SC_STRFOOD] = SI_FOODSTR;
	StatusIconChangeTable[SC_AGIFOOD] = SI_FOODAGI;
	StatusIconChangeTable[SC_VITFOOD] = SI_FOODVIT;
	StatusIconChangeTable[SC_INTFOOD] = SI_FOODINT;
	StatusIconChangeTable[SC_DEXFOOD] = SI_FOODDEX;
	StatusIconChangeTable[SC_LUKFOOD] = SI_FOODLUK;
	StatusIconChangeTable[SC_FLEEFOOD]= SI_FOODFLEE;
	StatusIconChangeTable[SC_HITFOOD] = SI_FOODHIT;
	StatusIconChangeTable[SC_MANU_ATK] = SI_MANU_ATK;
	StatusIconChangeTable[SC_MANU_DEF] = SI_MANU_DEF;
	StatusIconChangeTable[SC_SPL_ATK] = SI_SPL_ATK;
	StatusIconChangeTable[SC_SPL_DEF] = SI_SPL_DEF;
	StatusIconChangeTable[SC_MANU_MATK] = SI_MANU_MATK;
	StatusIconChangeTable[SC_SPL_MATK] = SI_SPL_MATK;
	StatusIconChangeTable[SC_ATKPOTION] = SI_PLUSATTACKPOWER;
	StatusIconChangeTable[SC_MATKPOTION] = SI_PLUSMAGICPOWER;
	//Cash Items
	StatusIconChangeTable[SC_FOOD_STR_CASH] = SI_FOOD_STR_CASH;
	StatusIconChangeTable[SC_FOOD_AGI_CASH] = SI_FOOD_AGI_CASH;
	StatusIconChangeTable[SC_FOOD_VIT_CASH] = SI_FOOD_VIT_CASH;
	StatusIconChangeTable[SC_FOOD_DEX_CASH] = SI_FOOD_DEX_CASH;
	StatusIconChangeTable[SC_FOOD_INT_CASH] = SI_FOOD_INT_CASH;
	StatusIconChangeTable[SC_FOOD_LUK_CASH] = SI_FOOD_LUK_CASH;
	StatusIconChangeTable[SC_EXPBOOST] = SI_EXPBOOST;
	StatusIconChangeTable[SC_ITEMBOOST] = SI_ITEMBOOST;
	StatusIconChangeTable[SC_JEXPBOOST] = SI_CASH_PLUSONLYJOBEXP;
	StatusIconChangeTable[SC_LIFEINSURANCE] = SI_LIFEINSURANCE;
	StatusIconChangeTable[SC_BOSSMAPINFO] = SI_BOSSMAPINFO;
	StatusIconChangeTable[SC_DEF_RATE] = SI_DEF_RATE;
	StatusIconChangeTable[SC_MDEF_RATE] = SI_MDEF_RATE;
	StatusIconChangeTable[SC_INCCRI] = SI_INCCRI;
	StatusIconChangeTable[SC_INCFLEE2] = SI_PLUSAVOIDVALUE;
	StatusIconChangeTable[SC_INCHEALRATE] = SI_INCHEALRATE;
	StatusIconChangeTable[SC_S_LIFEPOTION] = SI_S_LIFEPOTION;
	StatusIconChangeTable[SC_L_LIFEPOTION] = SI_L_LIFEPOTION;
	StatusIconChangeTable[SC_SPCOST_RATE] = SI_ATKER_BLOOD;
	StatusIconChangeTable[SC_COMMONSC_RESIST] = SI_TARGET_BLOOD;
	// Mercenary Bonus Effects
	StatusIconChangeTable[SC_MERC_FLEEUP] = SI_MERC_FLEEUP;
	StatusIconChangeTable[SC_MERC_ATKUP] = SI_MERC_ATKUP;
	StatusIconChangeTable[SC_MERC_HPUP] = SI_MERC_HPUP;
	StatusIconChangeTable[SC_MERC_SPUP] = SI_MERC_SPUP;
	StatusIconChangeTable[SC_MERC_HITUP] = SI_MERC_HITUP;
	// Warlock Spheres
	StatusIconChangeTable[SC_SPHERE_1] = SI_SPHERE_1;
	StatusIconChangeTable[SC_SPHERE_2] = SI_SPHERE_2;
	StatusIconChangeTable[SC_SPHERE_3] = SI_SPHERE_3;
	StatusIconChangeTable[SC_SPHERE_4] = SI_SPHERE_4;
	StatusIconChangeTable[SC_SPHERE_5] = SI_SPHERE_5;
	// Warlock Preserved spells
	StatusIconChangeTable[SC_SPELLBOOK1] = SI_SPELLBOOK1;
	StatusIconChangeTable[SC_SPELLBOOK2] = SI_SPELLBOOK2;
	StatusIconChangeTable[SC_SPELLBOOK3] = SI_SPELLBOOK3;
	StatusIconChangeTable[SC_SPELLBOOK4] = SI_SPELLBOOK4;
	StatusIconChangeTable[SC_SPELLBOOK5] = SI_SPELLBOOK5;
	StatusIconChangeTable[SC_SPELLBOOK6] = SI_SPELLBOOK6;
	StatusIconChangeTable[SC_MAXSPELLBOOK] = SI_SPELLBOOK7;

	StatusIconChangeTable[SC_NEUTRALBARRIER_MASTER] = SI_NEUTRALBARRIER_MASTER;
	StatusIconChangeTable[SC_STEALTHFIELD_MASTER] = SI_STEALTHFIELD_MASTER;
	StatusIconChangeTable[SC_OVERHEAT] = SI_OVERHEAT;
	StatusIconChangeTable[SC_OVERHEAT_LIMITPOINT] = SI_OVERHEAT_LIMITPOINT;

	StatusIconChangeTable[SC_HALLUCINATIONWALK_POSTDELAY] = SI_HALLUCINATIONWALK_POSTDELAY;
	StatusIconChangeTable[SC_TOXIN] = SI_TOXIN;
	StatusIconChangeTable[SC_PARALYSE] = SI_PARALYSE;
	StatusIconChangeTable[SC_VENOMBLEED] = SI_VENOMBLEED;
	StatusIconChangeTable[SC_MAGICMUSHROOM] = SI_MAGICMUSHROOM;
	StatusIconChangeTable[SC_DEATHHURT] = SI_DEATHHURT;
	StatusIconChangeTable[SC_PYREXIA] = SI_PYREXIA;
	StatusIconChangeTable[SC_OBLIVIONCURSE] = SI_OBLIVIONCURSE;
	StatusIconChangeTable[SC_LEECHESEND] = SI_LEECHESEND;

	StatusIconChangeTable[SC_SHIELDSPELL_DEF] = SI_SHIELDSPELL_DEF;
	StatusIconChangeTable[SC_SHIELDSPELL_MDEF] = SI_SHIELDSPELL_MDEF;
	StatusIconChangeTable[SC_SHIELDSPELL_REF] = SI_SHIELDSPELL_REF;
	StatusIconChangeTable[SC_BANDING_DEFENCE] = SI_BANDING_DEFENCE;

	StatusIconChangeTable[SC_GLOOMYDAY_SK] = SI_GLOOMYDAY;

	StatusIconChangeTable[SC_CURSEDCIRCLE_ATKER] = SI_CURSEDCIRCLE_ATKER;

	StatusIconChangeTable[SC_STOMACHACHE] = SI_STOMACHACHE;
	StatusIconChangeTable[SC_MYSTERIOUS_POWDER] = SI_MYSTERIOUS_POWDER;
	StatusIconChangeTable[SC_MELON_BOMB] = SI_MELON_BOMB;
	StatusIconChangeTable[SC_BANANA_BOMB] = SI_BANANA_BOMB;
	StatusIconChangeTable[SC_BANANA_BOMB_SITDOWN] = SI_BANANA_BOMB_SITDOWN_POSTDELAY;

	//Genetics New Food Items Status Icons
	StatusIconChangeTable[SC_SAVAGE_STEAK] = SI_SAVAGE_STEAK;
	StatusIconChangeTable[SC_COCKTAIL_WARG_BLOOD] = SI_COCKTAIL_WARG_BLOOD;
	StatusIconChangeTable[SC_MINOR_BBQ] = SI_MINOR_BBQ;
	StatusIconChangeTable[SC_SIROMA_ICE_TEA] = SI_SIROMA_ICE_TEA;
	StatusIconChangeTable[SC_DROCERA_HERB_STEAMED] = SI_DROCERA_HERB_STEAMED;
	StatusIconChangeTable[SC_PUTTI_TAILS_NOODLES] = SI_PUTTI_TAILS_NOODLES;

	StatusIconChangeTable[SC_BOOST500] |= SI_BOOST500;
	StatusIconChangeTable[SC_FULL_SWING_K] |= SI_FULL_SWING_K;
	StatusIconChangeTable[SC_MANA_PLUS] |= SI_MANA_PLUS;
	StatusIconChangeTable[SC_MUSTLE_M] |= SI_MUSTLE_M;
	StatusIconChangeTable[SC_LIFE_FORCE_F] |= SI_LIFE_FORCE_F;
	StatusIconChangeTable[SC_EXTRACT_WHITE_POTION_Z] |= SI_EXTRACT_WHITE_POTION_Z;
	StatusIconChangeTable[SC_VITATA_500] |= SI_VITATA_500;
	StatusIconChangeTable[SC_EXTRACT_SALAMINE_JUICE] |= SI_EXTRACT_SALAMINE_JUICE;

	// Elemental Spirit's 'side' status change icons.
	StatusIconChangeTable[SC_CIRCLE_OF_FIRE] = SI_CIRCLE_OF_FIRE;
	StatusIconChangeTable[SC_FIRE_CLOAK] = SI_FIRE_CLOAK;
	StatusIconChangeTable[SC_WATER_SCREEN] = SI_WATER_SCREEN;
	StatusIconChangeTable[SC_WATER_DROP] = SI_WATER_DROP;
	StatusIconChangeTable[SC_WIND_STEP] = SI_WIND_STEP;
	StatusIconChangeTable[SC_WIND_CURTAIN] = SI_WIND_CURTAIN;
	StatusIconChangeTable[SC_SOLID_SKIN] = SI_SOLID_SKIN;
	StatusIconChangeTable[SC_STONE_SHIELD] = SI_STONE_SHIELD;
	StatusIconChangeTable[SC_PYROTECHNIC] = SI_PYROTECHNIC;
	StatusIconChangeTable[SC_HEATER] = SI_HEATER;
	StatusIconChangeTable[SC_TROPIC] = SI_TROPIC;
	StatusIconChangeTable[SC_AQUAPLAY] = SI_AQUAPLAY;
	StatusIconChangeTable[SC_COOLER] = SI_COOLER;
	StatusIconChangeTable[SC_CHILLY_AIR] = SI_CHILLY_AIR;
	StatusIconChangeTable[SC_GUST] = SI_GUST;
	StatusIconChangeTable[SC_BLAST] = SI_BLAST;
	StatusIconChangeTable[SC_WILD_STORM] = SI_WILD_STORM;
	StatusIconChangeTable[SC_PETROLOGY] = SI_PETROLOGY;
	StatusIconChangeTable[SC_CURSED_SOIL] = SI_CURSED_SOIL;
	StatusIconChangeTable[SC_UPHEAVAL] = SI_UPHEAVAL;
	StatusIconChangeTable[SC_PUSH_CART] = SI_ON_PUSH_CART;
	StatusIconChangeTable[SC_ALL_RIDING] = SI_ALL_RIDING;

	//Other SC which are not necessarily associated to skills.
	StatusChangeFlagTable[SC_ASPDPOTION0] = SCB_ASPD;
	StatusChangeFlagTable[SC_ASPDPOTION1] = SCB_ASPD;
	StatusChangeFlagTable[SC_ASPDPOTION2] = SCB_ASPD;
	StatusChangeFlagTable[SC_ASPDPOTION3] = SCB_ASPD;
	StatusChangeFlagTable[SC_SPEEDUP0] = SCB_SPEED;
	StatusChangeFlagTable[SC_SPEEDUP1] = SCB_SPEED;
	StatusChangeFlagTable[SC_ATKPOTION] = SCB_BATK;
	StatusChangeFlagTable[SC_MATKPOTION] = SCB_MATK;
	StatusChangeFlagTable[SC_INCALLSTATUS] |= SCB_STR|SCB_AGI|SCB_VIT|SCB_INT|SCB_DEX|SCB_LUK;
	StatusChangeFlagTable[SC_INCSTR] |= SCB_STR;
	StatusChangeFlagTable[SC_INCAGI] |= SCB_AGI;
	StatusChangeFlagTable[SC_INCVIT] |= SCB_VIT;
	StatusChangeFlagTable[SC_INCINT] |= SCB_INT;
	StatusChangeFlagTable[SC_INCDEX] |= SCB_DEX;
	StatusChangeFlagTable[SC_INCLUK] |= SCB_LUK;
	StatusChangeFlagTable[SC_INCHIT] |= SCB_HIT;
	StatusChangeFlagTable[SC_INCHITRATE] |= SCB_HIT;
	StatusChangeFlagTable[SC_INCFLEE] |= SCB_FLEE;
	StatusChangeFlagTable[SC_INCFLEERATE] |= SCB_FLEE;
	StatusChangeFlagTable[SC_INCCRI] |= SCB_CRI;
	StatusChangeFlagTable[SC_INCASPDRATE] |= SCB_ASPD;
	StatusChangeFlagTable[SC_INCFLEE2] |= SCB_FLEE2;
	StatusChangeFlagTable[SC_INCMHPRATE] |= SCB_MAXHP;
	StatusChangeFlagTable[SC_INCMSPRATE] |= SCB_MAXSP;
	StatusChangeFlagTable[SC_INCMHP] |= SCB_MAXHP;
	StatusChangeFlagTable[SC_INCMSP] |= SCB_MAXSP;
	StatusChangeFlagTable[SC_INCATKRATE] |= SCB_BATK|SCB_WATK;
	StatusChangeFlagTable[SC_INCMATKRATE] |= SCB_MATK;
	StatusChangeFlagTable[SC_INCDEFRATE] |= SCB_DEF;
	StatusChangeFlagTable[SC_STRFOOD] |= SCB_STR;
	StatusChangeFlagTable[SC_AGIFOOD] |= SCB_AGI;
	StatusChangeFlagTable[SC_VITFOOD] |= SCB_VIT;
	StatusChangeFlagTable[SC_INTFOOD] |= SCB_INT;
	StatusChangeFlagTable[SC_DEXFOOD] |= SCB_DEX;
	StatusChangeFlagTable[SC_LUKFOOD] |= SCB_LUK;
	StatusChangeFlagTable[SC_HITFOOD] |= SCB_HIT;
	StatusChangeFlagTable[SC_FLEEFOOD] |= SCB_FLEE;
	StatusChangeFlagTable[SC_BATKFOOD] |= SCB_BATK;
	StatusChangeFlagTable[SC_WATKFOOD] |= SCB_WATK;
	StatusChangeFlagTable[SC_MATKFOOD] |= SCB_MATK;
	StatusChangeFlagTable[SC_ARMOR_ELEMENT] |= SCB_ALL;
	StatusChangeFlagTable[SC_ARMOR_RESIST] |= SCB_ALL;
	StatusChangeFlagTable[SC_SPCOST_RATE] |= SCB_ALL;
	StatusChangeFlagTable[SC_WALKSPEED] |= SCB_SPEED;
	StatusChangeFlagTable[SC_ITEMSCRIPT] |= SCB_ALL;
	// Cash Items
	StatusChangeFlagTable[SC_FOOD_STR_CASH] = SCB_STR;
	StatusChangeFlagTable[SC_FOOD_AGI_CASH] = SCB_AGI;
	StatusChangeFlagTable[SC_FOOD_VIT_CASH] = SCB_VIT;
	StatusChangeFlagTable[SC_FOOD_DEX_CASH] = SCB_DEX;
	StatusChangeFlagTable[SC_FOOD_INT_CASH] = SCB_INT;
	StatusChangeFlagTable[SC_FOOD_LUK_CASH] = SCB_LUK;
	// Mercenary Bonus Effects
	StatusChangeFlagTable[SC_MERC_FLEEUP] |= SCB_FLEE;
	StatusChangeFlagTable[SC_MERC_ATKUP] |= SCB_WATK;
	StatusChangeFlagTable[SC_MERC_HPUP] |= SCB_MAXHP;
	StatusChangeFlagTable[SC_MERC_SPUP] |= SCB_MAXSP;
	StatusChangeFlagTable[SC_MERC_HITUP] |= SCB_HIT;
	// Guillotine Cross Poison Effects
	StatusChangeFlagTable[SC_PARALYSE] |= SCB_ASPD|SCB_FLEE|SCB_SPEED;
	StatusChangeFlagTable[SC_DEATHHURT] |= SCB_REGEN;
	StatusChangeFlagTable[SC_VENOMBLEED] |= SCB_MAXHP;
	StatusChangeFlagTable[SC_OBLIVIONCURSE] |= SCB_REGEN;

	StatusChangeFlagTable[SC_SAVAGE_STEAK] |= SCB_STR;
	StatusChangeFlagTable[SC_COCKTAIL_WARG_BLOOD] |= SCB_INT;
	StatusChangeFlagTable[SC_MINOR_BBQ] |= SCB_VIT;
	StatusChangeFlagTable[SC_SIROMA_ICE_TEA] |= SCB_DEX;
	StatusChangeFlagTable[SC_DROCERA_HERB_STEAMED] |= SCB_AGI;
	StatusChangeFlagTable[SC_PUTTI_TAILS_NOODLES] |= SCB_LUK;
	StatusChangeFlagTable[SC_BOOST500] |= SCB_ASPD;
	StatusChangeFlagTable[SC_FULL_SWING_K] |= SCB_BATK;
	StatusChangeFlagTable[SC_MANA_PLUS] |= SCB_MATK;
	StatusChangeFlagTable[SC_MUSTLE_M] |= SCB_MAXHP;
	StatusChangeFlagTable[SC_LIFE_FORCE_F] |= SCB_MAXSP;
	StatusChangeFlagTable[SC_EXTRACT_WHITE_POTION_Z] |= SCB_REGEN;
	StatusChangeFlagTable[SC_VITATA_500] |= SCB_REGEN;
	StatusChangeFlagTable[SC_EXTRACT_SALAMINE_JUICE] |= SCB_ASPD;

	StatusChangeFlagTable[SC_ALL_RIDING] = SCB_SPEED;
	
	/* StatusDisplayType Table [Ind/Hercules] */
	StatusDisplayType[SC_ALL_RIDING]		= true;
	StatusDisplayType[SC_PUSH_CART]			= true;
	StatusDisplayType[SC_SPHERE_1]			= true;
	StatusDisplayType[SC_SPHERE_2]			= true;
	StatusDisplayType[SC_SPHERE_3]			= true;
	StatusDisplayType[SC_SPHERE_4]			= true;
	StatusDisplayType[SC_SPHERE_5]			= true;
	StatusDisplayType[SC_CAMOUFLAGE]		= true;
	StatusDisplayType[SC_DUPLELIGHT]		= true;
	StatusDisplayType[SC_ORATIO]			= true;
	StatusDisplayType[SC_FREEZING]			= true;
	StatusDisplayType[SC_VENOMIMPRESS]		= true;
	StatusDisplayType[SC_HALLUCINATIONWALK]	= true;
	StatusDisplayType[SC_ROLLINGCUTTER]		= true;
	StatusDisplayType[SC_BANDING]			= true;
	StatusDisplayType[SC_CRYSTALIZE]		= true;
	StatusDisplayType[SC_DEEPSLEEP]			= true;
	StatusDisplayType[SC_CURSEDCIRCLE_ATKER]= true;
	StatusDisplayType[SC_CURSEDCIRCLE_TARGET]= true;
	StatusDisplayType[SC_BLOODSUCKER]		= true;
	StatusDisplayType[SC__SHADOWFORM]		= true;
	StatusDisplayType[SC__MANHOLE]			= true;
	
#ifdef RENEWAL_EDP
	// renewal EDP increases your weapon atk
	StatusChangeFlagTable[SC_EDP] |= SCB_WATK;
#endif

	if( !battle_config.display_hallucination ) //Disable Hallucination.
		StatusIconChangeTable[SC_HALLUCINATION] = SI_BLANK;
}

static void initDummyData(void)
{
	memset(&dummy_status, 0, sizeof(dummy_status));
	dummy_status.hp =
	dummy_status.max_hp =
	dummy_status.max_sp =
	dummy_status.str =
	dummy_status.agi =
	dummy_status.vit =
	dummy_status.int_ =
	dummy_status.dex =
	dummy_status.luk =
	dummy_status.hit = 1;
	dummy_status.speed = 2000;
	dummy_status.adelay = 4000;
	dummy_status.amotion = 2000;
	dummy_status.dmotion = 2000;
	dummy_status.ele_lv = 1; //Min elemental level.
	dummy_status.mode = MD_CANMOVE;
}


//For copying a status_data structure from b to a, without overwriting current Hp and Sp
static inline void status_cpy(struct status_data* a, const struct status_data* b)
{
	memcpy((void*)&a->max_hp, (const void*)&b->max_hp, sizeof(struct status_data)-(sizeof(a->hp)+sizeof(a->sp)));
}

//Sets HP to given value. Flag is the flag passed to status_heal in case
//final value is higher than current (use 2 to make a healing effect display
//on players) It will always succeed (overrides Berserk block), but it can't kill.
int status_set_hp(struct block_list *bl, unsigned int hp, int flag)
{
	struct status_data *status;
	if (hp < 1) return 0;
	status = status_get_status_data(bl);
	if (status == &dummy_status)
		return 0;

	if (hp > status->max_hp) hp = status->max_hp;
	if (hp == status->hp) return 0;
	if (hp > status->hp)
		return status_heal(bl, hp - status->hp, 0, 1|flag);
	return status_zap(bl, status->hp - hp, 0);
}

//Sets SP to given value. Flag is the flag passed to status_heal in case
//final value is higher than current (use 2 to make a healing effect display
//on players)
int status_set_sp(struct block_list *bl, unsigned int sp, int flag)
{
	struct status_data *status;

	status = status_get_status_data(bl);
	if (status == &dummy_status)
		return 0;

	if (sp > status->max_sp) sp = status->max_sp;
	if (sp == status->sp) return 0;
	if (sp > status->sp)
		return status_heal(bl, 0, sp - status->sp, 1|flag);
	return status_zap(bl, 0, status->sp - sp);
}

int status_charge(struct block_list* bl, int hp, int sp)
{
	if(!(bl->type&BL_CONSUME))
		return hp+sp; //Assume all was charged so there are no 'not enough' fails.
	return status_damage(NULL, bl, hp, sp, 0, 3);
}

//Inflicts damage on the target with the according walkdelay.
//If flag&1, damage is passive and does not triggers cancelling status changes.
//If flag&2, fail if target does not has enough to substract.
//If flag&4, if killed, mob must not give exp/loot.
//flag will be set to &8 when damaging sp of a dead character
int status_damage(struct block_list *src,struct block_list *target,int hp, int sp, int walkdelay, int flag)
{
	struct status_data *status;
	struct status_change *sc;

	if(sp && !(target->type&BL_CONSUME))
		sp = 0; //Not a valid SP target.

	if (hp < 0) { //Assume absorbed damage.
		status_heal(target, -hp, 0, 1);
		hp = 0;
	}

	if (sp < 0) {
		status_heal(target, 0, -sp, 1);
		sp = 0;
	}

	if (target->type == BL_SKILL)
		return skill->unit_ondamaged((struct skill_unit *)target, src, hp, iTimer->gettick());

	status = status_get_status_data(target);
	if( status == &dummy_status )
		return 0;

	if ((unsigned int)hp >= status->hp) {
		if (flag&2) return 0;
		hp = status->hp;
	}

	if ((unsigned int)sp > status->sp) {
		if (flag&2) return 0;
		sp = status->sp;
	}

	if (!hp && !sp)
		return 0;

	if( !status->hp )
		flag |= 8;

// Let through. battle.c/skill.c have the whole logic of when it's possible or
// not to hurt someone (and this check breaks pet catching) [Skotlex]
//	if (!target->prev && !(flag&2))
//		return 0; //Cannot damage a bl not on a map, except when "charging" hp/sp

	sc = status_get_sc(target);
	if( hp && battle_config.invincible_nodamage && src && sc && sc->data[SC_INVINCIBLE] && !sc->data[SC_INVINCIBLEOFF] )
		hp = 1;

	if( hp && !(flag&1) ) {
		if( sc ) {
			struct status_change_entry *sce;
			if (sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
				status_change_end(target, SC_STONE, INVALID_TIMER);
			status_change_end(target, SC_FREEZE, INVALID_TIMER);
			status_change_end(target, SC_SLEEP, INVALID_TIMER);
			status_change_end(target, SC_WINKCHARM, INVALID_TIMER);
			status_change_end(target, SC_CONFUSION, INVALID_TIMER);
			status_change_end(target, SC_TRICKDEAD, INVALID_TIMER);
			status_change_end(target, SC_HIDING, INVALID_TIMER);
			status_change_end(target, SC_CLOAKING, INVALID_TIMER);
			status_change_end(target, SC_CHASEWALK, INVALID_TIMER);
			status_change_end(target, SC_CAMOUFLAGE, INVALID_TIMER);
			status_change_end(target, SC__INVISIBILITY, INVALID_TIMER);
			status_change_end(target, SC_DEEPSLEEP, INVALID_TIMER);
			if ((sce=sc->data[SC_ENDURE]) && !sce->val4) {
				//Endure count is only reduced by non-players on non-gvg maps.
				//val4 signals infinite endure. [Skotlex]
				if (src && src->type != BL_PC && !map_flag_gvg(target->m) && !map[target->m].flag.battleground && --(sce->val2) < 0)
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
			if(sc->data[SC_DANCING] && (unsigned int)hp > status->max_hp>>2)
				status_change_end(target, SC_DANCING, INVALID_TIMER);
			if(sc->data[SC_CLOAKINGEXCEED] && --(sc->data[SC_CLOAKINGEXCEED]->val2) <= 0)
				status_change_end(target, SC_CLOAKINGEXCEED, INVALID_TIMER);
			if(sc->data[SC_KAGEMUSYA] && --(sc->data[SC_KAGEMUSYA]->val3) <= 0)
				status_change_end(target, SC_KAGEMUSYA, INVALID_TIMER);
		}
		unit_skillcastcancel(target, 2);
	}

	status->hp-= hp;
	status->sp-= sp;

	if (sc && hp && status->hp) {
		if (sc->data[SC_AUTOBERSERK] &&
			(!sc->data[SC_PROVOKE] || !sc->data[SC_PROVOKE]->val2) &&
			status->hp < status->max_hp>>2)
			sc_start4(target,SC_PROVOKE,100,10,1,0,0,0);
		if (sc->data[SC_BERSERK] && status->hp <= 100)
			status_change_end(target, SC_BERSERK, INVALID_TIMER);
		if( sc->data[SC_RAISINGDRAGON] && status->hp <= 1000 )
			status_change_end(target, SC_RAISINGDRAGON, INVALID_TIMER);
		if (sc->data[SC_SATURDAYNIGHTFEVER] && status->hp <= 100)
			status_change_end(target, SC_SATURDAYNIGHTFEVER, INVALID_TIMER);
		if (sc->data[SC__BLOODYLUST] && status->hp <= 100)
			status_change_end(target, SC__BLOODYLUST, INVALID_TIMER);
	}

	switch (target->type) {
		case BL_PC:  iPc->damage((TBL_PC*)target,src,hp,sp); break;
		case BL_MOB: mob_damage((TBL_MOB*)target, src, hp); break;
		case BL_HOM: homun->damaged((TBL_HOM*)target); break;
		case BL_MER: mercenary_heal((TBL_MER*)target,hp,sp); break;
		case BL_ELEM: elemental_heal((TBL_ELEM*)target,hp,sp); break;
	}

	if( src && target->type == BL_PC && ((TBL_PC*)target)->disguise ) {// stop walking when attacked in disguise to prevent walk-delay bug
		unit_stop_walking( target, 1 );
	}

	if( status->hp || (flag&8) )
  	{	//Still lives or has been dead before this damage.
		if (walkdelay)
			unit_set_walkdelay(target, iTimer->gettick(), walkdelay, 0);
		return hp+sp;
	}

	status->hp = 1; //To let the dead function cast skills and all that.
	//NOTE: These dead functions should return: [Skotlex]
	//0: Death cancelled, auto-revived.
	//Non-zero: Standard death. Clear status, cancel move/attack, etc
	//&2: Also remove object from map.
	//&4: Also delete object from memory.
	switch (target->type) {
		case BL_PC:  flag = iPc->dead((TBL_PC*)target,src); break;
		case BL_MOB: flag = mob_dead((TBL_MOB*)target, src, flag&4?3:0); break;
		case BL_HOM: flag = homun->dead((TBL_HOM*)target); break;
		case BL_MER: flag = mercenary_dead((TBL_MER*)target); break;
		case BL_ELEM: flag = elemental_dead((TBL_ELEM*)target); break;
		default:	//Unhandled case, do nothing to object.
			flag = 0;
			break;
	}

	if(!flag) //Death cancelled.
		return hp+sp;

	//Normal death
	status->hp = 0;
	if (battle_config.clear_unit_ondeath &&
		battle_config.clear_unit_ondeath&target->type)
		skill->clear_unitgroup(target);

	if(target->type&BL_REGEN)
	{	//Reset regen ticks.
		struct regen_data *regen = status_get_regen_data(target);
		if (regen) {
			memset(&regen->tick, 0, sizeof(regen->tick));
			if (regen->sregen)
				memset(&regen->sregen->tick, 0, sizeof(regen->sregen->tick));
			if (regen->ssregen)
				memset(&regen->ssregen->tick, 0, sizeof(regen->ssregen->tick));
		}
	}

	if( sc && sc->data[SC_KAIZEL] && !map_flag_gvg(target->m) )
	{ //flag&8 = disable Kaizel
		int time = skill->get_time2(SL_KAIZEL,sc->data[SC_KAIZEL]->val1);
		//Look for Osiris Card's bonus effect on the character and revive 100% or revive normally
		if ( target->type == BL_PC && BL_CAST(BL_PC,target)->special_state.restart_full_recover )
			status_revive(target, 100, 100);
		else
			status_revive(target, sc->data[SC_KAIZEL]->val2, 0);
		status_change_clear(target,0);
		clif->skill_nodamage(target,target,ALL_RESURRECTION,1,1);
		sc_start(target,status_skill2sc(PR_KYRIE),100,10,time);

		if( target->type == BL_MOB )
			((TBL_MOB*)target)->state.rebirth = 1;

		return hp+sp;
	}
    if(target->type == BL_PC){
        TBL_PC *sd = BL_CAST(BL_PC,target);
        TBL_HOM *hd = sd->hd;
        if(hd && hd->sc.data[SC_LIGHT_OF_REGENE]){
            clif->skillcasting(&hd->bl, hd->bl.id, target->id, 0,0, MH_LIGHT_OF_REGENE, skill->get_ele(MH_LIGHT_OF_REGENE, 1), 10); //just to display usage
            clif->skill_nodamage(&sd->bl, target, ALL_RESURRECTION, 1, status_revive(&sd->bl,10*hd->sc.data[SC_LIGHT_OF_REGENE]->val1,0));
            status_change_end(&sd->hd->bl,SC_LIGHT_OF_REGENE,INVALID_TIMER);
            return hp + sp;
        }
    }
    if (target->type == BL_MOB && sc && sc->data[SC_REBIRTH] && !((TBL_MOB*) target)->state.rebirth) {// Ensure the monster has not already rebirthed before doing so.
        status_revive(target, sc->data[SC_REBIRTH]->val2, 0);
		status_change_clear(target,0);
		((TBL_MOB*)target)->state.rebirth = 1;

		return hp+sp;
	}

	status_change_clear(target,0);

	if(flag&4) //Delete from memory. (also invokes map removal code)
		unit_free(target,CLR_DEAD);
	else
	if(flag&2) //remove from map
		unit_remove_map(target,CLR_DEAD);
	else
	{ //Some death states that would normally be handled by unit_remove_map
		unit_stop_attack(target);
		unit_stop_walking(target,1);
		unit_skillcastcancel(target,0);
		clif->clearunit_area(target,CLR_DEAD);
		skill->unit_move(target,iTimer->gettick(),4);
		skill->cleartimerskill(target);
	}

	return hp+sp;
}

//Heals a character. If flag&1, this is forced healing (otherwise stuff like Berserk can block it)
//If flag&2, when the player is healed, show the HP/SP heal effect.
int status_heal(struct block_list *bl,int hp,int sp, int flag)
{
	struct status_data *status;
	struct status_change *sc;

	status = status_get_status_data(bl);

	if (status == &dummy_status || !status->hp)
		return 0;

	sc = status_get_sc(bl);
	if (sc && !sc->count)
		sc = NULL;

	if (hp < 0) {
		if (hp == INT_MIN) hp++; //-INT_MIN == INT_MIN in some architectures!
		status_damage(NULL, bl, -hp, 0, 0, 1);
		hp = 0;
	}

	if(hp) {
		if( sc && (sc->data[SC_BERSERK] || sc->data[SC__BLOODYLUST]) ) {
			if( flag&1 )
				flag &= ~2;
			else
				hp = 0;
		}

		if((unsigned int)hp > status->max_hp - status->hp)
			hp = status->max_hp - status->hp;
	}

	if(sp < 0) {
		if (sp==INT_MIN) sp++;
		status_damage(NULL, bl, 0, -sp, 0, 1);
		sp = 0;
	}

	if(sp) {
		if((unsigned int)sp > status->max_sp - status->sp)
			sp = status->max_sp - status->sp;
	}

	if(!sp && !hp) return 0;

	status->hp+= hp;
	status->sp+= sp;

	if(hp && sc &&
		sc->data[SC_AUTOBERSERK] &&
		sc->data[SC_PROVOKE] &&
		sc->data[SC_PROVOKE]->val2==1 &&
		status->hp>=status->max_hp>>2
	)	//End auto berserk.
		status_change_end(bl, SC_PROVOKE, INVALID_TIMER);

	// send hp update to client
	switch(bl->type) {
		case BL_PC:  iPc->heal((TBL_PC*)bl,hp,sp,flag&2?1:0); break;
		case BL_MOB: mob_heal((TBL_MOB*)bl,hp); break;
		case BL_HOM: homun->healed((TBL_HOM*)bl); break;
		case BL_MER: mercenary_heal((TBL_MER*)bl,hp,sp); break;
		case BL_ELEM: elemental_heal((TBL_ELEM*)bl,hp,sp); break;
	}

	return hp+sp;
}

//Does percentual non-flinching damage/heal. If mob is killed this way,
//no exp/drops will be awarded if there is no src (or src is target)
//If rates are > 0, percent is of current HP/SP
//If rates are < 0, percent is of max HP/SP
//If !flag, this is heal, otherwise it is damage.
//Furthermore, if flag==2, then the target must not die from the substraction.
int status_percent_change(struct block_list *src,struct block_list *target,signed char hp_rate, signed char sp_rate, int flag)
{
	struct status_data *status;
	unsigned int hp  =0, sp = 0;

	status = status_get_status_data(target);


	//It's safe now [MarkZD]
	if (hp_rate > 99)
		hp = status->hp;
	else if (hp_rate > 0)
		hp = status->hp>10000?
			hp_rate*(status->hp/100):
			((int64)hp_rate*status->hp)/100;
	else if (hp_rate < -99)
		hp = status->max_hp;
	else if (hp_rate < 0)
		hp = status->max_hp>10000?
			(-hp_rate)*(status->max_hp/100):
			((int64)-hp_rate*status->max_hp)/100;
	if (hp_rate && !hp)
		hp = 1;

	if (flag == 2 && hp >= status->hp)
		hp = status->hp-1; //Must not kill target.

	if (sp_rate > 99)
		sp = status->sp;
	else if (sp_rate > 0)
		sp = ((int64)sp_rate*status->sp)/100;
	else if (sp_rate < -99)
		sp = status->max_sp;
	else if (sp_rate < 0)
		sp = ((int64)-sp_rate)*status->max_sp/100;
	if (sp_rate && !sp)
		sp = 1;

	//Ugly check in case damage dealt is too much for the received args of
	//status_heal / status_damage. [Skotlex]
	if (hp > INT_MAX) {
	  	hp -= INT_MAX;
		if (flag)
			status_damage(src, target, INT_MAX, 0, 0, (!src||src==target?5:1));
		else
		  	status_heal(target, INT_MAX, 0, 0);
	}
  	if (sp > INT_MAX) {
		sp -= INT_MAX;
		if (flag)
			status_damage(src, target, 0, INT_MAX, 0, (!src||src==target?5:1));
		else
		  	status_heal(target, 0, INT_MAX, 0);
	}
	if (flag)
		return status_damage(src, target, hp, sp, 0, (!src||src==target?5:1));
	return status_heal(target, hp, sp, 0);
}

int status_revive(struct block_list *bl, unsigned char per_hp, unsigned char per_sp)
{
	struct status_data *status;
	unsigned int hp, sp;
	if (!status_isdead(bl)) return 0;

	status = status_get_status_data(bl);
	if (status == &dummy_status)
		return 0; //Invalid target.

	hp = (int64)status->max_hp * per_hp/100;
	sp = (int64)status->max_sp * per_sp/100;

	if(hp > status->max_hp - status->hp)
		hp = status->max_hp - status->hp;
	else if (per_hp && !hp)
		hp = 1;

	if(sp > status->max_sp - status->sp)
		sp = status->max_sp - status->sp;
	else if (per_sp && !sp)
		sp = 1;

	status->hp += hp;
	status->sp += sp;

	if (bl->prev) //Animation only if character is already on a map.
		clif->resurrection(bl, 1);
	switch (bl->type) {
		case BL_PC:  iPc->revive((TBL_PC*)bl, hp, sp); break;
		case BL_MOB: mob_revive((TBL_MOB*)bl, hp); break;
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
int status_check_skilluse(struct block_list *src, struct block_list *target, uint16 skill_id, int flag)
{
	struct status_data *status;
	struct status_change *sc=NULL, *tsc;
	int hide_flag;

	status = src?status_get_status_data(src):&dummy_status;

	if (src && src->type != BL_PC && status_isdead(src))
		return 0;

	if (!skill_id) { //Normal attack checks.
		if (!(status->mode&MD_CANATTACK))
			return 0; //This mode is only needed for melee attacking.
		//Dead state is not checked for skills as some skills can be used
		//on dead characters, said checks are left to skill.c [Skotlex]
		if (target && status_isdead(target))
			return 0;
		if( src && (sc = status_get_sc(src)) && sc->data[SC_CRYSTALIZE] && src->type != BL_MOB)
			return 0;
	}

	if( skill_id ) {
		
		if( src ) {
			int i;
			
			for(i = 0; i < map[src->m].zone->disabled_skills_count; i++) {
				if( skill_id == map[src->m].zone->disabled_skills[i]->nameid && (map[src->m].zone->disabled_skills[i]->type&src->type) ) {
					if( src->type == BL_PC )
						clif->msg((TBL_PC*)src, SKILL_CANT_USE_AREA); // This skill cannot be used within this area
					else if( src->type == BL_MOB && map[src->m].zone->disabled_skills[i]->subtype != MZS_NONE ) {
						if( (status->mode&MD_BOSS) && !(map[src->m].zone->disabled_skills[i]->subtype&MZS_BOSS) )
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
					tsc = status_get_sc(target);
					if(tsc && tsc->option&OPTION_HIDE)
						return 0;
				}
				break;
			case GN_WALLOFTHORN:
				if( target && status_isdead(target) )
					return 0;
				break;
			case AL_TELEPORT:
				//Should fail when used on top of Land Protector [Skotlex]
				if (src && iMap->getcell(src->m, src->x, src->y, CELL_CHKLANDPROTECTOR)
					&& !(status->mode&MD_BOSS)
					&& (src->type != BL_PC || ((TBL_PC*)src)->skillitem != skill_id))
					return 0;
				break;
			default:
				break;
		}
	}

	if ( src ) sc = status_get_sc(src);

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

		if (sc->data[SC_WINKCHARM] && target && !flag) { //Prevents skill usage
			if( unit_bl2ud(src) && (unit_bl2ud(src))->walktimer == INVALID_TIMER )
				unit_walktobl(src, iMap->id2bl(sc->data[SC_WINKCHARM]->val2), 3, 1);
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
				if( iPc->checkskill((TBL_PC*)src,WM_LESSON) < 5 )
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
				 sc->data[SC__BLOODYLUST] ||
				 sc->data[SC_OBLIVIONCURSE] ||
				 sc->data[SC_WHITEIMPRISON] ||
				 sc->data[SC__INVISIBILITY] ||
				(sc->data[SC_CRYSTALIZE] && src->type != BL_MOB) ||
				 sc->data[SC__IGNORANCE] ||
				 sc->data[SC_DEEPSLEEP] ||
				 sc->data[SC_SATURDAYNIGHTFEVER] ||
				 sc->data[SC_CURSEDCIRCLE_TARGET] ||
				(sc->data[SC_MARIONETTE] && skill_id != CG_MARIONETTE) || //Only skill you can use is marionette again to cancel it
				(sc->data[SC_MARIONETTE2] && skill_id == CG_MARIONETTE) || //Cannot use marionette if you are being buffed by another
				(sc->data[SC_STASIS] && skill->block_check(src, SC_STASIS, skill_id)) ||
				(sc->data[SC_KAGEHUMI] && skill->block_check(src, SC_KAGEHUMI, skill_id))
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

			if( sc->data[SC__MANHOLE] || ((tsc = status_get_sc(target)) && tsc->data[SC__MANHOLE]) ) {
				switch(skill_id) {//##TODO## make this a flag in skill_db?
					// Skills that can be used even under Man Hole effects.
					case SC_SHADOWFORM:
					case SC_STRIPACCESSARY:
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
		if (sc->option&OPTION_CHASEWALK && skill_id != ST_CHASEWALK)
			return 0;
		if( sc->data[SC_ALL_RIDING] )
			return 0;//New mounts can't attack nor use skills in the client; this check makes it cheat-safe [Ind]
	}

	if (target == NULL || target == src) //No further checking needed.
		return 1;

	tsc = status_get_sc(target);

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
	}

	//If targetting, cloak+hide protect you, otherwise only hiding does.
	hide_flag = flag?OPTION_HIDE:(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK);

 	//You cannot hide from ground skills.
	if( skill->get_ele(skill_id,1) == ELE_EARTH ) //TODO: Need Skill Lv here :/
		hide_flag &= ~OPTION_HIDE;

	switch( target->type ) {
		case BL_PC: {
				struct map_session_data *sd = (TBL_PC*) target;
				bool is_boss = (status->mode&MD_BOSS);
				bool is_detect = ((status->mode&MD_DETECTOR)?true:false);//god-knows-why gcc doesn't shut up until this happens
				if (pc_isinvisible(sd))
					return 0;
				if (tsc->option&hide_flag && !is_boss &&
					((sd->special_state.perfect_hiding || !is_detect) ||
					(tsc->data[SC_CLOAKINGEXCEED] && is_detect)))
					return 0;
				if( tsc->data[SC_CAMOUFLAGE] && !(is_boss || is_detect) && !skill_id )
					return 0;
				if( tsc->data[SC_STEALTHFIELD] && !is_boss )
					return 0;
			}
			break;
		case BL_ITEM:	//Allow targetting of items to pick'em up (or in the case of mobs, to loot them).
			//TODO: Would be nice if this could be used to judge whether the player can or not pick up the item it targets. [Skotlex]
			if (status->mode&MD_LOOTER)
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
				if( tsc->option&hide_flag && !(status->mode&(MD_BOSS|MD_DETECTOR)))
					return 0;
				if( tsc->data[SC_STEALTHFIELD] && !(status->mode&MD_BOSS) )
					return 0;
			}
	}
	return 1;
}

//Checks whether the source can see and chase target.
int status_check_visibility(struct block_list *src, struct block_list *target)
{
	int view_range;
	struct status_data* status = status_get_status_data(src);
	struct status_change* tsc = status_get_sc(target);
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

	if( tsc && tsc->data[SC_STEALTHFIELD] )
		return 0;

	switch (target->type)
	{	//Check for chase-walk/hiding/cloaking opponents.
	case BL_PC:
		if ( tsc->data[SC_CLOAKINGEXCEED] && !(status->mode&MD_BOSS) )
			return 0;
		if( (tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) || tsc->data[SC__INVISIBILITY] || tsc->data[SC_CAMOUFLAGE]) && !(status->mode&MD_BOSS) &&
			( ((TBL_PC*)target)->special_state.perfect_hiding || !(status->mode&MD_DETECTOR) ) )
			return 0;
		break;
	default:
		if( tsc && (tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) || tsc->data[SC__INVISIBILITY] || tsc->data[SC_CAMOUFLAGE]) && !(status->mode&(MD_BOSS|MD_DETECTOR)) )
			return 0;

	}

	return 1;
}

// Basic ASPD value
int status_base_amotion_pc(struct map_session_data* sd, struct status_data* status)
{
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
			? (aspd_base[iPc->class2idx(sd->status.class_)][sd->status.weapon]) // single weapon
			: ((aspd_base[iPc->class2idx(sd->status.class_)][sd->weapontype2] // dual-wield
			+ aspd_base[iPc->class2idx(sd->status.class_)][sd->weapontype2]) * 6 / 10 + 10 * mod
			- aspd_base[iPc->class2idx(sd->status.class_)][sd->weapontype2]
			+ aspd_base[iPc->class2idx(sd->status.class_)][sd->weapontype1]);

	if ( sd->status.shield )
			amotion += ( 2000 - aspd_base[iPc->class2idx(sd->status.class_)][W_FIST] ) +
					( aspd_base[iPc->class2idx(sd->status.class_)][MAX_WEAPON_TYPE] - 2000 );

#else
	// base weapon delay
	amotion = (sd->status.weapon < MAX_WEAPON_TYPE)
	 ? (aspd_base[iPc->class2idx(sd->status.class_)][sd->status.weapon]) // single weapon
	 : (aspd_base[iPc->class2idx(sd->status.class_)][sd->weapontype1] + aspd_base[iPc->class2idx(sd->status.class_)][sd->weapontype2])*7/10; // dual-wield

	// percentual delay reduction from stats
	amotion -= amotion * (4*status->agi + status->dex)/1000;
#endif
	// raw delay adjustment from bAspd bonus
	amotion += sd->bonus.aspd_add;

 	return amotion;
}

static unsigned short status_base_atk(const struct block_list *bl, const struct status_data *status)
{
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
		str = status->dex;
		dex = status->str;
	} else {
#ifdef RENEWAL
		rstr =
#endif
		str = status->str;
		dex = status->dex;
	}
	//Normally only players have base-atk, but homunc have a different batk
	// equation, hinting that perhaps non-players should use this for batk.
	// [Skotlex]
	dstr = str/10;
	str += dstr*dstr;
	if (bl->type == BL_PC)
#ifdef RENEWAL
		str = (rstr*10 + dex*10/5 + status->luk*10/3 + ((TBL_PC*)bl)->status.base_level*10/4)/10;
#else
		str+= dex/5 + status->luk/5;
#endif
	return cap_value(str, 0, USHRT_MAX);
}

#ifndef RENEWAL
static inline unsigned short status_base_matk_min(const struct status_data* status){ return status->int_+(status->int_/7)*(status->int_/7); }
static inline unsigned short status_base_matk_max(const struct status_data* status){ return status->int_+(status->int_/5)*(status->int_/5); }
#else
unsigned short status_base_matk(const struct status_data* status, int level){ return status->int_+(status->int_/2)+(status->dex/5)+(status->luk/3)+(level/4); }
#endif

//Fills in the misc data that can be calculated from the other status info (except for level)
void status_calc_misc(struct block_list *bl, struct status_data *status, int level)
{
	//Non players get the value set, players need to stack with previous bonuses.
	if( bl->type != BL_PC )
		status->batk =
		status->hit = status->flee =
		status->def2 = status->mdef2 =
		status->cri = status->flee2 = 0;

#ifdef RENEWAL // renewal formulas
    status->matk_min = status->matk_max = status_base_matk(status, level);
    status->hit += level + status->dex + status->luk/3 + 175; //base level + ( every 1 dex = +1 hit ) + (every 3 luk = +1 hit) + 175
    status->flee += level + status->agi + status->luk/5 + 100; //base level + ( every 1 agi = +1 flee ) + (every 5 luk = +1 flee) + 100
    status->def2 += (int)(((float)level + status->vit)/2 + ((float)status->agi/5)); //base level + (every 2 vit = +1 def) + (every 5 agi = +1 def)
    status->mdef2 += (int)(status->int_ + ((float)level/4) + ((float)status->dex/5) + ((float)status->vit/5)); //(every 4 base level = +1 mdef) + (every 1 int = +1 mdef) + (every 5 dex = +1 mdef) + (every 5 vit = +1 mdef)
#else
    status->matk_min = status_base_matk_min(status);
    status->matk_max = status_base_matk_max(status);
	status->hit += level + status->dex;
	status->flee += level + status->agi;
	status->def2 += status->vit;
	status->mdef2 += status->int_ + (status->vit>>1);
#endif

	if( bl->type&battle_config.enable_critical )
		status->cri += 10 + (status->luk*10/3); //(every 1 luk = +0.3 critical)
	else
		status->cri = 0;

	if (bl->type&battle_config.enable_perfect_flee)
		status->flee2 += status->luk + 10; //(every 10 luk = +1 perfect flee)
	else
		status->flee2 = 0;

	if (status->batk) {
		int temp = status->batk + status_base_atk(bl, status);
		status->batk = cap_value(temp, 0, USHRT_MAX);
	} else
		status->batk = status_base_atk(bl, status);
	if (status->cri)
	switch (bl->type) {
	case BL_MOB:
		if(battle_config.mob_critical_rate != 100)
			status->cri = status->cri*battle_config.mob_critical_rate/100;
		if(!status->cri && battle_config.mob_critical_rate)
		  	status->cri = 10;
		break;
	case BL_PC:
		//Players don't have a critical adjustment setting as of yet.
		break;
	default:
		if(battle_config.critical_rate != 100)
			status->cri = status->cri*battle_config.critical_rate/100;
		if (!status->cri && battle_config.critical_rate)
			status->cri = 10;
	}
	if(bl->type&BL_REGEN)
		status_calc_regen(bl, status, status_get_regen_data(bl));
}

//Skotlex: Calculates the initial status for the given mob
//first will only be false when the mob leveled up or got a GuardUp level.
int status_calc_mob_(struct mob_data* md, bool first)
{
	struct status_data *status;
	struct block_list *mbl = NULL;
	int flag=0;

	if(first)
	{	//Set basic level on respawn.
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
	if (md->class_ == MOBID_EMPERIUM)
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
		if(first)
			memcpy(&md->status, &md->db->status, sizeof(struct status_data));
		return 0;
	}
	if (!md->base_status)
		md->base_status = (struct status_data*)aCalloc(1, sizeof(struct status_data));

	status = md->base_status;
	memcpy(status, &md->db->status, sizeof(struct status_data));

	if (flag&(8|16))
		mbl = iMap->id2bl(md->master_id);

	if (flag&8 && mbl) {
		struct status_data *mstatus = status_get_base_status(mbl);
		if (mstatus &&
			battle_config.slaves_inherit_speed&(mstatus->mode&MD_CANMOVE?1:2))
			status->speed = mstatus->speed;
		if( status->speed < 2 ) /* minimum for the unit to function properly */
			status->speed = 2;
	}

	if (flag&16 && mbl)
	{	//Max HP setting from Summon Flora/marine Sphere
		struct unit_data *ud = unit_bl2ud(mbl);
		//Remove special AI when this is used by regular mobs.
		if (mbl->type == BL_MOB && !((TBL_MOB*)mbl)->special_state.ai)
			md->special_state.ai = 0;
		if (ud)
		{	// different levels of HP according to skill level
			if (ud->skill_id == AM_SPHEREMINE) {
				status->max_hp = 2000 + 400*ud->skill_lv;
			} else if(ud->skill_id == KO_ZANZOU){
				status->max_hp = 3000 + 3000 * ud->skill_lv;
			} else { //AM_CANNIBALIZE
				status->max_hp = 1500 + 200*ud->skill_lv + 10*status_get_lv(mbl);
				status->mode|= MD_CANATTACK|MD_AGGRESSIVE;
			}
			status->hp = status->max_hp;
		}
	}

	if (flag&1)
	{	// increase from mobs leveling up [Valaris]
		int diff = md->level - md->db->lv;
		status->str+= diff;
		status->agi+= diff;
		status->vit+= diff;
		status->int_+= diff;
		status->dex+= diff;
		status->luk+= diff;
		status->max_hp += diff*status->vit;
		status->max_sp += diff*status->int_;
		status->hp = status->max_hp;
		status->sp = status->max_sp;
		status->speed -= cap_value(diff, 0, status->speed - 10);
	}


	if (flag&2 && battle_config.mob_size_influence)
	{	// change for sized monsters [Valaris]
		if (md->special_state.size==SZ_MEDIUM) {
			status->max_hp>>=1;
			status->max_sp>>=1;
			if (!status->max_hp) status->max_hp = 1;
			if (!status->max_sp) status->max_sp = 1;
			status->hp=status->max_hp;
			status->sp=status->max_sp;
			status->str>>=1;
			status->agi>>=1;
			status->vit>>=1;
			status->int_>>=1;
			status->dex>>=1;
			status->luk>>=1;
			if (!status->str) status->str = 1;
			if (!status->agi) status->agi = 1;
			if (!status->vit) status->vit = 1;
			if (!status->int_) status->int_ = 1;
			if (!status->dex) status->dex = 1;
			if (!status->luk) status->luk = 1;
		} else if (md->special_state.size==SZ_BIG) {
			status->max_hp<<=1;
			status->max_sp<<=1;
			status->hp=status->max_hp;
			status->sp=status->max_sp;
			status->str<<=1;
			status->agi<<=1;
			status->vit<<=1;
			status->int_<<=1;
			status->dex<<=1;
			status->luk<<=1;
		}
	}

	status_calc_misc(&md->bl, status, md->level);

	if(flag&4)
	{	// Strengthen Guardians - custom value +10% / lv
		struct guild_castle *gc;
		gc=guild->mapname2gc(map[md->bl.m].name);
		if (!gc)
			ShowError("status_calc_mob: No castle set at map %s\n", map[md->bl.m].name);
		else
		if(gc->castle_id < 24 || md->class_ == MOBID_EMPERIUM) {
#ifdef RENEWAL
			status->max_hp += 50 * gc->defense;
			status->max_sp += 70 * gc->defense;
#else
			status->max_hp += 1000 * gc->defense;
			status->max_sp += 200 * gc->defense;
#endif
			status->hp = status->max_hp;
			status->sp = status->max_sp;
			status->def += (gc->defense+2)/3;
			status->mdef += (gc->defense+2)/3;
		}
		if(md->class_ != MOBID_EMPERIUM) {
			status->batk += status->batk * 10*md->guardian_data->guardup_lv/100;
			status->rhw.atk += status->rhw.atk * 10*md->guardian_data->guardup_lv/100;
			status->rhw.atk2 += status->rhw.atk2 * 10*md->guardian_data->guardup_lv/100;
			status->aspd_rate -= 100*md->guardian_data->guardup_lv;
		}
	}

	if( first ) //Initial battle status
		memcpy(&md->status, status, sizeof(struct status_data));

	return 1;
}

//Skotlex: Calculates the stats of the given pet.
int status_calc_pet_(struct pet_data *pd, bool first)
{
	nullpo_ret(pd);

	if (first) {
		memcpy(&pd->status, &pd->db->status, sizeof(struct status_data));
		pd->status.mode = MD_CANMOVE; // pets discard all modes, except walking
		pd->status.speed = pd->petDB->speed;

		if(battle_config.pet_attack_support || battle_config.pet_damage_support)
		{// attack support requires the pet to be able to attack
			pd->status.mode|= MD_CANATTACK;
		}
	}

	if (battle_config.pet_lv_rate && pd->msd)
	{
		struct map_session_data *sd = pd->msd;
		int lv;

		lv =sd->status.base_level*battle_config.pet_lv_rate/100;
		if (lv < 0)
			lv = 1;
		if (lv != pd->pet.level || first)
		{
			struct status_data *bstat = &pd->db->status, *status = &pd->status;
			pd->pet.level = lv;
			if (!first) //Lv Up animation
				clif->misceffect(&pd->bl, 0);
			status->rhw.atk = (bstat->rhw.atk*lv)/pd->db->lv;
			status->rhw.atk2 = (bstat->rhw.atk2*lv)/pd->db->lv;
			status->str = (bstat->str*lv)/pd->db->lv;
			status->agi = (bstat->agi*lv)/pd->db->lv;
			status->vit = (bstat->vit*lv)/pd->db->lv;
			status->int_ = (bstat->int_*lv)/pd->db->lv;
			status->dex = (bstat->dex*lv)/pd->db->lv;
			status->luk = (bstat->luk*lv)/pd->db->lv;

			status->rhw.atk = cap_value(status->rhw.atk, 1, battle_config.pet_max_atk1);
			status->rhw.atk2 = cap_value(status->rhw.atk2, 2, battle_config.pet_max_atk2);
			status->str = cap_value(status->str,1,battle_config.pet_max_stats);
			status->agi = cap_value(status->agi,1,battle_config.pet_max_stats);
			status->vit = cap_value(status->vit,1,battle_config.pet_max_stats);
			status->int_= cap_value(status->int_,1,battle_config.pet_max_stats);
			status->dex = cap_value(status->dex,1,battle_config.pet_max_stats);
			status->luk = cap_value(status->luk,1,battle_config.pet_max_stats);

			status_calc_misc(&pd->bl, &pd->status, lv);

			if (!first)	//Not done the first time because the pet is not visible yet
				clif->send_petstatus(sd);
		}
	} else if (first) {
		status_calc_misc(&pd->bl, &pd->status, pd->db->lv);
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
static void status_calc_sigma(void)
{
	int i,j;

	for(i = 0; i < CLASS_COUNT; i++)
	{
		unsigned int k = 0;
		hp_sigma_val[i][0] = hp_sigma_val[i][1] = 0;
		for(j = 2; j <= MAX_LEVEL; j++)
		{
			k += (hp_coefficient[i]*j + 50) / 100;
			hp_sigma_val[i][j] = k;
			if (k >= INT_MAX)
				break; //Overflow protection. [Skotlex]
		}
		for(; j <= MAX_LEVEL; j++)
			hp_sigma_val[i][j] = INT_MAX;
	}
}

/// Calculates base MaxHP value according to class and base level
/// The recursive equation used to calculate level bonus is (using integer operations)
///    f(0) = 35 | f(x+1) = f(x) + A + (x + B)*C/D
/// which reduces to something close to
///    f(x) = 35 + x*(A + B*C/D) + sum(i=2..x){ i*C/D }
static unsigned int status_base_pc_maxhp(struct map_session_data* sd, struct status_data* status)
{
	uint64 val = iPc->class2idx(sd->status.class_);
	val = 35 + sd->status.base_level*(int64)hp_coefficient2[val]/100 + hp_sigma_val[val][sd->status.base_level];

	if((sd->class_&MAPID_UPPERMASK) == MAPID_NINJA || (sd->class_&MAPID_UPPERMASK) == MAPID_GUNSLINGER)
		val += 100; //Since their HP can't be approximated well enough without this.
	if((sd->class_&MAPID_UPPERMASK) == MAPID_TAEKWON && sd->status.base_level >= 90 && iPc->famerank(sd->status.char_id, MAPID_TAEKWON))
		val *= 3; //Triple max HP for top ranking Taekwons over level 90.
	if((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->status.base_level >= 99)
		val += 2000; //Supernovice lvl99 hp bonus.

	val += val * status->vit/100; // +1% per each point of VIT

	if (sd->class_&JOBL_UPPER)
		val += val * 25/100; //Trans classes get a 25% hp bonus
	else if (sd->class_&JOBL_BABY)
		val -= val * 30/100; //Baby classes get a 30% hp penalty
	return (unsigned int)val;
}

static unsigned int status_base_pc_maxsp(struct map_session_data* sd, struct status_data *status)
{
	uint64 val;

	val = 10 + sd->status.base_level*(int64)sp_coefficient[iPc->class2idx(sd->status.class_)]/100;
	val += val * status->int_/100;

	if (sd->class_&JOBL_UPPER)
		val += val * 25/100;
	else if (sd->class_&JOBL_BABY)
		val -= val * 30/100;
	if ((sd->class_&MAPID_UPPERMASK) == MAPID_TAEKWON && sd->status.base_level >= 90 && iPc->famerank(sd->status.char_id, MAPID_TAEKWON))
		val *= 3; //Triple max SP for top ranking Taekwons over level 90.

	return (unsigned int)val;
}

//Calculates player data from scratch without counting SC adjustments.
//Should be invoked whenever players raise stats, learn passive skills or change equipment.
int status_calc_pc_(struct map_session_data* sd, bool first)
{
	static int calculating = 0; //Check for recursive call preemption. [Skotlex]
	struct status_data *status; // pointer to the player's base status
	const struct status_change *sc = &sd->sc;
	struct s_skill b_skill[MAX_SKILL]; // previous skill tree
	int b_weight, b_max_weight, b_cart_weight_max, // previous weight
	i, k, index, skill,refinedef=0;
	int64 i64;

	if (++calculating > 10) //Too many recursive calls!
		return -1;

	// remember player-specific values that are currently being shown to the client (for refresh purposes)
	memcpy(b_skill, &sd->status.skill, sizeof(b_skill));
	b_weight = sd->weight;
	b_max_weight = sd->max_weight;
	b_cart_weight_max = sd->cart_weight_max;

	iPc->calc_skilltree(sd);	// SkillTree calculation

	sd->max_weight = max_weight_base[iPc->class2idx(sd->status.class_)]+sd->status.str*300;

	if(first) {
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

	status = &sd->base_status;
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
		+ sizeof(sd->itemgrouphealrate)
		+ sizeof(sd->sp_gain_race)
		+ sizeof(sd->sp_gain_race_attack)
		+ sizeof(sd->hp_gain_race_attack)
		);

	memset (&sd->right_weapon.overrefine, 0, sizeof(sd->right_weapon) - sizeof(sd->right_weapon.atkmods));
	memset (&sd->left_weapon.overrefine, 0, sizeof(sd->left_weapon) - sizeof(sd->left_weapon.atkmods));

	if (sd->special_state.intravision && !sd->sc.data[SC_INTRAVISION]) //Clear intravision as long as nothing else is using it
		clif->sc_end(&sd->bl,sd->bl.id,SELF,SI_INTRAVISION);

	memset(&sd->special_state,0,sizeof(sd->special_state));
	memset(&status->max_hp, 0, sizeof(struct status_data)-(sizeof(status->hp)+sizeof(status->sp)));

	//FIXME: Most of these stuff should be calculated once, but how do I fix the memset above to do that? [Skotlex]
	if (!sd->state.permanent_speed)
		status->speed = DEFAULT_WALK_SPEED;
	//Give them all modes except these (useful for clones)
	status->mode = MD_MASK&~(MD_BOSS|MD_PLANT|MD_DETECTOR|MD_ANGRY|MD_TARGETWEAK);

	status->size = (sd->class_&JOBL_BABY)?SZ_SMALL:SZ_MEDIUM;
	if (battle_config.character_size && pc_isriding(sd)) { //[Lupus]
		if (sd->class_&JOBL_BABY) {
			if (battle_config.character_size&SZ_BIG)
				status->size++;
		} else
		if(battle_config.character_size&SZ_MEDIUM)
			status->size++;
	}
	status->aspd_rate = 1000;
	status->ele_lv = 1;
	status->race = RC_DEMIHUMAN;

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
	iPc->delautobonus(sd,sd->autobonus,ARRAYLENGTH(sd->autobonus),true);
	iPc->delautobonus(sd,sd->autobonus2,ARRAYLENGTH(sd->autobonus2),true);
	iPc->delautobonus(sd,sd->autobonus3,ARRAYLENGTH(sd->autobonus3),true);

	// Parse equipment.
	for(i=0;i<EQI_MAX-1;i++) {
		current_equip_item_index = index = sd->equip_index[i]; //We pass INDEX to current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
		if(index < 0)
			continue;
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

		for(k = 0; k < map[sd->bl.m].zone->disabled_items_count; k++) {
			if( map[sd->bl.m].zone->disabled_items[k] == sd->inventory_data[index]->nameid ) {
				break;
			}
		}
		
		if( k < map[sd->bl.m].zone->disabled_items_count )
			continue;
		
		status->def += sd->inventory_data[index]->def;

		if(first && sd->inventory_data[index]->equip_script)
	  	{	//Execute equip-script on login
			run_script(sd->inventory_data[index]->equip_script,0,sd->bl.id,0);
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
				wa = &status->lhw;
			} else {
				wd = &sd->right_weapon;
				wa = &status->rhw;
			}
			wa->atk += sd->inventory_data[index]->atk;
			if ( (r = sd->status.inventory[index].refine) )
				wa->atk2 = refine_info[wlv].bonus[r-1] / 100;

#ifdef RENEWAL
            wa->matk += sd->inventory_data[index]->matk;
            wa->wlv = wlv;
            if( r ) // renewal magic attack refine bonus
                wa->matk += refine_info[wlv].bonus[r-1] / 100;
#endif

			//Overrefine bonus.
			if (r)
				wd->overrefine = refine_info[wlv].randombonus_max[r-1] / 100;

			wa->range += sd->inventory_data[index]->range;
			if(sd->inventory_data[index]->script) {
				if (wd == &sd->left_weapon) {
					sd->state.lr_flag = 1;
					run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
					sd->state.lr_flag = 0;
				} else
					run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
				if (!calculating) //Abort, run_script retriggered this. [Skotlex]
					return 1;
			}

			if(sd->status.inventory[index].card[0]==CARD0_FORGE)
			{	// Forged weapon
				wd->star += (sd->status.inventory[index].card[1]>>8);
				if(wd->star >= 15) wd->star = 40; // 3 Star Crumbs now give +40 dmg
				if(iPc->famerank(MakeDWord(sd->status.inventory[index].card[2],sd->status.inventory[index].card[3]) ,MAPID_BLACKSMITH))
					wd->star += 10;

				if (!wa->ele) //Do not overwrite element from previous bonuses.
					wa->ele = (sd->status.inventory[index].card[1]&0x0f);
			}
		}
		else if(sd->inventory_data[index]->type == IT_ARMOR) {
			int r;
			if ( (r = sd->status.inventory[index].refine) )
				refinedef += refine_info[REFINE_TYPE_ARMOR].bonus[r-1];
			if(sd->inventory_data[index]->script) {
				if( i == EQI_HAND_L ) //Shield
					sd->state.lr_flag = 3;
				run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
				if( i == EQI_HAND_L ) //Shield
					sd->state.lr_flag = 0;
				if (!calculating) //Abort, run_script retriggered this. [Skotlex]
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
				run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
			sd->state.lr_flag = 0;
			if (!calculating) //Abort, run_script retriggered status_calc_pc. [Skotlex]
				return 1;
		}
	}

	/* we've got combos to process */
	if( sd->combos.count ) {
		for( i = 0; i < sd->combos.count; i++ ) {
			run_script(sd->combos.bonus[i],0,sd->bl.id,0);
			if (!calculating) //Abort, run_script retriggered this.
				return 1;
		}
	}

	//Store equipment script bonuses
	memcpy(sd->param_equip,sd->param_bonus,sizeof(sd->param_equip));
	memset(sd->param_bonus, 0, sizeof(sd->param_bonus));

	status->def += (refinedef+50)/100;

	//Parse Cards
	for(i=0;i<EQI_MAX-1;i++) {
		current_equip_item_index = index = sd->equip_index[i]; //We pass INDEX to current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
		if(index < 0)
			continue;
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
			for(j=0;j<MAX_SLOTS;j++){ // Uses MAX_SLOTS to support Soul Bound system [Inkfish]
				current_equip_card_id= c= sd->status.inventory[index].card[j];
				if(!c)
					continue;
				data = itemdb_exists(c);
				if(!data)
					continue;
				if(first && data->equip_script) {//Execute equip-script on login
					run_script(data->equip_script,0,sd->bl.id,0);
					if (!calculating)
						return 1;
				}
				if(!data->script)
					continue;
				
				for(k = 0; k < map[sd->bl.m].zone->disabled_items_count; k++) {
					if( map[sd->bl.m].zone->disabled_items[k] == data->nameid ) {
						break;
					}
				}
				
				if( k < map[sd->bl.m].zone->disabled_items_count )
					continue;
				
				if(i == EQI_HAND_L && sd->status.inventory[index].equip == EQP_HAND_L) { //Left hand status.
					sd->state.lr_flag = 1;
					run_script(data->script,0,sd->bl.id,0);
					sd->state.lr_flag = 0;
				} else
					run_script(data->script,0,sd->bl.id,0);
				if (!calculating) //Abort, run_script his function. [Skotlex]
					return 1;
			}
		}
	}

	if( sc->count && sc->data[SC_ITEMSCRIPT] )
	{
		struct item_data *data = itemdb_exists(sc->data[SC_ITEMSCRIPT]->val1);
		if( data && data->script )
			run_script(data->script,0,sd->bl.id,0);
	}

	if( sd->pd )
	{ // Pet Bonus
		struct pet_data *pd = sd->pd;
		if( pd && pd->petDB && pd->petDB->equip_script && pd->pet.intimate >= battle_config.pet_equip_min_friendly )
			run_script(pd->petDB->equip_script,0,sd->bl.id,0);
		if( pd && pd->pet.intimate > 0 && (!battle_config.pet_equip_required || pd->pet.equip > 0) && pd->state.skillbonus == 1 && pd->bonus )
			iPc->bonus(sd,pd->bonus->type, pd->bonus->val);
	}

	//param_bonus now holds card bonuses.
	if(status->rhw.range < 1) status->rhw.range = 1;
	if(status->lhw.range < 1) status->lhw.range = 1;
	if(status->rhw.range < status->lhw.range)
		status->rhw.range = status->lhw.range;

	sd->bonus.double_rate += sd->bonus.double_add_rate;
	sd->bonus.perfect_hit += sd->bonus.perfect_hit_add;
	sd->bonus.splash_range += sd->bonus.splash_add_range;

	// Damage modifiers from weapon type
	sd->right_weapon.atkmods[0] = atkmods[0][sd->weapontype1];
	sd->right_weapon.atkmods[1] = atkmods[1][sd->weapontype1];
	sd->right_weapon.atkmods[2] = atkmods[2][sd->weapontype1];
	sd->left_weapon.atkmods[0] = atkmods[0][sd->weapontype2];
	sd->left_weapon.atkmods[1] = atkmods[1][sd->weapontype2];
	sd->left_weapon.atkmods[2] = atkmods[2][sd->weapontype2];

	if(pc_isriding(sd) &&
		(sd->status.weapon==W_1HSPEAR || sd->status.weapon==W_2HSPEAR))
	{	//When Riding with spear, damage modifier to mid-class becomes
		//same as versus large size.
		sd->right_weapon.atkmods[1] = sd->right_weapon.atkmods[2];
		sd->left_weapon.atkmods[1] = sd->left_weapon.atkmods[2];
	}

// ----- STATS CALCULATION -----

	// Job bonuses
	index = iPc->class2idx(sd->status.class_);
	for(i=0;i<(int)sd->status.job_level && i<MAX_LEVEL;i++){
		if(!job_bonus[index][i])
			continue;
		switch(job_bonus[index][i]) {
			case 1: status->str++; break;
			case 2: status->agi++; break;
			case 3: status->vit++; break;
			case 4: status->int_++; break;
			case 5: status->dex++; break;
			case 6: status->luk++; break;
		}
	}

	// If a Super Novice has never died and is at least joblv 70, he gets all stats +10
	if((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->die_counter == 0 && sd->status.job_level >= 70){
		status->str += 10;
		status->agi += 10;
		status->vit += 10;
		status->int_+= 10;
		status->dex += 10;
		status->luk += 10;
	}

	// Absolute modifiers from passive skills
	if(iPc->checkskill(sd,BS_HILTBINDING)>0)
		status->str++;
	if((skill=iPc->checkskill(sd,SA_DRAGONOLOGY))>0)
		status->int_ += (skill+1)/2; // +1 INT / 2 lv
	if((skill=iPc->checkskill(sd,AC_OWL))>0)
		status->dex += skill;
	if((skill = iPc->checkskill(sd,RA_RESEARCHTRAP))>0)
		status->int_ += skill;

	// Bonuses from cards and equipment as well as base stat, remember to avoid overflows.
	i = status->str + sd->status.str + sd->param_bonus[0] + sd->param_equip[0];
	status->str = cap_value(i,0,USHRT_MAX);
	i = status->agi + sd->status.agi + sd->param_bonus[1] + sd->param_equip[1];
	status->agi = cap_value(i,0,USHRT_MAX);
	i = status->vit + sd->status.vit + sd->param_bonus[2] + sd->param_equip[2];
	status->vit = cap_value(i,0,USHRT_MAX);
	i = status->int_+ sd->status.int_+ sd->param_bonus[3] + sd->param_equip[3];
	status->int_ = cap_value(i,0,USHRT_MAX);
	i = status->dex + sd->status.dex + sd->param_bonus[4] + sd->param_equip[4];
	status->dex = cap_value(i,0,USHRT_MAX);
	i = status->luk + sd->status.luk + sd->param_bonus[5] + sd->param_equip[5];
	status->luk = cap_value(i,0,USHRT_MAX);

// ------ BASE ATTACK CALCULATION ------

	// Base batk value is set on status_calc_misc
	// weapon-type bonus (FIXME: Why is the weapon_atk bonus applied to base attack?)
	if (sd->status.weapon < MAX_WEAPON_TYPE && sd->weapon_atk[sd->status.weapon])
		status->batk += sd->weapon_atk[sd->status.weapon];
	// Absolute modifiers from passive skills
	if((skill=iPc->checkskill(sd,BS_HILTBINDING))>0)
		status->batk += 4;

// ----- HP MAX CALCULATION -----

	// Basic MaxHP value
	//We hold the standard Max HP here to make it faster to recalculate on vit changes.
	sd->status.max_hp = status_base_pc_maxhp(sd,status);
	//This is done to handle underflows from negative Max HP bonuses
	i64 = sd->status.max_hp + (int)status->max_hp;
	status->max_hp = (unsigned int)cap_value(i64, 0, INT_MAX);

	// Absolute modifiers from passive skills
	if((skill=iPc->checkskill(sd,CR_TRUST))>0)
		status->max_hp += skill*200;

	// Apply relative modifiers from equipment
	if(sd->hprate < 0)
		sd->hprate = 0;
	if(sd->hprate!=100)
		status->max_hp = (int64)status->max_hp * sd->hprate/100;
	if(battle_config.hp_rate != 100)
		status->max_hp = (int64)status->max_hp * battle_config.hp_rate/100;

	if(status->max_hp > (unsigned int)battle_config.max_hp)
		status->max_hp = battle_config.max_hp;
	else if(!status->max_hp)
		status->max_hp = 1;

// ----- SP MAX CALCULATION -----

	// Basic MaxSP value
	sd->status.max_sp = status_base_pc_maxsp(sd,status);
	//This is done to handle underflows from negative Max SP bonuses
	i64 = sd->status.max_sp + (int)status->max_sp;
	status->max_sp = (unsigned int)cap_value(i64, 0, INT_MAX);

	// Absolute modifiers from passive skills
	if((skill=iPc->checkskill(sd,SL_KAINA))>0)
		status->max_sp += 30*skill;
	if((skill=iPc->checkskill(sd,HP_MEDITATIO))>0)
		status->max_sp += (int64)status->max_sp * skill/100;
	if((skill=iPc->checkskill(sd,HW_SOULDRAIN))>0)
		status->max_sp += (int64)status->max_sp * 2*skill/100;
	if( (skill = iPc->checkskill(sd,RA_RESEARCHTRAP)) > 0 )
		status->max_sp += 200 + 20 * skill;
	if( (skill = iPc->checkskill(sd,WM_LESSON)) > 0 )
		status->max_sp += 30 * skill;


	// Apply relative modifiers from equipment
	if(sd->sprate < 0)
		sd->sprate = 0;
	if(sd->sprate!=100)
		status->max_sp = (int64)status->max_sp * sd->sprate/100;
	if(battle_config.sp_rate != 100)
		status->max_sp = (int64)status->max_sp * battle_config.sp_rate/100;

	if(status->max_sp > (unsigned int)battle_config.max_sp)
		status->max_sp = battle_config.max_sp;
	else if(!status->max_sp)
		status->max_sp = 1;

// ----- RESPAWN HP/SP -----
//
	//Calc respawn hp and store it on base_status
	if (sd->special_state.restart_full_recover)
	{
		status->hp = status->max_hp;
		status->sp = status->max_sp;
	} else {
		if((sd->class_&MAPID_BASEMASK) == MAPID_NOVICE && !(sd->class_&JOBL_2)
			&& battle_config.restart_hp_rate < 50)
			status->hp = status->max_hp>>1;
		else
			status->hp = (int64)status->max_hp * battle_config.restart_hp_rate/100;
		if(!status->hp)
			status->hp = 1;

		status->sp = (int64)status->max_sp * battle_config.restart_sp_rate /100;

		if( !status->sp ) /* the minimum for the respawn setting is SP:1 */
			status->sp = 1;
	}

// ----- MISC CALCULATION -----
	status_calc_misc(&sd->bl, status, sd->status.base_level);

	//Equipment modifiers for misc settings
	if(sd->matk_rate < 0)
		sd->matk_rate = 0;

	if(sd->matk_rate != 100){
		status->matk_max = status->matk_max * sd->matk_rate/100;
		status->matk_min = status->matk_min * sd->matk_rate/100;
	}

	if(sd->hit_rate < 0)
		sd->hit_rate = 0;
	if(sd->hit_rate != 100)
		status->hit = status->hit * sd->hit_rate/100;

	if(sd->flee_rate < 0)
		sd->flee_rate = 0;
	if(sd->flee_rate != 100)
		status->flee = status->flee * sd->flee_rate/100;

	if(sd->def2_rate < 0)
		sd->def2_rate = 0;
	if(sd->def2_rate != 100)
		status->def2 = status->def2 * sd->def2_rate/100;

	if(sd->mdef2_rate < 0)
		sd->mdef2_rate = 0;
	if(sd->mdef2_rate != 100)
		status->mdef2 = status->mdef2 * sd->mdef2_rate/100;

	if(sd->critical_rate < 0)
		sd->critical_rate = 0;
	if(sd->critical_rate != 100)
		status->cri = status->cri * sd->critical_rate/100;

	if(sd->flee2_rate < 0)
		sd->flee2_rate = 0;
	if(sd->flee2_rate != 100)
		status->flee2 = status->flee2 * sd->flee2_rate/100;

// ----- HIT CALCULATION -----

	// Absolute modifiers from passive skills
	if((skill=iPc->checkskill(sd,BS_WEAPONRESEARCH))>0)
		status->hit += skill*2;
	if((skill=iPc->checkskill(sd,AC_VULTURE))>0){
#ifndef RENEWAL
		status->hit += skill;
#endif
		if(sd->status.weapon == W_BOW)
			status->rhw.range += skill;
	}
	if(sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE)
  	{
		if((skill=iPc->checkskill(sd,GS_SINGLEACTION))>0)
			status->hit += 2*skill;
		if((skill=iPc->checkskill(sd,GS_SNAKEEYE))>0) {
			status->hit += skill;
			status->rhw.range += skill;
		}
	}

// ----- FLEE CALCULATION -----

	// Absolute modifiers from passive skills
	if((skill=iPc->checkskill(sd,TF_MISS))>0)
		status->flee += skill*(sd->class_&JOBL_2 && (sd->class_&MAPID_BASEMASK) == MAPID_THIEF? 4 : 3);
	if((skill=iPc->checkskill(sd,MO_DODGE))>0)
		status->flee += (skill*3)>>1;
// ----- EQUIPMENT-DEF CALCULATION -----

	// Apply relative modifiers from equipment
	if(sd->def_rate < 0)
		sd->def_rate = 0;
	if(sd->def_rate != 100) {
		i =  status->def * sd->def_rate/100;
		status->def = cap_value(i, DEFTYPE_MIN, DEFTYPE_MAX);
	}

#ifndef RENEWAL
	if (!battle_config.weapon_defense_type && status->def > battle_config.max_def)
	{
		status->def2 += battle_config.over_def_bonus*(status->def -battle_config.max_def);
		status->def = (unsigned char)battle_config.max_def;
	}
#endif

// ----- EQUIPMENT-MDEF CALCULATION -----

	// Apply relative modifiers from equipment
	if(sd->mdef_rate < 0)
		sd->mdef_rate = 0;
	if(sd->mdef_rate != 100) {
		i =  status->mdef * sd->mdef_rate/100;
		status->mdef = cap_value(i, DEFTYPE_MIN, DEFTYPE_MAX);
	}

#ifndef RENEWAL
	if (!battle_config.magic_defense_type && status->mdef > battle_config.max_def)
	{
		status->mdef2 += battle_config.over_def_bonus*(status->mdef -battle_config.max_def);
		status->mdef = (signed char)battle_config.max_def;
	}
#endif

// ----- ASPD CALCULATION -----
// Unlike other stats, ASPD rate modifiers from skills/SCs/items/etc are first all added together, then the final modifier is applied

	// Basic ASPD value
	i = status_base_amotion_pc(sd,status);
	status->amotion = cap_value(i,((sd->class_&JOBL_THIRD) ? battle_config.max_third_aspd : battle_config.max_aspd),2000);

	// Relative modifiers from passive skills
#ifndef RENEWAL_ASPD
	if((skill=iPc->checkskill(sd,SA_ADVANCEDBOOK))>0 && sd->status.weapon == W_BOOK)
		status->aspd_rate -= 5*skill;
	if((skill = iPc->checkskill(sd,SG_DEVIL)) > 0 && !iPc->nextjobexp(sd))
		status->aspd_rate -= 30*skill;
	if((skill=iPc->checkskill(sd,GS_SINGLEACTION))>0 &&
		(sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE))
		status->aspd_rate -= ((skill+1)/2) * 10;
	if(pc_isriding(sd))
		status->aspd_rate += 500-100*iPc->checkskill(sd,KN_CAVALIERMASTERY);
	else if(pc_isridingdragon(sd))
		status->aspd_rate += 250-50*iPc->checkskill(sd,RK_DRAGONTRAINING);
#else // needs more info
	if((skill=iPc->checkskill(sd,SA_ADVANCEDBOOK))>0 && sd->status.weapon == W_BOOK)
		status->aspd_rate += 5*skill;
	if((skill = iPc->checkskill(sd,SG_DEVIL)) > 0 && !iPc->nextjobexp(sd))
		status->aspd_rate += 30*skill;
	if((skill=iPc->checkskill(sd,GS_SINGLEACTION))>0 &&
		(sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE))
		status->aspd_rate += ((skill+1)/2) * 10;
	if(pc_isriding(sd))
		status->aspd_rate -= 500-100*iPc->checkskill(sd,KN_CAVALIERMASTERY);
	else if(pc_isridingdragon(sd))
		status->aspd_rate -= 250-50*iPc->checkskill(sd,RK_DRAGONTRAINING);
#endif
	status->adelay = 2*status->amotion;


// ----- DMOTION -----
//
	i =  800-status->agi*4;
	status->dmotion = cap_value(i, 400, 800);
	if(battle_config.pc_damage_delay_rate != 100)
		status->dmotion = status->dmotion*battle_config.pc_damage_delay_rate/100;

// ----- MISC CALCULATIONS -----

	// Weight
	if((skill=iPc->checkskill(sd,MC_INCCARRY))>0)
		sd->max_weight += 2000*skill;
	if(pc_isriding(sd) && iPc->checkskill(sd,KN_RIDING)>0)
		sd->max_weight += 10000;
	else if(pc_isridingdragon(sd))
		sd->max_weight += 5000+2000*iPc->checkskill(sd,RK_DRAGONTRAINING);
	if(sc->data[SC_KNOWLEDGE])
		sd->max_weight += sd->max_weight*sc->data[SC_KNOWLEDGE]->val1/10;
	if((skill=iPc->checkskill(sd,ALL_INCCARRY))>0)
		sd->max_weight += 2000*skill;

	sd->cart_weight_max = battle_config.max_cart_weight + (iPc->checkskill(sd, GN_REMODELING_CART)*5000);

	if (iPc->checkskill(sd,SM_MOVINGRECOVERY)>0)
		sd->regen.state.walk = 1;
	else
		sd->regen.state.walk = 0;

	// Skill SP cost
	if((skill=iPc->checkskill(sd,HP_MANARECHARGE))>0 )
		sd->dsprate -= 4*skill;

	if(sc->data[SC_SERVICE4U])
		sd->dsprate -= sc->data[SC_SERVICE4U]->val3;

	if(sc->data[SC_SPCOST_RATE])
		sd->dsprate -= sc->data[SC_SPCOST_RATE]->val1;

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
	if((skill=iPc->checkskill(sd,CR_TRUST))>0)
		sd->subele[ELE_HOLY] += skill*5;
	if((skill=iPc->checkskill(sd,BS_SKINTEMPER))>0) {
		sd->subele[ELE_NEUTRAL] += skill;
		sd->subele[ELE_FIRE] += skill*4;
	}
	if((skill=iPc->checkskill(sd,NC_RESEARCHFE))>0) {
		sd->subele[ELE_EARTH] += skill*10;
		sd->subele[ELE_FIRE] += skill*10;
	}
	if((skill=iPc->checkskill(sd,SA_DRAGONOLOGY))>0 ){
		skill = skill*4;
		sd->right_weapon.addrace[RC_DRAGON]+=skill;
		sd->left_weapon.addrace[RC_DRAGON]+=skill;
		sd->magic_addrace[RC_DRAGON]+=skill;
		sd->subrace[RC_DRAGON]+=skill;
	}

	if(sc->count){
     	if(sc->data[SC_CONCENTRATE]) { //Update the card-bonus data
			sc->data[SC_CONCENTRATE]->val3 = sd->param_bonus[1]; //Agi
			sc->data[SC_CONCENTRATE]->val4 = sd->param_bonus[4]; //Dex
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
		if(sc->data[SC_ARMOR_ELEMENT]) {	//This status change should grant card-type elemental resist.
			sd->subele[ELE_WATER] += sc->data[SC_ARMOR_ELEMENT]->val1;
			sd->subele[ELE_EARTH] += sc->data[SC_ARMOR_ELEMENT]->val2;
			sd->subele[ELE_FIRE] += sc->data[SC_ARMOR_ELEMENT]->val3;
			sd->subele[ELE_WIND] += sc->data[SC_ARMOR_ELEMENT]->val4;
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
		if( sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 3 )
			sd->magic_addele[ELE_FIRE] += 25;
		if( sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 3 )
			sd->magic_addele[ELE_WATER] += 25;
		if( sc->data[SC_WIND_INSIGNIA] && sc->data[SC_WIND_INSIGNIA]->val1 == 3 )
			sd->magic_addele[ELE_WIND] += 25;
		if( sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 3 )
			sd->magic_addele[ELE_EARTH] += 25;
	}
	status_cpy(&sd->battle_status, status);

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
		iPc->updateweightstatus(sd);
	}
	if( b_cart_weight_max != sd->cart_weight_max ) {
		clif->updatestatus(sd,SP_CARTINFO);
	}

	calculating = 0;

	return 0;
}

int status_calc_mercenary_(struct mercenary_data *md, bool first)
{
	struct status_data *status = &md->base_status;
	struct s_mercenary *merc = &md->mercenary;

	if( first )
	{
		memcpy(status, &md->db->status, sizeof(struct status_data));
		status->mode = MD_CANMOVE|MD_CANATTACK;
		status->hp = status->max_hp;
		status->sp = status->max_sp;
		md->battle_status.hp = merc->hp;
		md->battle_status.sp = merc->sp;
	}

	status_calc_misc(&md->bl, status, md->db->lv);
	status_cpy(&md->battle_status, status);

	return 0;
}

int status_calc_homunculus_(struct homun_data *hd, bool first)
{
	struct status_data *status = &hd->base_status;
	struct s_homunculus *hom = &hd->homunculus;
	int skill;
	int amotion;

	status->str = hom->str / 10;
	status->agi = hom->agi / 10;
	status->vit = hom->vit / 10;
	status->dex = hom->dex / 10;
	status->int_ = hom->int_ / 10;
	status->luk = hom->luk / 10;

	if (first) {	//[orn]
		const struct s_homunculus_db *db = hd->homunculusDB;
		status->def_ele =  db->element;
		status->ele_lv = 1;
		status->race = db->race;
		status->size = (hom->class_ == db->evo_class)?db->evo_size:db->base_size;
		status->rhw.range = 1 + status->size;
		status->mode = MD_CANMOVE|MD_CANATTACK;
		status->speed = DEFAULT_WALK_SPEED;
		if (battle_config.hom_setting&0x8 && hd->master)
			status->speed = status_get_speed(&hd->master->bl);

		status->hp = 1;
		status->sp = 1;
	}
	skill = hom->level/10 + status->vit/5;
	status->def = cap_value(skill, 0, 99);

	skill = hom->level/10 + status->int_/5;
	status->mdef = cap_value(skill, 0, 99);

	status->max_hp = hom->max_hp ;
	status->max_sp = hom->max_sp ;

	homun->calc_skilltree(hd, 0);

	if((skill=homun->checkskill(hd,HAMI_SKIN)) > 0)
		status->def +=	skill * 4;

	if((skill = homun->checkskill(hd,HVAN_INSTRUCT)) > 0) {
		status->int_ += 1 +skill/2 +skill/4 +skill/5;
		status->str  += 1 +skill/3 +skill/3 +skill/4;
	}

	if((skill=homun->checkskill(hd,HAMI_SKIN)) > 0)
		status->max_hp += skill * 2 * status->max_hp / 100;

	if((skill = homun->checkskill(hd,HLIF_BRAIN)) > 0)
		status->max_sp += (1 +skill/2 -skill/4 +skill/5) * status->max_sp / 100 ;

	if (first) {
		hd->battle_status.hp = hom->hp ;
		hd->battle_status.sp = hom->sp ;
	}

	status->rhw.atk = status->dex;
	status->rhw.atk2 = status->str + hom->level;

	status->aspd_rate = 1000;

	amotion = (1000 -4*status->agi -status->dex) * hd->homunculusDB->baseASPD/1000;
	status->amotion = cap_value(amotion,battle_config.max_aspd,2000);
	status->adelay = status->amotion; //It seems adelay = amotion for Homunculus.

	status_calc_misc(&hd->bl, status, hom->level);

#ifdef RENEWAL
	status->matk_max = status->matk_min;
#endif

	status_cpy(&hd->battle_status, status);
	return 1;
}

int status_calc_elemental_(struct elemental_data *ed, bool first) {
	struct status_data *status = &ed->base_status;
	struct s_elemental *ele = &ed->elemental;
	struct map_session_data *sd = ed->master;

	if( !sd )
		return 0;

	if( first ) {
		memcpy(status, &ed->db->status, sizeof(struct status_data));
		if( !ele->mode )
			status->mode = EL_MODE_PASSIVE;
		else
			status->mode = ele->mode;

		status_calc_misc(&ed->bl, status, 0);

		status->max_hp = ele->max_hp;
		status->max_sp = ele->max_sp;
		status->hp = ele->hp;
		status->sp = ele->sp;
		status->rhw.atk = ele->atk;
		status->rhw.atk2 = ele->atk2;

		status->matk_min += ele->matk;
		status->def += ele->def;
		status->mdef += ele->mdef;
		status->flee = ele->flee;
		status->hit = ele->hit;

		memcpy(&ed->battle_status,status,sizeof(struct status_data));
	} else {
		status_calc_misc(&ed->bl, status, 0);
		status_cpy(&ed->battle_status, status);
	}

	return 0;
}

int status_calc_npc_(struct npc_data *nd, bool first) {
	struct status_data *status = &nd->status;

	if (!nd)
		return 0;

	if (first) {
		status->hp = 1;
		status->sp = 1;
		status->max_hp = 1;
		status->max_sp = 1;

		status->def_ele = ELE_NEUTRAL;
		status->ele_lv = 1;
		status->race = RC_DEMIHUMAN;
		status->size = nd->size;
		status->rhw.range = 1 + status->size;
		status->mode = (MD_CANMOVE|MD_CANATTACK);
		status->speed = nd->speed;
	}

	status->str = nd->stat_point;
	status->agi = nd->stat_point;
	status->vit = nd->stat_point;
	status->int_= nd->stat_point;
	status->dex = nd->stat_point;
	status->luk = nd->stat_point;

	status_calc_misc(&nd->bl, status, nd->level);
	status_cpy(&nd->status, status);

	return 0;
}

static unsigned short status_calc_str(struct block_list *,struct status_change *,int);
static unsigned short status_calc_agi(struct block_list *,struct status_change *,int);
static unsigned short status_calc_vit(struct block_list *,struct status_change *,int);
static unsigned short status_calc_int(struct block_list *,struct status_change *,int);
static unsigned short status_calc_dex(struct block_list *,struct status_change *,int);
static unsigned short status_calc_luk(struct block_list *,struct status_change *,int);
static unsigned short status_calc_batk(struct block_list *,struct status_change *,int);
static unsigned short status_calc_watk(struct block_list *,struct status_change *,int);
static unsigned short status_calc_matk(struct block_list *,struct status_change *,int);
static signed short status_calc_hit(struct block_list *,struct status_change *,int);
static signed short status_calc_critical(struct block_list *,struct status_change *,int);
static signed short status_calc_flee(struct block_list *,struct status_change *,int);
static signed short status_calc_flee2(struct block_list *,struct status_change *,int);
static defType status_calc_def(struct block_list *bl, struct status_change *sc, int);
static signed short status_calc_def2(struct block_list *,struct status_change *,int);
static defType status_calc_mdef(struct block_list *bl, struct status_change *sc, int);
static signed short status_calc_mdef2(struct block_list *,struct status_change *,int);
static unsigned short status_calc_speed(struct block_list *,struct status_change *,int);
static short status_calc_aspd_rate(struct block_list *,struct status_change *,int);
static unsigned short status_calc_dmotion(struct block_list *bl, struct status_change *sc, int dmotion);
#ifdef RENEWAL_ASPD
static short status_calc_aspd(struct block_list *bl, struct status_change *sc, short flag);
#endif
static short status_calc_fix_aspd(struct block_list *bl, struct status_change *sc, int);
static unsigned int status_calc_maxhp(struct block_list *,struct status_change *, uint64);
static unsigned int status_calc_maxsp(struct block_list *,struct status_change *,unsigned int);
static unsigned char status_calc_element(struct block_list *bl, struct status_change *sc, int element);
static unsigned char status_calc_element_lv(struct block_list *bl, struct status_change *sc, int lv);
static unsigned short status_calc_mode(struct block_list *bl, struct status_change *sc, int mode);
#ifdef RENEWAL
static unsigned short status_calc_ematk(struct block_list *,struct status_change *,int);
#endif

//Calculates base regen values.
void status_calc_regen(struct block_list *bl, struct status_data *status, struct regen_data *regen)
{
	struct map_session_data *sd;
	int val, skill, reg_flag;

	if( !(bl->type&BL_REGEN) || !regen )
		return;

	sd = BL_CAST(BL_PC,bl);
	val = 1 + (status->vit/5) + (status->max_hp/200);

	if( sd && sd->hprecov_rate != 100 )
		val = val*sd->hprecov_rate/100;

	reg_flag = bl->type == BL_PC ? 0 : 1;

	regen->hp = cap_value(val, reg_flag, SHRT_MAX);

	val = 1 + (status->int_/6) + (status->max_sp/100);
	if( status->int_ >= 120 )
		val += ((status->int_-120)>>1) + 4;

	if( sd && sd->sprecov_rate != 100 )
		val = val*sd->sprecov_rate/100;

	regen->sp = cap_value(val, reg_flag, SHRT_MAX);

	if( sd )
	{
		struct regen_data_sub *sregen;
		if( (skill=iPc->checkskill(sd,HP_MEDITATIO)) > 0 )
		{
			val = regen->sp*(100+3*skill)/100;
			regen->sp = cap_value(val, 1, SHRT_MAX);
		}
		//Only players have skill/sitting skill regen for now.
		sregen = regen->sregen;

		val = 0;
		if( (skill=iPc->checkskill(sd,SM_RECOVERY)) > 0 )
			val += skill*5 + skill*status->max_hp/500;
		sregen->hp = cap_value(val, 0, SHRT_MAX);

		val = 0;
		if( (skill=iPc->checkskill(sd,MG_SRECOVERY)) > 0 )
			val += skill*3 + skill*status->max_sp/500;
		if( (skill=iPc->checkskill(sd,NJ_NINPOU)) > 0 )
			val += skill*3 + skill*status->max_sp/500;
		if( (skill=iPc->checkskill(sd,WM_LESSON)) > 0 )
			val += 3 + 3 * skill;

		sregen->sp = cap_value(val, 0, SHRT_MAX);

		// Skill-related recovery (only when sit)
		sregen = regen->ssregen;

		val = 0;
		if( (skill=iPc->checkskill(sd,MO_SPIRITSRECOVERY)) > 0 )
			val += skill*4 + skill*status->max_hp/500;

		if( (skill=iPc->checkskill(sd,TK_HPTIME)) > 0 && sd->state.rest )
			val += skill*30 + skill*status->max_hp/500;
		sregen->hp = cap_value(val, 0, SHRT_MAX);

		val = 0;
		if( (skill=iPc->checkskill(sd,TK_SPTIME)) > 0 && sd->state.rest )
		{
			val += skill*3 + skill*status->max_sp/500;
			if ((skill=iPc->checkskill(sd,SL_KAINA)) > 0) //Power up Enjoyable Rest
				val += (30+10*skill)*val/100;
		}
		if( (skill=iPc->checkskill(sd,MO_SPIRITSRECOVERY)) > 0 )
			val += skill*2 + skill*status->max_sp/500;
		sregen->sp = cap_value(val, 0, SHRT_MAX);
	}

	if( bl->type == BL_HOM ) {
		struct homun_data *hd = (TBL_HOM*)bl;
		if( (skill = homun->checkskill(hd,HAMI_SKIN)) > 0 ) {
			val = regen->hp*(100+5*skill)/100;
			regen->hp = cap_value(val, 1, SHRT_MAX);
		}
		if( (skill = homun->checkskill(hd,HLIF_BRAIN)) > 0 ) {
			val = regen->sp*(100+3*skill)/100;
			regen->sp = cap_value(val, 1, SHRT_MAX);
		}
	} else if( bl->type == BL_MER ) {
		val = (status->max_hp * status->vit / 10000 + 1) * 6;
		regen->hp = cap_value(val, 1, SHRT_MAX);

		val = (status->max_sp * (status->int_ + 10) / 750) + 1;
		regen->sp = cap_value(val, 1, SHRT_MAX);
	} else if( bl->type == BL_ELEM ) {
		val = (status->max_hp * status->vit / 10000 + 1) * 6;
		regen->hp = cap_value(val, 1, SHRT_MAX);

		val = (status->max_sp * (status->int_ + 10) / 750) + 1;
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

	if (
		(sc->data[SC_POISON] && !sc->data[SC_SLOWPOISON])
		|| (sc->data[SC_DPOISON] && !sc->data[SC_SLOWPOISON])
		|| sc->data[SC_BERSERK] || sc->data[SC__BLOODYLUST]
		|| sc->data[SC_TRICKDEAD]
		|| sc->data[SC_BLEEDING]
		|| sc->data[SC_MAGICMUSHROOM]
		|| sc->data[SC_RAISINGDRAGON]
		|| sc->data[SC_SATURDAYNIGHTFEVER]
	)	//No regen
		regen->flag = 0;

	if (
		sc->data[SC_DANCING] || sc->data[SC_OBLIVIONCURSE] || sc->data[SC_MAXIMIZEPOWER]
		|| (
			(bl->type == BL_PC && ((TBL_PC*)bl)->class_&MAPID_UPPERMASK) == MAPID_MONK &&
			(sc->data[SC_EXTREMITYFIST] || (sc->data[SC_EXPLOSIONSPIRITS] && (!sc->data[SC_SPIRIT] || sc->data[SC_SPIRIT]->val2 != SL_MONK)))
			)
	)	//No natural SP regen
		regen->flag &=~RGN_SP;

	if(
		sc->data[SC_TENSIONRELAX]
	  ) {
		regen->rate.hp += 2;
		if (regen->sregen)
			regen->sregen->rate.hp += 3;
	}
	if (sc->data[SC_MAGNIFICAT])
	{
		regen->rate.hp += 1;
		regen->rate.sp += 1;
	}
	if (sc->data[SC_REGENERATION])
	{
		const struct status_change_entry *sce = sc->data[SC_REGENERATION];
		if (!sce->val4)
		{
			regen->rate.hp += sce->val2;
			regen->rate.sp += sce->val3;
		} else
			regen->flag&=~sce->val4; //Remove regen as specified by val4
	}
	if(sc->data[SC_GT_REVITALIZE]){
		regen->hp = cap_value(regen->hp*sc->data[SC_GT_REVITALIZE]->val3/100, 1, SHRT_MAX);
		regen->state.walk= 1;
	}
	 if ((sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 1) //if insignia lvl 1
	        || (sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 1)
	        || (sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 1)
	        || (sc->data[SC_WIND_INSIGNIA] && sc->data[SC_WIND_INSIGNIA]->val1 == 1))
	    regen->rate.hp *= 2;

}
/// Recalculates parts of an object's battle status according to the specified flags.
/// @param flag bitfield of values from enum scb_flag
void status_calc_bl_main(struct block_list *bl, /*enum scb_flag*/int flag)
{
	const struct status_data *b_status = status_get_base_status(bl);
	struct status_data *status = status_get_status_data(bl);
	struct status_change *sc = status_get_sc(bl);
	TBL_PC *sd = BL_CAST(BL_PC,bl);
	int temp;

	if (!b_status || !status)
		return;

	if((!(bl->type&BL_REGEN)) && (!sc || !sc->count)) { //No difference.
		status_cpy(status, b_status);
		return;
	}

	if(flag&SCB_STR) {
		status->str = status_calc_str(bl, sc, b_status->str);
		flag|=SCB_BATK;
		if( bl->type&BL_HOM )
			flag |= SCB_WATK;
	}

	if(flag&SCB_AGI) {
		status->agi = status_calc_agi(bl, sc, b_status->agi);
		flag|=SCB_FLEE
#ifdef RENEWAL
			|SCB_DEF2
#endif
			;
		if( bl->type&(BL_PC|BL_HOM) )
			flag |= SCB_ASPD|SCB_DSPD;
	}

	if(flag&SCB_VIT) {
		status->vit = status_calc_vit(bl, sc, b_status->vit);
		flag|=SCB_DEF2|SCB_MDEF2;
		if( bl->type&(BL_PC|BL_HOM|BL_MER|BL_ELEM) )
			flag |= SCB_MAXHP;
		if( bl->type&BL_HOM )
			flag |= SCB_DEF;
	}

	if(flag&SCB_INT) {
		status->int_ = status_calc_int(bl, sc, b_status->int_);
		flag|=SCB_MATK|SCB_MDEF2;
		if( bl->type&(BL_PC|BL_HOM|BL_MER|BL_ELEM) )
			flag |= SCB_MAXSP;
		if( bl->type&BL_HOM )
			flag |= SCB_MDEF;
	}

	if(flag&SCB_DEX) {
		status->dex = status_calc_dex(bl, sc, b_status->dex);
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
		status->luk = status_calc_luk(bl, sc, b_status->luk);
		flag|=SCB_BATK|SCB_CRI|SCB_FLEE2
#ifdef RENEWAL
			|SCB_MATK|SCB_HIT|SCB_FLEE
#endif
			;
	}

	if(flag&SCB_BATK && b_status->batk) {
		status->batk = status_base_atk(bl,status);
		temp = b_status->batk - status_base_atk(bl,b_status);
		if (temp)
		{
			temp += status->batk;
			status->batk = cap_value(temp, 0, USHRT_MAX);
		}
		status->batk = status_calc_batk(bl, sc, status->batk);
	}

	if(flag&SCB_WATK) {

		status->rhw.atk = status_calc_watk(bl, sc, b_status->rhw.atk);
		if (!sd) //Should not affect weapon refine bonus
			status->rhw.atk2 = status_calc_watk(bl, sc, b_status->rhw.atk2);

		if(b_status->lhw.atk) {
			if (sd) {
				sd->state.lr_flag = 1;
				status->lhw.atk = status_calc_watk(bl, sc, b_status->lhw.atk);
				sd->state.lr_flag = 0;
			} else {
				status->lhw.atk = status_calc_watk(bl, sc, b_status->lhw.atk);
				status->lhw.atk2= status_calc_watk(bl, sc, b_status->lhw.atk2);
			}
		}

		if( bl->type&BL_HOM )
		{
			status->rhw.atk += (status->dex - b_status->dex);
			status->rhw.atk2 += (status->str - b_status->str);
			if( status->rhw.atk2 < status->rhw.atk )
				status->rhw.atk2 = status->rhw.atk;
		}
	}

	if(flag&SCB_HIT) {
		if (status->dex == b_status->dex
#ifdef RENEWAL
			&& status->luk == b_status->luk
#endif
			)
			status->hit = status_calc_hit(bl, sc, b_status->hit);
		else
			status->hit = status_calc_hit(bl, sc, b_status->hit + (status->dex - b_status->dex)
#ifdef RENEWAL
			 + (status->luk/3 - b_status->luk/3)
#endif
			 );
	}

	if(flag&SCB_FLEE) {
		if (status->agi == b_status->agi
#ifdef RENEWAL
			&& status->luk == b_status->luk
#endif
			)
			status->flee = status_calc_flee(bl, sc, b_status->flee);
		else
			status->flee = status_calc_flee(bl, sc, b_status->flee +(status->agi - b_status->agi)
#ifdef RENEWAL
			+ (status->luk/5 - b_status->luk/5)
#endif
			);
	}

	if(flag&SCB_DEF)
	{
		status->def = status_calc_def(bl, sc, b_status->def);

		if( bl->type&BL_HOM )
			status->def += (status->vit/5 - b_status->vit/5);
	}

	if(flag&SCB_DEF2) {
		if (status->vit == b_status->vit
#ifdef RENEWAL
			&& status->agi == b_status->agi
#endif
			)
			status->def2 = status_calc_def2(bl, sc, b_status->def2);
		else
			status->def2 = status_calc_def2(bl, sc, b_status->def2
#ifdef RENEWAL
			+ (int)( ((float)status->vit/2 - (float)b_status->vit/2) + ((float)status->agi/5 - (float)b_status->agi/5) )
#else
			+ (status->vit - b_status->vit)
#endif
		);
	}

	if(flag&SCB_MDEF)
	{
		status->mdef = status_calc_mdef(bl, sc, b_status->mdef);

		if( bl->type&BL_HOM )
			status->mdef += (status->int_/5 - b_status->int_/5);
	}

	if(flag&SCB_MDEF2) {
		if (status->int_ == b_status->int_ && status->vit == b_status->vit
#ifdef RENEWAL
			&& status->dex == b_status->dex
#endif
			)
			status->mdef2 = status_calc_mdef2(bl, sc, b_status->mdef2);
		else
			status->mdef2 = status_calc_mdef2(bl, sc, b_status->mdef2 +(status->int_ - b_status->int_)
#ifdef RENEWAL
			+ (int)( ((float)status->dex/5 - (float)b_status->dex/5) + ((float)status->vit/5 - (float)b_status->vit/5) )
#else
			+ ((status->vit - b_status->vit)>>1)
#endif
			);
	}

	if(flag&SCB_SPEED) {
		struct unit_data *ud = unit_bl2ud(bl);
		status->speed = status_calc_speed(bl, sc, b_status->speed);

		//Re-walk to adjust speed (we do not check if walktimer != INVALID_TIMER
		//because if you step on something while walking, the moment this
		//piece of code triggers the walk-timer is set on INVALID_TIMER) [Skotlex]
	  	if (ud)
			ud->state.change_walk_target = ud->state.speed_changed = 1;

		if( bl->type&BL_PC && status->speed < battle_config.max_walk_speed )
			status->speed = battle_config.max_walk_speed;

		if( bl->type&BL_HOM && battle_config.hom_setting&0x8 && ((TBL_HOM*)bl)->master)
			status->speed = status_get_speed(&((TBL_HOM*)bl)->master->bl);


	}

	if(flag&SCB_CRI && b_status->cri) {
		if (status->luk == b_status->luk)
			status->cri = status_calc_critical(bl, sc, b_status->cri);
		else
			status->cri = status_calc_critical(bl, sc, b_status->cri + 3*(status->luk - b_status->luk));
		/**
		 * after status_calc_critical so the bonus is applied despite if you have or not a sc bugreport:5240
		 **/
		if( bl->type == BL_PC && ((TBL_PC*)bl)->status.weapon == W_KATAR )
			status->cri <<= 1;

	}

	if(flag&SCB_FLEE2 && b_status->flee2) {
		if (status->luk == b_status->luk)
			status->flee2 = status_calc_flee2(bl, sc, b_status->flee2);
		else
			status->flee2 = status_calc_flee2(bl, sc, b_status->flee2 +(status->luk - b_status->luk));
	}

	if(flag&SCB_ATK_ELE) {
		status->rhw.ele = status_calc_attack_element(bl, sc, b_status->rhw.ele);
		if (sd) sd->state.lr_flag = 1;
		status->lhw.ele = status_calc_attack_element(bl, sc, b_status->lhw.ele);
		if (sd) sd->state.lr_flag = 0;
	}

	if(flag&SCB_DEF_ELE) {
		status->def_ele = status_calc_element(bl, sc, b_status->def_ele);
		status->ele_lv = status_calc_element_lv(bl, sc, b_status->ele_lv);
	}

	if(flag&SCB_MODE)
	{
		status->mode = status_calc_mode(bl, sc, b_status->mode);
		//Since mode changed, reset their state.
		if (!(status->mode&MD_CANATTACK))
			unit_stop_attack(bl);
		if (!(status->mode&MD_CANMOVE))
			unit_stop_walking(bl,1);
	}

// No status changes alter these yet.
//	if(flag&SCB_SIZE)
// if(flag&SCB_RACE)
// if(flag&SCB_RANGE)

	if(flag&SCB_MAXHP) {
		if( bl->type&BL_PC )
		{
			status->max_hp = status_base_pc_maxhp(sd,status);
			status->max_hp += b_status->max_hp - sd->status.max_hp;

			status->max_hp = status_calc_maxhp(bl, sc, status->max_hp);

			if( status->max_hp > (unsigned int)battle_config.max_hp )
				status->max_hp = (unsigned int)battle_config.max_hp;
		}
		else
		{
			status->max_hp = status_calc_maxhp(bl, sc, b_status->max_hp);
		}

		if( status->hp > status->max_hp ) //FIXME: Should perhaps a status_zap should be issued?
		{
			status->hp = status->max_hp;
			if( sd ) clif->updatestatus(sd,SP_HP);
		}
	}

	if(flag&SCB_MAXSP) {
		if( bl->type&BL_PC )
		{
			status->max_sp = status_base_pc_maxsp(sd,status);
			status->max_sp += b_status->max_sp - sd->status.max_sp;

			status->max_sp = status_calc_maxsp(&sd->bl, &sd->sc, status->max_sp);

			if( status->max_sp > (unsigned int)battle_config.max_sp )
				status->max_sp = (unsigned int)battle_config.max_sp;
		}
		else
		{
			status->max_sp = status_calc_maxsp(bl, sc, b_status->max_sp);
		}

		if( status->sp > status->max_sp )
		{
			status->sp = status->max_sp;
			if( sd ) clif->updatestatus(sd,SP_SP);
		}
	}

	if(flag&SCB_MATK) {
#ifndef RENEWAL
		status->matk_min = status_base_matk_min(status) + (sd?sd->bonus.ematk:0);
		status->matk_max = status_base_matk_max(status) + (sd?sd->bonus.ematk:0);
#else
		/**
		 * RE MATK Formula (from irowiki:http://irowiki.org/wiki/MATK)
		 * MATK = (sMATK + wMATK + eMATK) * Multiplicative Modifiers
		 **/
		status->matk_min = status->matk_max = status_base_matk(status, status_get_lv(bl));
		if( bl->type&BL_PC ){
			//  Any +MATK you get from skills and cards, including cards in weapon, is added here.
			if( sd->bonus.ematk > 0 ){
				status->matk_max += sd->bonus.ematk;
				status->matk_min += sd->bonus.ematk;
			}
            status->matk_min = status_calc_ematk(bl, sc, status->matk_min);
            status->matk_max = status_calc_ematk(bl, sc, status->matk_max);
			//This is the only portion in MATK that varies depending on the weapon level and refinement rate.
			if( status->rhw.matk > 0 ){
				int wMatk = status->rhw.matk;
				int variance = wMatk * status->rhw.wlv / 10;
				status->matk_min += wMatk - variance;
				status->matk_max += wMatk + variance;
			}
		}
#endif
        if (bl->type&BL_PC && sd->matk_rate != 100) {
			status->matk_max = status->matk_max * sd->matk_rate/100;
			status->matk_min = status->matk_min * sd->matk_rate/100;
		}

		status->matk_min = status_calc_matk(bl, sc, status->matk_min);
		status->matk_max = status_calc_matk(bl, sc, status->matk_max);

        if ((bl->type&BL_HOM && battle_config.hom_setting&0x20)  //Hom Min Matk is always the same as Max Matk
			|| sc->data[SC_RECOGNIZEDSPELL])
            status->matk_min = status->matk_max;

#ifdef RENEWAL
		if( sd && sd->right_weapon.overrefine > 0){
			status->matk_min++;
			status->matk_max += sd->right_weapon.overrefine - 1;
		}
#endif

	}

	if(flag&SCB_ASPD) {
		int amotion;
		if( bl->type&BL_PC )
		{
			amotion = status_base_amotion_pc(sd,status);
#ifndef RENEWAL_ASPD
			status->aspd_rate = status_calc_aspd_rate(bl, sc, b_status->aspd_rate);

			if(status->aspd_rate != 1000)
				amotion = amotion*status->aspd_rate/1000;
#else
			// aspd = baseaspd + floor(sqrt((agi^2/2) + (dex^2/5))/4 + (potskillbonus*agi/200))
			amotion -= (int)(sqrt( (pow(status->agi, 2) / 2) + (pow(status->dex, 2) / 5) ) / 4 + (status_calc_aspd(bl, sc, 1) * status->agi / 200)) * 10;

			if( (status_calc_aspd(bl, sc, 2) + status->aspd_rate2) != 0 ) // RE ASPD percertage modifier
				amotion -= ( amotion - ((sd->class_&JOBL_THIRD) ? battle_config.max_third_aspd : battle_config.max_aspd) )
							* (status_calc_aspd(bl, sc, 2) + status->aspd_rate2) / 100;

			if(status->aspd_rate != 1000) // absolute percentage modifier
				amotion = ( 200 - (200-amotion/10) * status->aspd_rate / 1000 ) * 10;
#endif
			amotion = status_calc_fix_aspd(bl, sc, amotion);
			status->amotion = cap_value(amotion,((sd->class_&JOBL_THIRD) ? battle_config.max_third_aspd : battle_config.max_aspd),2000);

			status->adelay = 2*status->amotion;
		}
		else
		if( bl->type&BL_HOM )
		{
			amotion = (1000 -4*status->agi -status->dex) * ((TBL_HOM*)bl)->homunculusDB->baseASPD/1000;
			status->aspd_rate = status_calc_aspd_rate(bl, sc, b_status->aspd_rate);

			if(status->aspd_rate != 1000)
				amotion = amotion*status->aspd_rate/1000;

			amotion = status_calc_fix_aspd(bl, sc, amotion);
			status->amotion = cap_value(amotion,battle_config.max_aspd,2000);

			status->adelay = status->amotion;
		}
		else // mercenary and mobs
		{
			amotion = b_status->amotion;
			status->aspd_rate = status_calc_aspd_rate(bl, sc, b_status->aspd_rate);

			if(status->aspd_rate != 1000)
				amotion = amotion*status->aspd_rate/1000;

			amotion = status_calc_fix_aspd(bl, sc, amotion);
			status->amotion = cap_value(amotion, battle_config.monster_max_aspd, 2000);

			temp = b_status->adelay*status->aspd_rate/1000;
			status->adelay = cap_value(temp, battle_config.monster_max_aspd*2, 4000);
		}
	}

	if(flag&SCB_DSPD) {
		int dmotion;
		if( bl->type&BL_PC )
		{
			if (b_status->agi == status->agi)
				status->dmotion = status_calc_dmotion(bl, sc, b_status->dmotion);
			else {
				dmotion = 800-status->agi*4;
				status->dmotion = cap_value(dmotion, 400, 800);
				if(battle_config.pc_damage_delay_rate != 100)
					status->dmotion = status->dmotion*battle_config.pc_damage_delay_rate/100;
				//It's safe to ignore b_status->dmotion since no bonus affects it.
				status->dmotion = status_calc_dmotion(bl, sc, status->dmotion);
			}
		}
		else
		if( bl->type&BL_HOM )
		{
			dmotion = 800-status->agi*4;
			status->dmotion = cap_value(dmotion, 400, 800);
			status->dmotion = status_calc_dmotion(bl, sc, b_status->dmotion);
		}
		else // mercenary and mobs
		{
			status->dmotion = status_calc_dmotion(bl, sc, b_status->dmotion);
		}
	}

	if(flag&(SCB_VIT|SCB_MAXHP|SCB_INT|SCB_MAXSP) && bl->type&BL_REGEN)
		status_calc_regen(bl, status, status_get_regen_data(bl));

	if(flag&SCB_REGEN && bl->type&BL_REGEN)
		status_calc_regen_rate(bl, status_get_regen_data(bl), sc);
}
/// Recalculates parts of an object's base status and battle status according to the specified flags.
/// Also sends updates to the client wherever applicable.
/// @param flag bitfield of values from enum scb_flag
/// @param first if true, will cause status_calc_* functions to run their base status initialization code
void status_calc_bl_(struct block_list* bl, enum scb_flag flag, bool first)
{
	struct status_data b_status; // previous battle status
	struct status_data* status; // pointer to current battle status

	// remember previous values
	status = status_get_status_data(bl);
	memcpy(&b_status, status, sizeof(struct status_data));

	if( flag&SCB_BASE ) {// calculate the object's base status too
		switch( bl->type ) {
		case BL_PC:  status_calc_pc_(BL_CAST(BL_PC,bl), first);          break;
		case BL_MOB: status_calc_mob_(BL_CAST(BL_MOB,bl), first);        break;
		case BL_PET: status_calc_pet_(BL_CAST(BL_PET,bl), first);        break;
		case BL_HOM: status_calc_homunculus_(BL_CAST(BL_HOM,bl), first); break;
		case BL_MER: status_calc_mercenary_(BL_CAST(BL_MER,bl), first);  break;
		case BL_ELEM: status_calc_elemental_(BL_CAST(BL_ELEM,bl), first);  break;
		case BL_NPC: status_calc_npc_(BL_CAST(BL_NPC,bl), first); break;
		}
	}

	if( bl->type == BL_PET )
		return; // pets are not affected by statuses

	if( first && bl->type == BL_MOB )
		return; // assume there will be no statuses active

	status_calc_bl_main(bl, flag);

	if( first && bl->type == BL_HOM )
		return; // client update handled by caller

	// compare against new values and send client updates
	if( bl->type == BL_PC )
	{
		TBL_PC* sd = BL_CAST(BL_PC, bl);
		if(b_status.str != status->str)
			clif->updatestatus(sd,SP_STR);
		if(b_status.agi != status->agi)
			clif->updatestatus(sd,SP_AGI);
		if(b_status.vit != status->vit)
			clif->updatestatus(sd,SP_VIT);
		if(b_status.int_ != status->int_)
			clif->updatestatus(sd,SP_INT);
		if(b_status.dex != status->dex)
			clif->updatestatus(sd,SP_DEX);
		if(b_status.luk != status->luk)
			clif->updatestatus(sd,SP_LUK);
		if(b_status.hit != status->hit)
			clif->updatestatus(sd,SP_HIT);
		if(b_status.flee != status->flee)
			clif->updatestatus(sd,SP_FLEE1);
		if(b_status.amotion != status->amotion)
			clif->updatestatus(sd,SP_ASPD);
		if(b_status.speed != status->speed)
			clif->updatestatus(sd,SP_SPEED);

		if(b_status.batk != status->batk
#ifndef RENEWAL
			|| b_status.rhw.atk != status->rhw.atk || b_status.lhw.atk != status->lhw.atk
#endif
			)
			clif->updatestatus(sd,SP_ATK1);

		if(b_status.def != status->def){
			clif->updatestatus(sd,SP_DEF1);
#ifdef RENEWAL
			clif->updatestatus(sd,SP_DEF2);
#endif
		}

		if(b_status.rhw.atk2 != status->rhw.atk2 || b_status.lhw.atk2 != status->lhw.atk2
#ifdef RENEWAL
		|| b_status.rhw.atk != status->rhw.atk || b_status.lhw.atk != status->lhw.atk
#endif
			)
			clif->updatestatus(sd,SP_ATK2);

		if(b_status.def2 != status->def2){
			clif->updatestatus(sd,SP_DEF2);
#ifdef RENEWAL
			clif->updatestatus(sd,SP_DEF1);
#endif
		}
		if(b_status.flee2 != status->flee2)
			clif->updatestatus(sd,SP_FLEE2);
		if(b_status.cri != status->cri)
			clif->updatestatus(sd,SP_CRITICAL);
#ifndef RENEWAL
		if(b_status.matk_max != status->matk_max)
			clif->updatestatus(sd,SP_MATK1);
		if(b_status.matk_min != status->matk_min)
            clif->updatestatus(sd,SP_MATK2);
#else
		if(b_status.matk_max != status->matk_max || b_status.matk_min != status->matk_min){
			clif->updatestatus(sd,SP_MATK2);
			clif->updatestatus(sd,SP_MATK1);
		}
#endif
		if(b_status.mdef != status->mdef){
			clif->updatestatus(sd,SP_MDEF1);
#ifdef RENEWAL
			clif->updatestatus(sd,SP_MDEF2);
#endif
		}
		if(b_status.mdef2 != status->mdef2){
			clif->updatestatus(sd,SP_MDEF2);
#ifdef RENEWAL
			clif->updatestatus(sd,SP_MDEF1);
#endif
		}
		if(b_status.rhw.range != status->rhw.range)
			clif->updatestatus(sd,SP_ATTACKRANGE);
		if(b_status.max_hp != status->max_hp)
			clif->updatestatus(sd,SP_MAXHP);
		if(b_status.max_sp != status->max_sp)
			clif->updatestatus(sd,SP_MAXSP);
		if(b_status.hp != status->hp)
			clif->updatestatus(sd,SP_HP);
		if(b_status.sp != status->sp)
			clif->updatestatus(sd,SP_SP);
	} else if( bl->type == BL_HOM ) {
		TBL_HOM* hd = BL_CAST(BL_HOM, bl);
		if( hd->master && memcmp(&b_status, status, sizeof(struct status_data)) != 0 )
			clif->hominfo(hd->master,hd,0);
	} else if( bl->type == BL_MER ) {
		TBL_MER* md = BL_CAST(BL_MER, bl);
		if( b_status.rhw.atk != status->rhw.atk || b_status.rhw.atk2 != status->rhw.atk2 )
			clif->mercenary_updatestatus(md->master, SP_ATK1);
		if( b_status.matk_max != status->matk_max )
			clif->mercenary_updatestatus(md->master, SP_MATK1);
		if( b_status.hit != status->hit )
			clif->mercenary_updatestatus(md->master, SP_HIT);
		if( b_status.cri != status->cri )
			clif->mercenary_updatestatus(md->master, SP_CRITICAL);
		if( b_status.def != status->def )
			clif->mercenary_updatestatus(md->master, SP_DEF1);
		if( b_status.mdef != status->mdef )
			clif->mercenary_updatestatus(md->master, SP_MDEF1);
		if( b_status.flee != status->flee )
			clif->mercenary_updatestatus(md->master, SP_MERCFLEE);
		if( b_status.amotion != status->amotion )
			clif->mercenary_updatestatus(md->master, SP_ASPD);
		if( b_status.max_hp != status->max_hp )
			clif->mercenary_updatestatus(md->master, SP_MAXHP);
		if( b_status.max_sp != status->max_sp )
			clif->mercenary_updatestatus(md->master, SP_MAXSP);
		if( b_status.hp != status->hp )
			clif->mercenary_updatestatus(md->master, SP_HP);
		if( b_status.sp != status->sp )
			clif->mercenary_updatestatus(md->master, SP_SP);
	} else if( bl->type == BL_ELEM ) {
		TBL_ELEM* ed = BL_CAST(BL_ELEM, bl);
		if( b_status.max_hp != status->max_hp )
			clif->elemental_updatestatus(ed->master, SP_MAXHP);
		if( b_status.max_sp != status->max_sp )
			clif->elemental_updatestatus(ed->master, SP_MAXSP);
		if( b_status.hp != status->hp )
			clif->elemental_updatestatus(ed->master, SP_HP);
		if( b_status.sp != status->sp )
			clif->mercenary_updatestatus(ed->master, SP_SP);
	}
}

/*==========================================
 * Apply shared stat mods from status changes [DracoRPG]
 *------------------------------------------*/
static unsigned short status_calc_str(struct block_list *bl, struct status_change *sc, int str)
{
	if(!sc || !sc->count)
		return cap_value(str,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		str -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(str,0,USHRT_MAX);
	}
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH && str < 50)
		return 50;
	if(sc->data[SC_INCALLSTATUS])
		str += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCSTR])
		str += sc->data[SC_INCSTR]->val1;
	if(sc->data[SC_STRFOOD])
		str += sc->data[SC_STRFOOD]->val1;
	if(sc->data[SC_FOOD_STR_CASH])
		str += sc->data[SC_FOOD_STR_CASH]->val1;
	if(sc->data[SC_BATTLEORDERS])
		str += 5;
	if(sc->data[SC_LEADERSHIP])
		str += sc->data[SC_LEADERSHIP]->val1;
	if(sc->data[SC_LOUD])
		str += 4;
	if(sc->data[SC_TRUESIGHT])
		str += 5;
	if(sc->data[SC_SPURT])
		str += 10;
	if(sc->data[SC_NEN])
		str += sc->data[SC_NEN]->val1;
	if(sc->data[SC_BLESSING]){
		if(sc->data[SC_BLESSING]->val2)
			str += sc->data[SC_BLESSING]->val2;
		else
			str >>= 1;
	}
	if(sc->data[SC_MARIONETTE])
		str -= ((sc->data[SC_MARIONETTE]->val3)>>16)&0xFF;
	if(sc->data[SC_MARIONETTE2])
		str += ((sc->data[SC_MARIONETTE2]->val3)>>16)&0xFF;
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

	return (unsigned short)cap_value(str,0,USHRT_MAX);
}

static unsigned short status_calc_agi(struct block_list *bl, struct status_change *sc, int agi)
{
	if(!sc || !sc->count)
		return cap_value(agi,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		agi -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(agi,0,USHRT_MAX);
	}
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH && agi < 50)
		return 50;
	if(sc->data[SC_CONCENTRATE] && !sc->data[SC_QUAGMIRE])
		agi += (agi-sc->data[SC_CONCENTRATE]->val3)*sc->data[SC_CONCENTRATE]->val2/100;
	if(sc->data[SC_INCALLSTATUS])
		agi += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCAGI])
		agi += sc->data[SC_INCAGI]->val1;
	if(sc->data[SC_AGIFOOD])
		agi += sc->data[SC_AGIFOOD]->val1;
	if(sc->data[SC_FOOD_AGI_CASH])
		agi += sc->data[SC_FOOD_AGI_CASH]->val1;
	if(sc->data[SC_SOULCOLD])
		agi += sc->data[SC_SOULCOLD]->val1;
	if(sc->data[SC_TRUESIGHT])
		agi += 5;
	if(sc->data[SC_INCREASEAGI])
		agi += sc->data[SC_INCREASEAGI]->val2;
	if(sc->data[SC_INCREASING])
		agi += 4;	// added based on skill updates [Reddozen]
	if(sc->data[SC_DECREASEAGI])
		agi -= sc->data[SC_DECREASEAGI]->val2;
	if(sc->data[SC_QUAGMIRE])
		agi -= sc->data[SC_QUAGMIRE]->val2;
	if(sc->data[SC_SUITON] && sc->data[SC_SUITON]->val3)
		agi -= sc->data[SC_SUITON]->val2;
	if(sc->data[SC_MARIONETTE])
		agi -= ((sc->data[SC_MARIONETTE]->val3)>>8)&0xFF;
	if(sc->data[SC_MARIONETTE2])
		agi += ((sc->data[SC_MARIONETTE2]->val3)>>8)&0xFF;
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

	return (unsigned short)cap_value(agi,0,USHRT_MAX);
}

static unsigned short status_calc_vit(struct block_list *bl, struct status_change *sc, int vit)
{
	if(!sc || !sc->count)
		return cap_value(vit,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		vit -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(vit,0,USHRT_MAX);
	}
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH && vit < 50)
		return 50;
	if(sc->data[SC_INCALLSTATUS])
		vit += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCVIT])
		vit += sc->data[SC_INCVIT]->val1;
	if(sc->data[SC_VITFOOD])
		vit += sc->data[SC_VITFOOD]->val1;
	if(sc->data[SC_FOOD_VIT_CASH])
		vit += sc->data[SC_FOOD_VIT_CASH]->val1;
	if(sc->data[SC_CHANGE])
		vit += sc->data[SC_CHANGE]->val2;
	if(sc->data[SC_GLORYWOUNDS])
		vit += sc->data[SC_GLORYWOUNDS]->val1;
	if(sc->data[SC_TRUESIGHT])
		vit += 5;
	if(sc->data[SC_MARIONETTE])
		vit -= sc->data[SC_MARIONETTE]->val3&0xFF;
	if(sc->data[SC_MARIONETTE2])
		vit += sc->data[SC_MARIONETTE2]->val3&0xFF;
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

	if(sc->data[SC_STRIPARMOR])
		vit -= vit * sc->data[SC_STRIPARMOR]->val2/100;

	return (unsigned short)cap_value(vit,0,USHRT_MAX);
}

static unsigned short status_calc_int(struct block_list *bl, struct status_change *sc, int int_)
{
	if(!sc || !sc->count)
		return cap_value(int_,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		int_ -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(int_,0,USHRT_MAX);
	}
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH && int_ < 50)
		return 50;
	if(sc->data[SC_INCALLSTATUS])
		int_ += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCINT])
		int_ += sc->data[SC_INCINT]->val1;
	if(sc->data[SC_INTFOOD])
		int_ += sc->data[SC_INTFOOD]->val1;
	if(sc->data[SC_FOOD_INT_CASH])
		int_ += sc->data[SC_FOOD_INT_CASH]->val1;
	if(sc->data[SC_CHANGE])
		int_ += sc->data[SC_CHANGE]->val3;
	if(sc->data[SC_BATTLEORDERS])
		int_ += 5;
	if(sc->data[SC_TRUESIGHT])
		int_ += 5;
	if(sc->data[SC_BLESSING]){
		if (sc->data[SC_BLESSING]->val2)
			int_ += sc->data[SC_BLESSING]->val2;
		else
			int_ >>= 1;
	}
	if(sc->data[SC_NEN])
		int_ += sc->data[SC_NEN]->val1;
	if(sc->data[SC_MARIONETTE])
		int_ -= ((sc->data[SC_MARIONETTE]->val4)>>16)&0xFF;
	if(sc->data[SC_MARIONETTE2])
		int_ += ((sc->data[SC_MARIONETTE2]->val4)>>16)&0xFF;
	if(sc->data[SC_MANDRAGORA])
		int_ -= 5 + 5 * sc->data[SC_MANDRAGORA]->val1;
	if(sc->data[SC_COCKTAIL_WARG_BLOOD])
		int_ += sc->data[SC_COCKTAIL_WARG_BLOOD]->val1;
	if(sc->data[SC_INSPIRATION])
		int_ += sc->data[SC_INSPIRATION]->val3;
	if(sc->data[SC_STOMACHACHE])
		int_ -= sc->data[SC_STOMACHACHE]->val1;
	if(sc->data[SC_KYOUGAKU])
		int_ -= sc->data[SC_KYOUGAKU]->val2;

	if(sc->data[SC_STRIPHELM])
		int_ -= int_ * sc->data[SC_STRIPHELM]->val2/100;
	if(sc->data[SC__STRIPACCESSORY])
		int_ -= int_ * sc->data[SC__STRIPACCESSORY]->val2 / 100;

	return (unsigned short)cap_value(int_,0,USHRT_MAX);
}

static unsigned short status_calc_dex(struct block_list *bl, struct status_change *sc, int dex)
{
	if(!sc || !sc->count)
		return cap_value(dex,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		dex -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(dex,0,USHRT_MAX);
	}
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH && dex < 50)
		return 50;
	if(sc->data[SC_CONCENTRATE] && !sc->data[SC_QUAGMIRE])
		dex += (dex-sc->data[SC_CONCENTRATE]->val4)*sc->data[SC_CONCENTRATE]->val2/100;
	if(sc->data[SC_INCALLSTATUS])
		dex += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCDEX])
		dex += sc->data[SC_INCDEX]->val1;
	if(sc->data[SC_DEXFOOD])
		dex += sc->data[SC_DEXFOOD]->val1;
	if(sc->data[SC_FOOD_DEX_CASH])
		dex += sc->data[SC_FOOD_DEX_CASH]->val1;
	if(sc->data[SC_BATTLEORDERS])
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
	if(sc->data[SC_INCREASING])
		dex += 4;	// added based on skill updates [Reddozen]
	if(sc->data[SC_MARIONETTE])
		dex -= ((sc->data[SC_MARIONETTE]->val4)>>8)&0xFF;
	if(sc->data[SC_MARIONETTE2])
		dex += ((sc->data[SC_MARIONETTE2]->val4)>>8)&0xFF;
	if(sc->data[SC_SIROMA_ICE_TEA])
		dex += sc->data[SC_SIROMA_ICE_TEA]->val1;
	if(sc->data[SC_INSPIRATION])
		dex += sc->data[SC_INSPIRATION]->val3;
	if(sc->data[SC_STOMACHACHE])
		dex -= sc->data[SC_STOMACHACHE]->val1;
	if(sc->data[SC_KYOUGAKU])
		dex -= sc->data[SC_KYOUGAKU]->val2;

	if(sc->data[SC__STRIPACCESSORY])
		dex -= dex * sc->data[SC__STRIPACCESSORY]->val2 / 100;

	return (unsigned short)cap_value(dex,0,USHRT_MAX);
}

static unsigned short status_calc_luk(struct block_list *bl, struct status_change *sc, int luk)
{
	if(!sc || !sc->count)
		return cap_value(luk,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		luk -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(luk,0,USHRT_MAX);
	}
	if(sc->data[SC_CURSE])
		return 0;
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH && luk < 50)
		return 50;
	if(sc->data[SC_INCALLSTATUS])
		luk += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCLUK])
		luk += sc->data[SC_INCLUK]->val1;
	if(sc->data[SC_LUKFOOD])
		luk += sc->data[SC_LUKFOOD]->val1;
	if(sc->data[SC_FOOD_LUK_CASH])
		luk += sc->data[SC_FOOD_LUK_CASH]->val1;
	if(sc->data[SC_TRUESIGHT])
		luk += 5;
	if(sc->data[SC_GLORIA])
		luk += 30;
	if(sc->data[SC_MARIONETTE])
		luk -= sc->data[SC_MARIONETTE]->val4&0xFF;
	if(sc->data[SC_MARIONETTE2])
		luk += sc->data[SC_MARIONETTE2]->val4&0xFF;
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

	if(sc->data[SC__STRIPACCESSORY])
		luk -= luk * sc->data[SC__STRIPACCESSORY]->val2 / 100;
	if(sc->data[SC_BANANA_BOMB])
		luk -= luk * sc->data[SC_BANANA_BOMB]->val1 / 100;

	return (unsigned short)cap_value(luk,0,USHRT_MAX);
}

static unsigned short status_calc_batk(struct block_list *bl, struct status_change *sc, int batk)
{
	if(!sc || !sc->count)
		return cap_value(batk,0,USHRT_MAX);

	if(sc->data[SC_ATKPOTION])
		batk += sc->data[SC_ATKPOTION]->val1;
	if(sc->data[SC_BATKFOOD])
		batk += sc->data[SC_BATKFOOD]->val1;
	if(sc->data[SC_GATLINGFEVER])
		batk += sc->data[SC_GATLINGFEVER]->val3;
	if(sc->data[SC_MADNESSCANCEL])
		batk += 100;
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
	if(sc->data[SC_ODINS_POWER])
		batk += 70;
	if(sc->data[SC_ASH] && (bl->type==BL_MOB)){
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
	if(sc->data[SC_CONCENTRATION])
		batk += batk * sc->data[SC_CONCENTRATION]->val2/100;
	if(sc->data[SC_SKE])
		batk += batk * 3;
	if(sc->data[SC_BLOODLUST])
		batk += batk * sc->data[SC_BLOODLUST]->val2/100;
	if(sc->data[SC_JOINTBEAT] && sc->data[SC_JOINTBEAT]->val2&BREAK_WAIST)
		batk -= batk * 25/100;
	if(sc->data[SC_CURSE])
		batk -= batk * 25/100;
//Curse shouldn't effect on this?  <- Curse OR Bleeding??
//	if(sc->data[SC_BLEEDING])
//		batk -= batk * 25/100;
	if(sc->data[SC_FLEET])
		batk += batk * sc->data[SC_FLEET]->val3/100;
	if(sc->data[SC__ENERVATION])
		batk -= batk * sc->data[SC__ENERVATION]->val2 / 100;
	if(sc->data[SC_RUSHWINDMILL])
		batk += batk * sc->data[SC_RUSHWINDMILL]->val2/100;
	if(sc->data[SC_SATURDAYNIGHTFEVER])
		batk += 100 * sc->data[SC_SATURDAYNIGHTFEVER]->val1;
	if(sc->data[SC_MELODYOFSINK])
		batk -= batk * sc->data[SC_MELODYOFSINK]->val3/100;
	if(sc->data[SC_BEYONDOFWARCRY])
		batk += batk * sc->data[SC_BEYONDOFWARCRY]->val3/100;
	if( sc->data[SC_ZANGETSU] )
		batk += batk * sc->data[SC_ZANGETSU]->val2 / 100;

	return (unsigned short)cap_value(batk,0,USHRT_MAX);
}

static unsigned short status_calc_watk(struct block_list *bl, struct status_change *sc, int watk)
{
	if(!sc || !sc->count)
		return cap_value(watk,0,USHRT_MAX);

	if(sc->data[SC_IMPOSITIO])
		watk += sc->data[SC_IMPOSITIO]->val2;
	if(sc->data[SC_WATKFOOD])
		watk += sc->data[SC_WATKFOOD]->val1;
	if(sc->data[SC_DRUMBATTLE])
		watk += sc->data[SC_DRUMBATTLE]->val2;
	if(sc->data[SC_VOLCANO])
		watk += sc->data[SC_VOLCANO]->val2;
	if(sc->data[SC_MERC_ATKUP])
		watk += sc->data[SC_MERC_ATKUP]->val2;
	if(sc->data[SC_FIGHTINGSPIRIT])
		watk += sc->data[SC_FIGHTINGSPIRIT]->val1;
	if(sc->data[SC_STRIKING])
		watk += sc->data[SC_STRIKING]->val2;
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
	if(sc->data[SC_NIBELUNGEN]) {
		if (bl->type != BL_PC)
			watk += sc->data[SC_NIBELUNGEN]->val2;
		else {
		#ifndef RENEWAL
			TBL_PC *sd = (TBL_PC*)bl;
			int index = sd->equip_index[sd->state.lr_flag?EQI_HAND_L:EQI_HAND_R];
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->wlv == 4)
		#endif
				watk += sc->data[SC_NIBELUNGEN]->val2;
		}
	}

	if(sc->data[SC_INCATKRATE])
		watk += watk * sc->data[SC_INCATKRATE]->val1/100;
	if(sc->data[SC_PROVOKE])
		watk += watk * sc->data[SC_PROVOKE]->val3/100;
	if(sc->data[SC_CONCENTRATION])
		watk += watk * sc->data[SC_CONCENTRATION]->val2/100;
	if(sc->data[SC_SKE])
		watk += watk * 3;
	if(sc->data[SC__ENERVATION])
		watk -= watk * sc->data[SC__ENERVATION]->val2 / 100;
	if(sc->data[SC_FLEET])
		watk += watk * sc->data[SC_FLEET]->val3/100;
	if(sc->data[SC_CURSE])
		watk -= watk * 25/100;
	if(sc->data[SC_STRIPWEAPON])
		watk -= watk * sc->data[SC_STRIPWEAPON]->val2/100;
	if(sc->data[SC__ENERVATION])
		watk -= watk * sc->data[SC__ENERVATION]->val2 / 100;
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
#ifdef RENEWAL_EDP
	if( sc->data[SC_EDP] )
		watk = watk * (100 + sc->data[SC_EDP]->val1 * 80) / 100;
#endif

	return (unsigned short)cap_value(watk,0,USHRT_MAX);
}
#ifdef RENEWAL
static unsigned short status_calc_ematk(struct block_list *bl, struct status_change *sc, int matk)
{

    if (!sc || !sc->count)
        return cap_value(matk,0,USHRT_MAX);
    if (sc->data[SC_MATKPOTION])
        matk += sc->data[SC_MATKPOTION]->val1;
    if (sc->data[SC_MATKFOOD])
        matk += sc->data[SC_MATKFOOD]->val1;
	if(sc->data[SC_MANA_PLUS])
		matk += sc->data[SC_MANA_PLUS]->val1;
	if(sc->data[SC_AQUAPLAY_OPTION])
		matk += sc->data[SC_AQUAPLAY_OPTION]->val2;
	if(sc->data[SC_CHILLY_AIR_OPTION])
		matk += sc->data[SC_CHILLY_AIR_OPTION]->val2;
	if(sc->data[SC_WATER_BARRIER])
		matk -= sc->data[SC_WATER_BARRIER]->val3;
	if(sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 3)
		matk += 50;
	if(sc->data[SC_ODINS_POWER])
		matk += 40 + 30 * sc->data[SC_ODINS_POWER]->val1; //70 lvl1, 100lvl2
	if(sc->data[SC_IZAYOI])
		matk += 50 * sc->data[SC_IZAYOI]->val1;
    return (unsigned short)cap_value(matk,0,USHRT_MAX);
}
#endif
static unsigned short status_calc_matk(struct block_list *bl, struct status_change *sc, int matk)
{
	if(!sc || !sc->count)
		return cap_value(matk,0,USHRT_MAX);
#ifndef RENEWAL
	// take note fixed value first before % modifiers
    if (sc->data[SC_MATKPOTION])
        matk += sc->data[SC_MATKPOTION]->val1;
    if (sc->data[SC_MATKFOOD])
        matk += sc->data[SC_MATKFOOD]->val1;
    if (sc->data[SC_MANA_PLUS])
        matk += sc->data[SC_MANA_PLUS]->val1;
    if (sc->data[SC_AQUAPLAY_OPTION])
        matk += sc->data[SC_AQUAPLAY_OPTION]->val2;
    if (sc->data[SC_CHILLY_AIR_OPTION])
        matk += sc->data[SC_CHILLY_AIR_OPTION]->val2;
    if (sc->data[SC_WATER_BARRIER])
        matk -= sc->data[SC_WATER_BARRIER]->val3;
    if (sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 3)
        matk += 50;
    if (sc->data[SC_ODINS_POWER])
        matk += 40 + 30 * sc->data[SC_ODINS_POWER]->val1; //70 lvl1, 100lvl2
    if (sc->data[SC_IZAYOI])
        matk += 50 * sc->data[SC_IZAYOI]->val1;
#endif
    if (sc->data[SC_MAGICPOWER] && sc->data[SC_MAGICPOWER]->val4)
        matk += matk * sc->data[SC_MAGICPOWER]->val3/100;
    if (sc->data[SC_MINDBREAKER])
        matk += matk * sc->data[SC_MINDBREAKER]->val2/100;
    if (sc->data[SC_INCMATKRATE])
        matk += matk * sc->data[SC_INCMATKRATE]->val1/100;
    if (sc->data[SC_MOONLITSERENADE])
        matk += matk * sc->data[SC_MOONLITSERENADE]->val2/100;
    if (sc->data[SC_MELODYOFSINK])
        matk += matk * sc->data[SC_MELODYOFSINK]->val3/100;
    if (sc->data[SC_BEYONDOFWARCRY])
        matk -= matk * sc->data[SC_BEYONDOFWARCRY]->val3/100;
	if( sc->data[SC_ZANGETSU] )
		matk += matk * sc->data[SC_ZANGETSU]->val2 / 100;

	return (unsigned short)cap_value(matk,0,USHRT_MAX);
}

static signed short status_calc_critical(struct block_list *bl, struct status_change *sc, int critical) {

	if(!sc || !sc->count)
		return cap_value(critical,10,SHRT_MAX);

	if (sc->data[SC_INCCRI])
		critical += sc->data[SC_INCCRI]->val2;
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
		critical += critical * sc->data[SC__INVISIBILITY]->val3 / 100;
	if(sc->data[SC__UNLUCKY])
		critical -= critical * sc->data[SC__UNLUCKY]->val2 / 100;

	return (short)cap_value(critical,10,SHRT_MAX);
}

static signed short status_calc_hit(struct block_list *bl, struct status_change *sc, int hit)
{

	if(!sc || !sc->count)
		return cap_value(hit,1,SHRT_MAX);

	if(sc->data[SC_INCHIT])
		hit += sc->data[SC_INCHIT]->val1;
	if(sc->data[SC_HITFOOD])
		hit += sc->data[SC_HITFOOD]->val1;
	if(sc->data[SC_TRUESIGHT])
		hit += sc->data[SC_TRUESIGHT]->val3;
	if(sc->data[SC_HUMMING])
		hit += sc->data[SC_HUMMING]->val2;
	if(sc->data[SC_CONCENTRATION])
		hit += sc->data[SC_CONCENTRATION]->val3;
	if(sc->data[SC_INSPIRATION])
		hit += 5 * sc->data[SC_INSPIRATION]->val1;
	if(sc->data[SC_ADJUSTMENT])
		hit -= 30;
	if(sc->data[SC_INCREASING])
		hit += 20; // RockmanEXE; changed based on updated [Reddozen]
	if(sc->data[SC_MERC_HITUP])
		hit += sc->data[SC_MERC_HITUP]->val2;

	if(sc->data[SC_INCHITRATE])
		hit += hit * sc->data[SC_INCHITRATE]->val1/100;
	if(sc->data[SC_BLIND])
		hit -= hit * 25/100;
	if(sc->data[SC__GROOMY])
		hit -= hit * sc->data[SC__GROOMY]->val3 / 100;
	if(sc->data[SC_FEAR])
		hit -= hit * 20 / 100;
	if (sc->data[SC_ASH])
		hit /= 2;

	return (short)cap_value(hit,1,SHRT_MAX);
}

static signed short status_calc_flee(struct block_list *bl, struct status_change *sc, int flee)
{
	if( bl->type == BL_PC )
	{
		if( map_flag_gvg(bl->m) )
			flee -= flee * battle_config.gvg_flee_penalty/100;
		else if( map[bl->m].flag.battleground )
			flee -= flee * battle_config.bg_flee_penalty/100;
	}

	if(!sc || !sc->count)
		return cap_value(flee,1,SHRT_MAX);

	if(sc->data[SC_INCFLEE])
		flee += sc->data[SC_INCFLEE]->val1;
	if(sc->data[SC_FLEEFOOD])
		flee += sc->data[SC_FLEEFOOD]->val1;
	if(sc->data[SC_WHISTLE])
		flee += sc->data[SC_WHISTLE]->val2;
	if(sc->data[SC_WINDWALK])
		flee += sc->data[SC_WINDWALK]->val2;
	if(sc->data[SC_VIOLENTGALE])
		flee += sc->data[SC_VIOLENTGALE]->val2;
	if(sc->data[SC_MOON_COMFORT]) //SG skill [Komurka]
		flee += sc->data[SC_MOON_COMFORT]->val2;
	if(sc->data[SC_CLOSECONFINE])
		flee += 10;
	if (sc->data[SC_ANGRIFFS_MODUS])
		flee -= sc->data[SC_ANGRIFFS_MODUS]->val3;
	if (sc->data[SC_OVERED_BOOST])
		flee = max(flee,sc->data[SC_OVERED_BOOST]->val2);
	if(sc->data[SC_ADJUSTMENT])
		flee += 30;
	if(sc->data[SC_SPEED])
		flee += 10 + sc->data[SC_SPEED]->val1 * 10;
	if(sc->data[SC_GATLINGFEVER])
		flee -= sc->data[SC_GATLINGFEVER]->val4;
	if(sc->data[SC_PARTYFLEE])
		flee += sc->data[SC_PARTYFLEE]->val1 * 10;
	if(sc->data[SC_MERC_FLEEUP])
		flee += sc->data[SC_MERC_FLEEUP]->val2;
	if( sc->data[SC_HALLUCINATIONWALK] )
		flee += sc->data[SC_HALLUCINATIONWALK]->val2;
	if( sc->data[SC_WATER_BARRIER] )
		flee -= sc->data[SC_WATER_BARRIER]->val3;
	if( sc->data[SC_MARSHOFABYSS] )
		flee -= (9 * sc->data[SC_MARSHOFABYSS]->val3 / 10 + sc->data[SC_MARSHOFABYSS]->val2 / 10) * (bl->type == BL_MOB ? 2 : 1);
#ifdef RENEWAL
	if( sc->data[SC_SPEARQUICKEN] )
		flee += 2 * sc->data[SC_SPEARQUICKEN]->val1;
#endif

	if(sc->data[SC_INCFLEERATE])
		flee += flee * sc->data[SC_INCFLEERATE]->val1/100;
	if(sc->data[SC_SPIDERWEB] && sc->data[SC_SPIDERWEB]->val1)
		flee -= flee * 50/100;
	if (sc->data[SC_BERSERK] || sc->data[SC__BLOODYLUST])
		flee -= flee * 50/100;
	if(sc->data[SC_BLIND])
		flee -= flee * 25/100;
	if(sc->data[SC_FEAR])
		flee -= flee * 20 / 100;
	if(sc->data[SC_PARALYSE])
		flee -= flee * 10 / 100; // 10% Flee reduction
	if(sc->data[SC_INFRAREDSCAN])
		flee -= flee * 30 / 100;
	if( sc->data[SC__LAZINESS] )
		flee -= flee * sc->data[SC__LAZINESS]->val3 / 100;
	if( sc->data[SC_GLOOMYDAY] )
		flee -= flee * sc->data[SC_GLOOMYDAY]->val2 / 100;
	if( sc->data[SC_SATURDAYNIGHTFEVER] )
		flee -= flee * (40 + 10 * sc->data[SC_SATURDAYNIGHTFEVER]->val1) / 100;
	if( sc->data[SC_WIND_STEP_OPTION] )
		flee += flee * sc->data[SC_WIND_STEP_OPTION]->val2 / 100;
	if( sc->data[SC_ZEPHYR] )
		flee += flee * sc->data[SC_ZEPHYR]->val2 / 100;
	if(sc->data[SC_ASH] && (bl->type==BL_MOB)){ //mob
		if(status_get_element(bl) == ELE_WATER) //water type
			flee /= 2;
	}

	return (short)cap_value(flee,1,SHRT_MAX);
}

static signed short status_calc_flee2(struct block_list *bl, struct status_change *sc, int flee2)
{
	if(!sc || !sc->count)
		return cap_value(flee2,10,SHRT_MAX);

	if(sc->data[SC_INCFLEE2])
		flee2 += sc->data[SC_INCFLEE2]->val2;
	if(sc->data[SC_WHISTLE])
		flee2 += sc->data[SC_WHISTLE]->val3*10;
	if(sc->data[SC__UNLUCKY])
		flee2 -= flee2 * sc->data[SC__UNLUCKY]->val2 / 100;

	return (short)cap_value(flee2,10,SHRT_MAX);
}
static defType status_calc_def(struct block_list *bl, struct status_change *sc, int def) {

	if(!sc || !sc->count)
		return (defType)cap_value(def,DEFTYPE_MIN,DEFTYPE_MAX);

	if (sc->data[SC_BERSERK] || sc->data[SC__BLOODYLUST])
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

	if(sc->data[SC_ARMORCHANGE])
		def += sc->data[SC_ARMORCHANGE]->val2;
	if(sc->data[SC_DRUMBATTLE])
		def += sc->data[SC_DRUMBATTLE]->val3;
	if(sc->data[SC_DEFENCE])	//[orn]
		def += sc->data[SC_DEFENCE]->val2 ;
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
	if(sc->data[SC_SIGNUMCRUCIS])
		def -= def * sc->data[SC_SIGNUMCRUCIS]->val2/100;
	if(sc->data[SC_CONCENTRATION])
		def -= def * sc->data[SC_CONCENTRATION]->val4/100;
	if(sc->data[SC_SKE])
		def >>=1;
	if(sc->data[SC_PROVOKE] && bl->type != BL_PC) // Provoke doesn't alter player defense->
		def -= def * sc->data[SC_PROVOKE]->val4/100;
	if(sc->data[SC_STRIPSHIELD])
		def -= def * sc->data[SC_STRIPSHIELD]->val2/100;
	if (sc->data[SC_FLING])
		def -= def * (sc->data[SC_FLING]->val2)/100;
	if( sc->data[SC_FREEZING] )
		def -= def * 10 / 100;
	if( sc->data[SC_MARSHOFABYSS] )
		def -= def * ( 6 + 6 * sc->data[SC_MARSHOFABYSS]->val3/10 + (bl->type == BL_MOB ? 5 : 3) * sc->data[SC_MARSHOFABYSS]->val2/36 ) / 100;
	if( sc->data[SC_ANALYZE] )
		def -= def * ( 14 * sc->data[SC_ANALYZE]->val1 ) / 100;
	if( sc->data[SC_FORCEOFVANGUARD] )
		def += def * 2 * sc->data[SC_FORCEOFVANGUARD]->val1 / 100;
	if(sc->data[SC_SATURDAYNIGHTFEVER])
		def -= def * (10 + 10 * sc->data[SC_SATURDAYNIGHTFEVER]->val1) / 100;
	if(sc->data[SC_EARTHDRIVE])
		def -= def * 25 / 100;
	if( sc->data[SC_ROCK_CRUSHER] )
		def -= def * sc->data[SC_ROCK_CRUSHER]->val2 / 100;
	if( sc->data[SC_POWER_OF_GAIA] )
		def += def * sc->data[SC_POWER_OF_GAIA]->val2 / 100;
	if( sc->data[SC_PRESTIGE] )
		def += def * sc->data[SC_PRESTIGE]->val1 / 100;
	if(sc->data[SC_ASH] && (bl->type==BL_MOB)){
		if(status_get_race(bl)==RC_PLANT)
			def /= 2;
	}

	return (defType)cap_value(def,DEFTYPE_MIN,DEFTYPE_MAX);;
}

static signed short status_calc_def2(struct block_list *bl, struct status_change *sc, int def2)
{
	if(!sc || !sc->count)
#ifdef RENEWAL
		return (short)cap_value(def2,SHRT_MIN,SHRT_MAX);
#else
		return (short)cap_value(def2,1,SHRT_MAX);
#endif

	if (sc->data[SC_BERSERK] || sc->data[SC__BLOODYLUST])
		return 0;
	if(sc->data[SC_ETERNALCHAOS])
		return 0;
	if(sc->data[SC_SUN_COMFORT])
		def2 += sc->data[SC_SUN_COMFORT]->val2;
	if( sc->data[SC_SHIELDSPELL_REF] && sc->data[SC_SHIELDSPELL_REF]->val1 == 1 )
		def2 += sc->data[SC_SHIELDSPELL_REF]->val2;
	if( sc->data[SC_BANDING] && sc->data[SC_BANDING]->val2 > 0 )
		def2 += (5 + sc->data[SC_BANDING]->val1) * (sc->data[SC_BANDING]->val2);

	if(sc->data[SC_ANGELUS])
#ifdef RENEWAL //in renewal only the VIT stat bonus is boosted by angelus
		def2 += status_get_vit(bl) / 2 * sc->data[SC_ANGELUS]->val2/100;
#else
		def2 += def2 * sc->data[SC_ANGELUS]->val2/100;
#endif
	if(sc->data[SC_CONCENTRATION])
		def2 -= def2 * sc->data[SC_CONCENTRATION]->val4/100;
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
	if( sc->data[SC_FREEZING] )
		def2 -= def2 * 3 / 10;
	if(sc->data[SC_ANALYZE])
		def2 -= def2 * ( 14 * sc->data[SC_ANALYZE]->val1 ) / 100;
	if( sc->data[SC_ECHOSONG] )
		def2 += def2 * sc->data[SC_ECHOSONG]->val2/100;
	if(sc->data[SC_ASH] && (bl->type==BL_MOB)){
		if(status_get_race(bl)==RC_PLANT)
			def2 /= 2;
	}
	if (sc->data[SC_PARALYSIS])
		def2 -= def2 * sc->data[SC_PARALYSIS]->val2 / 100;

#ifdef RENEWAL
	return (short)cap_value(def2,SHRT_MIN,SHRT_MAX);
#else
	return (short)cap_value(def2,1,SHRT_MAX);
#endif
}


static defType status_calc_mdef(struct block_list *bl, struct status_change *sc, int mdef) {

	if(!sc || !sc->count)
		return (defType)cap_value(mdef,DEFTYPE_MIN,DEFTYPE_MAX);

	if (sc->data[SC_BERSERK] || sc->data[SC__BLOODYLUST])
		return 0;
	if(sc->data[SC_BARRIER])
		return 100;

#ifndef RENEWAL // no longer provides 90 MDEF in renewal mode
	if(sc->data[SC_STEELBODY])
		return 90;
#endif

	if(sc->data[SC_ARMORCHANGE])
		mdef += sc->data[SC_ARMORCHANGE]->val3;
	if(sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 3)
		mdef += 50;
	if(sc->data[SC_ENDURE])// It has been confirmed that eddga card grants 1 MDEF, not 0, not 10, but 1.
		mdef += (sc->data[SC_ENDURE]->val4 == 0) ? sc->data[SC_ENDURE]->val1 : 1;
	if(sc->data[SC_CONCENTRATION])
		mdef += 1; //Skill info says it adds a fixed 1 Mdef point.
	if(sc->data[SC_STONEHARDSKIN])
		mdef += sc->data[SC_STONEHARDSKIN]->val1;
	if(sc->data[SC_WATER_BARRIER])
		mdef += sc->data[SC_WATER_BARRIER]->val2;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		mdef += 25*mdef/100;
	if(sc->data[SC_FREEZE])
		mdef += 25*mdef/100;
	if( sc->data[SC_MARSHOFABYSS] )
		mdef -= mdef * ( 6 + 6 * sc->data[SC_MARSHOFABYSS]->val3/10 + (bl->type == BL_MOB ? 5 : 3) * sc->data[SC_MARSHOFABYSS]->val2/36 ) / 100;
	if(sc->data[SC_ANALYZE])
		mdef -= mdef * ( 14 * sc->data[SC_ANALYZE]->val1 ) / 100;
	if(sc->data[SC_SYMPHONYOFLOVER])
		mdef += mdef * sc->data[SC_SYMPHONYOFLOVER]->val2 / 100;
	if(sc->data[SC_GT_CHANGE] && sc->data[SC_GT_CHANGE]->val4)
		mdef -= mdef * sc->data[SC_GT_CHANGE]->val4 / 100;
	if (sc->data[SC_ODINS_POWER])
		mdef -= 20 * sc->data[SC_ODINS_POWER]->val1;

	return (defType)cap_value(mdef,DEFTYPE_MIN,DEFTYPE_MAX);
}

static signed short status_calc_mdef2(struct block_list *bl, struct status_change *sc, int mdef2)
{
	if(!sc || !sc->count)
#ifdef RENEWAL
		return (short)cap_value(mdef2,SHRT_MIN,SHRT_MAX);
#else
		return (short)cap_value(mdef2,1,SHRT_MAX);
#endif


	if (sc->data[SC_BERSERK] || sc->data[SC__BLOODYLUST])
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

static unsigned short status_calc_speed(struct block_list *bl, struct status_change *sc, int speed)
{
	TBL_PC* sd = BL_CAST(BL_PC, bl);
	int speed_rate;

	if( sc == NULL )
		return cap_value(speed,10,USHRT_MAX);

	if (sd && sd->state.permanent_speed)
		return (short)cap_value(speed,10,USHRT_MAX);

	if( sd && sd->ud.skilltimer != INVALID_TIMER && (iPc->checkskill(sd,SA_FREECAST) > 0 || sd->ud.skill_id == LG_EXEEDBREAK) )
	{
		if( sd->ud.skill_id == LG_EXEEDBREAK )
			speed_rate = 100 + 60 - (sd->ud.skill_lv * 10);
		else
			speed_rate = 175 - 5 * iPc->checkskill(sd,SA_FREECAST);
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
					val = 15 + 5 * iPc->checkskill(sd, RA_WUGRIDER);
				else if( pc_ismadogear(sd) ) {
					val = (- 10 * (5 - iPc->checkskill(sd,NC_MADOLICENCE)));
					if( sc->data[SC_ACCELERATION] )
						val += 25;
				}
			}

			speed_rate -= val;
		}

		//GetMoveSlowValue()
		{
			int val = 0;

			if( sd && sc->data[SC_HIDING] && iPc->checkskill(sd,RG_TUNNELDRIVE) > 0 )
				val = 120 - 6 * iPc->checkskill(sd,RG_TUNNELDRIVE);
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
					val = max( val, 500 - (40 + 10 * (sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_BARDDANCER)) * iPc->checkskill(sd,(sd->status.sex?BA_MUSICALLESSON:DC_DANCINGLESSON)) );

				if( sc->data[SC_DECREASEAGI] )
					val = max( val, 25 );
				if( sc->data[SC_QUAGMIRE] || sc->data[SC_HALLUCINATIONWALK_POSTDELAY] || (sc->data[SC_GLOOMYDAY] && sc->data[SC_GLOOMYDAY]->val4) )
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
				if( sc->data[SC_GATLINGFEVER] )
					val = max( val, 100 );
				if( sc->data[SC_SUITON] )
					val = max( val, sc->data[SC_SUITON]->val3 );
				if( sc->data[SC_SWOO] )
					val = max( val, 300 );
				if( sc->data[SC_FREEZING] )
					val = max( val, 70 );
				if( sc->data[SC_MARSHOFABYSS] )
					val = max( val, 40 + 10 * sc->data[SC_MARSHOFABYSS]->val1 );
				if( sc->data[SC_CAMOUFLAGE] && (sc->data[SC_CAMOUFLAGE]->val3&1) == 0 )
					val = max( val, sc->data[SC_CAMOUFLAGE]->val1 < 3 ? 0 : 25 * (5 - sc->data[SC_CAMOUFLAGE]->val1) );
				if( sc->data[SC__GROOMY] )
					val = max( val, sc->data[SC__GROOMY]->val2);
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

				if( sd && sd->bonus.speed_rate + sd->bonus.speed_add_rate > 0 ) // permanent item-based speedup
					val = max( val, sd->bonus.speed_rate + sd->bonus.speed_add_rate );
			}

			speed_rate += val;
		}

		//GetMoveHasteValue1()
		{
			int val = 0;

			if( sc->data[SC_SPEEDUP1] ) //FIXME: used both by NPC_AGIUP and Speed Potion script
				val = max( val, 50 );
			if( sc->data[SC_INCREASEAGI] )
				val = max( val, 25 );
			if( sc->data[SC_WINDWALK] )
				val = max( val, 2 * sc->data[SC_WINDWALK]->val1 );
			if( sc->data[SC_CARTBOOST] )
				val = max( val, 20 );
			if( sd && (sd->class_&MAPID_UPPERMASK) == MAPID_ASSASSIN && iPc->checkskill(sd,TF_MISS) > 0 )
				val = max( val, 1 * iPc->checkskill(sd,TF_MISS) );
			if( sc->data[SC_CLOAKING] && (sc->data[SC_CLOAKING]->val4&1) == 1 )
				val = max( val, sc->data[SC_CLOAKING]->val1 >= 10 ? 25 : 3 * sc->data[SC_CLOAKING]->val1 - 3 );
			if (sc->data[SC_BERSERK] || sc->data[SC__BLOODYLUST])
				val = max( val, 25 );
			if( sc->data[SC_RUN] )
				val = max( val, 55 );
			if( sc->data[SC_AVOID] )
				val = max( val, 10 * sc->data[SC_AVOID]->val1 );
			if( sc->data[SC_INVINCIBLE] && !sc->data[SC_INVINCIBLEOFF] )
				val = max( val, 75 );
			if( sc->data[SC_CLOAKINGEXCEED] )
				val = max( val, sc->data[SC_CLOAKINGEXCEED]->val3);
			if( sc->data[SC_HOVERING] )
				val = max( val, 10 );
			if( sc->data[SC_GN_CARTBOOST] )
				val = max( val, sc->data[SC_GN_CARTBOOST]->val2 );
			if( sc->data[SC_SWINGDANCE] )
				val = max( val, sc->data[SC_SWINGDANCE]->val2 );
			if( sc->data[SC_WIND_STEP_OPTION] )
				val = max( val, sc->data[SC_WIND_STEP_OPTION]->val2 );

			//FIXME: official items use a single bonus for this [ultramage]
			if( sc->data[SC_SPEEDUP0] ) // temporary item-based speedup
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
			speed += speed * (50 - 5 * iPc->checkskill(sd,MC_PUSHCART)) / 100;
		if( sc->data[SC_PARALYSE] )
			speed += speed * 50 / 100;
		if( speed_rate != 100 )
			speed = speed * speed_rate / 100;
		if( sc->data[SC_STEELBODY] )
			speed = 200;
		if( sc->data[SC_DEFENDER] )
			speed = max(speed, 200);
		if( sc->data[SC_WALKSPEED] && sc->data[SC_WALKSPEED]->val1 > 0 ) // ChangeSpeed
			speed = speed * 100 / sc->data[SC_WALKSPEED]->val1;
	}

	return (short)cap_value(speed,10,USHRT_MAX);
}

#ifdef RENEWAL_ASPD
// flag&1 - fixed value [malufett]
// flag&2 - percentage value
static short status_calc_aspd(struct block_list *bl, struct status_change *sc, short flag)
{
	int i, pots = 0, skills1 = 0, skills2 = 0;

	if(!sc || !sc->count)
		return 0;

	if(sc->data[i=SC_ASPDPOTION3] ||
		sc->data[i=SC_ASPDPOTION2] ||
		sc->data[i=SC_ASPDPOTION1] ||
		sc->data[i=SC_ASPDPOTION0])
		pots += sc->data[i]->val1;

	if( !sc->data[SC_QUAGMIRE] ){
		if(sc->data[SC_STAR_COMFORT])
			skills1 = 5; // needs more info

		if(sc->data[SC_TWOHANDQUICKEN] && skills1 < 7)
			skills1 = 7;

		if(sc->data[SC_ONEHAND] && skills1 < 7) skills1 = 7;

		if(sc->data[SC_MERC_QUICKEN] && skills1 < 7) // needs more info
			skills1 = 7;

		if(sc->data[SC_ADRENALINE2] && skills1 < 6)
			skills1 = 6;

		if(sc->data[SC_ADRENALINE] && skills1 < 7)
			skills1 = 7;

		if(sc->data[SC_SPEARQUICKEN] && skills1 < 7)
			skills1 = 7;

		if(sc->data[SC_GATLINGFEVER] && skills1 < 9) // needs more info
			skills1 = 9;

		if(sc->data[SC_FLEET] && skills1 < 5)
			skills1 = 5;

		if(sc->data[SC_ASSNCROS] &&
			skills1 < 5+1*sc->data[SC_ASSNCROS]->val1) // needs more info
		{
			if (bl->type!=BL_PC)
				skills1 = 4+1*sc->data[SC_ASSNCROS]->val1;
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
					skills1 = 5+1*sc->data[SC_ASSNCROS]->val1;
			}
		}
	}

	if((sc->data[SC_BERSERK] || sc->data[SC__BLOODYLUST]) &&	skills1 < 15)
		skills1 = 15;
	else if(sc->data[SC_MADNESSCANCEL] && skills1 < 15) // needs more info
		skills1 = 15;

	if(sc->data[SC_DONTFORGETME])
		skills2 -= sc->data[SC_DONTFORGETME]->val2; // needs more info
	if(sc->data[SC_LONGING])
		skills2 -= sc->data[SC_LONGING]->val2; // needs more info
	if(sc->data[SC_STEELBODY])
		skills2 -= 25;
	if(sc->data[SC_SKA])
		skills2 -= 25;
	if(sc->data[SC_DEFENDER])
		skills2 -= sc->data[SC_DEFENDER]->val4; // needs more info
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
	if( sc->data[SC_FREEZING] )
		skills2 -= 30;
	if( sc->data[SC_HALLUCINATIONWALK_POSTDELAY] )
		skills2 -= 50;
	if( sc->data[SC_PARALYSE] )
		skills2 -= 10;
	if( sc->data[SC__BODYPAINT] )
		skills2 -=  2 + 5 * sc->data[SC__BODYPAINT]->val1;
	if( sc->data[SC__INVISIBILITY] )
		skills2 -= sc->data[SC__INVISIBILITY]->val2 ;
	if( sc->data[SC__GROOMY] )
		skills2 -= sc->data[SC__GROOMY]->val2;
	if( sc->data[SC_SWINGDANCE] )
		skills2 += sc->data[SC_SWINGDANCE]->val2;
	if( sc->data[SC_DANCEWITHWUG] )
		skills2 += sc->data[SC_DANCEWITHWUG]->val3;
	if( sc->data[SC_GLOOMYDAY] )
		skills2 -= sc->data[SC_GLOOMYDAY]->val3;
	if( sc->data[SC_EARTHDRIVE] )
		skills2 -= 25;
	if( sc->data[SC_GT_CHANGE] )
		skills2 += sc->data[SC_GT_CHANGE]->val3;
	if( sc->data[SC_MELON_BOMB] )
		skills2 -= sc->data[SC_MELON_BOMB]->val1;
	if( sc->data[SC_BOOST500] )
		skills2 += sc->data[SC_BOOST500]->val1;
	if( sc->data[SC_EXTRACT_SALAMINE_JUICE] )
		skills2 += sc->data[SC_EXTRACT_SALAMINE_JUICE]->val1;
	if( sc->data[SC_INCASPDRATE] )
		skills2 += sc->data[SC_INCASPDRATE]->val1;

	return ( flag&1? (skills1 + pots) : skills2 );
}
#endif

static short status_calc_fix_aspd(struct block_list *bl, struct status_change *sc, int aspd) {
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
		aspd -= (bl->type==BL_PC?iPc->checkskill((TBL_PC *)bl, RK_RUNEMASTERY):10) / 10 * 40;

    return cap_value(aspd, 0, 2000); // will be recap for proper bl anyway
}

/// Calculates an object's ASPD modifier (alters the base amotion value).
/// Note that the scale of aspd_rate is 1000 = 100%.
static short status_calc_aspd_rate(struct block_list *bl, struct status_change *sc, int aspd_rate)
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

		if(sc->data[SC_ONEHAND] &&
			max < sc->data[SC_ONEHAND]->val2)
			max = sc->data[SC_ONEHAND]->val2;

		if(sc->data[SC_MERC_QUICKEN] &&
			max < sc->data[SC_MERC_QUICKEN]->val2)
			max = sc->data[SC_MERC_QUICKEN]->val2;

		if(sc->data[SC_ADRENALINE2] &&
			max < sc->data[SC_ADRENALINE2]->val3)
			max = sc->data[SC_ADRENALINE2]->val3;

		if(sc->data[SC_ADRENALINE] &&
			max < sc->data[SC_ADRENALINE]->val3)
			max = sc->data[SC_ADRENALINE]->val3;

		if(sc->data[SC_SPEARQUICKEN] &&
			max < sc->data[SC_SPEARQUICKEN]->val2)
			max = sc->data[SC_SPEARQUICKEN]->val2;

		if(sc->data[SC_GATLINGFEVER] &&
			max < sc->data[SC_GATLINGFEVER]->val2)
			max = sc->data[SC_GATLINGFEVER]->val2;

		if(sc->data[SC_FLEET] &&
			max < sc->data[SC_FLEET]->val2)
			max = sc->data[SC_FLEET]->val2;

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

		if((sc->data[SC_BERSERK] || sc->data[SC__BLOODYLUST]))
			aspd_rate -= 300;
		else if(sc->data[SC_MADNESSCANCEL])
			aspd_rate -= 200;
	}

	if( sc->data[i=SC_ASPDPOTION3] ||
		sc->data[i=SC_ASPDPOTION2] ||
		sc->data[i=SC_ASPDPOTION1] ||
		sc->data[i=SC_ASPDPOTION0] )
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
	if( sc->data[SC_FREEZING] )
		aspd_rate += 300;
	if( sc->data[SC_HALLUCINATIONWALK_POSTDELAY] )
		aspd_rate += 500;
	if( sc->data[SC_FIGHTINGSPIRIT] && sc->data[SC_FIGHTINGSPIRIT]->val2 )
		aspd_rate -= sc->data[SC_FIGHTINGSPIRIT]->val2;
	if( sc->data[SC_PARALYSE] )
		aspd_rate += 100;
	if( sc->data[SC__BODYPAINT] )
		aspd_rate +=  200 + 50 * sc->data[SC__BODYPAINT]->val1;
	if( sc->data[SC__INVISIBILITY] )
		aspd_rate += sc->data[SC__INVISIBILITY]->val2 * 10 ;
	if( sc->data[SC__GROOMY] )
		aspd_rate += sc->data[SC__GROOMY]->val2 * 10;
	if( sc->data[SC_SWINGDANCE] )
		aspd_rate -= sc->data[SC_SWINGDANCE]->val2 * 10;
	if( sc->data[SC_DANCEWITHWUG] )
		aspd_rate -= sc->data[SC_DANCEWITHWUG]->val3 * 10;
	if( sc->data[SC_GLOOMYDAY] )
		aspd_rate += sc->data[SC_GLOOMYDAY]->val3 * 10;
	if( sc->data[SC_EARTHDRIVE] )
		aspd_rate += 250;
	if( sc->data[SC_GT_CHANGE] )
		aspd_rate -= sc->data[SC_GT_CHANGE]->val3 * 10;
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

static unsigned short status_calc_dmotion(struct block_list *bl, struct status_change *sc, int dmotion)
{
	if( !sc || !sc->count || map_flag_gvg(bl->m) || map[bl->m].flag.battleground )
		return cap_value(dmotion,0,USHRT_MAX);
	/**
	 * It has been confirmed on official servers that MvP mobs have no dmotion even without endure
	 **/
	if( sc->data[SC_ENDURE] || ( bl->type == BL_MOB && (((TBL_MOB*)bl)->status.mode&MD_BOSS) ) )
		return 0;
	if( sc->data[SC_CONCENTRATION] )
		return 0;
	if( sc->data[SC_RUN] || sc->data[SC_WUGDASH] )
		return 0;

	return (unsigned short)cap_value(dmotion,0,USHRT_MAX);
}

static unsigned int status_calc_maxhp(struct block_list *bl, struct status_change *sc, uint64 maxhp)
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
	if (sc->data[SC_BERSERK] || sc->data[SC__BLOODYLUST])
		maxhp += maxhp * 2;
	if(sc->data[SC_MARIONETTE])
		maxhp -= 1000;
	if(sc->data[SC_SOLID_SKIN_OPTION])
		maxhp += 2000;// Fix amount.
	if(sc->data[SC_POWER_OF_GAIA])
		maxhp += 3000;
	if(sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 2)
		maxhp += 500;

	if(sc->data[SC_MERC_HPUP])
		maxhp += maxhp * sc->data[SC_MERC_HPUP]->val2/100;

	if(sc->data[SC_EPICLESIS])
		maxhp += maxhp * 5 * sc->data[SC_EPICLESIS]->val1 / 100;
	if(sc->data[SC_VENOMBLEED])
		maxhp -= maxhp * 15 / 100;
	if(sc->data[SC__WEAKNESS])
		maxhp -= maxhp * sc->data[SC__WEAKNESS]->val2 / 100;
	if(sc->data[SC_LERADSDEW])
		maxhp += maxhp * sc->data[SC_LERADSDEW]->val3 / 100;
	if(sc->data[SC_FORCEOFVANGUARD])
		maxhp += maxhp * 3 * sc->data[SC_FORCEOFVANGUARD]->val1 / 100;
	if(sc->data[SC_INSPIRATION]) //Custom value.
		maxhp += maxhp * 3 * sc->data[SC_INSPIRATION]->val1 / 100;
	if(sc->data[SC_RAISINGDRAGON])
		maxhp += maxhp * (2 + sc->data[SC_RAISINGDRAGON]->val1) / 100;
	if(sc->data[SC_GT_CHANGE]) // Max HP decrease: [Skill Level x 4] %
		maxhp -= maxhp * (4 * sc->data[SC_GT_CHANGE]->val1) / 100;
	if(sc->data[SC_GT_REVITALIZE])// Max HP increase: [Skill Level x 2] %
		maxhp += maxhp * (2 * sc->data[SC_GT_REVITALIZE]->val1) / 100;
	if(sc->data[SC_MUSTLE_M])
		maxhp += maxhp * sc->data[SC_MUSTLE_M]->val1/100;
	if(sc->data[SC_MYSTERIOUS_POWDER])
		maxhp -= sc->data[SC_MYSTERIOUS_POWDER]->val1 / 100;
	if(sc->data[SC_PETROLOGY_OPTION])
		maxhp += maxhp * sc->data[SC_PETROLOGY_OPTION]->val2 / 100;
	if (sc->data[SC_ANGRIFFS_MODUS])
		maxhp += maxhp * 5 * sc->data[SC_ANGRIFFS_MODUS]->val1 /100;
	if (sc->data[SC_GOLDENE_FERSE])
		maxhp += maxhp * sc->data[SC_GOLDENE_FERSE]->val2 / 100;

	return (unsigned int)cap_value(maxhp,1,UINT_MAX);
}

static unsigned int status_calc_maxsp(struct block_list *bl, struct status_change *sc, unsigned int maxsp)
{
	if(!sc || !sc->count)
		return cap_value(maxsp,1,UINT_MAX);

	if(sc->data[SC_INCMSPRATE])
		maxsp += maxsp * sc->data[SC_INCMSPRATE]->val1/100;
	if(sc->data[SC_INCMSP])
		maxsp += (sc->data[SC_INCMSP]->val1);
	if(sc->data[SC_SERVICE4U])
		maxsp += maxsp * sc->data[SC_SERVICE4U]->val2/100;
	if(sc->data[SC_MERC_SPUP])
		maxsp += maxsp * sc->data[SC_MERC_SPUP]->val2/100;
	if(sc->data[SC_RAISINGDRAGON])
		maxsp += maxsp * (2 + sc->data[SC_RAISINGDRAGON]->val1) / 100;
	if(sc->data[SC_LIFE_FORCE_F])
		maxsp += maxsp * sc->data[SC_LIFE_FORCE_F]->val1/100;
	if(sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 3)
		maxsp += 50;

	return cap_value(maxsp,1,UINT_MAX);
}

static unsigned char status_calc_element(struct block_list *bl, struct status_change *sc, int element)
{
	if(!sc || !sc->count)
		return element;

	if(sc->data[SC_FREEZE])
		return ELE_WATER;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		return ELE_EARTH;
	if(sc->data[SC_BENEDICTIO])
		return ELE_HOLY;
	if(sc->data[SC_CHANGEUNDEAD])
		return ELE_UNDEAD;
	if(sc->data[SC_ELEMENTALCHANGE])
		return sc->data[SC_ELEMENTALCHANGE]->val2;
	if(sc->data[SC_SHAPESHIFT])
		return sc->data[SC_SHAPESHIFT]->val2;

	return (unsigned char)cap_value(element,0,UCHAR_MAX);
}

static unsigned char status_calc_element_lv(struct block_list *bl, struct status_change *sc, int lv)
{
	if(!sc || !sc->count)
		return lv;

	if(sc->data[SC_FREEZE])
		return 1;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		return 1;
	if(sc->data[SC_BENEDICTIO])
		return 1;
	if(sc->data[SC_CHANGEUNDEAD])
		return 1;
	if(sc->data[SC_ELEMENTALCHANGE])
		return sc->data[SC_ELEMENTALCHANGE]->val1;
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
	if(sc->data[SC_WATERWEAPON]
                || (sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 2) )
		return ELE_WATER;
	if(sc->data[SC_EARTHWEAPON]
                || (sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 2) )
		return ELE_EARTH;
	if(sc->data[SC_FIREWEAPON]
                || (sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 2) )
		return ELE_FIRE;
	if(sc->data[SC_WINDWEAPON]
                || (sc->data[SC_WIND_INSIGNIA] && sc->data[SC_WIND_INSIGNIA]->val1 == 2) )
		return ELE_WIND;
	if(sc->data[SC_ENCPOISON])
		return ELE_POISON;
	if(sc->data[SC_ASPERSIO])
		return ELE_HOLY;
	if(sc->data[SC_SHADOWWEAPON])
		return ELE_DARK;
	if(sc->data[SC_GHOSTWEAPON] || sc->data[SC__INVISIBILITY])
		return ELE_GHOST;
	if(sc->data[SC_TIDAL_WEAPON_OPTION] || sc->data[SC_TIDAL_WEAPON] )
		return ELE_WATER;
    if(sc->data[SC_PYROCLASTIC])
        return ELE_FIRE;
	return (unsigned char)cap_value(element,0,UCHAR_MAX);
}

static unsigned short status_calc_mode(struct block_list *bl, struct status_change *sc, int mode)
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
	nullpo_retr(&dummy_status, bl);

	switch (bl->type) {
		case BL_PC:  return &((TBL_PC*)bl)->battle_status;
		case BL_MOB: return &((TBL_MOB*)bl)->status;
		case BL_PET: return &((TBL_PET*)bl)->status;
		case BL_HOM: return &((TBL_HOM*)bl)->battle_status;
		case BL_MER: return &((TBL_MER*)bl)->battle_status;
		case BL_ELEM: return &((TBL_ELEM*)bl)->battle_status;
		case BL_NPC:  return ((mobdb_checkid(((TBL_NPC*)bl)->class_) == 0) ? &((TBL_NPC*)bl)->status : &dummy_status);
		default:
			return &dummy_status;
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
		case BL_NPC:  return ((mobdb_checkid(((TBL_NPC*)bl)->class_) == 0) ? &((TBL_NPC*)bl)->status : NULL); 
		default:
			return NULL;
	}
}
defType status_get_def(struct block_list *bl) {
	struct unit_data *ud;
	struct status_data *status = status_get_status_data(bl);
	int def = status?status->def:0;
	ud = unit_bl2ud(bl);
	if (ud && ud->skilltimer != INVALID_TIMER)
		def -= def * skill->get_castdef(ud->skill_id)/100;

	return cap_value(def, DEFTYPE_MIN, DEFTYPE_MAX);
}

unsigned short status_get_speed(struct block_list *bl)
{
	if(bl->type==BL_NPC)//Only BL with speed data but no status_data [Skotlex] 
		return ((struct npc_data *)bl)->speed; 
	return status_get_status_data(bl)->speed;
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
					if (md->special_state.ai && (msd = iMap->id2sd(md->master_id)) != NULL)
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
				if (md->special_state.ai && (msd = iMap->id2sd(md->master_id)) != NULL)
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
				if (md->special_state.ai && (msd = iMap->id2sd(md->master_id)) != NULL)
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

int status_isdead(struct block_list *bl)
{
	nullpo_ret(bl);
	return status_get_status_data(bl)->hp == 0;
}

int status_isimmune(struct block_list *bl)
{
	struct status_change *sc =status_get_sc(bl);
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
	if (mobdb_checkid(class_) || mob_is_clone(class_))
		vd = mob_get_viewdata(class_);
	else if (npcdb_checkid(class_) || (bl->type == BL_NPC && class_ == WARP_CLASS))
		vd = npc_get_viewdata(class_);
	else if (homdb_checkid(class_))
		vd = homun->get_viewdata(class_);
	else if (merc_class(class_))
		vd = merc_get_viewdata(class_);
	else if (elemental_class(class_))
		vd = elemental_get_viewdata(class_);
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

void status_change_init(struct block_list *bl)
{
	struct status_change *sc = status_get_sc(bl);
	nullpo_retv(sc);
	memset(sc, 0, sizeof (struct status_change));
}

//Applies SC defense to a given status change.
//Returns the adjusted duration based on flag values.
//the flag values are the same as in status_change_start.
int status_get_sc_def(struct block_list *bl, enum sc_type type, int rate, int tick, int flag)
{
	//Percentual resistance: 10000 = 100% Resist
	//Example: 50% -> sc_def=5000 -> 25%; 5000ms -> tick_def=5000 -> 2500ms
	int sc_def = 0, tick_def = -1; //-1 = use sc_def
	//Linear resistance substracted from rate and tick after percentual resistance was applied
	//Example: 25% -> sc_def2=2000 -> 5%; 2500ms -> tick_def2=2000 -> 500ms
	int sc_def2 = 0, tick_def2 = -1; //-1 = use sc_def2
	struct status_data* status;
	struct status_change* sc;
	struct map_session_data *sd;

	nullpo_ret(bl);

	//Status that are blocked by Golden Thief Bug card or Wand of Hermod
	if (status_isimmune(bl))
		switch (type) {
			case SC_DECREASEAGI:
			case SC_SILENCE:
			case SC_COMA:
			case SC_INCREASEAGI:
			case SC_BLESSING:
			case SC_SLOWPOISON:
			case SC_IMPOSITIO:
			case SC_AETERNA:
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
			case SC_HALLUCINATION:
			case SC_STONE:
			case SC_QUAGMIRE:
			case SC_SUITON:
			case SC_SWINGDANCE:
			case SC__ENERVATION:
			case SC__GROOMY:
			case SC__IGNORANCE:
			case SC__LAZINESS:
			case SC__UNLUCKY:
			case SC__WEAKNESS:
			case SC__BLOODYLUST:
				return 0;
		}

	sd = BL_CAST(BL_PC,bl);
	status = status_get_status_data(bl);
	sc = status_get_sc(bl);
	if( sc && !sc->count )
		sc = NULL;
	switch (type) {
		case SC_STUN:
		case SC_POISON:
			if( sc && sc->data[SC__UNLUCKY] )
				return tick;
		case SC_DPOISON:
		case SC_SILENCE:
		case SC_BLEEDING:
			sc_def = status->vit*100;
			sc_def2 = status->luk*10;
			break;
		case SC_SLEEP:
			sc_def = status->int_*100;
			sc_def2 = status->luk*10;
			break;
		case SC_DEEPSLEEP:
			sc_def = status->int_*50;
			tick_def = status->int_*10 + status_get_lv(bl) * 65 / 10; //Seems to be -1 sec every 10 int and -5% chance every 10 int.
			break;
		case SC_DECREASEAGI:
		case SC_ADORAMUS: //Arch Bishop
			if (sd) tick>>=1; //Half duration for players.
		case SC_STONE:
			//Impossible to reduce duration with stats
			tick_def = 0;
			tick_def2 = 0;
		case SC_FREEZE:
			sc_def = status->mdef*100;
			sc_def2 = status->luk*10;
			break;
		case SC_CURSE:
			//Special property: inmunity when luk is greater than level or zero
			if (status->luk > status_get_lv(bl) || status->luk == 0)
				return 0;
			sc_def = status->luk*100;
			sc_def2 = status->luk*10;
			tick_def = status->vit*100;
			break;
		case SC_BLIND:
			if( sc && sc->data[SC__UNLUCKY] )
				return tick;
			sc_def = (status->vit + status->int_)*50;
			sc_def2 = status->luk*10;
			break;
		case SC_CONFUSION:
			sc_def = (status->str + status->int_)*50;
			sc_def2 = status->luk*10;
			break;
		case SC_ANKLE:
			if(status->mode&MD_BOSS) // Lasts 5 times less on bosses
				tick /= 5;
			sc_def = status->agi*50;
			break;
		case SC_MAGICMIRROR:
		case SC_ARMORCHANGE:
			if (sd) //Duration greatly reduced for players.
				tick /= 15;
			sc_def2 = status_get_lv(bl)*20 + status->vit*25 + status->agi*10; // Lineal Reduction of Rate
			tick_def2 = 0; //No duration reduction
			break;
		case SC_MARSHOFABYSS:
			//5 second (Fixed) + 25 second - {( INT + LUK ) / 20 second }
			tick_def2 = (status->int_ + status->luk)*50;
			break;
		case SC_STASIS:
			//5 second (fixed) + { Stasis Skill level * 5 - (Target's VIT + DEX) / 20 }
			tick_def2 = (status->vit + status->dex)*50;
			break;
		if( bl->type == BL_PC )
			tick -= (status_get_lv(bl) / 5 + status->vit / 4 + status->agi / 10)*100;
		else
			tick -= (status->vit + status->luk) / 20 * 1000;
		break;
	case SC_BURNING:
		// From iROwiki : http://forums.irowiki.org/showpost.php?p=577240&postcount=583
		tick -= 50*status->luk + 60*status->int_ + 170*status->vit;
		tick = max(tick,10000); // Minimum Duration 10s.
		break;
	case SC_FREEZING:
		tick -= 1000 * ((status->vit + status->dex) / 20);
		tick = max(tick,10000); // Minimum Duration 10s.
		break;
	case SC_OBLIVIONCURSE: // 100% - (100 - 0.8 x INT)
		sc_def = 100 - ( 100 - status->int_* 8 / 10 );
		sc_def = max(sc_def, 5); // minimum of 5%
		break;
	case SC_BITE: // {(Base Success chance) - (Target's AGI / 4)}
		rate -= status->agi*1000/4;
		rate = max(rate,50000); // minimum of 50%
		break;
	case SC_ELECTRICSHOCKER:
		if( bl->type == BL_MOB )
			tick -= 1000 * (status->agi/10);
		break;
	case SC_CRYSTALIZE:
		tick -= (1000*(status->vit/10))+(status_get_lv(bl)/50);
		break;
	case SC_MANDRAGORA:
		sc_def = (status->vit+status->luk)/5;
		break;
	case SC_KYOUGAKU:
		tick -= 30*status->int_;
		break;
        case SC_PARALYSIS:
            tick -= 50 * (status->vit + status->luk); //(1000/20);
            break;
	default:
		//Effect that cannot be reduced? Likely a buff.
		if (!(rnd()%10000 < rate))
			return 0;
		return tick?tick:1;
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

	//When no tick def, reduction is the same for both.
	if(tick_def < 0)
		tick_def = sc_def;
	if(tick_def2 < 0)
		tick_def2 = sc_def2;

	//Natural resistance
	if (!(flag&8)) {
		rate -= rate*sc_def/10000;
		rate -= sc_def2;

		//Minimum chances
		switch (type) {
			case SC_BITE:
				rate = max(rate, 5000); //Minimum of 50%
				break;
		}

		//Item resistance (only applies to rate%)
		if(sd && SC_COMMON_MIN <= type && type <= SC_COMMON_MAX)
		{
			if( sd->reseff[type-SC_COMMON_MIN] > 0 )
				rate -= rate*sd->reseff[type-SC_COMMON_MIN]/10000;
			if( sd->sc.data[SC_COMMONSC_RESIST] )
				rate -= rate*sd->sc.data[SC_COMMONSC_RESIST]->val1/100;
		}
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
		case SC_ANKLE:
		case SC_MARSHOFABYSS:
		case SC_STASIS:
			tick = max(tick, 5000); //Minimum duration 5s
			break;
		case SC_BURNING:
		case SC_FREEZING:
			tick = max(tick, 10000); //Minimum duration 10s
			break;
		default:
			//Skills need to trigger even if the duration is reduced below 1ms
			tick = max(tick, 1);
			break;
	}

	return tick;
}
/* [Ind/Hercules] fast-checkin sc-display array */
void status_display_add(struct map_session_data *sd, enum sc_type type, int dval1, int dval2, int dval3) {
	struct sc_display_entry *entry = ers_alloc(pc_sc_display_ers, struct sc_display_entry);
	
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
		
		ers_free(pc_sc_display_ers, sd->sc_display[i]);
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
int status_change_start(struct block_list* bl,enum sc_type type,int rate,int val1,int val2,int val3,int val4,int tick,int flag) {
	struct map_session_data *sd = NULL;
	struct status_change* sc;
	struct status_change_entry* sce;
	struct status_data *status;
	struct view_data *vd;
	int opt_flag, calc_flag, undead_flag, val_flag = 0, tick_time = 0;

	nullpo_ret(bl);
	sc = status_get_sc(bl);
	status = status_get_status_data(bl);

	if( type <= SC_NONE || type >= SC_MAX )
	{
		ShowError("status_change_start: invalid status change (%d)!\n", type);
		return 0;
	}

	if( !sc )
		return 0; //Unable to receive status changes

	if( status_isdead(bl) && type != SC_NOCHAT ) // SC_NOCHAT should work even on dead characters
		return 0;

	if( bl->type == BL_MOB)
	{
		struct mob_data *md = BL_CAST(BL_MOB,bl);
		if(md && (md->class_ == MOBID_EMPERIUM || mob_is_battleground(md)) && type != SC_SAFETYWALL && type != SC_PNEUMA)
			return 0; //Emperium/BG Monsters can't be afflicted by status changes
	//	if(md && mob_is_gvg(md) && status_sc2scb_flag(type)&SCB_MAXHP)
	//		return 0; //prevent status addinh hp to gvg mob (like bloodylust=hp*3 etc...
	}

	if( sc->data[SC_REFRESH] ) {
		if( type >= SC_COMMON_MIN && type <= SC_COMMON_MAX) // Confirmed.
			return 0; // Immune to status ailements
		switch( type ) {
		case SC_QUAGMIRE://Tester said it protects against this and decrease agi.
		case SC_DECREASEAGI:
		case SC_BURNING:
		case SC_FREEZING:
		//case SC_WHITEIMPRISON://Need confirm. Protected against this in the past. [Rytech]
		case SC_MARSHOFABYSS:
		case SC_TOXIN:
		case SC_PARALYSE:
		case SC_VENOMBLEED:
		case SC_MAGICMUSHROOM:
		case SC_DEATHHURT:
		case SC_PYREXIA:
		case SC_OBLIVIONCURSE:
		case SC_LEECHESEND:
		case SC_CRYSTALIZE: ////08/31/2011 - Class Balance Changes
		case SC_DEEPSLEEP:
		case SC_MANDRAGORA:
			return 0;
		}
	}
	else if( sc->data[SC_INSPIRATION] ) {
		if( type >= SC_COMMON_MIN && type <= SC_COMMON_MAX )
			return 0; // Immune to status ailements
		switch( type ) {
			case SC_DEEPSLEEP:
			case SC_SATURDAYNIGHTFEVER:
			case SC_PYREXIA:
			case SC_DEATHHURT:
			case SC_MAGICMUSHROOM:
			case SC_VENOMBLEED:
			case SC_TOXIN:
			case SC_OBLIVIONCURSE:
			case SC_LEECHESEND:
			case SC__ENERVATION:
			case SC__GROOMY:
			case SC__LAZINESS:
			case SC__UNLUCKY:
			case SC__WEAKNESS:
			case SC__BODYPAINT:
			case SC__IGNORANCE:
				return 0;
		}
	}

	sd = BL_CAST(BL_PC, bl);

	//Adjust tick according to status resistances
	if( !(flag&(1|4)) )
	{
		tick = status_get_sc_def(bl, type, rate, tick, flag);
		if( !tick ) return 0;
	}

	undead_flag = battle->check_undead(status->race,status->def_ele);
	//Check for inmunities / sc fails
	switch (type) {
        case SC_ANGRIFFS_MODUS:
        case SC_GOLDENE_FERSE:
             if ((type==SC_GOLDENE_FERSE && sc->data[SC_ANGRIFFS_MODUS])
                     || (type==SC_ANGRIFFS_MODUS && sc->data[SC_GOLDENE_FERSE])
                     )
                return 0;
	case SC_STONE:
		if(sc->data[SC_POWER_OF_GAIA])
			return 0;
	case SC_FREEZE:
		//Undead are immune to Freeze/Stone
		if (undead_flag && !(flag&1))
			return 0;
	case SC_DEEPSLEEP:
	case SC_SLEEP:
	case SC_STUN:
	case SC_FREEZING:
	case SC_CRYSTALIZE:
		if (sc->opt1)
			return 0; //Cannot override other opt1 status changes. [Skotlex]
		if((type == SC_FREEZE || type == SC_FREEZING || type == SC_CRYSTALIZE) && sc->data[SC_WARMER])
			return 0; //Immune to Frozen and Freezing status if under Warmer status. [Jobbie]
	break;

            //There all like berserk, do not everlap each other
	case SC__BLOODYLUST:
		if(!sd) return 0; //should only affect player
	case SC_BERSERK:
		if (((type == SC_BERSERK) && (sc->data[SC_SATURDAYNIGHTFEVER] || sc->data[SC__BLOODYLUST]))
		|| ((type == SC__BLOODYLUST) && (sc->data[SC_SATURDAYNIGHTFEVER] || sc->data[SC_BERSERK]))
		)
			return 0;
		break;

	case SC_BURNING:
		if(sc->opt1 || sc->data[SC_FREEZING])
			return 0;
	break;

	case SC_SIGNUMCRUCIS:
		//Only affects demons and undead element (but not players)
		if((!undead_flag && status->race!=RC_DEMON) || bl->type == BL_PC)
			return 0;
	break;
	case SC_AETERNA:
		if( (sc->data[SC_STONE] && sc->opt1 == OPT1_STONE) || sc->data[SC_FREEZE] )
			return 0;
	break;
	case SC_KYRIE:
		if (bl->type == BL_MOB)
			return 0;
	break;
	case SC_OVERTHRUST:
		if (sc->data[SC_MAXOVERTHRUST])
			return 0; //Overthrust can't take effect if under Max Overthrust. [Skotlex]
	case SC_MAXOVERTHRUST:
		if( sc->option&OPTION_MADOGEAR )
			return 0;//Overthrust and Overthrust Max cannot be used on Mado Gear [Ind]
	break;
	case SC_ADRENALINE:
		if(sd && !pc_check_weapontype(sd,skill->get_weapontype(BS_ADRENALINE)))
			return 0;
		if (sc->data[SC_QUAGMIRE] ||
			sc->data[SC_DECREASEAGI] ||
			sc->option&OPTION_MADOGEAR //Adrenaline doesn't affect Mado Gear [Ind]
		)
			return 0;
	break;
	case SC_ADRENALINE2:
		if(sd && !pc_check_weapontype(sd,skill->get_weapontype(BS_ADRENALINE2)))
			return 0;
		if (sc->data[SC_QUAGMIRE] ||
			sc->data[SC_DECREASEAGI]
		)
			return 0;
	break;
	case SC_MAGNIFICAT:
		if( sc->option&OPTION_MADOGEAR ) //Mado is immune to magnificat
			return 0;
		break;
	case SC_ONEHAND:
	case SC_MERC_QUICKEN:
	case SC_TWOHANDQUICKEN:
		if(sc->data[SC_DECREASEAGI])
			return 0;

	case SC_INCREASEAGI:
		 if(sd && pc_issit(sd)){
			 iPc->setstand(sd);
		 }

	case SC_CONCENTRATE:
	case SC_SPEARQUICKEN:
	case SC_TRUESIGHT:
	case SC_WINDWALK:
	case SC_CARTBOOST:
	case SC_ASSNCROS:
		if (sc->data[SC_QUAGMIRE])
			return 0;
		if(sc->option&OPTION_MADOGEAR)
			return 0;//Mado is immune to increase agi, wind walk, cart boost, etc (others above) [Ind]
	break;
	case SC_CLOAKING:
		//Avoid cloaking with no wall and low skill level. [Skotlex]
		//Due to the cloaking card, we have to check the wall versus to known
		//skill level rather than the used one. [Skotlex]
		//if (sd && val1 < 3 && skill_check_cloaking(bl,NULL))
		if( sd && iPc->checkskill(sd, AS_CLOAKING) < 3 && !skill->check_cloaking(bl,NULL) )
			return 0;
	break;
	case SC_MODECHANGE:
	{
		int mode;
		struct status_data *bstatus = status_get_base_status(bl);
		if (!bstatus) return 0;
		if (sc->data[type])
		{	//Pile up with previous values.
			if(!val2) val2 = sc->data[type]->val2;
			val3 |= sc->data[type]->val3;
			val4 |= sc->data[type]->val4;
		}
		mode = val2?val2:bstatus->mode; //Base mode
		if (val4) mode&=~val4; //Del mode
		if (val3) mode|= val3; //Add mode
		if (mode == bstatus->mode) { //No change.
			if (sc->data[type]) //Abort previous status
				return status_change_end(bl, type, INVALID_TIMER);
			return 0;
		}
	}
	break;
	//Strip skills, need to divest something or it fails.
	case SC_STRIPWEAPON:
		if (sd && !(flag&4)) { //apply sc anyway if loading saved sc_data
			int i;
			opt_flag = 0; //Reuse to check success condition.
			if(sd->bonus.unstripable_equip&EQP_WEAPON)
				return 0;

			i = sd->equip_index[EQI_HAND_R];
			if (i>=0 && sd->inventory_data[i] && sd->inventory_data[i]->type == IT_WEAPON) {
				opt_flag|=2;
				iPc->unequipitem(sd,i,3);
			}
			if (!opt_flag) return 0;
		}
		if (tick == 1) return 1; //Minimal duration: Only strip without causing the SC
	break;
	case SC_STRIPSHIELD:
		if( val2 == 1 ) val2 = 0; //GX effect. Do not take shield off..
		else
		if (sd && !(flag&4)) {
			int i;
			if(sd->bonus.unstripable_equip&EQP_SHIELD)
				return 0;
			i = sd->equip_index[EQI_HAND_L];
			if ( i < 0 || !sd->inventory_data[i] || sd->inventory_data[i]->type != IT_ARMOR )
				return 0;
			iPc->unequipitem(sd,i,3);
		}
		if (tick == 1) return 1; //Minimal duration: Only strip without causing the SC
	break;
	case SC_STRIPARMOR:
		if (sd && !(flag&4)) {
			int i;
			if(sd->bonus.unstripable_equip&EQP_ARMOR)
				return 0;
			i = sd->equip_index[EQI_ARMOR];
			if ( i < 0 || !sd->inventory_data[i] )
				return 0;
			iPc->unequipitem(sd,i,3);
		}
		if (tick == 1) return 1; //Minimal duration: Only strip without causing the SC
	break;
	case SC_STRIPHELM:
		if (sd && !(flag&4)) {
			int i;
			if(sd->bonus.unstripable_equip&EQP_HELM)
				return 0;
			i = sd->equip_index[EQI_HEAD_TOP];
			if ( i < 0 || !sd->inventory_data[i] )
				return 0;
			iPc->unequipitem(sd,i,3);
		}
		if (tick == 1) return 1; //Minimal duration: Only strip without causing the SC
	break;
	case SC_MERC_FLEEUP:
	case SC_MERC_ATKUP:
	case SC_MERC_HPUP:
	case SC_MERC_SPUP:
	case SC_MERC_HITUP:
		if( bl->type != BL_MER )
			return 0; // Stats only for Mercenaries
	break;
	case SC_STRFOOD:
		if (sc->data[SC_FOOD_STR_CASH] && sc->data[SC_FOOD_STR_CASH]->val1 > val1)
			return 0;
	break;
	case SC_AGIFOOD:
		if (sc->data[SC_FOOD_AGI_CASH] && sc->data[SC_FOOD_AGI_CASH]->val1 > val1)
			return 0;
	break;
	case SC_VITFOOD:
		if (sc->data[SC_FOOD_VIT_CASH] && sc->data[SC_FOOD_VIT_CASH]->val1 > val1)
			return 0;
	break;
	case SC_INTFOOD:
		if (sc->data[SC_FOOD_INT_CASH] && sc->data[SC_FOOD_INT_CASH]->val1 > val1)
			return 0;
	break;
	case SC_DEXFOOD:
		if (sc->data[SC_FOOD_DEX_CASH] && sc->data[SC_FOOD_DEX_CASH]->val1 > val1)
			return 0;
	break;
	case SC_LUKFOOD:
		if (sc->data[SC_FOOD_LUK_CASH] && sc->data[SC_FOOD_LUK_CASH]->val1 > val1)
			return 0;
	break;
	case SC_FOOD_STR_CASH:
		if (sc->data[SC_STRFOOD] && sc->data[SC_STRFOOD]->val1 > val1)
			return 0;
	break;
	case SC_FOOD_AGI_CASH:
		if (sc->data[SC_AGIFOOD] && sc->data[SC_AGIFOOD]->val1 > val1)
			return 0;
	break;
	case SC_FOOD_VIT_CASH:
		if (sc->data[SC_VITFOOD] && sc->data[SC_VITFOOD]->val1 > val1)
			return 0;
	break;
	case SC_FOOD_INT_CASH:
		if (sc->data[SC_INTFOOD] && sc->data[SC_INTFOOD]->val1 > val1)
			return 0;
	break;
	case SC_FOOD_DEX_CASH:
		if (sc->data[SC_DEXFOOD] && sc->data[SC_DEXFOOD]->val1 > val1)
			return 0;
	break;
	case SC_FOOD_LUK_CASH:
		if (sc->data[SC_LUKFOOD] && sc->data[SC_LUKFOOD]->val1 > val1)
			return 0;
	break;
	case SC_CAMOUFLAGE:
		if( sd && iPc->checkskill(sd, RA_CAMOUFLAGE) < 3 && !skill->check_camouflage(bl,NULL) )
			return 0;
	break;
	case SC__STRIPACCESSORY:
		if( sd ) {
			int i = -1;
			if( !(sd->bonus.unstripable_equip&EQI_ACC_L) ) {
				i = sd->equip_index[EQI_ACC_L];
				if( i >= 0 && sd->inventory_data[i] && sd->inventory_data[i]->type == IT_ARMOR )
					iPc->unequipitem(sd,i,3); //L-Accessory
			} if( !(sd->bonus.unstripable_equip&EQI_ACC_R) ) {
				i = sd->equip_index[EQI_ACC_R];
				if( i >= 0 && sd->inventory_data[i] && sd->inventory_data[i]->type == IT_ARMOR )
					iPc->unequipitem(sd,i,3); //R-Accessory
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
	case SC_SATURDAYNIGHTFEVER:
		if (sc->data[SC_BERSERK] || sc->data[SC_INSPIRATION] || sc->data[SC__BLOODYLUST])
			return 0;
		break;
	}

	//Check for BOSS resistances
	if(status->mode&MD_BOSS && !(flag&1)) {
		 if (type>=SC_COMMON_MIN && type <= SC_COMMON_MAX)
			 return 0;
		 switch (type) {
			case SC_BLESSING:
			case SC_DECREASEAGI:
			case SC_PROVOKE:
			case SC_COMA:
			case SC_GRAVITATION:
			case SC_SUITON:
			case SC_RICHMANKIM:
			case SC_ROKISWEIL:
			case SC_FOGWALL:
			case SC_FREEZING:
			case SC_BURNING: 
			case SC_MARSHOFABYSS:
			case SC_ADORAMUS:
			case SC_PARALYSIS:
			case SC_DEEPSLEEP:
			case SC_CRYSTALIZE:

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
			case SC_BITE:
			case SC_ELECTRICSHOCKER:
			case SC_MAGNETICFIELD:

				return 0;
		}
	}

	//Before overlapping fail, one must check for status cured.
	switch (type) {
	case SC_BLESSING:
		//TO-DO Blessing and Agi up should do 1 damage against players on Undead Status, even on PvM
		//but cannot be plagiarized (this requires aegis investigation on packets and official behavior) [Brainstorm]
		if ((!undead_flag && status->race!=RC_DEMON) || bl->type == BL_PC) {
			status_change_end(bl, SC_CURSE, INVALID_TIMER);
			if (sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
				status_change_end(bl, SC_STONE, INVALID_TIMER);
		}
		break;
	case SC_INCREASEAGI:
		status_change_end(bl, SC_DECREASEAGI, INVALID_TIMER);
		break;
	case SC_QUAGMIRE:
		status_change_end(bl, SC_CONCENTRATE, INVALID_TIMER);
		status_change_end(bl, SC_TRUESIGHT, INVALID_TIMER);
		status_change_end(bl, SC_WINDWALK, INVALID_TIMER);
		//Also blocks the ones below...
	case SC_DECREASEAGI:
		status_change_end(bl, SC_CARTBOOST, INVALID_TIMER);
		//Also blocks the ones below...
	case SC_DONTFORGETME:
		status_change_end(bl, SC_INCREASEAGI, INVALID_TIMER);
		status_change_end(bl, SC_ADRENALINE, INVALID_TIMER);
		status_change_end(bl, SC_ADRENALINE2, INVALID_TIMER);
		status_change_end(bl, SC_SPEARQUICKEN, INVALID_TIMER);
		status_change_end(bl, SC_TWOHANDQUICKEN, INVALID_TIMER);
		status_change_end(bl, SC_ONEHAND, INVALID_TIMER);
		status_change_end(bl, SC_MERC_QUICKEN, INVALID_TIMER);
		status_change_end(bl, SC_ACCELERATION, INVALID_TIMER);
		break;
	case SC_ONEHAND:
	  	//Removes the Aspd potion effect, as reported by Vicious. [Skotlex]
		status_change_end(bl, SC_ASPDPOTION0, INVALID_TIMER);
		status_change_end(bl, SC_ASPDPOTION1, INVALID_TIMER);
		status_change_end(bl, SC_ASPDPOTION2, INVALID_TIMER);
		status_change_end(bl, SC_ASPDPOTION3, INVALID_TIMER);
		break;
	case SC_MAXOVERTHRUST:
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
		status_change_end(bl, SC_CLOSECONFINE, INVALID_TIMER);
		status_change_end(bl, SC_CLOSECONFINE2, INVALID_TIMER);
		break;
	case SC__BLOODYLUST:
	case SC_BERSERK:
		if(battle_config.berserk_cancels_buffs) {
			status_change_end(bl, SC_ONEHAND, INVALID_TIMER);
			status_change_end(bl, SC_TWOHANDQUICKEN, INVALID_TIMER);
			status_change_end(bl, SC_CONCENTRATION, INVALID_TIMER);
			status_change_end(bl, SC_PARRYING, INVALID_TIMER);
			status_change_end(bl, SC_AURABLADE, INVALID_TIMER);
			status_change_end(bl, SC_MERC_QUICKEN, INVALID_TIMER);
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
		if(sc->data[SC_DECREASEAGI])
		{	//Cancel Decrease Agi, but take no further effect [Skotlex]
			status_change_end(bl, SC_DECREASEAGI, INVALID_TIMER);
			return 0;
		}
		break;
	case SC_FUSION:
		status_change_end(bl, SC_SPIRIT, INVALID_TIMER);
		break;
	case SC_ADJUSTMENT:
		status_change_end(bl, SC_MADNESSCANCEL, INVALID_TIMER);
		break;
	case SC_MADNESSCANCEL:
		status_change_end(bl, SC_ADJUSTMENT, INVALID_TIMER);
		break;
	//NPC_CHANGEUNDEAD will debuff Blessing and Agi Up
	case SC_CHANGEUNDEAD:
		status_change_end(bl, SC_BLESSING, INVALID_TIMER);
		status_change_end(bl, SC_INCREASEAGI, INVALID_TIMER);
		break;
	case SC_STRFOOD:
		status_change_end(bl, SC_FOOD_STR_CASH, INVALID_TIMER);
		break;
	case SC_AGIFOOD:
		status_change_end(bl, SC_FOOD_AGI_CASH, INVALID_TIMER);
		break;
	case SC_VITFOOD:
		status_change_end(bl, SC_FOOD_VIT_CASH, INVALID_TIMER);
		break;
	case SC_INTFOOD:
		status_change_end(bl, SC_FOOD_INT_CASH, INVALID_TIMER);
		break;
	case SC_DEXFOOD:
		status_change_end(bl, SC_FOOD_DEX_CASH, INVALID_TIMER);
		break;
	case SC_LUKFOOD:
		status_change_end(bl, SC_FOOD_LUK_CASH, INVALID_TIMER);
		break;
	case SC_FOOD_STR_CASH:
		status_change_end(bl, SC_STRFOOD, INVALID_TIMER);
		break;
	case SC_FOOD_AGI_CASH:
		status_change_end(bl, SC_AGIFOOD, INVALID_TIMER);
		break;
	case SC_FOOD_VIT_CASH:
		status_change_end(bl, SC_VITFOOD, INVALID_TIMER);
		break;
	case SC_FOOD_INT_CASH:
		status_change_end(bl, SC_INTFOOD, INVALID_TIMER);
		break;
	case SC_FOOD_DEX_CASH:
		status_change_end(bl, SC_DEXFOOD, INVALID_TIMER);
		break;
	case SC_FOOD_LUK_CASH:
		status_change_end(bl, SC_LUKFOOD, INVALID_TIMER);
		break;
	case SC_FIGHTINGSPIRIT:
		status_change_end(bl, type, INVALID_TIMER); // Remove previous one.
		break;
	case SC_MARSHOFABYSS:
		status_change_end(bl, SC_INCAGI, INVALID_TIMER);
		status_change_end(bl, SC_WINDWALK, INVALID_TIMER);
		status_change_end(bl, SC_ASPDPOTION0, INVALID_TIMER);
		status_change_end(bl, SC_ASPDPOTION1, INVALID_TIMER);
		status_change_end(bl, SC_ASPDPOTION2, INVALID_TIMER);
		status_change_end(bl, SC_ASPDPOTION3, INVALID_TIMER);
		break;
	case SC_SWINGDANCE:
	case SC_SYMPHONYOFLOVER:
	case SC_MOONLITSERENADE:
	case SC_RUSHWINDMILL:
	case SC_ECHOSONG:
        case SC_HARMONIZE: //group A doesn't overlap
            if (type != SC_SWINGDANCE) status_change_end(bl, SC_SWINGDANCE, INVALID_TIMER);
            if (type != SC_SYMPHONYOFLOVER) status_change_end(bl, SC_SYMPHONYOFLOVER, INVALID_TIMER);
            if (type != SC_MOONLITSERENADE) status_change_end(bl, SC_MOONLITSERENADE, INVALID_TIMER);
            if (type != SC_RUSHWINDMILL) status_change_end(bl, SC_RUSHWINDMILL, INVALID_TIMER);
            if (type != SC_ECHOSONG) status_change_end(bl, SC_ECHOSONG, INVALID_TIMER);
            if (type != SC_HARMONIZE) status_change_end(bl, SC_HARMONIZE, INVALID_TIMER);
            break;
        case SC_VOICEOFSIREN:
        case SC_DEEPSLEEP:
        case SC_GLOOMYDAY:
        case SC_SONGOFMANA:
        case SC_DANCEWITHWUG:
        case SC_SATURDAYNIGHTFEVER:
        case SC_LERADSDEW:
        case SC_MELODYOFSINK:
        case SC_BEYONDOFWARCRY:
        case SC_UNLIMITEDHUMMINGVOICE: //group B
            if (type != SC_VOICEOFSIREN) status_change_end(bl, SC_VOICEOFSIREN, INVALID_TIMER);
            if (type != SC_DEEPSLEEP) status_change_end(bl, SC_DEEPSLEEP, INVALID_TIMER);
            if (type != SC_LERADSDEW) status_change_end(bl, SC_LERADSDEW, INVALID_TIMER);
            if (type != SC_MELODYOFSINK) status_change_end(bl, SC_MELODYOFSINK, INVALID_TIMER);
            if (type != SC_BEYONDOFWARCRY) status_change_end(bl, SC_BEYONDOFWARCRY, INVALID_TIMER);
            if (type != SC_UNLIMITEDHUMMINGVOICE) status_change_end(bl, SC_UNLIMITEDHUMMINGVOICE, INVALID_TIMER);
            if (type != SC_GLOOMYDAY) {
                status_change_end(bl, SC_GLOOMYDAY, INVALID_TIMER);
                status_change_end(bl, SC_GLOOMYDAY_SK, INVALID_TIMER);
            }
            if (type != SC_SONGOFMANA) status_change_end(bl, SC_SONGOFMANA, INVALID_TIMER);
            if (type != SC_DANCEWITHWUG) status_change_end(bl, SC_DANCEWITHWUG, INVALID_TIMER);
            if (type != SC_SATURDAYNIGHTFEVER) {
                if (sc->data[SC_SATURDAYNIGHTFEVER]) {
                    sc->data[SC_SATURDAYNIGHTFEVER]->val2 = 0; //mark to not lose hp
                    status_change_end(bl, SC_SATURDAYNIGHTFEVER, INVALID_TIMER);
                }
            }
            break;
	case SC_REFLECTSHIELD:
		status_change_end(bl, SC_REFLECTDAMAGE, INVALID_TIMER);
		break;
	case SC_REFLECTDAMAGE:
		status_change_end(bl, SC_REFLECTSHIELD, INVALID_TIMER);
		break;
	case SC_SHIELDSPELL_DEF:
	case SC_SHIELDSPELL_MDEF:
	case SC_SHIELDSPELL_REF:
		status_change_end(bl, SC_MAGNIFICAT, INVALID_TIMER);
		if( type != SC_SHIELDSPELL_DEF )
			status_change_end(bl, SC_SHIELDSPELL_DEF, INVALID_TIMER);
		if( type != SC_SHIELDSPELL_MDEF )
			status_change_end(bl, SC_SHIELDSPELL_MDEF, INVALID_TIMER);
		if( type != SC_SHIELDSPELL_REF )
			status_change_end(bl, SC_SHIELDSPELL_REF, INVALID_TIMER);
		break;
	case SC_GT_ENERGYGAIN:
	case SC_GT_CHANGE:
	case SC_GT_REVITALIZE:
		if( type != SC_GT_REVITALIZE )
			status_change_end(bl, SC_GT_REVITALIZE, INVALID_TIMER);
		if( type != SC_GT_ENERGYGAIN )
			status_change_end(bl, SC_GT_ENERGYGAIN, INVALID_TIMER);
		if( type != SC_GT_CHANGE )
			status_change_end(bl, SC_GT_CHANGE, INVALID_TIMER);
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
			case SC_MERC_FLEEUP:
			case SC_MERC_ATKUP:
			case SC_MERC_HPUP:
			case SC_MERC_SPUP:
			case SC_MERC_HITUP:
				if( sce->val1 > val1 )
					val1 = sce->val1;
				break;
			case SC_ADRENALINE:
			case SC_ADRENALINE2:
			case SC_WEAPONPERFECTION:
			case SC_OVERTHRUST:
				if (sce->val2 > val2)
					return 0;
				break;
			case SC_S_LIFEPOTION:
			case SC_L_LIFEPOTION:
			case SC_BOSSMAPINFO:
			case SC_STUN:
			case SC_SLEEP:
			case SC_POISON:
			case SC_CURSE:
			case SC_SILENCE:
			case SC_CONFUSION:
			case SC_BLIND:
			case SC_BLEEDING:
			case SC_DPOISON:
			case SC_CLOSECONFINE2: //Can't be re-closed in.
			case SC_MARIONETTE:
			case SC_MARIONETTE2:
			case SC_NOCHAT:
			case SC_CHANGE: //Otherwise your Hp/Sp would get refilled while still within effect of the last invocation.
			case SC__INVISIBILITY:
			case SC__ENERVATION:
			case SC__GROOMY:
			case SC__IGNORANCE:
			case SC__LAZINESS:
			case SC__WEAKNESS:
			case SC__UNLUCKY:
				return 0;
			case SC_COMBO:
			case SC_DANCING:
			case SC_DEVOTION:
			case SC_ASPDPOTION0:
			case SC_ASPDPOTION1:
			case SC_ASPDPOTION2:
			case SC_ASPDPOTION3:
			case SC_ATKPOTION:
			case SC_MATKPOTION:
			case SC_ENCHANTARMS:
			case SC_ARMOR_ELEMENT:
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
					iTimer->delete_timer(sce->val4,kaahi_heal_timer);
					sce->val4 = INVALID_TIMER;
				}
				break;
			case SC_JAILED:
				//When a player is already jailed, do not edit the jail data.
				val2 = sce->val2;
				val3 = sce->val3;
				val4 = sce->val4;
				break;
			case SC_LERADSDEW:
				if (sc && (sc->data[SC_BERSERK] || sc->data[SC__BLOODYLUST]))
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

	vd = status_get_viewdata(bl);
	calc_flag = StatusChangeFlagTable[type];
	if(!(flag&4)) { //&4 - Do not parse val settings when loading SCs
		switch(type) {
			case SC_DECREASEAGI:
			case SC_INCREASEAGI:
				val2 = 2 + val1; //Agi change
				break;
			case SC_ENDURE:
				val2 = 7; // Hit-count [Celest]
				if( !(flag&1) && (bl->type&(BL_PC|BL_MER)) && !map_flag_gvg(bl->m) && !map[bl->m].flag.battleground && !val4 )
				{
					struct map_session_data *tsd;
					if( sd )
					{
						int i;
						for( i = 0; i < 5; i++ )
						{
							if( sd->devotion[i] && (tsd = iMap->id2sd(sd->devotion[i])) )
								status_change_start(&tsd->bl, type, 10000, val1, val2, val3, val4, tick, 1);
						}
					}
					else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
						status_change_start(&tsd->bl, type, 10000, val1, val2, val3, val4, tick, 1);
				}
				//val4 signals infinite endure (if val4 == 2 it is infinite endure from Berserk)
				if( val4 )
					tick = -1;
				break;
			case SC_AUTOBERSERK:
				if (status->hp < status->max_hp>>2 &&
					(!sc->data[SC_PROVOKE] || sc->data[SC_PROVOKE]->val2==0))
						sc_start4(bl,SC_PROVOKE,100,10,1,0,0,60000);
				tick = -1;
				break;
			case SC_SIGNUMCRUCIS:
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
	#ifndef RENEWAL_EDP
				val3 = 50*(val1+1); //Damage increase (+50 +50*lv%)
	#endif
				if( sd )//[Ind] - iROwiki says each level increases its duration by 3 seconds
					tick += iPc->checkskill(sd,GC_RESEARCHNEWPOISON)*3000;
				break;
			case SC_POISONREACT:
				val2=(val1+1)/2 + val1/10; // Number of counters [Skotlex]
				val3=50; // + 5*val1; //Chance to counter. [Skotlex]
				break;
			case SC_MAGICROD:
				val2 = val1*20; //SP gained
				break;
			case SC_KYRIE:
				val2 = (int64)status->max_hp * (val1 * 2 + 10) / 100; //%Max HP to absorb
				val3 = (val1 / 2 + 5); //Hits
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
			case SC_ENCPOISON:
				val2= 250+50*val1;	//Poisoning Chance (2.5+0.5%) in 1/10000 rate
			case SC_ASPERSIO:
			case SC_FIREWEAPON:
			case SC_WATERWEAPON:
			case SC_WINDWEAPON:
			case SC_EARTHWEAPON:
			case SC_SHADOWWEAPON:
			case SC_GHOSTWEAPON:
				skill->enchant_elemental_end(bl,type);
				break;
			case SC_ELEMENTALCHANGE:
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
				if( !(flag&1) && (bl->type&(BL_PC|BL_MER)) )
				{
					struct map_session_data *tsd;
					if( sd )
					{
						int i;
						for( i = 0; i < 5; i++ )
						{
							if( sd->devotion[i] && (tsd = iMap->id2sd(sd->devotion[i])) )
								status_change_start(&tsd->bl, type, 10000, val1, val2, 0, 0, tick, 1);
						}
					}
					else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
						status_change_start(&tsd->bl, type, 10000, val1, val2, 0, 0, tick, 1);
				}
				break;
			case SC_STRIPWEAPON:
				if (!sd) //Watk reduction
					val2 = 25;
				break;
			case SC_STRIPSHIELD:
				if (!sd) //Def reduction
					val2 = 15;
				break;
			case SC_STRIPARMOR:
				if (!sd) //Vit reduction
					val2 = 40;
				break;
			case SC_STRIPHELM:
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
				if (status->def_ele != ELE_FIRE)
					val2 = 0;
	#endif
				break;
			case SC_VIOLENTGALE:
				val2 = val1*3; //Flee increase
			#ifndef RENEWAL
				if (status->def_ele != ELE_WIND)
					val2 = 0;
			#endif
				break;
			case SC_DELUGE:
				val2 = deluge_eff[val1-1]; //HP increase
	#ifndef RENEWAL
				if(status->def_ele != ELE_WATER)
					val2 = 0;
	#endif
				break;
			case SC_SUITON:
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
			case SC_ONEHAND:
			case SC_TWOHANDQUICKEN:
				val2 = 300;
				if (val1 > 10) //For boss casted skills [Skotlex]
					val2 += 20*(val1-10);
				break;
			case SC_MERC_QUICKEN:
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
					clif->status_change(bl,SI_MOONLIT,1,tick,0, 0, 0);
				val1|= (val3<<16);
				val3 = tick/1000; //Tick duration
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_LONGING:
				val2 = 500-100*val1; //Aspd penalty.
				break;
			case SC_EXPLOSIONSPIRITS:
				val2 = 75 + 25*val1; //Cri bonus
				break;

			case SC_ASPDPOTION0:
			case SC_ASPDPOTION1:
			case SC_ASPDPOTION2:
			case SC_ASPDPOTION3:
				val2 = 50*(2+type-SC_ASPDPOTION0);
				break;

			case SC_WEDDING:
			case SC_XMAS:
			case SC_SUMMER:
			case SC_HANBOK:
				if (!vd) return 0;
				//Store previous values as they could be removed.
				unit_stop_attack(bl);
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
				tick = max(tick, 1000); //Min time
				calc_flag = 0; //Actual status changes take effect on petrified state.
				break;

			case SC_DPOISON:
			//Lose 10/15% of your life as long as it doesn't brings life below 25%
			if (status->hp > status->max_hp>>2) {
				int diff = status->max_hp*(bl->type==BL_PC?10:15)/100;
				if (status->hp - diff < status->max_hp>>2)
					diff = status->hp - (status->max_hp>>2);
				if( val2 && bl->type == BL_MOB ) {
					struct block_list* src = iMap->id2bl(val2);
					if( src )
						mob_log_damage((TBL_MOB*)bl,src,diff);
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
				val4 = (type == SC_DPOISON) ? 3 + status->max_hp/50 : 3 + status->max_hp*3/200;
			else
				val4 = (type == SC_DPOISON) ? 3 + status->max_hp/100 : 3 + status->max_hp/200;

			break;
			case SC_CONFUSION:
				clif->emotion(bl,E_WHAT);
				break;
			case SC_BLEEDING:
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
			case SC_BOSSMAPINFO:
				if( sd != NULL )
				{
					struct mob_data *boss_md = iMap->getmob_boss(bl->m); // Search for Boss on this Map
					if( boss_md == NULL || boss_md->bl.prev == NULL )
					{ // No MVP on this map - MVP is dead
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
				if (sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_ROGUE)
					val3 -= 40;
				val4 = 10+val1*2; //SP cost.
				if (map_flag_gvg(bl->m) || map[bl->m].flag.battleground) val4 *= 5;
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
			case SC_SIGHTBLASTER:
				val3 = skill->get_splash(val2, val1); //Val2 should bring the skill-id.
				val2 = tick/250;
				tick_time = 10; // [GodLesZ] tick time
				break;

			//Permanent effects.
			case SC_AETERNA:
			case SC_MODECHANGE:
			case SC_WEIGHT50:
			case SC_WEIGHT90:
			case SC_BROKENWEAPON:
			case SC_BROKENARMOR:
			case SC_READYSTORM:
			case SC_READYDOWN:
			case SC_READYCOUNTER:
			case SC_READYTURN:
			case SC_DODGE:
			case SC_PUSH_CART:
			case SC_ALL_RIDING:
				tick = -1;
				break;

			case SC_AUTOGUARD:
				if( !(flag&1) )
				{
					struct map_session_data *tsd;
					int i,t;
					for( i = val2 = 0; i < val1; i++)
					{
						t = 5-(i>>1);
						val2 += (t < 0)? 1:t;
					}

					if( bl->type&(BL_PC|BL_MER) )
					{
						if( sd )
						{
							for( i = 0; i < 5; i++ )
							{
								if( sd->devotion[i] && (tsd = iMap->id2sd(sd->devotion[i])) )
									status_change_start(&tsd->bl, type, 10000, val1, val2, 0, 0, tick, 1);
							}
						}
						else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
							status_change_start(&tsd->bl, type, 10000, val1, val2, 0, 0, tick, 1);
					}
				}
				break;

			case SC_DEFENDER:
				if (!(flag&1))
				{
					val2 = 5 + 15*val1; //Damage reduction
					val3 = 0; // unused, previously speed adjustment
					val4 = 250 - 50*val1; //Aspd adjustment

					if (sd)
					{
						struct map_session_data *tsd;
						int i;
						for (i = 0; i < 5; i++)
						{	//See if there are devoted characters, and pass the status to them. [Skotlex]
							if (sd->devotion[i] && (tsd = iMap->id2sd(sd->devotion[i])))
								status_change_start(&tsd->bl,type,10000,val1,5+val1*5,val3,val4,tick,1);
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
					sc_start2(bl,SC_BLEEDING,100,val1,val3,skill->get_time2(status_sc2skill(type),val1));
				break;

			case SC_BERSERK:
				if (!sc->data[SC_ENDURE] || !sc->data[SC_ENDURE]->val4)
					sc_start4(bl, SC_ENDURE, 100,10,0,0,2, tick);
			case SC__BLOODYLUST:
				//HP healing is performing after the calc_status call.
				//Val2 holds HP penalty
				if (!val4) val4 = skill->get_time2(status_sc2skill(type),val1);
				if (!val4) val4 = 10000; //Val4 holds damage interval
				val3 = tick/val4; //val3 holds skill duration
				tick_time = val4; // [GodLesZ] tick time
				break;

			case SC_GOSPEL:
				if(val4 == BCT_SELF) {	// self effect
					val2 = tick/10000;
					tick_time = 10000; // [GodLesZ] tick time
					status_change_clear_buffs(bl,3); //Remove buffs/debuffs
				}
				break;

			case SC_MARIONETTE:
			{
				int stat;

				val3 = 0;
				val4 = 0;
				stat = ( sd ? sd->status.str : status_get_base_status(bl)->str ) / 2; val3 |= cap_value(stat,0,0xFF)<<16;
				stat = ( sd ? sd->status.agi : status_get_base_status(bl)->agi ) / 2; val3 |= cap_value(stat,0,0xFF)<<8;
				stat = ( sd ? sd->status.vit : status_get_base_status(bl)->vit ) / 2; val3 |= cap_value(stat,0,0xFF);
				stat = ( sd ? sd->status.int_: status_get_base_status(bl)->int_) / 2; val4 |= cap_value(stat,0,0xFF)<<16;
				stat = ( sd ? sd->status.dex : status_get_base_status(bl)->dex ) / 2; val4 |= cap_value(stat,0,0xFF)<<8;
				stat = ( sd ? sd->status.luk : status_get_base_status(bl)->luk ) / 2; val4 |= cap_value(stat,0,0xFF);
				break;
			}
			case SC_MARIONETTE2:
			{
				int stat,max_stat;
				// fetch caster information
				struct block_list *pbl = iMap->id2bl(val1);
				struct status_change *psc = pbl?status_get_sc(pbl):NULL;
				struct status_change_entry *psce = psc?psc->data[SC_MARIONETTE]:NULL;
				// fetch target's stats
				struct status_data* status = status_get_status_data(bl); // battle status

				if (!psce)
					return 0;

				val3 = 0;
				val4 = 0;
				max_stat = battle_config.max_parameter; //Cap to 99 (default)
				stat = (psce->val3 >>16)&0xFF; stat = min(stat, max_stat - status->str ); val3 |= cap_value(stat,0,0xFF)<<16;
				stat = (psce->val3 >> 8)&0xFF; stat = min(stat, max_stat - status->agi ); val3 |= cap_value(stat,0,0xFF)<<8;
				stat = (psce->val3 >> 0)&0xFF; stat = min(stat, max_stat - status->vit ); val3 |= cap_value(stat,0,0xFF);
				stat = (psce->val4 >>16)&0xFF; stat = min(stat, max_stat - status->int_); val4 |= cap_value(stat,0,0xFF)<<16;
				stat = (psce->val4 >> 8)&0xFF; stat = min(stat, max_stat - status->dex ); val4 |= cap_value(stat,0,0xFF)<<8;
				stat = (psce->val4 >> 0)&0xFF; stat = min(stat, max_stat - status->luk ); val4 |= cap_value(stat,0,0xFF);
				break;
			}
			case SC_REJECTSWORD:
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

			case SC_REGENERATION:
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

				if( (d_bl = iMap->id2bl(val1)) && (d_sc = status_get_sc(d_bl)) && d_sc->count )
				{ // Inherits Status From Source
					const enum sc_type types[] = { SC_AUTOGUARD, SC_DEFENDER, SC_REFLECTSHIELD, SC_ENDURE };
					enum sc_type type2;
					int i = (map_flag_gvg(bl->m) || map[bl->m].flag.battleground)?2:3;
					while( i >= 0 )
					{
						type2 = types[i];
						if( d_sc->data[type2] )
							sc_start(bl, type2, 100, d_sc->data[type2]->val1, skill->get_time(status_sc2skill(type2),d_sc->data[type2]->val1));
						i--;
					}
				}
				break;
			}

			case SC_COMA: //Coma. Sends a char to 1HP. If val2, do not zap sp
				if( val3 && bl->type == BL_MOB ) {
					struct block_list* src = iMap->id2bl(val3);
					if( src )
						mob_log_damage((TBL_MOB*)bl,src,status->hp - 1);
				}
				status_zap(bl, status->hp-1, val2?0:status->sp);
				return 1;
				break;
			case SC_CLOSECONFINE2:
			{
				struct block_list *src = val2?iMap->id2bl(val2):NULL;
				struct status_change *sc2 = src?status_get_sc(src):NULL;
				struct status_change_entry *sce2 = sc2?sc2->data[SC_CLOSECONFINE]:NULL;
				if (src && sc2) {
					if (!sce2) //Start lock on caster.
						sc_start4(src,SC_CLOSECONFINE,100,val1,1,0,0,tick+1000);
					else { //Increase count of locked enemies and refresh time.
						(sce2->val2)++;
						iTimer->delete_timer(sce2->timer, status_change_timer);
						sce2->timer = iTimer->add_timer(iTimer->gettick()+tick+1000, status_change_timer, src->id, SC_CLOSECONFINE);
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

			case SC_COMBO: {
					//val1: Skill ID
					//val2: When given, target (for autotargetting skills)
					//val3: When set, this combo time should NOT delay attack/movement
					//val3: TK: Last used kick
					//val4: TK: Combo time
					struct unit_data *ud = unit_bl2ud(bl);
					if (ud && !val3) {
						tick += 300 * battle_config.combo_delay_rate/100;
						ud->attackabletime = iTimer->gettick()+tick;
						unit_set_walkdelay(bl, iTimer->gettick(), tick, 1);
					}
					val3 = 0;
					val4 = tick;
				}
				break;
			case SC_EARTHSCROLL:
				val2 = 11-val1; //Chance to consume: 11-skill_lv%
				break;
			case SC_RUN:
				val4 = iTimer->gettick(); //Store time at which you started running.
				tick = -1;
				break;
			case SC_KAAHI:
				val2 = 200*val1; //HP heal
				val3 = 5*val1; //SP cost
				val4 = INVALID_TIMER;	//Kaahi Timer.
				break;
			case SC_BLESSING:
				if ((!undead_flag && status->race!=RC_DEMON) || bl->type == BL_PC)
					val2 = val1;
				else
					val2 = 0; //0 -> Half stat.
				break;
			case SC_TRICKDEAD:
				if (vd) vd->dead_sit = 1;
				tick = -1;
				break;
			case SC_CONCENTRATE:
				val2 = 2 + val1;
				if (sd) { //Store the card-bonus data that should not count in the %
					val3 = sd->param_bonus[1]; //Agi
					val4 = sd->param_bonus[4]; //Dex
				} else {
					val3 = val4 = 0;
				}
				break;
			case SC_MAXOVERTHRUST:
				val2 = 20*val1; //Power increase
				break;
			case SC_OVERTHRUST:
				//val2 holds if it was casted on self, or is bonus received from others
				val3 = 5*val1; //Power increase
				if(sd && iPc->checkskill(sd,BS_HILTBINDING)>0)
					tick += tick / 10;
				break;
			case SC_ADRENALINE2:
			case SC_ADRENALINE:
				val3 = (val2) ? 300 : 200; // aspd increase
			case SC_WEAPONPERFECTION:
				if(sd && iPc->checkskill(sd,BS_HILTBINDING)>0)
					tick += tick / 10;
				break;
			case SC_CONCENTRATION:
				val2 = 5*val1; //Batk/Watk Increase
				val3 = 10*val1; //Hit Increase
				val4 = 5*val1; //Def reduction
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
				val2 = (status_get_lv(bl) + status->dex + status->luk)/2; //def increase
				break;
			case SC_MOON_COMFORT:
				val2 = (status_get_lv(bl) + status->dex + status->luk)/10; //flee increase
				break;
			case SC_STAR_COMFORT:
				val2 = (status_get_lv(bl) + status->dex + status->luk); //Aspd increase
				break;
			case SC_QUAGMIRE:
				val2 = (sd?5:10)*val1; //Agi/Dex decrease.
				break;

			// gs_something1 [Vicious]
			case SC_GATLINGFEVER:
				val2 = 20*val1; //Aspd increase
				val3 = 20+10*val1; //Batk increase
				val4 = 5*val1; //Flee decrease
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
			case SC_AVOID:
				//val2 = 10*val1; //Speed change rate.
				break;
			case SC_DEFENCE:
				val2 = 2*val1; //Def bonus
				break;
			case SC_BLOODLUST:
				val2 = 20+10*val1; //Atk rate change.
				val3 = 3*val1; //Leech chance
				val4 = 20; //Leech percent
				break;
			case SC_FLEET:
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
						int pos =  (bl->x&0xFFFF)|(bl->y<<16), //Current Coordinates
						map =  sd->mapindex; //Current Map
						//1. Place in Jail (val2 -> Jail Map, val3 -> x, val4 -> y
						iPc->setpos(sd,(unsigned short)val2,val3,val4, CLR_TELEPORT);
						//2. Set restore point (val3 -> return map, val4 return coords
						val3 = map;
						val4 = pos;
					} else if (!val3 || val3 == sd->mapindex) { //Use save point.
						val3 = sd->status.save_point.map;
						val4 = (sd->status.save_point.x&0xFFFF)
							|(sd->status.save_point.y<<16);
					}
				}
				break;
			case SC_UTSUSEMI:
				val2=(val1+1)/2; // number of hits blocked
				val3=skill->get_blewcount(NJ_UTSUSEMI, val1); //knockback value.
				break;
			case SC_BUNSINJYUTSU:
				val2=(val1+1)/2; // number of hits blocked
				break;
			case SC_CHANGE:
				val2= 30*val1; //Vit increase
				val3= 20*val1; //Int increase
				break;
			case SC_SWOO:
				if(status->mode&MD_BOSS)
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

			case SC_ARMORCHANGE:
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
			case SC_EXPBOOST:
			case SC_JEXPBOOST:
				if (val1 < 0)
					val1 = 0;
				break;
			case SC_INCFLEE2:
			case SC_INCCRI:
				val2 = val1*10; //Actual boost (since 100% = 1000)
				break;
			case SC_SUFFRAGIUM:
				val2 = 15 * val1; //Speed cast decrease
				break;
			case SC_INCHEALRATE:
				if (val1 < 1)
					val1 = 1;
				break;
			case SC_HALLUCINATION:
				val2 = 5+val1; //Factor by which displayed damage is increased by
				break;
			case SC_DOUBLECAST:
				val2 = 30+10*val1; //Trigger rate
				break;
			case SC_KAIZEL:
				val2 = 10*val1; //% of life to be revived with
				break;
			// case SC_ARMOR_ELEMENT:
			// case SC_ARMOR_RESIST:
				// Mod your resistance against elements:
				// val1 = water | val2 = earth | val3 = fire | val4 = wind
				// break;
			//case ????:
				//Place here SCs that have no SCB_* data, no skill associated, no ICON
				//associated, and yet are not wrong/unknown. [Skotlex]
				//break;

			case SC_MERC_FLEEUP:
			case SC_MERC_ATKUP:
			case SC_MERC_HITUP:
				val2 = 15 * val1;
				break;
			case SC_MERC_HPUP:
			case SC_MERC_SPUP:
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
				val4 = tick / 2000; // Total Ticks to Burn!!
				tick_time = 2000; // [GodLesZ] tick time
				break;
			/**
			 * Rune Knight
			 **/
			case SC_DEATHBOUND:
				val2 = 500 + 100 * val1;
				break;
			case SC_STONEHARDSKIN:
				if( sd )
					val1 = sd->status.job_level * iPc->checkskill(sd, RK_RUNEMASTERY) / 4; //DEF/MDEF Increase
				break;
			case SC_FIGHTINGSPIRIT:
				val_flag |= 1|2;
				break;
			case SC_ABUNDANCE:
				val4 = tick / 10000;
				tick_time = 10000; // [GodLesZ] tick time
				break;
			case SC_GIANTGROWTH:
				val2 = 10; // Triple damage success rate.
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
				val_flag |= 1|2;
				break;
			case SC_POISONINGWEAPON:
				val_flag |= 1|2|4;
				break;
			case SC_WEAPONBLOCKING:
				val2 = 10 + 2 * val1; // Chance
				val4 = tick / 3000;
				tick_time = 3000; // [GodLesZ] tick time
				val_flag |= 1|2;
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
				status_change_start(bl,SC_BLIND,10000,val1,0,0,0,30000,11); // Blind status that last for 30 seconds
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
			case SC_ROLLINGCUTTER:
				val_flag |= 1;
				break;
			case SC_CLOAKINGEXCEED:
				val2 = ( val1 + 1 ) / 2; // Hits
				val3 = 90 + val1 * 10; // Walk speed
				val_flag |= 1|2|4;
				if (bl->type == BL_PC)
					val4 |= battle_config.pc_cloak_check_type&7;
				else
					val4 |= battle_config.monster_cloak_check_type&7;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_HALLUCINATIONWALK:
				val2 = 50 * val1; // Evasion rate of physical attacks. Flee
				val3 = 10 * val1; // Evasion rate of magical attacks.
				val_flag |= 1|2|4;
				break;
			case SC_WHITEIMPRISON:
				status_change_end(bl, SC_BURNING, INVALID_TIMER);
				status_change_end(bl, SC_FREEZING, INVALID_TIMER);
				status_change_end(bl, SC_FREEZE, INVALID_TIMER);
				status_change_end(bl, SC_STONE, INVALID_TIMER);
				break;
			case SC_FREEZING:
				status_change_end(bl, SC_BURNING, INVALID_TIMER);
				break;
			case SC_READING_SB:
				// val2 = sp reduction per second
				tick_time = 5000; // [GodLesZ] tick time
				break;
			case SC_SPHERE_1:
			case SC_SPHERE_2:
			case SC_SPHERE_3:
			case SC_SPHERE_4:
			case SC_SPHERE_5:
				if( !sd )
					return 0;	// Should only work on players.
				val4 = tick / 1000;
				if( val4 < 1 )
					val4 = 1;
				tick_time = 1000; // [GodLesZ] tick time
				val_flag |= 1;
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
			case SC_ELECTRICSHOCKER:
			case SC_CRYSTALIZE:
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
				val4 = iTimer->gettick(); //Store time at which you started running.
				tick = -1;
				break;
			case SC__SHADOWFORM: {
					struct map_session_data * s_sd = iMap->id2sd(val2);
					if( s_sd )
						s_sd->shadowform_id = bl->id;
					val4 = tick / 1000;
					val_flag |= 1|2|4;
					tick_time = 1000; // [GodLesZ] tick time
				}
				break;
			case SC__STRIPACCESSORY:
				if (!sd)
					val2 = 20;
				break;
			case SC__INVISIBILITY:
				val2 = 50 - 10 * val1; // ASPD
				val3 = 20 * val1; // CRITICAL
				val4 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
				val_flag |= 1|2;
				break;
			case SC__ENERVATION:
				val2 = 20 + 10 * val1; // ATK Reduction
				val_flag |= 1|2;
				if( sd ) iPc->delspiritball(sd,sd->spiritball,0);
				break;
			case SC__GROOMY:
				val2 = 20 + 10 * val1; //ASPD. Need to confirm if Movement Speed reduction is the same. [Jobbie]
				val3 = 20 * val1; //HIT
				val_flag |= 1|2|4;
				if( sd ) { // Removes Animals
					if( pc_isriding(sd) ) iPc->setriding(sd, 0);
					if( pc_isridingdragon(sd) ) iPc->setoption(sd, sd->sc.option&~OPTION_DRAGON);
					if( pc_iswug(sd) ) iPc->setoption(sd, sd->sc.option&~OPTION_WUG);
					if( pc_isridingwug(sd) ) iPc->setoption(sd, sd->sc.option&~OPTION_WUGRIDER);
					if( pc_isfalcon(sd) ) iPc->setoption(sd, sd->sc.option&~OPTION_FALCON);
					if( sd->status.pet_id > 0 ) pet_menu(sd, 3);
					if( homun_alive(sd->hd) ) homun->vaporize(sd,1);
					if( sd->md ) merc_delete(sd->md,3);
				}
				break;
			case SC__LAZINESS:
				val2 = 10 + 10 * val1; // Cast reduction
				val3 = 10 * val1; // Flee Reduction
				val_flag |= 1|2|4;
				break;
			case SC__UNLUCKY:
				val2 = 10 * val1; // Crit and Flee2 Reduction
				val_flag |= 1|2|4;
				break;
			case SC__WEAKNESS:
				val2 = 10 * val1;
				val_flag |= 1|2;
				// bypasses coating protection and MADO
				sc_start(bl,SC_STRIPWEAPON,100,val1,tick);
				sc_start(bl,SC_STRIPSHIELD,100,val1,tick);
				break;
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
				val_flag |= 1|2;
				val3 = 0;
				break;
			case SC_WARMER:
				status_change_end(bl, SC_FREEZE, INVALID_TIMER);
				status_change_end(bl, SC_FREEZING, INVALID_TIMER);
				status_change_end(bl, SC_CRYSTALIZE, INVALID_TIMER);
				break;
			case SC_STRIKING:
				val1 = 6 - val1;//spcost = 6 - level (lvl1:5 ... lvl 5: 1)
				val4 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_BLOODSUCKER:
				val4 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_VACUUM_EXTREME:
				tick -= (status->str / 20) * 1000;
				val4 = val3 = tick / 100;
				tick_time = 100; // [GodLesZ] tick time
				break;
			case SC_SWINGDANCE:
				val2 = 4 * val1; // Walk speed and aspd reduction.
				break;
			case SC_SYMPHONYOFLOVER:
			case SC_RUSHWINDMILL:
			case SC_ECHOSONG:
				val2 = 6 * val1;
				val2 += val3; //Adding 1% * Lesson Bonus
				val2 += (int)(val4*2/10); //Adding 0.2% per JobLevel
				break;
			case SC_MOONLITSERENADE:
				val2 = 10 * val1;
				break;
			case SC_HARMONIZE:
				val2 = 5 + 5 * val1;
				break;
			case SC_VOICEOFSIREN:
				val4 = tick / 2000;
				tick_time = 2000; // [GodLesZ] tick time
				break;
			case SC_DEEPSLEEP:
				val4 = tick / 2000;
				tick_time = 2000; // [GodLesZ] tick time
				break;
			case SC_SIRCLEOFNATURE:
				val2 = 1 + val1; //SP consume
				val3 = 40 * val1;	//HP recovery
				val4 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_SONGOFMANA:
				val3 = 10 + (2 * val2);
				val4 = tick/3000;
				tick_time = 3000; // [GodLesZ] tick time
				break;
			case SC_SATURDAYNIGHTFEVER:
				if (!val4) val4 = skill->get_time2(status_sc2skill(type),val1);
				if (!val4) val4 = 3000;
				val3 = tick/val4;
				tick_time = val4; // [GodLesZ] tick time
				break;
			case SC_GLOOMYDAY:
				val2 = 20 + 5 * val1; // Flee reduction.
				val3 = 15 + 5 * val1; // ASPD reduction.
				if( sd && rand()%100 < val1 ){ // (Skill Lv) %
					val4 = 1; // reduce walk speed by half.
					if( pc_isriding(sd) ) iPc->setriding(sd, 0);
					if( pc_isridingdragon(sd) ) iPc->setoption(sd, sd->sc.option&~OPTION_DRAGON);
				}
				break;
			case SC_GLOOMYDAY_SK:
				// Random number between [15 ~ (Voice Lesson Skill Level x 5) + (Skill Level x 10)] %.
				val2 = 15 + rand()%( (sd?iPc->checkskill(sd, WM_LESSON)*5:0) + val1*10 );
				break;
			case SC_SITDOWN_FORCE:
			case SC_BANANA_BOMB_SITDOWN:
				if( sd && !pc_issit(sd) )
				{
					pc_setsit(sd);
					skill->sit(sd,1);
					clif->sitting(bl);
				}
				break;
			case SC_DANCEWITHWUG:
				val3 = (5 * val1) + (1 * val2); //Still need official value.
				break;
			case SC_LERADSDEW:
				val3 = (5 * val1) + (1 * val2);
				break;
			case SC_MELODYOFSINK:
				val3 = (5 * val1) + (1 * val2);
				break;
			case SC_BEYONDOFWARCRY:
				val3 = (5 * val1) + (1 * val2);
				break;
			case SC_UNLIMITEDHUMMINGVOICE:
				{
					struct unit_data *ud = unit_bl2ud(bl);
					if( ud == NULL ) return 0;
					ud->state.skillcastcancel = 0;
					val3 = 15 - (2 * val2);
				}
				break;
			case SC_REFLECTDAMAGE:
				val2 = 15 + 5 * val1;
				val3 = (val1==5)?20:(val1+4)*2; // SP consumption
				val4 = tick/10000;
				tick_time = 10000; // [GodLesZ] tick time
				break;
			case SC_FORCEOFVANGUARD: // This is not the official way to handle it but I think we should use it. [pakpil]
				val2 = 20 + 12 * (val1 - 1); // Chance
				val3 = 5 + (2 * val1); // Max rage counters
				tick = -1; //endless duration in the client
				tick_time = 6000; // [GodLesZ] tick time
				val_flag |= 1|2|4;
				break;
			case SC_EXEEDBREAK:
				val1 *= 150; // 150 * skill_lv
				if( sd && sd->inventory_data[sd->equip_index[EQI_HAND_R]] ) {  // Chars.
					val1 += (sd->inventory_data[sd->equip_index[EQI_HAND_R]]->weight/10 * sd->inventory_data[sd->equip_index[EQI_HAND_R]]->wlv * status_get_lv(bl) / 100);
				val1 += 15 * (sd ? sd->status.job_level:50) + 100;
				}
				else	// Mobs
					val1 += (400 * status_get_lv(bl) / 100) + (15 * (status_get_lv(bl) / 2));	// About 1138% at mob_lvl 99. Is an aproximation to a standard weapon. [pakpil]
				break;
			case SC_PRESTIGE:	// Bassed on suggested formula in iRO Wiki and some test, still need more test. [pakpil]
				val2 = ((status->int_ + status->luk) / 6) + 5;	// Chance to evade magic damage.
				val1 *= 15; // Defence added
				if( sd )
					val1 += 10 * iPc->checkskill(sd,CR_DEFENDER);
				val_flag |= 1|2;
				break;
			case SC_BANDING:
				tick_time = 5000; // [GodLesZ] tick time
				val_flag |= 1;
				break;
			case SC_SHIELDSPELL_DEF:
			case SC_SHIELDSPELL_MDEF:
			case SC_SHIELDSPELL_REF:
				val_flag |= 1|2;
				break;
			case SC_MAGNETICFIELD:
				val3 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
				break;
			case SC_INSPIRATION:
				if( sd )
				{
					val2 = (40 * val1) + (3 * sd->status.job_level); // ATK bonus
					val3 = (sd->status.job_level / 10) * 2 + 12; // All stat bonus
				}
				val4 = tick / 1000;
				tick_time = 1000; // [GodLesZ] tick time
				status_change_clear_buffs(bl,3); //Remove buffs/debuffs
				break;
			case SC_SPELLFIST:
			case SC_CURSEDCIRCLE_ATKER:
				val_flag |= 1|2|4;
				break;
			case SC_CRESCENTELBOW:
				val2 = 94 + val1;
				val_flag |= 1|2;
				break;
			case SC_LIGHTNINGWALK: //  [(Job Level / 2) + (40 + 5 * Skill Level)] %
				val1 = (sd?sd->status.job_level:2)/2 + 40 + 5 * val1;
				val_flag |= 1;
				break;
			case SC_RAISINGDRAGON:
				val3 = tick / 5000;
				tick_time = 5000; // [GodLesZ] tick time
				break;
			case SC_GT_CHANGE:
				{// take note there is no def increase as skill desc says. [malufett]
					struct block_list * src;
					val3 = status->agi * val1 / 60; // ASPD increase: [(Target AGI x Skill Level) / 60] %
					if( (src = iMap->id2bl(val2)) )
						val4 = ( 200/status_get_int(src) ) * val1;// MDEF decrease: MDEF [(200 / Caster INT) x Skill Level]
				}
				break;
			case SC_GT_REVITALIZE:
				{// take note there is no vit,aspd,speed increase as skill desc says. [malufett]
					struct block_list * src;
					val3 = val1 * 30 + 150; // Natural HP recovery increase: [(Skill Level x 30) + 50] %
					if( (src = iMap->id2bl(val2)) ) // the stat def is not shown in the status window and it is process differently
						val4 = ( status_get_vit(src)/4 ) * val1; // STAT DEF increase: [(Caster VIT / 4) x Skill Level]
				}
				break;
			case SC_PYROTECHNIC_OPTION:
				val_flag |= 1|2|4;
				break;
			case SC_HEATER_OPTION:
				val2 = 120; // Watk. TODO: Renewal (Atk2)
				val3 = 33;	// % Increase effects.
				val4 = 3;	// Change into fire element.
				val_flag |= 1|2|4;
				break;
			case SC_TROPIC_OPTION:
				val2 = 180; // Watk. TODO: Renewal (Atk2)
				val3 = MG_FIREBOLT;
				break;
			case SC_AQUAPLAY_OPTION:
				val2 = 40;
				val_flag |= 1|2|4;
				break;
			case SC_COOLER_OPTION:
				val2 = 80;	// % Freezing chance
				val3 = 33;	// % increased damage
				val4 = 1;	// Change into water elemet
				val_flag |= 1|2|4;
				break;
			case SC_CHILLY_AIR_OPTION:
				val2 = 120; // Matk. TODO: Renewal (Matk1)
				val3 = MG_COLDBOLT;
				val_flag |= 1|2;
				break;
			case SC_GUST_OPTION:
				val_flag |= 1|2;
				break;
			case SC_WIND_STEP_OPTION:
				val2 = 50;	// % Increase speed and flee.
				break;
			case SC_BLAST_OPTION:
				val2 = 20;
				val3 = ELE_WIND;
				val_flag |= 1|2|4;
				break;
			case SC_WILD_STORM_OPTION:
				val2 = MG_LIGHTNINGBOLT;
				val_flag |= 1|2;
				break;
			case SC_PETROLOGY_OPTION:
				val2 = 5;
				val3 = 50;
				val_flag |= 1|2|4;
				break;
			case SC_CURSED_SOIL_OPTION:
				val2 = 10;
				val3 = 33;
				val4 = 2;
				val_flag |= 1|2|4;
				break;
			case SC_UPHEAVAL_OPTION:
				val2 = WZ_EARTHSPIKE;
				val_flag |= 1|2;
				break;
			case SC_CIRCLE_OF_FIRE_OPTION:
				val2 = 300;
				val_flag |= 1|2;
				break;
			case SC_FIRE_CLOAK_OPTION:
			case SC_WATER_DROP_OPTION:
			case SC_WIND_CURTAIN_OPTION:
			case SC_STONE_SHIELD_OPTION:
				val2 = 20;	// Elemental modifier. Not confirmed.
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
				val3 = 20;	// Reductions. Atk2, Flee1, Matk1 ????
				val_flag |= 1|2|4;
				break;
			case SC_ZEPHYR:
				val2 = 22;	// Flee.
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
				val2 = 2*val1 + rand()%val1;
				clif->status_change(bl,SI_ACTIVE_MONSTER_TRANSFORM,1,0,1002,0,0);
				break;
			case SC_KAGEMUSYA:
				val3 = val1 * 2;
			case SC_IZAYOI:
				val2 = tick/1000;
				tick_time = 1000;
				break;
			case SC_ZANGETSU:
				if( (status_get_hp(bl)+status_get_sp(bl)) % 2 == 0)
					val2 = status_get_lv(bl) / 2 + 50;
				else
					val2 -= 50;
				break;
			case SC_GENSOU:
				{
					int hp = status_get_hp(bl), lv = 5;
					short per = 100 / (status_get_max_hp(bl) / hp);

					if( per <= 15 )
						lv = 1;
					else if( per <= 30 )
						lv = 2;
					else if( per <= 50 )
						lv = 3;
					else if( per <= 75 )
						lv = 4;
					if( hp % 2 == 0)
						status_heal(bl, hp * (6-lv) * 4 / 100, status_get_sp(bl) * (6-lv) * 3 / 100, 1);
					else
						status_zap(bl, hp * (lv*4) / 100, status_get_sp(bl) * (lv*3) / 100);
				}
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
				case SC_PARALYSIS: //[Lighta] need real info
					val2 = 2*val1; //def reduction
					val3 = 500*val1; //varcast augmentation
					break;
				case SC_PAIN_KILLER: //[Lighta] need real info
					val2 = 2*val1; //aspd reduction %
					val3 = 2*val1; //dmg reduction %
					if(sc->data[SC_PARALYSIS])
					sc_start(bl, SC_ENDURE, 100, val1, tick); //start endure for same duration
					break;
							case SC_STYLE_CHANGE: //[Lighta] need real info
								tick = -1;
								if(val2 == MH_MD_FIGHTING) val2 = MH_MD_GRAPPLING;
								else val2 = MH_MD_FIGHTING;
								break;
			default:
				if( calc_flag == SCB_NONE && StatusSkillChangeTable[type] == 0 && StatusIconChangeTable[type] == 0 )
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
				if( !vd ) break;
				clif->changelook(bl,LOOK_BASE,vd->class_);
				clif->changelook(bl,LOOK_WEAPON,0);
				clif->changelook(bl,LOOK_SHIELD,0);
				clif->changelook(bl,LOOK_CLOTHES_COLOR,vd->cloth_color);
				break;
			case SC_KAAHI:
				val4 = INVALID_TIMER;
				break;
		}
	}
	
	/* [Ind/Hercules] */
	if( sd && StatusDisplayType[type] ) {
		int dval1 = 0, dval2 = 0, dval3 = 0;
		switch( type ) {
			case SC_ALL_RIDING:
				dval1 = 1;
				break;
			default: /* all others: just copy val1 */
				dval1 = val1;
				break;
		}
		status_display_add(sd,type,dval1,dval2,dval3);
	}

	//Those that make you stop attacking/walking....
	switch (type) {
		case SC_FREEZE:
		case SC_STUN:
		case SC_SLEEP:
		case SC_STONE:
		case SC_DEEPSLEEP:
			if (sd && pc_issit(sd)) //Avoid sprite sync problems.
				iPc->setstand(sd);
		case SC_TRICKDEAD:
			status_change_end(bl, SC_DANCING, INVALID_TIMER);
			// Cancel cast when get status [LuzZza]
			if (battle_config.sc_castcancel&bl->type)
				unit_skillcastcancel(bl, 0);
		case SC_WHITEIMPRISON:
			unit_stop_attack(bl);
		case SC_STOP:
		case SC_CONFUSION:
		case SC_CLOSECONFINE:
		case SC_CLOSECONFINE2:
		case SC_SPIDERWEB:
		case SC_ELECTRICSHOCKER:
		case SC_BITE:
		case SC_THORNSTRAP:
		case SC__MANHOLE:
		case SC_CRYSTALIZE:
		case SC_CURSEDCIRCLE_ATKER:
		case SC_CURSEDCIRCLE_TARGET:
		case SC_FEAR:
		case SC_NETHERWORLD:
		case SC_MEIKYOUSISUI:
		case SC_KYOUGAKU:
		case SC_PARALYSIS:
			unit_stop_walking(bl,1);
		break;
		case SC_ANKLE:
			if( battle_config.skill_trap_type || !map_flag_gvg(bl->m) )
				unit_stop_walking(bl,1);
			break;
		case SC_HIDING:
		case SC_CLOAKING:
		case SC_CLOAKINGEXCEED:
		case SC_CHASEWALK:
		case SC_WEIGHT90:
		case SC_CAMOUFLAGE:
		case SC_VOICEOFSIREN:
			unit_stop_attack(bl);
		break;
		case SC_SILENCE:
			if (battle_config.sc_castcancel&bl->type)
				unit_skillcastcancel(bl, 0);
		break;
	}

	// Set option as needed.
	opt_flag = 1;
	switch(type) {
		//OPT1
		case SC_STONE:  sc->opt1 = OPT1_STONEWAIT; break;
		case SC_FREEZE: sc->opt1 = OPT1_FREEZE;    break;
		case SC_STUN:   sc->opt1 = OPT1_STUN;      break;
		case SC_DEEPSLEEP:    opt_flag = 0;
		case SC_SLEEP:   sc->opt1 = OPT1_SLEEP;		break;
		case SC_BURNING:		sc->opt1 = OPT1_BURNING;	break; // Burning need this to be showed correctly. [pakpil]
		case SC_WHITEIMPRISON:  sc->opt1 = OPT1_IMPRISON;	break;
		case SC_CRYSTALIZE:		sc->opt1 = OPT1_CRYSTALIZE;	break;
		//OPT2
		case SC_POISON:       sc->opt2 |= OPT2_POISON;       break;
		case SC_CURSE:        sc->opt2 |= OPT2_CURSE;        break;
		case SC_SILENCE:      sc->opt2 |= OPT2_SILENCE;      break;

		case SC_SIGNUMCRUCIS:
			sc->opt2 |= OPT2_SIGNUMCRUCIS;
			break;

		case SC_BLIND:        sc->opt2 |= OPT2_BLIND;        break;
		case SC_ANGELUS:      sc->opt2 |= OPT2_ANGELUS;      break;
		case SC_BLEEDING:     sc->opt2 |= OPT2_BLEEDING;     break;
		case SC_DPOISON:      sc->opt2 |= OPT2_DPOISON;      break;
		//OPT3
		case SC_TWOHANDQUICKEN:
		case SC_ONEHAND:
		case SC_SPEARQUICKEN:
		case SC_CONCENTRATION:
		case SC_MERC_QUICKEN:
			sc->opt3 |= OPT3_QUICKEN;
			opt_flag = 0;
			break;
		case SC_MAXOVERTHRUST:
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
//		case SC__BLOODYLUST:
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
		case SC_MARIONETTE:
		case SC_MARIONETTE2:
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
		case SC_BUNSINJYUTSU:
			sc->opt3 |= OPT3_BUNSIN;
			opt_flag = 0;
			break;
		case SC_SPIRIT:
			sc->opt3 |= OPT3_SOULLINK;
			opt_flag = 0;
			break;
		case SC_CHANGEUNDEAD:
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
		case SC__INVISIBILITY:
			sc->option |= OPTION_CLOAK;
			opt_flag = 2;
			break;
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

	if( !(flag&4 && StatusDisplayType[type]) )
		clif->status_change(bl,StatusIconChangeTable[type],1,tick,(val_flag&1)?val1:1,(val_flag&2)?val2:0,(val_flag&4)?val3:0);

	/**
	 * used as temporary storage for scs with interval ticks, so that the actual duration is sent to the client first.
	 **/
	if( tick_time )
		tick = tick_time;

	//Don't trust the previous sce assignment, in case the SC ended somewhere between there and here.
	if((sce=sc->data[type])) {// reuse old sc
		if( sce->timer != INVALID_TIMER )
			iTimer->delete_timer(sce->timer, status_change_timer);
	} else {// new sc
		++(sc->count);
		sce = sc->data[type] = ers_alloc(sc_data_ers, struct status_change_entry);
	}
	sce->val1 = val1;
	sce->val2 = val2;
	sce->val3 = val3;
	sce->val4 = val4;
	if (tick >= 0)
		sce->timer = iTimer->add_timer(iTimer->gettick() + tick, status_change_timer, bl->id, type);
	else
		sce->timer = INVALID_TIMER; //Infinite duration

	if (calc_flag)
		status_calc_bl(bl,calc_flag);

	if(sd && sd->pd)
		pet_sc_check(sd, type); //Skotlex: Pet Status Effect Healing

    switch (type) {
		case SC__BLOODYLUST:
		case SC_BERSERK:
			if (!(sce->val2)) { //don't heal if already set
				status_heal(bl, status->max_hp, 0, 1); //Do not use percent_heal as this healing must override BERSERK's block.
				status_set_sp(bl, 0, 0); //Damage all SP
			}
			sce->val2 = 5 * status->max_hp / 100;
			break;
		case SC_CHANGE:
			status_percent_heal(bl, 100, 100);
			break;
		case SC_RUN:
			{
				struct unit_data *ud = unit_bl2ud(bl);
				if( ud )
					ud->state.running = unit_run(bl);
			}
			break;
		case SC_BOSSMAPINFO:
			clif->bossmapinfo(sd->fd, iMap->id2boss(sce->val1), 0); // First Message
			break;
		case SC_MERC_HPUP:
			status_percent_heal(bl, 100, 0); // Recover Full HP
			break;
		case SC_MERC_SPUP:
			status_percent_heal(bl, 0, 100); // Recover Full SP
			break;
		/**
		 * Ranger
		 **/
		case SC_WUGDASH:
			{
				struct unit_data *ud = unit_bl2ud(bl);
				if( ud )
					ud->state.running = unit_wugdash(bl, sd);
			}
			break;
		case SC_COMBO:
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
					if (sd && iPc->checkskill(sd, SR_DRAGONCOMBO) > 0)
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
			sce->val2 = status->max_hp / 100;// Officially tested its 1%hp drain. [Jobbie]
			break;
	}

	if( opt_flag&2 && sd && sd->touching_id )
		npc_touchnext_areanpc(sd,false); // run OnTouch_ on next char in range

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

	sc = status_get_sc(bl);

	if (!sc || !sc->count)
		return 0;

	for(i = 0; i < SC_MAX; i++) {
		if(!sc->data[i])
		  continue;

		if(type == 0) {
			switch (i) {	//Type 0: PC killed -> Place here statuses that do not dispel on death.
				case SC_ELEMENTALCHANGE://Only when its Holy or Dark that it doesn't dispell on death
					if( sc->data[i]->val2 != ELE_HOLY && sc->data[i]->val2 != ELE_DARK )
						break;
				case SC_WEIGHT50:
				case SC_WEIGHT90:
				case SC_EDP:
				case SC_MELTDOWN:
				case SC_XMAS:
				case SC_SUMMER:
				case SC_HANBOK:
				case SC_NOCHAT:
				case SC_FUSION:
				case SC_EARTHSCROLL:
				case SC_READYSTORM:
				case SC_READYDOWN:
				case SC_READYCOUNTER:
				case SC_READYTURN:
				case SC_DODGE:
				case SC_JAILED:
				case SC_EXPBOOST:
				case SC_ITEMBOOST:
				case SC_HELLPOWER:
				case SC_JEXPBOOST:
				case SC_AUTOTRADE:
				case SC_WHISTLE:
				case SC_ASSNCROS:
				case SC_POEMBRAGI:
				case SC_APPLEIDUN:
				case SC_HUMMING:
				case SC_DONTFORGETME:
				case SC_FORTUNE:
				case SC_SERVICE4U:
				case SC_FOOD_STR_CASH:
				case SC_FOOD_AGI_CASH:
				case SC_FOOD_VIT_CASH:
				case SC_FOOD_DEX_CASH:
				case SC_FOOD_INT_CASH:
				case SC_FOOD_LUK_CASH:
				case SC_DEF_RATE:
				case SC_MDEF_RATE:
				case SC_INCHEALRATE:
				case SC_INCFLEE2:
				case SC_INCHIT:
				case SC_ATKPOTION:
				case SC_MATKPOTION:
				case SC_S_LIFEPOTION:
				case SC_L_LIFEPOTION:
				case SC_PUSH_CART:
				case SC_ALL_RIDING:
					continue;
			}
		}

		if( type == 3 ) {
			switch (i) {// TODO: This list may be incomplete
				case SC_WEIGHT50:
				case SC_WEIGHT90:
				case SC_NOCHAT:
				case SC_PUSH_CART:
				case SC_ALL_RIDING:
					continue;
			}
		}

		status_change_end(bl, (sc_type)i, INVALID_TIMER);

		if( type == 1 && sc->data[i] ) {
			//If for some reason status_change_end decides to still keep the status when quitting. [Skotlex]
			(sc->count)--;
			if (sc->data[i]->timer != INVALID_TIMER)
				iTimer->delete_timer(sc->data[i]->timer, status_change_timer);
			ers_free(sc_data_ers, sc->data[i]);
			sc->data[i] = NULL;
		}
	}

	sc->opt1 = 0;
	sc->opt2 = 0;
	sc->opt3 = 0;
	sc->bs_counter = 0;
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
	struct status_data *status;
	struct view_data *vd;
	int opt_flag=0, calc_flag;

	nullpo_ret(bl);

	sc = status_get_sc(bl);
	status = status_get_status_data(bl);

	if(type < 0 || type >= SC_MAX || !sc || !(sce = sc->data[type]))
		return 0;

	sd = BL_CAST(BL_PC,bl);

	if (sce->timer != tid && tid != INVALID_TIMER)
		return 0;

	if (tid == INVALID_TIMER) {
		if (type == SC_ENDURE && sce->val4)
			//Do not end infinite endure.
			return 0;
		if (sce->timer != INVALID_TIMER) //Could be a SC with infinite duration
			iTimer->delete_timer(sce->timer,status_change_timer);
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
				sce->timer = iTimer->add_timer(iTimer->gettick()+10, status_change_timer, bl->id, type);
				return 1;
			}
		}
	}

	(sc->count)--;

	sc->data[type] = NULL;

	if( sd && StatusDisplayType[type] ) {
		status_display_remove(sd,type);
	}
	
	vd = status_get_viewdata(bl);
	calc_flag = StatusChangeFlagTable[type];
	switch(type){
        case SC_GRANITIC_ARMOR:{
            int dammage = status->max_hp*sce->val3/100;
            if(status->hp < dammage) //to not kill him
                dammage = status->hp-1;
            status_damage(NULL, bl, dammage,0,0,1);
            break;
        }
        case SC_PYROCLASTIC:
            if(bl->type == BL_PC)
                skill->break_equip(bl,EQP_WEAPON,10000,BCT_SELF);
            break;
		case SC_RUN:
		{
			struct unit_data *ud = unit_bl2ud(bl);
			bool begin_spurt = true;
			if (ud) {
				if(!ud->state.running)
					begin_spurt = false;
				ud->state.running = 0;
				if (ud->walktimer != INVALID_TIMER)
					unit_stop_walking(bl,1);
			}
			if (begin_spurt && sce->val1 >= 7 &&
				DIFF_TICK(iTimer->gettick(), sce->val4) <= 1000 &&
				(!sd || (sd->weapontype1 == 0 && sd->weapontype2 == 0))
			)
				sc_start(bl,SC_SPURT,100,sce->val1,skill->get_time2(status_sc2skill(type), sce->val1));
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
				if( bl->type == BL_PC )
				{ // Clear Status from others
					int i;
					for( i = 0; i < 5; i++ )
					{
						if( sd->devotion[i] && (tsd = iMap->id2sd(sd->devotion[i])) && tsd->sc.data[type] )
							status_change_end(&tsd->bl, type, INVALID_TIMER);
					}
				}
				else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag )
				{ // Clear Status from Master
					tsd = ((TBL_MER*)bl)->master;
					if( tsd && tsd->sc.data[type] )
						status_change_end(&tsd->bl, type, INVALID_TIMER);
				}
			}
			break;
		case SC_DEVOTION:
			{
				struct block_list *d_bl = iMap->id2bl(sce->val1);
				if( d_bl )
				{
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
			if(sce->val4)
			{
				int tid = sce->val4;
				struct block_list *tbl = iMap->id2bl(tid);
				struct status_change *tsc = status_get_sc(tbl);
				sce->val4 = 0;
				if(tbl && tsc && tsc->data[SC_BLADESTOP])
				{
					tsc->data[SC_BLADESTOP]->val4 = 0;
					status_change_end(tbl, SC_BLADESTOP, INVALID_TIMER);
				}
				clif->bladestop(bl, tid, 0);
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

				if(sce->val4 && sce->val4 != BCT_SELF && (dsd=iMap->id2sd(sce->val4)))
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
					clif->sc_end(bl,bl->id,AREA,SI_MOONLIT);

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
				struct block_list *src=iMap->id2bl(sce->val3);
				if(src && tid != INVALID_TIMER)
					skill->castend_damage_id(src, bl, sce->val2, sce->val1, iTimer->gettick(), SD_LEVEL );
			}
			break;
		case SC_CLOSECONFINE2:
			{
				struct block_list *src = sce->val2?iMap->id2bl(sce->val2):NULL;
				struct status_change *sc2 = src?status_get_sc(src):NULL;
				if (src && sc2 && sc2->data[SC_CLOSECONFINE]) {
					//If status was already ended, do nothing.
					//Decrease count
					if (--(sc2->data[SC_CLOSECONFINE]->val1) <= 0) //No more holds, free him up.
						status_change_end(src, SC_CLOSECONFINE, INVALID_TIMER);
				}
			}
		case SC_CLOSECONFINE:
			if (sce->val2 > 0) {
				//Caster has been unlocked... nearby chars need to be unlocked.
				int range = 1
					+skill->get_range2(bl, status_sc2skill(type), sce->val1)
					+skill->get_range2(bl, TF_BACKSLIDING, 1); //Since most people use this to escape the hold....
				iMap->foreachinarea(status_change_timer_sub,
					bl->m, bl->x-range, bl->y-range, bl->x+range,bl->y+range,BL_CHAR,bl,sce,type,iTimer->gettick());
			}
			break;
		case SC_COMBO:
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
					if (iPc->checkskill(sd, SR_DRAGONCOMBO) > 0)
						clif->skillinfo(sd, SR_DRAGONCOMBO, 0);
					break;
				case SR_FALLENEMPIRE:
					clif->skillinfo(sd, SR_GATEOFHELL, 0);
					clif->skillinfo(sd, SR_TIGERCANNON, 0);
					break;
			}
			break;

		case SC_MARIONETTE:
		case SC_MARIONETTE2:	/// Marionette target
			if (sce->val1)
			{	// check for partner and end their marionette status as well
				enum sc_type type2 = (type == SC_MARIONETTE) ? SC_MARIONETTE2 : SC_MARIONETTE;
				struct block_list *pbl = iMap->id2bl(sce->val1);
				struct status_change* sc2 = pbl?status_get_sc(pbl):NULL;

				if (sc2 && sc2->data[type2])
				{
					sc2->data[type2]->val1 = 0;
					status_change_end(pbl, type2, INVALID_TIMER);
				}
			}
			break;

		case SC_BERSERK:
		case SC_SATURDAYNIGHTFEVER:
			//If val2 is removed, no HP penalty (dispelled?) [Skotlex]
			if (status->hp > 100 && sce->val2)
				status_set_hp(bl, 100, 0);
			if(sc->data[SC_ENDURE] && sc->data[SC_ENDURE]->val4 == 2)
			{
				sc->data[SC_ENDURE]->val4 = 0;
				status_change_end(bl, SC_ENDURE, INVALID_TIMER);
			}
		case SC__BLOODYLUST:
			sc_start4(bl, SC_REGENERATION, 100, 10,0,0,(RGN_HP|RGN_SP), skill->get_time(LK_BERSERK, sce->val1));
			if( type == SC_SATURDAYNIGHTFEVER ) //Sit down force of Saturday Night Fever has the duration of only 3 seconds.
				sc_start(bl,SC_SITDOWN_FORCE,100,sce->val1,skill->get_time2(WM_SATURDAY_NIGHT_FEVER,sce->val1));
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
				iTimer->delete_timer(sce->val4,kaahi_heal_timer);
			break;
		case SC_JAILED:
			if(tid == INVALID_TIMER)
				break;
		  	//natural expiration.
			if(sd && sd->mapindex == sce->val2)
				iPc->setpos(sd,(unsigned short)sce->val3,sce->val4&0xFFFF, sce->val4>>16, CLR_TELEPORT);
			break; //guess hes not in jail :P
		case SC_CHANGE:
			if (tid == INVALID_TIMER)
		 		break;
			// "lose almost all their HP and SP" on natural expiration.
			status_set_hp(bl, 10, 0);
			status_set_sp(bl, 10, 0);
			break;
		case SC_AUTOTRADE:
			if (tid == INVALID_TIMER)
				break;
			// Note: vending/buying is closed by unit_remove_map, no
			// need to do it here.
			iMap->quit(sd);
			// Because iMap->quit calls status_change_end with tid -1
			// from here it's not neccesary to continue
			return 1;
			break;
		case SC_STOP:
			if( sce->val2 )
			{
				struct block_list* tbl = iMap->id2bl(sce->val2);
				sce->val2 = 0;
				if( tbl && (sc = status_get_sc(tbl)) && sc->data[SC_STOP] && sc->data[SC_STOP]->val2 == bl->id )
					status_change_end(tbl, SC_STOP, INVALID_TIMER);
			}
			break;
		/**
		 * 3rd Stuff
		 **/
		case SC_MILLENNIUMSHIELD:
			clif->millenniumshield(sd,0);
			break;
		case SC_HALLUCINATIONWALK:
			sc_start(bl,SC_HALLUCINATIONWALK_POSTDELAY,100,sce->val1,skill->get_time2(GC_HALLUCINATIONWALK,sce->val1));
			break;
		case SC_WHITEIMPRISON:
			{
				struct block_list* src = iMap->id2bl(sce->val2);
				if( tid == -1 || !src)
					break; // Terminated by Damage
				status_fix_damage(src,bl,400*sce->val1,clif->damage(bl,bl,iTimer->gettick(),0,0,400*sce->val1,0,0,0));
			}
			break;
		case SC_WUGDASH:
			{
				struct unit_data *ud = unit_bl2ud(bl);
				if (ud) {
					ud->state.running = 0;
					if (ud->walktimer != -1)
						unit_stop_walking(bl,1);
				}
			}
			break;
		case SC_ADORAMUS:
			status_change_end(bl, SC_BLIND, INVALID_TIMER);
			break;
		case SC__SHADOWFORM: {
				struct map_session_data *s_sd = iMap->id2sd(sce->val2);
				if( !s_sd )
					break;
				s_sd->shadowform_id = 0;
			}
			break;
		case SC_SITDOWN_FORCE:
			if( sd && pc_issit(sd) ) {
				iPc->setstand(sd);
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
				iMap->foreachinrange(status_change_timer_sub, bl, battle_config.area_size,BL_CHAR, bl, sce, SC_CURSEDCIRCLE_TARGET, iTimer->gettick());
			break;
		case SC_RAISINGDRAGON:
			if( sd && sce->val2 && !pc_isdead(sd) ) {
				int i;
				i = min(sd->spiritball,5);
				iPc->delspiritball(sd, sd->spiritball, 0);
				status_change_end(bl, SC_EXPLOSIONSPIRITS, INVALID_TIMER);
				while( i > 0 ) {
					iPc->addspiritball(sd, skill->get_time(MO_CALLSPIRITS, iPc->checkskill(sd,MO_CALLSPIRITS)), 5);
					--i;
				}
			}
			break;
		case SC_CURSEDCIRCLE_TARGET:
			{
				struct block_list *src = iMap->id2bl(sce->val2);
				struct status_change *sc = status_get_sc(src);
				if( sc && sc->data[SC_CURSEDCIRCLE_ATKER] && --(sc->data[SC_CURSEDCIRCLE_ATKER]->val2) == 0 ){
					status_change_end(src, SC_CURSEDCIRCLE_ATKER, INVALID_TIMER);
					clif->bladestop(bl, sce->val2, 0);
				}
			}
			break;
		case SC_BLOODSUCKER:
			if( sce->val2 ){
				struct block_list *src = iMap->id2bl(sce->val2);
				if(src){
					struct status_change *sc = status_get_sc(src);
					sc->bs_counter--;
				}
			}
			break;
		case SC_KYOUGAKU:
			clif->sc_end(&sd->bl,sd->bl.id,AREA,SI_KYOUGAKU);
			clif->sc_end(&sd->bl,sd->bl.id,AREA,SI_ACTIVE_MONSTER_TRANSFORM);
			break;
		case SC_INTRAVISION:
			calc_flag = SCB_ALL;/* required for overlapping */
			break;
	}

	opt_flag = 1;
	switch(type){
		case SC_STONE:
		case SC_FREEZE:
		case SC_STUN:
		case SC_SLEEP:
		case SC_DEEPSLEEP:
		case SC_BURNING:
		case SC_WHITEIMPRISON:
		case SC_CRYSTALIZE:
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
		case SC_SIGNUMCRUCIS:
			sc->opt2 &= ~OPT2_SIGNUMCRUCIS;
			break;

		case SC_HIDING:
			sc->option &= ~OPTION_HIDE;
			opt_flag|= 2|4; //Check for warp trigger + AoE trigger
			break;
		case SC_CLOAKING:
		case SC_CLOAKINGEXCEED:
		case SC__INVISIBILITY:
			sc->option &= ~OPTION_CLOAK;
		case SC_CAMOUFLAGE:
			opt_flag|= 2;
			break;
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
		case SC_ONEHAND:
		case SC_SPEARQUICKEN:
		case SC_CONCENTRATION:
		case SC_MERC_QUICKEN:
			sc->opt3 &= ~OPT3_QUICKEN;
			opt_flag = 0;
			break;
		case SC_OVERTHRUST:
		case SC_MAXOVERTHRUST:
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
	//	case SC__BLOODYLUST:
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
		case SC_MARIONETTE2:
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
		case SC_BUNSINJYUTSU:
			sc->opt3 &= ~OPT3_BUNSIN;
			opt_flag = 0;
			break;
		case SC_SPIRIT:
			sc->opt3 &= ~OPT3_SOULLINK;
			opt_flag = 0;
			break;
		case SC_CHANGEUNDEAD:
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

	if (calc_flag&SCB_DYE) { //Restore DYE color
		if (vd && !vd->cloth_color && sce->val4)
			clif->changelook(bl,LOOK_CLOTHES_COLOR,sce->val4);
		calc_flag&=~SCB_DYE;
	}

	//On Aegis, when turning off a status change, first goes the sc packet, then the option packet.
	clif->sc_end(bl,bl->id,AREA,StatusIconChangeTable[type]);

	if( opt_flag&8 ) //bugreport:681
		clif->changeoption2(bl);
	else if(opt_flag) {
		clif->changeoption(bl);
		if( sd && opt_flag&0x4 ) {
			clif->changelook(bl,LOOK_BASE,vd->class_);
			clif->get_weapon_view(sd, &sd->vd.weapon, &sd->vd.shield);
			clif->changelook(bl,LOOK_WEAPON,sd->vd.weapon);
			clif->changelook(bl,LOOK_SHIELD,sd->vd.shield);
			clif->changelook(bl,LOOK_CLOTHES_COLOR,cap_value(sd->status.clothes_color,0,battle_config.max_cloth_color));
		}
	}

	if (calc_flag)
		status_calc_bl(bl,calc_flag);

	if(opt_flag&4) //Out of hiding, invoke on place.
		skill->unit_move(bl,iTimer->gettick(),1);

	if(opt_flag&2 && sd && iMap->getcell(bl->m,bl->x,bl->y,CELL_CHKNPC))
		npc_touch_areanpc(sd,bl->m,bl->x,bl->y); //Trigger on-touch event.

	ers_free(sc_data_ers, sce);
	return 1;
}

int kaahi_heal_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct block_list *bl;
	struct status_change *sc;
	struct status_change_entry *sce;
	struct status_data *status;
	int hp;

	if(!((bl=iMap->id2bl(id))&&
		(sc=status_get_sc(bl)) &&
		(sce = sc->data[SC_KAAHI])))
		return 0;

	if(sce->val4 != tid) {
		ShowError("kaahi_heal_timer: Timer mismatch: %d != %d\n", tid, sce->val4);
		sce->val4 = INVALID_TIMER;
		return 0;
	}

	status=status_get_status_data(bl);
	if(!status_charge(bl, 0, sce->val3)) {
		sce->val4 = INVALID_TIMER;
		return 0;
	}

	hp = status->max_hp - status->hp;
	if (hp > sce->val2)
		hp = sce->val2;
	if (hp)
		status_heal(bl, hp, 0, 2);
	sce->val4 = INVALID_TIMER;
	return 1;
}

/*==========================================
 * For recusive status, like for each 5s we drop sp etc.
 * Reseting the end timer.
 *------------------------------------------*/
int status_change_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	enum sc_type type = (sc_type)data;
	struct block_list *bl;
	struct map_session_data *sd;
	struct status_data *status;
	struct status_change *sc;
	struct status_change_entry *sce;

	bl = iMap->id2bl(id);
	if(!bl)
	{
		ShowDebug("status_change_timer: Null pointer id: %d data: %d\n", id, data);
		return 0;
	}
	sc = status_get_sc(bl);
	status = status_get_status_data(bl);

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
#define sc_timer_next(t,f,i,d) \
	if( (sce=sc->data[type]) ) \
		sce->timer = iTimer->add_timer(t,f,i,d); \
	else \
		ShowError("status_change_timer: Unexpected NULL status change id: %d data: %d\n", id, data)

	switch(type)
	{
	case SC_MAXIMIZEPOWER:
	case SC_CLOAKING:
		if(!status_charge(bl, 0, 1))
			break; //Not enough SP to continue.
		sc_timer_next(sce->val2+tick, status_change_timer, bl->id, data);
		return 0;

	case SC_CHASEWALK:
		if(!status_charge(bl, 0, sce->val4))
			break; //Not enough SP to continue.

		if (!sc->data[SC_INCSTR]) {
			sc_start(bl, SC_INCSTR,100,1<<(sce->val1-1),
				(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_ROGUE?10:1) //SL bonus -> x10 duration
				*skill->get_time2(status_sc2skill(type),sce->val1));
		}
		sc_timer_next(sce->val2+tick, status_change_timer, bl->id, data);
		return 0;
	break;

	case SC_SKA:
		if(--(sce->val2)>0){
			sce->val3 = rnd()%100; //Random defense.
			sc_timer_next(1000+tick, status_change_timer,bl->id, data);
			return 0;
		}
		break;

	case SC_HIDING:
		if(--(sce->val2)>0){

			if(sce->val2 % sce->val4 == 0 && !status_charge(bl, 0, 1))
				break; //Fail if it's time to substract SP and there isn't.

			sc_timer_next(1000+tick, status_change_timer,bl->id, data);
			return 0;
		}
	break;

	case SC_SIGHT:
	case SC_RUWACH:
	case SC_SIGHTBLASTER:
		if(type == SC_SIGHTBLASTER)
			iMap->foreachinrange( status_change_timer_sub, bl, sce->val3, BL_CHAR|BL_SKILL, bl, sce, type, tick);
		else
			iMap->foreachinrange( status_change_timer_sub, bl, sce->val3, BL_CHAR, bl, sce, type, tick);

		if( --(sce->val2)>0 ){
			sce->val4 += 250; // use for Shadow Form 2 seconds checking.
			sc_timer_next(250+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_PROVOKE:
		if(sce->val2) { //Auto-provoke (it is ended in status_heal)
			sc_timer_next(1000*60+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_STONE:
		if(sc->opt1 == OPT1_STONEWAIT && sce->val3) {
			sce->val4 = 0;
			unit_stop_walking(bl,1);
			unit_stop_attack(bl);
			sc->opt1 = OPT1_STONE;
			clif->changeoption(bl);
			sc_timer_next(1000+tick,status_change_timer, bl->id, data );
			status_calc_bl(bl, StatusChangeFlagTable[type]);
			return 0;
		}
		if(--(sce->val3) > 0) {
			if(++(sce->val4)%5 == 0 && status->hp > status->max_hp/4)
				status_percent_damage(NULL, bl, 1, 0, false);
			sc_timer_next(1000+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_POISON:
		if(status->hp <= max(status->max_hp>>2, sce->val4)) //Stop damaging after 25% HP left.
			break;
	case SC_DPOISON:
		if (--(sce->val3) > 0) {
			if (!sc->data[SC_SLOWPOISON]) {
				if( sce->val2 && bl->type == BL_MOB ) {
					struct block_list* src = iMap->id2bl(sce->val2);
					if( src )
						mob_log_damage((TBL_MOB*)bl,src,sce->val4);
				}
				iMap->freeblock_lock();
				status_zap(bl, sce->val4, 0);
				if (sc->data[type]) { // Check if the status still last ( can be dead since then ).
					sc_timer_next(1000 + tick, status_change_timer, bl->id, data );
				}
				iMap->freeblock_unlock();
			}
			return 0;
		}
		break;

	case SC_TENSIONRELAX:
		if(status->max_hp > status->hp && --(sce->val3) > 0){
			sc_timer_next(sce->val4+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_KNOWLEDGE:
		if (!sd) break;
		if(bl->m == sd->feel_map[0].m ||
			bl->m == sd->feel_map[1].m ||
			bl->m == sd->feel_map[2].m)
		{	//Timeout will be handled by iPc->setpos
			sce->timer = INVALID_TIMER;
			return 0;
		}
		break;

	case SC_BLEEDING:
		if (--(sce->val4) >= 0) {
			int hp =  rnd()%600 + 200;
			struct block_list* src = iMap->id2bl(sce->val2);
			if( src && bl && bl->type == BL_MOB ) {
				mob_log_damage((TBL_MOB*)bl,src,sd||hp<status->hp?hp:status->hp-1);
			}
			iMap->freeblock_lock();
			status_fix_damage(src, bl, sd||hp<status->hp?hp:status->hp-1, 1);
			if( sc->data[type] ) {
				if( status->hp == 1 ) {
					iMap->freeblock_unlock();
					break;
				}
				sc_timer_next(10000 + tick, status_change_timer, bl->id, data);
			}
			iMap->freeblock_unlock();
			return 0;
		}
		break;

	case SC_S_LIFEPOTION:
	case SC_L_LIFEPOTION:
		if( sd && --(sce->val4) >= 0 )
		{
			// val1 < 0 = per max% | val1 > 0 = exact amount
			int hp = 0;
			if( status->hp < status->max_hp )
				hp = (sce->val1 < 0) ? (int)(sd->status.max_hp * -1 * sce->val1 / 100.) : sce->val1 ;
			status_heal(bl, hp, 0, 2);
			sc_timer_next((sce->val2 * 1000) + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_BOSSMAPINFO:
		if( sd && --(sce->val4) >= 0 )
		{
			struct mob_data *boss_md = iMap->id2boss(sce->val1);
			if( boss_md && sd->bl.m == boss_md->bl.m )
			{
				clif->bossmapinfo(sd->fd, boss_md, 1); // Update X - Y on minimap
				if (boss_md->bl.prev != NULL) {
					sc_timer_next(5000 + tick, status_change_timer, bl->id, data);
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
			if( s != 0 && sce->val3 % s == 0 )
			{
				if (sc->data[SC_LONGING])
					sp*= 3;
				if (!status_charge(bl, 0, sp))
					break;
			}
			sc_timer_next(1000+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;
	case SC__BLOODYLUST:
	case SC_BERSERK:
		// 5% every 10 seconds [DracoRPG]
		if( --( sce->val3 ) > 0 && status_charge(bl, sce->val2, 0) && status->hp > 100 )
		{
			sc_timer_next(sce->val4+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_NOCHAT:
		if(sd){
			sd->status.manner++;
			clif->changestatus(sd,SP_MANNER,sd->status.manner);
			clif->updatestatus(sd,SP_MANNER);
			if (sd->status.manner < 0)
			{	//Every 60 seconds your manner goes up by 1 until it gets back to 0.
				sc_timer_next(60000+tick, status_change_timer, bl->id, data);
				return 0;
			}
		}
		break;

	case SC_SPLASHER:
		// custom Venom Splasher countdown timer
		//if (sce->val4 % 1000 == 0) {
		//	char timer[10];
		//	snprintf (timer, 10, "%d", sce->val4/1000);
		//	clif->message(bl, timer);
		//}
		if((sce->val4 -= 500) > 0) {
			sc_timer_next(500 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_MARIONETTE:
	case SC_MARIONETTE2:
		{
			struct block_list *pbl = iMap->id2bl(sce->val1);
			if( pbl && check_distance_bl(bl, pbl, 7) )
			{
				sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
				return 0;
			}
		}
		break;

	case SC_GOSPEL:
		if(sce->val4 == BCT_SELF && --(sce->val2) > 0)
		{
			int hp, sp;
			hp = (sce->val1 > 5) ? 45 : 30;
			sp = (sce->val1 > 5) ? 35 : 20;
			if(!status_charge(bl, hp, sp))
				break;
			sc_timer_next(10000+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_JAILED:
		if(sce->val1 == INT_MAX || --(sce->val1) > 0)
		{
			sc_timer_next(60000+tick, status_change_timer, bl->id,data);
			return 0;
		}
		break;

	case SC_BLIND:
		if(sc->data[SC_FOGWALL])
		{	//Blind lasts forever while you are standing on the fog.
			sc_timer_next(5000+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;
	case SC_ABUNDANCE:
		if(--(sce->val4) > 0) {
			status_heal(bl,0,60,0);
			sc_timer_next(10000+tick, status_change_timer, bl->id, data);
		}
		break;

	case SC_PYREXIA:
		if( --(sce->val4) >= 0 ) {
			iMap->freeblock_lock();
			clif->damage(bl,bl,tick,status_get_amotion(bl),status_get_dmotion(bl)+500,100,0,0,0);
			status_fix_damage(NULL,bl,100,0);
			if( sc->data[type] ) {
				sc_timer_next(3000+tick,status_change_timer,bl->id,data);
			}
			iMap->freeblock_unlock();
			return 0;
		}
		break;

	case SC_LEECHESEND:
		if( --(sce->val4) >= 0 ) {
			int damage = status->max_hp/100; // {Target VIT x (New Poison Research Skill Level - 3)} + (Target HP/100)
			damage += status->vit * (sce->val1 - 3);
			unit_skillcastcancel(bl,2);
			iMap->freeblock_lock();
			status_damage(bl, bl, damage, 0, clif->damage(bl,bl,tick,status_get_amotion(bl),status_get_dmotion(bl)+500,damage,1,0,0), 1);
			if( sc->data[type] ) {
				sc_timer_next(1000 + tick, status_change_timer, bl->id, data );
			}
			iMap->freeblock_unlock();
			return 0;
		}
		break;

	case SC_MAGICMUSHROOM:
		if( --(sce->val4) >= 0 ) {
			bool flag = 0;
			int damage = status->max_hp * 3 / 100;
			if( status->hp <= damage )
				damage = status->hp - 1; // Cannot Kill

			if( damage > 0 ) { // 3% Damage each 4 seconds
				iMap->freeblock_lock();
				status_zap(bl,damage,0);
				flag = !sc->data[type]; // Killed? Should not
				iMap->freeblock_unlock();
			}

			if( !flag ) { // Random Skill Cast
				if (sd && !pc_issit(sd)) { //can't cast if sit
					int mushroom_skill_id = 0, i;
					unit_stop_attack(bl);
					unit_skillcastcancel(bl,1);
					do {
						i = rnd() % MAX_SKILL_MAGICMUSHROOM_DB;
						mushroom_skill_id = skill_magicmushroom_db[i].skill_id;
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
				sc_timer_next(4000+tick,status_change_timer,bl->id,data);
			}
			return 0;
		}
		break;

	case SC_TOXIN:
		if( --(sce->val4) >= 0 )
		{ //Damage is every 10 seconds including 3%sp drain.
			iMap->freeblock_lock();
			clif->damage(bl,bl,tick,status_get_amotion(bl),1,1,0,0,0);
			status_damage(NULL, bl, 1, status->max_sp * 3 / 100, 0, 0); //cancel dmg only if cancelable
			if( sc->data[type] ) {
				sc_timer_next(10000 + tick, status_change_timer, bl->id, data );
			}
			iMap->freeblock_unlock();
			return 0;
		}
		break;

	case SC_OBLIVIONCURSE:
		if( --(sce->val4) >= 0 )
		{
			clif->emotion(bl,E_WHAT);
			sc_timer_next(3000 + tick, status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_WEAPONBLOCKING:
		if( --(sce->val4) >= 0 )
		{
			if( !status_charge(bl,0,3) )
				break;
			sc_timer_next(3000+tick,status_change_timer,bl->id,data);
			return 0;
		}
		break;

	case SC_CLOAKINGEXCEED:
		if(!status_charge(bl,0,10-sce->val1))
			break;
		sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
		return 0;

	case SC_RENOVATIO:
		if( --(sce->val4) >= 0 )
		{
			int heal = status->max_hp * 3 / 100;
			if( sc && sc->data[SC_AKAITSUKI] && heal )
				heal = ~heal + 1;
			status_heal(bl, heal, 0, 2);
			sc_timer_next(5000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_BURNING:
		if( --(sce->val4) >= 0 )
		{
			struct block_list *src = iMap->id2bl(sce->val3);
			int damage = 1000 + 3 * status_get_max_hp(bl) / 100; // Deals fixed (1000 + 3%*MaxHP)

			iMap->freeblock_lock();
			clif->damage(bl,bl,tick,0,0,damage,1,9,0); //damage is like endure effect with no walk delay
			status_damage(src, bl, damage, 0, 0, 1);

			if( sc->data[type]){ // Target still lives. [LimitLine]
				sc_timer_next(2000 + tick, status_change_timer, bl->id, data);
			}
			iMap->freeblock_unlock();
			return 0;
		}
		break;

	case SC_FEAR:
		if( --(sce->val4) >= 0 )
		{
			if( sce->val2 > 0 )
				sce->val2--;
			sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_SPHERE_1:
	case SC_SPHERE_2:
	case SC_SPHERE_3:
	case SC_SPHERE_4:
	case SC_SPHERE_5:
		if( --(sce->val4) >= 0 )
		{
			if( !status_charge(bl, 0, 1) )
				break;
			sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_READING_SB:
		if( !status_charge(bl, 0, sce->val2) ){
			int i;
			for(i = SC_SPELLBOOK1; i <= SC_MAXSPELLBOOK; i++) // Also remove stored spell as well.
				status_change_end(bl, (sc_type)i, INVALID_TIMER);
			break;
		}
		sc_timer_next(5000 + tick, status_change_timer, bl->id, data);
		return 0;

	case SC_ELECTRICSHOCKER:
		if( --(sce->val4) >= 0 )
		{
			status_charge(bl, 0, status->max_sp / 100 * sce->val1 );
			sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_CAMOUFLAGE:
		if(--(sce->val4) > 0){
			status_charge(bl,0,7 - sce->val1);
			sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC__REPRODUCE:
		if(!status_charge(bl, 0, 1))
			break;
		sc_timer_next(1000+tick, status_change_timer, bl->id, data);
		return 0;

	case SC__SHADOWFORM:
		if( --(sce->val4) >= 0 )
		{
			if( !status_charge(bl, 0, sce->val1 - (sce->val1 - 1)) )
				break;
			sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC__INVISIBILITY:
		if( --(sce->val4) >= 0 )
		{
			if( !status_charge(bl, 0, (status->sp * 6 - sce->val1) / 100) )// 6% - skill_lv.
				break;
			sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_STRIKING:
		if( --(sce->val4) >= 0 )
		{
			if( !status_charge(bl,0, sce->val1 ) )
				break;
			sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;
	case SC_VACUUM_EXTREME:
		if( --(sce->val4) >= 0 ){
			sc_timer_next(100 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;
	case SC_BLOODSUCKER:
		if( --(sce->val4) >= 0 ) {
			struct block_list *src = iMap->id2bl(sce->val2);
			int damage;
			if( !src || (src && (status_isdead(src) || src->m != bl->m || distance_bl(src, bl) >= 12)) )
				break;
			iMap->freeblock_lock();
			damage =  200 + 100 * sce->val1 + status_get_int(src);
			status_damage(src, bl, damage, 0, clif->damage(bl,bl,tick,status->amotion,status->dmotion+200,damage,1,0,0), 1);
			unit_skillcastcancel(bl,1);
			if ( sc->data[type] ) {
				sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
			}
			iMap->freeblock_unlock();
			status_heal(src, damage*(5 + 5 * sce->val1)/100, 0, 0); // 5 + 5% per level
			return 0;
		}
		break;

	case SC_VOICEOFSIREN:
		if( --(sce->val4) >= 0 )
		{
			clif->emotion(bl,E_LV);
			sc_timer_next(2000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_DEEPSLEEP:
		if( --(sce->val4) >= 0 )
		{ // Recovers 1% HP/SP every 2 seconds.
			status_heal(bl, status->max_hp / 100, status->max_sp / 100, 2);
			sc_timer_next(2000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_SIRCLEOFNATURE:
		if( --(sce->val4) >= 0 )
		{
			if( !status_charge(bl,0,sce->val2) )
				break;
			status_heal(bl, sce->val3, 0, 1);
			sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_SONGOFMANA:
		if( --(sce->val4) >= 0 )
		{
			status_heal(bl,0,sce->val3,3);
			sc_timer_next(3000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;


	case SC_SATURDAYNIGHTFEVER:
		// 1% HP/SP drain every val4 seconds [Jobbie]
		if( --(sce->val3) >= 0 )
		{
			int hp = status->hp / 100;
			int sp = status->sp / 100;
			if( !status_charge(bl, hp, sp) )
				break;
			sc_timer_next(sce->val4+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_CRYSTALIZE:
		if( --(sce->val4) >= 0 )
		{ // Drains 2% of HP and 1% of SP every seconds.
			if( bl->type != BL_MOB) // doesn't work on mobs
				status_charge(bl, status->max_hp * 2 / 100, status->max_sp / 100);
			sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_FORCEOFVANGUARD:
		if( !status_charge(bl,0,20) )
			break;
		sc_timer_next(6000 + tick, status_change_timer, bl->id, data);
		return 0;

	case SC_BANDING:
		if( status_charge(bl, 0, 7 - sce->val1) )
		{
			if( sd ) iPc->banding(sd, sce->val1);
			sc_timer_next(5000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_REFLECTDAMAGE:
		if( --(sce->val4) >= 0 ) {
			if( !status_charge(bl,0,sce->val3) )
				break;
			sc_timer_next(10000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;

	case SC_OVERHEAT_LIMITPOINT:
		if( --(sce->val1) > 0 ) { // Cooling
			sc_timer_next(30000 + tick, status_change_timer, bl->id, data);
		}
		break;

	case SC_OVERHEAT:
		{
			int damage = status->max_hp / 100; // Suggestion 1% each second
			if( damage >= status->hp ) damage = status->hp - 1; // Do not kill, just keep you with 1 hp minimum
			iMap->freeblock_lock();
			status_fix_damage(NULL,bl,damage,clif->damage(bl,bl,tick,0,0,damage,0,0,0));
			if( sc->data[type] ) {
				sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
			}
			iMap->freeblock_unlock();
		}
		break;

	case SC_MAGNETICFIELD:
		{
			if( --(sce->val3) <= 0 )
				break; // Time out
			if( sce->val2 == bl->id )
			{
				if( !status_charge(bl,0,14 + (3 * sce->val1)) )
					break; // No more SP status should end, and in the next second will end for the other affected players
			}
			else
			{
				struct block_list *src = iMap->id2bl(sce->val2);
				struct status_change *ssc;
				if( !src || (ssc = status_get_sc(src)) == NULL || !ssc->data[SC_MAGNETICFIELD] )
					break; // Source no more under Magnetic Field
			}
			sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
		}
		break;

	case SC_INSPIRATION:
		if(--(sce->val4) >= 0)
		{
			int hp = status->max_hp * (7-sce->val1) / 100;
			int sp = status->max_sp * (9-sce->val1) / 100;

			if( !status_charge(bl,hp,sp) ) break;

			sc_timer_next(1000+tick,status_change_timer,bl->id, data);
			return 0;
		}
		break;

	case SC_RAISINGDRAGON:
		// 1% every 5 seconds [Jobbie]
		if( --(sce->val3)>0 && status_charge(bl, sce->val2, 0) )
		{
			if( !sc->data[type] ) return 0;
			sc_timer_next(5000 + tick, status_change_timer, bl->id, data);
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
		if( !status_charge(bl,0,sce->val2) ){
			struct block_list *s_bl = battle->get_master(bl);
			if( s_bl )
				status_change_end(s_bl,type+1,INVALID_TIMER);
			status_change_end(bl,type,INVALID_TIMER);
			break;
		}
		sc_timer_next(2000 + tick, status_change_timer, bl->id, data);
		return 0;

	case SC_STOMACHACHE:
		if( --(sce->val4) > 0 ){
			status_charge(bl,0,sce->val2);	// Reduce 8 every 10 seconds.
			if( sd && !pc_issit(sd) )	// Force to sit every 10 seconds.
			{
				pc_stop_walking(sd,1|4);
				pc_stop_attack(sd);
				pc_setsit(sd);
				clif->sitting(bl);
			}
			sc_timer_next(10000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;
	case SC_LEADERSHIP:
	case SC_GLORYWOUNDS:
	case SC_SOULCOLD:
	case SC_HAWKEYES:
		/* they only end by status_change_end */
		sc_timer_next(600000 + tick, status_change_timer, bl->id, data);
		return 0;
	case SC_MEIKYOUSISUI:
		if( --(sce->val4) > 0 ){
			status_heal(bl, status->max_hp * (sce->val1+1) / 100, status->max_sp * sce->val1 / 100, 0);
			sc_timer_next(1000 + tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;
	case SC_IZAYOI:
	case SC_KAGEMUSYA:
		if( --(sce->val2) > 0 ){
			if(!status_charge(bl, 0, 1)) break;
			sc_timer_next(1000+tick, status_change_timer, bl->id, data);
			return 0;
		}
		break;
	case SC_ANGRIFFS_MODUS:
		if(--(sce->val4) >= 0) { //drain hp/sp
			if( !status_charge(bl,100,20) ) break;
			sc_timer_next(1000+tick,status_change_timer,bl->id, data);
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
int status_change_timer_sub(struct block_list* bl, va_list ap)
{
	struct status_change* tsc;

	struct block_list* src = va_arg(ap,struct block_list*);
	struct status_change_entry* sce = va_arg(ap,struct status_change_entry*);
	enum sc_type type = (sc_type)va_arg(ap,int); //gcc: enum args get promoted to int
	unsigned int tick = va_arg(ap,unsigned int);

	if (status_isdead(bl))
		return 0;

	tsc = status_get_sc(bl);

	switch( type ) {
    case SC_SIGHT: /* Reveal hidden ennemy on 3*3 range */
		if( tsc && tsc->data[SC__SHADOWFORM] && (sce && sce->val4 >0 && sce->val4%2000 == 0) && // for every 2 seconds do the checking
			rnd()%100 < 100-tsc->data[SC__SHADOWFORM]->val1*10 ) // [100 - (Skill Level x 10)] %
				status_change_end(bl, SC__SHADOWFORM, INVALID_TIMER);
	case SC_CONCENTRATE:
		status_change_end(bl, SC_HIDING, INVALID_TIMER);
		status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
		status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
		status_change_end(bl, SC_CAMOUFLAGE, INVALID_TIMER);
		status_change_end(bl, SC__INVISIBILITY, INVALID_TIMER);
		break;
    case SC_RUWACH: /* Reveal hidden target and deal little dammages if ennemy */
		if (tsc && (tsc->data[SC_HIDING] || tsc->data[SC_CLOAKING] ||
				tsc->data[SC_CAMOUFLAGE] || tsc->data[SC_CLOAKINGEXCEED] ||
					tsc->data[SC__INVISIBILITY])) {
			status_change_end(bl, SC_HIDING, INVALID_TIMER);
			status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
			status_change_end(bl, SC_CAMOUFLAGE, INVALID_TIMER);
			status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
			status_change_end(bl, SC__INVISIBILITY, INVALID_TIMER);
			if(battle->check_target( src, bl, BCT_ENEMY ) > 0)
				skill->attack(BF_MAGIC,src,src,bl,AL_RUWACH,1,tick,0);
		}
		if( tsc && tsc->data[SC__SHADOWFORM] && (sce && sce->val4 >0 && sce->val4%2000 == 0) && // for every 2 seconds do the checking
			rnd()%100 < 100-tsc->data[SC__SHADOWFORM]->val1*10 ) // [100 - (Skill Level x 10)] %
				status_change_end(bl, SC__SHADOWFORM, INVALID_TIMER);
		break;
	case SC_SIGHTBLASTER:
		if (battle->check_target( src, bl, BCT_ENEMY ) > 0 &&
			status_check_skilluse(src, bl, WZ_SIGHTBLASTER, 2))
		{
			if (sce && !(bl->type&BL_SKILL) //The hit is not counted if it's against a trap
				&& skill->attack(BF_MAGIC,src,src,bl,WZ_SIGHTBLASTER,1,tick,0)){
				sce->val2 = 0; //This signals it to end.
			}
		}
		break;
	case SC_CLOSECONFINE:
		//Lock char has released the hold on everyone...
		if (tsc && tsc->data[SC_CLOSECONFINE2] && tsc->data[SC_CLOSECONFINE2]->val2 == src->id) {
			tsc->data[SC_CLOSECONFINE2]->val2 = 0;
			status_change_end(bl, SC_CLOSECONFINE2, INVALID_TIMER);
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

/*==========================================
 * Clears buffs/debuffs of a character.
 * type&1 -> buffs, type&2 -> debuffs
 * type&4 -> especific debuffs(implemented with refresh)
 *------------------------------------------*/
int status_change_clear_buffs (struct block_list* bl, int type)
{
	int i;
	struct status_change *sc= status_get_sc(bl);

	if (!sc || !sc->count)
		return 0;

    if (type&6) //Debuffs
        for (i = SC_COMMON_MIN; i <= SC_COMMON_MAX; i++)
            status_change_end(bl, (sc_type)i, INVALID_TIMER);

	for( i = SC_COMMON_MAX+1; i < SC_MAX; i++ )
	{
		if(!sc->data[i])
			continue;

		switch (i) {
			//Stuff that cannot be removed
			case SC_WEIGHT50:
			case SC_WEIGHT90:
			case SC_COMBO:
			case SC_SMA:
			case SC_DANCING:
			case SC_LEADERSHIP:
			case SC_GLORYWOUNDS:
			case SC_SOULCOLD:
			case SC_HAWKEYES:
			case SC_GUILDAURA:
			case SC_SAFETYWALL:
			case SC_PNEUMA:
			case SC_NOCHAT:
			case SC_JAILED:
			case SC_ANKLE:
			case SC_BLADESTOP:
			case SC_CP_WEAPON:
			case SC_CP_SHIELD:
			case SC_CP_ARMOR:
			case SC_CP_HELM:
			case SC_STRFOOD:
			case SC_AGIFOOD:
			case SC_VITFOOD:
			case SC_INTFOOD:
			case SC_DEXFOOD:
			case SC_LUKFOOD:
			case SC_HITFOOD:
			case SC_FLEEFOOD:
			case SC_BATKFOOD:
			case SC_WATKFOOD:
			case SC_MATKFOOD:
			case SC_FOOD_STR_CASH:
			case SC_FOOD_AGI_CASH:
			case SC_FOOD_VIT_CASH:
			case SC_FOOD_DEX_CASH:
			case SC_FOOD_INT_CASH:
			case SC_FOOD_LUK_CASH:
			case SC_EXPBOOST:
			case SC_JEXPBOOST:
			case SC_ITEMBOOST:
			case SC_ELECTRICSHOCKER:
			case SC__MANHOLE:
			case SC_GIANTGROWTH:
			case SC_MILLENNIUMSHIELD:
			case SC_REFRESH:
			case SC_STONEHARDSKIN:
			case SC_VITALITYACTIVATION:
			case SC_FIGHTINGSPIRIT:
			case SC_ABUNDANCE:
			case SC_CURSEDCIRCLE_ATKER:
			case SC_CURSEDCIRCLE_TARGET:
			case SC_PUSH_CART:
			case SC_ALL_RIDING:
				continue;

		 //Debuffs that can be removed.
			case SC_DEEPSLEEP:
			case SC_BURNING:
			case SC_FREEZING:
			case SC_CRYSTALIZE:
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
			case SC_HALLUCINATION:
			case SC_QUAGMIRE:
			case SC_SIGNUMCRUCIS:
			case SC_DECREASEAGI:
			case SC_SLOWDOWN:
			case SC_MINDBREAKER:
			case SC_WINKCHARM:
			case SC_STOP:
			case SC_ORCISH:
			case SC_STRIPWEAPON:
			case SC_STRIPSHIELD:
			case SC_STRIPARMOR:
			case SC_STRIPHELM:
			case SC_BITE:
			case SC_ADORAMUS:
			case SC_VACUUM_EXTREME:
			case SC_FEAR:
			case SC_MAGNETICFIELD:
			case SC_NETHERWORLD:
				if (!(type&2))
					continue;
				break;
				//The rest are buffs that can be removed.
			case SC__BLOODYLUST:
			case SC_BERSERK:
			case SC_SATURDAYNIGHTFEVER:
				if (!(type&1))
					continue;
				sc->data[i]->val2 = 0;
				break;
			default:
				if (!(type&1))
					continue;
				break;
		}
		status_change_end(bl, (sc_type)i, INVALID_TIMER);
	}
	return 0;
}

int status_change_spread( struct block_list *src, struct block_list *bl ) {
	int i, flag = 0;
	struct status_change *sc = status_get_sc(src);
	const struct TimerData *timer;
	unsigned int tick;
	struct status_change_data data;

	if( !sc || !sc->count )
		return 0;

	tick = iTimer->gettick();

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
			case SC_HALLUCINATION:
			case SC_SIGNUMCRUCIS:
			case SC_DECREASEAGI:
			case SC_SLOWDOWN:
			case SC_MINDBREAKER:
			case SC_WINKCHARM:
			case SC_STOP:
			case SC_ORCISH:
			//case SC_STRIPWEAPON://Omg I got infected and had the urge to strip myself physically.
			//case SC_STRIPSHIELD://No this is stupid and shouldnt be spreadable at all.
			//case SC_STRIPARMOR:// Disabled until I can confirm if it does or not. [Rytech]
			//case SC_STRIPHELM:
			//case SC__STRIPACCESSORY:
			case SC_BITE:
			case SC_FREEZING:
			case SC_VENOMBLEED:
			case SC_DEATHHURT:
			case SC_PARALYSE:
				if( sc->data[i]->timer != INVALID_TIMER ) {
					timer = iTimer->get_timer(sc->data[i]->timer);
					if (timer == NULL || timer->func != status_change_timer || DIFF_TICK(timer->tick,tick) < 0)
						continue;
					data.tick = DIFF_TICK(timer->tick,tick);
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
			case SC_BLEEDING:
				data.tick = sc->data[i]->val4 * 10000;
				break;
			default:
					continue;
				break;
		}
		if( i ){
			data.val1 = sc->data[i]->val1;
			data.val2 = sc->data[i]->val2;
			data.val3 = sc->data[i]->val3;
			data.val4 = sc->data[i]->val4;
			status_change_start(bl,(sc_type)i,10000,data.val1,data.val2,data.val3,data.val4,data.tick,1|2|8);
			flag = 1;
		}
	}

	return flag;
}

//Natural regen related stuff.
static unsigned int natural_heal_prev_tick,natural_heal_diff_tick;
static int status_natural_heal(struct block_list* bl, va_list args)
{
	struct regen_data *regen;
	struct status_data *status;
	struct status_change *sc;
	struct unit_data *ud;
	struct view_data *vd = NULL;
	struct regen_data_sub *sregen;
	struct map_session_data *sd;
	int val,rate,bonus = 0,flag;

	regen = status_get_regen_data(bl);
	if (!regen) return 0;
	status = status_get_status_data(bl);
	sc = status_get_sc(bl);
	if (sc && !sc->count)
		sc = NULL;
	sd = BL_CAST(BL_PC,bl);

	flag = regen->flag;
	if (flag&RGN_HP && (status->hp >= status->max_hp || regen->state.block&1))
		flag&=~(RGN_HP|RGN_SHP);
	if (flag&RGN_SP && (status->sp >= status->max_sp || regen->state.block&2))
		flag&=~(RGN_SP|RGN_SSP);

	if (flag && (
		status_isdead(bl) ||
		(sc && (sc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) || sc->data[SC__INVISIBILITY]))
	))
		flag=0;

	if (sd) {
		if (sd->hp_loss.value || sd->sp_loss.value)
			iPc->bleeding(sd, natural_heal_diff_tick);
		if (sd->hp_regen.value || sd->sp_regen.value)
			iPc->regen(sd, natural_heal_diff_tick);
	}

	if(flag&(RGN_SHP|RGN_SSP) && regen->ssregen &&
		(vd = status_get_viewdata(bl)) && vd->dead_sit == 2)
	{	//Apply sitting regen bonus.
		sregen = regen->ssregen;
		if(flag&(RGN_SHP))
		{	//Sitting HP regen
			val = natural_heal_diff_tick * sregen->rate.hp;
			if (regen->state.overweight)
				val>>=1; //Half as fast when overweight.
			sregen->tick.hp += val;
			while(sregen->tick.hp >= (unsigned int)battle_config.natural_heal_skill_interval)
			{
				sregen->tick.hp -= battle_config.natural_heal_skill_interval;
				if(status_heal(bl, sregen->hp, 0, 3) < sregen->hp)
				{	//Full
					flag&=~(RGN_HP|RGN_SHP);
					break;
				}
			}
		}
		if(flag&(RGN_SSP))
		{	//Sitting SP regen
			val = natural_heal_diff_tick * sregen->rate.sp;
			if (regen->state.overweight)
				val>>=1; //Half as fast when overweight.
			sregen->tick.sp += val;
			while(sregen->tick.sp >= (unsigned int)battle_config.natural_heal_skill_interval)
			{
				sregen->tick.sp -= battle_config.natural_heal_skill_interval;
				if(status_heal(bl, 0, sregen->sp, 3) < sregen->sp)
				{	//Full
					flag&=~(RGN_SP|RGN_SSP);
					break;
				}
			}
		}
	}

	if (flag && regen->state.overweight)
		flag=0;

	ud = unit_bl2ud(bl);

	if (flag&(RGN_HP|RGN_SHP|RGN_SSP) && ud && ud->walktimer != INVALID_TIMER)
	{
		flag&=~(RGN_SHP|RGN_SSP);
		if(!regen->state.walk)
			flag&=~RGN_HP;
	}

	if (!flag)
		return 0;

	if (flag&(RGN_HP|RGN_SP))
	{
		if(!vd) vd = status_get_viewdata(bl);
		if(vd && vd->dead_sit == 2)
			bonus++;
		if(regen->state.gc)
			bonus++;
	}

	//Natural Hp regen
	if (flag&RGN_HP)
	{
		rate = natural_heal_diff_tick*(regen->rate.hp+bonus);
		if (ud && ud->walktimer != INVALID_TIMER)
			rate/=2;
		// Homun HP regen fix (they should regen as if they were sitting (twice as fast)
		if(bl->type==BL_HOM) rate *=2;

		regen->tick.hp += rate;

		if(regen->tick.hp >= (unsigned int)battle_config.natural_healhp_interval)
		{
			val = 0;
			do {
				val += regen->hp;
				regen->tick.hp -= battle_config.natural_healhp_interval;
			} while(regen->tick.hp >= (unsigned int)battle_config.natural_healhp_interval);
			if (status_heal(bl, val, 0, 1) < val)
				flag&=~RGN_SHP; //full.
		}
	}

	//Natural SP regen
	if(flag&RGN_SP)
	{
		rate = natural_heal_diff_tick*(regen->rate.sp+bonus);
		// Homun SP regen fix (they should regen as if they were sitting (twice as fast)
		if(bl->type==BL_HOM) rate *=2;

		regen->tick.sp += rate;

		if(regen->tick.sp >= (unsigned int)battle_config.natural_healsp_interval)
		{
			val = 0;
			do {
				val += regen->sp;
				regen->tick.sp -= battle_config.natural_healsp_interval;
			} while(regen->tick.sp >= (unsigned int)battle_config.natural_healsp_interval);
			if (status_heal(bl, 0, val, 1) < val)
				flag&=~RGN_SSP; //full.
		}
	}

	if (!regen->sregen)
		return flag;

	//Skill regen
	sregen = regen->sregen;

	if(flag&RGN_SHP)
	{	//Skill HP regen
		sregen->tick.hp += natural_heal_diff_tick * sregen->rate.hp;

		while(sregen->tick.hp >= (unsigned int)battle_config.natural_heal_skill_interval)
		{
			sregen->tick.hp -= battle_config.natural_heal_skill_interval;
			if(status_heal(bl, sregen->hp, 0, 3) < sregen->hp)
				break; //Full
		}
	}
	if(flag&RGN_SSP)
	{	//Skill SP regen
		sregen->tick.sp += natural_heal_diff_tick * sregen->rate.sp;
		while(sregen->tick.sp >= (unsigned int)battle_config.natural_heal_skill_interval)
		{
			val = sregen->sp;
			if (sd && sd->state.doridori) {
				val*=2;
				sd->state.doridori = 0;
				if ((rate = iPc->checkskill(sd,TK_SPTIME)))
					sc_start(bl,status_skill2sc(TK_SPTIME),
						100,rate,skill->get_time(TK_SPTIME, rate));
				if (
					(sd->class_&MAPID_UPPERMASK) == MAPID_STAR_GLADIATOR &&
					rnd()%10000 < battle_config.sg_angel_skill_ratio
				) { //Angel of the Sun/Moon/Star
					clif->feel_hate_reset(sd);
					iPc->resethate(sd);
					iPc->resetfeel(sd);
				}
			}
			sregen->tick.sp -= battle_config.natural_heal_skill_interval;
			if(status_heal(bl, 0, val, 3) < val)
				break; //Full
		}
	}
	return flag;
}

//Natural heal main timer.
static int status_natural_heal_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	natural_heal_diff_tick = DIFF_TICK(tick,natural_heal_prev_tick);
	iMap->map_foreachregen(status_natural_heal);
	natural_heal_prev_tick = tick;
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

	return refine_info[wlv].chance[refine];
}


/*------------------------------------------
 * DB reading.
 * job_db1.txt    - weight, hp, sp, aspd
 * job_db2.txt    - job level stat bonuses
 * size_fix.txt   - size adjustment table for weapons
 * refine_db.txt  - refining data table
 *------------------------------------------*/
static bool status_readdb_job1(char* fields[], int columns, int current)
{// Job-specific values (weight, HP, SP, ASPD)
	int idx, class_;
	unsigned int i;

	class_ = atoi(fields[0]);

	if(!pcdb_checkid(class_))
	{
		ShowWarning("status_readdb_job1: Invalid job class %d specified.\n", class_);
		return false;
	}
	idx = iPc->class2idx(class_);

	max_weight_base[idx] = atoi(fields[1]);
	hp_coefficient[idx]  = atoi(fields[2]);
	hp_coefficient2[idx] = atoi(fields[3]);
	sp_coefficient[idx]  = atoi(fields[4]);
#ifdef RENEWAL_ASPD
	for(i = 0; i <= MAX_WEAPON_TYPE; i++)
#else
	for(i = 0; i < MAX_WEAPON_TYPE; i++)
#endif
	{
		aspd_base[idx][i] = atoi(fields[i+5]);
	}
	return true;
}

static bool status_readdb_job2(char* fields[], int columns, int current)
{
	int idx, class_, i;

	class_ = atoi(fields[0]);

	if(!pcdb_checkid(class_))
	{
		ShowWarning("status_readdb_job2: Invalid job class %d specified.\n", class_);
		return false;
	}
	idx = iPc->class2idx(class_);

	for(i = 1; i < columns; i++)
	{
		job_bonus[idx][i-1] = atoi(fields[i]);
	}
	return true;
}

static bool status_readdb_sizefix(char* fields[], int columns, int current)
{
	unsigned int i;

	for(i = 0; i < MAX_WEAPON_TYPE; i++)
	{
		atkmods[current][i] = atoi(fields[i]);
	}
	return true;
}

static bool status_readdb_refine(char* fields[], int columns, int current)
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

		refine_info[current].chance[i] = atoi(fields[4+i]);

		if (i >= random_bonus_start_level - 1)
			refine_info[current].randombonus_max[i] = random_bonus * (i - random_bonus_start_level + 2);

		refine_info[current].bonus[i] = bonus_per_level + atoi(delim+1);
		if (i > 0)
			refine_info[current].bonus[i] += refine_info[current].bonus[i-1];
	}
	return true;
}

/*
* Read status db
* job1.txt
* job2.txt
* size_fixe.txt
* refine_db.txt
*/
int status_readdb(void)
{
	int i, j;

	// initialize databases to default
	//

	// reset job_db1.txt data
	memset(max_weight_base, 0, sizeof(max_weight_base));
	memset(hp_coefficient, 0, sizeof(hp_coefficient));
	memset(hp_coefficient2, 0, sizeof(hp_coefficient2));
	memset(sp_coefficient, 0, sizeof(sp_coefficient));
	memset(aspd_base, 0, sizeof(aspd_base));
	// reset job_db2.txt data
	memset(job_bonus,0,sizeof(job_bonus)); // Job-specific stats bonus

	// size_fix.txt
	for(i=0;i<ARRAYLENGTH(atkmods);i++)
		for(j=0;j<MAX_WEAPON_TYPE;j++)
			atkmods[i][j]=100;

	// refine_db.txt
	for(i=0;i<ARRAYLENGTH(refine_info);i++)
	{
		for(j=0;j<MAX_REFINE; j++)
		{
			refine_info[i].chance[j] = 100;
			refine_info[i].bonus[j] = 0;
			refine_info[i].randombonus_max[j] = 0;
		}
	}

	// read databases
	//


#ifdef RENEWAL_ASPD
	sv->readdb(iMap->db_path, "re/job_db1.txt",   ',',	6+MAX_WEAPON_TYPE, 6+MAX_WEAPON_TYPE,	-1,		&status_readdb_job1);
#else
	sv->readdb(iMap->db_path, "pre-re/job_db1.txt",   ',',	5+MAX_WEAPON_TYPE, 5+MAX_WEAPON_TYPE,	-1,		&status_readdb_job1);
#endif
	sv->readdb(iMap->db_path, "job_db2.txt",   ',', 1,                 1+MAX_LEVEL,       -1,                            &status_readdb_job2);
	sv->readdb(iMap->db_path, "size_fix.txt",  ',', MAX_WEAPON_TYPE,   MAX_WEAPON_TYPE,    ARRAYLENGTH(atkmods),         &status_readdb_sizefix);
	sv->readdb(iMap->db_path, DBPATH"refine_db.txt", ',', 4+MAX_REFINE, 4+MAX_REFINE, ARRAYLENGTH(refine_info), &status_readdb_refine);

	return 0;
}

/*==========================================
 * Status db init and destroy.
 *------------------------------------------*/
int do_init_status(void)
{
	iTimer->add_timer_func_list(status_change_timer,"status_change_timer");
	iTimer->add_timer_func_list(kaahi_heal_timer,"kaahi_heal_timer");
	iTimer->add_timer_func_list(status_natural_heal_timer,"status_natural_heal_timer");
	initChangeTables();
	initDummyData();
	status_readdb();
	status_calc_sigma();
	natural_heal_prev_tick = iTimer->gettick();
	sc_data_ers = ers_new(sizeof(struct status_change_entry),"status.c::sc_data_ers",ERS_OPT_NONE);
	iTimer->add_timer_interval(natural_heal_prev_tick + NATURAL_HEAL_INTERVAL, status_natural_heal_timer, 0, 0, NATURAL_HEAL_INTERVAL);
	return 0;
}
void do_final_status(void)
{
	ers_destroy(sc_data_ers);
}
