/*
 * Copyright (C) 2016 Muhammad Tayyab Akram
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _SF_JOINING_TYPE_H
#define _SF_JOINING_TYPE_H

#include <SFTypes.h>

enum {
    SFJoiningTypeNil = 0x00,
    SFJoiningTypeU   = 0x01, /**< Non Joining */
    SFJoiningTypeL   = 0x02, /**< Left Joining */
    SFJoiningTypeR   = 0x03, /**< Right Joining */
    SFJoiningTypeD   = 0x04, /**< Dual Joining */
    SFJoiningTypeC   = 0x05, /**< Join Causing */
    SFJoiningTypeT   = 0x06, /**< Transparent */
    SFJoiningTypeF   = 0x07  /**< Unavailable */
};
typedef SFUInt8 SFJoiningType;

#endif
