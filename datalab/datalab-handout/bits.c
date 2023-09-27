/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2022 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2017, fifth edition, plus
   the following additions from Amendment 1 to the fifth edition:
   - 56 emoji characters
   - 285 hentaigana
   - 3 additional Zanabazar Square characters */
/* 
 * bitXnor - ~(x^y) using only ~ and | 
 *   Example: bitXnor(6, -5) = 2
 *   Legal ops: ~ |
 *   Max ops: 7
 *   Rating: 1
 */
int bitXnor(int x, int y) {
  return (~(x|y)) | (~((~x) | (~y)));
}
/* 
 * bitConditional - x ? y : z for each bit respectively
 *   Example: bitConditional(0b00110011, 0b01010101, 0b00001111) = 0b00011101
 *   Legal ops: & | ^ ~
 *   Max ops: 4
 *   Rating: 1
 */
int bitConditional(int x, int y, int z) {
  return (x & y) | ((~x) & z);
}
/* 
 * byteSwap - swaps the nth byte and the mth byte
 *  Examples: byteSwap(0x12345678, 1, 3) = 0x56341278
 *            byteSwap(0xDEADBEEF, 0, 2) = 0xDEEFBEAD
 *  You may assume that 0 <= n <= 3, 0 <= m <= 3
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 16
 *  Rating: 2
 */
int byteSwap(int x, int n, int m) {
    int mask=0, moven=0, movem=0;
    int nn = n << 3, mm = m << 3;
    int tempn = 0x000000ff << nn, tempm = 0x000000ff << mm;
    int ftempn = ~tempn, ftempm = ~tempm;
    mask = (x & ftempn) & ftempm;
    moven = ((x >> nn) << mm) & tempm;
    movem = ((x >> mm) << nn) & tempn;
    return mask | movem | moven;
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int logicalShift(int x, int n) {
    int temp = 0x1 << 31;
    int mask = (temp >> n) << 1;
    return (x >> n) & (~mask);
}
/* 
 * cleanConsecutive1 - change any consecutive 1 to zeros in the binary form of x.
 *   Consecutive 1 means a set of 1 that contains more than one 1.
 *   Examples cleanConsecutive1(0x10) = 0x10
 *            cleanConsecutive1(0xF0) = 0x0
 *            cleanConsecutive1(0xFFFF0001) = 0x1
 *            cleanConsecutive1(0x4F4F4F4F) = 0x40404040
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 4
 */
int cleanConsecutive1(int x){
    int num = ~(0x1 << 31);
    int leftjudge = (x << 1), rightjudge = (x >> 1) & num;
    int left = x & leftjudge, right = x & rightjudge;
    int judge = left | right;
    return judge ^ x;
}
/*
 * leftBitCount - returns count of number of consective 1's in
 *     left-hand (most significant) end of word.
 *   Examples: leftBitCount(-1) = 32, leftBitCount(0xFFF0F0F0) = 12
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 50
 *   Rating: 4
 */
int leftBitCount(int x) {
    int judge16=0, judge8=0, judge4=0, judge2=0, judge1=0;
    int mask1=0, mask2=0, mask3=0, mask4=0, mask5=0;
    int count = 0;
    int temp = 0;
    mask1 = 0xff | (0xff << 8);
    judge16 = (!(~(x | mask1)));
    temp = judge16 << 4;
    count = count + temp;
    x = x << temp;

    mask2 = mask1 | (0xff << 16);
    judge8 = (!(~(x | mask2)));
    temp = judge8 << 3;
    count = count + temp;
    x = x << temp;

    mask3 = mask2 | (0x0f << 24);
    judge4 = (!(~(x | mask3)));
    temp = judge4 << 2;
    count = count + temp;
    x = x << temp;

    mask4 = mask3 | (0x30 << 24);
    judge2 = (!(~(x | mask4)));
    temp = judge2 << 1;
    count = count + temp;
    x = x << temp;

    mask5 = mask3 | (0x70 << 24);
    judge1 = (!(~(x | mask5)));
    temp = judge1;
    count = count + temp;
    x = x << temp;

    judge1 = (!(~(x | mask5)));
    temp = judge1;
    count = count + temp;
    return count;
}
/* 
 * counter1To5 - return 1 + x if x < 5, return 1 otherwise, we ensure that 1<=x<=5
 *   Examples counter1To5(2) = 3,  counter1To5(5) = 1
 *   Legal ops: ~ & ! | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int counter1To5(int x) {
    int mask = 0x00000005;
    int xor = !!((~mask & x) | (mask & ~x));
    int judge = ((~xor) & 0x01) | (xor & ~(0x01));
    x = x >> (judge << 3);
    return x + 1;
}
/* 
 * sameSign - return 1 if x and y have same sign, and 0 otherwise
 *   Examples sameSign(0x12345678, 0) = 1, sameSign(0xFFFFFFFF,0x1) = 0
 *   Legal ops: ! ~ & ! ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int sameSign(int x, int y) {
    int judge = (x ^ y) >> 31;
    return !(judge);
}
/*
 * satMul3 - multiplies by 3, saturating to Tmin or Tmax if overflow
 *  Examples: satMul3(0x10000000) = 0x30000000
 *            satMul3(0x30000000) = 0x7FFFFFFF (Saturate to TMax)
 *            satMul3(0x70000000) = 0x7FFFFFFF (Saturate to TMax)
 *            satMul3(0xD0000000) = 0x80000000 (Saturate to TMin)
 *            satMul3(0xA0000000) = 0x80000000 (Saturate to TMin)
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 3
 */
int satMul3(int x) {
    int mul2 = x << 1;
    int judge1 = (x ^ mul2) >> 31;
    int mul3 = mul2 + x;
    int judge2 = (mul3 ^ x) >> 31;
    int judge = judge1 | judge2;
    int tmax = ~(0x01 << 31);
    int sign = x >> 31;
    return ((tmax ^ sign) & judge) | ((~judge) & mul3);
}
/* 
 * isGreater - if x > y  then return 1, else return 0 
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
    int temp = (x + ((~y) + 1)) >> 31;
    int judge = ((x ^ temp) & (x ^ y)) >> 31;
    int equal = !!(x ^ y);
    return equal & (!(judge ^ temp));
}
/* 
 * subOK - Determine if can compute x-y without overflow
 *   Example: subOK(0x80000000,0x80000000) = 1,
 *            subOK(0x80000000,0x70000000) = 0, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int subOK(int x, int y) {
    int temp = x + ((~y) + 1);
    int judge = ((x ^ temp) & (x ^ y));
    return !(judge >> 31);
}
/*
 * trueFiveEighths - multiplies by 5/8 rounding toward 0,
 *  avoiding errors due to overflow
 *  Examples: trueFiveEighths(11) = 6
 *            trueFiveEighths(-9) = -5
 *            trueFiveEighths(0x30000000) = 0x1E000000 (no overflow)
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 4
 */
int trueFiveEighths(int x)
{
    int memorymul5 = 0x00;
    int judge = (x >> 31);
    int mask = 0x00000007;
    int memory = mask & x;
    int bias = 0x07 & judge;
    x = x >> 3;
    memorymul5 = ((memory << 2) + memory);
    memorymul5 = (memorymul5 + bias) >> 3;
    x = (x << 2) + x;
    x = x + memorymul5;
    return x;
}
/* 
 * float_half - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_half(unsigned uf) {
    unsigned mask = 0x7f800000;
    unsigned exp=0x00;
    unsigned ans = 0x00;
    unsigned maskm = 0x007fffff;
    unsigned m = 0x00;
    unsigned judgetail = 0x00;
    unsigned mm = 0x00;
    m = maskm & uf;
    exp = (mask & uf) >> 23;
    if(exp == 0xff){
        return uf;
    }
    if(exp > 1){
        ans = uf & 0x807fffff;
        exp = exp + (~(0x01) + 1);
        exp = exp << 23;
        return ans | exp;
    }
    else if(exp == 1){
        ans = uf & 0x80000000;
        m = m | 0x00800000;
        mm = m >> 1;
        judgetail = (mm & m) & 0x01;
        return ans | (mm + judgetail);
    }
    else{
        ans = uf & 0xff800000;
        mm = m >> 1;
        judgetail = (mm & m) & 0x01;
        return ans | (mm + judgetail);
    }
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
    unsigned mask = 0x80000000;
    unsigned exp = 0x00; 
    unsigned m = 0x00;
    unsigned sign = 0x00;
    unsigned flag = 0x00;
    unsigned mi = 31;
    if(x == 0x00000000){
        return 0x00;
    }
    sign = mask & x;
    if(x < 0)
        x = ~x + 1;
    m = x;
    while(mi >= 0){
        unsigned temp = 0x01 << mi;
        if((x & temp) == temp){
            exp = exp + mi;
            break;
        }
        mi = mi - 1;
    }
    exp = exp + 127;
    m = m << (32 - mi);
    if((m & 0x00000300) == 0x00000300){
        flag = 0x01;
    }
    if((m & 0x000001ff) > 256){
        flag = 0x01;
    }
    m = m >> 9;
    m = m & 0x007fffff;
    m = m + flag;
    if(m == 0x00800000){
        exp = exp + 1;
        m = 0x00;
    }
    return sign | (exp << 23) | m;
}
/* 
 * float64_f2i - Return bit-level equivalent of expression (int) f
 *   for 64 bit floating point argument f.
 *   Argument is passed as two unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   double-precision floating point value.
 *   Notice: uf1 contains the lower part of the f64 f
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 20
 *   Rating: 4
 *   1 11 52
 */
int float64_f2i(unsigned uf1, unsigned uf2) {
    int sign = 0x00;
    int exp = 0x00;
    unsigned m = 0x00;
    sign = 0x80000000 & uf2;
    exp = (0x7ff00000 & uf2) >> 20;
    if(exp == 0){
        return 0x00;
    }
    exp = exp - 1023;
    m = 0x000fffff & uf2;
    m = m << 12;
    m = m | (0x00000fff & (uf1 >> 20));
    m = (m >> 1) | 0x80000000;
    if(exp > 30){
        return 0x80000000;
    }
    else if(exp < 0){
        return 0x00;
    }
    else{
        m = m >> (31 - exp);
        if(sign == 0x80000000){
            return ~m + 1;
        }
        else{
            return m;
        }
    }
}
/* 
 * float_negpwr2 - Return bit-level equivalent of the expression 2.0^-x
 *   (2.0 raised to the power -x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^-x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 20 
 *   Rating: 4
 */
unsigned float_negpwr2(int x) {
    unsigned exp = 0x01;
    if(x >= 150){
        return 0;
    }
    else if(x >= 127){
        return 0x01 << (149 - x);
    }
    else if(x >= -127){
        exp = 127 - x;
        return exp << 23;
    }
    else{
        return 0x7f800000;
    }
}
