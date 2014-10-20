/*
 * filter.h
 *
 *  Created on: 25 сент. 2014 г.
 *      Author: Kreyl
 */

#ifndef FILTER_H_
#define FILTER_H_

#include "cmd_uart.h"
//#include "arm_math.h"
#include "math.h"

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

#define FIR_MAX_SZ  20
#define IIR_MAX_SZ  10

#if 0 // ============================ FIR int ==================================
class FirInt_t : public Filter_t {
private:
    int32_t x[FIR_MAX_SZ];
public:
    // Settins
    int32_t Sz = 1;
    int32_t Divider = 1024;
    int32_t a[FIR_MAX_SZ] = { a[0] = 1024 };
    // Commands
    void Reset() { for(int i=0; i<FIR_MAX_SZ; i++) x[i] = 0; }
    void ResetCoefs() {
        Sz=0;
        for(int i=0; i<FIR_MAX_SZ; i++) a[i] = 0;
    }
    int32_t AddXAndCalculate(int32_t x0) {
        if(!Running) return 0;
        int32_t y0=0;
        x[0] = x0;
        for(int i=Sz-1; i>=1; i--) {
            y0 = MulAndAcc(x[i], a[i], y0);
            x[i] = x[i-1];
        }
        y0 = MulAndAcc(x[0], a[0], y0);
        y0 = (Divider == 0)? 0 : y0 / Divider;
        return y0;
    }

    // For debug purposes
    void PrintState() {
        Uart.Printf("\rSz=%d; Div=%d;\r", Sz, Divider);
        for(int32_t i=0; i<Sz; i++) Uart.Printf("a%d=%d ", i, a[i]);
//        for(uint32_t i=0; i<Sz; i++) Uart.Printf("x%d=%d ", i, a[i]);
    }
};
#endif

#if 0 // ============================ IIR int ==================================
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
        for(int i=SzA-1; i>=1; i--) {
            y0 = MulAndAcc(x[i], a[i], y0);
            x[i] = x[i-1];
        }
        y0 = MulAndAcc(x[0], a[0], y0);
        // Recursive part
        for(int i=SzB-1; i>=1; i--) {
            y0 = MulAndAcc(y[i], b[i], y0);
            y[i] = y[i-1];
        }
        y0 = MulAndAcc(y[0], b[0], y0);
        // Divide and add y0 to buffer
        y0 = (Divider == 0)? 0 : y0 / Divider;
        y[0] = y0;
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

#if 1 // ============================ FIR Float ================================
class FirFloat_t : public Filter_t {
private:
    float x[FIR_MAX_SZ];
public:
    // Settins
    int32_t Sz = 1;
    float a[FIR_MAX_SZ] = { 1 };
    // Commands
    void Reset() { for(int i=0; i<FIR_MAX_SZ; i++) x[i] = 0; }
    void ResetCoefs() {
        Sz=0;
        for(int i=0; i<FIR_MAX_SZ; i++) a[i] = 0;
    }
    int32_t AddXAndCalculate(int32_t x0) {
        if(!Running) return 0;
        float y0=0;
        x[0] = x0;
        for(int i=Sz-1; i>=1; i--) {
            y0 += x[i] * a[i];
            x[i] = x[i-1];
        }
        y0 += x[0] * a[0];
        return roundf(y0);
    }
    // For debug purposes
    void PrintState() {
        Uart.Printf("\rSz=%d;\r", Sz);
        //for(int32_t i=0; i<Sz; i++) Uart.Printf("a%d=%d ", i, a[i]);
//        for(uint32_t i=0; i<Sz; i++) Uart.Printf("x%d=%d ", i, a[i]);
    }
};
#endif

#if 1 // ============================ IIR Float ==================================
class IirFloat_t : public Filter_t {
private:
    float x[IIR_MAX_SZ];
    float y[IIR_MAX_SZ];
public:
    // Settins
    int32_t SzA = 1, SzB = 0;
    float a[IIR_MAX_SZ] = { a[0] = 1 };
    float b[IIR_MAX_SZ];
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
        float y0=0;
        // None-recursive part
        x[0] = x0;
        for(int i=SzA-1; i>=1; i--) {
            y0 += x[i] * a[i];
            x[i] = x[i-1];
        }
        y0 += x[0] * a[0];
        // Recursive part
        for(int i=SzB-1; i>=1; i--) {
            y0 += y[i] * b[i];
            y[i] = y[i-1];
        }
        y0 += y[0] * b[0];
        y[0] = y0;
        return roundf(y0);
    }

    // For debug purposes
    void PrintState() {
        Uart.Printf("\rSzA=%d; SzB=%d", SzA, SzB);
//        for(int32_t i=0; i<SzA; i++) Uart.Printf("a%d=%d ", i, a[i]);
//        Uart.Printf("\r");
//        for(int32_t i=0; i<SzB; i++) Uart.Printf("b%d=%d ", i, b[i]);
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
