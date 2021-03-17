# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project does not adhere to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]

<!--
If you are reading this in a text editor, simply ignore this section

## [vYYYY.MM.DD] `MMMM DD YYYY`

### Added

### Changed

### Fixed

### Deprecated

### Removed
-->

## [v2021.03.08] `March 08 2021`

### Added

- Added support for preview in the cash shop. This is disabled by default and can be enabled by defining `ENABLE_CASHSHOP_PREVIEW_PATCH` or through the configure flag `--enable-cashshop-preview-patch`. A client patch is necessary, available at http://nemo.herc.ws/patches/ExtendCashShopPreview (#2944)
- Added a console warning if a message that is not present in `messages.conf` is requested. (#2958)
- Added the missing icons for `SC_DEFSET` and `SC_MDEFSET`. (#2953)
- Added the `SC_NO_RECOVER_STATE` status preventing HP/SP recovery and the related item bonus `bStateNoRecoverRace`. (#2956)

### Changed

- De-hardcoded the association between status changes and skills from the source code. A new field `Skills` is added to the `sc_config`, allowing to specify a list of skills for each status change entry. The macro `add_sc` has been removed from `status.c`, use `status->set_sc()` instead. (#2954)
- Converted packets `ZC_NOTIFY_SKILL`, `ZC_USE_SKILL`, `ZC_NOTIFY_GROUNDSKILL`, `ZC_SKILL_POSTDELAY` and `ZC_NOTIFY_SKILL_POSITION` to the structure format. (#2951)
- Converted the Homunculus database to libconfig. A tool to help converting custom databases has been provided in `tools/homundbconverter.py`. (#2941)
- De-hardcoded the list of skills that are blocked under `SC_STASIS` and `SC_KG_KAGEHUMI`. A new pair of `SkillInfo` flags `BlockedByStasis` and `BlockedByKagehumi` has been added to the skill database. (#2959)
- Updated the item script of `Velum_Flail` to its official effects. (part of #2956)

### Fixed

- Fixed compilation with mingw. (#2945)
- Fixed the CodeQL analysis builds in the CI. (#2946)
- Fixed a possible use after free in `unit_skilluse_id2()`. (#2947)
- Fixed the save point message of the Kafra in `alb2trea`. (#2950)
- Fixed some issues/regressions in the regeneration code: (#2952)
  - Fixed an issue that caused the SP regeneration rate bonus to be applied to the HP regeneration.
  - Fixed the HP/SP regeneration always capping to a minimum of 1, causing unintended behavior. (issue #2910)
  - Fixed a issue that caused the homunculus regeneration configuration to apply to elementals instead.
  - Fixed the Happy Break bonus not triggering.
  - Fixed the doridori doubled SP regeneration applying to jobs other than Super Novice.
- Fixed Emergency Call ignoring `unit_skilluse_id2()` in Renewal. (#2949)
- Fixed Manhole working on Guardians/Emperium while it shouldn't. (#2942)
- Fixed `successremovecard()` not running the cards' unequip scripts. (#2933, issue #2922)

## [v2021.02.08] `February 08 2021`

### Added

- Extended the quest database with new options. (#2874)
  - Mob ID can be set to 0 to allow any monster ID.
  - A monster level range can now be specified.
  - A target monster's map can now be specified.
  - A target monster's type (size, race and/or element) can now be specified.
- Added new quest database entries using the new options. (part of #2874)
- Added a failed assertion backtrace report in the removing player error in `unit->remove_map()`. (part of #2938)
- Added support for constants and improved error messages in the quest DB. This affects the `Mob_ID`, `Drops/ItemId` and `Drops/MobId` fields. (#2886)
- Added inheritance mechanism for the pet DB. Inheritance works in the same way as the mob and item databases, allowing to specify the `Inherit: true` flag in order to inherit (rather than overriding) an existing entry with the same Id. (#2206, issue #2181)
- Updated the map database, NPC and Hateffect constants with new data. (#2936)

### Changed

- Changed the free cell search (as used by random mob spawns or teleport) to ignore the map margins, as in official servers. The margin size defaults to the official value of 15 and can be changed by editing the `search_freecell_map_margin` setting in `misc.conf`. (#2911)
- Refactored and sanitized `map->search_freecell()`. The function has been renamed to `map->search_free_cell()` since the meaning of its return values has changed. (part of #2911)
- Refactored and documented some pet database functions and added validation of the pet DB entries before they're inserted into the database. The new constant `ITEID_PET_FOOD` has been added. (part of #2206)

### Fixed

- Fixed a signed left shift overflow in socket.c. (part of #2938)
- Fixed failing github workflows builds, switching from clang-10 to clang-11 since the former is no longer available in the Debian repositories. (part of #2938)
- Forcefully disabled the compiler flag `-fcf-protection` to avoid issues in the `setjmp()` calls. (#2938)
- Fixed some missing item IDs referenced by the quest DB in pre-re mode. (part of #2886)
- Fixed grfio issues with large grf files. (#2937)

## [v2021.01.11] `January 11 2021`

### Added

- Integrated the Renewal mob database with the correct `DamageTakenRate` value for MVPs that require it. Those MVPs have a green aura and only receive 10% of the damage dealt to them. (#2875)
- Added an enum for client action types. See `enum action_type`. (#2930)
- Added skill prerequisite checks before leveling up homunculus skills, allowing the definition of prerequisites for non-evolved or non-loyal homunculi, as it was previously limited to those. The check is disabled when the `player_skillfree` setting is enabled in the battle config. (#2807)
- Added constants for the `mercenary_delete()` script command as well as the `mercenary->delete()` function and documentation for the formerly undocumented values. See `enum merc_delete_type` and the script constants `MERC_DELETE_*`. (part of #2858, issue #2843)

### Changed

- Cleaned up mob database from redundant `MVPExp: 0` fields. (part of #2875)
- Updated packet `CZ_REQUEST_ACTNPC` to use a struct. (part of #2930)
- Changed the default mercenary delete type of the `mercenary_delete()` script command to be fired by the user (not updating loyalty). (#2843)

### Fixed

- Fixed an issue that caused the Sign quest to lock up and become unfinishable for everyone when a player times out or logs out under certain conditions. (#2921)
- Fixed an exploit that allowed multiple people into the solo room of the Sign quest. (part of #2921)
- Fixed a failed assertion when using the `MH_SUMMON_LEGION` skill. (#2929)

### Removed

- Removed the undocumented and meaningless return value of `mercenary->delete()`, now returning void. It was previously relying on the return value of two other functions, and ultimately always returning zero. (part of #2843)

## [v2020.12.14+1] `December 14 2020` `PATCH 1`

### Fixed

- Fixed a crash in `unit->run_hit()` caused by a regression in the last update. (#2924)

## [v2020.12.14] `December 14 2020`

### Added

- Added a warning when `setnpcdisplay()` or `setunitdata()` is called on a floating NPC. (#2907)
- Added support for RSW formats up to 2.5 for reading map water level. (#2916)
- Added a `status->check_skilluse_mapzone()` function, simplifying `status->check_skilluse()` and adding a useful plugin hooking point. (#2893)

### Changed

- Second part of the refactoring of the functions in `unit.c`, adding code documentation and following the code style guidelines. Functions have been renamed when backward compatible changes to the arguments or return values were made. (#2783)
  - Added proper documentation in doxygen-format.
  - Moved variables declaration closer to their first use
  - Corrected mistreatment on checks of non-boolean variables
  - Renamed some variables to clarify their use
  - Saved re-used calculations in variables or create local macros for them
  - Simplified logical checks / conditions when possible
  - Changed returning error-code in functions to obey code-style guidelines (Functions that are affected by this are renamed so that custom code fails to compile as to point at that change of behavior.)
  - Split too long lines according to code-style guidelines
  - Made functions use enums for directions (`enum unit_dir`), when dealing with a direction context
  - Reduced code-repetition by separating code-chunks into new functions
  - Fixed remaining code-style after applying all these changes if necessary
  - The following functions have been refactored:
    - `unit->walktobl()` (renamed to `unit->walk_tobl()`)
    - `unit->run_hit()`
    - `unit->run()`
    - `unit->escape()` (renamed to `unit->attempt_escape()`)
    - `unit->movepos()`
    - `unit->walk_toxy_timer()` (return value fix)
    - `unit->blown()` (renamed to `unit->push()`)
    - `path->blownpos()` (related to the `unit->blown()` change)
    - added `npc->handle_touch_events()` function
    - the `is_boss()` macro now returns a boolean
- Added static assertion checks to prevent `MAX_STORAGE` and `MAX_GUILD_STORAGE` from causing oversized inter-server packets (#2904)
- Disabled the HP bar on Emperium and MVP monsters by default. The configuration flag `show_monster_hp_bar` has been extended to a bitmask, to allow specifying different setting for normal monsters, Emperium and MVPs. The new default value is 1 (normal monsters only), while the value corresponding to the previous behavior is 7 (`4|2|1`). (#2821)

### Fixed

- Fixed a walk delay error in `unit_stop_walking()`. (part of #2783)
- Fixed homunculi and mercenaries not warping back to their master in time. (part of #2783)
- Reverted an unintended logical change in `unit->walk_toxy_timer()` done during the first part of the `unit.c` refactoring. (#2783)
- Fixed a warning (`Warning: #1681 Integer display width is deprecated and will be removed in a future release.`) when importing the database schema in recent versions of MySQL. (#2903)
- Fixed compilation with recent versions of the MySQL/MariaDB client libraries. (#2917)
- Fixed a farming exploit at the Cursed Spirit NPC. (#2883)
- Fixed the configuration flag `monster_loot_type` not working as intended. (#2855, issue #2834)

### Removed

- Removed the unused function `skill->get_linked_song_dance_id()`. (#2906)

## [v2020.11.16] `November 16 2020`

### Added

- Added a new script command `waitingroomkick()` offering better control than `kickwaitingroomall()`. (#2048)
- Added a new function `map->get_random_cell_in_range()` to get a random cell in a square around a center cell. (part of #2882)
- Added a new function `map->get_random_cell()` imitating an official cell randomization behavior, returning a cell between a minimum and a maximum distance away from the center, in a square area. (part of #2882)
- Added `pc->calc_pvprank_sub()` to the `pc` interface to allow plugins to hook into it. (#2894)

### Changed

- Updated the list of Hat Effect ID constants with new values. (#2878)
- Updated the list of NPC Sprite ID constants with new values. (#2879)
- Renamed the `PARTY_BOOKING_JOBS` and `PARTY_BOOKING_RESULTS` macros to `MAX_PARTY_BOOKING_JOBS` and `MAX_PARTY_BOOKING_RESULTS` respectively to better suit their purpose, and refactored some related code. The macros can now also be redefined externally. (#2231)
- Extended the `warpwaitingpc()` and `waitingroom()` script commands with an argument to specify which NPC to execute the commands on, for consistency with other waitingroom commands. (part of #2048)
- Refactored `map->random_dir()`, fixed a possible runtime error and added/clarified the function documentation. (#2882)
- Changed the libconfig directive `@include "FILENAME"` directive to search inside `conf/import/include/{SERVERNAME}/{FILEPATH}` before `{FILEPATH}` allowing for example to easily override `sql_connection.conf` on a server by server basis without editing other files. (#2881)
- Improved the error messages in the mob skill DB parser to include the monster ID constants instead of just the numeric IDs. (#2884)
- Corrected the return values in the null pointer checks of `mob->skill_use()` and renamed it to `mob->use_skill()` due to the changed API. (part of #2887)
- Corrected the return values in the null pointer checks of `mob->skill_event()` and renamed it to `mob->use_skill_event()` due to the changed API. (part of #2887)
- Removed an unnecessary call to `unit->set_dir()` when the direction doesn't change in the busy `unit->attack_timer_sub()`, preventing a needless `clif->changed_dir()` broadcast. (#2891)
- Added data validation to the `rodex_sendmail*()` commands to ensure that the item amount is valid. (#2901, issue #2897)
- Added data validation to the `rodex_sendmail_acc*()` commands to ensure that the receiver ID is in a valid range. (#2901, issue #2898)

### Fixed

- Added checks to the waitingroom script commands to ensure that their related waiting room and NPC exist. (part of #2048)
- Fixed the random cell selection of Confusion and Chaos according to their official behavior (random cell in a square area). (part of #2882)
- Fixed the random cell selection of `WE_CALLPARTNER`, `WE_CALLPARENT`, `WE_CALLBABY`, `AM_RESURRECTION` according to their official behavior (random cell in a quare area with minimum and maximum distance). (part of #2882)
- Fixed the damage buff effect of Watery Evasion when using Freezing Spear. (#2873)
- Fixed a missing scoreboard UI for battleground type 2. (#2849, issue #2848)
- Fixed a null pointer error in the search store function when trying to search with an empty list or with an empty card list. (#2885)
- Fixed the handling of the return value of `mob->skill_use()` and `mob->ai_sub_hard()`. (#2887)
- Fixed summoned monsters not following their master if it's a `BL_PC`. (#2888, issue #2808)
- Fixed the Impositio Manus behavior to always overwrite itself regardless of level. (#2890, issue #695)
- Fixed a possible crash when changing the display class of a floating NPC, caused by missing data validation. (#2900, issue #2899)
- Fixed the name of the `Fruit Pom Spider Card` item. (#2902, issue #2896)
- Fixed the `KO_GENWAKU` skill effect, swapping the position of the caster and the target. (#2831)

### Deprecated

- Deprecated the `HAT_DIGITAL_SPACE` constant in favor of `HAT_EF_DIGITAL_SPACE`. (part of #2878)
- Deprecated the `kickwaitingroomall()` script command in favor of `waitingroomkick()`. (part of #2048)

## [v2020.10.19] `October 19 2020`

### Added

- Added a `HERCULES_VERSION` constant with the identifier of the current Hercules release, exposed to source and script engine. (#2868)
- Added a CI script to run a `CodeQL` analysis. (#2861)

### Changed

- Removed `SC_FRIGG_SONG` from the Group A Song Skills list, since it is no longer part of it. (#2864)
- Improved, clarified and corrected the documentation comments of the `client.conf` configuration file. (#2870)
- Moved some messages to `messages.conf`. (#2866)
- Updated the `F_InsertComma()` function to work with numbers that are too large to fit in a numeric variable (such as numbers, larger than a 32 bit signed integer, returned by SQL queries). (#2860)
- Extended `getcharid()` to optionally accept a character ID as an alternative to the character name. (#2876)

### Fixed

- Fixed a failed assertion in the char server when trying to save a plagiarized skill. (#2877)
- Fixed the `KO_ZENKAI` AoE, to only trigger one of its status effects per tick. (#2863)
- Removed duplicated code from the mapflag parser function. (#2857)
- Fixed incorrect job class checks in several scripts. This fixes the special discount for assassins in the Morocc pub as well as a number of class-specific flavor text that wasn't displayed in various other scripts. (#2856)
- Fixed a wrong variable name in the `getcartinventorylist()` documentation. (#2850, part of issue #2843)
- Fixed the effect of Convex Mirror, now properly showing the position of the boss monsters on the map, if any are present. (#2862)
- Fixed the Dancer Soul Link not granting the associated Bard skills. (#2852, issue #2815)

## [v2020.09.20] `September 20 2020`

### Added

- Added a configure option `--with-maxconn=VALUE` to change the maximum number of allowed concurrent connections. (#2837)
- Added a new script command `mercenary_delete()`, to remove the mercenary from a character. (#2818)
- Added the `MDAMAGE_SIZE_SMALL_TARGET`, `MDAMAGE_SIZE_MIDIUM_TARGET`, `MDAMAGE_SIZE_LARGE_TARGET` item options to the db. (#2816)
- Added a configuration flag `features/show_attendance_window` to control whether the attendance window should pop up on login when there are unclaimed rewards. (part of #2812, issue #2101)

### Changed

- Increased the maximum number of concurrent connections to be 3072 by default when epoll is enabled. (part of #2837)
- Converted the mapcache documentation to the Markdown format and updated to reflect the way the mapcache currently works and is generated. (#2274, issue #2060)
- Improved, clarified and corrected the documentation comments of the configuration files. (#2827)
- Changed the attendance system from character bound to account bound. This includes an irreversible database migration, a backup is strongly advised. (#2812, issue #2704)
- Refactored and improved the natural HP/SP regeneration code. (#2594, cc7e1ecf0a, #2841)
  - Removed several unused variables from `struct regen_data`.
  - Renamed the members of `struct regen_data` to be more clear.
  - Changed the HP/SP heal frequency rate modifiers from a base of 1 to a base of 100 to reduce precision loss due to truncation.
  - Fixed the natural healing bonus of castle owners, to be correctly applied and removed when a castle changes ownership.
  - Extended the range of the variables in the regen data structures to int to reduce the possibility of overflows and underflows.
  - Split the `status_calc_regen` and `status_calc_regen_rate` functions into specialized functions for each affected bl type.
  - Limited various regeneration related status effects to the bl types that can be affected by them.
  - Fixed various regeneration related bonuses that were affecting the regeneration rate (time) instead of the regeneration power (amount healed).
  - Fixed the way different regeneration bonuses are stacked, to match the official behavior.
  - Added support for different and configurable regeneration rates for different bl types. This can be configured through the `elem_natural_heal_hp`, `elem_natural_heal_sp`, `hom_natural_heal_hp`, `hom_natural_heal_sp`, `merc_natural_heal_hp`, `merc_natural_heal_sp` configuration flags.
  - Fixed the `HLIF_BRAIN` and `HAMI_SKIN` regeneration bonuses to affect the regeneration rates instead of the regeneration powers.
  - Added a configurable cap to the regeneration rate. This can be configured through the `elem_natural_heal_cap`, `hom_natural_heal_cap`, `merc_natural_heal_cap`, `natural_heal_cap` configuration flags.
  - Fixed the behavior or Tension Relax when overweight, to just allow regeneration instead of increasing the regeneration rate and removed its effect on the SP regeneration.
  - Fixed the Magnificat effect in pre-renewal to only affect SP instead of both HP and SP. It was originally affecting both officially, but it was removed very early (Comodo update).
  - Made the regeneration data get recalculated on status changes that set the `SCB_REGEN` flag and added it to the 50% and 90% overweight SCs.

### Fixed

- Fixed GitLab and GitHub CI builds broken due to upstream package changes. (#2838)
- Fixed the `generate_translations` plugin on windows, not generating the output directories correctly. (#2836)
- Fixed the MDEF and DEF reduction of Odin's Power to depend on the skill level. (#2833)
- Fixed some skills (such as `MO_EXTREMITYFIST` `TK_JUMPKICK`, `SR_DRAGONCOMBO`, `SR_GATEOFHELL`) not sending the correct target type to the client when used as part of a combo. (#2829)
- Fixed typos in the documentation comments of several db files. (#2828)
- Removed duplicate lines in `cash_hair.txt` and `clif.c`. (#2840)
- Fixed an issue that prevented the "night mode" effect to be displayed. (#2839)
- Fixed a dangling pointer causing memory corruption when using `@unloadnpc` or any other way to unload NPCs. (#2835)
- Fixed a wrong IP check in the geoip code. (#2842)

## [v2020.08.23] `August 23 2020`

### Added

- Added the missing mapflag constants `MF_NOMAPCHANNELAUTOJOIN`, `MF_NOKNOCKBACK`, `MF_CVC`, `MF_SRC4INSTANCE`. (part of #2654)
- Added official item script for Rune Boots. (#2806)

### Changed

- Refactored the mapflag related code to reduce repetition. The mapflag constants exposed to the script engine `mf_*` have been changed to uppercase `MF_*` to match their source counterparts. Custom scripts may need to be updated to follow. (#2654)

### Fixed

- Corrected the >10+ refine success chance with HD ores to match the normal ores. (#2772, issue #2771)
- Corrected the duration of the Tao Gunka Scroll to be 3 minutes. (#2804)
- Updated the Flying Galapago item script to match the official version. (#2799)
- Fixed a duplicate path separator in the `item_combo_db.conf` loader console messages. (#2814)
- Fixed an exploit in the autocast system, allowing to bypass the deletion of skill requirements under certain conditions. (#2819)
- Fixed a typo that made the `MERCINFO_ID` constant unusable from the script engine. (#2817)

## [v2020.07.26] `July 26 2020`

### Added

- Added information about the Random Item Options to the `OnSellItem` array list. (#2794, part of issue #2379)
- Added a new `mf_nopet` mapflag to control pet restrictions on a map by map basis. This supersedes the `pet_disable_in_gvg` battleconf setting. (#2652)
- Added/updated packets, encryption keys and message tables for clients up to 2020-07-15. (#2788)
- Added a new pair of item bonuses `bSubDefEle` and `bMagicSubDefEle` to reduce damage (physical and magical respectively) against a specific defense element. (#2790)

### Changed

- Changed the script command `gettimetick(0)` to never return negative values. The tick loop interval is reduced from about 50 to about 25 days, but the values are guaranteed to be positive. Since `gettimetick(0)` is only intended to be used for precise calculation of short durations, it is care of the scripter to account for overflows of the counter, treating it as a 31 bit unsigned integer operating in modulo `2**31`. For most uses, `gettimetick(2)` should be preferred. (#2791, issue #2779)
- Updated the Renewal formula for the `RG_SNATCHER` skill. (#2802)
- Refactored the scripts that use `OnTouch` areas and `enablednpc()`/`disablenpc()` logic to warp characters, to use `areawarp()` instead, simplifying the scripts and resolving some possible exploits and issues. (#2798)
- Changed the `@item2` atcommand's parameters to be optional, except the item ID. Default values of 1 for the Identified and Quantity parameters and 0 for everything else will be used, when not specified. (#2795)
- Updated/added the script for items that use `bSubDefEle` and `bMagicSubDefEle`. (part of #2790, issue #548)

### Fixed

- Fixed a possible exploit in the Dokebi Battle Quest allowing to spawn a higher than expected amount of Am Muts. (#2797)
- Fixed the Dokebi Battle Quest becoming impossible to complete until server restart under certain circumstances. (part of #2797)
- Fixed the `@changecharsex` command not correctly saving the updated gender to the database. (#2796, issue #2789)
- Fixed an issue that made the Moscovia Whale Island Quest impossible to complete. (#2792, issue #2715)
- Fixed a missing cleanup of the `dnsbl` vectors on shutdown. (part of #2788)
- Fixed the experience gain messages printing a literal `%"PRIu64"` instead of the gained amount of experience. (#2647)
- Fixed several typos in the configuration files. (#2769)

### Removed

- Removed the `pet_disable_in_gvg` battleconf setting in favor of the new `mf_nopet` mapflag. (part of #2652)

## [v2020.06.28] `June 28 2020`

### Added

- Added support to display a pet's intimacy in the egg's item description window. (#2781)
- Added a convenience macro `pc_has_pet()` to check whether a character has a pet. (part of #2781)
- Added convenience macros `pc_istrading_except_npc()` and `pc_cant_act_except_npc_chat()`. (part of #2775)
- Added support for `PACKET_ZC_PERSONAL_INFOMATION`, to replace the old custom status messages about rates and penalties. (#2757)
- Added a new configuration flag `display_rate_messages` (`conf/map/battle/client.conf`) to control whether and when to display the rate modifiers to players. (part of #2757)
- Added a new configuration flag `display_config_messages` (`conf/map/battle/client.conf`) to control whether and when to display the configuration messages to players as well as which messages to display. By default, now the pet autofeed and guild urgent call setting are also displayed, along with the others. (part of #2757)
- Added a new configuration flag `send_party_options` (`conf/map/battle/client.conf`) to control whether and when to display the party option messages to players, including some cases (on login, when options are changed, when a party member is added or removed) that were previously not available. (part of #2757)
- Added a new configuration flag `display_overweight_messages` (`conf/map/battle/client.conf`) to control whether and when to display the overweight notification message to players. (part of #2757)
- Added support to display the Tip of the Day message box on login. A new configuration flag `show_tip_window` (`conf/map/battle/client.conf`) is provided, in order to disable this feature. (part of #2757)
- Added missing plugins to the makefiles. (part of #2778)
- Added missing mobs and items in the pre-re database, necessary for loading custom scripts. (part of #2778)
- Added support for GitHub actions and added builds to test different flags and compilers and different MySQL/MariaDB versions. (part of #2778 and 9b89425550)
- Added/updated packets, encryption keys and message tables for clients up to 2020-06-03. (#2763)

### Changed

- Updated the documentation of the `instance_create()` to clarify the type of ID required to create each type of instance. Notably, instances of type `IOT_CHAR` require an account ID and not a character ID. (part of #2732, issue #2326)
- Updated the instancing system so that the instance information window is also displayed on login for instances of type `IOT_CHAR`, `IOT_PARTY`, `IOT_GUILD`, even if the instance state is `INSTANCE_IDLE`. (part of #2732)
- Changed the chatroom creation and trade checks to allow dead characters to perform them. A new configuration flag `allowed_actions_when_dead` (`conf/map/battle/player.conf`) is now available, to allow neither, either or both. (#2755, issue #2740)
- Changed the behavior when a pet's intimacy drops to 0 to immediately remove the pet rather than leaving it free to roam on the map. A new configuration flag `pet_remove_immediately` (`conf/map/battle/pet.conf`) has been added, to restore the old behavior. (part of #2781)
- Centralized some repeated code related to pet spawning and consolidated the sending of the pet's intimacy and hunger information to the client when appropriate. (part of #2781)
- Extended the `guild_notice_changemap` configuration flag with more fine grained settings on when to display the guild notice. Note: if you are currently overriding this setting, you'll need to update its value with the new meaning. (part of #2757)
- Enforced the use of signed characters on platforms where `char` is unsigned. (part of #2778)
- Travis-CI scripts and configuration updates: (part of #2778)
  - Improved the build speed by reducing the clone depth to 1.
  - Improved error reporting if an error occurs during tests.
  - Added configurations targeting the arm64 and ppc64le cpu architectures as well as the gcc-10 compiler.
  - Reduced the total amount of build configurations to improve the CI build time.
  - Added execution of the servers with all the plugins enabled in order to detect memory violations and errors.
  - Fixed some build failures caused by a false positive odr violation.
  - Added execution of the map server with all the custom scripts uncommented.
  - Disabled asan in the gcc-7 builds, as it's too slow.
- Converted `validateinterfaces.py` to Python 3. (part of #2778)
- Changed the plugin handler to call all plugin events even when the server is running in minimal mode. (part of #2778)
- Updated the friend list related packets for Zero clients. (part of #2763)
- Changed the storage (account and guild storage) to automatically close when using the teleport skill. A configuration flag `teleport_close_storage` (`conf/map/battle/skill.conf`) has been added to restore the previous behavior. (#2756, issue #1762)

### Fixed

- Fixed an issue when deleting instances of type `IOT_CHAR`. (part of #2732)
- Fixed an issue that prevented the removal of offline characters from parties to get correctly saved to the database. (#2762)
- Fixed the deletion of skill units belonging to an NPC when it gets unloaded. (#2712, issue #768)
- Fixed the selection of required items for various skills, such as Slim Potion Pitcher, for skill levels greater than 2. Required items are now selected through the `skill->get_item_index()` function. (#2774)
- Fixed the description of the meaning of rows and columns in the documentation for `db/*/attr_fix.txt`. (#2765)
- Fixed the behavior of the Megaphone item script to remove the normal script restrictions (walking, attacking, using skills and items, dropping and picking up items, trading, etc) while the message input box is present and not to be cancelled on death. (#2775, issue #2751)
- Fixed a client freeze when talking with an NPC or using a Megaphone while the Rodex window is open. Rodex and NPC scripts (or megaphones) are now mutually exclusive. (part of #2775)
- Added a workaround in the CI scripts to support MySQL/MariaDB setups where the normal grant code does not work. (part of #2778)
- Fixed a memory violation between core and plugins in the HPMDataCheck code. (part of #2778)
- Fixed warnings in the skill database parser when running in minimal mode. The battle configuration is now read in minimal mode. (part of #2778, issue #2776)
- Fixed warnings about missing maps that were present in the map index and scripts. (part of #2778)
- Fixed a duplicated `fclose()` call in the mapcache plugin. (part of #2778)
- Fixed conflicting NPC names in `re/merchants/hd_refiner.txt` and in various custom scripts. (part of #2778)
- Fixed builds on ARMv8, some ARMv7 versions and PPC64. (part of #2778)
- Fixed the field size of `struct script_state::npc_item_flag` to support all the possible values and reduced the maximum value of the `item_enabled_npc` configuration flag to 3. (#2784)
- Fixed the width of the path affected by Focused Arrow Strike to be 1 cell wide instead of 2 on each side. (part of #2785)
- Fixed a missing character ID in name requests. (part of #2763)
- Fixed an issue that caused loss of items when selling items to an NPC fails because of the character zeny cap. (#2782, issue #2780)
- Fixed the disappearance of status icon timers when the character spawns. (#2786, issue #580)

### Removed

- Removed a duplicated function `time2str` from `bg_common.txt`. (part of #2778)

## [v2020.05.31+1] `May 31 2020` `PATCH 1`

### Fixed

- Fixed a crash in the db2sql plugin with the MariaDB client library. (#2748)
- Fixed the job level stat bonuses for the Novice class to match the official servers. (#2747)
- Fixed an issue that caused the walk-path check to be never executed for skills that require the caster to be able to move. (#2761)
- Fixed an issue that caused "Unknown Skill" errors to appear while casting skills. The default value for the skill damage type field of the skill database is now `NK_NONE` instead of `NK_NO_DAMAGE`. (#2761, issue #2760)

## [v2020.05.31] `May 31 2020`

### Added

- Added the possibility to declare local NPC functions as public or private. (#2142)
  - Functions declared as private can be called from other scripts with the syntax `"npc name"::function_name()`.
  - The configuration option `script_configuration.functions_private_by_default` controls whether functions are public or private when not specified.
- Added a new cast condition `MSC_MAGICATTACKED` to the mob skill database, allowing mobs to react to magical attacks. (#2733, issue #2578)
- Added support for level-specific values in the skill database fields `Hit`, `AttackType`, `InterruptCast`, `CastDefRate`, `Requirement.State`, `Unit.Id`, `Unit.Interval`, `Unit.Target`, `Requirements.Items.Amount` (part of #2731)
  - Removed hardcoded required item amounts for various skills, moving them to the skill database.
- Added support for `Requirements.Items.Any` in the skill database, allowing skills that require any one of their item conditions to be verified (as opposed to all). (part of #2731, issue #1250)
- Added support for `Requirements.Equip` in the skill database, allowing to specify a required equipment to cast a skill. (part of #2731)
  - Removed hardcoded equip requirements for various skills, moving them to the skill database.
- Added support for `Requirements.MaxSPTrigger` in the skill database, allowing to specify a maximum SP percentage that allows to cast a skill. (part of #2731)
- Added/updated packets, encryption keys and message tables for clients up to 2020-05-20. (#2713)
- Added support for the gcc sanitizer flags `address-use-after-scope`, `pointer-overflow`, `builtin`, `pointer-compare`, `pointer-subtract`, `shift-exponent`, `shift-base`, `sanitize-address-use-after-scope`. (part of #2713)
- Added support for binary and octal number literals in scripts and libconfig configuration files, using the syntax 0b000 / 0o000. (#2671)
- Added support for number separators in number literals in scripts and libconfig configuration files. The symbol `_` can be used as grouping separator (`1_000_000`, `0x_ffff_ffff`, etc). (part of #2671)
- Added support for overriding `ENABLE_SC_SAVING`, `MAX_CARTS`, `MAX_SLOTS`, `MAX_AMOUNT`, `MAX_ZENY`, `MAX_BANK_ZENY`, `MAX_FAME`, `MAX_CART` through CFLAGS. (#2220)
- Added a `skill_enabled_npc` battle flag allowing to specify whether self-targeted or targeted skills can be used while interacting with NPCs. (part of #2718, issue #862)
- Added the `loudhailer()` script command, as used by the `Megaphone_` item. (#2758, issue #2751)

### Changed

- Added validation for the maximum length of permanent string variables and consolidated it to 255 characters. This requires a database migration. (#2705, issue #1037)
- Split the `mapreg` SQL table into separate tables for integer and string variables. This requires a database migration. (#2720)
- Updated the AUTHORS file to include names and emails from every commit so far. A helper script `tools/authors.sh` to extract them has been added. (#2729, issue #245)
- Updated the drop chance of the Black and White Wing Suits. (#2739, issue #562)
- Improved validation and bounds checking in the skill database loader. (#2731)
  - Increased the maximum length of the skill descriptions (display names) to 50. (part of #2731)
  - Increased the maximum skill level to 20 (to support `NPC_RUN`). (part of #2731)
  - Capped the SkillInstances values to 25 (`MAX_SKILLUNITGROUP`) in the skill database. (part of #2731)
- Extended the `item_enabled_npc` battle flag and the `enable_items()`/`disable_items()` script commands with finer grained options to allow the use of usable items or equipment while interacting with NPCs. (#2718)
- Changed the default setting of `player_warp_keep_direction` to match the official servers' setting. (#2752)
- Replaced the (failing) gitlab-ci centos6 builds with centos8 (released in september 2019). (#2759)

### Fixed

- Fixed the logic and interaction between the (element)proof Potions and Undead Scrolls and their status icons. (#2708)
- Fixed an issue in the Lost Puppies quest causing the dogs to be unable to reappear. (#2698)
- Fixed a possible data corruption caused by gender mismatch after a changesex/changecharsex operation. (#2714, issue #985)
- Fixed a warning and name truncation when receiving a whisper message with a recipient name with a length of 24 characters. This allows to whisper to scripts with a name length up to 20 (through the `NPC:name` syntax). (#2721, issue #718)
- Fixed interaction between `itemskill()` other item/skill uses, including other `itemskill()`. The autocast code has been changed to use a vector, to allow multiple concurrent skills. (#2699)
  - It's now possible to use multiple `itemskill()` calls in the same item, as long there is at most one targeted skill, and it's the last one used.
  - It's now possible to use items while the target cursor for a previously activated skill is visible, without aborting it. (issue #816)
  - It's now possible to use a scroll while casting another skill (and the provided skill will be cast after the current one finishes). (issue #1026)
- Fixed use of Fly Wing/Butterfly Wing while riding a cash shop mount (Boarding Halter) and having a falcon. (part of #2699, issue #2750)
- Fixed the `Requirements.MaxHPTrigger` conditions for mercenary skills. (part of #2731)
- Fixed a possible memory corruption or crash in the mob delayed removal function. (part of #2713, issue #2719)
- Fixed a crash in the `npcshopdelitem()` script command. (part of #2713)
- Fixed a crash in the achievement progress update code. (part of #2713)
- Fixed a possible crash in the RODEX check name function. (part of #2713)
- Added null pointer checks for missing view data in clif and status code. (part of #2713)
- Added a check for the current map in mob and map code. (part of #2713)
- Fixed an error or possible crash when a mob dies on an invalid map. (part of #2713)
- Fixed an incorrect npc ID for the MOTD script after reloading or unloading scripts. (part of #2713)
- Fixed an use after free in `party->broken()`. (part of #7213)
- Fixed a possible crash in `mapif->guild_withdraw()`. (part of #2713)
- Fixed a null pointer error in `unit->steptimer()`. (#2723, issue #2707)
- Fixed the left-shift of a negative value in `GN_CRAZYWEED_ATK`. (#2734, issue #1151)
- Fixed a missing "Beloved" attribute on the eggs of renamed pets. (#2744, issue #2743)
- Fixed an inverted logic in the `storage_use_item` battle flag. (#2746)
- Fixed the mineffect map property flag so that it's not affected by the character's minimized effects but only by the map type. It's enabled on all the guild castles, regardless of whether WoE is running. (#2754, issue #803)
- Fixed a crash in the equip check code if a character logs wearing an item that was previously, but is no longer, equippable. (#2745)
- Fixed a compiler warnings in 32 bit builds. (#2759)

### Deprecated

- Deprecated the (unintended and undocumented) possibility of calling local NPC functions as event labels if their name started with `On`. (part of #2142, issue #2137)
  - The functionality is now disabled by default, and can be enabled by changing the `script_configuration.functions_as_events` setting.

### Removed

- Removed old debug code from the `SC_DANCING` case of `status_change_end_()`. (#2736, issue #2716)
- Removed useless `FixedCastTime` values from the pre-re skill database. (part of #2731)

## [v2020.05.03] `May 03 2020`

### Added

- Added the new pets (including the jRO exclusive ones) and their related items/monsters to the renewal database. (#2689)
- Added constants `ALL_MOBS_NONBOSS`, `ALL_MOBS_BOSS`, `ALL_MOBS` for the special mob IDs for global skill assignment in the mob skill database. (part of #2691)
- Added support for `__func__` on Windows, since it's now available in every supported compiler. (part of #2691)
- Added documentation for the mob skill database. See `doc/mob_skill_db.conf`. (#2680)
- Added missing functions for the name ack packets for `BL_ITEM` and `BL_SKILL`. (part of #2695)
- Added/updated packets and encryption keys for clients up to 2020-04-14. (#2695)
- Added support for packets `ZC_LAPINEUPGRADE_OPEN`, `CZ_LAPINEUPGRADE_CLOSE` and `ZC_LAPINEUPGRADE_RESULT` and a placeholder for `CZ_LAPINEUPGRADE_MAKE_ITEM`. (part of #2695)
- Added a new cell type `cell_noskill`, to block skill usage. (#2188)

### Changed

- Removed warning messages about missing elements in the mob db, since it's an optional field. (part of #2689)
- Updated the renewal pet database with the correct values and bonuses. (part of #2689, issue #2435)
- Changed `mob_getfriendstatus()` to consider character as friends of their support monsters, for consistency with `mob_getfriendhprate()`. (part of #2691)
- Changed `MSC_AFTERSKILL` to trigger on any skill when its `ConditionData` is 0, for consistency with `MSC_SKILLUSED`. (part of #2691)
- Improved data validation and error reporting in the mob skill database. (part of #2691)
- Changed return values of `mob_skill_use()` and `mobskill_event()`. Any third party code that uses them needs to be updated. (part of #2691)
- Changed the battle configuration flag `manner_system` to be applied immediately to any existing `SC_NOCHAT`. (#2696, issue #227)
- Changed the `atcommand()` command to ignore `PCBLOCK_COMMANDS`. (#2062)

### Fixed

- Fixed `SC_AUTOTRADE`, `SC_KSPROTECTED` and `SC_NO_SWITCH_EQUIP` incorrectly recognized as unknown status changes. (#2686, issue #2684)
- Prevented `SC_KSPROTECTED` from starting on dead monsters. (part of #2686)
- Fixed character unhiding while disguised or when using `@option 0`. (#2687, issues #1556 and #2104)
- Fixed an incorrect order of operations causing results too small in various calculations related to free cell search, mob skill chances/delays, exp penalty, pet hunger and friendly rates, cast duration. (#2690)
- Fixed errors caused by `pet_ai_sub_hard()` called for monsters that haven't been added to a map yet. (#2693)
- Fixed a wrong packet error displayed when using an incorrect password for the char-login connection. (part of #2695)
- Fixed a security check in the lapine ack packet handler. (part of #2695)
- Fixed some incorrect assumptions about the skill index values, causing the Bard/Dancer soul link to grant the wrong skills. (#2710, issue #2670)
- Fixed some conditions that could cause a skill to be attempted to save to the database with a negative skill level, resulting in query errors and data loss. (part of #2710)
- Fixed the skill type of `RK_DRAGONBREATH` and `RK_DRAGONBREATH_WATER` to be `BF_WEAPON` and support the `bLongAtkRate` bonus. (#1304)
- Fixed `SC_TELEKINESIS_INTENSE` to add percent MATK instead of fixed MATK. (part of #1304)

## [v2020.04.05] `April 05 2020` `PATCH 1`

### Fixed

- Fixed a regression that prevented pets from hatching. (#2685, issue #2683)

## [v2020.04.05] `April 05 2020`

### Added

- Added/updated packets, encryption keys and message tables for clients up to 2020-04-01. (#2663)
- The `setpcblock()` and `checkpcblock()` can now be used on another character by passing the account id. (#2668)
- Added new StatusChange types (`SC_POPECOOKIE`, `SC_VITALIZE_POTION`, `SC_SKF_MATK`, `SC_SKF_ATK`, `SC_SKF_ASPD`, `SC_SKF_CAST`, `SC_ALMIGHTY`) and updated relevant items. (#2658, related to #1177)
- Added _libbacktrace_ support (currently Linux-only) for better error call stack logging. (#2581)

### Changed

- Extended the atcommand `@fakename` with a new `options` parameter, to select which names will be displayed. (#2637, related to issue #1966 and #2168)
- Refactored the pet system code. (#2600, issues #2434 and #303)
  - Added enumerations for pet hunger/intimacy levels
  - Added value capping to `pet_set_intimate()` function.
  - Adjusted pet catch rate calculation. The old, custom, calculation can be restored by setting the `pet_catch_rate_official_formula` battle config flag to false.
  - Adjusted pet intimacy calculation when feeding.
  - Improved validation of the Pet DB fields and of the input of various pet related functions.
  - Removed the redundant `SpriteName` field from pet DB.
  - Changed `EggItem` field in pet DB to be mandatory.
  - Added new field `HungerDecrement` to pet DB. This replaces the `pet_hungry_friendly_decrease` battle config setting.
  - Added new field `Intimacy.StarvingDelay` to pet DB.
  - Added new field `Intimacy.StarvingDecrement` to pet DB.
  - Increased `MAX_MOB_DB` to 22000.
  - Added pet DB documentation file. (`doc/pet_db.txt`)
  - Removed fields from pet DB where default values can be used.
  - Added intimacy validation to pet DB `EquipScript` fields. This replaces the `pet_equip_min_friendly` battle config setting.
  - Adjusted `inter_pet_tosql()` and `inter_pet_fromsql()` functions to use prepared statements.
  - Refactored and/or updated code style of various functions that were touched by this pull request.
- Added a backtrace to the error message of `clif_unknownname_ack()`. (part of #2663)
- Added a `UNIQUE` constraint to the `userid` column of the `login` SQL table to prevent having multiple accounts with the same name. (#2666, related to #2169)
- Increased the column size of `list`for the `ipbanlist` SQL table to accomodate for non-wildcard IPv4 and for IPv6 compatiblity. (#2665, issue #2631)

### Fixed

- Fixed memory violations and incorrect handling of `npc_data` in the quest info code. (#2682)
- Fixed an issue that prevented the fake name to show up when using `@fakename` in RE clients. (part of #2637)
- Fixed a compiler error in `PACKET_ZC_SE_CASHSHOP_OPEN`. (part of #2663, issue #2669)
- Added missing libraries into the plugins Makefile, causing a linking error when a plugin uses MySQL or other libraries. (part of #2663)
- Fixed a bug causing failed assertions that appeared when attacking a skill unit (such as Ice Wall). (#2678)
- Fixed a bug causing failed assertions in `timer_do_delete()`, related to `ud->walktimer`. (#2676)
- Fixed a bug allowing to equip bullets and grenades regardless of the weapon type. (#2660, issue #2661, related to #2579)
- Fixed a memory leak in barter NPCs. (#2655)
- Fixed a pointer overflow in the script command `getiteminfo()`.  (#2656)
- Refactored and fixed several bugs in the skill auto-cast system. (#2657, issue #1211)

### Removed

- Removed the `pet_hungry_friendly_decrease` battle config setting, superseded by the `HungerDecrement` field of the Pet DB. (part of #2600)
- Removed the `pet_equip_min_friendly` battle config setting, superseded by the code inside the Pet DB `EquipScript` fields. (part of #2600)
- Removed the redundant `SpriteName` field from pet DB. (part of #2600)

## [v2020.03.08+2] `March 08 2020` `PATCH 2`

### Fixed

- Fixed an incorrect return value in `unit->walktobl()` causing mobs to get stuck when they try to loot. (#2664)

## [v2020.03.08+1] `March 08 2020` `PATCH 1`

### Fixed

- Fixed an incorrect return value in `unit->walktobl()` causing mobs to be unable to walk to their target. (#2659)

## [v2020.03.08] `March 08 2020`

### Added

- Added/updated packets, encryption keys and message tables for clients up to 2020-03-04. (#2645)
- Exposed the item bound type (`IBT_*`) constants to the script engine. (#2650)
- Added item scripts for item IDs 12459 through 12465 and corrected their name in the pre-renewal DB. (#2634, issue #1196)
- Added the `unitiswalking()` script command, to check whether an unit is walking at a given time. (#2628)

### Changed

- Changed the default `PACKETVER` to 2019-05-30. (part of #2645)
- Major refactoring of the functions in `unit.c`, adding code documentation and following the code style guidelines. Functions have been renamed when backward compatible changes to the arguments or return values were made. (#2546)
  - A new header `unitdefines.h` has been added.
  - `enum unit_dir` is now provided, to standardize handling of facing/walking directions.
  - The macros `unit_get_opposite_dir()`, `unit_is_diagonal_dir()`, `unit_is_dir_or_opposite()`, `unit_get_ccw90_dir()`, `unit_get_rnd_diagonal_dir()` have been added.
  - `unit->walktoxy_timer()` has been renamed to `unit->walk_toxy_timer()` and its return values have been changed and documented.
  - `unit->walktoxy_sub()` has been renamed to `unit->walk_toxy_sub()` and its return values have been changed and documented.
  - `unit->delay_walktoxy_timer()` has been renamed to `unit->delay_walk_toxy_timer()` and its return values have been changed and documented.
  - `unit->walktoxy()` has been renamed to `unit->walk_toxy()` and its return values have been changed and documented.
  - `unit->walktobl_sub()` has been renamed to `unit->walk_tobl_timer()` and its return values have been changed and documented.
  - `unit->setdir()` has been renamed to `unit->set_dir()` and its return value and arguments have been changed and documented.
  - `unit->getdir()` has been renamed to `unit->get_dir()` and its return type and constness of the arguments have been changed.
  - `unit->warpto_master()` has been added.
  - `unit->sleep_timer()` has been renamed to `unit->sleeptimer()` and its return values have been changed and documented.
  - `unit->calc_pos()` now accepts `enum unit_dir`.
  - `map->check_dir()` now accepts `enum unit_dir`.
  - `map->calc_dir()` now returns `enum unit_dir` and accepts a const bl.
  - `npc->create_npc()` now accepts `enum unit_dir`.
  - `skill->blown()` now accepts `enum unit_dir`.
  - `skill->brandishspear_first()` now accepts `enum unit_dir`.
  - `skill->brandishspear_dir()` now accepts `enum unit_dir`.
  - `skill->attack_blow_unknown()` now accepts `enum unit_dir`.
  - The remaining unit functions have been documented.
- New return values have been added to `pc->setpos()`, for better error handling. (#2633, issue #2632)
- Increased the `MAX_MOB_LIST_PER_MAP` value to 115 in pre-renewal builds, to fit all the default spawns. (#2638, issue #1915)
- Extended the `getiteminfo()` command to also accept item names, and added the types `ITEMINFO_ID`, `ITEMINFO_AEGISNAME`, `ITEMINFO_NAME`. (#2639)
- Changed `itemskill()` to ignore conditions by default. The `ISF_CHECKCONDITIONS` needs to be explicitly passed if conditions should be checked/consumed. (part of #2648)
- Changed the NPC shop behavior to prevent selling items from the favorites tab of the inventory. (#2651)
- Updated Doxygen configuration to speed up generation and fix compatibility warnings. (0d747896e0)
- Updated the Travis-CI configuration file according to the validation warnings and notices. (eb97973e68)

### Fixed

- Fixed a missing `get_index()` call in `Skill2SCTable`, causing some skills to activate the wrong status. (#2643, issue #2636)
- Fixed a compilation error C2233 in Visual Studio. (part of #2645)
- Fixed Basilica unintentionally restraining boss mobs. (#2612, issue #1276, related to issue #2420)
- Fixed the handling of unequip scripts in zones where an item is restricted. The `OnUnequip` script is now never executed when unequipping in a restricted zone, but it is always executed when entering such zones, regardless of the `unequip_restricted_equipment` battle flag. (#2642, issue #2180)
- Fixed the handling of skill requirements and conditions by the `itemskill()` command. (#2648, issue #2646)
- Added missing requirements to `CASH_INCAGI` and `RK_CRUSHSTRIKE`. (part of #2648)

### Removed

- Removed the `ISF_IGNORECONDITIONS` flag previously used by `itemskill()`, now the default behavior. (#2648)

## [v2020.02.09] `February 09 2020`

### Added

- Added/updated packets, encryption keys and message tables for clients up to 2020-02-06. (#2625)
- Added support for the expanded barter shop, see example script in `npc/custom/expandedbartershop.txt` and the related script commands `sellitem()`, `startsellitem()`, `sellitemcurrency()`, `endsellitem()`. Note: this includes a database migration. (part of #2625)
- Added the new script commands `cloakonnpc()` and `cloakoffnpc()`. (part of #2615)
- Added the new script command `achievement_iscompleted()` to check for achievement completion. (part of #2615)
- Added Exploration Achievement NPCs. (#2615)
- Added support for an `OnNPCUnload` label, where a script can perform its cleanup before being unloaded (such as removing mapflags, etc). (part of #2590)
- Added the Rebellion jobchange quest and its related mobs and items. (#2621)
- Added support for switching madogear type. See the documentation for `setmount()` and the related `MADO_ROBOT` and `MADO_SUITE` constants. (#2586)

### Changed

- Changed the logic of `skill_get_index` to make it easier to add new skills, custom or official. (#2596)
- Refactored `int_party` code related to exp sharing calculations. (part of #2601)
- Updated the Duel system cooldown to be configurable with seconds granularity. The `duel_time_interval` setting can be safely updated and with `@reloadbattleconf`, applying to any existing cooldowns. (#1495)
- Converted `PACKET_ZC_STATE_CHANGE` to use its struct format and added a function to send it to a single target. (part of #2615)
- Changed the behavior of `@unloadnpc`, `@reloadnpc`, `@unloadnpcfile` to kill the monsters that were spawned by the unloaded script (non-permanent spawns). (#2590, issue #2530)
- Updated the `mobdbconverter.py` tool to support Race, Size and Element constants. The script now requires Python 3. (#2620)
- Cleaned up a duplicate definition of `SP_VARCASTRATE` in `pc_readparam()`. (#2624, issue #1311)
- Extended `itemskill()` with a new `ISF_INSTANTCAST` flag to be able to cast a skill without cast time. (part of #2616)
- Extended `itemskill()` with a new `ISF_CASTONSELF` flag to be able to forcefully cast a skill on the invoking character. (part of #2616)
- Converted `ZC_AUTORUN_SKILL` to use its struct format. (part of #2616)

### Fixed

- Fixed some signed/unsigned mismatch compiler warnings. (#2601, issue #1434)
- Fixed some cases where the family exp sharing state wasn't correctly detected. (part of #2601)
- Fixed a duplicate "Skill Failed" message when using Asura Strike. (part of #2248, issue #1239)
- Fixed an issue that made a character turn to face its target after casting Asura Strike. (#2248, issue #1239)
- Fixed the position of a character after a failed Asura Strike cast, to be 3 cells from its original position instead of 2. (part of #2248, issue #1239)
- Fixed the status icon from the Elemental Resistance Potions and the Undead Element Scroll not disappearing when the status changes end. (#2614, issue #2593)
- Fixed a crash when using `@loadnpc`, `@reloadnpc` or `@unloadnpcfile` on a directory. (part of #2590, issue #1279)
- Fixed `PR_STRECOVERY` to only cure status effects if the target's element isn't Undead. (#2618, issue #2558)
- Fixed the mobs spawned by `mob_clone_spawn()` (`clone()`, `@evilclone`) being invulnerable because of an uninitialized `dmg_taken_rate` taking the value of 0. (#2617, issue #2607)
- Fixed a crash when the script function `getunits()` was called with an invalid mapindex. Now an error message is added when the mapindex validation fails. (#2619)
- Fixed a crash when `map_forcountinmap()` was called with an invalid mapindex. Now the function will return 0 instead. (part of #2619)
- Fixed clientside errors when using a `ViewData` block in the mob database without specifying a `HairStyleId`. The hair style will now default to 1 when a `ViewData` block is specified. (#2622)
- Fixed the damage calculation being too low when using weapons of type `W_RIFLE` (in pre-renewal). (#2623)
- Fixed an issue that could cause homunculi to be in an unexpected partially-vaporized state (i.e. when a Vanilmirth kills its own master through `HVAN_EXPLOSION`). (#2626)
- Fixed NPC ID validation when `SECURE_NPCTIMEOUT` is defined, causing scripts to abort with an "Unknown NPC" error on the console. (#2627, issue #2073)
- Fixed the skill condition checks and the `flag` parameter of `itemskill()`. The constants `ISF_NONE` and `ISF_IGNORECONDITIONS` are now available. (part of #2616)
- Fixed the `itemskill()` items to provide their proper Aegis behavior, in their requirement checks, self-targeting and item consumption.  (#2616, issue #819)
- Fixed the Earth Spike Scroll to consume 10 SP when `SC_EARTHSCROLL` is active. (part of #2616)
- Fixed warnings in the HPMHookGenerator. (1000b10026)

## [v2020.01.12] `January 12 2020`

### Added

- Added/updated packets, encryption keys and message tables for clients up to 2020-01-08. (#2599)
- Added support for auto exp insurance items. (#2603)
- Added the script commands `resetfeel()` and `resethate()`. (#2285)
- Added the atcommand `@hatereset`. (part of #2285)

### Changed

- Extracted packet `ZC_SE_CASHSHOP_OPEN` to a separate function. (part of #2599)
- Renamed the functions `clif_parse_CashShop*` into `clif_parse_cashShop*`. (part of #2599)
- Converted `clif_partytickack()` to use the struct version of packet `ZC_PARTY_CONFIG`. (part of #2599)
- Extended `setpcblock()` with a new `PCBLOCK_NPC` functionality to prevent players interacting with NPCs (e.g. During some npctalk dialogue). (#2606)
- Extended `warpparty()` and `warpguild()` with an option to disregard the `nowarp` and `nowarpto` mapflags. (#2604, issue #1861)
- Updated copyright header for year 2020.

### Fixed

- Fixed reading water level from RSW version 2.2 and newer. (part of #2599)
- Fixed `pc_have_item_chain()` to retrieve the chain ID from the chain cache. (part of #2603)
- Fixed an overflow in a zeny check condition in RODEX, causing code to never be executed. (#2266)
- Fixed a re-definition of HPM related symbols in plugins with multiple compilation units. (#2608)

## [v2019.12.15] `December 15 2019`

### Added

- Added/updated packets, encryption keys and message tables for clients up to 2019-12-11. (#2585)
- Added new version of packet `ZC_NOTIFY_EFFECT3`. (part of #2583)
- Added script function `specialeffectnum()`. (part of #2583)

### Changed

- Reduced the IP ban column length to 13 characters, matching the length of the inserted data. A database migration is required. (#2583, issue #2349)
- Converted packet `CZ_SE_CASHSHOP_OPEN` into a struct. (part of #2583)
- Replaced the old MySQL Connector with MariaDB C Connector 3.1.5 / Client Lib 10.4.3, for the Windows VS builds. (#2580)
- Moved the functionalities of `mob_avail.txt` to the mob database, expanding it with more fields (see the `mob_db` documentation for details). (#2572)

### Fixed

- Fixed incompatibilities with MySQL 8. (part of #2580)
- Fixed errors when `guild_skill_relog_delay` is set to 1 (reset on relog). (#2592, issue #2591)
- Fixed Tarot Card equipment breaking behavior to match the official, targeting only Left Hand (Shield), Armor and Helm. (#2589)
- Fixed racial crit bonuses not being affected by katar crit bonus. (#2588)
- Fixed interaction between Lex Aetherna and Stone/Freezing, now mutually exclusive. (#2598, issue #2559)

### Removed

- Removed `mob_avail.txt`, since its functionality has been moved to the mob database. (part of #2572)

## [v2019.11.17+1] `November 17 2019` `PATCH 1`

### Added

- Added an SQL linter. The `./tools/checksql.sh` script can be used to automatically validate the syntax of every file in the `sql-files` folder (note: dependencies might need to be installed through composer). The script is also executed in the Travis builds. (#2582)

### Fixed

- Fixed a syntax error in the `2019-10-12--14-21.sql` migration file. (part of #2582)

## [v2019.11.17] `November 17 2019`

### Added

- Added/updated packets, encryption keys and message tables for clients up to 2019-11-13. (#2568)
- Added support for packet `CZ_REQ_MOUNTOFF`. (part of #2568)
- Added a missing building entrance portal in Juno and in Lighthalzen. (#2542)
- Added the script command `getguildinfo()` and its related constants `GUILDINFO_*`, to lookup information about a guild. (#2566)
- Added a separate configuration flag in `map_log.enable` to control the logging of `LOG_TYPE_LOOT`. (part of #2560, issue #2414)
- Added a new log type, `LOG_TYPE_ACHIEVEMENT` and its configuration flag, to control the logging of achievement-granted items. A database migration is required. (#2560, issue #2414)
- De-hardcoded the boss monsters' resistance to some status effects. It's now controlled by a new `NoBoss` flag in `sc_config`. (#2570)
- De-hardcoded the combo skills chaining check. It's now controlled by a new `IsCombo` flag in `skill_db`. (#2573)
- De-hardcoded the status icons. They are now defined through a new `Icon` field in `sc_config`. (#2577)

### Changed

- Added error details to the python converter tools when a libconfig parsing error is encountered. (part of #2568)
- Converted packet `CZ_LAPINEDDUKDDAK_CLOSE` into a struct. (part of #2568)
- Updated the location of various NPCs: portals in Juno, sign post in Brasilis, Young Man in Payon (pre-renewal). (part of #2542)
- Reordered the loading of the stylist DB to be before the loading of NPC scripts, for consistence with the other DB files. (#2571)

### Fixed

- Fixed an incorrect nullpo check when slave monsters are summoned by an alchemist. (#2574, issue #2576)
- Fixed the Steal skill not showing the HP bar of the targeted monster right away but only when leaving and re-entering sight range. (part of #2567)
- Fixed a regression in the Steal skill that caused it to allow stealing of some cards. Card stealing prevention is now enforced by item type rather than by position in the drop list. (#2567)
- Fixed the `@fakename` to display the overridden name regardless of whether the character is disguised. (#2548, issue #2539)
- Fixed the `target_to` field not being cleared appropriately, causing monsters to get stuck in a loop walking to their previous target that has died, and causing hunters with auto-attack to be unable to walk away from their target and cancel their attack action. (#2564)
- Fixed the handling of HULD .po translations that contain the `\r` escape sequence. (#2569)
- Fixed the unintended clearing of status changes granted by passive guild skills, via `sc_end(SC_ALL)`. (#2575, issue #1147)

### Deprecated

- Deprecated the script command `getguildname()`, use `getguildinfo(GUILDINFO_NAME, <guild id>)` instead. (part of #2566)
- Deprecated the script command `getguildmaster()`, use `getguildinfo(GUILDINFO_MASTER_NAME, <guild id>)` instead. (part of #2566)
- Deprecated the script command `getguildmasterid()`, use `getguildinfo(GUILDINFO_MASTER_CID, <guild id>)` instead. (part of #2566)

### Removed

- Removed the `SI_*` constants from the source code, now available through `constants.conf`. (part of #2577)

## [v2019.10.20] `October 20 2019`

### Added

- Added/updated packets, encryption keys and message tables for clients up to 2019-10-02. (#2537)
- Added a new config file `conf/common/map-index.conf` to customize the location of the `map_index.txt` file. (part of #2547)

### Changed

- Moved several hardcoded messages to `messages.conf`. (#2152, issue #1282)
- Updated the `@dropall` command to correctly show the amount of dropped (and skipped) items. (#2545)
- Split the HULD generated translations into smaller (and easier to manage) files. A translation will now consist of a folder, with one .po (.pot) file per script. Third party translations may need to be updated to match this change. (#2492)
- Changed the slave monsters' behavior to react to chase the same target as their master, to match the official behavior. A configuration setting `slave_chase_masters_chasetarget` has been provided in `battle/monster.conf` for those that wish to keep using the old custom behavior. (#2561)
- De-hardcoded the path to the `db` folder, now using `map_configuration.database.db_path` and `char_configuration.database.db_path` in the map and char server respectively. This allows the user to customize the location of the db folder. (#2547)

### Fixed

- Fixed an exploitable issue in the Izlude Arena party mode script. (#2538)
- Fixed a buffer overflow in the `buildin_npcshopdelitem()`. (#2540)
- Fixed a potentially exploitable issue in the Ore Downgrade script. (#1935, issue #1934)
- Corrected the item bonus for `Drooping_Kitty_C`. (#2543)
- Corrected the display of the Sense skill to cap to 0 the negative resistance values instead of underflowing them. (#2544)
- Fixed compilation warning with gcc-9. (part of #2537)
- Fixed the HP bar of party members not showing when they unhide. (#2549)
- Fixed the status change timers not showing the correct values in the client, after relogging. This requires a database migration. (#2551, issue #2018)
- Corrected Magnum Break's 2 second delay to be an after-cast delay (reducible by Bragi's Poem) instead of a cooldown. (#2553)
- Fixed an issue that prevented players from closing their own vending shop. (#2555, issue #2554)
- Fixed the Homunculus skill requirements being applied to the master as well. (#2556)
- Fixed the Homunculus skill failure message not displaying any required items (part of #2556)
- Fixed the Chaotic Blessings skill from Vanilmirth never picking the enemy as its random target to heal. (part of #2556)
- Fixed an issue that caused the saved character data to retain the old party ID after leaving or getting kicked. (#2562)
- Fixed some possible crashes or memory corruption caused by dangling pointers to guilds in the character data. (part of #2562, related to issue #1266)
- Fixed the party name not getting removed from all affected characters (clientside) when a party is disbanded. (part of #2562)
- Fixed a crash in the console command parser when a line consisting only of spaces is executed. (#2563)
- Fixed the argument string passed to console commands when the input starts with multiple adjacent spaces. (part of #2563)
- Fixed the mapindex value not getting updated in the `gm:info` console command. (part of #2563)
- Fixed an issue that caused aggressive monsters with ranged attack to be unable to attack from above a cliff. (#2550)

### Removed

- Removed the legacy, unused, `castle_defense_rate` option from `battle/guild.conf`. (#2552)

## [v2019.09.22] `September 22 2019`

### Added

- Added/updated packets, encryption keys and message tables for clients up to 2019-09-18. (#2528)
- Added the `@changecharsex` command, to change a character's sex. (part of #2528)
- Added support for clan names in the name packets. (part of #2528)
- Added support for multiple Token of Siegfried item IDs. (#2515)
- Added support for the new guild UI features in the client. (#2519)
- Added per-item scriptable start/end rental functions, replacing the previous hardcoded functionality. See the new item DB fields `OnRentalStartScript` and `OnRentalEndScript`. (#2462, issue #140)
- Added the `getfont()` script command, to check the player's current chat font. (part of #2462)
- Added support for gcc-9 by disabling the array bound checks until the `ZEROED_BLOCK` related code will be fully compatible (#2536)
- Implemented the LapineDdukDdak System. (#2336)
- Implemented the Library Mistake Quest, allowing players to bypass the rebirth costs. (#2532)

### Changed

- Converted `sc_config` to libconfig. A tool to convert from the old format has been provided in `tools/scconfigconverter.py`. (#2526)
- Converted packet `ZC_TALKBOX_CHATCONTENTS` into a struct. (part of #2528)
- Extracted homunculus experience gain message code to a separate function. (part of #2528)
- Changed function arguments to type `enum battle_dmg_type` where applicable. (part of #2528)
- Changed pets, homunculi, etc. not to be loaded when autotrading. (part of #2524)
- Changed the guild castle IDs order to match the client's. (#part of #2519)
- Converted the item combo DB to libconfig. A tool to convert from the old format has been provided in `tools/itemcombodbconverter.py`. (#2529)
- Changed some remaining symbols to `static`. (part of #2536)
- Updated the gitlab-ci builds to reflect the release of Debian 10 buster. Gcc-8 is now the primary compiler used for the gcov, asan and i386 builds. (part of #2536)
- Increased the maximum allowed item ID to int32 max, for clients supporting it. (part of #2336)

### Fixed

- Fixed packet `ZC_ACK_RANKING` on old (2013 and earlier) clients. (part of #2528)
- Fixed an issue preventing homunculus auto-vaporize on death or skill reset, when the 80% HP condition isn't met. (#2524)
- Fixed a bug that caused homunculi's HP and SP to be refilled on every login instead of just on creation. (part of #2524)
- Fixed the intimacy requirement check for the homunculus ultimate skills. (part of #2524)
- Fixed the MVP tombstones causing players to get stuck if they were reading their message when the MVP respawns. (#2525)
- Fixed the MVP tombstones showing their message multiple times when clicked. (part of #2525)
- Fixed some incorrect examples of use of `while (select(...))` in the script documentation. (#2533)
- Corrected the item ID used by the KVM Logistic Officer. (#2527, issue #2404)
- Fixed several subtle issues caused by the nick partial match feature, when enabled. Now the partial match is only performed for lookups requested by atcommands and client features, while a full match is used for source and script lookups. (#2523)
- Rewritten the `itemdb_searchname_array` function, now properly supporting the items with IDs greater than 65535. (#2535)
- Fixed support for items with IDs greater than 65535 in the constdb2doc plugin. (part of #2535)
- Fixed a minor C standard compliance error, mixing function pointers and non-function pointers. (part of #2536)
- Fixed the (commented out by default) custom Venom Splasher countdown timer code. (part of #2536)

## [v2019.08.25] `August 25 2019`

### Added

- Added/updated packets, encryption keys and message tables for clients up to 2019-08-21. (#2517)
- Added icons for the elemental resistance status changes (`SC_ARMORPROPERTY`). (#2516)
- Added Visual Studio 2019 solution. (#2520)
- Added new NPC ID constants. (#2521)

### Changed

- Converted various packets (`ZC_ADD_SKILL`, `ZC_SKILLINFO_LIST`, `ZC_SKILLINFO_UPDATE2`) into structs and added a new version for `ZC_NPC_MARKET_PURCHASE_RESULT`. (part of #2517)
- Added missing sanity checks into many clif functions. (#2501)
- Extended the `getequiprefinerycnt()` command to accept multiple equipment slots at the same time, returning the total refine of them. (#2512)
- Added the path (relative to the Hercules root) to various database reading status messages. (#2513)
- Extended `setiteminfo()` and `getiteminfo()` with additional options: `ITEMINFO_ELV_MAX`, `ITEMINFO_DELAY`, `ITEMINFO_DROPEFFECT_MODE`, `ITEMINFO_CLASS_*`, `ITEMINFO_FLAG_*`, `ITEMINFO_STACK_*`, `ITEMINFO_ITEM_USAGE_*`, `ITEMINFO_GM_LV_TRADE_OVERRIDE`. (#2518)

### Fixed

- Fixed packets `ZC_BROADCASTING_SPECIAL_ITEM_OBTAIN` and `ZC_MAKINGITEM_LIST`. (part of #2517)
- Fixed an overflow in the auto bonus processing function, that made it unable to handle costume/shadow gears. (#2514, issues #1355, #1190, #2451)

### Removed

- Removed Visual Studio 2013 solution. (part of #2520)
- Removed round-trips to the inter-server for packets related to whisper messages, reports to GMs, GM broadcasts, party, guild and main chat, previously needed for, now unsupported, multi-zone setups. (#2522)

## [v2019.07.28] `July 28 2019`

### Added

- Added/updated packets, encryption keys and message tables for clients up to 2019-07-24. (#2498)
- Added a setting to have `@autoloot` take player drop penalties/bonuses into account. See `autoloot_adjust` in `drops.conf` for details. (#2505)
- Added a mob DB field `DamageTakenRate` to change the rate of the damage received by each monster. (#2510)
- Added a setting to allow homunculi to obtain a portion of the EXP gained by their master. See `hom_bonus_exp_from_master` in `homunc.conf`. Note: in order to restore the previous behavior, the setting can be changed to `0`. (#2507, issue #2313)

### Changed

- Converted various packets (`ZC_ACK_RANKING`, `ZC_STATUS_CHANGE_ACK`, `ZC_HAT_EFFECT`) into structs. (part of #2498)
- Changed `pc_statusup()` to send the actual stat value back to the client in case of error. (part of #2498)
- Disabled the `@refresh` and `@refreshall` commands during NPC conversations, causing the character to be stuck on an invisible NPC dialog until relogging. (#2499)
- Extended `@reloadmobdb` to update the currently spawned monsters. (#2500)
- Changed the party sharing checks to avoid unnecessary SQL queries. (part of #2502)
- Extended the `@refine` command with shortcuts to refine every normal equipment (-1), every costume equipment (-2) or every shadow equipment (-3). (#2504)
- Extended the `@refine` command to list the costume and shadow equipments. (#part of #2504)
- Increased the stack limit of Turisus, Asir and Pertz to unlimited and the other runestones to 60. (#2509)
- Extended the commands `getmonsterinfo()` and `setunitdata()` / `getunitdata()` with accessors for the new `DamageTakenRate` mob DB field, using, respectively, `MOB_DMG_TAKEN_RATE` and `UDT_DAMAGE_TAKEN_RATE`. (part of #2510)
- Converted the Guild Castle database to the libconfig format, in preparation for future updates. (#2506)

### Fixed

- Sanitized the use of `input()` in all the scripts to work correctly if the minimum accepted value is changed to less than zero in the configuration. It's recommended that third party scripts get updated not to assume a positive value, since the default setting may change in the future. (#2494)
- Fixed an issue in the Sealed Shrine script, allowing to unseal multiple Baphomets under certain conditions. (#2332)
- Fixed an issue that allowed an appropriately created party to enable experience sharing even if outside the sharing range. (#2502)
- Corrected the cooldown after killing Wounded Morroc. (#2503)
- Corrected `isequipped()` and `isequippedcnt()` to correctly handle costume equipment. (#2508)

## [v2019.06.30] `June 30 2019`

### Added

- Added/updated packets, encryption keys and message tables for clients up to 2019-06-05. (#2491)
- Added support for the new shortcuts packets in the Zero clients. (part of #2491)
- Added support for the Summoner class in `stylist.txt`. (part of #2357, issue #2356)
- Implemented the new `setfavoriteitemidx()` and `autofavoriteitem()` script commands. (#2427)
- Implemented the new `@reloadnpc` atcommand, to reload a single script file. (#2476)
- Implemented the new `identify()` and `identifyidx()` script commands and `@identifyall` atcommand. (#2487)

### Changed

- Suppressed unnecessary ShowWarning messages related to the `nosave`, `adjust_unit_duration` and `adjust_skill_damage` mapflags when using `@reloadscript`. (#2410, issue #2347)
- Updated the Rune Knight, Guillotine Cross and Ranger shops with missing items. (#2343)

### Fixed

- Fixed monster spawns disregarding the custom names specified. (#2496, #2491, issue #2495)
- Fixed the style range in `stylist.txt`, now starting from 1 instead of 0. (part of #2357, issue #2356)

## [v2019.06.02] `June 2 2019`

### Added

- Added Stat Reduction Potions to the Renewal item DB. (#2483)
- Added the constant `MAX_NPC_PER_MAP` to the script engine. (part of #2474)
- Added the `cap_value()` script command, to cap a value between a minimum and maximum. (#2472)
- Added the `mesclear()` script command, to clean an NPC message dialog without user interaction. (#2471)
- Added a script for simplified installation on Windows development machines. (#2222)
- Added/updated packets, encryption keys and message tables for clients up to 2019-05-30. (#2468, #2490)
- Added support for multiple hotkeys sets (two 'tabs' on the RE clients). The constant `MAX_HOTKEYS_DB` represents the maximum amount of hotkeys saved to the database. This requires a database migration. (part of #2468)
- Added the `delitemidx()` script command, to delete an item by its inventory index. (#2394)
- Added the `getguildonline()` script command, to return the amount of online guild members. (#2290)
- Added the `nostorage` and `nogstorage` mapflags, disallowing storage usage on the affected maps. The `bypass_nostorage` permission is also provided, to bypass those mapflags. (#2221)

### Changed

- Moved the questinfo data from map to npc data, allowing the use of multiple `questinfo()` blocks. (#2433, issue #2431)
- Removed code duplication from the map data cleanup functions. (part of #2433)
- Allow to read negative values from `input()`. The minimum value is still set to 0 in the default configuration, but it can be overridden globally by editing `input_min_value` or locally by specifying the `min` and `max` arguments to `input()`. (#2375)
- Extended the `getmapinfo()` command to return the total number of NPCs in a map (`MAPINFO_NPC_COUNT`). (#2474)
- Updated the pre-renewal Byorgue summon slave delay to match the official value, increased before renewal to prevent farming exploits. (#2456)
- Changed the `"all"` special value used by `killmonster()` to be lowercase and case sensitive, for consistency with other script commands. (#2380)
- Updated and simplified the Windows installation instructions. (part of #2222)
- Updated some NPC/name translations to match the official ones or the official intent. Cougar -> Kuuga Gai, Gaebolg -> Geoborg, Family -> Clan, Magic Gear -> Mado Gear (#2457)
- Updated the Mado Gear rental NPC to sell Mado Gear Box and Cooling Device. (part of #2457)
- Changed the `expandinventoryack()`, `expandinventoryresult()`, `expandinventory()` and `getinventorysize()` script commands to be lowercase, for consistency. (#2374)

### Fixed

- Fixed the `failedremovecards()` command, to only remove the carts when `type` is set to 1, as described in its documentation. (#2477, issue #2469)
- Fixed a crash when using `npcspeed()`, `npcwalkto()`, `npcstop()`, `unitwalk()`, `unitwarp()`, `unitstop()` on a floating NPC without a sprite. (#2430)
- Fixed a stats calculation regression. (#2482)
- Fixed a version check for the `ZC_PING` packet. (part of #2468)
- Fixed errors caused by missing Option DB and Option Drop Groups DB data when the map server loads the mob database in minimal mode. (#2486, related to issue #2484)

### Deprecated

- Deprecated use of `"All"` with `killmonster()`. Use `"all"` instead. (part of #2380)
- Deprecated the mixed case version of the `expandInventoryAck()`, `expandInventoryResult()`, `expandInventory()` and `getInventorySize()` script commands. Use the lowercase variants instead. (part of #2374)

## [v2019.05.05+4] `May 5 2019` `PATCH 4`

### Fixed

- Fixed a reading error in refine database caused refine chances to be incorrectly read. (#2481)

## [v2019.05.05+3] `May 5 2019` `PATCH 3`

### Fixed

- Fixed a calculation error in the ASPD (and/or other substats) when the maximum stats have values higher than default. (#2419)
- Fixed a parsing error in the HPM Hooks Generator. (#2467)

## [v2019.05.05+2] `May 5 2019` `PATCH 2`

### Fixed

- Fixed a packet generation issue that caused the Guild Storage to appear empty. (#2464, issue #2463)
- Fixed use of `ZC_SE_PC_BUY_CASHITEM_RESULT` on old packet versions that didn't support it. (#2465)

## [v2019.05.05+1] `May 5 2019` `PATCH 1`

### Fixed

- Fixed an issue in the player name packet causing names not to be sent correctly. (#2460, issue #2459)
- Fixed a null pointer error on MVP drops. (#2461)

## [v2019.05.05] `May 5 2019`

### Added

- Added `consolemes()` script function which allow the script engine to print error, warning, status, debug and info messages to the console. (part of #2440)
- Added the item combo effect for Geffenia Tomb of Water (2161) and La'cryma Stick (1646). (#2441, issue #1982)
- Added support for mobs to drop items with Random Options. See the new database file `db/option_drop_group.conf` and the new optional syntax in the drop entries of `mob_db.conf`. (#2309)
- Added a global function `F_MesItemInfo()`, to print an item name with description link, formatted for the current client version. (#2068)
- Added/updated packets, encryption keys and message tables for clients up to 2019-05-02. (#2432)
- Added a new function `clif_selforarea()` to send packets to self, falling back to area when no target is specified. (part of #2432)
- Added script commands `getunittitle()` and `setunittitle()`, and the related information in the `unit_data` structure. (part of #2432)
- Added support for players to automatically reject party invites through the party options. (part of #2432)
- Added an option to automatically drop the connection on the server side when a character is kicked. See `drop_connection_on_quit` in `client.conf` (note: the previous behavior was equivalent to `true`). (part of #2432)
- Added an option to force character save when the party options are changed. See `save_settings` in `map-server.conf`. (part of #2432)
- Added the script command `closeroulette()` and the related packet `ZC_ACK_CLOSE_ROULETTE` to close the roulette window. (part of #2432)
- Added a missing `CSBR_BUSY` value to `enum CASH_SHOP_BUY_RESULT`. (part of #2432)
- Added support for the refinery UI. See the new `refine_db.conf` changes and the settings `enable_refinery_ui` and `replace_refine_npcs` in `features.conf` to toggle between the new UI and the previous scripted refiner. (#2446)
- Added a new `SkillInfo` flag `HiddenTrap`, to make certain traps invisible, according to the `trap_options` configuration flag. The default settings have changed to match Renewal. Pre-renewal users might want to review `trap_options` and the now superseded `traps_setting`. (#2232, issues #1927 and #1928)

### Changed

- Extended `@dropall` to accept an optional argument for item type (#2439).
- Updated copyright header in the configuration files for year 2019. (part of #2452)
- Extended the `getinventorylist()` script command to return an array `@inventorylist_favorite`, set to true when the item is located in the favorite tab. (#2426)
- Split the function `clif_blname_ack()` into bl type-specific functions. (part of #2432)
- Extended `getunitdata()` and `setunitdata()` with support for the group ID (`UDT_GROUP`), and added the related information in the `unit_data` structure. (part of #2432)
- Moved the `UDT_*` constants from `constants.conf` to `script.c`. (part of #2432)
- Extended the guild expulsion information to include the character ID for supported client versions. This includes a database migration. (part of #2432)
- Disabled packet validation in `socket_datasync()`. (part of #2432)
- Moved the refine database and refine related functions to a new `refine.c` file. A public and private interface is provided, with public database accessors `refine->get_bonus()` and `refine->get_randombonus_max()`. (part of #2446)
- Changed several battle calculation limits to be configurable through `battle/limits.conf` and no longer hardcoded. Several `status_data` fields and battle functions now use `int` instead of `short` to accommodate this change. (#2419)
- Changed the `unitwarp()` script command to allow NPCs to be relocated to non-walkable cells (like `movenpc()`). (#2453)

### Fixed

- Corrected MSVC version naming in console (#2450).
- Corrected an example using a sprite number instead of a constant in `README.md`. (#2449)
- Fixed an issue in a monster death label callback in `npc/custom/events/mushroom_event.txt` when the monster is killed without an attached player. (#2442, issue #1955)
- Fixed an issue where when a chat room handler leaves, the following leader won't be checked for `cell_chknochat` and will bypass it. (#2443, issue #1569)
- Corrected the documentation for `pincode.enabled` in the char-server configuration. (part of #2452)
- Fixed an incorrectly displayed ITEMLINK entry in the OldGlastHeim script. (part of #2068)
- Fixed a packet size underflow in the storage packet for certain client versions. (#2424)
- Fixed an issue that caused named/brewed/forged items to be saved to database with the wrong character ID. Database migrations are provided, to update the existing data. (#2425, issue #2409)
- Fixed a truncated title in the inventory window. (part of #2432)
- Fixed a possible overflow in the guild member login field (only supporting timestamps until 2036-12-31). (part of #2432)
- Fixed a compile error with old packet versions. (part of #2432, issue #2438)
- Fixed a potential exploit related to the vending skill, by adding stricter validation on the vending status flags. (part of #2432)
- Fixed a regression, restoring the ability for HPM Hooks to hook into private interfaces, through the new macros `addHookPrePriv()` and `addHookPostPriv()`. (#2447)
- Fixed a zeny loss caused by the inter-server deleting zeny from messages when the user only requests to take items. (#2455)
- Fixed a compatibility issue with Perl 5.26 in the item converter script. (#2444)
- Fixed various gitlab-ci build failures, by removing some no longer supported debian versions and packages. The gcc-4.6, gcc-4.7 and gcc-5 builds have been removed. (#2458)

### Deprecated

- Deprecated the script command `debugmes()`, superseded by `consolemes()`. (part of #2440)

### Removed

- Removed the superseded `traps_setting` configuration flag, replaced by `trap_options`. (part of #2232)

## [v2019.04.07+1] `April 7 2019` `PATCH 1`

### Fixed

- Fixed some race conditions and missing validation in the item and zeny handling code for RODEX. (#2437)
- Fixed pet eggs getting lost for pets that were hatched before the pet evolution system. Pets are now automatically migrated to the new system that keeps eggs in the user's inventory. (#2428)

## [v2019.04.07] `April 7 2019`

### Added

- Added a configuration flag to disable the achievement system even then it's supported by the current client version. See `features/enable_achievement_system` in `battle/feature.conf`. (#2170)
- Added the `PETINFO_*` constants, to be used with `getpetinfo()`. (part of #2398)
- Added/updated packets, encryption keys and message tables for clients up to 2019-04-03. (#2406)
- Added support for the `ZC_PING` and `CZ_PING` packets. (part of #2406)
- Added support for the `CZ_COOLDOWN_RESET` packet and the related `/resetcooltime` client command. (part of #2406)
- Added support for the "allow call" player configuration option. (part of #2406)
- Added support to open the macro UI in the client. (part of #2406)
- Added support for the `CZ_STYLE_CLOSE` packet. (part of #2406)
- Exposed the `MAX_ITEM_ID` constant to the script engine. (#2367)

### Changed

- Extended `getinventorylist()` to return the item's inventory index in the `@inventorylist_idx[]` array. (#2401)
- Extended `gettimestr()` to accept an optional argument providing a UNIX timestamp, as returned for example by `getcalendartime()`. (#2388)
- Extended `getpetinfo()` to include information previously returned by `petstat()`. (#2398)
- Renamed `clif_charnameack()` to the more accurate `clif_blname_ack()`. (part of #2406)
- Extended `showscript()` to accept an optional argument to specify the send target. (#2415)
- Updated `README.md` to include links to the sections. (#2354)

### Fixed

- Fixed a compilation error in the sample plugin, on systems where `rand()` is not available. (#2403)
- Fixed a visual glitch caused by `setunitdata(UDT_LEVEL, ...)` not updating the monster level, when `show_mob_info` is configured to display it. (#2408)
- Fixed the `features/enable_pet_autofeed` configuration value that was ignored and `features/enable_homunculus_autofeed` was used instead. (#2417)
- Fixed an unescaped string in the db2sql generated data for the mob skill database. (#2416, related to #2407)
- Fixed some possible null pointer warnings reported by gcc. (part of #2406)
- Fixed a client crash with `@bodystyle` and `Job_Super_Novice_E`. (part of #2402, related to #2383)
- Fixed a client crash when using `@jobchange` to a class that doesn't support alternate body styles while a body style is applied. (#2402)

### Deprecated

- Deprecated the command `petstat()`, superseded by `getpetinfo()`. (part of #2398)
- Deprecated the `PET_*` constants, used by the `petstat()` command. (part of #2398)

## [v2019.03.10] `March 10 2019`

### Added

- Added `MOB_CLONE_START` and `MOB_CLONE_END` to the constants available to the script engine. (#2390)
- Added crash dumps to the Travis-CI output in case one of the servers crashes during the tests. (#2385)
- Added gcc-7 and gcc-8 builds to Travis-CI. (part of #2385)
- Added a configuration setting `magicrod_type` (`skill.conf`) to restore the old eAthena behavior for the Magic Rod skill. (#2034)
- Added some missing information to the documentation for `bg_create_team()` and `waitingroom2bg()`, to remove the automatic respawn. (#2381)
- Added the `MERCINFO_*` constants to the script engine, for `getmercinfo()`. (#2397)
- Added support for `MERCINFO_GID` to `getmercinfo()`. (#2397)
- Added the script commands `mobattached()` and `killmonstergid()`. (#2396)
- Added/updated packets, encryption keys and message tables for clients up to 2019-03-06. (#2377)
- Added a missing value into `enum BATTLEGROUNDS_QUEUE_ACK`. (part of #2377)

### Changed

- Changed the Windows SDK from version 10.0.15063.0 into 10.0.17763.0 for Visual Studio 2017. (#2368)
- Changed the return value of `getunitdata()` from `0` to `-1` in case the requested value couldn't be retrieved, in order to differentiate between a zero and an invalid value. Note: this may break existing scripts. (#2392)
- Updated the `getunitdata()` and `setunitdata()` documentation to clarify that the command only handles integer values. (#2391)
- Added the function `connect_client()` into the socket interface. (#2378)
- Moved the variable `SOCKET_CONF_FILENAME` to the socket interface. (#2378)
- Moved local variables from `atcommand.c` to the interface. (#2378)
- Moved defines from `map.h` to `mapdefines.h` to remove an inclusion loop. (#2378)
- Moved the stylist-related functions to their own interface. (#2400)

### Fixed

- Fixed some typos in the item bonus documentation. (#2376)
- Fixed a typo in the `setpcblock()` documentation. (c9bab97108)
- Fixed a missing return value in `F_GetTradeRestriction()`. (#2360)
- Fixed the return value of `bg_create_team()` to be -1 in case of failure, as described in the documentation. (part of #2381)
- Fixed the date field in the member list packet. (part of #2377)
- Fixed the documentation for `needed_status_point()`, not supporting the `char_id` argument. (#2399)

### Deprecated

- Deprecated the `UDT_MAPIDXY` constant. Its use in `setunitdata()` is replaced by `unitwarp()` and its use in `getunitdata()` is replaced by `getmapxy()`. (#2391)
- Deprecated the `UDT_WALKTOXY` constant. Its use in `setunitdata()` is replaced by `unitwalk()`. (#2391)

## [v2019.02.10+1] `February 10 2019` `PATCH 1`

### Fixed

- Fixed a buffer size issue in inter server packets (#2370, issue #2369)

## [v2019.02.10] `February 10 2019`

### Added

- Added/updated packets, encryption keys and message tables for clients up to 2019-01-09. (#2339)
- Added support for the barter type shops. See `sellitem()`, `NST_BARTER` and the demo scripts in `doc/sample/npc_trader_sample.txt` and `npc/custom/bartershop.txt`. (part of #2339)
- Added the `countnameditem()` script command. (#2307)
- Added/updated packets, encryption keys and message tables for clients up to 2019-01-30. (#2353)

### Changed

- Improved the response codes and error messages related to the login/char server authentication. (#2151, issue #737)
- Changed the character creation to use `FIXED_INVENTORY_SIZE` as default inventory size instead of relying on the SQL table default value. (part of #2353)
- Changed the type of several variables from `short` to `int`. (#2364)

### Fixed

- Fixed a bug that caused the custom disguise event to run indefinitely. (#2351)
- Fixed issues (item db loading, item search by name) for item IDs higher than 65535. (#2337)
- Fixed an issue while sending the last page of `HC_ACK_CHARINFO_PER_PAGE`. (part of #2339)
- Fixed the minimum duration of Voice of Siren. (#1631)
- Fixed the Sura Job Change Quest getting stuck after the first attempt. (#1656, issue #1655)
- Added support for recent MySQL versions that don't define the `my_bool` type. (#2365, issue #2363)

## [v2018.12.16+1] `December 16 2018` `PATCH 1`

### Fixed

- Added a missing check in `run_script_main()`. (#2362)

## [v2018.12.16] `December 16 2018`

### Added

- Added/updated packets, encryption keys and message tables for clients up to 2018-12-12. (#2324)
- Added support for the `AC_LOGIN_OTP` packets. (part of #2324)
- Added script command `enchantitem()` and related packet `ZC_ENCHANT_EQUIPMENT`. (part of #2324)
- Added script command `servicemessage()` and related packet `ZC_SERVICE_MESSAGE_COLOR`. (part of #2324)
- Split packets struct definitions out from lclif into separate files for AC and CA packets. (part of #2324)
- Added struct definitions header for HC packets. (part of #2324)
- Added support for expandable inventory size, including database persistence, inventory expansion packets, the script commands `expandInventoryAck()`, `expandInventoryResult()`, `expandInventory()`, `getInventorySize()`, the item `Inventory_Extension_Coupon` and the script `npc/other/inventory_expansion.txt`. (part of #2324)
- Added support for the client commands `/viewpointvalue` and `/setcamera` (like `@camerainfo`). The atcommand aliases `@setcamera` and `@viewpointvalue` are also provided. (part of #2324)
- Added `needed_status_point()` script function and the global function `F_CashReduceStat()`, for permanent status point reduction. (#2246)
- Added an option to include script interaction into the idle criteria, disabled by default. See `idletime_criteria` in `conf/map/battle/player.conf` and `BCIDLE_SCRIPT`. (#2244)

### Changed

- Extended support for 32 bit item IDs to the Zero and Main clients that support them. (part of #2324)
- Renamed packet identifier macros from `PACKET_ID_*` to `HEADER_*`. (part of #2324)
- Renamed packet struct definitions from `packet_*` to `PACKET_*`. (part of #2324)
- Increased `MAX_PACKET_LOGIN_DB` to `0x0AD0` to match the existing packets. (part of #2324)
- Added buffer size validation for `char_mmo_char_tobuf`. (part of #2324)
- Updated `npc/woe-se` files to modern standards, including `mes()` and `mesf()`. (#2261)
- Changed HPMDataCheck to exclude packetver-specific packet structs that would cause compile-time errors. (794ce3c89497a17bd64eacbc82bce22f07f00acb)

### Fixed

- Fixed `getunits()` returning the wrong value if no area size is passed. (41d370cd3308be48b4ce00a50ee46515742978b0, issue #2330)
- Fixed support for old (2010 and older) clients in packet `ZC_PROPERTY_HOMUN`. (part of #2324)
- Fixed Gaia Sword not granting any bonus drops. This reworks the way `s_add_drop` differentiates between items and groups and the parameters passed to `pc_bonus_item_drop()`. Custom code may need to be updated to match. (#2327)
- Fixed a 'Gungslinger' typo in `item_db2`. (#2335)
- Fixed delay-consumed items missing consumption after using Abracadabra/Improvised Song (#2298, issue #1169)

### Removed

- Removed unnecessary typedef from `clr_type`. The type is now only available as `enum clr_type`. (part of #2324)

## [v2018.11.18+1] `November 18 2018` `PATCH 1`

### Fixed

- Fixed a regression in #1215 that prevented characters from being resurrected. (c34871cb3f65412db663a4793df6f055663e16fa)

## [v2018.11.18] `November 18 2018`

### Added

- Added an option to prevent character renames when in a guild or a party. This is the official behavior, but disabled by default since it's unnecessary in Hercules. To enable, `char_configuration/use_aegis_rename` can be set to `true` in `char-server.conf`. (#1866, issue #1805)
- Added the script command `data_to_string()`, to return the string representation of the given data (counterpart to `getd()`). (#2304)
- Added/updated packets, encryption keys and message tables for clients up to 2018-11-14. (#2310, #2321)
- Added a configuration option for the maximum delay allowed by `@channel setopt MessageDelay` command, see `chsys/channel_opt_msg_delay` in `channels.conf`. (#2287)
- Added a configuration option for more accurate emulation of the HP display on dead characters. This setting is enabled by default, but the old, less confusing, behavior is available by setting `display_fake_hp_when_dead` to `false` in `client.conf`. (#1215, issues #889 and #840)
- Added `common/packets`, to provide a common interface to the packet DB, shared by all three servers. (#2321)
- Added proof of concept of compile time validation of packet struct definition against the packet lengths DB (currently only for `ZC_ITEM_PREVIEW`). (part of #2321)
- Added runtime client-server packet length validation in `RFIFOSKIP()` and `WFIFOSET()`. An unchecked `WFIFOSET2()` alternative is still provided, for packets that lack a db entry. (part of #2321)
- Added runtime validation for the size of `WIFOHEAD()` buffer allocations. (part of #2321)

### Changed

- Made `getunits()` stop unnecessarily iterating when the maximum specified amount of units is reached. (#2105)
- Updated the item bonus documentation, converted to the Markdown format in `doc/item_bonus.md`. (#2259)
- Improved the channel delay message to include the remaining time before a new message can be sent. (#2286)
- Removed the unused `type` argument from `getnpcid()`. All the shipped scripts have been updated. (#2289)
- Improved the `charlog` to include the stats, class, hair color and style whenever available. This affects the character selection and rename log entries, that had most fields zeroed before. (#2320)
- Updated the quest variables documentation, converted to the Markdown format in `doc/quest_variables.md`. (#2256)
- Updated the monster modes documentation, converted to the Markdown format in `doc/mob_db_mode_list.md`. (#2255)
- Documented `flag` of the `status->heal()` function through the `enum status_heal_flag`. (part of #1215)
- Added packet versions for all server types to the socket datasync validation. (part of #2321)

### Fixed

- Fixed packet `ZC_FORMATSTRING_MSG_COLOR` for clients older than 20160406. (part of #2310)
- Fixed the output formatting in the unhandled packet reporter. (part of #2310)
- Improved grammar in the `CONTRIBUTING.md` document. (#2303)
- Fixed some logically dead code in `status.c`. (#2265)
- Fixed some alerts reported by LGTM for the python scripts. (#2268)
- Fixed some typos in the `README.md` document. (#2283)
- Fixed a crash and/or mapflag inconsistency when reloading scripts, especially in PK servers. Map zones are now removed correctly during a reload. (#2247, issue #2242)
- Fixed an overflow in the defense calculation when fighting more than 22 enemies. (#1233, issue #1201)
- Fixed the gitlab-ci builds with clang-4.0, which was removed from Debian testing/unstable. (#2323)
- Fixed an issue that caused a character leaving their guild to still receive some `#ally` messages from certain allied guilds. (#2322)
- Fixed an issue that caused a character joining a guild not to join its `#ally` channel group. (part of #2322)
- Fixed a duplicated line in the `@channel` help output. (part of #2322)
- Fixed some code that assumed a character to be already on a map and attempt to leave an invalid `#map` channel when logging in. (part of #2322)

### Removed

- Removed support for the `MINICORE` libraries, which were unused but needlessly built since the new mapcache system was implemented in #1552. (#2319)
- Removed the unused `src/tool` directory. (part of #2319)

## [v2018.10.21] `October 21 2018`

### Added

- Unknown packets are now printed to the console, when the option to dump them to disk (`DUMP_UNKNOWN_PACKET`) is disabled. (part of #2226)
- Added/updated packets, encryption keys and message tables for clients up to 2018-10-02 (#2226)
- Implemented the script command `removespecialeffect()`. (part of #2226)
- Implemented the atcommand `@camerainfo` and the script commands `camerainfo()` and `changecamera()`. (part of #2226)
- Added options to enforce a minimum buy/sell price for NPC items, defaulting to the official values of 1 and 0, respectively. (#2208, issue #2177)
- Added documentation for the script command `achievement_progress()`. (#2249)
- Added/updated packets, encryption keys and message tables for clients up to 2018-10-17 (#2278)
- Implemented script command `itempreview()`. (part of #2278)
- Added placeholders for 493 items from kRO. (#2280)

### Changed

- Converted the effect list documentation to Markdown (`effect_list.md`). (#2230, issue #2215)
- Improved the GitHub pull request and issue templates. (#2237)
- Allowed `getd()` to work with constants and params (although this is a discouraged practice). (#2240)
- Converted the permissions documentation to Markdown (`permissions.md`). (#2253)
- Extended `getiteminfo()` and `setitmeinfo()` with the trade restriction information (`ITEMINFO_TRADE`). The `ITR_*` constants are made available to the script engine, and the global function `F_GetTradeRestriction()` has been provided, for convenience. (#2172)
- Converted the global configuration documentation to Markdown (`global_configuration.md`). (#2229, issue #2216)
- Removed duplicated code from the `showevent()` icon validation. (#2250)
- Extended `setquestinfo()` with a mercenary class option (`QINFO_MERCENARY_CLASS`). (#2251)
- Removed duplicated/diverging code from the `@bodystyle` command. (#2264)
- Removed duplicated code for `prompt()`, now sharing the same function as `select()`. The new constants `MAX_MENU_OPTIONS` and `MAX_MENU_LENGTH` have been provided. (#2279)

### Fixed

- Fixed an assertion failure in the zeny achievement, when the amount of zeny is zero. (#2227)
- Fixed issues when setting a char or account variable of another player. (#2238, issue #2212)
- Fixed a failed assertion when a character is invited to and joins a guild. (#2235, issue #2210)
- Fixed a failed assertion when `sc_end()` is called for `SC_BERSERK`. (#2239, issue #1388)
- Fixed display issues with homunculus in old clients. (#2252)
- Fixed damage reflection (through Reflect Shield, High Orc Card, etc) to work on traps. The old, unofficial, behavior can be restored through the battle configuration flag `trap_reflect`. (#2182, issue #1926)
- Fixed Blast Mine and Claymore Trap damage, that wasn't getting split by the number of targets. (#2182, issue #1900)
- Fixed an assertion failure when refining an item fails. (#2234, issue #2217)
- Fixed the gitlab-ci builds with clang-5.0, which was removed from Debian testing/unstable. (58afe047cd)

## [v2018.09.23] `September 23 2018`

### Added

- Added LGTM.com code quality badges to the README. (#2202)
- Added maps and constants related to episode 17.1 to the map database and constants list. (#2203)
- Added/updated packets, encryption keys and message tables for clients up to 2018-09-19 (#2199)
- Added `-Wvla` to the compiler flags, to prevent accidental usage of variable length arrays. (#2199)
- Implemented `PACKET_CZ_MEMORIALDUNGEON_COMMAND`. (#2195)
- Extended `setquestinfo()` with support for item amount ranges. (#2218)
- Implemented the Mob Skill DB generator into the `db2sql` plugin. (#2149)

### Changed

- Changed the Travis build to use the maximum available PACKETVER, so that the recent code is tested. (#2199)
- Added a shorthand to call `mes()` without arguments to mean `mes("")`. (#2193)

### Fixed

- Added a missing `IF NOT EXISTS` clause in the `char_achievements` table creation query. (e71e41b36b)
- Fixed an issue in packet `ZC_INVENTIRY_MOVE_FAILED` (#2199, issue #2213)
- Fixed a validation error in `setquestinfo()`. (#2218)
- Fixed an error in the achievement system, when killing a cloned mob. (#2204, issue #2201)
- Fixed a truncation issue in the card columns of the database. (#2205, issue #2187)
- Fixed a crash when a character is removed from the `char` table but not from the `guild_member`. (#2209, issue #2173)

## [v2018.08.26+1] `August 29 2018`

### Fixed

- Fixed a bug which prevented the script engine from updating params. (#2200, e554c1c9c)

## [v2018.08.26] `August 26 2018`

### Added

- Added the `@setzone` command, which allows changing the zone of the current map on the fly. (#2162)
- Added/updated packets, encryption keys, and message tables for clients up to 2018-08-08. (#2176)
- Added support for `sak` and `ad` clients. (#2185)
- Made the server display the client type (`main`, `RE`, `zero`, `sak`, `ad`) on startup. (#2185)
- Added const-correct wrappers for `strchr()`, `strrchr()`, `strstr()`, when building with C11-compatible compilers. (#2189)

### Changed

- Made the map zone db also reload when `@reloadscript` is used. (#2162)
- Updated the `is_function` script command to support built-in commands, local functions, and local subroutines. (#2154)
- Updated the `debugmes` script command to support printf format strings. (#2146)
- Changed the language specification to `C11` in autoconf builds. (#2189)

### Deprecated

- `script->add_str()` should no longer be used by plugins to inject variables, as `script->add_variable()` supersedes it. (#2164)

### Fixed

- Fixed the `@mapflag` command not working with the `town` mapflag. (#2133, #2162)
- Fixed some issues with the banking and roulette packets. (#2190)
- Fixed the compiler throwing a warning when `MAGIC_REFLECTION_TYPE` is set to `0`. (#1920, 2175)
- Fixed some vague item bonus documentation for `bHealPower` and `bHealPower2`. (#2125)
- Fixed some issues in the GitLab CI CentOS builds that prevented the pipelines from succeeding. (#2191)
- Reverted [v2018.07.29+1] and fixed the underlying issue, which caused some script variables to end up with an incorrect type. (#2164)
- Fixed some constants that had an invalid type, which caused undefined behaviour with `getdatatype`. (#1801, #2164)
- Fixed zeny spending achievements recording the zeny amount in negative values. (#2171)

## [v2018.07.29+2] `August 1 2018` `PATCH 2`

### Fixed

- Fixed a wrong preprocessor directive that prevented some clients from connecting. (#2165, #2166)

## [v2018.07.29+1] `Jul 30 2018` `PATCH 1`

### Fixed

- Added a temporary patch for `getd()` when variable types are `C_NOP`. (#2163)

## [v2018.07.29] `Jul 29 2018`

### Added

- Added support for the Achievements system and the Titles system. (#2067, #2157, #2161)
- Added a stylist db option to restrict some hairstyles for the Doram race. (#2155)
- Added/updated packets, encryption keys, and message tables for clients up to 2018-07-18. (#2139, #2126, #2132)
- Added support for the overweight percentage packet for clients older than 2017-10-25. (#2139)
- Added support for the chat commands `/changedress` and `/nocosplay`. (#2139)
- Added support for the party member death notification packet for clients older than 2017-05-02. (#2139)
- Added support for body style to the stylist. (#2138)
- Added a `dead_area_size` battle config option to configure the area to which player death packets are sent. (#2088)
- Added project files for Microsoft Visual Studio 2017 (#2131)

### Changed

- Updated the warp list packet for clients older than 2017-04-19. (#2139)
- Simplified the `questinfo()` script command and added `setquestinfo()`. This may break some scripts, but is easily fixable with a regular expression search/replace. (#2107)
- The constants database will now also be reloaded when calling `@reloadscript`. (#2130)
- The maximum item ID for the item database is now `65535` for clients older than 2018-07-04, and `131072` for newer clients. It may be increased up to a maximum of `2147483648` in the future, as needed. (#2134)
- Added the missing `pos` parameter to `skill_init_unit_layout_unknown()`. (#2143)

### Deprecated

- Microsoft Visual Studio 2012 is no longer officially supported. (#2131)

### Removed

- Removed the `EF_ANGEL3` effect from the novice academy, as it is now triggered by the Achievements system. (#2156)

### Fixed

- Fixed a bug which made the `Venom Splasher` skill consume gemstones twice. (#2148)
- Fixed a bug that could make skill cooldowns never expire, rendering the skill unusable. (#2147)
- Fixed the maximum array size being higher than the maximum integer (uint32 vs int32), which could cause integer overflows in scripts. (#2093)
- Fixed a wrongly named constant, which made `Sea-Otter Card` not increase the `Sushi` heal rate. (#2117)
- Fixed misc bugs related to pet evolution. (#2136, #2153)
- Fixed a bug that sent an attendance system message without the attendance ui being opened. (#2129)
- Corrected several outdated documentation references to `db/constants.conf`, to point to `doc/constants.md`. (#2090)
- Fixed an issue in the script command `getd()` that wouldn't properly initialize the type of newly created variables through `set(getd(...), ...)` (#2158)
- Fixed a missing memory initialization in several dummy `struct block_list` entries created as local variables. (#2159)

## [v2018.07.01+1] `Jul 1 2018` `PATCH 1`

### Fixed

- Fixed a regression that made it impossible to invite guild members. (#2124, issue #2122)

## [v2018.07.01] `Jul 1 2018`

### Added

- Added/updated packets and message tables for clients from 2018-05-30 to 2018-06-12. (#2064)
- Added/updated a pair of enums (`cz_ui_types`, `zc_ui_types`) for the values used by packets 0xa68 and 0x0ae2, fixed compatibility with older clients. (part of #2064)
- Added the possibility for a plugin to abort the skill currently being cast by returning true from `skill_check_condition_castend_unknown()`. (#2076)
- Implemented Pet Evolution. This adds a new `Evolve` field to the Pet DB, and a generator script is provided, to automatically create entries from the pet evolution lua. (#2063)
- Implemented Pet Autofeeding. This adds a new `AutoFeed` field to the Pet DB, and can be completely disabled through the `enabe_pet_autofeed` flag in `feature.conf`. (#2063)
- Added the script command `setparam()`, the setter counterpart of `readparam()`, accepting an optional account id argument. (#2081)
- Updated maps database/mapcache with new maps (part of #2098)
- Added/updated packets and message tables for clients from 2018-06-20 to 2018-06-27. (#2095)
- Added new map/mapserver-change packets for the airship system. (part of #2095)
- Added new (unused) 'dns host' field in `char_send_map_info()`, compatible with clients newer than 2017-03-29. (part of #2095)
- Added an option to hide names in the script commands `unittalk()` and `npctalk()` (#1831, formerly #1571, issue #1523)

### Changed

- Updated README with more info about the development dependencies. (b57232ac29)
- Updated `instance_create()` when trying to create an already existing instance, to match the official behavior. (#1924, issue #1651)
- Removed the `RTLD_DEEPBIND` flag from the plugin-loading functions, for compatibility with asan in gcc-8. (#2079)
- Standardized the function call syntax in the script command documentation. (#2084)
- Changed the way pet eggs are handles in the inventory (they don't get deleted when hatched), for compatibility with the pet evolution system. (part of #2063)
- Extended the script command `readparam()` with the ability to receive an account id as optional argument, as an alternative to the character name. (#part of 2081)
- Updated the Private Airship map list to match the main kRO servers, and enabled the functionality on the main packetvers. (#2098)
- Updated pin code status packets for the 2018 clients. (part of #2095)
- Updated the authentication error packets in the map server to use the most recent version for the current packetver. (part of #2095)
- Updated the roulette packets for the 2018 clients. (part of #2095)
- Updated GitLab-CI builds to include more recent compilers and platforms: clang-5.0, clang-6.0, clang-7, gcc-7, gcc-8 ubuntu 18.04, MariaDB 10.1 are now tested. (part of #2111)
- Updated GitLab-CI builds to include builds with a recent Zero packetver, to ensure that recent code is compiled/tested. (part of #2111)
- Split the function `clif_disp_overhead` into two and converted its packet handling into the struct format. (part of #1831)
- Cleaned up the mapif-related code, splitting `mapif` packet processing from `inter` logic and moving the `mapif` functions to `mapif.c`. (#2108)
- Changed all the functions (where possible) to have static linking, in order to prevent incorrect symbols to be used by plugins, as a safer alternative to `RTLD_DEEPBIND`. Plugin authors are still advised to avoid naming their local symbols with the same name as non-static symbols exported by the code. (#2112)
- Prevented compilation of the non-memmgr memory management function wrappers when the memory manager is enabled. (part of #2112)

### Fixed

- Fixed a crash when entries from the `job_db` are removed. (#2071, issue #2070)
- Fixed `getunits()` to always return a value, even in case of error. (d2c0e453fc)
- Fixed an incorrect response message in the stylist shop. (#2066, issue #2065)
- Fixed an issue in the `clif_parse_OpenVending()` processing when the item list is empty. (#2072)
- Fixed various typos in code documentation/comments. (#2069)
- Fixed a field size in the character creation packet. (part of #2064)
- Added some missing fields in the `AC_ACCEPT_LOGIN` packet structure. (part of #2064)
- Fixed compilation with packetvers older than 20090805. (part of #2064)
- Fixed the `rodex_sendmail_acc()` command to correctly use the `account_id` field as stated in the documentation. (#2075, issue #2024)
- Fixed the shutdown callback calls, that weren't getting called any longer since core.c was interfaced. (#2106)
- Fixed a parsing error when the pre-increment/pre-decrement operator is used in a conditional's body without braces. (#2077, issues #705, #912, #1553, #1710)
- Fixed `SC_NOEQUIPWEAPON`/`RG_STRIPWEAPON` (Renewal only) and `SC_INCATKRATE`/`NPC_POWERUP` whose ATK increment/reduction were ineffective on monsters. (#2097)
- Fixed an error when a player's PIN code is set for the first time. (#2100, issue #2046)
- Fixed the searchstore packet for compatibility with item options. (part of #2095)
- Fixed GitLab-CI build failures caused by MySQL client versions incompatible with the updated docker images. (#2111)
- Fixed the novending map/cell flag that would cause players to get stuck. (#2091, issue #662)
- Fixed the documentation for the `queueopt()` script command. (#2086)
- Fixed documentation comments related to the `exp_group_db` (#2114, #2115)
- Fixed an issue in the travis builds when the console error output is too long. (part of #2112)

## [v2018.06.03] `Jun 3 2018`

### Added

- Added/updated packets support for clients from 2018-05-09 to 2018-05-23. (#2043)
- Added client/version-specific `enum clif_messages` values for msgstringtable message IDs. All the related functions have been updated. (#2038)
- Added the script commands `setpcblock()` and `checkpcblock()`, to prevent various character actions - see the script command docs for details. (#842)
- Implemented the Stylist UI, available in clients starting from 2015. Configurable in `stylist_db.conf`, accessible to scripts through `openstylist()`. (#2004)

### Changed

- Extended the script command `getunits()` with support to look up units globally, making the map argument optional. (#1851)
- Updated copyright headers to year 2018. (#2054)
- Converted `exp.txt` (now `exp_group_db.conf`) to the libconfig format, now better integrated with `job_db.conf`. (#2036, originally #1944)

### Fixed

- Fixed an issue in the mob skill db parser that limited the mob skills to a maximum of 5 (#2042, issue #2044)
- Fixed some incorrect msgstringtable IDs. (part of #2038)
- Fixed inheritance in the mob DB, no longer overwriting the Range field with a default value. (#2055)
- Fixed the skill element getter for levels above `MAX_SKILL_LEVEL`. (#2059)
- Fixed interaction between the `pvp_nocalcrank` mapflag and the script/atcommands to toggle PvP. (#2057, issue #2056)

### Deprecated

- While not officially deprecated yet, use of `maprespawnguildid()` and `playbgmall()` has been superseded by `getunits()`. (part of #1851)
- Deprecated the `pcblockmove()` script command. Use the more flexible `setpcblock()` instead. (part pf #842)

## [v2018.05.06] `May 6 2018`

### Added

- Added a configurable PIN code blacklist, to prevent use of certain codes. (#2007 and #2029, issue #769)
- Added/updated packets support for clients from 2018-04-11 to 2018-05-02. (#2021, #2030)
- Implemented option to allow guild skill cooldowns to continue when the leader is logged out. Enabled by default and controlled by the `guild_skill_relog_delay` flag in guild.conf. (#2005, issue #1774)
- Implemented the Private Airship system, currently using the list of maps from the Zero server. (#1998)
- Added the constants `DEFAULT_MOB_JNAME` and `DEFAULT_MOB_NAME` (source only) to replace hardcoded use of `"--ja--"` and `"--en--"` respectively. (part of #2027)

### Changed

- Replaced custom messages related to the PIN code system with the official ones. (part of #2007)
- Updated the minimum client version that enables certain features: new drop packet now in `PACKETVER >= 20180418`, attendance system now in `PACKETVER_ZERO_NUM >= 20180411`. (#2020)
- Introduced a friendly error message when the `delwall()` script command fails due to a non-existent wall. (#2017)
- Refactored some code to move `MAPID_*` related code into separate functions. (#2022)
- Changed the plugins Makefile to honor the `$MYPLUGIN` variable passed through the environment, to make it easier to compile specific plugins without editing files. (#2025)
- Converted the Mob Skill DB to the libconfig format. A converter script (`mobskilldbconverter.py`) has been provided for convenience. (#2019)

### Fixed

- Fixed interaction between Curse and Blessing. When under Curse or Stone Curse, Blessing will only remove the negative statuses and will need to be cast again to obtain the buff. (#1706, issue #680)
- Added support for `time_t` as return type in the HPMHookGen. (bb0e228bd29dd689ca76f64578de8759415a763b)
- Fixed some possible buffer overflows. (#2028)
- Fixed the return value of `BUILDIN(getunitdata)`. (d6785d389cbee4f34078f6762626ca61b2d6cc25)
- Improved support for clients version 2018-02-07 and Zero 2018-01-31. (part of #2030)
- Fixed the clan names in some clan-related NPC dialogs. (#2032)
- Fixed the display name of monster summoned through the `SA_SUMMONMONSTER` skill. (#2027)

### Removed

- Removed all the code related to the anonymous-stat-reporting system. (#2023)

## [v2018.04.08] `April 8 2018`

### Added

- Added/updated packets support for clients from 2018-03-14 to 2018-04-04. (#1994 and #2014)
- Introduced macros `PACKETVER_RE_NUM`, `PACKETVER_ZERO_NUM` and `PACKETVER_MAIN_NUM` to simplify client type-specific version checks.
  These macros are defined to `PACKETVER` only if, respectively, `PACKETVER_RE`, `PACKETVER_ZERO` or neither are defined. (part of #1994)
- Implemented Hat Effects, available in clients starting from 2015-04-22. (#1965)
  - The `hateffect()` script command has been implemented.
  - The related constants (with prefix `HAT_EF_*`) have been added and made available to the script engine.
- Added the 2015 variant of the quest-related packets. (#1111)
- Added login date information for guild members, on clients starting from 2016-10-26. The message format can be customized on the client side, by editing line 3012 of msgstringtable.txt. (#1986)
- Added support for the `ZC_FORMATSTRING_MSG` and `ZC_MSG_COLOR` packets, handling msgstringtable messages. (#2012)
- Added a setting (`storage_use_item` in `items.conf`) to control the use of items (usable/consumable/boxes) when the storage is open. (#1868, issue #1806)
- Implemented the Attendance System, requiring client 2018-03-07bRagexeRE or newer. Configuration is available in `feature.conf` and `db/attendance_db.conf`. (#1990)
- Added a configurable delay to the MVP Tombstone. The delay can be configured through the `mvp_tomb_spawn_delay` setting in `monster.conf`. (#2001, issue #1980)

### Changed

- Updated the functions handling quest-related packets to use the struct-based form. (part of #1111)
- Converted the Pet DB to the libconfig format. A converter script (`petdbconverter.py`) has been provided for convenience. (#2000)
- The `noteleport` mapflag has been added to the Archer Village (`pay_arche`), to match official servers. (part of #2006)
- The `script->sprintf()` function has been renamed to `script->sprintf_helper()`. (part of #2009)

### Fixed

- Removed a duplicated line in the login server VS project that would prevent Visual Studio from loading it. (#1992)
- Prevented a console warning when a nonexistent map is passed to the `getmapinfo()` script command. (584e8de35)
- Fixed a RODEX loading data problem when a message's expiration date was manually edited. (#1995)
- Corrected the error messages displayed when using various restricted items to match the official servers. (#2006)
- Added a missing status refresh for the Homunculus Autofeed system when changing maps. (#2002)
- Fixed a NULL pointer check failure when `TK_JUMPKICK` is used by a non-player. (#2015, issue #1875)
- Fixed compilation of the HPMHooking plugin on systems where `sprintf()` is a macro. (#2009, issue #2003)

## [v2018.03.11] `March 11 2018`

### Added

- Added a new `mapcache` plugin to convert, update or recreate mapcache files in the new format. (part of #1552)
- Added appveyor configuration to the repository. (part of #1552)
- Exposed `script->sprintf()` to plugins. (#1976)
- Added/updated packets support for clients from 2018-02-21 to 2018-03-09. (#1983)

### Changed

- Updated the mapcache to a new, git-friendly, format having one file per map. (#1552, #1981)
  - For info on how to convert or recreate mapcache entries, see the mapcache plugin (`./map-server --load-plugin mapcache --help`)
- Removed the display of PIN codes and passwords from the `@accinfo` GM command. Old code is kept commented out for those that may wish to re-enable it. (#1975)
- Updated `README.md` with some clarifications and corrections. (#1985)

### Fixed

- Updated the VS project files with the recently added .h files, for better intellisense/search. (#1970)
- Fixed a NULL pointer in `login->accounts`, only affecting plugins. (part of #1979)
- Fixed a case of use after free in the `@reloadatcommand` GM command. (part of #1979)
- Added several missing checks in various `clif_parse_*` functions. (part of #1979)
- Fixed various PIN code related exploits. (part of #1979)
- Fixed a case of use after free when the option `delay_battle_damage` is set to false. (part of #1979)
- Fixed a segmentation fault in clan-related code when using the `db2sql` plugin. (#1989, issue #1984)
- Fixed an incorrect behavior in RODEX returned messages. (part of #1987)
- Fixed an error that made RODEX mails impossible to delete in some cases. (part of #1987)
- Fixed a NULL pointer in RODEX when the user tried to perform actions on unloaded mails. (part of #1987, issue #1977)
- Fixed an incorrect interaction between RODEX and NPCs. (#1936)
- Fixed an incorrect Kafra Points / Cash Points calculation. (#1541, issue #1540)

### Removed

- Removed the old `mapcache` executable, superseded by the new plugin. (part of #1552)

## [v2018.02.11+1] `February 13 2018` `PATCH 1`

### Fixed

- Fixed a possible crash in `@cvcon` (and possibly other functions) when a referenced map zone doesn't exist. (#1972, issue #1971)
- Fixed the messages displayed when enabling or disabling CvC. (part of #1972)
- Extended the `bg_message` string termination fix to all the clients. (#1973)

## [v2018.02.11] `February 11 2018`

### Added

- Added/updated packets support for clients from 2017-12-13 to 2018-01-24. (part of #1957)
- Implemented the official Clan System, including the possibility of customization and a Clan vs Clan versus mode. (#1718, #1964, #1968, related to issue #241)
  - New GM commands: `@claninfo`, `@joinclan`, `@leaveclan`, `@reloadclans`, `@cvcon` and `@cvcoff`.
  - New script commands: `clan_join()`, `clan_leave()` and `clan_master()`; extended `strcharinfo()` and `getcharid()`.
  - Configuration changes: see `conf/clans.conf`, `conf/map/logs.conf`, `db/clans.conf`, `db/*/map_zone_db.conf`.
  - Note: This requires the SQL migrations `2017-06-04--15-04.sql` and `2017-06-04--15-05.sql`.
  - Note: The `npc/re/other/clans.txt` script is now loaded by default in renewal mode.
- Added several (status-icon related) constants to the script engine (through the new `constants.inc` file). (part of #1718)
- Implemented the missing HPM interfaces in the login server (account, ipban, lchrif), added the missing variables into the login interfaces. (#1963, issue #1908)
  - The `_sql` suffix has been removed from the source files in the login server.
  - Functions in `account.c` and `loginlog.c` have been prefixed with `account_` and `loginlog_` respectively.
  - The `chrif_` functions of the login server have been renamed to `lchrif_`.
  - The `server[]` array has been moved to `login->dbs->server[]`.
  - The `account` (account.h), `ipban` (`ipban.h`), `lchrif` (login.h), `loginlog` (`loginlog.h`)
  - Several `log_*` global variables have been moved to the loginlog interface, with their respective names.
  - The `account_engine[0]` variable has been moved to `login->dbs->account_engine` (note: this is not an array!)
- Added/updated packets support for clients from 2018-01-31 to 2018-02-07. (#1969)

### Changed

- Applied script standardization to the Bakonawa Lake instance script. (#1874)
- Applied script standardization to the Buwaya Cave instance script. (#1877)
- Applied script standardization to the Eclage Interior instance script. (#1878)
- Applied script standardization to the Hazy Forest instance script. (#1880)
- Applied script standardization to the Malangdo Culvert instance script. (#1881)

### Fixed

- Fixed compatibility issues with the 2013-12-23 client. (part of #1957, issue #1956)
- Prevented the leak of a hidden GM's presence through area packets. (#1200)
- Fixed an unterminated string in the `bg_message()` related packets, with certain client versions. (#1890)

## [v2018.01.14] `January 14 2018`

### Added

- Added support for the `AllowReproduce` flag in the skill DB. This supersedes the `skill_reproduce_db`. (#1943)
- Added support for the `ZC_PROGRESS_ACTOR` packet. The packet is exposed to the script engine through the `progressbar_unit()` command (available on PACKETVER 20130821 and newer). (#1929)
- Added support for the new item drop packet for the Zero clients. The packet is controlled by the `ShowDropEffect` and `DropEffectMode` item DB flags and ignored by non-Zero clients. (#1939)
- Added support for the new Map Server Change packet 0x0ac7. (part of #1948)

### Changed

- Always enabled assertions and null pointer checks. In order to disable them (very discouraged, as it may lead to security issues), it is now necessary to edit `nullpo.h`. (#1937)
- Disabled the address sanitizer's memory leak detector in the travis builds, since it produced failures in third libraries. (#1949, #1952)
- Applied script standardization to the Nydhogg's Nest instance script. (#1871)
- Split `packet_keys.h` into separate files for main clients and zero clients. (part of #1948)
- Split `packets_shuffle.h` into separate files for main clients and zero clients. (part of #1948)
- Replaced the custom bank unavailable error message with the actual bank check error packet. (part of #1948)
- Updated and corrected the party member and party info packets. (part of #1948)
- Updated `README.md` with more relevant badges and links (added Discord, removed Waffle, added more GitHub information). (#1951)

### Fixed

- Updated Xcode project to include the RODEX related files. (#1942)
- Fixed RODEX loading mails when it requires multiple packets. (#1945, issue #1933)

### Removed

- Removed the `skill_reproduce_db`, now superseded by the `AllowReproduce` skill flag. (part of #1943)

## [v2017.12.17] `December 17 2017`

### Added

- Implemented Homunculus Autofeeding, available on the 2017 clients. The feature can be disabled by flipping `features.enable_homun_autofeed` in `feature.conf`. (#1898)
- Added support for the newly released Ragnarok Zero clients. The client type is controlled with the `--enable-packetver-zero` configure-time flag (disabled by default). (#1923)

### Changed

- Applied script standardization to the Old Glast Heim instance script. (#1883)
- Split packets.h into two files: `packets.h` and `packets_shuffle.h`. (part of #1923)

### Fixed

- Corrected a wrong path displayed in an error message pointing to the map-server configuration. (#1913)
- Fixed the natural expiration of the Poison status when under the effect of Slow Poison. (#1925)

## [v2017.11.19+2] `November 28 2017` `PATCH 2`

### Fixed

- Fixed an item loading failure in RODEX. (#1917, issue #1912)
- Fixed invisible NPCs (such as `FAKE_NPC`) being displayed as novices. (#1918, issue #1916)

## [v2017.11.19+1] `November 24 2017` `PATCH 1`

### Fixed

- Suppressed assertions in the Skill DB accessors when called with `skill_id = 0` (normal attacks). (#1910, issue #1909)

## [v2017.11.19] `November 19 2017`

### Added

- Added several missing members to the login interface. (Part of #1891)
- Added support for colored character server population counter in the service selection list. Configurable through `users_count` in `login-server.conf`. (#1891)
- Added top-level command `miniboss_monster` to label monsters as minibosses, and to send them as such to the client. (part of #1889)
- Added support for 2017-10-25 - 2017-11-01 clients. (#1889)
- Added support to display NPCs with player classes, including equipment and styles (best with clients starting from 20170726). This extends `getunitdata()` and `setunitdata()` with support for `UDT_SEX`, `UDT_HAIRSTYLE`, `UDT_HAIRCOLOR`, `UDT_HEADBOTTOM`, `UDT_HEADMIDDLE`, `UDT_HEADTOP`, `UDT_CLOTHCOLOR`, `UDT_SHIELD`, `UDT_WEAPON`, `UDT_ROBE`, `UDT_BODY2`. (#1893)
- Added type constants for the `getiteminfo()` and `setiteminfo()` script commands. Existing third party code must be updated to use the new constants (see the pull request description and the command documentation for details). (#1902)
- Added global function `F_GetAmmoType()`, counterpart to `F_GetWeaponType()` for ammunitions. Both functions have now been updated to only check the subtype if the item type is correct (`IT_AMMO` and `IT_WEAPON` respectively). (part of #1902)
- Added support for the Skill Scale packet, available in client versions 20151223 and newer. (#1903)

### Changed

- Applied script standardization to the Octopus Cave instance script. (#1882)
- Applied script standardization to the Ghost Palace instance script. (#1879)
- Applied script standardization to the Sara's Memory instance script. (#1884)
- Extended the script command `setequipoption()` with the possibility to remove item options from an equipment piece. (#1865)
- Updated the `QTYPE_*` constants (`questinfo()`, `showevent()`) to support the new 2017 client icons. (#1894)
- Applied script standardization to the Orc's Memory instance script. (#1872)
- Applied script standardization to the Sealed Shrine instance script. (#1873)
- Extended the global function `F_GetArmorType()` to support costumes and shadow equipment. (#1836)
- Extended the script commands `has_instance()` and `has_instance2()` with support to search instances of type `IOT_NONE`. (#1397)
- Applied script standardization and improvements to the Endless Tower instance script. (#1862)
- Cleared some confusion between skill IDs and indexes through the codebase. Rewritten Skill DB accessors in a safer, more readable way. (part of #1896)

### Deprecated

- The use of numeric type constants with `getiteminfo()` and `setiteminfo()` is deprecated. For technical reasons, no deprecation notice is displayed. (part of #1902)

### Removed

- The `MAX_SKILL` constant has been removed, in favor of the more clear `MAX_SKILL_DB`, to be used in all places that use the compacted Skill DB array. For use with the non-compacted clientside Skill IDs, the `MAX_SKILL_ID` constant is still available. (part of #1896)

### Fixed

- Fixed compilation warnings when compiling with gcc-7. (#1887)
- Fixed the display flag for monsters labeled as `boss_monster` to be that of MVP monsters instead of miniboss monsters. (part of #1889)
- Fixed a subtle error in case `skill->unit_group_newid` overflows, causing certain skill unit entries to get stuck and never get deleted correctly. This can manifest itself with some monster spawns becoming immune to certain AoE spells having the `UF_NOOVERLAP` flag (Storm Gust, Lord of Vermillion, etc). (#1896)
- Implemented MATK support in the `getiteminfo()` and `setiteminfo()`. This functionality was previously advertised as available in the command documentation, but was not implemented. (part of #1902)
- Restored View Sprite support in the `getiteminfo()` and `setiteminfo()`. This functionality was lost with #1828. (part of #1902, issue #1895)
- Reimplemented the global function `F_GetArmorType()` to reflect the fact that  `ITEMINFO_LOC` returns a bitmask. The function now handles multi-slot headgears and other uncommon cases better. (part of #1902)
- Corrected some incorrect data types passed to the SQL `StmtBind` functions, causing query errors and data loss. Said functions will now have a runtime assertion to ensure the right data type is passed. Third party code needs to be updated to reflect this stricter requirement. (#1901, issue #1531)
- Corrected some RODEX related queries in case `MAX_SLOTS` or `MAX_ITEM_OPTIONS` are set to custom values. (part of #1901)

## [v2017.10.22+1] `October 22 2017` `PATCH 1`

### Fixed

- Fixed a wrong null pointer check in `logmes()`, which caused the command to never log and instead print debug information.

## [v2017.10.22] `October 22 2017`

### Added

- Added the script command `getmapinfo()`, which allows to obtain misc information about a map. (#1852)
- Added an option to restrict party leader changes to characters on the same map. Controlled by the setting `party_change_leader_same_map` (defaults to true). (#1812)
- Added initial support (shuffle packets, obfuscation keys) for clients 2017-09-27, 2017-10-02, 2017-10-11, 2017-10-18. (#1859)
- Added the `noautoloot` mapflag, allowing to disable the `@autoloot` functionality on a map by map basis. (#1833)

### Changed

- Extended the script command `logmes()` with an option to log to the `atcommandlog` table. (#1843)
- Updated RoDEX, with support for packetver `20170419` and newer. (#1859)
- Updated Exp-related packets and handling functions to support values larger than 2 billions (as seen in packetver `20170830` and newer). (#1859)
- Changed the diagnostic message in `skill_init_unit_layout()` to report the skill ID instead of its index. (#1854)

### Fixed

- Corrected the Kafra dialog in case a Doram without the Summoner's Basic Skill attempts to open the Storage. (#1864)
- Changed the cell stack counting algorithm to ignore invisible NPCs, improving the Dancer Quest experience as well as other cases of hidden NPCs blocking off certain cells. (#1827)
- Improved the handling of the `cardfix` value to make it more resistant to overflows, especially in renewal mode. Simplified the related renewal/pre-renewal conditional code. (#1825)
- Fixed some compilation warnings occurring in VS2017. (#1870)

### Other

- New versioning scheme and project changelogs/release notes (#1853)

[Unreleased]: https://github.com/HerculesWS/Hercules/compare/stable...master
[v2021.03.08]: https://github.com/HerculesWS/Hercules/compare/v2021.02.08...v2021.03.08
[v2021.02.08]: https://github.com/HerculesWS/Hercules/compare/v2021.01.11...v2021.02.08
[v2021.01.11]: https://github.com/HerculesWS/Hercules/compare/v2020.12.14+1...v2021.01.11
[v2020.12.14+1]: https://github.com/HerculesWS/Hercules/compare/v2020.12.14...v2020.12.14+1
[v2020.12.14]: https://github.com/HerculesWS/Hercules/compare/v2020.11.16...v2020.12.14
[v2020.11.16]: https://github.com/HerculesWS/Hercules/compare/v2020.10.19...v2020.11.16
[v2020.10.19]: https://github.com/HerculesWS/Hercules/compare/v2020.09.20...v2020.10.19
[v2020.09.20]: https://github.com/HerculesWS/Hercules/compare/v2020.08.23...v2020.09.20
[v2020.08.23]: https://github.com/HerculesWS/Hercules/compare/v2020.07.26...v2020.08.23
[v2020.07.26]: https://github.com/HerculesWS/Hercules/compare/v2020.06.28...v2020.07.26
[v2020.06.28]: https://github.com/HerculesWS/Hercules/compare/v2020.05.31+1...v2020.06.28
[v2020.05.31+1]: https://github.com/HerculesWS/Hercules/compare/v2020.05.31...v2020.05.31+1
[v2020.05.31]: https://github.com/HerculesWS/Hercules/compare/v2020.05.03...v2020.05.31
[v2020.05.03]: https://github.com/HerculesWS/Hercules/compare/v2020.04.05+1...v2020.05.03
[v2020.04.05+1]: https://github.com/HerculesWS/Hercules/compare/v2020.04.05...v2020.04.05+1
[v2020.04.05]: https://github.com/HerculesWS/Hercules/compare/v2020.03.08+2...v2020.04.05
[v2020.03.08+2]: https://github.com/HerculesWS/Hercules/compare/v2020.03.08+1...v2020.03.08+2
[v2020.03.08+1]: https://github.com/HerculesWS/Hercules/compare/v2020.03.08...v2020.03.08+1
[v2020.03.08]: https://github.com/HerculesWS/Hercules/compare/v2020.02.09...v2020.03.08
[v2020.02.09]: https://github.com/HerculesWS/Hercules/compare/v2020.01.12...v2020.02.09
[v2020.01.12]: https://github.com/HerculesWS/Hercules/compare/v2019.12.15...v2020.01.12
[v2019.12.15]: https://github.com/HerculesWS/Hercules/compare/v2019.11.17+1...v2019.12.15
[v2019.11.17+1]: https://github.com/HerculesWS/Hercules/compare/v2019.11.17...v2019.11.17+1
[v2019.11.17]: https://github.com/HerculesWS/Hercules/compare/v2019.10.20...v2019.11.17
[v2019.10.20]: https://github.com/HerculesWS/Hercules/compare/v2019.09.22...v2019.10.20
[v2019.09.22]: https://github.com/HerculesWS/Hercules/compare/v2019.08.25...v2019.09.22
[v2019.08.25]: https://github.com/HerculesWS/Hercules/compare/v2019.07.28...v2019.08.25
[v2019.07.28]: https://github.com/HerculesWS/Hercules/compare/v2019.06.30...v2019.07.28
[v2019.06.30]: https://github.com/HerculesWS/Hercules/compare/v2019.06.02...v2019.06.30
[v2019.06.02]: https://github.com/HerculesWS/Hercules/compare/v2019.05.05+4...v2019.06.02
[v2019.05.05+4]: https://github.com/HerculesWS/Hercules/compare/v2019.05.05+3...v2019.05.05+4
[v2019.05.05+3]: https://github.com/HerculesWS/Hercules/compare/v2019.05.05+2...v2019.05.05+3
[v2019.05.05+2]: https://github.com/HerculesWS/Hercules/compare/v2019.05.05+1...v2019.05.05+2
[v2019.05.05+1]: https://github.com/HerculesWS/Hercules/compare/v2019.05.05...v2019.05.05+1
[v2019.05.05]: https://github.com/HerculesWS/Hercules/compare/v2019.04.07+1...v2019.05.05
[v2019.04.07+1]: https://github.com/HerculesWS/Hercules/compare/v2019.04.07...v2019.04.07+1
[v2019.04.07]: https://github.com/HerculesWS/Hercules/compare/v2019.03.10...v2019.04.07
[v2019.03.10]: https://github.com/HerculesWS/Hercules/compare/v2019.02.10+1...v2019.03.10
[v2019.02.10+1]: https://github.com/HerculesWS/Hercules/compare/v2019.02.10...v2019.02.10+1
[v2019.02.10]: https://github.com/HerculesWS/Hercules/compare/v2018.12.16+1...v2019.02.10
[v2018.12.16+1]: https://github.com/HerculesWS/Hercules/compare/v2018.12.16...v2018.12.16+1
[v2018.12.16]: https://github.com/HerculesWS/Hercules/compare/v2018.11.18+1...v2018.12.16
[v2018.11.18+1]: https://github.com/HerculesWS/Hercules/compare/v2018.11.18...v2018.11.18+1
[v2018.11.18]: https://github.com/HerculesWS/Hercules/compare/v2018.10.21...v2018.11.18
[v2018.10.21]: https://github.com/HerculesWS/Hercules/compare/v2018.09.23...v2018.10.21
[v2018.09.23]: https://github.com/HerculesWS/Hercules/compare/v2018.08.26+1...v2018.09.23
[v2018.08.26+1]: https://github.com/HerculesWS/Hercules/compare/v2018.08.26...v2018.08.26+1
[v2018.08.26]: https://github.com/HerculesWS/Hercules/compare/v2018.07.29+2...v2018.08.26
[v2018.07.29+2]: https://github.com/HerculesWS/Hercules/compare/v2018.07.29+1...v2018.07.29+2
[v2018.07.29+1]: https://github.com/HerculesWS/Hercules/compare/v2018.07.29...v2018.07.29+1
[v2018.07.29]: https://github.com/HerculesWS/Hercules/compare/v2018.07.01+1...v2018.07.29
[v2018.07.01+1]: https://github.com/HerculesWS/Hercules/compare/v2018.07.01...v2018.07.01+1
[v2018.07.01]: https://github.com/HerculesWS/Hercules/compare/v2018.06.03...v2018.07.01
[v2018.06.03]: https://github.com/HerculesWS/Hercules/compare/v2018.05.06...v2018.06.03
[v2018.05.06]: https://github.com/HerculesWS/Hercules/compare/v2018.04.08...v2018.05.06
[v2018.04.08]: https://github.com/HerculesWS/Hercules/compare/v2018.03.11...v2018.04.08
[v2018.03.11]: https://github.com/HerculesWS/Hercules/compare/v2018.02.11+1...v2018.03.11
[v2018.02.11+1]: https://github.com/HerculesWS/Hercules/compare/v2018.02.11...v2018.02.11+1
[v2018.02.11]: https://github.com/HerculesWS/Hercules/compare/v2018.01.14...v2018.02.11
[v2018.01.14]: https://github.com/HerculesWS/Hercules/compare/v2017.12.17...v2018.01.14
[v2017.12.17]: https://github.com/HerculesWS/Hercules/compare/v2017.11.19+2...v2017.12.17
[v2017.11.19+2]: https://github.com/HerculesWS/Hercules/compare/v2017.11.19+1...v2017.11.19+2
[v2017.11.19+1]: https://github.com/HerculesWS/Hercules/compare/v2017.11.19...v2017.11.19+1
[v2017.11.19]: https://github.com/HerculesWS/Hercules/compare/v2017.10.22+1...v2017.11.19
[v2017.10.22+1]: https://github.com/HerculesWS/Hercules/compare/v2017.10.22...v2017.10.22+1
[v2017.10.22]: https://github.com/HerculesWS/Hercules/compare/6b1fe2d...v2017.10.22
