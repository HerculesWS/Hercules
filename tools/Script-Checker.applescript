(*
  This file is part of Hercules.
  http://herc.ws - http://github.com/HerculesWS/Hercules

  Copyright (C) 2014-2015  Hercules Dev Team

  Hercules is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*)

-- Base Author: Haru @ http://herc.ws

(*
	*************************************************************
	*************************************************************
	********                                                                       **************
	********    NOTE:  This script must be saved as app!!!     **************
	********                                                                       **************
	*************************************************************
	*************************************************************
*)

property allowed_extensions : {"txt", "c", "ath", "herc"}
property allowed_types : {"TEXT"}

on run
	try
		set the resource_path to path to resource "Scripts"
		tell application "Finder" to set the resource_path to the container of the resource_path
	on error
		display alert "Error" message "You need to save this script as an app bundle." buttons {"Cancel"} cancel button 1 as warning
	end try
	set the dialog_result to display dialog "You can drag files to this app to have them checked." with title "Hercules Script Checker" buttons {"Cancel", "(Re)build Hercules", "Check Script"} default button 3 cancel button 1
	if the button returned of the dialog_result is "(Re)build Hercules" then
		rebuild()
	else
		set these_items to choose file of type allowed_extensions with prompt "Choose the script(s) you want to check:" with multiple selections allowed
		process_items(these_items)
	end if
end run

on open these_items
	process_items(these_items)
end open

on build_hercules(hercules_repo)
	try
		set the resource_path to path to resource "Scripts"
		tell application "Finder" to set the resource_path to the container of the resource_path
	on error
		display alert "Error" message "You need to save this script as an app bundle." buttons {"Cancel"} cancel button 1 as warning
	end try
	set the configuration_flags to display dialog "Do you want to use any configuration flags? (i.e. --disable-renewal)" with title "Configuration" default answer "" buttons {"Cancel", "Ok"} default button 2 cancel button 1
	try
		set the command_output to do shell script "cd " & (the quoted form of the POSIX path of the hercules_repo) & " && echo './configure " & the text returned of the configuration_flags & " 2>&1' | bash -i"
	on error
		display dialog "Configuration failed" with title "Configuration result" buttons {"Abort"} cancel button 1
	end try
	tell application "TextEdit"
		activate
		set the new_document to make new document
		set the text of new_document to the command_output
	end tell
	display dialog "Configuration successfully completed. Please check the log file for details." with title "Configuration result" buttons {"Abort", "Continue"} default button 2 cancel button 1
	try
		set the command_output to do shell script "cd " & (the quoted form of the POSIX path of the hercules_repo) & " && echo 'make clean 2>&1 && make sql -j8 2>&1' | bash -i"
	on error
		display dialog "Build failed." with title "Build result" buttons {"Abort"} cancel button 1
	end try
	tell application "TextEdit"
		activate
		set the new_document to make new document
		set the text of new_document to the command_output
	end tell
	display dialog "Build successfully completed. Please check the log file for details." with title "Build result" buttons {"Abort", "Continue"} default button 2 cancel button 1
	set the files_to_copy to {"map-server", "script-checker"}
	set the conf_files_to_copy to {"inter-server.conf", "import", "packet.conf", "script.conf"}
	set the db_files_to_copy to {"map_index.txt", "item_db2.txt", "constants.conf", "mob_db2.txt"}
	set the db2_files_to_copy to {"map_cache.dat", "item_db.txt", "skill_db.txt", "mob_db.txt"}
	try
		set the hercules_path to path to resource "Hercules"
		do shell script "rm -r " & the quoted form of ((the POSIX path of hercules_path) & "/")
	end try
	set the hercules_path to the resource_path
	tell application "Finder" to make new folder at hercules_path with properties {name:"Hercules"}
	delay 3
	set the hercules_path to path to resource "Hercules"
	repeat with each_file in files_to_copy
		copy_file(each_file, hercules_repo, hercules_path, ".")
	end repeat
	do shell script "mkdir " & the quoted form of ((POSIX path of the hercules_path) & "/conf")
	repeat with each_file in conf_files_to_copy
		copy_file(each_file, hercules_repo, hercules_path, "conf")
	end repeat
	do shell script "mkdir " & the quoted form of ((POSIX path of the hercules_path) & "/db")
	repeat with each_file in db_files_to_copy
		copy_file(each_file, hercules_repo, hercules_path, "db")
	end repeat
	do shell script "mkdir " & the quoted form of ((POSIX path of the hercules_path) & "/db/pre-re")
	repeat with each_file in db2_files_to_copy
		copy_file(each_file, hercules_repo, hercules_path, "db/pre-re")
	end repeat
	do shell script "mkdir " & the quoted form of ((POSIX path of the hercules_path) & "/db/re")
	repeat with each_file in db2_files_to_copy
		copy_file(each_file, hercules_repo, hercules_path, "db/re")
	end repeat
	display dialog "Build complete" with title "Done" buttons {"Ok"} default button "Ok"
end build_hercules

on rebuild()
	set the repo_path to choose folder with prompt "Choose the folder where your Hercules repository is located:"
	build_hercules(repo_path)
end rebuild

on copy_file(filename, source, destination, subpath)
	do shell script "cp -rp " & the quoted form of ((the POSIX path of source) & "/" & subpath & "/" & filename) & " " & the quoted form of ((the POSIX path of destination) & "/" & subpath & "/")
end copy_file

on process_items(these_items)
	repeat
		try
			set the scriptchecker to the path to resource "script-checker" in directory "Hercules"
			set the mapserver to the path to resource "map-server" in directory "Hercules"
		on error
			display alert "Missing Hercules binaries" message "You need to build Hercules and place it within this app bundle before running the script checker." & linefeed & linefeed & "I can try to build it for you, but only if you have the Xcode command line tools installed." & linefeed & "Do you want to continue?" buttons {"Cancel", "Build"} default button "Build" cancel button "Cancel" as warning
			rebuild()
			return false
		end try
		exit repeat
	end repeat
	repeat with i from 1 to the count of these_items
		set this_item to item i of these_items
		set the item_info to info for this_item
		set this_name to the name of the item_info
		try
			set this_extension to the name extension of item_info
		on error
			set this_extension to ""
		end try
		try
			set this_filetype to the file type of item_info
		on error
			set this_filetype to ""
		end try
		if (folder of the item_info is false) and (alias of the item_info is false) and ((this_filetype is in the allowed_types) or (this_extension is in the allowed_extensions)) then
			process_item(scriptchecker, this_item)
		end if
	end repeat
end process_items

on process_item(checkerpath, this_item)
	set the_result to do shell script (the quoted form of the POSIX path of checkerpath & " " & the quoted form of the POSIX path of this_item) & " 2>&1"
	if the_result is "" then
		set the_result to "Check passed."
	end if
	display dialog ("File: " & POSIX path of this_item) & linefeed & "Result: " & linefeed & the_result with title "Output" buttons {"Abort", "Next"} default button "Next" cancel button "Abort"
end process_item
