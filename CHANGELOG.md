# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project does not adhere to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]
<!--
If you are reading this in a text editor, simply ignore this section
-->

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
[v2018.01.14]: https://github.com/HerculesWS/Hercules/compare/v2017.12.17...v2018.01.14
[v2017.12.17]: https://github.com/HerculesWS/Hercules/compare/v2017.11.19+2...v2017.12.17
[v2017.11.19+2]: https://github.com/HerculesWS/Hercules/compare/v2017.11.19+1...v2017.11.19+2
[v2017.11.19+1]: https://github.com/HerculesWS/Hercules/compare/v2017.11.19...v2017.11.19+1
[v2017.11.19]: https://github.com/HerculesWS/Hercules/compare/v2017.10.22+1...v2017.11.19
[v2017.10.22+1]: https://github.com/HerculesWS/Hercules/compare/v2017.10.22...v2017.10.22+1
[v2017.10.22]: https://github.com/HerculesWS/Hercules/compare/6b1fe2d...v2017.10.22
