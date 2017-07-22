SheenFigure
=========
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Build Status](https://api.travis-ci.org/mta452/SheenFigure.svg?branch=master)](https://travis-ci.org/mta452/SheenFigure)
[![Coverage Status](https://coveralls.io/repos/github/mta452/SheenFigure/badge.svg?branch=master)](https://coveralls.io/github/mta452/SheenFigure?branch=master)

SheenFigure aims to implement the advanced typographic tables of OpenType specification available at https://www.microsoft.com/en-us/Typography/OpenTypeSpecification.aspx along with script specific shaping engines available at https://www.microsoft.com/en-us/Typography/SpecificationsOverview.aspx. Currently, it only supports Arabic script and a subset of [GDEF](https://www.microsoft.com/typography/otspec/GDEF.htm), [GSUB](https://www.microsoft.com/typography/otspec/GSUB.htm) and [GPOS](https://www.microsoft.com/typography/otspec/GPOS.htm) tables. The support is intended to increase in each newer versions of the library.

Here are some of the advantages of SheenFigure.

* Object based
* Designed to be thread safe
* Lightweight API for interaction
* Clear differentiation of public and private API
* Thoroughly tested

## Dependency
SheenFigure only depends on [SheenBidi](https://github.com/mta452/SheenBidi) in order to support UTF-8, UTF-16 and UTF-32 string encodings. Other than that, it only uses standard C library headers ```stddef.h```, ```stdint.h```, ```stdlib.h``` and  ```string.h```.

## Configuration
The configuration options are available in `Headers/SFConfig.h`.

* ```SF_CONFIG_UNITY``` builds the library as a single module and lets the compiler make decisions to inline functions.

## Compiling
SheenFigure can be compiled with any C compiler. The best way for compiling is to add all the files in an IDE and hit build. The only thing to consider however is that if ```SF_CONFIG_UNITY``` is enabled then only ```Source/SheenFigure.c``` should be compiled.

## Public API
Here is a glimpse of public API in the form of UML class diagram.
![Public API](https://raw.githubusercontent.com/mta452/SheenFigure/images/PublicAPI.png)

## Conformance
Following are the tables implemented by SheenFigure.

### OpenType Layout Common Table Formats
| Table            	| Supported 	|
|------------------	|:---------:	|
| Script List      	|     ✔️     	|
| Feature List     	|     ✔️     	|
| Lookup List      	|     ✔️     	|
| Coverage         	|     ✔️     	|
| Class Definition 	|     ✔️     	|
| Device           	|     ❌     	|
| Variation Index  	|     ❌     	|

### GDEF (Glyph Definition Table)
| Subtable                         	| Supported 	|
|----------------------------------	|:---------:	|
| Glyph Class Definition           	|     ✔️     	|
| Attachment List                  	|     ❌     	|
| Ligature Caret List              	|     ❌     	|
| Mark Attachment Class Definition 	|     ✔️     	|
| Mark Glyph Sets                  	|     ✔️     	|
| Item Variation Store             	|     ❌     	|

### GPOS (Glyph Positioning Table)
| Subtable                    	| Format 1 	| Format 2 	| Format 3 	|
|-----------------------------	|:--------:	|:--------:	|:--------:	|
| Single Adjustment           	|     ✔️    	|     ✔️    	|          	|
| Pair Adjustment             	|     ✔️    	|     ✔️    	|          	|
| Cursive Attachment          	|     ✔️    	|          	|          	|
| Mark To Base Attachment     	|     ✔️    	|          	|          	|
| Mark To Ligature Attachment 	|     ✔️    	|          	|          	|
| Mark To Mark Attachment     	|     ✔️    	|          	|          	|
| Context Positioning         	|     ✔️    	|     ✔️    	|     ✔️    	|
| Chained Context Positioning 	|     ✔️    	|     ✔️    	|     ✔️    	|
| Extension Positioning       	|     ✔️    	|          	|          	|

### GSUB (Glyph Substitution Table)
| Subtable                 	| Format 1 	| Format 2 	| Format 3 	|
|--------------------------	|:--------:	|:--------:	|:--------:	|
| Single                   	|     ✔️    	|     ✔️    	|          	|
| Multiple                 	|     ✔️    	|          	|          	|
| Alternate                	|     ✔️    	|          	|          	|
| Ligature                 	|     ✔️    	|          	|          	|
| Context                  	|     ✔️    	|     ✔️    	|     ✔️    	|
| Chaining Context         	|     ✔️    	|     ✔️    	|     ✔️    	|
| Extension                	|     ✔️    	|          	|          	|
| Reverse Chaining Context 	|     ❌    	|          	|          	|
