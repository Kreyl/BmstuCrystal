/*
 * filter.h
 *
 *  Created on: 25 сент. 2014 г.
 *      Author: Kreyl
 */

#ifndef FILTER_H_
#define FILTER_H_

#include "cmd_uart.h"

#if 1 // ==== DSP instructions ====
__attribute__((always_inline))
static inline int32_t MulAndAcc(int32_t op1, int32_t op2, int32_t acc) {
    int32_t rslt;
    asm volatile ("mla %0, %1, %2, %3" : "=r" (rslt) : "r" (op1), "r" (op2), "r" (acc));
    return rslt;
}

#endif

class Filter_t {
public:
    bool Running = true;
    void Stop() {
        Running = false;
        Reset();
    }
    void Start() { Running = true; }
    virtual void Reset();
    virtual int32_t AddXAndCalculate(int32_t x0);
};

#if 1 // ============================ FIR int ==================================
#define FIR_MAX_SZ  20
class FirInt_t : public Filter_t {
private:
    int32_t x[FIR_MAX_SZ];
    int IndxWx = 0;
public:
    // Settins
    int32_t Sz = 1;
    int32_t Divider = 1024;
    int32_t a[FIR_MAX_SZ] = { a[0] = 1024 };
    // Commands
    void Reset() { for(int i=0; i<FIR_MAX_SZ; i++) x[i] = 0; }
    int32_t AddXAndCalculate(int32_t x0) {
        if(!Running) return 0;
        int32_t y0 = x0 * a[0];
        if(Sz > 1) {
            int IndxR = IndxWx;
            for(int32_t i=1; i<Sz; i++) {
                y0 += x[IndxR] * a[i];
    //            Uart.Printf("x%u=%d; ", R, x[R], i);
                if(++IndxR >= Sz-1) IndxR = 0;
            }
            // Add x0 to buffer
            if(IndxWx == 0) IndxWx = Sz - 2;
            else IndxWx--;
            x[IndxWx] = x0;
        } // if Sz > 1
//        Uart.Printf("rslt=%d", rslt);
        return (Divider == 0)? 0 : y0 / Divider;
    }

    // For debug purposes
    void PrintState() {
        Uart.Printf("\rSz=%d; Div=%d;\r", Sz, Divider);
        for(int32_t i=0; i<Sz; i++) Uart.Printf("a%d=%d ", i, a[i]);
//        for(uint32_t i=0; i<Sz; i++) Uart.Printf("x%d=%d ", i, a[i]);
    }
};
#endif

#if 1 // ============================ IIR int ==================================
#define IIR_MAX_SZ  10
class IirInt_t : public Filter_t {
private:
    int32_t x[IIR_MAX_SZ];
    int32_t y[IIR_MAX_SZ];
public:
    // Settins
    int32_t SzA = 1, SzB = 0;
    int32_t Divider = 1024;
    int32_t a[IIR_MAX_SZ] = { a[0] = 1024 };
    int32_t b[IIR_MAX_SZ];
    // Commands
    void Reset() {
        for(int i=0; i<IIR_MAX_SZ; i++) {
            x[i] = 0;
            y[i] = 0;
        }
    }
    void ResetCoefs() {
        SzA=0;
        SzB=0;
        for(int i=0; i<IIR_MAX_SZ; i++) {
            a[i] = 0;
            b[i] = 0;
        }
    }
    int32_t AddXAndCalculate(int32_t x0) {
        if(!Running) return 0;
        int32_t y0=0;
        // None-recursive part
        x[0] = x0;
        y0 = MulAndAcc(x[0], a[0], y0);
        for(int i=1; i<SzA; i++) {
            y0 = MulAndAcc(x[i], a[i], y0);
            x[i] = x[i-1];
        }
        // Recursive part
        y0 = MulAndAcc(y[0], b[0], y0);
        for(int i=1; i<SzB; i++) {
            y0 = MulAndAcc(y[i], b[i], y0);
            y[i] = y[i-1];
        }
        // Divide and add y0 to buffer
        y0 = (Divider == 0)? 0 : y0 / Divider;
        y[0] = y0;
//        Uart.Printf("rslt=%d", rslt);
        return y0;
    }

    // For debug purposes
    void PrintState() {
        Uart.Printf("\rSzA=%d; SzB=%d; Div=%d;\r", SzA, SzB, Divider);
        for(int32_t i=0; i<SzA; i++) Uart.Printf("a%d=%d ", i, a[i]);
        Uart.Printf("\r");
        for(int32_t i=0; i<SzB; i++) Uart.Printf("b%d=%d ", i, b[i]);
    }
};
#endif




#if 0 // ========================== Const filter ===============================
/* ==== Example ====
#define FILTER_ORDER    4
#define FILTER_DIVIDER  1000
static const int32_t FirCoeff[FILTER_ORDER] = {
        400,
        300,
        200,
        100,
};
*/

template <uint32_t Sz, uint32_t Divider>
class FilterFir_t {
private:
    int32_t *a;
    int32_t x[Sz-1], W;
public:
    void Init(const int32_t *PCoefSettings) { a = (int32_t*)PCoefSettings; }
    int32_t AddAndCalc(int32_t x0) {
//        Uart.Printf("\r %d   ", x0);
        // Calculate filter
        int32_t rslt = x0 * a[0], R = W;
        for(uint32_t i=1; i<Sz; i++) {
            rslt += x[R] * a[i];
//            Uart.Printf("x%u=%d; ", R, x[R], i);
            if(++R >= Sz-1) R = 0;
        }
        // Add value to buffer
        if(W == 0) W = Sz - 2;
        else W--;
        x[W] = x0;
//        Uart.Printf("rslt=%d", rslt);
        return rslt / Divider;
    }
};
#endif


#endif /* FILTER_H_ */
