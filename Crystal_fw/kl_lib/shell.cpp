/*
 * shell.cpp
 *
 *  Created on: 21 апр. 2017 г.
 *      Author: Kreyl
 */

#include "shell.h"
#include "uart.h"

extern CmdUart_t Uart;

void Printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    chSysLock();
    Uart.IVsPrintf(format, args);
    chSysUnlock();
    va_end(args);
}

void Printf(CmdUart_t &AUart, const char *format, ...) {
    va_list args;
    va_start(args, format);
    chSysLock();
    AUart.IVsPrintf(format, args);
    chSysUnlock();
    va_end(args);
}

void PrintfI(const char *format, ...) {
    va_list args;
    va_start(args, format);
    Uart.IVsPrintf(format, args);
    va_end(args);
}

void PrintfEOL() {
    Uart.PrintEOL();
}

extern "C" {
void PrintfC(const char *format, ...) {
    va_list args;
    va_start(args, format);
    Uart.IVsPrintf(format, args);
    va_end(args);
}
} // exern C


class PrintToBuf_t : public PrintfHelper_t {
public:
    char *S;
    uint8_t IPutChar(char c) {
        *S++ = c;
        return retvOk;
    }
    void IStartTransmissionIfNotYet() {}
};

char* PrintfToBuf(char* PBuf, const char *format, ...) {
    PrintToBuf_t PtB;
    PtB.S = PBuf;
    va_list args;
    va_start(args, format);
    PtB.IVsPrintf(format, args);
    va_end(args);
    *PtB.S = 0;
    return PtB.S;
}

#if 0
void ByteShell_t::Reply(uint8_t CmdCode, uint32_t Len, uint8_t *PData) {
//    Printf("BSendCmd %X; %u; %A\r", CmdCode, Len, PData, Len, ' ');
    // Send StartOfCmd
    if(IPutChar('#') != retvOk) return;
    // Send command code
    if(IPutChar(HalfByte2Char(CmdCode >> 4)) != retvOk) return;
    if(IPutChar(HalfByte2Char(CmdCode)) != retvOk) return;
    // Send data
    for(uint32_t i=0; i<Len; i++) {
        if(IPutChar(HalfByte2Char((*PData) >> 4)) != retvOk) return;
        if(IPutChar(HalfByte2Char(*PData++)) != retvOk) return;
    }
    // Send EOL
    if(IPutChar('\r') != retvOk) return;
    if(IPutChar('\n') != retvOk) return;
    IStartTransmissionIfNotYet();
}
#endif

#if PRINTF_FLOAT_EN
#define FLOAT_PRECISION     9
static const long power10Table[FLOAT_PRECISION] = {
    10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
};
#endif

void PrintfHelper_t::PrintEOL() {
    IPutChar('\r');
    IPutChar('\n');
    IStartTransmissionIfNotYet();
}

void PrintfHelper_t::IVsPrintf(const char *format, va_list args) {
    const char *fmt = format;
    uint32_t width = 0, precision;
    char c, filler;
    while(true) {
        c = *fmt++;
        if(c == 0) goto End;
        if(c != '%') {  // Not %
            if(IPutChar(c) != retvOk) goto End;
            else continue;
        }

        // Here goes optional width specification.
        // If it starts with zero (zero_padded is true), it means we use '0' instead of ' ' as a filler.
        filler = ' ';
        if(*fmt == '0') {
            fmt++;
            filler = '0';
        }

        width = 0;
        while(true) {
            c = *fmt++;
            if(c >= '0' && c <= '9') c -= '0';
            else if (c == '*') c = va_arg(args, int);
            else break;
            width = width * 10 + c;
        }

        precision = 0;
        if(c == '.') {
            while(true) {
                c = *fmt++;
                if(c >= '0' && c <= '9') c -= '0';
                else if(c == '*') c = va_arg(args, int);
                else break;
                precision = precision * 10 + c;
            }
        }

        // Command decoding
        switch(c) {
            case 'c':
                if(IPutChar(va_arg(args, int)) != retvOk) goto End;
                break;

            case 's':
            case 'S': {
                char *s = va_arg(args, char*);
                while(*s != 0) {
                    if(IPutChar(*s++) != retvOk) goto End;
                }
            }
            break;

            case 'X':
                if(IPutUint(va_arg(args, uint32_t), 16, width, filler) != retvOk) goto End;
                break;
            case 'u':
                if(IPutUint(va_arg(args, uint32_t), 10, width, filler) != retvOk) goto End;
                break;

            case 'd':
            case 'i':
            {
                int32_t n = va_arg(args, int32_t);
                if(n < 0) {
                    if(IPutChar('-') != retvOk) goto End;
                    n = -n;
                }
                if(IPutUint(n, 10, width, filler) != retvOk) goto End;
            }
            break;

#if PRINTF_FLOAT_EN
            case 'f': {
                float f = (float)va_arg(args, double);
                if (f < 0) {
                    if(IPutChar('-') != retvOk) goto End;
                    f = -f;
                }
                int32_t n;
                if((precision == 0) || (precision > FLOAT_PRECISION)) precision = FLOAT_PRECISION;
                n = (int32_t)f;
                if(IPutUint(n, 10, width, filler) != retvOk) goto End;
                if(IPutChar('.') != retvOk) goto End;
                filler = '0';
                width = precision;
                n = (long)((f - n) * power10Table[precision - 1]);
                if(IPutUint(n, 10, width, filler) != retvOk) goto End;
            } break;
#endif

            case 'A': {
                uint8_t *arr = va_arg(args, uint8_t*);
                int32_t n = va_arg(args, int32_t);
                int32_t Delimiter = va_arg(args, int32_t);
                filler = '0';       // }
                width = 2;          // } 01 02 0A etc.; not 1 2 A
                for(int32_t i = 0; i < n; i++) {
                    if((i > 0) && (Delimiter != 0)) { // do not place delimiter before or after array
                        if(IPutChar((char)Delimiter) != retvOk) goto End;
                    }
                    if(IPutUint(arr[i], 16, width, filler) != retvOk) goto End;
                }
            } break;

            case '%':
                if(IPutChar('%') != retvOk) goto End;
                break;
        } // switch
    } // while
    End:
    IStartTransmissionIfNotYet();
}

uint8_t PrintfHelper_t::IPutUint(uint32_t n, uint32_t base, uint32_t width, char filler) {
    char digits[10];
    uint32_t len = 0;
    // Place digits to buffer
    do {
        uint32_t digit = n % base;
        n /= base;
        digits[len++] = (digit < 10)? '0'+digit : 'A'+digit-10;
    } while(n > 0);
    // Add padding
    for(uint32_t i = len; i < width; i++) {
        if(IPutChar(filler) != retvOk) return retvOverflow;
    }
    // Print digits
    while(len > 0) {
        if(IPutChar(digits[--len]) != retvOk) return retvOverflow;
    }
    return retvOk;
} // IPutUint

#if 1 // =========================== ByteShell_t ===============================
static const uint8_t crc_array[256] = {
    0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
    0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
    0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,
    0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
    0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,
    0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
    0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,
    0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
    0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,
    0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
    0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,
    0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
    0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,
    0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
    0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
    0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
    0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f,
    0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
    0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92,
    0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
    0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,
    0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
    0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1,
    0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
    0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49,
    0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
    0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,
    0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
    0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a,
    0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
    0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7,
    0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35,
};

ProcessDataResult_t ByteCmd_t::PutChar(char c) {
    // Reset cmd if it was completed, and after that new char arrived
    if(Completed) {
        Completed = false;
        Cnt = 0;
        FirstHalfOfByte = true;
        WasStarted = false;
    }
    // Process char
    if(c == '#') {  // Start of new cmd
        Cnt = 0;
        FirstHalfOfByte = true;
        WasStarted = true;
    }
    else if(WasStarted) {   // Do all the next if was started
        if(c == '\b') { if(Cnt > 0) Cnt--; }    // Do backspace
        else if(c == '#') { Cnt = 0; FirstHalfOfByte = true; }
        else if((c == '\r') or (c == '\n')) {   // End of line, check if cmd completed
            if(Cnt != 0) {  // if not empty
                // Check crc
                uint8_t crc = 0;
                for(uint32_t i=0; i<Cnt; i++) crc = crc_array[IBuf[i] ^ crc];
                CrcOk = (crc == 0);
                // Put bytes
                CmdCode = IBuf[0];
                PktID = IBuf[1] | (IBuf[2] << 8);
                Cnt -= 3;  // Remove CmdCode and PktID out of cnt
                Completed = true;
                return pdrNewCmd;
            }
        }
        // Some other char, maybe good one
        else if(Cnt < BYTECMD_DATA_SZ) {
            if     (c >= '0' and c <= '9') AddHalfOfByte(c - '0');
            else if(c >= 'A' and c <= 'F') AddHalfOfByte(c - 'A' + 0xA);
            else if(c >= 'a' and c <= 'f') AddHalfOfByte(c - 'a' + 0xA);
        }
    }
    return pdrProceed;
}

void ByteShell_t::SendPkt(uint8_t CmdCode, uint16_t PktID, uint32_t Len, void *PData, uint8_t stat) {
    uint8_t crc = 0, b, *IData = (uint8_t*)PData;
    // Start
    IPutByte('#');
    // Cmd Code
    IPutHexByte(CmdCode);
    crc = crc_array[CmdCode ^ crc];
    // Pkt ID
    b = PktID &0x00FF;
    IPutHexByte(b);
    crc = crc_array[b ^ crc];
    b = (PktID >> 8) & 0x00FF;
    IPutHexByte(b);
    crc = crc_array[b ^ crc];
    // Data
    for(uint32_t i=0; i<Len; i++) {
        b = IData[i];
        IPutHexByte(b);
        crc = crc_array[b ^ crc];
    }
    // State
    IPutHexByte(stat);
    crc = crc_array[stat ^ crc];
    // CRC
    IPutHexByte(crc);
    // End of pkt
    IPutByte('\r');
    IPutByte('\n');
    IStartTransmissionIfNotYet();
}


void ByteShell_t::IPutHexByte(uint8_t b) {
    IPutByte(HalfByte2Char(b >> 4));
    IPutByte(HalfByte2Char(b));
}
#endif

