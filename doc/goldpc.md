<!--
> This file is part of Hercules.
> http://herc.ws - http://github.com/HerculesWS/Hercules
> 
> Copyright (C) 2023-2023 Hercules Dev Team
> Copyright (C) KirieZ
> 
> Hercules is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
> 
> This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
> See the GNU General Public License for more details.
>
> You should have received a copy of the GNU General Public License along with this program.  
> If not, see <http://www.gnu.org/licenses/>.
-->

# GoldPC system (also known as Mileage)
> GoldPC is disabled by default, see [Enabling](#enabling) section to learn how to enable it.

The GoldPC system gives player points by spending time online. Time is tracked regardless of what the player is doing,
and will stop once the player logs out, either in a normal logout or due to starting autotrade.

Played time will be stored, and on the next log in, it will resume from where it was before.

GoldPC may be applied differently for each player by using different "modes", specified in `db/goldpc_db.conf`.

Client will render a small UI to tell the user how much time they still have to play before being rewarded with points.
They may also click in this UI to summon the Gold Points Manager NPC, which allows player to exchange their points.

By default, points may be manipulated by script using the `#GOLDPC_POINTS` account variable.

**Note:** By default, point and time is tracked per account.

> Check out [Customizing](#customizing) section for details on how to use different modes and configure rewards.

## Requirements
This feature requires client version 2014-06-11 or newer.

> Although the packets were present in previous versions, this is the version where it actually got released.

## Enabling
In order to use GoldPC system, you must enable it in the feature configs (`conf/feature.conf`) by
setting `enable` to `true` under `goldpc` settings.

Additionally, you may specify which GoldPC mode should be turned on by default using `default_mode` config.

Once GoldPC is enabled, players who log in will already be able to use GoldPC basic functionalities,
where everybody will use the same GoldPC mode and will be able to call the point exchange NPC.

> If you want to do further customization, checkout [Customizing](#customizing) section.

## Customizing
There are a few ways you can customize GoldPC system:
1. Change and create modes
2. Change modes dynamically (e.g. per player)
3. Customize GoldPC button click and rewards

Depending on what you want to do, a change in the `db` folder may be enough, for more elaborate changes
you may need to do some scripting or source changes.

This section covers those topics with some examples.

### Change and create modes
Modes are defined on `db/goldpc_db.conf`, each mode represents a pair of Time and Points rewarded at the end of it.

You are allowed to give any amount of points up to `GOLDPC_MAX_POINTS` (default: 300)
and set any amount of time, in seconds, below `GOLDPC_MAX_TIME` (default: 3600 / 1 hour).

`Const` field is meant for use in scripts, and should be a unique value as it will become a script constant,
it is recommended to follow the pattern `GOLDPC_[name]` to avoid conflicts.

> Id is sent to the client, and creating new Ids may require client changes. But it is possible.

### Change modes dynamically
By default, every player who logs in will start in the default mode, continuing from the accumulated time they
had when they logged out. But this doesn't have to be this way!

You may use scripts to change a player's GoldPC mode into another one, and even tweak the time they had stored.
To do that, you will need the `setgoldpcmode` script command (Check `doc/script_commands.txt` for its documentation).

For example, let's say that you want to give players below level 30 a 2x bonus, by giving them the mode `GOLDPC_DOUBLE`.

> Note: this example only covers login check, if a player reaches level 30 and doesn't log out, they will still
> receive double points until they log out.

You may achieve that with the following script:
```
-	script	Login GoldPC	FAKE_NPC,{
	end;

OnPCLoginEvent:
	if (BaseLevel < 30)
		setgoldpcmode(GOLDPC_DOUBLE);
	end;
}
```

Upon log out, the mode change is reset, so once they log in being level 30 or above, they will be back to the default mode.

You can see more examples in `doc/sample/goldpc.txt`.

### Customize GoldPC button click and rewards
Clicking in the GoldPC button, by default, brings the Gold Points Manager NPC.
This is done (and may be customized) by changing `npc/other/dynamicnpc_create.txt`,
it will allow you to do whatever you want upon clicking in this button.

If you simply want to change the Gold Point Manager NPC, you can find it in `npc/other/goldpc.txt`.
