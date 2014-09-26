/*
 * cmd.h
 *
 *  Created on: 26 сент. 2014 г.
 *      Author: g.kruglov
 */

#ifndef CMD_H_
#define CMD_H_

#include "hal.h"
#include <cstring>

#ifdef STM32F4XX
#include "kl_lib_f40x.h"
#endif

#define DELIMITERS      " ,"

enum ProcessDataResult_t {pdrProceed, pdrNewCmd};

template <uint32_t BufSz>
class Cmd_t {
private:
    char IString[BufSz];
    void Finalize() {
        for(uint32_t i=Cnt; i < BufSz; i++) IString[i] = 0;
        Name = strtok(IString, DELIMITERS);
        GetNextToken();
    }
public:
    uint32_t Cnt;
    char *Name, *Token;
    ProcessDataResult_t PutChar(char c) {
        if(c == '\b') { if(Cnt > 0) Cnt--; }    // do backspace
        else if((c == '\r') or (c == '\n')) {
            if(Cnt == 0) return pdrProceed;     // Empty cmd, nothing to do
            else {
                Finalize();
                return
            }
        }

        if(Cnt < BufSz-1) IString[Cnt++] = c;
    }
    uint8_t GetNextToken() {
        Token = strtok(NULL, DELIMITERS);
        return (*Token == '\0')? 1 : 0;
    }
    uint8_t TryConvertTokenToNumber(uint32_t *POutput) { return Convert::TryStrToUInt32(Token, POutput); }
    uint8_t TryConvertTokenToNumber( int32_t *POutput) { return Convert::TryStrToInt32(Token, POutput); }
    bool NameIs(const char *SCmd) { return (strcasecmp(Name, SCmd) == 0); }
};

#endif /* CMD_H_ */
