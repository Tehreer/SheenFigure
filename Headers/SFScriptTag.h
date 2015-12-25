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
    SFScriptTagARAB = SFTagMake('a', 'r', 'a', 'b'),
    SFScriptTagARMN = SFTagMake('a', 'r', 'm', 'n'),
    SFScriptTagAVST = SFTagMake('a', 'v', 's', 't'),
    SFScriptTagBALI = SFTagMake('b', 'a', 'l', 'i'),
    SFScriptTagBAMU = SFTagMake('b', 'a', 'm', 'u'),
    SFScriptTagBATK = SFTagMake('b', 'a', 't', 'k'),
    SFScriptTagBENG = SFTagMake('b', 'e', 'n', 'g'),
    SFScriptTagBNG2 = SFTagMake('b', 'n', 'g', '2'),
    SFScriptTagBOPO = SFTagMake('b', 'o', 'p', 'o'),
    SFScriptTagBRAI = SFTagMake('b', 'r', 'a', 'i'),
    SFScriptTagBRAH = SFTagMake('b', 'r', 'a', 'h'),
    SFScriptTagBUGI = SFTagMake('b', 'u', 'g', 'i'),
    SFScriptTagBUHD = SFTagMake('b', 'u', 'h', 'd'),
    SFScriptTagBYZM = SFTagMake('b', 'y', 'z', 'm'),
    SFScriptTagCANS = SFTagMake('c', 'a', 'n', 's'),
    SFScriptTagCARI = SFTagMake('c', 'a', 'r', 'i'),
    SFScriptTagCAKM = SFTagMake('c', 'a', 'k', 'm'),
    SFScriptTagCHAM = SFTagMake('c', 'h', 'a', 'm'),
    SFScriptTagCHER = SFTagMake('c', 'h', 'e', 'r'),
    SFScriptTagHANI = SFTagMake('h', 'a', 'n', 'i'),
    SFScriptTagCOPT = SFTagMake('c', 'o', 'p', 't'),
    SFScriptTagCPRT = SFTagMake('c', 'p', 'r', 't'),
    SFScriptTagCYRL = SFTagMake('c', 'y', 'r', 'l'),
    SFScriptTagDFLT = SFTagMake('D', 'F', 'L', 'T'),
    SFScriptTagDSRT = SFTagMake('d', 's', 'r', 't'),
    SFScriptTagDEVA = SFTagMake('d', 'e', 'v', 'a'),
    SFScriptTagDEV2 = SFTagMake('d', 'e', 'v', '2'),
    SFScriptTagEGYP = SFTagMake('e', 'g', 'y', 'p'),
    SFScriptTagETHI = SFTagMake('e', 't', 'h', 'i'),
    SFScriptTagGEOR = SFTagMake('g', 'e', 'o', 'r'),
    SFScriptTagGLAG = SFTagMake('g', 'l', 'a', 'g'),
    SFScriptTagGOTH = SFTagMake('g', 'o', 't', 'h'),
    SFScriptTagGREK = SFTagMake('g', 'r', 'e', 'k'),
    SFScriptTagGUJR = SFTagMake('g', 'u', 'j', 'r'),
    SFScriptTagGJR2 = SFTagMake('g', 'j', 'r', '2'),
    SFScriptTagGURU = SFTagMake('g', 'u', 'r', 'u'),
    SFScriptTagGUR2 = SFTagMake('g', 'u', 'r', '2'),
    SFScriptTagHANG = SFTagMake('h', 'a', 'n', 'g'),
    SFScriptTagJAMO = SFTagMake('j', 'a', 'm', 'o'),
    SFScriptTagHANO = SFTagMake('h', 'a', 'n', 'o'),
    SFScriptTagHEBR = SFTagMake('h', 'e', 'b', 'r'),
    SFScriptTagARMI = SFTagMake('a', 'r', 'm', 'i'),
    SFScriptTagPHLI = SFTagMake('p', 'h', 'l', 'i'),
    SFScriptTagPRTI = SFTagMake('p', 'r', 't', 'i'),
    SFScriptTagJAVA = SFTagMake('j', 'a', 'v', 'a'),
    SFScriptTagKTHI = SFTagMake('k', 't', 'h', 'i'),
    SFScriptTagKNDA = SFTagMake('k', 'n', 'd', 'a'),
    SFScriptTagKND2 = SFTagMake('k', 'n', 'd', '2'),
    SFScriptTagKANA = SFTagMake('k', 'a', 'n', 'a'),
    SFScriptTagKALI = SFTagMake('k', 'a', 'l', 'i'),
    SFScriptTagKHAR = SFTagMake('k', 'h', 'a', 'r'),
    SFScriptTagKHMR = SFTagMake('k', 'h', 'm', 'r'),
    SFScriptTagLAO  = SFTagMake('l', 'a', 'o', ' '),
    SFScriptTagLATN = SFTagMake('l', 'a', 't', 'n'),
    SFScriptTagLEPC = SFTagMake('l', 'e', 'p', 'c'),
    SFScriptTagLIMB = SFTagMake('l', 'i', 'm', 'b'),
    SFScriptTagLINB = SFTagMake('l', 'i', 'n', 'b'),
    SFScriptTagLISU = SFTagMake('l', 'i', 's', 'u'),
    SFScriptTagLYCI = SFTagMake('l', 'y', 'c', 'i'),
    SFScriptTagLYDI = SFTagMake('l', 'y', 'd', 'i'),
    SFScriptTagMLYM = SFTagMake('m', 'l', 'y', 'm'),
    SFScriptTagMLM2 = SFTagMake('m', 'l', 'm', '2'),
    SFScriptTagMAND = SFTagMake('m', 'a', 'n', 'd'),
    SFScriptTagMATH = SFTagMake('m', 'a', 't', 'h'),
    SFScriptTagMTEI = SFTagMake('m', 't', 'e', 'i'),
    SFScriptTagMERC = SFTagMake('m', 'e', 'r', 'c'),
    SFScriptTagMERO = SFTagMake('m', 'e', 'r', 'o'),
    SFScriptTagMONG = SFTagMake('m', 'o', 'n', 'g'),
    SFScriptTagMUSC = SFTagMake('m', 'u', 's', 'c'),
    SFScriptTagMYMR = SFTagMake('m', 'y', 'm', 'r'),
    SFScriptTagTALU = SFTagMake('t', 'a', 'l', 'u'),
    SFScriptTagNKO  = SFTagMake('n', 'k', 'o', ' '),
    SFScriptTagOGAM = SFTagMake('o', 'g', 'a', 'm'),
    SFScriptTagOLCK = SFTagMake('o', 'l', 'c', 'k'),
    SFScriptTagITAL = SFTagMake('i', 't', 'a', 'l'),
    SFScriptTagXPEO = SFTagMake('x', 'p', 'e', 'o'),
    SFScriptTagSARB = SFTagMake('s', 'a', 'r', 'b'),
    SFScriptTagORKH = SFTagMake('o', 'r', 'k', 'h'),
    SFScriptTagORYA = SFTagMake('o', 'r', 'y', 'a'),
    SFScriptTagORY2 = SFTagMake('o', 'r', 'y', '2'),
    SFScriptTagOSMA = SFTagMake('o', 's', 'm', 'a'),
    SFScriptTagPHAG = SFTagMake('p', 'h', 'a', 'g'),
    SFScriptTagPHNX = SFTagMake('p', 'h', 'n', 'x'),
    SFScriptTagRJNG = SFTagMake('r', 'j', 'n', 'g'),
    SFScriptTagRUNR = SFTagMake('r', 'u', 'n', 'r'),
    SFScriptTagSAMR = SFTagMake('s', 'a', 'm', 'r'),
    SFScriptTagSAUR = SFTagMake('s', 'a', 'u', 'r'),
    SFScriptTagSHRD = SFTagMake('s', 'h', 'r', 'd'),
    SFScriptTagSHAW = SFTagMake('s', 'h', 'a', 'w'),
    SFScriptTagSINH = SFTagMake('s', 'i', 'n', 'h'),
    SFScriptTagSORA = SFTagMake('s', 'o', 'r', 'a'),
    SFScriptTagXSUX = SFTagMake('x', 's', 'u', 'x'),
    SFScriptTagSUND = SFTagMake('s', 'u', 'n', 'd'),
    SFScriptTagSYLO = SFTagMake('s', 'y', 'l', 'o'),
    SFScriptTagSYRC = SFTagMake('s', 'y', 'r', 'c'),
    SFScriptTagTGLG = SFTagMake('t', 'g', 'l', 'g'),
    SFScriptTagTAGB = SFTagMake('t', 'a', 'g', 'b'),
    SFScriptTagTALE = SFTagMake('t', 'a', 'l', 'e'),
    SFScriptTagLANA = SFTagMake('l', 'a', 'n', 'a'),
    SFScriptTagTAVT = SFTagMake('t', 'a', 'v', 't'),
    SFScriptTagTAKR = SFTagMake('t', 'a', 'k', 'r'),
    SFScriptTagTAML = SFTagMake('t', 'a', 'm', 'l'),
    SFScriptTagTML2 = SFTagMake('t', 'm', 'l', '2'),
    SFScriptTagTELU = SFTagMake('t', 'e', 'l', 'u'),
    SFScriptTagTEL2 = SFTagMake('t', 'e', 'l', '2'),
    SFScriptTagTHAA = SFTagMake('t', 'h', 'a', 'a'),
    SFScriptTagTHAI = SFTagMake('t', 'h', 'a', 'i'),
    SFScriptTagTIBT = SFTagMake('t', 'i', 'b', 't'),
    SFScriptTagTFNG = SFTagMake('t', 'f', 'n', 'g'),
    SFScriptTagUGAR = SFTagMake('u', 'g', 'a', 'r'),
    SFScriptTagVAI  = SFTagMake('v', 'a', 'i', ' '),
    SFScriptTagYI   = SFTagMake('y', 'i', ' ', ' ')
};
typedef SFTag SFScriptTag;

#endif
