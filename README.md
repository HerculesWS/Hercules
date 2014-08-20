Hercules
========

Build Status: [![Build Status](https://travis-ci.org/HerculesWS/Hercules.png?branch=master)](https://travis-ci.org/HerculesWS/Hercules) 

Table of Contents
---------
* 1 What is Hercules?
* 2 Prerequisites
* 3 Installation
* 4 Troubleshooting
* 5 Helpful Links
* 6 More Documentation

1. What is Hercules?
---------
Hercules is a collaborative software development project revolving around the
creation of a robust massively multiplayer online role playing game (MMORPG)
server package. Written in C, the program is very versatile and provides NPCs,
warps and modifications. The project is jointly managed by a group of volunteers
located around the world as well as a tremendous community providing QA and
support. Hercules is a continuation of the original Athena project.

2. Prerequisites
---------
Before installing Hercules there are certain tools and applications you will need.
This differs between the varying operating systems available, so the following
is broken down into Windows and Linux prerequisites.

* Windows
	* TortoiseGIT ( http://code.google.com/p/tortoisegit/ )
	* MSysGit ( http://code.google.com/p/msysgit/downloads/list?can=2 )
	* MySQL ( http://www.mysql.com/downloads/mysql/ )
	* MySQL Workbench ( http://www.mysql.com/downloads/workbench/ )
	* MS Visual C++ ( http://www.microsoft.com/visualstudio/en-us/products/2010-editions/visual-cpp-express )

* Linux (names of packages may require specific version numbers on certain distributions)
	* gcc
	* make
	* mysql
	* mysql-devel
	* mysql-server
	* pcre-devel
	* git
	* zlib-devel

3. Installation
---------
This section is a very brief set of installation instructions. For more concise guides
relevant to your Operation System, please refer to the Wiki (links at the end of this file).

* Windows
	* Install prerequisites
	* Create a folder to download Hercules into (e.g. C:\Hercules)
	* Right click this folder and select "Git Clone".
	* Paste the GIT URL into the box: https://github.com/HerculesWS/Hercules.git
	* Open MySQL Workbench and create an instance to connect to your MySQL Server
	* Create a database (hercules), a user (hercules), give permissions (GRANT SELECT,INSERT,UPDATE,DELETE)
		and then login using the new user
	* Use MySQL Workbench to run the .sql files in /sql-files/ on the new Hercules database

* Linux
	* (For CentOS)
		* Step 1: yum install gcc make mysql mysql-devel mysql-server pcre-devel zlib-devel
			* Step 2: rpm -Uvh http://repo.webtatic.com/yum/centos/5/latest.rpm
			* Step 3: yum install --enablerepo=webtatic git-all
			* Step 4: yum install --enablerepo=webtatic --disableexcludes=main git-all
	* (For Debian/Others)
		* Type: apt-get install git make gcc libmysqlclient-dev zlib1g-dev libpcre3-dev
	* Type: mysql_secure_installation
	* Start your MySQL server
	* Setup a MySQL user:

				CREATE USER 'hercules'@'localhost' IDENTIFIED BY 'password';
	* Assign permissions:

				GRANT SELECT,INSERT,UPDATE,DELETE ON `hercules\_rag`.* TO 'hercules'@'localhost';
	* Type: git clone https://github.com/HerculesWS/Hercules.git ~/Hercules
	* Insert SQL files: mysql --user=root -p hercules_rag < trunk/sql-files/main.sql (and others)
	* Type: cd trunk && ./configure && make clean && make sql
	* When you're ready, type: ./athena-start start



4. Troubleshooting
---------
If you're having problems with starting your server, the first thing you should
do is check what's happening on your consoles. More often that not, all support issues
can be solved simply by looking at the error messages given.

Examples:

* You get an error on your map-server_sql that looks something like this:

		[Error]: npc_parsesrcfile: Unable to parse, probably a missing or extra TAB in 
			file 'npc/custom/jobmaster.txt', line '17'. Skipping line...
			* w1=prontera,153,193,6 script
			* w2=Job Master
			* w3=123,{
			* w4=

	If you look at the error, it's telling you that you're missing (or have an extra) TAB.
		This is easily fixed by looking at this part of the error: * w1=prontera,153,193,6 script
		If there was a TAB where it's supposed to be, that line would have prontera,153,193,6 at w1
		and 'script' at w2. As there's a space instead of a TAB, the two sections are read as a
		single parameter.

* You have a default user/password warning similar to the following:
		
			[Warning]: Using the default user/password s1/p1 is NOT RECOMMENDED.
			[Notice]: Please edit your 'login' table to create a proper inter-server user/pa
			ssword (gender 'S')
			[Notice]: and then edit your user/password in conf/map-server.conf (or conf/impo
			rt/map_conf.txt)

	Relax. This is just indicating that you're using the default username and password. To
		fix this, check over the part in the installation instructions relevant to the `login` table.

* Your Map Server says the following:

			[Error]: make_connection: connect failed (socket #2, error 10061: No connection
			could be made because the target machine actively refused it.
			)!

	If this shows up on the map server, it generally means that there is no Char Server available
		to accept the connection.

5. Helpful Links
---------
The following list of links point to various help files within the GIT, articles or
pages on the Wiki or topics within the Hercules forum.

* Hercules Forums
	http://hercules.ws/board/

* GIT Repository URL:
	https://github.com/HerculesWS/Hercules

* Hercules IRC Channel
	irc.rizon.net
	Channel: #Hercules



6. More Documentation
---------
Hercules has a large collection of help files and sample NPC scripts located in /doc/

* Scripting
	It is recommended to look through /doc/script_commands.txt for help, pointers or
	even for ideas for your next NPC script. Most script commands have a usage example.

* @commands
	In-game, Game Masters have the ability to use Atcommands (@) to control players, 
	create items, spawn mobs, reload configuration files and even control the weather.
	For an in-depth explanation, please see /doc/atcommands.txt

* Permissions
	The Hercules emulator has a permission system that enables certain groups of players
	to perform certain actions, or have access to certain visual enhancements or in-game
	activity. To see what permissions are available, they are detailed in /doc/permissions.txt

There are more files in the /doc/ directory that will help you to create scripts or update the
mapcache, or even explain how the job system and item bonuses work. Before posting a topic asking
for help on the forums, we recommend that all users take the time to look over this directory.
