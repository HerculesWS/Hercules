# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2018  Hercules Dev Team
# Copyright (C) 2018  Kubix
#
# Hercules is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# This script will convert old-styled HERCULES item_db.conf to new style.
import os
import io
import libconf
import argparse

ALL_JOBS = 0xFFFFFFFF
ALL_EXCEPT_NOVICE = 0xFFFFFFFE


def item_get_upper_mask(upper: int):
    if not upper or upper == 63:
        return ''

    item_class_upper = (
        (0x00, 0, 'ITEMUPPER_NONE'),
        (0x01, 1, 'ITEMUPPER_NORMAL'),
        (0x02, 2, 'ITEMUPPER_UPPER'),
        (0x04, 4, 'ITEMUPPER_BABY'),
        (0x08, 8, 'ITEMUPPER_THIRD'),
        (0x10, 16, 'ITEMUPPER_THIRDUPPER'),
        (0x20, 32, 'ITEMUPPER_THIRDBABY'),
        (0x3f, 63, 'ITEMUPPER_ALL')
    )

    for item_upper in item_class_upper:
        if upper in item_upper:
            return item_upper[2]
    return ''


def item_get_gender(sex: int):
    if sex == 2:
        return ''

    genders = (
        (0, 'SEX_FEMALE'),
        (1, 'SEX_MALE'),
        (2, 'SEX_ANY')
    )

    for gender in genders:
        if sex in gender:
            return gender[1]
    return ''


def item_type_formatter(item_type: int):
    if item_type == 3:
        return ''

    types = (
        (0, 'IT_HEALING'),
        (2, 'IT_USABLE'),
        (3, 'IT_ETC'),
        (4, 'IT_WEAPON'),
        (5, 'IT_ARMOR'),
        (6, 'IT_CARD'),
        (7, 'IT_PETEGG'),
        (8, 'IT_PETARMOR'),
        (10, 'IT_AMMO'),
        (11, 'IT_DELAYCONSUME'),
        (18, 'IT_CASH')
    )
    for type_ in types:
        if item_type in type_:
            return type_[1]
    return ''


def item_get_job_const(job: int):
    job_names = (
        "Novice",
        "Swordsman",
        "Magician",
        "Archer",
        "Acolyte",
        "Merchant",
        "Thief",
        "Knight",
        "Priest",
        "Wizard",
        "Blacksmith",
        "Hunter",
        "Assassin",
        "Unused",
        "Crusader",
        "Monk",
        "Sage",
        "Rogue",
        "Alchemist",
        "Bard",
        "Unused",
        "Taekwon",
        "Star_Gladiator",
        "Soul_Linker",
        "Gunslinger",
        "Ninja",
        "Gangsi",
        "Death_Knight",
        "Dark_Collector",
        "Kagerou",
        "Rebellion",
        "Summoner"
    )
    job_mask = int(hex(job), 16)
    result = ""
    if job_mask == '':
        return "\n"

    if job_mask & ALL_JOBS == ALL_JOBS:
        return "\t\tAll: true\n"
    if job_mask & ALL_EXCEPT_NOVICE == ALL_EXCEPT_NOVICE:
        result = "\t\tAll: true\n"
        result = result + "\t\tNovice: false\n"
        return result

    for x, job in enumerate(job_names):
        current_bit = 1 << x
        if job_mask & current_bit == current_bit:
            result += '\t\t{job}: true\n'.format(job=job_names[x])
    return result


def item_get_trade_options(trade):
    if not trade:
        return ''
    result = ''
    for trade_opt in trade.items():
        result += '\t\t{key}: {value}\n'.format(key=trade_opt[0], value=str(trade_opt[1]).lower())
    return result


def item_get_nouse_options(nouse):
    if not nouse:
        return ''

    result = ''
    for nouse_opt in nouse.items():
        result += '\t\t{key}: {value}\n'.format(key=nouse_opt[0], value=str(nouse_opt[1]).lower())
    return result


def get_optional_fields(item):
    # Check all fields
    # if no old field found just don't put it in new file
    optional_fields = ''
    item_type = item_type_formatter(item.get('Type'))
    buy = item.get('Buy')
    sell = item.get('Sell')
    weight = item.get('Weight')
    atk = item.get('Atk')
    matk = item.get('Matk')
    defense = item.get('Def')
    range_ = item.get('Range')
    slots = item.get('Slots')
    job = item_get_job_const(item.get('Job', 0))
    upper = item_get_upper_mask(item.get('Upper', None))
    gender = item_get_gender(item.get('Gender'))
    loc = item.get('Loc')
    weapon_lv = item.get('WeaponLv')
    equip_lv = item.get('EquipLv')
    refine = item.get('Refine')
    disable_options = item.get('DisableOptions')
    subtype = item.get('Subtype')
    view_sprite = item.get('ViewSprite')
    bind_on_equip = item.get('BindOnEquip')
    force_serial = item.get('ForceSerial')
    buying_store = item.get('BuyingStore')
    delay = item.get('Delay')
    keep_after_use = item.get('KeepAfterUse')
    drop_announce = item.get('DropAnnounce')
    show_drop_effect = item.get('ShowDropEffect')
    drop_effect_mode = item.get('DropEffectMode')
    trade = item_get_trade_options(item.get('Trade'))
    nouse = item_get_nouse_options(item.get('Nouse'))
    stack = item.get('Stack')
    sprite = item.get('Sprite')
    script = item.get('Script')
    on_equip_script = item.get('OnEquipScript')
    on_unequip_script = item.get('OnUnequipScript')

    if item_type:
        optional_fields += '\tItemType: "{type}"\n'.format(type=item_type)
    if buy:
        optional_fields += '\tBuy: {buy}\n'.format(buy=buy)
    if sell:
        optional_fields += '\tSell: {sell}\n'.format(sell=sell)
    if weight:
        optional_fields += '\tWeight: {weight}\n'.format(weight=weight)
    if atk:
        optional_fields += '\tAtk: {atk}\n'.format(atk=atk)
    if matk:
        optional_fields += '\tMatk: {matk}\n'.format(matk=matk)
    if defense:
        optional_fields += '\tDef: {defense}\n'.format(defense=defense)
    if range_:
        optional_fields += '\tRange: {rng}\n'.format(rng=range_)
    if slots:
        optional_fields += '\tSlots: {slots}\n'.format(slots=slots)
    if job:
        jobs = '\tJob: {\n'
        jobs = jobs + '{jobs}'.format(jobs=job)
        optional_fields += jobs + '\t}\n'
    if upper:
        optional_fields += '\tUpper: "{upper}"\n'.format(upper=upper)
    if gender:
        optional_fields += '\tGender: "{gender}"\n'.format(gender=gender)
    if loc:
        optional_fields += '\tLoc: {loc}\n'.format(loc=loc)
    if weapon_lv:
        optional_fields += '\tWeaponLv: {weapon_lv}\n'.format(weapon_lv=weapon_lv)
    if equip_lv:
        optional_fields += '\tEquipLv: {equip_lv}\n'.format(equip_lv=equip_lv)
    if refine:
        optional_fields += '\tRefine: {refine}\n'.format(refine=refine)
    if disable_options:
        optional_fields += '\tDisableOptions: {disable_options}\n'.format(disable_options=disable_options)
    if subtype:
        optional_fields += '\tSubtype: {subtype}\n'.format(subtype=subtype)
    if view_sprite:
        optional_fields += '\tViewSprite: {view_sprite}\n'.format(view_sprite=view_sprite)
    if bind_on_equip:
        optional_fields += '\tBindOnEquip: {bind}\n'.format(bind=bind_on_equip)
    if force_serial:
        optional_fields += '\tForceSerial: {serial}\n'.format(serial=force_serial)
    if buying_store:
        optional_fields += '\tBuyingStore: {store}\n'.format(store=buying_store)
    if delay:
        optional_fields += '\tDelay: {delay}\n'.format(delay=delay)
    if keep_after_use:
        optional_fields += '\tKeepAfterUse: {keep}\n'.format(keep=keep_after_use)
    if drop_announce:
        optional_fields += '\tDropAnnounce: {announce}\n'.format(announce=drop_announce)
    if show_drop_effect:
        optional_fields += '\tShowDropEffect: {show_drop}\n'.format(show_drop=show_drop_effect)
    if drop_effect_mode:
        optional_fields += '\tDropEffectMode: {drop_effect}\n'.format(drop_effect=drop_effect_mode)
    if trade:
        trade_opt = '\tTrade: {\n'
        trade_opt = trade_opt + '{trade}'.format(trade=trade)
        optional_fields += trade_opt + '\t}\n'
    if nouse:
        nouse_opt = '\tNouse: {\n'
        nouse_opt = nouse_opt + '{nouse}'.format(nouse=nouse)
        optional_fields += nouse_opt + '\t}\n'
    if stack:
        optional_fields += '\tStack: {stack}\n'.format(stack=stack)
    if sprite:
        optional_fields += '\tSprite: {sprite}\n'.format(sprite=sprite)
    if script:
        optional_fields += '\tScript: <{script}>\n'.format(script=script)
    if on_equip_script:
        optional_fields += '\tOnEquipScript: <{script}>\n'.format(script=on_equip_script)
    if on_unequip_script:
        optional_fields += '\tOnUnequipScript: <{script}>\n'.format(script=on_equip_script)
    return optional_fields


def item_db_formatter(args, db_name: str = 'item_db'):
    # Load old .conf file
    try:
        with io.open(args.old_path, 'r') as file:
            config = libconf.load(file)
    except FileNotFoundError:
        print(f'old_path: {args.old_path} file not found')
        return

    db = config[db_name]
    formatted_conf = ''
    for x, item in enumerate(db):
        # Mandatory fields
        item_id = db[x].Id
        aegis_name = db[x].AegisName
        name = db[x].Name
        # Optional fields
        optional_fields = get_optional_fields(item)

        header = '{\n'
        footer = '},\n'

        item_id = '\tId: {id}\n'.format(id=item_id)
        aegis_name = '\tAegisName: "{aegis_name}"\n'.format(aegis_name=aegis_name)
        name = '\tName: "{name}"\n'.format(name=name)
        field = header + item_id + aegis_name + name + optional_fields + footer
        formatted_conf += field

    # Check if new path is correct
    if os.path.exists(args.new_path):
        # Create new .conf file
        with io.open(args.new_path, 'w') as file:
            file.write(formatted_conf)
    else:
        print(f'new_path: {args.new_path} is not correct path')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Input old and new config path')
    parser.add_argument('old_path', type=str, help='old config file path [example: /home/User/old_item_db.conf]')
    parser.add_argument('new_path', type=str, help='new config file path [example: /home/User/item_db.conf]')
    parsed_args = parser.parse_args()
    item_db_formatter(parsed_args)
