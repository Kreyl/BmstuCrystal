/*
 * shell.h
 *
 *  Created on: 25 окт. 2015 г.
 *      Author: Kreyl
 */

#pragma once

#include <cstring>
#include <stdarg.h>
#include "kl_lib.h"

#define CMD_BUF_SZ		99
#define DELIMITERS      " ,"

enum ProcessDataResult_t {pdrProceed, pdrNewCmd};

class Cmd_t {
private:
    char IString[CMD_BUF_SZ];
    uint32_t Cnt;
    bool Completed;
public:
    char *Name, *Token;
    ProcessDataResult_t PutChar(char c) {
        // Reset cmd if it was completed, and after that new char arrived
        if(Completed) {
            Completed = false;
            Cnt = 0;
        }
        // Process char
        if(c == '\b') { if(Cnt > 0) Cnt--; }    // do backspace
        else if((c == '\r') or (c == '\n')) {   // end of line, check if cmd completed
            if(Cnt != 0) {  // if cmd is not empty
                IString[Cnt] = 0; // End of string
                Name = strtok(IString, DELIMITERS);
                Completed = true;
                return pdrNewCmd;
            }
        }
        else if(Cnt < (CMD_BUF_SZ-1)) IString[Cnt++] = c;  // Add char if buffer not full
        return pdrProceed;
    }
    uint8_t GetNextString(char **PStr = nullptr) {
        Token = strtok(NULL, DELIMITERS);
        if(PStr != nullptr) *PStr = Token;
        return (*Token == '\0')? retvEmpty : retvOk;
    }

    template <typename T>
    uint8_t GetNext(T *POutput) {
        uint8_t r = GetNextString();
        if(r == retvOk) {
            char *p;
            int32_t dw32 = strtol(Token, &p, 0);
            if(*p == '\0') *POutput = (T)dw32;
            else r = retvNotANumber;
        }
        return r;
    }

    template <typename T>
    uint8_t GetArray(T *Ptr, int32_t Len) {
        for(int32_t i=0; i<Len; i++) {
            T Number;
            uint8_t r = GetNext<T>(&Number);
            if(r == retvOk) *Ptr++ = Number;
            else return r;
        }
        return retvOk;
    }

    /*  int32_t Indx, Value;
        if(PCmd->GetParams<int32_t>(2, &Indx, &Value) == retvOk) {...}
        else PShell->Ack(retvCmdError);    */
    template <typename T>
    uint8_t GetParams(uint8_t Cnt, ...) {
        uint8_t Rslt = retvOk;
        va_list args;
        va_start(args, Cnt);
        while(Cnt--) {
            T* ptr = va_arg(args, T*);
            Rslt = GetNext<T>(ptr);
            if(Rslt != retvOk) break;
        }
        va_end(args);
        return Rslt;
    }

    bool NameIs(const char *SCmd) { return (kl_strcasecmp(Name, SCmd) == 0); }
    Cmd_t() {
        Cnt = 0;
        Completed = false;
        Name = nullptr;
        Token = nullptr;
    }
};

class Shell_t {
public:
	Cmd_t Cmd;
	virtual void SignalCmdProcessed() = 0;
	virtual void Print(const char *format, ...) = 0;
	void Reply(const char* CmdCode, int32_t Data) { Print("%S,%d\r\n", CmdCode, Data); }
	void Ack(int32_t Result) { Print("Ack %d\r\n", Result); }
};


// Parent class for everything that prints
class PrintfHelper_t {
private:
    uint8_t IPutUint(uint32_t n, uint32_t base, uint32_t width, char filler);
protected:
    virtual uint8_t IPutChar(char c) = 0;
    virtual void IStartTransmissionIfNotYet() = 0;
public:
    void IVsPrintf(const char *format, va_list args);
    void PrintEOL();
};

#if 1 // ========================= Byte protocol ===============================
#define BYTECMD_DATA_SZ     99
class ByteCmd_t {
private:
    bool Completed;
    uint8_t IBuf[BYTECMD_DATA_SZ];
    bool FirstHalfOfByte = true, WasStarted = false;
    void AddHalfOfByte(uint8_t Half) {
        if(FirstHalfOfByte) {
            IBuf[Cnt] = Half << 4;
            FirstHalfOfByte = false;
        }
        else {
            IBuf[Cnt++] |= Half;
            FirstHalfOfByte = true;
        }
    }
public:
    uint8_t CmdCode, *Data = &IBuf[3];
    uint16_t PktID;
    uint32_t Cnt;
    bool CrcOk;
    ProcessDataResult_t PutChar(char c);
};

class ByteShell_t {
protected:
    uint8_t HalfByte2Char(uint8_t hb) {
        hb &= 0x0F;
        if(hb < 0x0A) return ('0' + hb);    // 0...9
        else return ('A' + hb - 0x0A);      // A...F
    }
    virtual uint8_t IPutByte(uint8_t b) = 0;
    void IPutHexByte(uint8_t b);
public:
    ByteCmd_t Cmd;
    virtual void SignalCmdProcessed() = 0;
    virtual void IStartTransmissionIfNotYet() = 0;
    void SendPkt(uint8_t CmdCode, uint16_t PktID, uint32_t Len, void *PData, uint8_t stat);
};

#endif

// Functions
class CmdUart_t;

void Printf(const char *format, ...);
void Printf(CmdUart_t &AUart, const char *format, ...);
void PrintfI(const char *format, ...);
void PrintfEOL();
//void PrintfNow(const char *format, ...);

char* PrintfToBuf(char* PBuf, const char *format, ...);

extern "C" {
void PrintfC(const char *format, ...);
//void PrintfCNow(const char *format, ...);
}
