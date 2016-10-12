-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2014-2015  Hercules Dev Team
--
-- Hercules is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.

-- Base Author: Dastgir @ http://herc.ws
--
-- This script requires lua 5.1 to run.

getmetatable('').__call = string.sub
require('math')

--Bit Operators
local tab = {  -- tab[i][j] = xor(i-1, j-1)
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, },
  {1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14, },
  {2, 3, 0, 1, 6, 7, 4, 5, 10, 11, 8, 9, 14, 15, 12, 13, },
  {3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12, },
  {4, 5, 6, 7, 0, 1, 2, 3, 12, 13, 14, 15, 8, 9, 10, 11, },
  {5, 4, 7, 6, 1, 0, 3, 2, 13, 12, 15, 14, 9, 8, 11, 10, },
  {6, 7, 4, 5, 2, 3, 0, 1, 14, 15, 12, 13, 10, 11, 8, 9, },
  {7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8, },
  {8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, },
  {9, 8, 11, 10, 13, 12, 15, 14, 1, 0, 3, 2, 5, 4, 7, 6, },
  {10, 11, 8, 9, 14, 15, 12, 13, 2, 3, 0, 1, 6, 7, 4, 5, },
  {11, 10, 9, 8, 15, 14, 13, 12, 3, 2, 1, 0, 7, 6, 5, 4, },
  {12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3, },
  {13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2, },
  {14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1, },
  {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, },
}
function bxor (a,b)
  local res, c = 0, 1
  while a > 0 and b > 0 do
    local a2, b2 = math.fmod(a, 16), math.fmod(b, 16)
    res = res + tab[a2+1][b2+1]*c
    a = (a-a2)/16
    b = (b-b2)/16
    c = c*16
  end
  res = res + a*c + b*c
  return res
end
local ff = 2^32 - 1
function bnot (a) return (ff - a) end
function band (a,b) return ((a+b) - bxor(a,b))/2 end
function bor (a,b) return (ff - band(ff - a, ff - b)) end

function default_value(input,default)
	if (input==nil) then
		return default
	else
		return input
	end
end

print("-----------------------------------------------------------------")
print("----------------- item_*.txt to  ItemDB.conf --------------------")
print("------------------- By Dastgir[Hercules] ------------------------")
print("-----------------------------------------------------------------")
if (arg[1]==nil) then --If any argument is missing, error
	print("Invalid Command:")
	print('Usage: item_merge.lua'
		..' {item_db.conf}'
		..' {item_trade.txt}'
		..' {item_buyingstore.txt}'
		..' {item_delay.txt}'
		..' {item_nouse.txt}'
		..' {item_stack.txt}'
		..' {item_avail.txt}'
		..' {output.conf}'
		--..' {sql_update_file.sql}'
	)
	print("","If you have default names of *.txt files, then just run this tool")
	print("","","","\"item_merge.lua item_db.conf\"")
	os.exit()
end
arg[2] = default_value(arg[2],"item_trade.txt")
arg[3] = default_value(arg[3],"item_buyingstore.txt")
arg[4] = default_value(arg[4],"item_delay.txt")
arg[5] = default_value(arg[5],"item_nouse.txt")
arg[6] = default_value(arg[6],"item_stack.txt")
arg[7] = default_value(arg[7],"item_avail.txt")
arg[8] = default_value(arg[8],string.sub(arg[1],1,-6).."_converted.conf")

function replace_char(pos, str, r)
    return str:sub(1, pos-1) .. r .. str:sub(pos+1)
end
function ltrim(trb)
  return (trb:gsub("^%s*", ""))
end

function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

function d_t_s_quotes(s)	--Double Quotes to Single Quotes and Comment Replacer
	s = s:gsub('\\\\',"\\")
	--Replace Lua Comments to C Style
	s = s:gsub('%-%-%[%[', '/%*')
	s = s:gsub("%-%-%]%]", '%*/')
	local newline = ParseCSVLine(s,"\n")
	if (#newline==0) then
		newline[1] = s
	end
	for i=1,#newline do
		if(string.sub(ltrim(newline[i]),1,2)=="--") then
			newline[i] = newline[i]:gsub("%-%-", '//', 1)
		end
	end
	s = MergeCSVLine(newline)
	return s
end

function st_fill_zero(s)	--Fill remaining zero in Job Field of item_db
	zeros = ""
	if (s:len()>=8) then
		return s
	else
		for i=1,(8-s:len()) do
			zeros = zeros.."0"
		end
		zeros = zeros..""..s
	end
	--print(s:len(),zeros)
	return zeros
end

function newline(s,a)	--Add Newline to Scripts Field
	if ((#(ParseCSVLine(s,"\n"))) > 1) then
		if(a==1) then
			return "\n"
		else
			return "\n\t"
		end
	else
		return ""
	end
end

function MergeCSVLine(linetbl)
	local merged_tbl = ""
	for j=1,#linetbl do
		if (j==#linetbl) then
			merged_tbl = merged_tbl..linetbl[j]
		else
			merged_tbl = merged_tbl..linetbl[j].."\n"
		end
	end
	return merged_tbl
end

function ParseCSVLine (line,sep) --Parse CSV for itemdb_*.txt fields
	local res = {}
	local pos = 1
	sep = sep or ','
	while true do
		local c = string.sub(line,pos,pos)
		if (c == "") then break end
		if (c == '"') then
			local txt = ""
			repeat
				local startp,endp = string.find(line,'^%b""',pos)
				txt = txt..string.sub(line,startp+1,endp-1)
				pos = endp + 1
				c = string.sub(line,pos,pos)
				if (c == '"') then txt = txt..'"' end
			until (c ~= '"')
			table.insert(res,txt)
			assert(c == sep or c == "")
			pos = pos + 1
		else
			local startp,endp = string.find(line,sep,pos)
			if (startp) then
				table.insert(res,string.sub(line,pos,startp-1))
				pos = endp + 1
			else
				table.insert(res,string.sub(line,pos))
				break
			end
		end
	end
	return res
end

function deepcopy(orig)	--Copy one table to other
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[deepcopy(orig_key)] = deepcopy(orig_value)
        end
        setmetatable(copy, deepcopy(getmetatable(orig)))
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

function parse_line(line2,typl)	--1=trade
	line_number = line_number+1
	csv = ParseCSVLine(line2,",")
	csv1 = ParseCSVLine(csv[1],"\t")
	csv2 = ParseCSVLine(csv[1]," ")
	csv4 = tonumber(csv1[1])
	if (csv4==nil) then
		csv4 = csv2[1]
		if (csv4 == nil) then
			csv4 = csv[1]
		end
	end
	k=tonumber(csv4)
	if (typl==3) then
		if (item_db2[k] ~= nil) then
			item_db2[k].BuyingStore = true
			--sql_update = sql_update.."UPDATE `item_db` SET `buyingstore` = 1 WHERE `id`="..csv4..";\n"
		end
		return
	end
	if (#csv<1) then
		return
	end
	k=tonumber(csv4)
	if (item_db2[k]~=nil) then
		if(typl>1 and typl~=3 and typl~=4 and typl~=7) then
			m=3
		else
			m=2
		end
		csv1 = ParseCSVLine(csv[m],"\t")
		csv2 = ParseCSVLine(csv[m]," ")
		csv3 = tonumber(csv1[1])
		if (csv3==nil) then
			csv3 = csv2[1]
			if (csv3 == nil) then
				csv3 = csv[m]
			end
		end
		if (csv3==nil) then
			print("Invalid format on "..arg[m]..", line:"..line_number..", Skipping.")
			return
		end
		if (typl==2 and #csv==3) then
				item_db2[k].trade_flag = tonumber(csv[2])
				item_db2[k].trade_override = tonumber(csv3)
				--sql_update = sql_update.."UPDATE `item_db` SET `trade_flag` = "..tonumber(csv[2]).." AND `trade_group` = "..tonumber(csv3).." WHERE `id`="..k..";\n"
		--No 3, since its parsed differently.
		elseif (typl==4 and #csv>=2) then
			if (tonumber(csv3)~=nil) then
				item_db2[k].Delay = tonumber(csv3)
				--sql_update = sql_update.."UPDATE `item_db` SET `delay` = "..tonumber(csv3).." WHERE `id`="..k..";\n"
			end
		elseif (typl==5 and #csv==3) then
			item_db2[k].nouse_flag = tonumber(csv[2])
			item_db2[k].nouse_override = tonumber(csv3)
			--sql_update = sql_update.."UPDATE `item_db` SET `nouse_flag` = "..tonumber(csv[2]).." and `nouse_group` = "..tonumber(csv3).." WHERE `id`="..k..";\n"
		elseif (typl==6 and #csv==3) then
				item_db2[k].Stack1 = tonumber(csv[2])
				item_db2[k].Stack2 = tonumber(csv3)
				--sql_update = sql_update.."UPDATE `item_db` SET `stack_amount` = "..tonumber(csv[2]).." and `stack_flag` = "..tonumber(csv3).." WHERE `id`="..k..";\n"
		elseif (typl==7 and #csv==2) then
			item_db2[k].Sprite = tonumber(csv3)
			--sql_update = sql_update.."UPDATE `item_db` SET `avail` = "..tonumber(csv3).." WHERE `id`="..k..";\n"
		else
			print("Invalid format on "..arg[typl]..", line:"..line_number..", Skipping.")
			--[[
			--Debug Lines
			print("Invalid CSV",#csv,typl)
			for mo=1,#csv do
				print("csv["..mo.."]",csv[mo])
			end
			os.exit()
			--]]
		end
	end
	
	return
end

function check_val(field)	--Check if field null
	if (field == nil) then
		return("")
	end
	return(field)
end

function check_val_bool(field)	--Checks boolean value
	if (field == false or field == "false") then
		return "false"
	elseif (field == true or field == "true") then
		return "true"
	end
	return ""
end

function check_val_two(field)	--Check [a, b] types of field
	local field1,field2
	field1 = ""
	field2 = ""
	if (field~=nil) then
		if (#field==2) then
			if (field[1][1] ~= nil and field[2] ~= nil) then
				field1 = field[1][1]
				field2 = field[2]
			elseif (field[1][1] ~= nil and field[2] == nil) then
				field1 = field[1][1]
			else
				field2 = field[2]
			end
		elseif(#field==1) then
			field1 = field[1]
		end
	end
	return field1, field2
end


function item_db_write(s)	--Write into item_db
	for i=1,65535 do	----Last ID till now
		if (s[i]~=nil) then
			s[i].Atk = check_val(s[i].Atk)
			s[i].Matk = check_val(s[i].Matk)
			s[i].EquipLv = check_val(s[i].EquipLv)
			
			if ((s[i].Atk == "" or s[i].Atk == 0) and (s[i].Matk == "" or s[i].Matk == 0)) then	--Both Nil
				s[i].at = ""
			elseif (s[i].Atk ~= "" and (s[i].Matk == ""  or s[i].Matk == 0)) then
				s[i].at = s[i].Atk

			elseif ((s[i].Atk == "" or s[i].Atk == 0) and s[i].Matk ~= "") then
				s[i].at = "0:"..s[i].Matk
			else
				s[i].at = s[i].Atk..":"..s[i].Matk
			end
			
			s[i].Refine = check_val_bool(s[i].Refine)
			s[i].BindOnEquip = check_val_bool(s[i].BindOnEquip)
			s[i].BuyingStore = check_val_bool(s[i].BuyingStore)
			s[i].Script = check_val(s[i].Script)
			s[i].OnEquipScript = check_val(s[i].OnEquipScript)
			s[i].OnUnequipScript = check_val(s[i].OnUnequipScript)
			s[i].ELv1, s[i].ELv2 = check_val_two(s[i].EquipLv)
			s[i].EquipMin = s[i].EquipLv[1]	--Equip Min
			s[i].EquipMax = s[i].EquipLv[2]	--Equip Max
			s[i].Script = d_t_s_quotes(s[i].Script)
			s[i].OnEquipScript = d_t_s_quotes(s[i].OnEquipScript)
			s[i].OnUnequipScript = d_t_s_quotes(s[i].OnUnequipScript)
			file3:write("{\n")
			--four Must fields to write.
			file3:write("\tId: "..(s[i].Id).."\n")
			file3:write("\tAegisName: \""..(s[i].AegisName).."\"\n")
			file3:write("\tName: \""..(s[i].Name).."\"\n")
			file3:write("\tType: "..(s[i].Type).."\n")
			if (check_val(s[i].Buy)~="") then
				file3:write("\tBuy: "..(s[i].Buy).."\n")
			end
			if (check_val(s[i].Sell)~="") then
				file3:write("\tSell: "..(s[i].Sell).."\n")
			end
			if (check_val(s[i].Weight)~="") then
				file3:write("\tWeight: "..(s[i].Weight).."\n")
			end
			if (s[i].Atk~="") then
				file3:write("\tAtk: "..(s[i].Atk).."\n")
			end
			if (s[i].Matk~="") then
				file3:write("\tMatk: "..(s[i].Matk).."\n")
			end
			if (check_val(s[i].Def)~="") then
				file3:write("\tDef: "..(s[i].Def).."\n")
			end
			if (check_val(s[i].Range)~="") then
				file3:write("\tRange: "..(s[i].Range).."\n")
			end
			if (check_val(s[i].Slots)~="") then
				file3:write("\tSlots: "..(s[i].Slots).."\n")
			end
			if (check_val(s[i].Job)~="") then
				file3:write("\tJob: 0x"..st_fill_zero(string.upper(string.format("%x",s[i].Job))).."\n")	--Convert to Hex,Make it to UpperCase, Fill with zero's
			end
			if (check_val(s[i].Upper)~="") then
				file3:write("\tUpper: "..(s[i].Upper).."\n")
			end
			if (check_val(s[i].Gender)~="") then
				file3:write("\tGender: "..(s[i].Gender).."\n")
			end
			if (check_val(s[i].Loc)~="") then
				file3:write("\tLoc: "..(s[i].Loc).."\n")
			end
			if (check_val(s[i].WeaponLv)~="") then
				file3:write("\tWeaponLv: "..(s[i].WeaponLv).."\n")
			end
			if (s[i].ELv1~="" and s[i].ELv2~="" ) then	--Both Not Empty
				file3:write("\tEquipLv: ["..(s[i].ELv1)..", "..(s[i].ELv2).."]\n")
			else
				if(s[i].ELv1~="") then	--First not empty
					file3:write("\tEquipLv: "..(s[i].ELv1).."\n")
				end
			end
			if (check_val(s[i].Refine)~="") then
				file3:write("\tRefine: "..(s[i].Refine).."\n")
			end
			if (check_val(s[i].View)~="") then
				file3:write("\tView: "..(s[i].View).."\n")
			end
			if (s[i].BindOnEquip~="") then
				file3:write("\tBindOnEquip: "..(s[i].BindOnEquip).."\n")
			end
			if (s[i].BuyingStore~="") then
				file3:write("\tBuyingStore: true\n")
			end
			if (check_val(s[i].Delay) ~= "") then
				file3:write("\tDelay: "..(s[i].Delay).."\n")
			end
			if (check_val(s[i].trade_flag)~="" and check_val(s[i].trade_override)~="" ) then
				file3:write("\tTrade: {\n")
				if (s[i].trade_override > 0 and s[i].trade_override < 100) then
					file3:write("\t\toverride: ".. s[i].trade_override .."\n")
				end
				if (band(s[i].trade_flag,1)>0) then
					file3:write("\t\tnodrop: true\n")
				end
				if (band(s[i].trade_flag,2)>0) then
					file3:write("\t\tnotrade: true\n")
				end
				if (band(s[i].trade_flag,4)>0) then
					file3:write("\t\tpartneroverride: true\n")
				end
				if (band(s[i].trade_flag,8)>0) then
					file3:write("\t\tnoselltonpc: true\n")
				end
				if (band(s[i].trade_flag,16)>0) then
					file3:write("\t\tnocart: true\n")
				end
				if (band(s[i].trade_flag,32)>0) then
					file3:write("\t\tnostorage: true\n")
				end
				if (band(s[i].trade_flag,64)>0) then
					file3:write("\t\tnogstorage: true\n")
				end
				if (band(s[i].trade_flag,128)>0) then
					file3:write("\t\tnomail: true\n")
				end
				if (band(s[i].trade_flag,256)>0) then
					file3:write("\t\tnoauction: true\n")
				end
				file3:write("\t}\n")
			end
			if (check_val(s[i].nouse_flag)~="" and check_val(s[i].nouse_override)~="" ) then
				file3:write("\tNouse: {\n")
				if (tonumber(s[i].nouse_override) > 0 and tonumber(s[i].nouse_override) < 100) then
					file3:write("\t\toverride: ".. s[i].nouse_override .."\n")
				end
				if (band(s[i].nouse_flag,1)>0) then
					file3:write("\t\tsitting: true\n")
				end
				file3:write("\t}\n")
			end
			if check_val((s[i].Stack1)~="" and check_val(s[i].Stack2)~="" ) then
				file3:write("\tStack: ["..(s[i].Stack1)..", "..(s[i].Stack2).."]\n")
			end
			if (check_val(s[i].Sprite)~="" ) then
				file3:write("\tSprite: "..(s[i].Sprite).."\n")
			end
			if (s[i].Script ~= "") then
				file3:write("\tScript: <\""..newline(s[i].Script,1)..""..s[i].Script.."\">\n")
			end
			if (s[i].OnEquipScript ~= "") then
				file3:write("\tOnEquipScript: <\""..newline(s[i].OnEquipScript,1)..""..s[i].OnEquipScript.."\">\n")
			end
			if (s[i].OnUnequipScript ~= "") then
				file3:write("\tOnUnequipScript: <\""..newline(s[i].OnUnequipScript,1)..""..s[i].OnUnequipScript.."\">\n")
			end
			file3:write("},\n")
		end
	end
end

for i=1,7 do
	if (file_exists(arg[i])==false) then
		print("Error: ",arg[i]," Not Found")
		os.exit()
	end
end

file = assert(io.open(arg[1]))	--Open ItemDB And Read its contents
contents = file:read'*a'
file:close()

--Replacing Contents
find =    {  '\\',   '\'', '//',  '/%*',  '%*/',  '<\"',   '\">', 'Id([^:]?):([^\n]+)', 'AegisName([^:]?):([^\n]+)', 'Name([^:]?):([^\n]+)', 'Type([^:]?):([^\n]+)', 'Buy([^:]?):([^\n]+)', 'Sell([^:]?):([^\n]+)', 'Weight([^:]?):([^\n]+)', 'Atk([^:]?):([^\n]+)', 'Matk([^:]?):([^\n]+)', 'Def([^:]?):([^\n]+)', 'Range([^:]?):([^\n]+)', 'Slots([^:]?):([^\n]+)', 'Job([^:]?):([^\n]+)', 'Upper([^:]?):([^\n]+)', 'Gender([^:]?):([^\n]+)', 'Loc([^:]?):([^\n]+)', 'WeaponLv([^:]?):([^\n]+)', 'EquipLv([^:]?):([^\n]+)', 'Refine([^:]?):([^\n]+)', 'View([^:]?):([^\n]+)', 'BindOnEquip([^:]?):([^\n]+)', "BuyingStore([^:]?):([^\n]+)", "Delay([^:]?):([^\n]+)", "Stack([^:]?): %[([^,]+), ([^]]+)%]", "Sprite([^:]?):([^\n]+)", 'override([^:]?):([^\n]+)', 'nodrop([^:]?):([^\n]+)', 'notrade([^:]?):([^\n]+)', 'partneroverride([^:]?):([^\n]+)', 'noselltonpc([^:]?):([^\n]+)', 'nocart([^:]?):([^\n]+)', 'nostorage([^:]?):([^\n]+)', 'nogstorage([^:]?):([^\n]+)', 'nomail([^:]?):([^\n]+)', 'noauction([^:]?):([^\n]+)', 'sitting([^:]?):([^\n]+)', 'Script([^:]?):', 'OnEquipScript([^:]?):', 'OnUnequipScript([^:]?):', 'EquipLv([^=]?)= %[([^,]+), ([^]]+)%],', 'EquipLv([^=]?)= ([^,]+),', 'item_db: %(',  '= ""',    '= \n', ',,'}
replace = {'\\\\', '\\\'', '--', '--[[', '--]]', '[==[', ']==],',           'Id%1=%2,',           'AegisName%1=%2,',           'Name%1=%2,',           'Type%1=%2,',           'Buy%1=%2,',           'Sell%1=%2,',           'Weight%1=%2,',           'Atk%1=%2,',           'Matk%1=%2,',           'Def%1=%2,',           'Range%1=%2,',           'Slots%1=%2,',           'Job%1=%2,',           'Upper%1=%2,',           'Gender%1=%2,',           'Loc%1=%2,',           'WeaponLv%1=%2,',           'EquipLv%1=%2,',           'Refine%1=%2,',           'View%1=%2,',           'BindOnEquip%1=%2,',           "BuyingStore%1=%2,",           "Delay%1=%2,",                 "Stack%1= {%2, %3},",           "Sprite%1=%2,",           'override%1=%2,',           'nodrop%1=%2,',           'notrade%1=%2,',           'partneroverride%1=%2,',           'noselltonpc%1=%2,',           'nocart%1=%2,',           'nostorage%1=%2,',           'nogstorage%1=%2,',           'nomail%1=%2,',           'noauction%1=%2,',           'sitting%1=%2,',      'Script%1=',      'OnEquipScript%1=',      'OnUnequipScript%1=',                  'EquipLv%1= {%2, %3},',         'EquipLv%1= {%2},', 'item_db = {', '= "",', '= "",\n', ',' }
findk=0

if #find==#replace then	--Check if both table matches
	start_time = os.time()
	print("-- Putting ",arg[1]," Items into Memory")
	--Replace Every Strings
	contents = contents:gsub("Nouse: {([^}]+)}","Nouse= {%1},")	--It needs to be done first.
	contents = contents:gsub("Trade: {([^}]+)}","Trade= {%1},")	--It needs to be done first.
	for i=1,#find do
		contents = contents:gsub(find[i],replace[i])
	end
	
	--Check for last ) in item_db (it auto breaks if found, can have many empty lines on end)
	for i=1,1000000 do
		if (string.sub(contents,-i,-i) == ")") then
			findk = 1
			contents = replace_char(contents:len()-i+1,contents,'}')
		end
		if (findk==1) then break end
	end
else
	print('Error, Cannot Convert to Lua(Code:1['..#find..' '..#replace..'])')
	os.exit()
end
item_db = {}	--Initialize Empty item_db table
a = assert(loadstring(contents))	--Load the string as lua code
a()	--Execute it so it gets loaded into item_db table
e_time = os.difftime((os.time())-start_time)	--Time Taken
print("---- Total Items Found"," = ",#item_db,"(".. e_time .."s)")
csv = {}
item_db2 = {}

for i=1,#item_db do
	item_db2[item_db[i].Id] = deepcopy(item_db[i])
end


--Initializing Global Tables
csv = {}
csv1 = {}
csv2 = {}
csv3 = ""
csv4 = 0
--sql_update = ""
k=0
line = ""

--file4 = io.open(arg[9],"w")
for m=2,7 do	--arg[2] to arg[7] = item_*.txt
	line_number = 0
	sp_file = assert(io.open(arg[m]))
	for line in sp_file:lines() do
		if ( line ~= "" and line:sub(1,2)~="//")  then	--If Line is comment or empty, don't parse it
			parse_line(line,m)
		end
	end
	print(arg[m],"Done")
	sp_file:close()
	--os.exit()
end
--file4:write(sql_update)
--file4:close()
print("-- Writing Item_DB")
file3 = io.open(arg[8],"w")	--Open Output File
file3:write('item_db: (\n'
	..'//  Items Database\n'
	..'//\n'
	..'/******************************************************************************\n'
	..' ************* Entry structure ************************************************\n'
	..' ******************************************************************************\n'
	..'{\n'
	..'	// =================== Mandatory fields ===============================\n'
	..'	Id: ID                        (int)\n'
	..'	AegisName: "Aegis_Name"       (string)\n'
	..'	Name: "Item Name"             (string)\n'
	..'	// =================== Optional fields ================================\n'
	..'	Type: Item Type               (int, defaults to 3 = etc item)\n'
	..'	Buy: Buy Price                (int, defaults to Sell * 2)\n'
	..'	Sell: Sell Price              (int, defaults to Buy / 2)\n'
	..'	Weight: Item Weight           (int, defaults to 0)\n'
	..'	Atk: Attack                   (int, defaults to 0)\n'
	..'	Matk: Magical Attack          (int, defaults to 0, ignored in pre-re)\n'
	..'	Def: Defense                  (int, defaults to 0)\n'
	..'	Range: Attack Range           (int, defaults to 0)\n'
	..'	Slots: Slots                  (int, defaults to 0)\n'
	..'	Job: Job mask                 (int, defaults to all jobs = 0xFFFFFFFF)\n'
	..'	Upper: Upper mask             (int, defaults to any = 0x3f)\n'
	..'	Gender: Gender                (int, defaults to both = 2)\n'
	..'	Loc: Equip location           (int, required value for equipment)\n'
	..'	WeaponLv: Weapon Level        (int, defaults to 0)\n'
	..'	EquipLv: Equip required level (int, defaults to 0)\n'
	..'	EquipLv: [min, max]           (alternative syntax with min / max level)\n'
	..'	Refine: Refineable            (boolean, defaults to true)\n'
	..'	View: View ID                 (int, defaults to 0)\n'
	..'	BindOnEquip: true/false       (boolean, defaults to false)\n'
	..'	BuyingStore: true/false       (boolean, defaults to false)\n'
	..'	Delay: Delay to use item      (int, defaults to 0)\n'
	..'	Trade: {                      (defaults to no restrictions)\n'
	..'		override: GroupID             (int, defaults to 100)\n'
	..'		nodrop: true/false            (boolean, defaults to false)\n'
	..'		notrade: true/false           (boolean, defaults to false)\n'
	..'		partneroverride: true/false   (boolean, defaults to false)\n'
	..'		noselltonpc: true/false       (boolean, defaults to false)\n'
	..'		nocart: true/false            (boolean, defaults to false)\n'
	..'		nostorage: true/false         (boolean, defaults to false)\n'
	..'		nogstorage: true/false        (boolean, defaults to false)\n'
	..'		nomail: true/false            (boolean, defaults to false)\n'
	..'		noauction: true/false         (boolean, defaults to false)\n'
	..'	}\n'
	..'	Nouse: {                      (defaults to no restrictions)\n'
	..'		override: GroupID             (int, defaults to 100)\n'
	..'		sitting: true/false           (boolean, defaults to false)\n'
	..'	}\n'
	..'	Stack: [amount, type]         (int, defaults to 0)\n'
	..'	Sprite: SpriteID              (int, defaults to 0)\n'
	..'	Script: <"\n'
	..'		Script\n'
	..'		(it can be multi-line)\n'
	..'	">\n'
	..'	OnEquipScript: <" OnEquip Script (can also be multi-line) ">\n'
	..'	OnUnequipScript: <" OnUnequip Script (can also be multi-line) ">\n'
	..'},\n'
	..'******************************************************************************/\n')

item_db_write(item_db2)
file3:write('\n)\n')
file3:close()	--Close Output File
os.exit()
