# Hercules Documentation
By: [Myriad](http://herc.ws/board/topic/15868-guide-mapcache-generation-2018/)
Current Version: 20180923
Description: An updated manual for Hercules' map cache generator 

##Mapcache Generation 2018
As of [Release v2018.03.13](https://github.com/HerculesWS/Hercules/commit/d89690fbdbaa5dc78f98d96ee91403e329c12af1), the method to generate mapcache for Hercules has changed. 
### Old  (your source predates [is older than] [Feb 18, 2018](https://github.com/HerculesWS/Hercules/commit/60870581e1e2dd740751c1104299536975015b9e))

In the old system, there were two ways to generate mapcache

- You could run the `mapcache` executable in Hercules root folder.
- Or use a program such as `WeeMapCache` to edit in your required mapcache.

These two methods would generate or alter your required mapcache located in `db/[pre-re or re]/map_cache.dat`. However, they are no longer supported.

### New (your source is using Release v2018.03.13 or newer)

The new system involves the use of the new 'mapcache' plugin to generate files. Some quick points:

- `db/[pre-re or re]/map_cache.dat` has been dropped (no longer supported).
- In its place are individual `.mcache` files for every map located in `maps/[pre-re or re]/`
- `mapcache`executable has been removed.
- Replaced with the `mapcache` plugin (`src/plugins/mapcache.c`).

### How to generate the mapcache?

1. Same as before, check `conf/map/maps.conf` and `db/map_index.txt` have all the maps you want to cache.
2. Your maps need to exist somewhere in your repository! There are two ways for the plugin to find them:
    a. Place all your maps, including `resnametable.txt`, inside the **data folder** of your Hercules repo. I.e. `Hercules/data/prontera.gat/gnd/gnd/rsw` (note: I forget if all three files are needed).
    b. OR Configure your `conf/grf-files.txt` to tell it where to find your GRF(s) which contains your maps.
3. Build the `mapcache` plugin. On linux, this can be done by running the following command: 
    - `make plugin.mapcache`
    - If using `MSVC`, compile as you would any other plugin.
4. Execute plugin. This can be done using the following command:
    - `./map-server --load-plugin mapcache [param]`
    - In windows, just remove the `./` and run the commands in your command prompt.

### The params:

The first thing you should do is run 
`./map-server --load-plugin mapcache --help`

A list of usable parameters will appear. Here are the ones you need to know for mapcache:
```
[Info]:   --convert-old-mapcache         Converts an old db/pre-re/map_cache.dat file to the new format. [Mapcache]
[Info]:   --rebuild-mapcache             Rebuilds the entire mapcache folder (maps/pre-re/), using db/map_index.txt as index. [Mapcache]
[Info]:   --map <name>                   Rebuilds an individual map's cache into maps/pre-re/ (usage: --map <map_name_without_extension>). [Mapcache]
[Info]:   --fix-md5                      Updates the checksum for the files in maps/pre-re/, using db/map_index.txt as index (see PR #1981). [Mapcache]
```

- Rebuild all the `.mcache files` using your old `db/[pre-re or re]map_cache.dat` file.
`./map-server --load-plugin mapcache --convert-old-mapcache`

- Rebuild all the `.mcache` files using your map files specified in step 2 of generation.
`./map-server --load-plugin mapcache --rebuild-mapcache`

- Rebuild the `.mcache file` for the map name you specify. E.g. if you replace <name> with prontera, the maps/[pre-re or re]/prontera.mcache file will be rebuilt.
`./map-server --load-plugin mapcache --map <name>`
