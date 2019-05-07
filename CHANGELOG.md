# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project does not adhere to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]
<!--
If you are reading this in a text editor, simply ignore this section
-->

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
- Corrected an example using a sprite number instead of a constant in README.md. (#2449)
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
- Added a missing value into enum `BATTLEGROUNDS_QUEUE_ACK`. (part of #2377)

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
- Added a shortand to call `mes()` without arguments to mean `mes("")`. (#2193)

### Fixed

- Added a missing `IF NOT EXISTS` clause in the `char_achievements` table creation query. (e71e41b36b)
- Fixed an issue in packet `ZC_INVENTIRY_MOVE_FAILED` (#2199, issue #2213)
- Fixed a validation error in `setquestinfo()`. (#2218)
- Fixed an error in the achievement system, when killing a cloned mob. (#2204, issue #2201)
- Fixed a trucation issue in the card columns of the database. (#2205, issue #2187)
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
- Added a temporary patch for getd when variable types are C_NOP. (#2163)

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
- Fixed a bug that sent an attendance system message without the attendence ui being opened. (#2129)
- Corrected several outdated documentation references to db/constants.conf, to point to doc/constants.md. (#2090)
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
- Updated README.md with some clarifications and corrections. (#1985)

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
  - The `account` (account.h), `ipban` (ipban.h), `lchrif` (login.h), `loginlog` (loginlog.h)
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
- Added support for the `AllowReproduce` flag in the skill DB. This supersedes the skill_reproduce_db. (#1943)
- Added support for the `ZC_PROGRESS_ACTOR` packet. The packet is exposed to the script engine through the `progressbar_unit()` command (available on PACKETVER 20130821 and newer). (#1929)
- Added support for the new item drop packet for the Zero clients. The packet is controlled by the `ShowDropEffect` and `DropEffectMode` item DB flags and ignored by non-Zero clients. (#1939)
- Added support for the new Map Server Change packet 0x0ac7. (part of #1948)

### Changed
- Always enabled assertions and null pointer checks. In order to disable them (very discouraged, as it may lead to security issues), it is now necessary to edit `nullpo.h`. (#1937)
- Disabled the address sanitizer's memory leak detector in the travis builds, since it produced failures in third libraries. (#1949, #1952)
- Applied script standardization to the Nydhogg's Nest instance script. (#1871)
- Split packet_keys.h into separate files for main clients and zero clients. (part of #1948)
- Split packets_shuffle.h into separate files for main clients and zero clients. (part of #1948)
- Replaced the custom bank unavailable error message with the actual bank check error packet. (part of #1948)
- Updated and corrected the party member and party info packets. (part of #1948)
- Updated README.md with more relevant badges and links (added Discord, removed Waffle, added more GitHub information). (#1951)

### Fixed
- Updated Xcode project to include the RODEX related files. (#1942)
- Fixed RODEX loading mails when it requires multiple packets. (#1945, issue #1933)

### Removed
- Removed the skill_reproduce_db, now superseded by the `AllowReproduce` skill flag. (part of #1943)

## [v2017.12.17] `December 17 2017`
### Added
- Implemented Homunculus Autofeeding, available on the 2017 clients. The feature can be disabled by flipping `features.enable_homun_autofeed` in feature.conf. (#1898)
- Added support for the newly released Ragnarok Zero clients. The client type is controlled with the `--enable-packetver-zero` configure-time flag (disabled by default). (#1923)

### Changed
- Applied script standardization to the Old Glast Heim instance script. (#1883)
- Split packets.h into two files: packets.h and packets_shuffle.h. (part of #1923)

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
- Added support for colored character server population counter in the service selection list. Configurable through `users_count` in login-server.conf. (#1891)
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
- Extended the script commands `has_instance()` and `has_instance2()` with suport to search instances of type `IOT_NONE`. (#1397)
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
- Implemented MATK support in the `getiteminfo()` and `setiteminfo()`. This functionality was previously advertised as availble in the command documentation, but was not implemented. (part of #1902)
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
