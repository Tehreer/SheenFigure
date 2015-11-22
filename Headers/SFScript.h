/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SF_SCRIPT_H
#define SF_SCRIPT_H

#include "SFTypes.h"

/**
 * Referance: https://www.microsoft.com/typography/otspec/scripttags.htm
 */

enum {
    SFScriptARAB = SF_TAG__MAKE('a', 'r', 'a', 'b'),
    SFScriptARMN = SF_TAG__MAKE('a', 'r', 'm', 'n'),
    SFScriptAVST = SF_TAG__MAKE('a', 'v', 's', 't'),
    SFScriptBALI = SF_TAG__MAKE('b', 'a', 'l', 'i'),
    SFScriptBAMU = SF_TAG__MAKE('b', 'a', 'm', 'u'),
    SFScriptBATK = SF_TAG__MAKE('b', 'a', 't', 'k'),
    SFScriptBENG = SF_TAG__MAKE('b', 'e', 'n', 'g'),
    SFScriptBNG2 = SF_TAG__MAKE('b', 'n', 'g', '2'),
    SFScriptBOPO = SF_TAG__MAKE('b', 'o', 'p', 'o'),
    SFScriptBRAI = SF_TAG__MAKE('b', 'r', 'a', 'i'),
    SFScriptBRAH = SF_TAG__MAKE('b', 'r', 'a', 'h'),
    SFScriptBUGI = SF_TAG__MAKE('b', 'u', 'g', 'i'),
    SFScriptBUHD = SF_TAG__MAKE('b', 'u', 'h', 'd'),
    SFScriptBYZM = SF_TAG__MAKE('b', 'y', 'z', 'm'),
    SFScriptCANS = SF_TAG__MAKE('c', 'a', 'n', 's'),
    SFScriptCARI = SF_TAG__MAKE('c', 'a', 'r', 'i'),
    SFScriptCAKM = SF_TAG__MAKE('c', 'a', 'k', 'm'),
    SFScriptCHAM = SF_TAG__MAKE('c', 'h', 'a', 'm'),
    SFScriptCHER = SF_TAG__MAKE('c', 'h', 'e', 'r'),
    SFScriptHANI = SF_TAG__MAKE('h', 'a', 'n', 'i'),
    SFScriptCOPT = SF_TAG__MAKE('c', 'o', 'p', 't'),
    SFScriptCPRT = SF_TAG__MAKE('c', 'p', 'r', 't'),
    SFScriptCYRL = SF_TAG__MAKE('c', 'y', 'r', 'l'),
    SFScriptDFLT = SF_TAG__MAKE('D', 'F', 'L', 'T'),
    SFScriptDSRT = SF_TAG__MAKE('d', 's', 'r', 't'),
    SFScriptDEVA = SF_TAG__MAKE('d', 'e', 'v', 'a'),
    SFScriptDEV2 = SF_TAG__MAKE('d', 'e', 'v', '2'),
    SFScriptEGYP = SF_TAG__MAKE('e', 'g', 'y', 'p'),
    SFScriptETHI = SF_TAG__MAKE('e', 't', 'h', 'i'),
    SFScriptGEOR = SF_TAG__MAKE('g', 'e', 'o', 'r'),
    SFScriptGLAG = SF_TAG__MAKE('g', 'l', 'a', 'g'),
    SFScriptGOTH = SF_TAG__MAKE('g', 'o', 't', 'h'),
    SFScriptGREK = SF_TAG__MAKE('g', 'r', 'e', 'k'),
    SFScriptGUJR = SF_TAG__MAKE('g', 'u', 'j', 'r'),
    SFScriptGJR2 = SF_TAG__MAKE('g', 'j', 'r', '2'),
    SFScriptGURU = SF_TAG__MAKE('g', 'u', 'r', 'u'),
    SFScriptGUR2 = SF_TAG__MAKE('g', 'u', 'r', '2'),
    SFScriptHANG = SF_TAG__MAKE('h', 'a', 'n', 'g'),
    SFScriptJAMO = SF_TAG__MAKE('j', 'a', 'm', 'o'),
    SFScriptHANO = SF_TAG__MAKE('h', 'a', 'n', 'o'),
    SFScriptHEBR = SF_TAG__MAKE('h', 'e', 'b', 'r'),
    SFScriptARMI = SF_TAG__MAKE('a', 'r', 'm', 'i'),
    SFScriptPHLI = SF_TAG__MAKE('p', 'h', 'l', 'i'),
    SFScriptPRTI = SF_TAG__MAKE('p', 'r', 't', 'i'),
    SFScriptJAVA = SF_TAG__MAKE('j', 'a', 'v', 'a'),
    SFScriptKTHI = SF_TAG__MAKE('k', 't', 'h', 'i'),
    SFScriptKNDA = SF_TAG__MAKE('k', 'n', 'd', 'a'),
    SFScriptKND2 = SF_TAG__MAKE('k', 'n', 'd', '2'),
    SFScriptKANA = SF_TAG__MAKE('k', 'a', 'n', 'a'),
    SFScriptKALI = SF_TAG__MAKE('k', 'a', 'l', 'i'),
    SFScriptKHAR = SF_TAG__MAKE('k', 'h', 'a', 'r'),
    SFScriptKHMR = SF_TAG__MAKE('k', 'h', 'm', 'r'),
    SFScriptLAO  = SF_TAG__MAKE('l', 'a', 'o', ' '),
    SFScriptLATN = SF_TAG__MAKE('l', 'a', 't', 'n'),
    SFScriptLEPC = SF_TAG__MAKE('l', 'e', 'p', 'c'),
    SFScriptLIMB = SF_TAG__MAKE('l', 'i', 'm', 'b'),
    SFScriptLINB = SF_TAG__MAKE('l', 'i', 'n', 'b'),
    SFScriptLISU = SF_TAG__MAKE('l', 'i', 's', 'u'),
    SFScriptLYCI = SF_TAG__MAKE('l', 'y', 'c', 'i'),
    SFScriptLYDI = SF_TAG__MAKE('l', 'y', 'd', 'i'),
    SFScriptMLYM = SF_TAG__MAKE('m', 'l', 'y', 'm'),
    SFScriptMLM2 = SF_TAG__MAKE('m', 'l', 'm', '2'),
    SFScriptMAND = SF_TAG__MAKE('m', 'a', 'n', 'd'),
    SFScriptMATH = SF_TAG__MAKE('m', 'a', 't', 'h'),
    SFScriptMTEI = SF_TAG__MAKE('m', 't', 'e', 'i'),
    SFScriptMERC = SF_TAG__MAKE('m', 'e', 'r', 'c'),
    SFScriptMERO = SF_TAG__MAKE('m', 'e', 'r', 'o'),
    SFScriptMONG = SF_TAG__MAKE('m', 'o', 'n', 'g'),
    SFScriptMUSC = SF_TAG__MAKE('m', 'u', 's', 'c'),
    SFScriptMYMR = SF_TAG__MAKE('m', 'y', 'm', 'r'),
    SFScriptTALU = SF_TAG__MAKE('t', 'a', 'l', 'u'),
    SFScriptNKO  = SF_TAG__MAKE('n', 'k', 'o', ' '),
    SFScriptOGAM = SF_TAG__MAKE('o', 'g', 'a', 'm'),
    SFScriptOLCK = SF_TAG__MAKE('o', 'l', 'c', 'k'),
    SFScriptITAL = SF_TAG__MAKE('i', 't', 'a', 'l'),
    SFScriptXPEO = SF_TAG__MAKE('x', 'p', 'e', 'o'),
    SFScriptSARB = SF_TAG__MAKE('s', 'a', 'r', 'b'),
    SFScriptORKH = SF_TAG__MAKE('o', 'r', 'k', 'h'),
    SFScriptORYA = SF_TAG__MAKE('o', 'r', 'y', 'a'),
    SFScriptORY2 = SF_TAG__MAKE('o', 'r', 'y', '2'),
    SFScriptOSMA = SF_TAG__MAKE('o', 's', 'm', 'a'),
    SFScriptPHAG = SF_TAG__MAKE('p', 'h', 'a', 'g'),
    SFScriptPHNX = SF_TAG__MAKE('p', 'h', 'n', 'x'),
    SFScriptRJNG = SF_TAG__MAKE('r', 'j', 'n', 'g'),
    SFScriptRUNR = SF_TAG__MAKE('r', 'u', 'n', 'r'),
    SFScriptSAMR = SF_TAG__MAKE('s', 'a', 'm', 'r'),
    SFScriptSAUR = SF_TAG__MAKE('s', 'a', 'u', 'r'),
    SFScriptSHRD = SF_TAG__MAKE('s', 'h', 'r', 'd'),
    SFScriptSHAW = SF_TAG__MAKE('s', 'h', 'a', 'w'),
    SFScriptSINH = SF_TAG__MAKE('s', 'i', 'n', 'h'),
    SFScriptSORA = SF_TAG__MAKE('s', 'o', 'r', 'a'),
    SFScriptXSUX = SF_TAG__MAKE('x', 's', 'u', 'x'),
    SFScriptSUND = SF_TAG__MAKE('s', 'u', 'n', 'd'),
    SFScriptSYLO = SF_TAG__MAKE('s', 'y', 'l', 'o'),
    SFScriptSYRC = SF_TAG__MAKE('s', 'y', 'r', 'c'),
    SFScriptTGLG = SF_TAG__MAKE('t', 'g', 'l', 'g'),
    SFScriptTAGB = SF_TAG__MAKE('t', 'a', 'g', 'b'),
    SFScriptTALE = SF_TAG__MAKE('t', 'a', 'l', 'e'),
    SFScriptLANA = SF_TAG__MAKE('l', 'a', 'n', 'a'),
    SFScriptTAVT = SF_TAG__MAKE('t', 'a', 'v', 't'),
    SFScriptTAKR = SF_TAG__MAKE('t', 'a', 'k', 'r'),
    SFScriptTAML = SF_TAG__MAKE('t', 'a', 'm', 'l'),
    SFScriptTML2 = SF_TAG__MAKE('t', 'm', 'l', '2'),
    SFScriptTELU = SF_TAG__MAKE('t', 'e', 'l', 'u'),
    SFScriptTEL2 = SF_TAG__MAKE('t', 'e', 'l', '2'),
    SFScriptTHAA = SF_TAG__MAKE('t', 'h', 'a', 'a'),
    SFScriptTHAI = SF_TAG__MAKE('t', 'h', 'a', 'i'),
    SFScriptTIBT = SF_TAG__MAKE('t', 'i', 'b', 't'),
    SFScriptTFNG = SF_TAG__MAKE('t', 'f', 'n', 'g'),
    SFScriptUGAR = SF_TAG__MAKE('u', 'g', 'a', 'r'),
    SFScriptVAI  = SF_TAG__MAKE('v', 'a', 'i', ' '),
    SFScriptYI   = SF_TAG__MAKE('y', 'i', ' ', ' ')
};
typedef SF_TAG SFScript;

#endif
