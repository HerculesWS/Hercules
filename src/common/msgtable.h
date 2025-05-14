/**
 * Messages from messages.conf
 */
enum msgtable_messages {
	// 0 ~ 499: Messages of GM commands
	// -----------------------
	/** Warped. */
	MSGTBL_WARPED = 0,
	/** Map not found. */
	MSGTBL_MAP_NOT_FOUND = 1,
	/** Invalid coordinates, using random target cell. */
	MSGTBL_INVALID_COORDINATES = 2,
	/** Character not found. */
	MSGTBL_CHARACTER_NOT_FOUND = 3,
	/** Jump to %s */
	MSGTBL_JUMP_TO_NAME = 4,
	/** Jump to %d %d */
	MSGTBL_JUMP_TO_XY = 5,
	/** Your save point has been changed. */
	MSGTBL_SAVE_POINT_CHANGED = 6,
	/** Warping to save point. */
	MSGTBL_WARP_TO_SAVE_POINT = 7,
	/** Speed changed. */
	MSGTBL_SPEED_CHANGED = 8,
	/** Options changed. */
	MSGTBL_OPTIONS_CHANGED = 9,
	/** Invisible: Off */
	MSGTBL_INVISIBLE_OFF = 10,
	/** Invisible: On */
	MSGTBL_INVISIBLE_ON = 11,
	/** Your job has been changed. */
	MSGTBL_JOB_CHANGED = 12,
	/** You've died. */
	MSGTBL_YOU_DIED = 13,
	/** Character killed. */
	MSGTBL_CHARACTER_KILLED = 14,
	/** Unknown */
	MSGTBL_UNKNOWN = 15,
	/** You've been revived! */
	MSGTBL_REVIVED = 16,
	/** HP, SP recovered. */
	MSGTBL_HP_SP_RECOVERED = 17,
	/** Item created. */
	MSGTBL_ITEM_CREATED = 18,
	/** Invalid item ID or name. */
	MSGTBL_INVALID_ITEMID_NAME = 19,
	/** All of your items have been removed. */
	MSGTBL_ITEMS_REMOVED = 20,
	/** Base level raised. */
	MSGTBL_BASE_LEVEL_RAISED = 21,
	/** Base level lowered. */
	MSGTBL_BASE_LEVEL_LOWERED = 22,
	/** Job level can't go any higher. */
	MSGTBL_CANT_INCREASE_JOB_LEVEL = 23,
	/** Job level raised. */
	MSGTBL_JOB_LEVEL_RAISED = 24,
	/** Job level lowered. */
	MSGTBL_JOB_LEVEL_LOWERED = 25,
	/** [%d] seconds left until you can use */
	MSGTBL_SECONDS_UNTIL_USE = 26,
	/** Storage has been not loaded yet. */
	MSGTBL_STORAGE_NOT_LOADED = 27,
	/** No player found. */
	MSGTBL_NO_PLAYER_FOUND = 28,
	/** 1 player found. */
	MSGTBL_1_PLAYER_FOUND = 29,
	/** %d players found. */
	MSGTBL_N_PLAYERS_FOUND = 30,
	/** PvP: Off. */
	MSGTBL_PVP_OFF = 31,
	/** PvP: On. */
	MSGTBL_PVP_ON = 32,
	/** GvG: Off. */
	MSGTBL_GVG_OFF = 33,
	/** GvG: On. */
	MSGTBL_GVG_ON = 34,
	/** This job has no alternate body styles. */
	MSGTBL_NO_ALTERNATE_BODY_AVAILABLE = 35,
	/** Appearance changed. */
	MSGTBL_APPEARANCE_CHANGED = 36,
	/** An invalid number was specified. */
	MSGTBL_INVALID_NUMBER = 37,
	/** Invalid location number, or name. */
	MSGTBL_INVALID_LOCATION_OR_NAME = 38,
	/** All monsters summoned! */
	MSGTBL_MONSTERS_SUMMONED = 39,
	/** Invalid monster ID or name. */
	MSGTBL_INVALID_MONSTER_ID_NAME = 40,
	/** Unable to decrease the number/value. */
	MSGTBL_UNABLE_TO_DECREASE_VALUE = 41,
	/** Stat changed. */
	MSGTBL_STAT_CHANGED = 42,
	/** You're not in a guild. */
	MSGTBL_NOT_IN_A_GUILD = 43,
	/** You're not the master of your guild. */
	MSGTBL_NOT_GUILDMASTER = 44,
	/** Guild level change failed. */
	MSGTBL_GUILD_LEVEL_CHANGE_FAILED = 45,
	/** %s recalled! */
	MSGTBL_RECALLED_NAME = 46,
	/** Base level can't go any higher. */
	MSGTBL_CANT_INCREASE_BASE_LEVEL = 47,
	/** Any work in progress (NPC dialog, manufacturing ...) quit and try again. */
	MSGTBL_WORK_IN_PROGRESS_TRY_AGAIN = 48,
	/** Unable to Teleport in this area */
	MSGTBL_TELEPORT_DISABLED_IN_AREA = 49,
	/** This skill cannot be used within this area. */
	MSGTBL_SKILL_DISABLED_IN_AREA = 50,
	/** This item cannot be used within this area. */
	MSGTBL_ITEM_DISABLED_IN_AREA = 51,
	/** This command is disabled in this area. */
	MSGTBL_COMMAND_DISABLED_IN_AREA = 52,
	/** '%s' stats: */
	MSGTBL_TARGET_STATS = 53,
	/** No player found in map '%s'. */
	MSGTBL_NO_PLAYER_IN_MAP = 54,
	/** 1 player found in map '%s'. */
	MSGTBL_1_PLAYER_IN_MAP = 55,
	/** %d players found in map '%s'. */
	MSGTBL_N_PLAYERS_IN_MAP = 56,
	// 57-58 FREE
	/** Night Mode Activated. */
	MSGTBL_NIGHT_MODE_ACTIVATED = 59,
	/** Day Mode Activated. */
	MSGTBL_DAY_MODE_ACTIVATED = 60,
	/** The holy messenger has given judgement. */
	MSGTBL_HOLY_MESSENGER_JUDGMENT = 61,
	/** Judgement has passed. */
	MSGTBL_JUDGMENT_PASSED = 62,
	/** Mercy has been shown. */
	MSGTBL_MERCY_SHOWN = 63,
	/** Mercy has been granted. */
	MSGTBL_MERCY_GRANTED = 64,
	// 65-69 FREE
	/** You have learned the skill. */
	MSGTBL_LEARNED_SKILL = 70,
	/** You have forgotten the skill. */
	MSGTBL_FORGOTTEN_SKILL = 71,

	/** War of Emperium has been initiated. */
	MSGTBL_AGITSTART_INITIATED = 72,
	/** War of Emperium is currently in progress. */
	MSGTBL_AGITSTART_ALREADY_IN_PROGRESS = 73,
	/** War of Emperium has been ended. */
	MSGTBL_AGITEND_ENDED = 74,
	/** War of Emperium is currently not in progress. */
	MSGTBL_AGITEND_NOT_IN_PROGRESS = 75,

	/** All skills have been added to your skill tree. */
	MSGTBL_ALL_SKILLS_ADDED_TO_SKILL_TREE = 76,
	/** Search results for '%s' (name: id): */
	MSGTBL_SEARCH_RESULTS_NAME = 77,
	/** %s: %d */
	MSGTBL_SEARCH_RESULT_NAME_ID = 78,
	/** %d results found. */
	MSGTBL_N_RESULTS_FOUND = 79,
	/** Please specify a display name or monster name/id. */
	MSGTBL_SPECIFY_NAME_OR_ID = 80,
	/** Your GM level doesn't authorize you to perform this action on the specified player. */
	MSGTBL_GM_LEVEL_UNAUTHORIZED = 81,
	/** Roulette is disabled. */
	MSGTBL_ROULETTE_DISABLED = 82,
	/** BG Match Canceled: not enough players. */
	MSGTBL_BG_MATCH_CANCELED_NO_PLAYERS = 83,
	/** All stats changed! */
	MSGTBL_ALL_STATS_CHANGED = 84,
	/** Invalid time for ban command. */
	MSGTBL_INVALID_BAN_TIME = 85,
	// 86-87 FREE
	/** Sending request to login server... */
	MSGTBL_SENDING_REQUEST_TO_LOGIN = 88,
	/** Night mode is already enabled. */
	MSGTBL_NIGHT_MODE_ALREADY_ENABLED = 89,
	/** Day mode is already enabled. */
	MSGTBL_DAY_MODE_ALREADY_ENABLED = 90,
	// 91 FREE
	/** All characters recalled! */
	MSGTBL_RECALL_CHARACTERS_RECALLED = 92,
	/** All online characters of the %s guild have been recalled to your position. */
	MSGTBL_RECALL_GUILD_RECALLED = 93,
	/** Incorrect name/ID, or no one from the specified guild is online. */
	MSGTBL_RECALL_INVALID_GUILD_NAME = 94,
	/** All online characters of the %s party have been recalled to your position. */
	MSGTBL_RECALL_PARTY_RECALLED = 95,
	/** Incorrect name/ID, or no one from the specified party is online. */
	MSGTBL_RECALL_INVALID_PARTY_NAME = 96,
	/** Item database has been reloaded. */
	MSGTBL_RELOAD_ITEMDB_RELOADED = 97,
	/** Monster database has been reloaded. */
	MSGTBL_RELOAD_MOBDB_RELOADED = 98,
	/** Skill database has been reloaded. */
	MSGTBL_RELOAD_SKILLDB_RELOADED = 99,
	/** Scripts have been reloaded. */
	MSGTBL_RELOAD_SCRIPTS_RELOADED = 100,
	// 101 FREE
	/** You have mounted a Peco Peco. */
	MSGTBL_MOUNTED_PECOPECO = 102,
	/** No longer spying on the %s guild. */
	MSGTBL_GUILDSPY_OFF = 103,
	/** Spying on the %s guild. */
	MSGTBL_GUILDSPY_ON = 104,
	/** No longer spying on the %s party. */
	MSGTBL_PARTYSPY_OFF = 105,
	/** Spying on the %s party. */
	MSGTBL_PARTYSPY_ON = 106,
	/** All items have been repaired. */
	MSGTBL_ALL_ITEMS_REPAIRED = 107,
	/** No item need to be repaired. */
	MSGTBL_NOTHING_TO_REPAIR = 108,
	/** Player has been nuked! */
	MSGTBL_PLAYER_NUKED = 109,
	/** NPC Enabled. */
	MSGTBL_NPC_ENABLED = 110,
	/** This NPC doesn't exist. */
	MSGTBL_NPC_DOESNT_EXIST = 111,
	/** NPC Disabled. */
	MSGTBL_NPC_DISABLED = 112,
	/** %d item(s) removed by a GM. */
	MSGTBL_N_ITEMS_REMOVED_BY_GM = 113,
	/** %d item(s) removed from the player. */
	MSGTBL_N_ITEMS_REMOVED_FROM_PLAYER = 114,
	/** %d item(s) removed. Player had only %d on %d items. */
	MSGTBL_N_ITEMS_REMOVED_AMOUNT = 115,
	/** Character does not have the specified item. */
	MSGTBL_DOES_NOT_HAVE_ITEM = 116,
	/** You have been jailed by a GM. */
	MSGTBL_JAILED_BY_GM = 117,
	/** Player warped to jail. */
	MSGTBL_WARPED_TO_JAIL = 118,
	/** This player is not in jail. */
	MSGTBL_PLAYER_NOT_IN_JAIL = 119,
	/** A GM has discharged you from jail. */
	MSGTBL_UNJAILED_BY_GM = 120,
	/** Player unjailed. */
	MSGTBL_UNJAILED = 121,
	/** Disguise applied. */
	MSGTBL_DISGUISE_ON = 122,
	/** Invalid Monster/NPC name/ID specified. */
	MSGTBL_DISGUISE_INVALID_ID = 123,
	/** Disguise removed. */
	MSGTBL_DISGUISE_OFF = 124,
	/** You're not disguised. */
	MSGTBL_DISGUISE_NOT_DISGUISE = 125,

	// Clone Messages
	/** Cannot clone a player of higher GM level than yourself. */
	MSGTBL_CLONE_HIGHER_GM = 126,
	/** You've reached your slave clones limit. */
	MSGTBL_CLONE_LIMIT_REACHED = 127,
	/** Evil clone spawned. */
	MSGTBL_EVILCLONE_SPAWNED = 128,
	/** Unable to spawn evil clone. */
	MSGTBL_EVILCLONE_FAIL = 129,
	/** Clone spawned. */
	MSGTBL_CLONE_SPAWNED = 130,
	/** Unable to spawn clone. */
	MSGTBL_CLONE_FAIL = 131,
	/** Slave clone spawned. */
	MSGTBL_SLAVECLONE_SPAWNED = 132,
	/** Unable to spawn slave clone. */
	MSGTBL_SLAVECLONE_FAIL = 133,
	// 134-136 FREE (possibly for other clone types)

	/** CvC: Off */
	MSGTBL_CVC_OFF = 137,
	/** CvC: On */
	MSGTBL_CVC_ON = 138,
	/** CvC ON | */
	MSGTBL_MAPINFO_CVC_ON = 139,
	/** You can't join in a clan if you're in a guild. */
	MSGTBL_CANT_JOIN_CLAN_WHILE_IN_GUILD = 140,
	/** CvC is already Off. */
	MSGTBL_CVC_ALREADY_OFF = 141,
	/** CvC is already On. */
	MSGTBL_CVC_ALREADY_ON = 142,
	/** Commands are disabled in this map. */
	MSGTBL_COMMANDS_DISABLE_IN_MAP = 143,
	/** Invalid e-mail. If your email hasn't been set, use a@a.com. */
	MSGTBL_EMAIL_INVALID_EMAIL = 144,
	/** Invalid new e-mail. Please enter a real e-mail address. */
	MSGTBL_EMAIL_INVALID_NEW_EMAIL = 145,
	/** New e-mail must be a real e-mail address. */
	MSGTBL_EMAIL_MUST_BE_REAL = 146,
	/** New e-mail must be different from the current e-mail address. */
	MSGTBL_EMAIL_MUST_BE_DIFFERENT = 147,
	/** Information sent to login-server via char-server. */
	MSGTBL_EMAIL_SENT_TO_LOGIN = 148,

	/** Impossible to increase the number/value. */
	MSGTBL_IMPOSSIBLE_TO_INCREASE_VALUE = 149,
	/** No GM found. */
	MSGTBL_NO_GM_FOUND = 150,
	/** 1 GM found. */
	MSGTBL_1_GM_FOUND = 151,
	/** %d GMs found. */
	MSGTBL_N_GMS_FOUND = 152,
	/** %s is Unknown Command. */
	MSGTBL_UNKNOWN_COMMAND = 153,
	/** %s failed. */
	MSGTBL_COMMAND_FAILED = 154,
	/** You are unable to change your job. */
	MSGTBL_CANT_CHANGE_JOB = 155,

	/** HP or/and SP modified. */
	MSGTBL_HEAL_RECOVERED = 156,
	/** HP and SP have already been recovered. */
	MSGTBL_HEAL_ALREADY_FULL = 157,

	/** Base level can't go any lower. */
	MSGTBL_CANT_DECREASE_BASE_LEVEL = 158,
	/** Job level can't go any lower. */
	MSGTBL_CANT_DECREASE_JOB_LEVEL = 159,

	/** PvP is already Off. */
	MSGTBL_PVP_ALREADY_OFF = 160,
	/** PvP is already On. */
	MSGTBL_PVP_ALREADY_ON = 161,
	/** GvG is already Off. */
	MSGTBL_GVG_ALREADY_OFF = 162,
	/** GvG is already On. */
	MSGTBL_GVG_ALREADY_ON = 163,
	/** You need to learn the basic skills first. */
	MSGTBL_BASIC_SKILLS_REQUIRED = 164,
	/** All monsters killed! */
	MSGTBL_ALL_MONSTERS_KILLED = 165,

	/** No item has been refined. */
	MSGTBL_REFINE_NOTHING = 166,
	/** 1 item has been refined. */
	MSGTBL_REFINE_1_ITEM_REFINED = 167,
	/** %d items have been refined. */
	MSGTBL_REFINE_N_iTEMS_REFINED = 168,

	/** The item (%d: '%s') is not equippable. */
	MSGTBL_PRODUCE_NOT_EQUIPPABLE_NAME = 169,
	/** The item is not equippable. */
	MSGTBL_PRODUCE_NOT_EQUIPPABLE = 170,

	/** %d - void */
	MSGTBL_MEMO_VOID = 171,
	/** Speed returned to normal. */
	MSGTBL_SPEED_NORMAL = 172,
	/** Please enter a valid madogear type. */
	MSGTBL_MOUNT_ENTER_VALID_MADO_TYPE = 173,
	/** Number of status points changed. */
	MSGTBL_STATS_POINTS_CHANGED = 174,
	/** Number of skill points changed. */
	MSGTBL_SKILL_POINTS_CHANGED = 175,
	/** Current amount of zeny changed. */
	MSGTBL_ZENY_CHANGED = 176,
	/** You cannot decrease that stat anymore. */
	MSGTBL_STATS_CANT_DECREASE = 177,
	/** You cannot increase that stat anymore. */
	MSGTBL_STATS_CANT_INCREASE = 178,
	/** Guild level changed. */
	MSGTBL_GUILD_LEVEL_CHANGED = 179,
	/** The monster/egg name/ID doesn't exist. */
	MSGTBL_MAKEEGG_INVALID_EGG = 180,
	/** You already have a pet. */
	MSGTBL_ALREADY_HAVE_PET = 181,
	/** Pet intimacy changed. */
	MSGTBL_PET_INTIMACY_CHANGED = 182,
	/** Pet intimacy is already at maximum. */
	MSGTBL_PET_INTIMACY_MAXED = 183,
	/** Sorry, but you have no pet. */
	MSGTBL_NO_PET = 184,
	/** Pet hunger changed. */
	MSGTBL_PET_HUNGER_CHANGED = 185,
	/** Pet hunger is already at maximum. */
	MSGTBL_PET_HUNTER_MAXED = 186,
	/** You can now rename your pet. */
	MSGTBL_CAN_RENAME_PET = 187,
	/** You can already rename your pet. */
	MSGTBL_PET_CAN_BE_RENAMED_ALREADY = 188,
	/** Autofeed is disabled for this pet. */
	MSGTBL_AUTOFEED_DISABLED = 189,
	// 190-194 FREE
	/** All players have been kicked! */
	MSGTBL_ALL_PLAYERS_KICKED = 195,
	/** You already have this quest skill. */
	MSGTBL_ALREADY_HAVE_QUEST_SKILL = 196,
	/** This skill number doesn't exist or isn't a quest skill. */
	MSGTBL_INVALID_QUEST_SKILL_ID = 197,
	/** This skill number doesn't exist. */
	MSGTBL_INVALID_SKILL_ID = 198,
	// 199-200 FREE
	/** You don't have this quest skill. */
	MSGTBL_QUEST_SKILL_NOT_LEARNED = 201,
	// 202-203 FREE
	/** You can't open a shop on this cell. */
	MSGTBL_CANT_OPEN_SHOP_IN_CELL = 204,
	/** Maybe you meant:  */
	MSGTBL_MAYBE_YOU_MEANT = 205,
	/** '%s' skill points reset. */
	MSGTBL_PLAYER_SKILL_POINTS_RESET = 206,
	/** '%s' stats points reset. */
	MSGTBL_PLAYER_STATS_POINTS_RESET = 207,
	/** '%s' skill and stat points have been reset. */
	MSGTBL_PLAYER_SKILL_AND_STATS_RESET = 208,
	// 209-211 FREE
	/** Cannot mount while in disguise. */
	MSGTBL_CANT_MOUNT_IN_DISGUISE = 212,
	/** You need %s to mount! */
	MSGTBL_REQUIREMENT_FOR_MOUNT = 213,
	/** You have released your Peco Peco. */
	MSGTBL_PECO_RELEASED = 214,
	/** Your class can't mount! */
	MSGTBL_NO_MOUNT_FOR_CLASS = 215,
	// 216-218 FREE
	/** %d day */
	MSGTBL_DAY = 219,
	/** %d days */
	MSGTBL_DAYS = 220,
	/** %d hour */
	MSGTBL_HOUR = 221,
	/** %d hours */
	MSGTBL_HOURS = 222,
	/** %d minute */
	MSGTBL_MINUTE = 223,
	/** %d minutes */
	MSGTBL_MINUTES = 224,
	/**  and %d second */
	MSGTBL_SECOND = 225,
	/**  and %d seconds */
	MSGTBL_SENDS = 226,
	/** Party modification is disabled in this map. */
	MSGTBL_PARTY_CHANGE_DISABLED = 227,
	/** Guild modification is disabled in this map. */
	MSGTBL_GUILD_CHANGE_DISABLED = 228,
	/** Your effect has changed. */
	MSGTBL_EFFECT_CHANGED = 229,
	/** Server time (normal time): %A, %B %d %Y %X. */
	MSGTBL_SERVER_TIME = 230,
	/** Game time: The game is in permanent daylight. */
	MSGTBL_GAMETIME_ALWAYS_DAY = 231,
	/** Game time: The game is in permanent night. */
	MSGTBL_GAMETIME_ALWAYS_NIGHT = 232,
	/** Game time: The game is in night for %s. */
	MSGTBL_GAMETIME_NIGHT_TIME = 233,
	// 234 FREE
	/** Game time: The game is in daylight for %s. */
	MSGTBL_GAMETIME_DAY_TIME = 235,
	/** You've got a new mail! */
	MSGTBL_NEW_MAIL = 236,
	/** Game time: After, the game will be in night for %s. */
	MSGTBL_GAMETIME_FUTURE_NIGHT_TIME = 237,
	/** Game time: A day cycle has a normal duration of %s. */
	MSGTBL_GAMETIME_DAY_CYCLE = 238,
	/** Game time: After, the game will be in daylight for %s. */
	MSGTBL_GAMETIME_FUTURE_DAY_TIME = 239,
	/** %d monster(s) summoned! */
	MSGTBL_N_MONSTERS_SUMMONED = 240,
	/** You can now attack and kill players freely. */
	MSGTBL_KILLER_ON = 241,
	/** You can now be attacked and killed by players. */
	MSGTBL_KILLABLE_ON = 242,
	/** Skills have been disabled in this map. */
	MSGTBL_SKILLOFF = 243,
	/** Skills have been enabled in this map. */
	MSGTBL_SKILLON = 244,
	/** Server Uptime: %ld days, %ld hours, %ld minutes, %ld seconds. */
	MSGTBL_UPTIME = 245,
	/** Your GM level doesn't authorize you to perform this action. */
	MSGTBL_CANT_GIVE_ITEMS = 246,
	/** You are not authorized to warp to this map. */
	MSGTBL_CANT_WARP_TO = 247,
	/** You are not authorized to warp from your current map. */
	MSGTBL_CANT_WARP_FROM = 248,
	/** You are not authorized to warp to your save map. */
	MSGTBL_CANT_WARP_TO_SAVE_POINT = 249,
	/** You have already opened your storage. Close it first. */
	MSGTBL_STORAGE_ALREADY_OPEN = 250,
	/** You have already opened your guild storage. Close it first. */
	MSGTBL_GSTORAGE_ALREADY_OPEN = 251,
	/** You are not in a guild. */
	MSGTBL_NOT_IN_A_GUILD2 = 252,
	/** You already are at your destination! */
	MSGTBL_WARP_SAMEPLACE = 253,
	/** GM command configuration has been reloaded. */
	MSGTBL_RELOAD_ATCOMMAND_RELOADED = 254,
	/** Battle configuration has been reloaded. */
	MSGTBL_RELOAD_BATTLE_RELOADED = 255,
	/** Status database has been reloaded. */
	MSGTBL_RELOAD_STATUS_RELOADED = 256,
	/** Player database has been reloaded. */
	MSGTBL_RELOAD_PLAYER_RELOADED = 257,
	/** Sent packet 0x%x (%d) */
	MSGTBL_SEND_PACKET_SENT = 258,
	/** Invalid packet */
	MSGTBL_SEND_INVALID_PACKET = 259,
	/** This item cannot be traded. */
	MSGTBL_CANT_TRADE_ITEM = 260,
	/** Script could not be loaded. */
	MSGTBL_COULD_NOT_LOAD_SCRIPT = 261,
	/** Script loaded. */
	MSGTBL_SCRIPT_LOADED = 262,
	/** This item cannot be dropped. */
	MSGTBL_CANT_DROP_ITEM = 263,
	/** This item cannot be stored. */
	MSGTBL_CANT_STORE_ITEM = 264,
	/** %s has bought your item(s). */
	MSGTBL_NAME_BOUGHT_ITEM = 265,
	/** Some of your items cannot be vended and were removed from the shop. */
	MSGTBL_ITEMS_REMOVED_FROM_SHOP = 266,
	// 267-268 FREE
	/** Displaying first %d out of %d matches */
	MSGTBL_SEARCH_RESULT_OFFSET = 269,
	//@me output format
	/** * :%s %s: * */
	MSGTBL_ATCMD_ME_OUTPUT_FORMAT = 270,
	/** You can't drop items in this map */
	MSGTBL_CANT_DROP_ITEMS_IN_MAP = 271,
	/** You can't trade in this map */
	MSGTBL_CANT_TRADE_IN_MAP = 272,
	/** Available commands: */
	MSGTBL_AVAILABLE_COMMANDS = 273,
	/** %d commands found. */
	MSGTBL_N_COMMANDS_FOUND = 274,
	/** Custom commands: */
	MSGTBL_CUSTOM_COMMANDS = 275,
	/** You can't open a shop in this map */
	MSGTBL_CANT_OPEN_SHOP_IN_MAP = 276,

	/** Usage: @request <petition/message to online GMs>. */
	MSGTBL_REQUEST_USAGE = 277,
	/** (@request): %s */
	MSGTBL_REQUEST_MESSAGE = 278,
	/** @request sent. */
	MSGTBL_REQUEST_SENT = 279,

	/** Invalid name. */
	MSGTBL_BAD_HOMPET_NAME = 280,
	/** You can't create chat rooms in this map */
	MSGTBL_CANT_CREATE_CHAT_IN_MAP = 281,

	// Party-related
	/** You need to be a party leader to use this command. */
	MSGTBL_MUST_BE_PARTY_LEADER = 282,
	/** Target character must be online and in your current party. */
	MSGTBL_REQUIRE_ONLINE_PARTY_MEMBER = 283,
	/** Leadership transferred. */
	MSGTBL_LEADERSHIP_TRANSFERRED = 284,
	/** You've become the party leader. */
	MSGTBL_BECOME_PARTY_LEADER = 285,
	/** There's been no change in the setting. */
	MSGTBL_NO_CHANGE_IN_PARTY_SETTINGS = 286,
	/** You cannot change party leaders in this map. */
	MSGTBL_CANT_CHANGE_PARTY_LEADER_IN_MAP = 287,

	// Missing stuff for @killer related commands.
	/** You are no longer killable. */
	MSGTBL_KILLABLE_OFF = 288,
	// 289-290 FREE
	/** Weather effects will disappear after teleporting or refreshing. */
	MSGTBL_CLEARWEATHER = 291,
	/** Killer state reset. */
	MSGTBL_KILLER_OFF = 292,

	// Item Bind System
	/** This bound item cannot be traded to that character. */
	MSGTBL_BOUND_CANT_TRADE = 293,
	/** This bound item cannot be stored there. */
	MSGTBL_BOUND_CANT_STORE = 294,
	/** Please enter an item name or ID (usage: @itembound <item name/ID> <quantity> <bound_type>). */
	MSGTBL_ITEMBOUND_USAGE = 295,
	/** Please enter all parameters (usage: @itembound2 <item name/ID> <quantity> */
	MSGTBL_ITEMBOUND2_USAGE = 296,
	/**   <identify_flag> <refine> <attribute> <card1> <card2> <card3> <card4> <bound_type>). */
	MSGTBL_ITEMBOUND_USAGE2 = 297,
	/** Invalid bound type. Valid types are - 1:Account 2:Guild 3:Party 4:Character */
	MSGTBL_ITEMBOUND_INVALID_TYPE = 298,
	// 299 FREE

	// Guild Castles Number
	// --------------------
	/** None Taken */
	MSGTBL_CASTLES_0 = 300,
	/** One Castle */
	MSGTBL_CASTLES_1 = 301,
	/** Two Castles */
	MSGTBL_CASTLES_2 = 302,
	/** Three Castles */
	MSGTBL_CASTLES_3 = 303,
	/** Four Castles */
	MSGTBL_CASTLES_4 = 304,
	/** Five Castles */
	MSGTBL_CASTLES_5 = 305,
	/** Six Castles */
	MSGTBL_CASTLES_6 = 306,
	/** Seven Castles */
	MSGTBL_CASTLES_7 = 307,
	/** Eight Castles */
	MSGTBL_CASTLES_8 = 308,
	/** Nine Castles */
	MSGTBL_CASTLES_9 = 309,
	/** Ten Castles */
	MSGTBL_CASTLES_10 = 310,
	/** Eleven Castles */
	MSGTBL_CASTLES_11 = 311,
	/** Twelve Castles */
	MSGTBL_CASTLES_12 = 312,
	/** Thirteen Castles */
	MSGTBL_CASTLES_13 = 313,
	/** Fourteen Castles */
	MSGTBL_CASTLES_14 = 314,
	/** Fifteen Castles */
	MSGTBL_CASTLES_15 = 315,
	/** Sixteen Castles */
	MSGTBL_CASTLES_16 = 316,
	/** Seventeen Castles */
	MSGTBL_CASTLES_17 = 317,
	/** Eighteen Castles */
	MSGTBL_CASTLES_18 = 318,
	/** Nineteen Castles */
	MSGTBL_CASTLES_19 = 319,
	/** Twenty Castles */
	MSGTBL_CASTLES_20 = 320,
	/** Twenty-One Castles */
	MSGTBL_CASTLES_21 = 321,
	/** Twenty-Two Castles */
	MSGTBL_CASTLES_22 = 322,
	/** Twenty-Three Castles */
	MSGTBL_CASTLES_23 = 323,
	/** Twenty-Four Castles */
	MSGTBL_CASTLES_24 = 324,
	/** Twenty-Five Castles */
	MSGTBL_CASTLES_25 = 325,
	/** Twenty-Six Castles */
	MSGTBL_CASTLES_26 = 326,
	/** Twenty-Seven Castles */
	MSGTBL_CASTLES_27 = 327,
	/** Twenty-Eight Castles */
	MSGTBL_CASTLES_28 = 328,
	/** Twenty-Nine Castles */
	MSGTBL_CASTLES_29 = 329,
	/** Thirty Castles */
	MSGTBL_CASTLES_30 = 330,
	/** Thirty-One Castles */
	MSGTBL_CASTLES_31 = 331,
	/** Thirty-Two Castles */
	MSGTBL_CASTLES_32 = 332,
	/** Thirty-Three Castles */
	MSGTBL_CASTLES_33 = 333,
	// 334: Thirty-Four Castles
	/** Total Domination */
	MSGTBL_CASTLES_34 = 334,

	/** Your guild doesn't have storage! */
	MSGTBL_GUILD_DOES_NOT_HAVE_STORAGE = 335,
	/** You're not authorized to open your guild storage! */
	MSGTBL_NOT_AUTHORIZED_TO_USE_GSTORAGE = 336,
	// 337-342 FREE

	// Templates for @who output
	/** Name: %s  */
	MSGTBL_WHO_NAME_FORMAT = 343,
	/** (%s)  */
	MSGTBL_WHO_TITLE_FORMAT = 344,
	/** | Party: '%s'  */
	MSGTBL_WHO_PARTY_FORMAT = 345,
	/** | Guild: '%s' */
	MSGTBL_WHO_GUILD_FORMAT = 346,
	// You may omit the last %s, then you won't see players job name
	/** | Lv:%d/%d | Job: %s */
	MSGTBL_WHO_LEVEL_JOB_FORMAT = 347,
	// You may omit 2 last %d, then you won't see players coordinates, just map name
	/** | Location: %s %d %d */
	MSGTBL_WHO_LOCATION_FORMAT = 348,
	// 349 FREE

	// @duel
	/** Duel: You can't use @invite. You aren't a duelist. */
	MSGTBL_DUEL_CANT_INVITE = 350,
	/** Duel: The limit of players has been reached. */
	MSGTBL_DUEL_LIMIT_REACHED = 351,
	/** Duel: Player name not found. */
	MSGTBL_DUEL_PLAYER_NOT_FOUND = 352,
	/** Duel: The Player is in the duel already. */
	MSGTBL_DUEL_PLAYER_IN_DUEL = 353,
	/** Duel: Invitation has been sent. */
	MSGTBL_DUEL_INVITATION_SENT = 354,
	/** Duel: You can't use @duel without @reject. */
	MSGTBL_DUEL_NEEDS_REJECT_FIRST = 355,
	/** Duel: You can take part in duel again after %d seconds. */
	MSGTBL_DUEL_COOLDOWN = 356,
	/** Duel: Invalid value. */
	MSGTBL_DUEL_INVALID_VALUE = 357,
	/** Duel: You can't use @leave. You aren't a duelist. */
	MSGTBL_DUEL_CANT_LEAVE = 358,
	/** Duel: You've left the duel. */
	MSGTBL_DUEL_LEFT = 359,
	/** Duel: You can't use @accept without a duel invitation. */
	MSGTBL_DUEL_CANT_ACCEPT = 360,
	/** Duel: The duel invitation has been accepted. */
	MSGTBL_DUEL_INVITATION_ACCEPTED = 361,
	/** Duel: You can't use @reject without a duel invitation. */
	MSGTBL_DUEL_CANT_REJECT = 362,
	/** Duel: The duel invitation has been rejected. */
	MSGTBL_DUEL_INVITATION_REJECTED = 363,
	/** Duel: You can't invite %s because he/she isn't in the same map. */
	MSGTBL_DUEL_PLAYER_NOT_IN_MAP = 364,
	/** Duel: Can't use %s in duel. */
	MSGTBL_DUEL_CANT_USE = 365,

	// Stylist Shop
	/** Styling Shop */
	MSGTBL_STYLESHOP_MAIL_SENDER = 366,
	/** <MSG>2949</MSG> */
	MSGTBL_STYLESHOP_MAIL_TITLE = 367,
	/** <MSG>2950</MSG> */
	MSGTBL_STYLESHOP_MAIL_BODY = 368,
	// 369 FREE

	/**  -- Duels: %d/%d, Members: %d/%d, Max players: %d -- */
	MSGTBL_DUEL_INFO_LIMIT = 370,
	/**  -- Duels: %d/%d, Members: %d/%d -- */
	MSGTBL_DUEL_INFO_NO_LIMIT = 371,
	/**  -- Duel has been created (Use @invite/@leave) -- */
	MSGTBL_DUEL_CREATED = 372,
	/**  -- Player %s invites %s to duel -- */
	MSGTBL_DUEL_INVITE_INFO = 373,
	/** Blue -- Player %s invites you to PVP duel (Use @accept/@reject) -- */
	MSGTBL_DUEL_INVITE_MESSAGE = 374,
	/**  <- Player %s has left the duel -- */
	MSGTBL_DUEL_PLAYER_LEFT = 375,
	/**  -> Player %s has accepted the duel -- */
	MSGTBL_DUEL_PLAYER_ACCEPTED = 376,
	/**  -- Player %s has rejected the duel -- */
	MSGTBL_DUEL_PLAYER_REJECTED = 377,

	// 378-385 FREE

	// Main chat
	/** %s :Main: %s */
	MSGTBL_MAIN_CHAT = 386,

	// 387-389 FREE

	// NoAsk
	/** Autorejecting is activated. */
	MSGTBL_NOASK_ON = 390,
	/** Autorejecting is deactivated. */
	MSGTBL_NOASK_OFF = 391,
	/** You request has been rejected by autoreject option. */
	MSGTBL_NOASK_REJECTED = 392,
	/** Autorejected trade request from %s. */
	MSGTBL_NOASK_REJECTED_TRADE = 393,
	/** Autorejected party invite from %s. */
	MSGTBL_NOASK_REJECTED_PARTY_INVITE = 394,
	/** Autorejected guild invite from %s. */
	MSGTBL_NOASK_REJECTED_GUILD_INVITE = 395,
	/** Autorejected alliance request from %s. */
	MSGTBL_NOASK_REJECTED_ALLIANCE = 396,
	/** Autorejected opposition request from %s. */
	MSGTBL_NOASK_REJECTED_OPPOSITION = 397,
	/** Autorejected friend request from %s. */
	MSGTBL_NOASK_REJECTED_FRIEND = 398,

	// 399 FREE

	/** Usage: @jailfor <time> <character name> */
	MSGTBL_JAILFOR_USAGE = 400,
	// 401 FREE
	/** %s in jail for %d years, %d months, %d days, %d hours and %d minutes */
	MSGTBL_JAILFOR_TIME = 402,

	// WoE SE (@agitstart2)
	/** War of Emperium SE has been initiated. */
	MSGTBL_AGITSTART2_INITIATED = 403,
	/** War of Emperium SE is currently in progress. */
	MSGTBL_AGITSTART2_ALREADY_IN_PROGRESS = 404,
	/** War of Emperium SE has been ended. */
	MSGTBL_AGITEND2_ENDED = 405,
	/** War of Emperium SE is currently not in progress. */
	MSGTBL_AGITEND2_NOT_IN_PROGRESS = 406,

	// 407 FREE

	// chrif related
	/** Disconnecting to perform change-sex request... */
	MSGTBL_CHANGESEX_DISCONNECT = 408,
	/** Your sex has been changed (disconnection required to complete the process)... */
	MSGTBL_CHANGESEX_PERFORMED = 409,
	// 410-411 used by cash shop
	/** Your account is 'Unregistered'. */
	MSGTBL_CHRIF_ACCOUNT_UNREGISTERED = 412,
	/** Your account has an 'Incorrect Password'... */
	MSGTBL_CHRIF_INCORRECT_PASSWORD = 413,
	/** Your account has expired. */
	MSGTBL_CHRIF_ACCOUNT_EXPIRED = 414,
	/** Your account has been rejected from server. */
	MSGTBL_CHRIF_ACCOUNT_REJECTED = 415,
	/** Your account has been blocked by the GM Team. */
	MSGTBL_CHRIF_ACCOUNT_BLOCKED = 416,
	/** Your Game's EXE file is not the latest version. */
	MSGTBL_CHRIF_OUTDATED_EXE = 417,
	/** Your account has been prohibited to log in. */
	MSGTBL_CHRIF_RESTRICTED_ACCOUNT = 418,
	/** Server is jammed due to overpopulation. */
	MSGTBL_CHRIF_SERVER_JAMMED = 419,
	/** Your account is no longer authorized. */
	MSGTBL_CHRIF_ACCOUNT_UNAUTHORIZED = 420,
	/** Your account has been totally erased. */
	MSGTBL_CHRIF_ACCOUNT_ERASED = 421,
	// 422 FREE
	/** Your account has been banished until  */
	MSGTBL_CHRIF_ACCOUNT_BANNED = 423,
	/** Login-server has been asked to %s the player '%.*s'. */
	MSGTBL_CHRIF_LOGIN_REQUEST = 424,
	/** The player '%.*s' doesn't exist. */
	MSGTBL_CHRIF_PLAYER_NOT_FOUND = 425,
	/** Your GM level doesn't authorize you to %s the player '%.*s'. */
	MSGTBL_CHRIF_GM_UNAUTHORIZED = 426,
	/** Login-server is offline. Impossible to %s the player '%.*s'. */
	MSGTBL_CHRIF_LOGIN_SERVER_OFFLINE = 427,
	/** block */
	MSGTBL_CHRIF_ACTION_BLOCK = 428,
	/** ban */
	MSGTBL_CHRIF_ACTION_BAN = 429,
	/** unblock */
	MSGTBL_CHRIF_ACTION_UNBLOCK = 430,
	/** unban */
	MSGTBL_CHRIF_ACTION_UNBAN = 431,
	/** change the sex of */
	MSGTBL_CHRIF_ACTION_CHANGESEX = 432,
	/** This character has been banned until  */
	MSGTBL_CHRIF_CHAR_BANNED_UNTIL = 433,
	/** Char-server has been asked to %s the character '%.*s'. */
	MSGTBL_CHRIF_CHAR_REQUEST = 434,
	// 435-448 FREE

	// Homunculus messages
	/** Homunculus Experience Gained Base:%u (%.2f%%) */
	MSGTBL_HOMUN_EXP_GAINED = 449,
	/** You already have a homunculus. */
	MSGTBL_ALREADY_HAVE_HOMUN = 450,

	// Return pet to egg message
	/** You can't return your pet because your inventory is full. */
	MSGTBL_CANT_RETURN_PET_INVENTORY_FULL = 451,

	/** usage @camerainfo range rotation latitude */
	MSGTBL_CAMERAINFO_USAGE = 452,

	// Refinary
	/** Refinery UI is not available. */
	MSGTBL_REFINEUI_NOT_AVAILABLE = 453,

	// Battlegrounds
	/** Server : %s is leaving the battlefield... */
	MSGTBL_BG_PLAYER_LEFT = 454,
	/** Server : %s has been afk-kicked from the battlefield... */
	MSGTBL_BG_PLAYER_AFK_KICK = 455,
	/** You are a deserter! Wait %u minute(s) before you can apply again */
	MSGTBL_BG_QUEUE_PUNISHMENT_MINUTES = 456,
	/** You are a deserter! Wait %u seconds before you can apply again */
	MSGTBL_BG_QUEUE_PUNISHMENT_SECONDS = 457,
	/** You can't reapply to this arena so fast. Apply to the different arena or wait %u minute(s) */
	MSGTBL_BG_QUEUE_COOLDOWN_MINUTES = 458,
	/** You can't reapply to this arena so fast. Apply to the different arena or wait %u seconds */
	MSGTBL_BG_QUEUE_COOLDOWN_SECONDS = 459,
	/** Can't apply: not enough members in your team/guild that have not entered the queue in individual mode, minimum is %d
	 * Note: in this case, your guild does have enough members, but some of them are not available to join now
	 */
	MSGTBL_BG_QUEUE_AVAILABLE_GUILD_TOO_SMALL = 460,
	/** Can't apply: not enough members in your team/guild, minimum is %d
	 * Note: in this case, guild's online members count is smaller than needed
	 */
	MSGTBL_BG_QUEUE_GUILD_TOO_SMALL = 461,
	/** Can't apply: not enough members in your team/party that have not entered the queue in individual mode, minimum is %d
	 * Note: in this case, your party does have enough members, but some of them are not available to join now
	 */
	MSGTBL_BG_QUEUE_AVAILABLE_PARTY_TOO_SMALL = 462,
	/** Can't apply: not enough members in your team/party, minimum is %d
	 * Note: in this case, party's online members count is smaller than needed
	 */
	MSGTBL_BG_QUEUE_PARTY_TOO_SMALL = 463,
	/** Server : %s has quit the game... */
	MSGTBL_BG_PLAYER_QUIT = 464,

	// IRC
	/** [ #%s ] User IRC.%s left the channel. [Quit: %s] */
	MSGTBL_IRC_USER_QUIT = 465,
	/** [ #%s ] User IRC.%s left the channel. [%s] */
	MSGTBL_IRC_USER_LEFT = 466,
	/** [ #%s ] User IRC.%s is now known as IRC.%s */
	MSGTBL_IRC_USER_NICK = 467,
	/** [ #%s ] User IRC.%s joined the channel. */
	MSGTBL_IRC_USER_JOIN = 468,

	// Clan System
	/** Please enter a Clan ID (usage: @joinclan <clan ID>). */
	MSGTBL_JOINCLAN_USAGE = 469,
	/** You are already in a clan. */
	MSGTBL_JOINCLAN_ALREADY_IN_CLAN = 470,
	/** You must leave your guild before enter in a clan. */
	MSGTBL_JOINCLAN_MUST_LEAVE_GUILD = 471,
	/** Invalid Clan ID. */
	MSGTBL_JOINCLAN_INVALID_CLAN = 472,
	/** The clan couldn't be joined. */
	MSGTBL_JOINCLAN_FAILED = 473,
	/** You aren't in a clan. */
	MSGTBL_LEAVECLAN_NOT_IN_CLAN = 474,
	/** Failed on leaving clan. */
	MSGTBL_LEAVECLAN_FAILED = 475,
	/** Clan configuration and database have been reloaded. */
	MSGTBL_RELOAD_CLAN_RELOADED = 476,
	/** You cannot create a guild because you are in a clan. */
	MSGTBL_CANT_CREATE_GUILD_WHILE_IN_CLAN = 477,

	// 478-497 FREE

	// @itembound / @itembound2
	/** Cannot create bound pet eggs or pet armors. */
	MSGTBL_ITEMBOUND_CANT_CREATE_PET_ITEMS = 498,

	// MSGTBL_499 = 499, // FREE

	// 500 - 549: Messages of others (not for GM commands)
	// ----------------------------------------
	// MSGTBL_500 = 500, // FREE

	/** Your account time limit is: %d-%m-%Y %H:%M:%S. */
	MSGTBL_ACCOUNT_EXPIRE_TIME = 501,
	/** Day Mode is activated.
	 * This differs from MSGTBL_DAY_MODE_ACTIVATED as MSGTBL_DAY_MODE_ACTIVATED2 is used when "natural" change happens,
	 * while MSGTBL_DAY_MODE_ACTIVATED is triggered by GM command
	 */
	MSGTBL_DAY_MODE_ACTIVATED2 = 502,
	/** Night Mode is activated.
	 * This differs from MSGTBL_NIGHT_MODE_ACTIVATED as MSGTBL_NIGHT_MODE_ACTIVATED2 is used when "natural" change happens,
	 * while MSGTBL_NIGHT_MODE_ACTIVATED is triggered by GM command
	 */
	MSGTBL_NIGHT_MODE_ACTIVATED2 = 503,

	// Cash point change messages
	/** Used %d Kafra points and %d cash points. %d Kafra and %d cash points remaining. */
	MSGTBL_USED_KAFRA_CASH_POINTS = 504,
	/** Gained %d cash points. Total %d points. */
	MSGTBL_GAINED_CASHPOINTS = 505,
	/** Gained %d Kafra points. Total %d points. */
	MSGTBL_GAINED_KAFRAPOINTS = 506,
	/** Removed %d cash points. Total %d points. */
	MSGTBL_REMOVED_CASHPOINTS = 410,
	/** Removed %d Kafra points. Total %d points. */
	MSGTBL_REMOVED_KAFRAPOINTS = 411,

	// Trade Spoof Messages
	/** This player has been banned for %d minute(s). */
	MSGTBL_TRADE_SPOOF_BAN_ALERT = 507,
	/** This player hasn't been banned (Ban option is disabled). */
	MSGTBL_TRADE_SPOOF_NOT_BANNED = 508,
	// 509 FREE

	// mail system
	//----------------------
	/** You have %d new emails (%d unread) */
	MSGTBL_MAIL_COUNT = 510,
	/** Inbox is full (Max %d). Delete some mails. */
	MSGTBL_MAIL_INBOX_FULL = 511,
	/** You can't send mails from your current location. */
	MSGTBL_CANT_SEND_MAILS_IN_MAP = 512,
	// 513-537 FREE

	// Trade Spoof Messages
	/** Hack on trade: character '%s' (account: %d) try to trade more items that he has. */
	MSGTBL_TRADE_SPOOF_TOO_MANY_ITEMS = 538,
	/** This player has %d of a kind of item (id: %d), and tried to trade %d of them. */
	MSGTBL_TRADE_SPOOF_TOO_MANY_ITEMS2 = 539,
	/** This player has been definitively blocked. */
	MSGTBL_TRADE_SPOOF_PERMANENT_BAN = 540,
	// 541-543 FREE

	/** <MSG>3455</MSG> */
	MSGTBL_ATTENDANCE_MAIL_SENDER = 544,
	/** <MSG>3456,%d</MSG> */
	MSGTBL_ATTENDANCE_MAIL_TITLE = 545,

	// @showmobs
	/** Please enter a mob name/id (usage: @showmobs <mob name/id>) */
	MSGTBL_SHOWMOBS_USAGE = 546,
	/** Invalid mob name %s! */
	MSGTBL_SHOMOBS_INVALID_NAME = 547,

	// @clearcart
	/** You can't clean a cart while vending! */
	MSGTBL_CLEARCART_FAIL_VENDING = 548,

	// @autotrade
	/** You should have a shop open in order to use @autotrade. */
	MSGTBL_AUTOTRADE_MISSING_SHOP = 549,

	// 550 - 699: Job names (also loaded by char-server)
	// ----------------------------------------
	/** Novice */
	MSGTBL_JOB_NOVICE = 550,
	/** Swordsman */
	MSGTBL_JOB_SWORDMAN = 551,
	/** Magician */
	MSGTBL_JOB_MAGE = 552,
	/** Archer */
	MSGTBL_JOB_ARCHER = 553,
	/** Acolyte */
	MSGTBL_JOB_ACOLYTE = 554,
	/** Merchant */
	MSGTBL_JOB_MERCHANT = 555,
	/** Thief */
	MSGTBL_JOB_THIEF = 556,
	/** Knight */
	MSGTBL_JOB_KNIGHT = 557,
	/** Priest */
	MSGTBL_JOB_PRIEST = 558,
	/** Wizard */
	MSGTBL_JOB_WIZARD = 559,
	/** Blacksmith */
	MSGTBL_JOB_BLACKSMITH = 560,
	/** Hunter */
	MSGTBL_JOB_HUNTER = 561,
	/** Assassin */
	MSGTBL_JOB_ASSASSIN = 562,
	/** Crusader */
	MSGTBL_JOB_CRUSADER = 563,
	/** Monk */
	MSGTBL_JOB_MONK = 564,
	/** Sage */
	MSGTBL_JOB_SAGE = 565,
	/** Rogue */
	MSGTBL_JOB_ROGUE = 566,
	/** Alchemist */
	MSGTBL_JOB_ALCHEMIST = 567,
	/** Bard */
	MSGTBL_JOB_BARD = 568,
	/** Dancer */
	MSGTBL_JOB_DANCER = 569,
	/** Wedding */
	MSGTBL_JOB_WEDDING = 570,
	/** Super Novice */
	MSGTBL_JOB_SUPER_NOVICE = 571,
	/** Gunslinger */
	MSGTBL_JOB_GUNSLINGER = 572,
	/** Ninja */
	MSGTBL_JOB_NINJA = 573,
	/** Christmas */
	MSGTBL_JOB_XMAS = 574,
	/** High Novice */
	MSGTBL_JOB_NOVICE_HIGH = 575,
	/** High Swordsman */
	MSGTBL_JOB_SWORDMAN_HIGH = 576,
	/** High Magician */
	MSGTBL_JOB_MAGE_HIGH = 577,
	/** High Archer */
	MSGTBL_JOB_ARCHER_HIGH = 578,
	/** High Acolyte */
	MSGTBL_JOB_ACOLYTE_HIGH = 579,
	/** High Merchant */
	MSGTBL_JOB_MERCHANT_HIGH = 580,
	/** High Thief */
	MSGTBL_JOB_THIEF_HIGH = 581,
	/** Lord Knight */
	MSGTBL_JOB_LORD_KNIGHT = 582,
	/** High Priest */
	MSGTBL_JOB_HIGH_PRIEST = 583,
	/** High Wizard */
	MSGTBL_JOB_HIGH_WIZARD = 584,
	/** Whitesmith */
	MSGTBL_JOB_WHITESMITH = 585,
	/** Sniper */
	MSGTBL_JOB_SNIPER = 586,
	/** Assassin Cross */
	MSGTBL_JOB_ASSASSIN_CROSS = 587,
	/** Paladin */
	MSGTBL_JOB_PALADIN = 588,
	/** Champion */
	MSGTBL_JOB_CHAMPION = 589,
	/** Professor */
	MSGTBL_JOB_PROFESSOR = 590,
	/** Stalker */
	MSGTBL_JOB_STALKER = 591,
	/** Creator */
	MSGTBL_JOB_CREATOR = 592,
	/** Clown */
	MSGTBL_JOB_CLOWN = 593,
	/** Gypsy */
	MSGTBL_JOB_GYPSY = 594,
	/** Baby Novice */
	MSGTBL_JOB_BABY = 595,
	/** Baby Swordsman */
	MSGTBL_JOB_BABY_SWORDMAN = 596,
	/** Baby Magician */
	MSGTBL_JOB_BABY_MAGE = 597,
	/** Baby Archer */
	MSGTBL_JOB_BABY_ARCHER = 598,
	/** Baby Acolyte */
	MSGTBL_JOB_BABY_ACOLYTE = 599,
	/** Baby Merchant */
	MSGTBL_JOB_BABY_MERCHANT = 600,
	/** Baby Thief */
	MSGTBL_JOB_BABY_THIEF = 601,
	/** Baby Knight */
	MSGTBL_JOB_BABY_KNIGHT = 602,
	/** Baby Priest */
	MSGTBL_JOB_BABY_PRIEST = 603,
	/** Baby Wizard */
	MSGTBL_JOB_BABY_WIZARD = 604,
	/** Baby Blacksmith */
	MSGTBL_JOB_BABY_BLACKSMITH = 605,
	/** Baby Hunter */
	MSGTBL_JOB_BABY_HUNTER = 606,
	/** Baby Assassin */
	MSGTBL_JOB_BABY_ASSASSIN = 607,
	/** Baby Crusader */
	MSGTBL_JOB_BABY_CRUSADER = 608,
	/** Baby Monk */
	MSGTBL_JOB_BABY_MONK = 609,
	/** Baby Sage */
	MSGTBL_JOB_BABY_SAGE = 610,
	/** Baby Rogue */
	MSGTBL_JOB_BABY_ROGUE = 611,
	/** Baby Alchemist */
	MSGTBL_JOB_BABY_ALCHEMIST = 612,
	/** Baby Bard */
	MSGTBL_JOB_BABY_BARD = 613,
	/** Baby Dancer */
	MSGTBL_JOB_BABY_DANCER = 614,
	/** Super Baby */
	MSGTBL_JOB_SUPER_BABY = 615,
	/** Taekwon */
	MSGTBL_JOB_TAEKWON = 616,
	/** Star Gladiator */
	MSGTBL_JOB_STAR_GLADIATOR = 617,
	/** Soul Linker */
	MSGTBL_JOB_SOUL_LINKER = 618,
	// MSGTBL_JOB_619 = 619, // FREE
	/** Unknown Job */
	MSGTBL_UNKNOWN_JOB = 620,
	/** Summer */
	MSGTBL_JOB_SUMMER = 621,
	/** Gangsi */
	MSGTBL_JOB_GANGSI = 622,
	/** Death Knight */
	MSGTBL_JOB_DEATH_KNIGHT = 623,
	/** Dark Collector */
	MSGTBL_JOB_DARK_COLLECTOR = 624,
	/** Rune Knight */
	MSGTBL_JOB_RUNE_KNIGHT = 625,
	/** Warlock */
	MSGTBL_JOB_WARLOCK = 626,
	/** Ranger */
	MSGTBL_JOB_RANGER = 627,
	/** Arch Bishop */
	MSGTBL_JOB_ARCH_BISHOP = 628,
	/** Mechanic */
	MSGTBL_JOB_MECHANIC = 629,
	/** Guillotine Cross */
	MSGTBL_JOB_GUILLOTINE_CROSS = 630,
	/** Royal Guard */
	MSGTBL_JOB_ROYAL_GUARD = 631,
	/** Sorcerer */
	MSGTBL_JOB_SORCERER = 632,
	/** Minstrel */
	MSGTBL_JOB_MINSTREL = 633,
	/** Wanderer */
	MSGTBL_JOB_WANDERER = 634,
	/** Sura */
	MSGTBL_JOB_SURA = 635,
	/** Genetic */
	MSGTBL_JOB_GENETIC = 636,
	/** Shadow Chaser */
	MSGTBL_JOB_SHADOW_CHASER = 637,
	/** Baby Rune Knight */
	MSGTBL_JOB_BABY_RUNE = 638,
	/** Baby Warlock */
	MSGTBL_JOB_BABY_WARLOCK = 639,
	/** Baby Ranger */
	MSGTBL_JOB_BABY_RANGER = 640,
	/** Baby Arch Bishop */
	MSGTBL_JOB_BABY_BISHOP = 641,
	/** Baby Mechanic */
	MSGTBL_JOB_BABY_MECHANIC = 642,
	/** Baby Guillotine Cross */
	MSGTBL_JOB_BABY_CROSS = 643,
	/** Baby Royal Guard */
	MSGTBL_JOB_BABY_GUARD = 644,
	/** Baby Sorcerer */
	MSGTBL_JOB_BABY_SORCERER = 645,
	/** Baby Minstrel */
	MSGTBL_JOB_BABY_MINSTREL = 646,
	/** Baby Wanderer */
	MSGTBL_JOB_BABY_WANDERER = 647,
	/** Baby Sura */
	MSGTBL_JOB_BABY_SURA = 648,
	/** Baby Genetic */
	MSGTBL_JOB_BABY_GENETIC = 649,
	/** Baby Shadow Chaser */
	MSGTBL_JOB_BABY_CHASER = 650,
	/** Expanded Super Novice */
	MSGTBL_JOB_SUPER_NOVICE_E = 651,
	/** Expanded Super Baby */
	MSGTBL_JOB_SUPER_BABY_E = 652,
	/** Kagerou */
	MSGTBL_JOB_KAGEROU = 653,
	/** Oboro */
	MSGTBL_JOB_OBORO = 654,
	/** Rebellion */
	MSGTBL_JOB_REBELLION = 655,
	/** Rune Knight T */
	MSGTBL_JOB_RUNE_KNIGHT_T = 656,
	/** Warlock T */
	MSGTBL_JOB_WARLOCK_T = 657,
	/** Ranger T */
	MSGTBL_JOB_RANGER_T = 658,
	/** Arch Bishop T */
	MSGTBL_JOB_ARCH_BISHOP_T = 659,
	/** Mechanic T */
	MSGTBL_JOB_MECHANIC_T = 660,
	/** Guillotine Cross T */
	MSGTBL_JOB_GUILLOTINE_CROSS_T = 661,
	/** Royal Guard T */
	MSGTBL_JOB_ROYAL_GUARD_T = 662,
	/** Sorcerer T */
	MSGTBL_JOB_SORCERER_T = 663,
	/** Minstrel T */
	MSGTBL_JOB_MINSTREL_T = 664,
	/** Wanderer T */
	MSGTBL_JOB_WANDERER_T = 665,
	/** Sura T */
	MSGTBL_JOB_SURA_T = 666,
	/** Genetic T */
	MSGTBL_JOB_GENETIC_T = 667,
	/** Shadow Chaser T */
	MSGTBL_JOB_SHADOW_CHASER_T = 668,
	/** Summoner */
	MSGTBL_JOB_SUMMONER = 669,
	/** Baby Summoner */
	MSGTBL_JOB_BABY_SUMMONER = 670,
	/** Baby Ninja */
	MSGTBL_JOB_BABY_NINJA = 671,
	/** Baby Kagerou */
	MSGTBL_JOB_BABY_KAGEROU = 672,
	/** Baby Oboro */
	MSGTBL_JOB_BABY_OBORO = 673,
	/** Baby Taekwon */
	MSGTBL_JOB_BABY_TAEKWON = 674,
	/** Baby Star Gladiator */
	MSGTBL_JOB_BABY_STAR_GLADIATOR = 675,
	/** Baby Soul Linker */
	MSGTBL_JOB_BABY_SOUL_LINKER = 676,
	/** Baby Gunslinger */
	MSGTBL_JOB_BABY_GUNSLINGER = 677,
	/** Baby Rebellion */
	MSGTBL_JOB_BABY_REBELLION = 678,
	/** Star Emperor */
	MSGTBL_JOB_STAR_EMPEROR = 679,
	/** Baby Star Emperor */
	MSGTBL_JOB_BABY_STAR_EMPEROR = 680,
	/** Soul Reaper */
	MSGTBL_JOB_SOUL_REAPER = 681,
	/** Baby Soul Reaper */
	MSGTBL_JOB_BABY_SOUL_REAPER = 682,

	// 683-849 FREE (please start using from the top if you need, leave the 670+ range for new jobs)

	/** No Knockback | */
	MSGTBL_MAPINFO_NO_KNOCKBACK = 850,
	/** Source For Instance | */
	MSGTBL_MAPINFO_SOURCE_FOR_INSTANCE = 851,
	/** No Map Channel Auto Join | */
	MSGTBL_MAPINFO_NO_CHANNEL_AUTOJOIN = 852,
	/** NoPet | */
	MSGTBL_MAPINFO_NO_PET = 853,

	// Mapflag to disable Autoloot Commands
	/** Auto loot item are disabled on this map. */
	MSGTBL_AUTOLOOT_DISABLED_IN_MAP = 854,

	// MVP exp message issue clients 2013-12-23cRagexe and newer.
	/** Congratulations! You are the MVP! Your reward EXP Points are %u !! */
	MSGTBL_MVP_REWARD_EXP = 855,

	// MVP Tomb
	/** Tomb */
	MSGTBL_TOMB = 856,
	/** [ ^EE0000%s^000000 ] */
	MSGTBL_TOMB_MONSTER_NAME = 857,
	/** Has met its demise */
	MSGTBL_TOMB_MET_ITS_DEMISE = 858,
	/** Time of death : ^EE0000%s^000000 */
	MSGTBL_TOMB_TIME_OF_DEATH = 859,
	/** Defeated by */
	MSGTBL_TOMB_DEFATED_BY = 860,
	/** [^EE0000%s^000000] */
	MSGTBL_TOMB_PLAYER_NAME = 861,

	// Etc messages from source
	/** You're too close to an NPC, you must be at least %d cells away from any NPC. */
	MSGTBL_TOO_CLOSE_NPC = 862,
	/** Duel: Can't use this item in duel. */
	MSGTBL_ITEM_LOCKED_DUEL = 863,
	/** You cannot use this command when dead. */
	MSGTBL_CANNOT_USE_WHEN_DEAD = 864,
	/** Can't create chat rooms in this area. */
	MSGTBL_CANNOT_CREATE_CHAT = 865,
	/** Pets are disabled in this map. */
	MSGTBL_PETS_DISABLED_IN_MAP = 866,
	/** You're not dead. */
	MSGTBL_NOT_DEAD = 867,
	/** Your current memo positions are: */
	MSGTBL_CURRENT_MEMO_POSITION = 868,
	/** You broke the target's weapon. */
	MSGTBL_BROKEN_TARGET_WEAPON = 869,
	/** You can't leave battleground guilds. */
	MSGTBL_CANNOT_LEAVE_BG_GUILD = 870,
	/** Friend already exists. */
	MSGTBL_FRIEND_ALREADY_EXISTS = 871,
	/** Name not found in list. */
	MSGTBL_NAME_NOT_FOUND = 872,
	/** This action can't be performed at the moment. Please try again later. */
	MSGTBL_ACTION_CANNOT_BE_PERFORMED = 873,
	/** Friend removed. */
	MSGTBL_FRIEND_REMOVED = 874,
	/** Cannot send mails too fast!! */
	MSGTBL_NOT_SEND_MAILS_TOO_FAST = 875,
	/** Alliances cannot be made during Guild Wars! */
	MSGTBL_NOT_MAKE_ALLIANCE_DURING_GW = 876,
	/** Alliances cannot be broken during Guild Wars! */
	MSGTBL_NOT_BREAK_ALLIANCE_DURING_GW = 877,
	/** You are no longer the Guild Master. */
	MSGTBL_NOT_GUILD_MASTER = 878,
	/** You have become the Guild Master! */
	MSGTBL_BECOME_GUILD_MASTER = 879,
	/** You have been recovered! */
	MSGTBL_PLAYER_RECOVERED = 880,
	/** Shop is out of stock! Come again later! */
	MSGTBL_SHOP_OUT_STOCK = 881,

	// Frost Joker / Scream text for monster (MobName : SkillName !!)
	/** %s : %s !! */
	MSGTBL_SKILL_FROST_JOKER = 882,
	// Cursed Circle
	/** You are too close to a stone or emperium to do this skill */
	MSGTBL_TOO_CLOSE_TO_STONE = 883,
	//
	/** Skill Failed. [%s] requires %dx %s. */
	MSGTBL_SKILL_FAILED_REQUIREMENTS = 884,
	/** Removed %dz. */
	MSGTBL_REMOVED_AMOUNT = 885,
	/** Gained %dz. */
	MSGTBL_GAINED_AMOUNT = 886,
	/** %s stole an Unknown Item (id: %i). */
	MSGTBL_STOLE_UNKNOWN_ITEM = 887,
	/** %s stole %s. */
	MSGTBL_STOLE_ITEM = 888,
	/** Experience Gained Base:%llu (%.2f%%) Job:%llu (%.2f%%) */
	MSGTBL_EXPERIENCE_GAINED = 889,
	/** [KS Warning!! - Owner : %s] */
	MSGTBL_KS_WARNING_OWNER = 890,
	/** [Watch out! %s is trying to KS you!] */
	MSGTBL_KS_WARNING_PLAYER = 891,
	/** Growth: hp:%d sp:%d str(%.2f) agi(%.2f) vit(%.2f) int(%.2f) dex(%.2f) luk(%.2f) */
	MSGTBL_GROWTH_STATS = 892,
	/** [ Kill Steal Protection Disabled. KS is allowed in this map ] */
	MSGTBL_KS_PROTECTION_DISABLED = 893,
	/** %s is in autotrade mode and cannot receive whispered messages. */
	MSGTBL_AUTOTRADE_MODE = 894,
	// 895 FREE
	/** Base EXP : %d%% | Base Drop: %d%% | Base Death Penalty: %d%% */
	MSGTBL_BASE_EXP_DROP_PENALTY = 896,
	/** #%s '%s' joined */
	MSGTBL_PLAYER_JOINED = 897,
	/** #%s '%s' left */
	MSGTBL_PLAYER_LEFT = 898,
	// 899 FREE

	//------------------------------------
	// More atcommand messages
	//------------------------------------

	// @send
	/** Usage: */
	MSGTBL_SEND_USAGE = 900,
	/**   @send len <packet hex number> */
	MSGTBL_SEND_LEN = 901,
	/**   @send <packet hex number> {<value>}* */
	MSGTBL_SEND_PACKET = 902,
	/**   Value: <type=B(default),W,L><number> or S<length>"<string>" */
	MSGTBL_SEND_VALUE_TYPE = 903,
	/** Packet 0x%x length: %d */
	MSGTBL_PACKET_LENGTH = 904,
	/** Unknown packet: 0x%x */
	MSGTBL_UNKNOWN_PACKET = 905,
	/** Not a string: */
	MSGTBL_NOT_A_STRING = 906,
	/** Not a hexadecimal digit: */
	MSGTBL_NOT_HEX_DIGIT = 907,
	/** Unknown type of value in: */
	MSGTBL_UNKNOWN_VALUE_TYPE = 908,

	// @rura
	/** Please enter a map (usage: @warp/@rura/@mapmove <mapname> <x> <y>). */
	MSGTBL_ENTER_MAP_NAME = 909,

	// @where
	/** Please enter a player name (usage: @where <char name>). */
	MSGTBL_ENTER_PLAYER_NAME = 910,

	// @jumpto
	/** Please enter a player name (usage: @jumpto/@warpto/@goto <char name/ID>). */
	MSGTBL_ENTER_PLAYER_NAME_OR_ID = 911,

	// @who
	/** (CID:%d/AID:%d)  */
	MSGTBL_WHO_PLAYER_INFO = 912,

	// @whogm
	/** Name: %s (GM) */
	MSGTBL_WHOGM_NAME_GM = 913,
	/** Name: %s (GM:%d) | Location: %s %d %d */
	MSGTBL_WHOGM_NAME_LOCATION = 914,
	/**       BLvl: %d | Job: %s (Lvl: %d) */
	MSGTBL_WHOGM_BLEVEL_JLEVEL = 915,
	/**       Party: '%s' | Guild: '%s' */
	MSGTBL_WHOGM_PARTY_GUILD = 916,
	/** None */
	MSGTBL_WHOGM_NONE = 917,

	// @speed
	/** Please enter a speed value (usage: @speed <%d-%d>). */
	MSGTBL_ENTER_SPEED_VALUE = 918,

	// @storage
	/** Storage opened. */
	MSGTBL_STORAGE_OPENED = 919,

	// @guildstorage
	/** Guild storage opened. */
	MSGTBL_GUILD_STORAGE_OPENED = 920,

	// @option
	/** Please enter at least one option. */
	MSGTBL_ENTER_LEAST_ONE_OPTION = 921,

	// 922 FREE

	// @jobchange
	/** You can not change to this job by command. */
	MSGTBL_CANNOT_CHANGE_JOB_COMMAND = 923,

	// atcommand_setzone
	/** Usage: @setzone <zone name> */
	MSGTBL_SETZONE_USAGE = 924,
	/** The zone is already set to '%s'. */
	MSGTBL_SETZONE_ALREADY_SET = 925,
	/** Zone not found. Keep in mind that the names are CaSe SenSitiVe. */
	MSGTBL_SETZONE_NOT_FOUND = 926,
	/** Zone successfully changed from '%s' to '%s'. */
	MSGTBL_SETZONE_SUCCESS_CHANGED = 927,

	// 928-978 FREE

	// @hatereset
	/** Reset 'Hatred' targets. */
	MSGTBL_RESET_HATRED_TARGETS = 979,

	// @kami
	/** Please enter a message (usage: @kami <message>). */
	MSGTBL_KAMI_ENTER_MSG = 980,
	/** Please enter color and message (usage: @kamic <color> <message>). */
	MSGTBL_KAMI_ENTER_COLOR_MSG = 981,
	/** Invalid color. */
	MSGTBL_KAMI_INVALID_COLOR = 982,

	// @item
	/** Please enter an item name or ID (usage: @item <item name/ID> <quantity>). */
	MSGTBL_ITEM_ENTER_NAME_OR_ID = 983,

	// @item2
	/** Please enter all parameters (usage: @item2 <item name/ID> <quantity>). */
	MSGTBL_ITEM2_ENTER_ALL_PARAM = 984,
	/**   <identify_flag> <refine> <attribute> <card1> <card2> <card3> <card4>). */
	MSGTBL_ITEM2_PARAMETERS = 985,

	// @baselevelup
	/** Please enter a level adjustment (usage: @lvup/@blevel/@baselvlup <number of levels>). */
	MSGTBL_ENTER_LV_ADJUSTMENT = 986,

	// @joblevelup
	/** Please enter a level adjustment (usage: @joblvup/@jlevel/@joblvlup <number of levels>). */
	MSGTBL_ENTER_JLV_ADJUSTMENT = 987,

	// @help
	/** There is no help for %c%s. */
	MSGTBL_NO_HELP_FOR = 988,
	/** Help for command %c%s: */
	MSGTBL_HELP_FOR_COMMAND = 989,
	/** Available aliases: */
	MSGTBL_HELP_AVAILABLE_ALIASES = 990,

	// @model
	/** Please enter at least one value (usage: @model <hair ID: %d-%d> <hair color: %d-%d> <clothes color: %d-%d>). */
	MSGTBL_MODEL_ENTER_VALUES = 991,

	// @dye
	/** Please enter a clothes color (usage: @dye/@ccolor <clothes color: %d-%d>). */
	MSGTBL_ENTER_CLOTHES_COLOR = 992,

	// @hairstyle
	/** Please enter a hair style (usage: @hairstyle/@hstyle <hair ID: %d-%d>). */
	MSGTBL_ENTER_HAIRSTYLE = 993,

	// @haircolor
	/** Please enter a hair color (usage: @haircolor/@hcolor <hair color: %d-%d>). */
	MSGTBL_ENTER_HAIR_COLOR = 994,

	// 995 FREE

	// @refine - Part 1
	/** Please enter a position bitmask and an amount (usage: @refine <equip position> <+/- amount>). */
	MSGTBL_REFINE_ENTER_POS_AMOUNT = 996,
	/** %d: Headgear (Low) */
	MSGTBL_REFINE_HEAD_LOW = 997,
	/** %d: Hand (Right) */
	MSGTBL_REFINE_HAND_RIGHT = 998,
	/** %d: Garment */
	MSGTBL_REFINE_GARMENT = 999,
	/** %d: Accessory (Left) */
	MSGTBL_REFINE_ACC_LEFT = 1000,
	/** %d: Body Armor */
	MSGTBL_REFINE_BODY_ARMOR = 1001,
	/** %d: Hand (Left) */
	MSGTBL_REFINE_HAND_LEFT = 1002,
	/** %d: Shoes */
	MSGTBL_REFINE_SHOES = 1003,
	/** %d: Accessory (Right) */
	MSGTBL_REFINE_ACC_RIGHT = 1004,
	/** %d: Headgear (Top) */
	MSGTBL_REFINE_HEAD_TOP = 1005,
	/** %d: Headgear (Mid) */
	MSGTBL_REFINE_HEAD_MID = 1006,

	// @produce
	/** Please enter at least one item name/ID (usage: @produce <equip name/ID> <element> <# of very's>). */
	MSGTBL_ENTER_PRODUCE_ITEM = 1007,

	// @memo
	/** Please enter a valid position (usage: @memo <memo_position:%d-%d>). */
	MSGTBL_ENTER_MEMO_POSITION = 1008,

	// @displaystatus
	/** Please enter a status type/flag (usage: @displaystatus <status type> <flag> <tick> {<val1> {<val2> {<val3>}}}). */
	MSGTBL_ENTER_DISPLAY_STATUS = 1009,

	// @stpoint
	/** Please enter a number (usage: @stpoint <number of points>). */
	MSGTBL_ENTER_STAT_POINT = 1010,

	// @skpoint
	/** Please enter a number (usage: @skpoint <number of points>). */
	MSGTBL_ENTER_SKILL_POINT = 1011,

	// @zeny
	/** Please enter an amount (usage: @zeny <amount>). */
	MSGTBL_ENTER_ZENY_AMOUNT = 1012,

	// @param
	/** Please enter a valid value (usage: @str/@agi/@vit/@int/@dex/@luk <+/-adjustment>). */
	MSGTBL_ENTER_PARAM_ADJUSTMENT = 1013,

	// @guildlevelup
	/** Please enter a valid level (usage: @guildlvup/@guildlvlup <# of levels>). */
	MSGTBL_ENTER_GUILD_LEVEL = 1014,

	// @makeeg
	/** Please enter a monster/egg name/ID (usage: @makeegg <pet>). */
	MSGTBL_ENTER_MAKE_EGG = 1015,

	// @petfriendly
	/** Please enter a valid value (usage: @petfriendly <0-1000>). */
	MSGTBL_ENTER_PET_FRIENDLY_VALUE = 1016,

	// @pethungry
	/** Please enter a valid number (usage: @pethungry <0-100>). */
	MSGTBL_ENTER_PET_HUNGRY_VALUE = 1017,

	// @recall
	/** Please enter a player name (usage: @recall <char name/ID>). */
	MSGTBL_ENTER_RECALL_PLAYER_NAME = 1018,
	/** You are not authorized to warp someone to this map. */
	MSGTBL_NOT_AUTHORIZED_WARP_TO_MAP = 1019,

	// @recall
	/** You are not authorized to warp this player from their map. */
	MSGTBL_NOT_AUTHORIZED_WARP_FROM_MAP = 1020,

	// @block
	/** Please enter a player name (usage: @block <char name>). */
	MSGTBL_ENTER_BLOCK_PLAYER_NAME = 1021,

	// @ban
	/** Please enter ban time and a player name (usage: @ban <time> <char name>). */
	MSGTBL_ENTER_BAN_TIME_PLAYER_NAME = 1022,
	/** You are not allowed to reduce the length of a ban. */
	MSGTBL_NOT_REDUCE_BAN_LENGTH = 1023,

	// @unblock
	/** Please enter a player name (usage: @unblock <char name>). */
	MSGTBL_ENTER_UNBLOCK_PLAYER_NAME = 1024,

	// @unban
	/** Please enter a player name (usage: @unban <char name>). */
	MSGTBL_ENTER_UNBAN_PLAYER_NAME = 1025,

	// @kick
	/** Please enter a player name (usage: @kick <char name/ID>). */
	MSGTBL_ENTER_KICK_PLAYER_NAME = 1026,

	// @questskill / @lostskill
	/** Please enter a quest skill ID. */
	MSGTBL_ENTER_QUEST_SKILL_ID = 1027,

	// @spiritball
	/** Please enter an amount (usage: @spiritball <number: 0-%d>). */
	MSGTBL_ENTER_SPIRITBALL_AMOUNT = 1028,

	// @party
	/** Please enter a party name (usage: @party <party_name>). */
	MSGTBL_ENTER_PARTY_NAME = 1029,

	// @guild
	/** Please enter a guild name (usage: @guild <guild_name>). */
	MSGTBL_ENTER_GUILD_NAME = 1030,

	// @idsearch
	/** Please enter part of an item name (usage: @idsearch <part_of_item_name>). */
	MSGTBL_ENTER_IDSEARCH_PART = 1031,

	// @recallall / @guildrecall / @partyrecall
	/** You are not authorized to warp someone to your current map. */
	MSGTBL_NOT_AUTHORIZED_WARP_TO = 1032,
	/** Because you are not authorized to warp from some maps, %d player(s) have not been recalled. */
	MSGTBL_NOT_AUTHORIZED_WARP_FROM = 1033,

	// @guildrecall
	/** Please enter a guild name/ID (usage: @guildrecall <guild_name/ID>). */
	MSGTBL_ENTER_GUILDRECALL_NAME_ID = 1034,

	// @partyrecall
	/** Please enter a party name/ID (usage: @partyrecall <party_name/ID>). */
	MSGTBL_ENTER_PARTYRECALL_NAME_ID = 1035,

	// @reloadatcommand
	/** Error reading groups.conf, reload failed. */
	MSGTBL_ERROR_READ_GROUPS_CONF = 1036,
	/** Error reading atcommand.conf, reload failed. */
	MSGTBL_ERROR_READ_ATCOMMAND_CONF = 1037,

	// @mapinfo
	/** Please enter at least one valid list number (usage: @mapinfo <0-3> <map>). */
	MSGTBL_ENTER_VALID_LIST_NUMBER = 1038,
	/** ------ Map Info ------ */
	MSGTBL_MAP_INFO_SEPARATOR = 1039,
	/** Map: %s (Zone:%s) | Players: %d | NPCs: %d | Chats: %d | Vendings: %d */
	MSGTBL_MAP_INFO = 1040,
	/** ------ Map Flags ------ */
	MSGTBL_MAP_FLAGS = 1041,
	/** Town Map */
	MSGTBL_MAPINFO_TOWN_MAP = 1042,
	/** Autotrade Enabled */
	MSGTBL_MAPINFO_AUTOTRADE_ENABLED = 1043,
	/** Autotrade Disabled */
	MSGTBL_MAPINFO_AUTOTRADE_DISABLED = 1044,
	/** Battlegrounds ON */
	MSGTBL_MAPINFO_BATTLEGROUND_ON = 1045,
	/** PvP Flags: */
	MSGTBL_MAPINFO_PVP_FLAGS = 1046,
	/** PvP ON | */
	MSGTBL_MAPINFO_PVP_ON = 1047,
	/** NoGuild | */
	MSGTBL_MAPINFO_NO_GUILD = 1048,
	/** NoParty | */
	MSGTBL_MAPINFO_NO_PARTY = 1049,
	/** NightmareDrop | */
	MSGTBL_MAPINFO_NIGHTMARE_DROP = 1050,
	/** NoCalcRank | */
	MSGTBL_MAPINFO_NO_CALC_RANK = 1051,
	/** GvG Flags: */
	MSGTBL_MAPINFO_GVG_FLAGS = 1052,
	/** GvG ON | */
	MSGTBL_MAPINFO_GVG_ON = 1053,
	/** GvG Dungeon | */
	MSGTBL_MAPINFO_GVG_DUNGEON = 1054,
	/** GvG Castle | */
	MSGTBL_MAPINFO_GVG_CASTLE = 1055,
	/** NoParty | */
	MSGTBL_MAPINFO_NO_PARTY_GVG = 1056,
	/** Teleport Flags: */
	MSGTBL_MAPINFO_TELEPORT_FLAGS = 1057,
	/** NoTeleport | */
	MSGTBL_MAPINFO_NO_TELEPORT = 1058,
	/** Monster NoTeleport | */
	MSGTBL_MAPINFO_MONSTER_NO_TELEPORT = 1059,
	/** NoWarp | */
	MSGTBL_MAPINFO_NO_WARP = 1060,
	/** NoWarpTo | */
	MSGTBL_MAPINFO_NO_WARP_TO = 1061,
	/** NoReturn | */
	MSGTBL_MAPINFO_NO_RETURN = 1062,
	/** NoAutoloot | */
	MSGTBL_MAPINFO_NO_AUTOLOOT = 1063,
	/** NoMemo | */
	MSGTBL_MAPINFO_NO_MEMO = 1064,
	/** No Exp Penalty: %s | No Zeny Penalty: %s */
	MSGTBL_MAPINFO_NO_PENALTY = 1065,
	/** On */
	MSGTBL_MAPINFO_ON = 1066,
	/** Off */
	MSGTBL_MAPINFO_OFF = 1067,
	/** No Save (Return to last Save Point) */
	MSGTBL_MAPINFO_NO_SAVE = 1068,
	/** No Save, Save Point: %s, Random */
	MSGTBL_MAPINFO_NO_SAVE_RANDOM = 1069,
	/** No Save, Save Point: %s, %d, %d */
	MSGTBL_MAPINFO_NO_SAVE_SPECIFIC = 1070,
	/** Weather Flags: */
	MSGTBL_MAPINFO_WEATHER_FLAGS = 1071,
	/** Snow | */
	MSGTBL_MAPINFO_SNOW = 1072,
	/** Fog | */
	MSGTBL_MAPINFO_FOG = 1073,
	/** Sakura | */
	MSGTBL_MAPINFO_SAKURA = 1074,
	/** Clouds | */
	MSGTBL_MAPINFO_CLOUDS = 1075,
	/** Clouds2 | */
	MSGTBL_MAPINFO_CLOUDS2 = 1076,
	/** Fireworks | */
	MSGTBL_MAPINFO_FIREWORKS = 1077,
	/** Leaves | */
	MSGTBL_MAPINFO_LEAVES = 1078,
	/** NoViewID | */
	MSGTBL_MAPINFO_NO_VIEW_ID = 1079,
	/** Displays Night | */
	MSGTBL_MAPINFO_DISPLAYS_NIGHT = 1080,
	/** Other Flags: */
	MSGTBL_MAPINFO_OTHER_FLAGS = 1081,
	/** NoBranch | */
	MSGTBL_MAPINFO_NO_BRANCH = 1082,
	/** NoTrade | */
	MSGTBL_MAPINFO_NO_TRADE = 1083,
	/** NoVending | */
	MSGTBL_MAPINFO_NO_VENDING = 1084,
	/** NoDrop | */
	MSGTBL_MAPINFO_NO_DROP = 1085,
	/** NoSkill | */
	MSGTBL_MAPINFO_NO_SKILL = 1086,
	/** NoIcewall | */
	MSGTBL_MAPINFO_NO_ICEWALL = 1087,
	/** AllowKS | */
	MSGTBL_MAPINFO_ALLOW_KS = 1088,
	/** Reset | */
	MSGTBL_MAPINFO_RESET = 1089,
	/** Other Flags: */
	MSGTBL_MAPINFO_OTHER_FLAGS_2 = 1090,
	/** NoCommand | */
	MSGTBL_MAPINFO_NO_COMMAND = 1091,
	/** NoBaseEXP | */
	MSGTBL_MAPINFO_NO_BASE_EXP = 1092,
	/** NoJobEXP | */
	MSGTBL_MAPINFO_NO_JOB_EXP = 1093,
	/** NoMobLoot | */
	MSGTBL_MAPINFO_NO_MOB_LOOT = 1094,
	/** NoMVPLoot | */
	MSGTBL_MAPINFO_NO_MVP_LOOT = 1095,
	/** PartyLock | */
	MSGTBL_MAPINFO_PARTY_LOCK = 1096,
	/** GuildLock | */
	MSGTBL_MAPINFO_GUILD_LOCK = 1097,
	/** ----- Players in Map ----- */
	MSGTBL_PLAYERS_IN_MAP = 1098,
	/** Player '%s' (session #%d) | Location: %d,%d */
	MSGTBL_PLAYER_INFO = 1099,
	/** ----- NPCs in Map ----- */
	MSGTBL_NPCS_IN_MAP = 1100,
	/** North */
	MSGTBL_NORTH = 1101,
	/** North West */
	MSGTBL_NORTH_WEST = 1102,
	/** West */
	MSGTBL_WEST = 1103,
	/** South West */
	MSGTBL_SOUTH_WEST = 1104,
	/** South */
	MSGTBL_SOUTH = 1105,
	/** South East */
	MSGTBL_SOUTH_EAST = 1106,
	/** East */
	MSGTBL_EAST = 1107,
	/** North East */
	MSGTBL_NORTH_EAST = 1108,
	/** North */
	MSGTBL_NORTH_2 = 1109,
	/** Unknown */
	MSGTBL_MAPINFO_UNKNOWN = 1110,
	/** NPC %d: %s | Direction: %s | Sprite: %d | Location: %d %d */
	MSGTBL_NPC_INFO = 1111,
	/** NPC %d: %s::%s | Direction: %s | Sprite: %d | Location: %d %d */
	MSGTBL_NPC_EXTENDED_INFO = 1112,
	/** ----- Chats in Map ----- */
	MSGTBL_CHATS_IN_MAP = 1113,
	/** Chat: %s | Player: %s | Location: %d %d */
	MSGTBL_CHAT_INFO = 1114,
	/**    Users: %d/%d | Password: %s | Public: %s */
	MSGTBL_CHAT_ADDITIONAL_INFO = 1115,
	/** Yes */
	MSGTBL_YES = 1116,
	/** No */
	MSGTBL_NO = 1117,
	/** Please enter at least one valid list number (usage: @mapinfo <0-3> <map>). */
	MSGTBL_ENTER_VALID_LIST_NUMBER_2 = 1118,

	// @mount
	/** You have mounted your Dragon. */
	MSGTBL_MOUNTED_DRAGON = 1119,
	/** You have released your Dragon. */
	MSGTBL_RELEASED_DRAGON = 1120,
	/** You have mounted your Warg. */
	MSGTBL_MOUNTED_WARG = 1121,
	/** You have released your Warg. */
	MSGTBL_RELEASED_WARG = 1122,
	/** You have mounted your Mado Gear. */
	MSGTBL_MOUNTED_MADO_GEAR = 1123,
	/** You have released your Mado Gear. */
	MSGTBL_RELEASED_MADO_GEAR = 1124,

	// @guildspy
	/** The mapserver has spy command support disabled. */
	MSGTBL_SPY_SUPPORT_DISABLED = 1125,
	/** Please enter a guild name/ID (usage: @guildspy <guild_name/ID>). */
	MSGTBL_ENTER_GUILD_NAME_OR_ID = 1126,

	// @partyspy
	/** Please enter a party name/ID (usage: @partyspy <party_name/ID>). */
	MSGTBL_ENTER_PARTY_NAME_OR_ID = 1127,

	// @nuke
	/** Please enter a player name (usage: @nuke <char name>). */
	MSGTBL_ENTER_PLAYER_NAME_FOR_NUKE = 1128,

	// @tonpc
	/** Please enter an NPC name (usage: @tonpc <NPC_name>). */
	MSGTBL_ENTER_NPC_NAME_FOR_TONPC = 1129,

	// @enablenpc
	/** Please enter an NPC name (usage: @enablenpc <NPC_name>). */
	MSGTBL_ENTER_NPC_NAME_FOR_ENABLENPC = 1130,

	// @hidenpc
	/** Please enter an NPC name (usage: @hidenpc <NPC_name>). */
	MSGTBL_ENTER_NPC_NAME_FOR_HIDENPC = 1131,

	// @loadnpc
	/** Please enter a script file name (usage: @loadnpc <file name>). */
	MSGTBL_ENTER_SCRIPT_FOR_LOADNPC = 1132,

	// @unloadnpc
	/** Please enter an NPC name (Usage: @unloadnpc <NPC_name> {<flag>}). */
	MSGTBL_ENTER_NPC_NAME_FOR_UNLOADNPC = 1133,

	// @jail
	/** Please enter a player name (usage: @jail <char_name>). */
	MSGTBL_ENTER_NAME_FOR_JAIL = 1134,

	// @unjail
	/** Please enter a player name (usage: @unjail/@discharge <char_name>). */
	MSGTBL_ENTER_NAME_FOR_UNJAIL = 1135,

	// @jailfor
	/** Invalid time for jail command. */
	MSGTBL_INVALID_TIME_FOR_JAIL = 1136,
	/** You are now */
	MSGTBL_YOU_ARE_NOW = 1137,
	/** This player is now */
	MSGTBL_PLAYER_IS_NOW = 1138,

	// @jailtime
	/** You are not in jail. */
	MSGTBL_NOT_IN_JAIL = 1139,
	/** You have been jailed indefinitely. */
	MSGTBL_JAILED_INDEFINITELY = 1140,
	/** You have been jailed for an unknown amount of time. */
	MSGTBL_JAILED_UNKNOWN_TIME = 1141,
	/** You will remain */
	MSGTBL_WILL_REMAIN = 1142,

	// @disguise
	/** Please enter a Monster/NPC name/ID (usage: @disguise <name/ID>). */
	MSGTBL_DISGUISE_ENTER_TARGET = 1143,
	/** Character cannot be disguised while mounted. */
	MSGTBL_NOT_DISGUISE_MOUNTED = 1144,

	// @disguiseall
	/** Please enter a Monster/NPC name/ID (usage: @disguiseall <name/ID>). */
	MSGTBL_DISGUISE_ALL_ENTER_TARGET = 1145,

	// @disguiseguild
	/** Please enter a mob name/ID and guild name/ID (usage: @disguiseguild <mob name/ID>, <guild name/ID>). */
	MSGTBL_DISGUISE_GUILD_ENTER_TARGET = 1146,

	// @undisguiseguild
	/** Please enter guild name/ID (usage: @undisguiseguild <guild name/ID>). */
	MSGTBL_UNDISGUISE_GUILD_ENTER_TARGET = 1147,

	// @exp
	/** Base Level: %d (%.3f%%) | Job Level: %d (%.3f%%) */
	MSGTBL_EXP_INFO = 1148,

	// @broadcast
	/** Please enter a message (usage: @broadcast <message>). */
	MSGTBL_ENTER_BROADCAST_MSG = 1149,

	// @localbroadcast
	/** Please enter a message (usage: @localbroadcast <message>). */
	MSGTBL_ENTER_LOCAL_BROADCAST_MSG = 1150,

	// @email
	/** Please enter two e-mail addresses (usage: @email <current@email> <new@email>). */
	MSGTBL_ENTER_EMAIL_ADDRESSES = 1151,

	// @effect
	/** Please enter an effect number (usage: @effect <effect number>). */
	MSGTBL_ENTER_EFFECT_NUMBER = 1152,

	// @npcmove
	/** Usage: @npcmove <X> <Y> <npc_name> */
	MSGTBL_NPC_MOVE_USAGE = 1153,
	/** NPC is not in this map. */
	MSGTBL_NPC_NOT_IN_MAP = 1154,
	/** NPC moved. */
	MSGTBL_NPC_MOVED = 1155,

	// @addwarp
	/** Usage: @addwarp <mapname> <X> <Y> <npc name> */
	MSGTBL_ADDWARP_USAGE = 1156,
	/** Unknown map '%s'. */
	MSGTBL_ADDWARP_UNKNOWN_MAP = 1157,
	/** New warp NPC '%s' created. */
	MSGTBL_ADDWARP_NPC_CREATED = 1158,

	// @follow
	/** Follow mode OFF. */
	MSGTBL_FOLLOW_MODE_OFF = 1159,
	/** Follow mode ON. */
	MSGTBL_FOLLOW_MODE_ON = 1160,

	// @storeall
	/** You currently cannot open your storage. */
	MSGTBL_CANNOT_OPEN_STORAGE = 1161,
	/** All items stored. */
	MSGTBL_ALL_ITEMS_STORED = 1162,

	// @skillid
	/** Please enter a skill name to look up (usage: @skillid <skill name>). */
	MSGTBL_SKILL_ID_ENTER_NAME = 1163,
	/** skill %d: %s (%s) */
	MSGTBL_SKILL_ID_INFO = 1164,

	// @useskill
	/** Usage: @useskill <skill ID> <skill level> <target> */
	MSGTBL_USESKILL_USAGE = 1165,

	// @displayskill
	/** Usage: @displayskill <skill ID> {<skill level>} */
	MSGTBL_DISPLAYSKILL_USAGE = 1166,

	// @skilltree
	/** Usage: @skilltree <skill ID> <target> */
	MSGTBL_SKILLTREE_USAGE = 1167,
	/** Player is using %s skill tree (%d basic points). */
	MSGTBL_PLAYER_SKILL_TREE_INFO = 1168,
	/** The player cannot use that skill. */
	MSGTBL_CANNOT_USE_SKILL = 1169,
	/** Player requires level %d of skill %s. */
	MSGTBL_PLAYER_REQUIRES_LEVEL = 1170,
	/** The player meets all the requirements for that skill. */
	MSGTBL_PLAYER_MEETS_REQUIREMENTS = 1171,

	// @marry
	/** Usage: @marry <char name> */
	MSGTBL_MARRY_USAGE = 1172,
	/** They are married... wish them well. */
	MSGTBL_ALREADY_MARRIED = 1173,
	/** The two cannot wed because one is either a baby or already married. */
	MSGTBL_CANNOT_WED = 1174,

	// @divorce
	/** '%s' is not married. */
	MSGTBL_NOT_MARRIED = 1175,
	/** '%s' and his/her partner are now divorced. */
	MSGTBL_DIVORCE_SUCCESS = 1176,

	// @changelook
	/** Usage: @changelook {<position>} <view id> */
	MSGTBL_CHANGELOOK_USAGE = 1177,
	/** Position: 1:Top 2:Middle 3:Bottom 4:Weapon 5:Shield 6:Shoes 7:Robe 8:Body */
	MSGTBL_CHANGELOOK_POSITION_INFO = 1178,

	// @autotrade
	/** Autotrade is not allowed in this map. */
	MSGTBL_AUTOTRADE_NOT_ALLOWED = 1179,
	/** You cannot autotrade when dead. */
	MSGTBL_CANNOT_AUTOTRADE_WHEN_DEAD = 1180,

	// @changegm
	/** You need to be a Guild Master to use this command. */
	MSGTBL_CHANGEGM_GM_REQUIRED = 1181,
	/** You cannot change guild leaders in this map. */
	MSGTBL_CHANGEGM_MAP_RESTRICTED = 1182,
	/** Usage: @changegm <guild_member_name> */
	MSGTBL_CHANGEGM_USAGE = 1183,
	/** Target character must be online and be a guild member. */
	MSGTBL_CHANGEGM_TARGET_CONDITIONS = 1184,

	// @changeleader
	/** Usage: @changeleader <party_member_name> */
	MSGTBL_CHANGELEADER_USAGE = 1185,

	// @partyoption
	/** Usage: @partyoption <pickup share: yes/no> <item distribution: yes/no> */
	MSGTBL_PARTYOPTION_USAGE = 1186,

	// @autoloot
	/** Autolooting items with drop rates of %0.02f%% and below. */
	MSGTBL_AUTOLOOT_DROP_RATE = 1187,
	/** Autoloot is now off. */
	MSGTBL_AUTOLOOT_OFF = 1188,

	// @autolootitem
	/** Item not found. */
	MSGTBL_AUTOLOOTITEM_NOT_FOUND = 1189,
	/** You're already autolooting this item. */
	MSGTBL_AUTOLOOTITEM_ALREADY = 1190,
	/** Your autolootitem list is full. Remove some items first with @autolootid -<item name or ID>. */
	MSGTBL_AUTOLOOTITEM_LIST_FULL = 1191,
	/** Autolooting item: '%s'/'%s' {%d} */
	MSGTBL_AUTOLOOTITEM_ADDED = 1192,
	/** You're currently not autolooting this item. */
	MSGTBL_AUTOLOOTITEM_NOT_CURRENTLY = 1193,
	/** Removed item: '%s'/'%s' {%d} from your autolootitem list. */
	MSGTBL_AUTOLOOTITEM_REMOVED = 1194,
	/** You can have %d items on your autolootitem list. */
	MSGTBL_AUTOLOOTITEM_LIST_LIMIT = 1195,
	/** To add an item to the list, use "@alootid +<item name or ID>". To remove an item, use "@alootid -<item name or ID>". */
	MSGTBL_AUTOLOOTITEM_ADD_REMOVE_INFO = 1196,
	/** "@alootid reset" will clear your autolootitem list. */
	MSGTBL_AUTOLOOTITEM_RESET_INFO = 1197,
	/** Your autolootitem list is empty. */
	MSGTBL_AUTOLOOTITEM_EMPTY = 1198,
	/** Items on your autolootitem list: */
	MSGTBL_AUTOLOOTITEM_LIST_HEADER = 1199,
	/** Your autolootitem list has been reset. */
	MSGTBL_AUTOLOOTITEM_RESET_SUCCESS = 1200,

	// @guildstorage
	/** Your guild's storage has already been opened by another member, try again later. */
	MSGTBL_GUILDSTORAGE_ALREADY_OPENED = 1201,

	// 1202 FREE

	// @snow
	/** Snow has stopped falling. */
	MSGTBL_SNOW_STOPPED = 1203,
	/** It has started to snow. */
	MSGTBL_SNOW_STARTED = 1204,

	// @sakura
	/** Cherry tree leaves no longer fall. */
	MSGTBL_SAKURA_STOPPED = 1205,
	/** Cherry tree leaves have begun to fall. */
	MSGTBL_SAKURA_STARTED = 1206,

	// @clouds
	/** Clouds have disappeared. */
	MSGTBL_CLOUDS_DISAPPEARED = 1207,
	/** Clouds appeared. */
	MSGTBL_CLOUDS_APPEARED = 1208,

	// @clouds2
	/** Alternative clouds have disappeared. */
	MSGTBL_CLOUDS2_DISAPPEARED = 1209,
	/** Alternative clouds appeared. */
	MSGTBL_CLOUDS2_APPEARED = 1210,

	// @fog
	/** The fog has gone. */
	MSGTBL_FOG_GONE = 1211,
	/** Fog hangs over. */
	MSGTBL_FOG_APPEARED = 1212,

	// @leaves
	/** Leaves have stopped falling. */
	MSGTBL_LEAVES_STOPPED = 1213,
	/** Leaves started falling. */
	MSGTBL_LEAVES_STARTED = 1214,

	// @fireworks
	/** Fireworks have ended. */
	MSGTBL_FIREWORKS_ENDED = 1215,
	/** Fireworks are launched. */
	MSGTBL_FIREWORKS_LAUNCHED = 1216,

	// @sound
	/** Please enter a sound filename (usage: @sound <filename>). */
	MSGTBL_SOUND_USAGE = 1217,

	// @mobsearch
	/** Please enter a monster name (usage: @mobsearch <monster name>). */
	MSGTBL_MOBSEARCH_USAGE = 1218,
	/** Invalid mob ID %s! */
	MSGTBL_MOBSEARCH_INVALID_ID = 1219,
	/** Mob Search... %s %s */
	MSGTBL_MOBSEARCH_RESULT = 1220,

	// @cleanmap
	/** All dropped items have been cleaned up. */
	MSGTBL_CLEANMAP_SUCCESS = 1221,

	// @npctalk
	/** Please enter the correct parameters (usage: @npctalk <npc name>, <message>). */
	MSGTBL_NPCTALK_USAGE = 1222,
	/** Please enter the correct parameters (usage: @npctalkc <color> <npc name>, <message>). */
	MSGTBL_NPCTALKC_USAGE = 1223,

	// @pettalk
	/** Please enter a message (usage: @pettalk <message>). */
	MSGTBL_PETTALK_USAGE = 1224,

	// @summon
	/** Please enter a monster name (usage: @summon <monster name> {duration}). */
	MSGTBL_SUMMON_USAGE = 1225,

	// @adjgroup
	/** Usage: @adjgroup <group_id> */
	MSGTBL_ADJGROUP_USAGE = 1226,
	/** Specified group does not exist. */
	MSGTBL_ADJGROUP_NOT_EXIST = 1227,
	/** Group changed successfully. */
	MSGTBL_ADJGROUP_CHANGE_SUCCESS = 1228,
	/** Your group has been changed. */
	MSGTBL_ADJGROUP_YOUR_CHANGE = 1229,

	// @trade
	/** Please enter a player name (usage: @trade <char name>). */
	MSGTBL_TRADE_USAGE = 1230,

	// @setbattleflag
	/** Usage: @setbattleflag <flag> <value> */
	MSGTBL_SETBATTLEFLAG_USAGE = 1231,
	/** Unknown battle_config flag. */
	MSGTBL_SETBATTLEFLAG_UNKNOWN_FLAG = 1232,
	/** Set battle_config as requested. */
	MSGTBL_SETBATTLEFLAG_SUCCESS = 1233,

	// @unmute
	/** Please enter a player name (usage: @unmute <char name>). */
	MSGTBL_UNMUTE_USAGE = 1234,
	/** Player is not muted. */
	MSGTBL_UNMUTE_NOT_MUTED = 1235,
	/** Player unmuted. */
	MSGTBL_UNMUTE_SUCCESS = 1236,

	// @mute
	/** Usage: @mute <time> <char name> */
	MSGTBL_MUTE_USAGE = 1237,

	// @identify
	/** There are no items to appraise. */
	MSGTBL_IDENTIFY_NO_ITEMS = 1238,

	// @mobinfo
	/** Please enter a monster name/ID (usage: @mobinfo <monster_name_or_monster_ID>). */
	MSGTBL_MOBINFO_USAGE = 1239,
	/** MVP Monster: '%s'/'%s'/'%s' (%d) */
	MSGTBL_MOBINFO_MVP = 1240,
	/** Monster: '%s'/'%s'/'%s' (%d) */
	MSGTBL_MOBINFO_NORMAL = 1241,
	/**  Lv:%d  HP:%d  Base EXP:%u  Job EXP:%u  HIT:%d  FLEE:%d */
	MSGTBL_MOBINFO_STATS = 1242,
	/**  DEF:%d  MDEF:%d  STR:%d  AGI:%d  VIT:%d  INT:%d  DEX:%d  LUK:%d */
	MSGTBL_MOBINFO_ATTRIBUTES = 1243,
	/**  ATK:%d~%d  Range:%d~%d~%d  Size:%s  Race: %s  Element: %s (Lv:%d) */
	MSGTBL_MOBINFO_ATTACK = 1244,
	/**  Drops: */
	MSGTBL_MOBINFO_DROPS_HEADER = 1245,
	/** This monster has no drops. */
	MSGTBL_MOBINFO_NO_DROPS = 1246,
	/**  MVP Bonus EXP:%u */
	MSGTBL_MOBINFO_MVP_BONUS_EXP = 1247,
	/**  MVP Items: */
	MSGTBL_MOBINFO_MVP_ITEMS_HEADER = 1248,
	/** This monster has no MVP prizes. */
	MSGTBL_MOBINFO_NO_MVP_PRIZES = 1249,

	// @showmobs
	/** Invalid mob id %s! */
	MSGTBL_SHOWMOBS_INVALID_ID = 1250,
	/** Can't show boss mobs! */
	MSGTBL_SHOWMOBS_BOSS_MOBS = 1251,
	/** Mob Search... %s %s */
	MSGTBL_SHOWMOBS_SEARCH_RESULT = 1252,

	// @homlevel
	/** Please enter a level adjustment (usage: @homlevel <number of levels>). */
	MSGTBL_HOMUN_ENTER_LV_ADJUST = 1253,

	// @homlevel / @homevolve / @homfriendly / @homhungry / @homtalk / @hominfo / @homstats
	/** You do not have a homunculus. */
	MSGTBL_HOMUN_NOT_EXIST = 1254,

	// @homevolve
	/** Your homunculus doesn't evolve. */
	MSGTBL_HOMUN_NO_EVOLVE = 1255,

	// @makehomun
	/** Please enter a homunculus ID (usage: @makehomun <homunculus id>). */
	MSGTBL_MAKEHOMUN_USAGE = 1256,
	/** Invalid Homunculus ID. */
	MSGTBL_MAKEHOMUN_INVALID_ID = 1257,

	// @homfriendly
	/** Please enter an intimacy value (usage: @homfriendly <intimacy value [0-1000]>). */
	MSGTBL_HOMFRIENDLY_USAGE = 1258,

	// @homhungry
	/** Please enter a hunger value (usage: @homhungry <hunger value [0-100]>). */
	MSGTBL_HOMHUNGRY_USAGE = 1259,

	// @homtalk
	/** Please enter a message (usage: @homtalk <message>). */
	MSGTBL_HOMTALK_USAGE = 1260,

	// @hominfo
	/** Homunculus stats: */
	MSGTBL_HOMINFO_STATS_HEADER = 1261,
	/** HP: %d/%d - SP: %d/%d */
	MSGTBL_HOMINFO_HP_SP = 1262,
	/** ATK: %d - MATK: %d~%d */
	MSGTBL_HOMINFO_ATK_MATK = 1263,
	/** Hungry: %d - Intimacy: %u */
	MSGTBL_HOMINFO_HUNGRY_INTIMACY = 1264,
	/** Stats: Str %d / Agi %d / Vit %d / Int %d / Dex %d / Luk %d */
	MSGTBL_HOMINFO_STATS_DETAIL = 1265,

	// @homstats
	/** Homunculus growth stats (Lv %d %s): */
	MSGTBL_HOMSTATS_GROWTH = 1266,
	/** Max HP: %d (%d~%d) */
	MSGTBL_HOMSTATS_MAX_HP = 1267,
	/** Max SP: %d (%d~%d) */
	MSGTBL_HOMSTATS_MAX_SP = 1268,
	/** Str: %d (%d~%d) */
	MSGTBL_HOMSTATS_STR = 1269,
	/** Agi: %d (%d~%d) */
	MSGTBL_HOMSTATS_AGI = 1270,
	/** Vit: %d (%d~%d) */
	MSGTBL_HOMSTATS_VIT = 1271,
	/** Int: %d (%d~%d) */
	MSGTBL_HOMSTATS_INT = 1272,
	/** Dex: %d (%d~%d) */
	MSGTBL_HOMSTATS_DEX = 1273,
	/** Luk: %d (%d~%d) */
	MSGTBL_HOMSTATS_LUK = 1274,

	// @homshuffle
	/** Homunculus stats altered. */
	MSGTBL_HOMSHUFFLE_ALTERED = 1275,

	// @iteminfo
	/** Please enter an item name/ID (usage: @ii/@iteminfo <item name/ID>). */
	MSGTBL_ITEMINFO_USAGE = 1276,
	/** Item: '%s'/'%s'[%d] (%d) Type: %s | Extra Effect: %s */
	MSGTBL_ITEMINFO_DETAILS = 1277,
	/** None */
	MSGTBL_ITEMINFO_NONE = 1278,
	/** With script */
	MSGTBL_ITEMINFO_WITH_SCRIPT = 1279,
	/** NPC Buy:%dz, Sell:%dz | Weight: %.1f  */
	MSGTBL_ITEMINFO_NPC_DETAILS = 1280,
	/**  - Available in shops only. */
	MSGTBL_ITEMINFO_SHOPS_ONLY = 1281,
	/**  - Maximal monsters drop chance: %02.02f%% */
	MSGTBL_ITEMINFO_MAX_DROP_CHANCE = 1282,
	/**  - Monsters don't drop this item. */
	MSGTBL_ITEMINFO_NO_MONSTER_DROP = 1283,

	// @whodrops
	/** Please enter item name/ID (usage: @whodrops <item name/ID>). */
	MSGTBL_WHODROPS_USAGE = 1284,
	/** Item: '%s'[%d] */
	MSGTBL_WHODROPS_ITEM = 1285,
	/**  - Item is not dropped by any mobs. */
	MSGTBL_WHODROPS_NO_DROP = 1286,
	/**  - Common mobs with highest drop chance (only max %d are listed): */
	MSGTBL_WHODROPS_COMMON_MOBS = 1287,

	// @whereis
	/** Please enter a monster name/ID (usage: @whereis <monster_name_or_monster_ID>). */
	MSGTBL_WHEREIS_USAGE = 1288,
	/** %s spawns in: */
	MSGTBL_WHEREIS_SPAWNS_IN = 1289,
	/** This monster does not spawn normally. */
	MSGTBL_WHEREIS_NO_SPAWN = 1290,

	// @mobinfo ...
	/**  ATK:%d~%d MATK:%d~%d Range:%d~%d~%d  Size:%s  Race: %s  Element: %s (Lv:%d) */
	MSGTBL_MOBINFO_ADDITIONAL_INFO = 1291,

	/** PrivateAirshipStartable |  */
	MSGTBL_MAPINFO_PRIVATE_AIRSHIP_STARTABLE = 1292,
	/** PrivateAirshipEndable |  */
	MSGTBL_MAPINFO_PRIVATE_AIRSHIP_ENDABLE = 1293,
	// 1294 used by hercules chat feature

	// @version
	/** %s revision '%s' (src) / '%s' (scripts) */
	MSGTBL_VERSION_REVISION_INFO = 1295,
	/** Hercules %d-bit for %s */
	MSGTBL_HERCULES_BIT_INFO = 1296,

	// @mutearea
	/** Please enter a time in minutes (usage: @mutearea/@stfu <time in minutes>). */
	MSGTBL_MUTEAREA_USAGE = 1297,

	// @rates
	/** Experience rates: Base %.2fx / Job %.2fx */
	MSGTBL_RATES_EXP = 1298,
	/** Normal Drop Rates: Common %.2fx / Healing %.2fx / Usable %.2fx / Equipment %.2fx / Card %.2fx */
	MSGTBL_RATES_NORMAL_DROP = 1299,
	/** Boss Drop Rates: Common %.2fx / Healing %.2fx / Usable %.2fx / Equipment %.2fx / Card %.2fx */
	MSGTBL_RATES_BOSS_DROP = 1300,
	/** Other Drop Rates: MvP %.2fx / Card-Based %.2fx / Treasure %.2fx */
	MSGTBL_RATES_OTHER_DROP = 1301,

	// @me
	/** Please enter a message (usage: @me <message>). */
	MSGTBL_ME_USAGE = 1302,

	// @size / @sizeall / @sizeguild
	/** Size change applied. */
	MSGTBL_SIZE_CHANGE_APPLIED = 1303,

	// @sizeguild
	/** Please enter guild name/ID (usage: @sizeguild <size> <guild name/ID>). */
	MSGTBL_SIZEGUILD_USAGE = 1304,

	// @monsterignore
	/** You are now immune to attacks. */
	MSGTBL_MONSTERIGNORE_IMMUNE = 1305,
	/** Returned to normal state. */
	MSGTBL_MONSTERIGNORE_NORMAL_STATE = 1306,

	// @fakename
	/** Returned to real name. */
	MSGTBL_FAKENAME_RETURNED = 1307,
	/** You must enter a name. */
	MSGTBL_FAKENAME_NO_NAME = 1308,
	/** Fake name must be at least two characters. */
	MSGTBL_FAKENAME_SHORT_NAME = 1309,
	/** Fake name enabled. */
	MSGTBL_FAKENAME_ENABLED = 1310,

	// @mapflag
	/** Enabled Mapflags in this map: */
	MSGTBL_MAPFLAG_ENABLED = 1311,
	/** Usage: "@mapflag monster_noteleport 1" (0=Off | 1=On) */
	MSGTBL_MAPFLAG_USAGE = 1312,
	/** Type "@mapflag available" to list the available mapflags. */
	MSGTBL_MAPFLAG_TYPE_AVAILABLE = 1313,
	/** Invalid flag name or flag. */
	MSGTBL_MAPFLAG_INVALID_FLAG = 1314,
	/** Available Flags: */
	MSGTBL_MAPFLAG_AVAILABLE_FLAGS = 1315,

	// @showexp
	/** Gained exp will not be shown. */
	MSGTBL_SHOWEXP_NOT_SHOWN = 1316,
	/** Gained exp is now shown. */
	MSGTBL_SHOWEXP_NOW_SHOWN = 1317,

	// @showzeny
	/** Gained zeny will not be shown. */
	MSGTBL_SHOWZENY_NOT_SHOWN = 1318,
	/** Gained zeny is now shown. */
	MSGTBL_SHOWZENY_NOW_SHOWN = 1319,

	// @showdelay
	/** Skill delay failures will not be shown. */
	MSGTBL_SHOWDELAY_NOT_SHOWN = 1320,
	/** Skill delay failures are now shown. */
	MSGTBL_SHOWDELAY_NOW_SHOWN = 1321,

	// @cash
	/** Please enter an amount. */
	MSGTBL_CASH_ENTER_AMOUNT = 1322,

	// @clone
	/** You must enter a player name or ID. */
	MSGTBL_CLONE_ENTER_NAME_OR_ID = 1323,

	// @feelreset
	/** Reset 'Feeling' maps. */
	MSGTBL_FEELRESET_RESET = 1324,

	// @noks
	/** [ K.S Protection Inactive ] */
	MSGTBL_NOKS_INACTIVE = 1325,
	/** [ K.S Protection Active - Option: Party ] */
	MSGTBL_NOKS_ACTIVE_PARTY = 1326,
	/** [ K.S Protection Active - Option: Self ] */
	MSGTBL_NOKS_ACTIVE_SELF = 1327,
	/** [ K.S Protection Active - Option: Guild ] */
	MSGTBL_NOKS_ACTIVE_GUILD = 1328,
	/** Usage: @noks <self|party|guild> */
	MSGTBL_NOKS_USAGE = 1329,

	// @allowks
	/** [ Map K.S Protection Active ] */
	MSGTBL_ALLOWKS_ACTIVE = 1330,
	/** [ Map K.S Protection Inactive ] */
	MSGTBL_ALLOWKS_INACTIVE = 1331,

	// @itemlist
	/** ------ %s items list of '%s' ------ */
	MSGTBL_ITEMLIST_TITLE = 1332,
	/**  | equipped:  */
	MSGTBL_ITEMLIST_EQUIPPED = 1333,
	/** garment,  */
	MSGTBL_ITEMLIST_GARMENT = 1334,
	/** left accessory,  */
	MSGTBL_ITEMLIST_LEFT_ACCESSORY = 1335,
	/** body/armor,  */
	MSGTBL_ITEMLIST_BODY_ARMOR = 1336,
	/** right hand,  */
	MSGTBL_ITEMLIST_RIGHT_HAND = 1337,
	/** left hand,  */
	MSGTBL_ITEMLIST_LEFT_HAND = 1338,
	/** both hands,  */
	MSGTBL_ITEMLIST_BOTH_HANDS = 1339,
	/** feet,  */
	MSGTBL_ITEMLIST_FEET = 1340,
	/** right accessory,  */
	MSGTBL_ITEMLIST_RIGHT_ACCESSORY = 1341,
	/** lower head,  */
	MSGTBL_ITEMLIST_LOWER_HEAD = 1342,
	/** top head,  */
	MSGTBL_ITEMLIST_TOP_HEAD = 1343,
	/** lower/top head,  */
	MSGTBL_ITEMLIST_LOWER_TOP_HEAD = 1344,
	/** mid head,  */
	MSGTBL_ITEMLIST_MID_HEAD = 1345,
	/** lower/mid head,  */
	MSGTBL_ITEMLIST_LOWER_MID_HEAD = 1346,
	/** lower/mid/top head,  */
	MSGTBL_ITEMLIST_LOWER_MID_TOP_HEAD = 1347,
	/**  -> (pet egg, pet id: %u, named) */
	MSGTBL_ITEMLIST_PET_NAMED = 1348,
	/**  -> (pet egg, pet id: %u, unnamed) */
	MSGTBL_ITEMLIST_PET_UNNAMED = 1349,
	/**  -> (crafted item, creator id: %u, star crumbs %d, element %d) */
	MSGTBL_ITEMLIST_CRAFTED_ITEM = 1350,
	/**  -> (produced item, creator id: %u) */
	MSGTBL_ITEMLIST_PRODUCED_ITEM = 1351,
	/**  -> (card(s):  */
	MSGTBL_ITEMLIST_CARDS = 1352,
	/** No item found in this player's %s. */
	MSGTBL_ITEMLIST_NO_ITEM_FOUND = 1353,
	/** %d item(s) found in %d %s slots. */
	MSGTBL_ITEMLIST_ITEMS_FOUND = 1354,

	// @delitem
	/** Please enter an item name/ID, a quantity, and a player name (usage: #delitem <player> <item_name_or_ID> <quantity>). */
	MSGTBL_DELITEM_USAGE = 1355,

	// @font
	/** Returning to normal font. */
	MSGTBL_FONT_RETURN_NORMAL = 1356,
	/** Use @font <1-9> to change your message font. */
	MSGTBL_FONT_CHANGE_USAGE = 1357,
	/** Use 0 or no parameter to return to normal font. */
	MSGTBL_FONT_RETURN_NO_PARAM = 1358,
	/** Invalid font. Use a value from 0 to 9. */
	MSGTBL_FONT_INVALID = 1359,
	/** Font changed. */
	MSGTBL_FONT_CHANGED = 1360,
	/** You are already using this font. */
	MSGTBL_FONT_ALREADY_USED = 1361,

	// @new_mount
	/** NOTICE: If you crash with mount, your Lua files are outdated. */
	MSGTBL_NEW_MOUNT_NOTICE = 1362,
	/** You are mounted now. */
	MSGTBL_NEW_MOUNT_MOUNTED = 1363,
	/** You have released your mount. */
	MSGTBL_NEW_MOUNT_RELEASED = 1364,

	// @accinfo
	/** Usage: @accinfo/@accountinfo <account_id/char name> */
	MSGTBL_ACCINFO_USAGE = 1365,
	/** You may search partial name by making use of '%' in the search, ex. "@accinfo %Mario%" lists all characters whose name contains "Mario". */
	MSGTBL_ACCINFO_SEARCH_TIP = 1366,

	// @set
	/** Usage: @set <variable name> <value> */
	MSGTBL_SET_USAGE = 1367,
	/** Usage: ex. "@set PoringCharVar 50" */
	MSGTBL_SET_EXAMPLE_1 = 1368,
	/** Usage: ex. "@set PoringCharVarSTR$ Super Duper String" */
	MSGTBL_SET_EXAMPLE_2 = 1369,
	/** Usage: ex. "@set PoringCharVarSTR$" outputs its value, Super Duper String. */
	MSGTBL_SET_EXAMPLE_3 = 1370,
	/** NPC variables may not be used with @set. */
	MSGTBL_SET_NPC_VARIABLE = 1371,
	/** Instance variables may not be used with @set. */
	MSGTBL_SET_INSTANCE_VARIABLE = 1372,
	/** %s value is now :%d */
	MSGTBL_SET_VALUE_INT = 1373,
	/** %s value is now :%s */
	MSGTBL_SET_VALUE_STRING = 1374,
	/** %s is empty */
	MSGTBL_SET_EMPTY = 1375,
	/** %s data type is not supported :%u */
	MSGTBL_SET_UNSUPPORTED_TYPE = 1376,

	// @reloadquestdb
	/** Quest database has been reloaded. */
	MSGTBL_RELOADQUESTDB_SUCCESS = 1377,

	// @addperm
	/** Usage: %s <permission_name> */
	MSGTBL_ADDPERM_USAGE = 1378,
	/** -- Permission List */
	MSGTBL_ADDPERM_PERMISSION_LIST = 1379,
	/** '%s' is not a known permission. */
	MSGTBL_ADDPERM_UNKNOWN_PERMISSION = 1380,
	/** User '%s' already possesses the '%s' permission. */
	MSGTBL_ADDPERM_USER_ALREADY_PERMISSION = 1381,
	/** User '%s' doesn't possess the '%s' permission. */
	MSGTBL_ADDPERM_USER_NO_PERMISSION = 1382,
	/** -- User '%s' Permissions */
	MSGTBL_ADDPERM_USER_PERMISSIONS = 1383,
	/** User '%s' permissions updated successfully. The changes are temporary. */
	MSGTBL_ADDPERM_SUCCESS = 1384,

	// @unloadnpcfile
	/** Usage: @unloadnpcfile <path> {<flag>} */
	MSGTBL_UNLOADNPCFILE_USAGE = 1385,
	/** File unloaded. Be aware that some changes made by NPC are not reverted on unload. See doc/atcommands.txt for details. */
	MSGTBL_UNLOADNPCFILE_SUCCESS = 1386,
	/** File not found. */
	MSGTBL_UNLOADNPCFILE_NOT_FOUND = 1387,

	// General command messages
	/** Charcommand failed (usage: %c<command> <char name> <parameters>). */
	MSGTBL_CHARCOMMAND_FAILED = 1388,
	/** %s failed. Player not found. */
	MSGTBL_CHARCOMMAND_PLAYER_NOT_FOUND = 1389,

	// @cart
	/** Unknown Cart (usage: %s <0-%d>). */
	MSGTBL_CART_UNKNOWN_CART = 1390,
	/** You do not possess a cart to be removed */
	MSGTBL_CART_NO_CART_TO_REMOVE = 1391,
	/** Cart Added. */
	MSGTBL_CART_ADDED = 1392,

	// atcommand.c::is_atcommand
	/** You can't use commands while dead. */
	MSGTBL_IS_ATCOMMAND_DEAD = 1393,

	// @clearstorage
	/** Your storage was cleaned. */
	MSGTBL_CLEARSTORAGE_SUCCESS = 1394,
	/** Your guild storage was cleaned. */
	MSGTBL_CLEARGUILDSTORAGE_SUCCESS = 1395,

	// @clearcart
	/** You do not have a cart to be cleaned. */
	MSGTBL_CLEARCART_NO_CART = 1396,
	/** Your cart was cleaned. */
	MSGTBL_CLEARCART_SUCCESS = 1397,

	// @skillid (extension)
	/** -- Displaying first %d partial matches */
	MSGTBL_SKILLID_PARTIAL_MATCHES = 1398,

	// @join
	/** Unknown Channel (usage: %s <#channel_name>) */
	MSGTBL_JOIN_UNKNOWN_CHANNEL = 1399,
	/** Unknown Channel '%s' (usage: %s <#channel_name>) */
	MSGTBL_JOIN_UNKNOWN_CHANNEL_NAME = 1400,
	/** '%s' Channel is password protected (usage: %s <#channel_name> <password>) */
	MSGTBL_JOIN_CHANNEL_PW_PROTECTED = 1401,
	// 1402 used by hercules chat feature
	/** You're now in the '%s' channel */
	MSGTBL_JOIN_CHANNEL_SUCCESS = 1403,

	// Hercules Chat Feature
	/** You're not in that channel, type '@join <#channel_name>' */
	MSGTBL_HERC_CHAT_NOT_IN_CHANNEL = 1402,
	/** You're now in the '#%s' channel for '%s' */
	MSGTBL_HERC_CHAT_JOIN_SUCCESS = 1435,
	/** You're already in the '%s' channel */
	MSGTBL_HERC_CHAT_ALREADY_IN_CHANNEL = 1436,
	/** You're not allowed to talk on this channel */
	MSGTBL_HERC_CHAT_NOT_ALLOWED_TALK = 1294,

	// @channel
	/** %s failed */
	MSGTBL_CHANNEL_FAILED = 1404,
	/** Channel name must start with a '#' */
	MSGTBL_CHANNEL_NAME_START = 1405,
	/** Channel length must be between 3 and %d */
	MSGTBL_CHANNEL_NAME_LENGTH_INVALID = 1406,
	/** Channel '%s' is not available */
	MSGTBL_CHANNEL_NOT_AVAILABLE = 1407,
	/** Channel password may not contain spaces */
	MSGTBL_CHANNEL_PW_INVALID = 1408,
	/** - #%s ( %d users ) */
	MSGTBL_CHANNEL_LIST_ENTRY = 1409,
	/** -- Public Channels */
	MSGTBL_CHANNEL_PUBLIC_LIST = 1410,
	/** Unknown color '%s' */
	MSGTBL_CHANNEL_UNKNOWN_COLOR = 1411,
	/** You're not the owner of channel '%s' */
	MSGTBL_CHANNEL_NOT_OWNER = 1412,
	/** '%s' channel color updated to '%s' */
	MSGTBL_CHANNEL_COLOR_UPDATED = 1413,
	/** --- Available options: */
	MSGTBL_CHANNEL_AVAILABLE_OPT = 1414,
	/** -- %s create <channel name> <channel password> */
	MSGTBL_CHANNEL_CREATE_USAGE = 1415,
	/** - creates a new channel */
	MSGTBL_CHANNEL_CREATE_DESC = 1416,
	/** -- %s list */
	MSGTBL_CHANNEL_LIST_USAGE = 1417,
	/** - lists public channels */
	MSGTBL_CHANNEL_LIST_DESC = 1418,
	/** -- %s list colors */
	MSGTBL_CHANNEL_LIST_COLORS_USAGE = 1419,
	/** - lists colors available to select for custom channels */
	MSGTBL_CHANNEL_LIST_COLORS_DESC = 1420,
	/** -- %s setcolor <channel name> <color name> */
	MSGTBL_CHANNEL_SETCOLOR_USAGE = 1421,
	/** - changes <channel name> color to <color name> */
	MSGTBL_CHANNEL_SETCOLOR_DESC = 1422,
	/** -- %s leave <channel name> */
	MSGTBL_CHANNEL_LEAVE_USAGE = 1423,
	/** - leaves <channel name> */
	MSGTBL_CHANNEL_LEAVE_DESC = 1424,
	/** You're not part of the '%s' channel */
	MSGTBL_CHANNEL_NOT_PART_OF = 1425,
	/** You've left the '%s' channel */
	MSGTBL_CHANNEL_LEFT = 1426,
	/** -- %s bindto <channel name> */
	MSGTBL_CHANNEL_BINDTO_USAGE = 1427,
	/** - binds your global chat to <channel name>, making anything you type in global be sent to the channel */
	MSGTBL_CHANNEL_BINDTO_DESC = 1428,
	/** -- %s unbind */
	MSGTBL_CHANNEL_UNBIND_USAGE = 1429,
	/** - unbinds your global chat from its attached channel (if bound) */
	MSGTBL_CHANNEL_UNBIND_DESC = 1430,
	/** Your global chat is now bound to the '%s' channel */
	MSGTBL_CHANNEL_BIND_SUCCESS = 1431,
	/** Your global chat is not bound to any channel */
	MSGTBL_CHANNEL_NOT_BOUND = 1432,
	/** Your global chat is no longer bound to the '#%s' channel */
	MSGTBL_CHANNEL_UNBIND_SUCCESS = 1433,
	/** Player '%s' was not found */
	MSGTBL_CHANNEL_PLAYER_NOT_FOUND = 1434,
	// 1435-1436 used by hercules chat feature
	/** Player '%s' has now been banned from the '%s' channel */
	MSGTBL_CHANNEL_BAN_SUCCESS = 1437,
	/** You cannot join the '%s' channel because you've been banned from it */
	MSGTBL_CHANNEL_CANNOT_JOIN_BANNED = 1438,
	/** Channel '%s' has no banned players */
	MSGTBL_CHANNEL_NO_BANNED_PLAYERS = 1439,
	/** Player '%s' is not banned from this channel */
	MSGTBL_CHANNEL_PLAYER_NOT_BANNED = 1440,
	/** Player '%s' has now been unbanned from the '%s' channel */
	MSGTBL_CHANNEL_UNBAN_SUCCESS = 1441,
	/** Removed all bans from the '%s' channel */
	MSGTBL_CHANNEL_UNBAN_ALL_SUCCESS = 1442,
	/** -- '%s' ban list */
	MSGTBL_CHANNEL_BAN_LIST = 1443,
	/** - %s */
	MSGTBL_CHANNEL_BAN_LIST_ENTRY = 1444,
	/** - %s (%d) */
	MSGTBL_CHANNEL_BAN_LIST_ENTRY_VALUE = 1445,
	/** You need to input a option */
	MSGTBL_CHANNEL_OPT_NO_INPUT = 1446,
	/** '%s' is not a known channel option */
	MSGTBL_CHANNEL_UNKNOWN_OPT = 1447,
	/** -- Available options */
	MSGTBL_CHANNEL_AVAILABLE_OPT2 = 1448,
	/** option '%s' is already enabled, if you'd like to disable it type '@channel opt %s 0' */
	MSGTBL_CHANNEL_OPT_ALREADY_ENABLED = 1449,
	/** option '%s' is now enabled for channel '%s' */
	MSGTBL_CHANNEL_OPT_ENABLED = 1450,
	/** value '%d' for option '%s' is out of range (limit is 0-%d) */
	MSGTBL_CHANNEL_OPT_VALUE_RANGE = 1451,
	/** option '%s' is now enabled for channel '%s' with %d seconds */
	MSGTBL_CHANNEL_OPT_WITH_DURATION = 1452,
	/** option '%s' is now disabled for channel '%s' */
	MSGTBL_CHANNEL_OPT_DISABLED = 1453,
	/** option '%s' is not enabled on channel '%s' */
	MSGTBL_CHANNEL_OPT_NOT_ENABLED = 1454,
	/** You cannot send a message to this channel for another %d seconds. */
	MSGTBL_CHANNEL_COOLDOWN = 1455,
	/** -- %s ban <channel name> <character name> */
	MSGTBL_CHANNEL_BAN_USAGE = 1456,
	/** - bans <character name> from <channel name> channel */
	MSGTBL_CHANNEL_BAN_DESC = 1457,
	/** -- %s banlist <channel name> */
	MSGTBL_CHANNEL_BANLIST_USAGE = 1458,
	/** - lists all banned characters from <channel name> channel */
	MSGTBL_CHANNEL_BANLIST_DESC = 1459,
	/** -- %s unban <channel name> <character name> */
	MSGTBL_CHANNEL_UNBAN_USAGE = 1460,
	/** - unbans <character name> from <channel name> channel */
	MSGTBL_CHANNEL_UNBAN_DESC = 1461,
	/** -- %s setopt <channel name> <option name> <option value> */
	MSGTBL_CHANNEL_SETOPT_USAGE = 1462,
	/** - adds or removes <option name> with <option value> to <channel name> channel */
	MSGTBL_CHANNEL_SETOPT_DESC = 1463,
	/** Ban failed, it is not possible to ban this user. */
	MSGTBL_CHANNEL_BAN_FAILED = 1464,
	/** Player '%s' is already banned from this channel */
	MSGTBL_CHANNEL_PLAYER_BANNED = 1465,
	/** For '%s' you need the amount of seconds (from 0 to 10) */
	MSGTBL_CHANNEL_SECONDS_RANGE = 1466,
	/** -- %s unbanall <channel name> */
	MSGTBL_CHANNEL_UNBANALL_USAGE = 1467,
	/** - unbans everyone from <channel name> */
	MSGTBL_CHANNEL_UNBANALL = 1468,

	// @costume
	/** '%s' is not a known costume */
	MSGTBL_COSTUME_UNKNOWN = 1469,
	/** You're already with a '%s' costume, type '@costume' to remove it. */
	MSGTBL_COSTUME_ALREADY = 1470,
	/** -- %s */
	MSGTBL_COSTUME_HEADER = 1471,
	/** - Available Costumes */
	MSGTBL_COSTUME_AVAILABLE = 1472,
	/** Costume '%s' removed. */
	MSGTBL_COSTUME_REMOVED = 1473,

	// src/map/pc.c::pc_isUseitem
	/** You cannot use this item while sitting. */
	MSGTBL_NOT_USE_ITEM_SITTING = 1474,
	/** You cannot use this item while your storage is open. */
	MSGTBL_NOT_USE_ITEM_STORAGE_OPEN = 1475,

	/** You are already mounting something else. */
	MSGTBL_ALREADY_MOUNTED = 1476,

	// src/map/pc.c::pc_isUseitem
	/** Item cannot be opened when the inventory is full. */
	MSGTBL_INVENTORY_FULL = 1477,

	//@homlv
	/** Homunculus reached its maximum level of '%d'. */
	MSGTBL_HOMUNCULU_MAX_LV = 1478,

	// src/map/clif.c::clif_parse_GlobalMessage
	/** Dear angel, can you hear my voice? */
	MSGTBL_DEAR_ANGEL_MSG = 1479,
	/** I am %s Super Novice~ */
	MSGTBL_SUPER_NOVICE_MSG = 1480,
	/** Help me out~ Please~ T_T */
	MSGTBL_HELP_ME_MSG = 1481,

	// Banking
	/** You can't withdraw that much money. */
	MSGTBL_CANNOT_WITHDRAW_MONEY = 1482,
	/** Banking is disabled. */
	MSGTBL_BANKING_DISABLED = 1483,

	// src/map/atcommand.c::ACMD(auction)
	/** Auction is disabled. */
	MSGTBL_AUCTION_DISABLED = 1484,

	// src/map/clif.c::clif_change_title_ack
	/** Title is not yet earned. */
	MSGTBL_TITLE_NOT_EARNED = 1485,

	// Monster Transformation
	/** Cannot transform into a monster while in disguise. */
	MSGTBL_NOT_TRANSFORM_WHILE_DISGUISED = 1486,
	/** Character cannot be disguised while in monster form. */
	MSGTBL_NOT_DISGUISE_WHILE_TRANSFORMED = 1487,
	/** Transforming into a monster is not allowed in Guild Wars. */
	MSGTBL_TRANSFORM_NOT_ALLOWED_GW = 1488,

	// CashShop mapflag
	/** Cash Shop is disabled in this map. */
	MSGTBL_CASH_SHOP_DISABLED = 1489,

	// @autoloottype
	/** You're already autolooting this item type. */
	MSGTBL_AUTOLOOT_ALREADY_ENABLED = 1490,
	/** Item type not found. */
	MSGTBL_AUTOLOOT_TYPE_NOT_FOUND = 1491,
	/** Autolooting item type: '%s' */
	MSGTBL_AUTOLOOT_TYPE_ENABLED = 1492,
	/** You're currently not autolooting this item type. */
	MSGTBL_AUTOLOOT_NOT_ENABLED = 1493,
	/** Removed item type: '%s' from your autoloottype list. */
	MSGTBL_AUTOLOOT_TYPE_REMOVED = 1494,
	/** Your autoloottype list is empty. */
	MSGTBL_AUTOLOOT_TYPE_LIST_EMPTY = 1495,
	/** Item types on your autoloottype list: */
	MSGTBL_AUTOLOOT_TYPE_LIST = 1496,
	/** Your autoloottype list has been reset. */
	MSGTBL_AUTOLOOT_TYPE_RESET = 1497,

	// Item Bind
	/** You can't add a party bound item to a character without party! */
	MSGTBL_CANNOT_ADD_PARTY_ITEM = 1498,
	/** You can't add a guild bound item to a character without guild! */
	MSGTBL_CANNOT_ADD_GUILD_ITEM = 1499,

	// @dropall
	/** Usage: @dropall {<type>} */
	MSGTBL_DROPALL_USAGE = 1500,
	/** Type List: (default) all = -1, healing = 0, usable = 2, etc = 3, weapon = 4, armor = 5, card = 6, petegg = 7, petarmor = 8, ammo = 10, delayed-consumable = 11, cash = 18 */
	MSGTBL_DROPALL_TYPE_LIST = 1501,
	/** %d items are dropped (%d skipped)! */
	MSGTBL_DROPALL_ITEMS_DROPPED = 1502,

	// @refine - Part 2
	/** %d: Costume Headgear (Top) */
	MSGTBL_REFINE_COSTUME_HEAD_TOP = 1503,
	/** %d: Costume Headgear (Mid) */
	MSGTBL_REFINE_COSTUME_HEAD_MID = 1504,
	/** %d: Costume Headgear (Low) */
	MSGTBL_REFINE_COSTUME_HEAD_LOW = 1505,
	/** %d: Costume Garment */
	MSGTBL_REFINE_COSTUME_GARMENT = 1506,
	/** %d: Shadow Armor */
	MSGTBL_REFINE_SHADOW_ARMOR = 1507,
	/** %d: Shadow Weapon */
	MSGTBL_REFINE_SHADOW_WEAPON = 1508,
	/** %d: Shadow Shield */
	MSGTBL_REFINE_SHADOW_SHIELD = 1509,
	/** %d: Shadow Shoes */
	MSGTBL_REFINE_SHADOW_SHOES = 1510,
	/** %d: Shadow Accessory (Right) */
	MSGTBL_REFINE_SHADOW_ACC_RIGHT = 1511,
	/** %d: Shadow Accessory (Left) */
	MSGTBL_REFINE_SHADOW_ACC_LEFT = 1512,
	/** %d: Refine All Equip (General) */
	MSGTBL_REFINE_ALL_EQP_GENERAL = 1513,
	/** %d: Refine All Equip (Costume) */
	MSGTBL_REFINE_ALL_EQP_COSTUME = 1514,
	/** %d: Refine All Equip (Shadow) */
	MSGTBL_REFINE_ALL_EQP_SHADOW = 1515,

	// @reloadnpc
	/** Usage: @reloadnpc <path> {<flag>} */
	MSGTBL_RELOADNPC_USAGE = 1516,
	/** Script could not be unloaded. */
	MSGTBL_RELOADNPC_NOT_UNLOADED = 1517,

	// File name validation
	/** Not a file. */
	MSGTBL_NOT_A_FILE = 1518,
	/** Can't open file. */
	MSGTBL_CANT_OPEN_FILE = 1519,

	// Macro Interface
	/** Player %d is unavailable. */
	MSGTBL_MACRO_PLAYER_UNAVAILABLE = 1520,
	/** You are not authorized to use the macro interface. */
	MSGTBL_MACRO_NOT_AUTHORIZED = 1521,

	/** Instant monster kill state started. */
	MSGTBL_MONSTER_KILL_STARTED = 1522,
	/** Instant monster kill state ended. */
	MSGTBL_MONSTER_KILL_ENDED = 1523,

	/** Please enter a position bitmask and an amount (usage: @grade <equip position> <+/- amount>). */
	MSGTBL_GRADE_USAGE = 1524,
	/** %d: Grade All Equip (General) */
	MSGTBL_GRADE_ALL_EQUIP_GENERAL = 1525,
	/** %d: Grade All Equip (Costume) */
	MSGTBL_GRADE_ALL_EQUIP_COSTUME = 1526,
	/** %d: Grade All Equip (Shadow) */
	MSGTBL_GRADE_ALL_EQUIP_SHADOW = 1527,
	/** No item has been graded. */
	MSGTBL_NO_ITEM_GRADED = 1528,
	/** 1 item has been graded. */
	MSGTBL_ONE_ITEM_GRADED = 1529,
	/** %d items have been graded. */
	MSGTBL_ITEMS_GRADED = 1530,

	// @quest
	/** Usage: @quest <action> <quest id>. Action: */
	MSGTBL_QUEST_USAGE = 1531,
	/** -- "add" or "set": adds this quest to the player (as a new quest) */
	MSGTBL_QUEST_ADD_ACTION = 1532,
	/** -- "del" or "delete": removes this quest from player's log */
	MSGTBL_QUEST_DELETE_ACTION = 1533,
	/** -- "complete": marks this quest as complete */
	MSGTBL_QUEST_COMPLETE_ACTION = 1534,
	/** This quest does not exist. */
	MSGTBL_QUEST_NOT_EXIST = 1535,
	/** You already have this quest. */
	MSGTBL_QUEST_ALREADY_HAVE = 1536,
	/** Failed to add quest. */
	MSGTBL_QUEST_ADD_FAILED = 1537,
	/** Quest added to your log. */
	MSGTBL_QUEST_ADDED = 1538,
	/** You don't have this quest in your log. */
	MSGTBL_QUEST_NOT_IN_LOG = 1539,
	/** Failed to erase quest. */
	MSGTBL_QUEST_ERASE_FAILED = 1540,
	/** Quest was removed from your log. */
	MSGTBL_QUEST_REMOVED = 1541,
	/** Failed to complete quest. */
	MSGTBL_QUEST_COMPLETE_FAILED = 1542,
	/** Quest was marked as complete in your log. */
	MSGTBL_QUEST_COMPLETED = 1543,

	/** Please enter an enchant group id. */
	MSGTBL_ENCHANT_GROUP_ID_REQUIRED = 1544,

	/** SpecialPopup | */
	MSGTBL_MAPINFO_SPECIAL_POPUP = 1545,

// Users may compile with custom MSGTBL_MAX (e.g. for custom msg from plugins)
#ifndef MSGTBL_MAX
	MSGTBL_MAX, // Must always be the last one -- automatically updated max, used in for's to check bounds
#endif
};
