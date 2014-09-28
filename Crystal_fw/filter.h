/*
 * filter.h
 *
 *  Created on: 25 сент. 2014 г.
 *      Author: Kreyl
 */

#ifndef FILTER_H_
#define FILTER_H_

#include "cmd_uart.h"

class Filter_t {
public:
    bool Running = true;
    int32_t Sz = 1;
    void Stop() {
        Running = false;
        Reset();
    }
    void Start() { Running = true; }
    virtual void Reset();
};

#if 1 // ============================ FIR int ==================================
#define FIR_MAX_SZ  10
class FirInt_t : public Filter_t {
private:
public:
    // Settins
    int32_t Divider = 1024;
    int32_t a[FIR_MAX_SZ] = { a[0] = 1024 };
    // Commands
    void Reset() {
    }
    // For debug purposes
    void PrintState() {
        Uart.Printf("\rSz=%d; Div=%d;\r", Sz, Divider);
        for(int32_t i=0; i<Sz; i++) Uart.Printf("a%d=%d ", i, a[i]);
//        for(uint32_t i=0; i<Sz; i++) Uart.Printf("x%d=%d ", i, a[i]);
    }
};

#endif

#if 1 // ========================== Const filter ===============================
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
