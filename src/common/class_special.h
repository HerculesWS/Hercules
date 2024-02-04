/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2024 Hercules Dev Team
 * Copyright (C) Athena Dev Teams
 *
 * Hercules is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// JOB_ENUM_VALUE(JOB_CONSTANT, JOB_ID, MSGTBL_CONSTANT)
// - JOB_CONSTANT - The name of the constant representing this job ID. This will generate JOB_ and MAPID_ constants
//                  (e.g. NOVICE --> generates JOB_NOVICE and MAPID_NOVICE)
// - JOB_ID - Official numerical ID value of the job, as expected by the client
// - MSGTBL_CONSTANT - Constant for the Job name in messages.conf, should be a msgtable.h enum value WITHOUT MSGTBL_ prefix.
//                  (e.g. JOB_NOVICE --> becomes MSGTBL_JOB_NOVICE)

JOB_ENUM_VALUE(BARD, 19, JOB_BARD)
JOB_ENUM_VALUE(DANCER, 20, JOB_DANCER)
JOB_ENUM_VALUE(MAX_BASIC, 28, UNKNOWN_JOB)
JOB_ENUM_VALUE(2004_BEGIN, 4000, UNKNOWN_JOB)
JOB_ENUM_VALUE(CLOWN, 4020, JOB_CLOWN)
JOB_ENUM_VALUE(GYPSY, 4021, JOB_GYPSY)
JOB_ENUM_VALUE(KAGEROU, 4211, JOB_KAGEROU)
JOB_ENUM_VALUE(OBORO, 4212, JOB_OBORO)
JOB_ENUM_VALUE(BABY_KAGEROU, 4223, JOB_BABY_KAGEROU)
JOB_ENUM_VALUE(BABY_OBORO, 4224, JOB_BABY_OBORO)
JOB_ENUM_VALUE(BABY_BARD, 4042, JOB_BABY_BARD)
JOB_ENUM_VALUE(BABY_DANCER, 4043, JOB_BABY_DANCER)
JOB_ENUM_VALUE(MINSTREL, 4068, JOB_MINSTREL)
JOB_ENUM_VALUE(WANDERER, 4069, JOB_WANDERER)
JOB_ENUM_VALUE(MINSTREL_T, 4075, JOB_MINSTREL_T)
JOB_ENUM_VALUE(WANDERER_T, 4076, JOB_WANDERER_T)
JOB_ENUM_VALUE(BABY_MINSTREL, 4104, JOB_BABY_MINSTREL)
JOB_ENUM_VALUE(BABY_WANDERER, 4105, JOB_BABY_WANDERER)
