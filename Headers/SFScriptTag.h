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

#ifndef SF_SCRIPT_TAG_H
#define SF_SCRIPT_TAG_H

#include "SFTypes.h"

/**
 * Referance: https://www.microsoft.com/typography/otspec/scripttags.htm
 */

enum {
    SFScriptTagARAB = SF_TAG__MAKE('a', 'r', 'a', 'b'),
    SFScriptTagARMN = SF_TAG__MAKE('a', 'r', 'm', 'n'),
    SFScriptTagAVST = SF_TAG__MAKE('a', 'v', 's', 't'),
    SFScriptTagBALI = SF_TAG__MAKE('b', 'a', 'l', 'i'),
    SFScriptTagBAMU = SF_TAG__MAKE('b', 'a', 'm', 'u'),
    SFScriptTagBATK = SF_TAG__MAKE('b', 'a', 't', 'k'),
    SFScriptTagBENG = SF_TAG__MAKE('b', 'e', 'n', 'g'),
    SFScriptTagBNG2 = SF_TAG__MAKE('b', 'n', 'g', '2'),
    SFScriptTagBOPO = SF_TAG__MAKE('b', 'o', 'p', 'o'),
    SFScriptTagBRAI = SF_TAG__MAKE('b', 'r', 'a', 'i'),
    SFScriptTagBRAH = SF_TAG__MAKE('b', 'r', 'a', 'h'),
    SFScriptTagBUGI = SF_TAG__MAKE('b', 'u', 'g', 'i'),
    SFScriptTagBUHD = SF_TAG__MAKE('b', 'u', 'h', 'd'),
    SFScriptTagBYZM = SF_TAG__MAKE('b', 'y', 'z', 'm'),
    SFScriptTagCANS = SF_TAG__MAKE('c', 'a', 'n', 's'),
    SFScriptTagCARI = SF_TAG__MAKE('c', 'a', 'r', 'i'),
    SFScriptTagCAKM = SF_TAG__MAKE('c', 'a', 'k', 'm'),
    SFScriptTagCHAM = SF_TAG__MAKE('c', 'h', 'a', 'm'),
    SFScriptTagCHER = SF_TAG__MAKE('c', 'h', 'e', 'r'),
    SFScriptTagHANI = SF_TAG__MAKE('h', 'a', 'n', 'i'),
    SFScriptTagCOPT = SF_TAG__MAKE('c', 'o', 'p', 't'),
    SFScriptTagCPRT = SF_TAG__MAKE('c', 'p', 'r', 't'),
    SFScriptTagCYRL = SF_TAG__MAKE('c', 'y', 'r', 'l'),
    SFScriptTagDFLT = SF_TAG__MAKE('D', 'F', 'L', 'T'),
    SFScriptTagDSRT = SF_TAG__MAKE('d', 's', 'r', 't'),
    SFScriptTagDEVA = SF_TAG__MAKE('d', 'e', 'v', 'a'),
    SFScriptTagDEV2 = SF_TAG__MAKE('d', 'e', 'v', '2'),
    SFScriptTagEGYP = SF_TAG__MAKE('e', 'g', 'y', 'p'),
    SFScriptTagETHI = SF_TAG__MAKE('e', 't', 'h', 'i'),
    SFScriptTagGEOR = SF_TAG__MAKE('g', 'e', 'o', 'r'),
    SFScriptTagGLAG = SF_TAG__MAKE('g', 'l', 'a', 'g'),
    SFScriptTagGOTH = SF_TAG__MAKE('g', 'o', 't', 'h'),
    SFScriptTagGREK = SF_TAG__MAKE('g', 'r', 'e', 'k'),
    SFScriptTagGUJR = SF_TAG__MAKE('g', 'u', 'j', 'r'),
    SFScriptTagGJR2 = SF_TAG__MAKE('g', 'j', 'r', '2'),
    SFScriptTagGURU = SF_TAG__MAKE('g', 'u', 'r', 'u'),
    SFScriptTagGUR2 = SF_TAG__MAKE('g', 'u', 'r', '2'),
    SFScriptTagHANG = SF_TAG__MAKE('h', 'a', 'n', 'g'),
    SFScriptTagJAMO = SF_TAG__MAKE('j', 'a', 'm', 'o'),
    SFScriptTagHANO = SF_TAG__MAKE('h', 'a', 'n', 'o'),
    SFScriptTagHEBR = SF_TAG__MAKE('h', 'e', 'b', 'r'),
    SFScriptTagARMI = SF_TAG__MAKE('a', 'r', 'm', 'i'),
    SFScriptTagPHLI = SF_TAG__MAKE('p', 'h', 'l', 'i'),
    SFScriptTagPRTI = SF_TAG__MAKE('p', 'r', 't', 'i'),
    SFScriptTagJAVA = SF_TAG__MAKE('j', 'a', 'v', 'a'),
    SFScriptTagKTHI = SF_TAG__MAKE('k', 't', 'h', 'i'),
    SFScriptTagKNDA = SF_TAG__MAKE('k', 'n', 'd', 'a'),
    SFScriptTagKND2 = SF_TAG__MAKE('k', 'n', 'd', '2'),
    SFScriptTagKANA = SF_TAG__MAKE('k', 'a', 'n', 'a'),
    SFScriptTagKALI = SF_TAG__MAKE('k', 'a', 'l', 'i'),
    SFScriptTagKHAR = SF_TAG__MAKE('k', 'h', 'a', 'r'),
    SFScriptTagKHMR = SF_TAG__MAKE('k', 'h', 'm', 'r'),
    SFScriptTagLAO  = SF_TAG__MAKE('l', 'a', 'o', ' '),
    SFScriptTagLATN = SF_TAG__MAKE('l', 'a', 't', 'n'),
    SFScriptTagLEPC = SF_TAG__MAKE('l', 'e', 'p', 'c'),
    SFScriptTagLIMB = SF_TAG__MAKE('l', 'i', 'm', 'b'),
    SFScriptTagLINB = SF_TAG__MAKE('l', 'i', 'n', 'b'),
    SFScriptTagLISU = SF_TAG__MAKE('l', 'i', 's', 'u'),
    SFScriptTagLYCI = SF_TAG__MAKE('l', 'y', 'c', 'i'),
    SFScriptTagLYDI = SF_TAG__MAKE('l', 'y', 'd', 'i'),
    SFScriptTagMLYM = SF_TAG__MAKE('m', 'l', 'y', 'm'),
    SFScriptTagMLM2 = SF_TAG__MAKE('m', 'l', 'm', '2'),
    SFScriptTagMAND = SF_TAG__MAKE('m', 'a', 'n', 'd'),
    SFScriptTagMATH = SF_TAG__MAKE('m', 'a', 't', 'h'),
    SFScriptTagMTEI = SF_TAG__MAKE('m', 't', 'e', 'i'),
    SFScriptTagMERC = SF_TAG__MAKE('m', 'e', 'r', 'c'),
    SFScriptTagMERO = SF_TAG__MAKE('m', 'e', 'r', 'o'),
    SFScriptTagMONG = SF_TAG__MAKE('m', 'o', 'n', 'g'),
    SFScriptTagMUSC = SF_TAG__MAKE('m', 'u', 's', 'c'),
    SFScriptTagMYMR = SF_TAG__MAKE('m', 'y', 'm', 'r'),
    SFScriptTagTALU = SF_TAG__MAKE('t', 'a', 'l', 'u'),
    SFScriptTagNKO  = SF_TAG__MAKE('n', 'k', 'o', ' '),
    SFScriptTagOGAM = SF_TAG__MAKE('o', 'g', 'a', 'm'),
    SFScriptTagOLCK = SF_TAG__MAKE('o', 'l', 'c', 'k'),
    SFScriptTagITAL = SF_TAG__MAKE('i', 't', 'a', 'l'),
    SFScriptTagXPEO = SF_TAG__MAKE('x', 'p', 'e', 'o'),
    SFScriptTagSARB = SF_TAG__MAKE('s', 'a', 'r', 'b'),
    SFScriptTagORKH = SF_TAG__MAKE('o', 'r', 'k', 'h'),
    SFScriptTagORYA = SF_TAG__MAKE('o', 'r', 'y', 'a'),
    SFScriptTagORY2 = SF_TAG__MAKE('o', 'r', 'y', '2'),
    SFScriptTagOSMA = SF_TAG__MAKE('o', 's', 'm', 'a'),
    SFScriptTagPHAG = SF_TAG__MAKE('p', 'h', 'a', 'g'),
    SFScriptTagPHNX = SF_TAG__MAKE('p', 'h', 'n', 'x'),
    SFScriptTagRJNG = SF_TAG__MAKE('r', 'j', 'n', 'g'),
    SFScriptTagRUNR = SF_TAG__MAKE('r', 'u', 'n', 'r'),
    SFScriptTagSAMR = SF_TAG__MAKE('s', 'a', 'm', 'r'),
    SFScriptTagSAUR = SF_TAG__MAKE('s', 'a', 'u', 'r'),
    SFScriptTagSHRD = SF_TAG__MAKE('s', 'h', 'r', 'd'),
    SFScriptTagSHAW = SF_TAG__MAKE('s', 'h', 'a', 'w'),
    SFScriptTagSINH = SF_TAG__MAKE('s', 'i', 'n', 'h'),
    SFScriptTagSORA = SF_TAG__MAKE('s', 'o', 'r', 'a'),
    SFScriptTagXSUX = SF_TAG__MAKE('x', 's', 'u', 'x'),
    SFScriptTagSUND = SF_TAG__MAKE('s', 'u', 'n', 'd'),
    SFScriptTagSYLO = SF_TAG__MAKE('s', 'y', 'l', 'o'),
    SFScriptTagSYRC = SF_TAG__MAKE('s', 'y', 'r', 'c'),
    SFScriptTagTGLG = SF_TAG__MAKE('t', 'g', 'l', 'g'),
    SFScriptTagTAGB = SF_TAG__MAKE('t', 'a', 'g', 'b'),
    SFScriptTagTALE = SF_TAG__MAKE('t', 'a', 'l', 'e'),
    SFScriptTagLANA = SF_TAG__MAKE('l', 'a', 'n', 'a'),
    SFScriptTagTAVT = SF_TAG__MAKE('t', 'a', 'v', 't'),
    SFScriptTagTAKR = SF_TAG__MAKE('t', 'a', 'k', 'r'),
    SFScriptTagTAML = SF_TAG__MAKE('t', 'a', 'm', 'l'),
    SFScriptTagTML2 = SF_TAG__MAKE('t', 'm', 'l', '2'),
    SFScriptTagTELU = SF_TAG__MAKE('t', 'e', 'l', 'u'),
    SFScriptTagTEL2 = SF_TAG__MAKE('t', 'e', 'l', '2'),
    SFScriptTagTHAA = SF_TAG__MAKE('t', 'h', 'a', 'a'),
    SFScriptTagTHAI = SF_TAG__MAKE('t', 'h', 'a', 'i'),
    SFScriptTagTIBT = SF_TAG__MAKE('t', 'i', 'b', 't'),
    SFScriptTagTFNG = SF_TAG__MAKE('t', 'f', 'n', 'g'),
    SFScriptTagUGAR = SF_TAG__MAKE('u', 'g', 'a', 'r'),
    SFScriptTagVAI  = SF_TAG__MAKE('v', 'a', 'i', ' '),
    SFScriptTagYI   = SF_TAG__MAKE('y', 'i', ' ', ' ')
};
typedef SF_TAG SFScriptTag;

#endif
