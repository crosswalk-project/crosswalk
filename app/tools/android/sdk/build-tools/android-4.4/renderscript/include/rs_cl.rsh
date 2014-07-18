/*
 * Copyright (C) 2011-2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @file rs_cl.rsh
 *  \brief Basic math functions
 *
 *
 */

#ifndef __RS_CL_RSH__
#define __RS_CL_RSH__

// Conversions
#define CVT_FUNC_2(typeout, typein)                             \
_RS_RUNTIME typeout##2 __attribute__((const, overloadable))     \
        convert_##typeout##2(typein##2 v);                      \
_RS_RUNTIME typeout##3 __attribute__((const, overloadable))     \
        convert_##typeout##3(typein##3 v);                      \
_RS_RUNTIME typeout##4 __attribute__((const, overloadable))     \
        convert_##typeout##4(typein##4 v);


#define CVT_FUNC(type)  CVT_FUNC_2(type, uchar)     \
                        CVT_FUNC_2(type, char)      \
                        CVT_FUNC_2(type, ushort)    \
                        CVT_FUNC_2(type, short)     \
                        CVT_FUNC_2(type, uint)      \
                        CVT_FUNC_2(type, int)       \
                        CVT_FUNC_2(type, float)

/**
 * Convert to char.
 *
 * Supports 2,3,4 components of uchar, char, ushort, short, uint, int, float.
 */
CVT_FUNC(char)

/**
 * Convert to unsigned char.
 *
 * Supports 2,3,4 components of uchar, char, ushort, short, uint, int, float.
 */
CVT_FUNC(uchar)

/**
 * Convert to short.
 *
 * Supports 2,3,4 components of uchar, char, ushort, short, uint, int, float.
 */
CVT_FUNC(short)

/**
 * Convert to unsigned short.
 *
 * Supports 2,3,4 components of uchar, char, ushort, short, uint, int, float.
 */
CVT_FUNC(ushort)

/**
 * Convert to int.
 *
 * Supports 2,3,4 components of uchar, char, ushort, short, uint, int, float.
 */
CVT_FUNC(int)

/**
 * Convert to unsigned int.
 *
 * Supports 2,3,4 components of uchar, char, ushort, short, uint, int, float.
 */
CVT_FUNC(uint)

/**
 * Convert to float.
 *
 * Supports 2,3,4 components of uchar, char, ushort, short, uint, int, float.
 */
CVT_FUNC(float)

// Float ops, 6.11.2

#ifdef DOXYGEN

#define FN_FUNC_FN(fnc)
#define F_FUNC_FN(fnc)
#define IN_FUNC_FN(fnc)
#define FN_FUNC_FN_FN(fnc)
#define F_FUNC_FN_FN(fnc)
#define FN_FUNC_FN_F(fnc)
#define FN_FUNC_FN_IN(fnc)
#define FN_FUNC_FN_I(fnc)
#define FN_FUNC_FN_PFN(fnc)
#define FN_FUNC_FN_PIN(fnc)
#define FN_FUNC_FN_FN_FN(fnc)
#define FN_FUNC_FN_FN_F(fnc)
#define FN_FUNC_FN_F_F(fnc)
#define FN_FUNC_FN_FN_PIN(fnc)

#else

#define FN_FUNC_FN(fnc)                                                \
_RS_RUNTIME float2 __attribute__((const, overloadable)) fnc(float2 v); \
_RS_RUNTIME float3 __attribute__((const, overloadable)) fnc(float3 v); \
_RS_RUNTIME float4 __attribute__((const, overloadable)) fnc(float4 v);

#define F_FUNC_FN(fnc)                                                \
_RS_RUNTIME float __attribute__((const, overloadable)) fnc(float2 v); \
_RS_RUNTIME float __attribute__((const, overloadable)) fnc(float3 v); \
_RS_RUNTIME float __attribute__((const, overloadable)) fnc(float4 v);

#define IN_FUNC_FN(fnc)                                              \
_RS_RUNTIME int2 __attribute__((const, overloadable)) fnc(float2 v); \
_RS_RUNTIME int3 __attribute__((const, overloadable)) fnc(float3 v); \
_RS_RUNTIME int4 __attribute__((const, overloadable)) fnc(float4 v);

#define FN_FUNC_FN_FN(fnc)                                                         \
_RS_RUNTIME float2 __attribute__((const, overloadable)) fnc(float2 v1, float2 v2); \
_RS_RUNTIME float3 __attribute__((const, overloadable)) fnc(float3 v1, float3 v2); \
_RS_RUNTIME float4 __attribute__((const, overloadable)) fnc(float4 v1, float4 v2);

#define F_FUNC_FN_FN(fnc)                                                         \
_RS_RUNTIME float __attribute__((const, overloadable)) fnc(float2 v1, float2 v2); \
_RS_RUNTIME float __attribute__((const, overloadable)) fnc(float3 v1, float3 v2); \
_RS_RUNTIME float __attribute__((const, overloadable)) fnc(float4 v1, float4 v2);

#define FN_FUNC_FN_F(fnc)                                                         \
_RS_RUNTIME float2 __attribute__((const, overloadable)) fnc(float2 v1, float v2); \
_RS_RUNTIME float3 __attribute__((const, overloadable)) fnc(float3 v1, float v2); \
_RS_RUNTIME float4 __attribute__((const, overloadable)) fnc(float4 v1, float v2);

#define FN_FUNC_FN_IN(fnc)                                                       \
_RS_RUNTIME float2 __attribute__((const, overloadable)) fnc(float2 v1, int2 v2); \
_RS_RUNTIME float3 __attribute__((const, overloadable)) fnc(float3 v1, int3 v2); \
_RS_RUNTIME float4 __attribute__((const, overloadable)) fnc(float4 v1, int4 v2);

#define FN_FUNC_FN_I(fnc)                                                       \
_RS_RUNTIME float2 __attribute__((const, overloadable)) fnc(float2 v1, int v2); \
_RS_RUNTIME float3 __attribute__((const, overloadable)) fnc(float3 v1, int v2); \
_RS_RUNTIME float4 __attribute__((const, overloadable)) fnc(float4 v1, int v2);

#define FN_FUNC_FN_PFN(fnc)                            \
_RS_RUNTIME float2 __attribute__((pure, overloadable)) \
        fnc(float2 v1, float2 *v2);                    \
_RS_RUNTIME float3 __attribute__((pure, overloadable)) \
        fnc(float3 v1, float3 *v2);                    \
_RS_RUNTIME float4 __attribute__((pure, overloadable)) \
        fnc(float4 v1, float4 *v2);

#define FN_FUNC_FN_PIN(fnc)                                                      \
_RS_RUNTIME float2 __attribute__((pure, overloadable)) fnc(float2 v1, int2 *v2); \
_RS_RUNTIME float3 __attribute__((pure, overloadable)) fnc(float3 v1, int3 *v2); \
_RS_RUNTIME float4 __attribute__((pure, overloadable)) fnc(float4 v1, int4 *v2);

#define FN_FUNC_FN_FN_FN(fnc)                           \
_RS_RUNTIME float2 __attribute__((const, overloadable)) \
        fnc(float2 v1, float2 v2, float2 v3);           \
_RS_RUNTIME float3 __attribute__((const, overloadable)) \
        fnc(float3 v1, float3 v2, float3 v3);           \
_RS_RUNTIME float4 __attribute__((const, overloadable)) \
        fnc(float4 v1, float4 v2, float4 v3);

#define FN_FUNC_FN_FN_F(fnc)                            \
_RS_RUNTIME float2 __attribute__((const, overloadable)) \
        fnc(float2 v1, float2 v2, float v3);            \
_RS_RUNTIME float3 __attribute__((const, overloadable)) \
        fnc(float3 v1, float3 v2, float v3);            \
_RS_RUNTIME float4 __attribute__((const, overloadable)) \
        fnc(float4 v1, float4 v2, float v3);

#define FN_FUNC_FN_F_F(fnc)                             \
_RS_RUNTIME float2 __attribute__((const, overloadable)) \
        fnc(float2 v1, float v2, float v3);             \
_RS_RUNTIME float3 __attribute__((const, overloadable)) \
        fnc(float3 v1, float v2, float v3);             \
_RS_RUNTIME float4 __attribute__((const, overloadable)) \
        fnc(float4 v1, float v2, float v3);

#define FN_FUNC_FN_FN_PIN(fnc)                         \
_RS_RUNTIME float2 __attribute__((pure, overloadable)) \
        fnc(float2 v1, float2 v2, int2 *v3);           \
_RS_RUNTIME float3 __attribute__((pure, overloadable)) \
        fnc(float3 v1, float3 v2, int3 *v3);           \
_RS_RUNTIME float4 __attribute__((pure, overloadable)) \
        fnc(float4 v1, float4 v2, int4 *v3);

#endif  // DOXYGEN


/**
 * Return the inverse cosine.
 *
 * Supports float, float2, float3, float4
 */
extern float __attribute__((const, overloadable)) acos(float);
FN_FUNC_FN(acos)

/**
 * Return the inverse hyperbolic cosine.
 *
 * Supports float, float2, float3, float4
 */
extern float __attribute__((const, overloadable)) acosh(float);
FN_FUNC_FN(acosh)

/**
 * Return the inverse cosine divided by PI.
 *
 * Supports float, float2, float3, float4
 */
_RS_RUNTIME float __attribute__((const, overloadable)) acospi(float v);
FN_FUNC_FN(acospi)

/**
 * Return the inverse sine.
 *
 * Supports float, float2, float3, float4
 */
extern float __attribute__((const, overloadable)) asin(float);
FN_FUNC_FN(asin)

/**
 * Return the inverse hyperbolic sine.
 *
 * Supports float, float2, float3, float4
 */
extern float __attribute__((const, overloadable)) asinh(float);
FN_FUNC_FN(asinh)


/**
 * Return the inverse sine divided by PI.
 *
 * Supports float, float2, float3, float4
 */
_RS_RUNTIME float __attribute__((const, overloadable)) asinpi(float v);
FN_FUNC_FN(asinpi)

/**
 * Return the inverse tangent.
 *
 * Supports float, float2, float3, float4
 */
extern float __attribute__((const, overloadable)) atan(float);
FN_FUNC_FN(atan)

/**
 * Return the inverse tangent of y / x.
 *
 * Supports float, float2, float3, float4.  Both arguments must be of the same
 * type.
 *
 * @param y
 * @param x
 */
extern float __attribute__((const, overloadable)) atan2(float y, float x);
FN_FUNC_FN_FN(atan2)

/**
 * Return the inverse hyperbolic tangent.
 *
 * Supports float, float2, float3, float4
 */
extern float __attribute__((const, overloadable)) atanh(float);
FN_FUNC_FN(atanh)

/**
 * Return the inverse tangent divided by PI.
 *
 * Supports float, float2, float3, float4
 */
_RS_RUNTIME float __attribute__((const, overloadable)) atanpi(float v);
FN_FUNC_FN(atanpi)

/**
 * Return the inverse tangent of y / x, divided by PI.
 *
 * Supports float, float2, float3, float4.  Both arguments must be of the same
 * type.
 *
 * @param y
 * @param x
 */
_RS_RUNTIME float __attribute__((const, overloadable)) atan2pi(float y, float x);
FN_FUNC_FN_FN(atan2pi)


/**
 * Return the cube root.
 *
 * Supports float, float2, float3, float4.
 */
extern float __attribute__((const, overloadable)) cbrt(float);
FN_FUNC_FN(cbrt)

/**
 * Return the smallest integer not less than a value.
 *
 * Supports float, float2, float3, float4.
 */
extern float __attribute__((const, overloadable)) ceil(float);
FN_FUNC_FN(ceil)

/**
 * Copy the sign bit from y to x.
 *
 * Supports float, float2, float3, float4.  Both arguments must be of the same
 * type.
 *
 * @param x
 * @param y
 */
extern float __attribute__((const, overloadable)) copysign(float x, float y);
FN_FUNC_FN_FN(copysign)

/**
 * Return the cosine.
 *
 * Supports float, float2, float3, float4.
 */
extern float __attribute__((const, overloadable)) cos(float);
FN_FUNC_FN(cos)

/**
 * Return the hypebolic cosine.
 *
 * Supports float, float2, float3, float4.
 */
extern float __attribute__((const, overloadable)) cosh(float);
FN_FUNC_FN(cosh)

/**
 * Return the cosine of the value * PI.
 *
 * Supports float, float2, float3, float4.
 */
_RS_RUNTIME float __attribute__((const, overloadable)) cospi(float v);
FN_FUNC_FN(cospi)

/**
 * Return the complementary error function.
 *
 * Supports float, float2, float3, float4.
 */
extern float __attribute__((const, overloadable)) erfc(float);
FN_FUNC_FN(erfc)

/**
 * Return the error function.
 *
 * Supports float, float2, float3, float4.
 */
extern float __attribute__((const, overloadable)) erf(float);
FN_FUNC_FN(erf)

/**
 * Return e ^ value.
 *
 * Supports float, float2, float3, float4.
 */
extern float __attribute__((const, overloadable)) exp(float);
FN_FUNC_FN(exp)

/**
 * Return 2 ^ value.
 *
 * Supports float, float2, float3, float4.
 */
extern float __attribute__((const, overloadable)) exp2(float);
FN_FUNC_FN(exp2)

/**
 * Return x ^ y.
 *
 * Supports float, float2, float3, float4. Both arguments must be of the same
 * type.
 */
extern float __attribute__((const, overloadable)) pow(float x, float y);
FN_FUNC_FN_FN(pow)

/**
 * Return 10 ^ value.
 *
 * Supports float, float2, float3, float4.
 */
_RS_RUNTIME float __attribute__((const, overloadable)) exp10(float v);
FN_FUNC_FN(exp10)

/**
 * Return (e ^ value) - 1.
 *
 * Supports float, float2, float3, float4.
 */
extern float __attribute__((const, overloadable)) expm1(float);
FN_FUNC_FN(expm1)

/**
 * Return the absolute value of a value.
 *
 * Supports float, float2, float3, float4.
 */
extern float __attribute__((const, overloadable)) fabs(float);
FN_FUNC_FN(fabs)

/**
 * Return the positive difference between two values.
 *
 * Supports float, float2, float3, float4.  Both arguments must be of the same
 * type.
 */
extern float __attribute__((const, overloadable)) fdim(float, float);
FN_FUNC_FN_FN(fdim)

/**
 * Return the smallest integer not greater than a value.
 *
 * Supports float, float2, float3, float4.
 */
extern float __attribute__((const, overloadable)) floor(float);
FN_FUNC_FN(floor)

/**
 * Return a*b + c.
 *
 * Supports float, float2, float3, float4.
 */
extern float __attribute__((const, overloadable)) fma(float a, float b, float c);
FN_FUNC_FN_FN_FN(fma)

/**
 * Return (x < y ? y : x)
 *
 * Supports float, float2, float3, float4.
 * @param x: may be float, float2, float3, float4
 * @param y: may be float or vector.  If vector must match type of x.
 */
extern float __attribute__((const, overloadable)) fmax(float x, float y);
FN_FUNC_FN_FN(fmax);
FN_FUNC_FN_F(fmax);

/**
 * Return (x > y ? y : x)
 *
 * @param x: may be float, float2, float3, float4
 * @param y: may be float or vector.  If vector must match type of x.
 */
extern float __attribute__((const, overloadable)) fmin(float x, float y);
FN_FUNC_FN_FN(fmin);
FN_FUNC_FN_F(fmin);

/**
 * Return the remainder from x / y
 *
 * Supports float, float2, float3, float4.
 */
extern float __attribute__((const, overloadable)) fmod(float x, float y);
FN_FUNC_FN_FN(fmod)

/**
 * Return fractional part of v
 *
 * @param iptr  iptr[0] will be set to the floor of the input value.
 * Supports float, float2, float3, float4.
 */
_RS_RUNTIME float __attribute__((pure, overloadable)) fract(float v, float *iptr);
FN_FUNC_FN_PFN(fract)

/**
 * Return fractional part of v
 *
 * Supports float, float2, float3, float4.
 */
static inline float __attribute__((const, overloadable)) fract(float v) {
    float unused;
    return fract(v, &unused);
}

static inline float2 __attribute__((const, overloadable)) fract(float2 v) {
    float2 unused;
    return fract(v, &unused);
}

static inline float3 __attribute__((const, overloadable)) fract(float3 v) {
    float3 unused;
    return fract(v, &unused);
}

static inline float4 __attribute__((const, overloadable)) fract(float4 v) {
    float4 unused;
    return fract(v, &unused);
}

/**
 * Return the mantissa and place the exponent into iptr[0]
 *
 * @param v Supports float, float2, float3, float4.
 * @param iptr  Must have the same vector size as v.
 */
extern float __attribute__((pure, overloadable)) frexp(float v, int *iptr);
FN_FUNC_FN_PIN(frexp)

/**
 * Return sqrt(x*x + y*y)
 *
 * Supports float, float2, float3, float4.
 */
extern float __attribute__((const, overloadable)) hypot(float x, float y);
FN_FUNC_FN_FN(hypot)

/**
 * Return the integer exponent of a value
 *
 * Supports 1,2,3,4 components
 */
extern int __attribute__((const, overloadable)) ilogb(float);
IN_FUNC_FN(ilogb)

/**
 * Return (x * 2^y)
 *
 * @param x Supports 1,2,3,4 components
 * @param y Supports single component or matching vector.
 */
extern float __attribute__((const, overloadable)) ldexp(float x, int y);
FN_FUNC_FN_IN(ldexp)
FN_FUNC_FN_I(ldexp)

/**
 * Return the log gamma
 *
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) lgamma(float);
FN_FUNC_FN(lgamma)

/**
 * Return the log gamma and sign
 *
 * @param x Supports 1,2,3,4 components
 * @param y Supports matching vector.
 */
extern float __attribute__((pure, overloadable)) lgamma(float x, int* y);
FN_FUNC_FN_PIN(lgamma)

/**
 * Return the natural logarithm
 *
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) log(float);
FN_FUNC_FN(log)

/**
 * Return the base 10 logarithm
 *
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) log10(float);
FN_FUNC_FN(log10)

/**
 * Return the base 2 logarithm
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) log2(float v);
FN_FUNC_FN(log2)

/**
 * Return the natural logarithm of (v + 1.0f)
 *
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) log1p(float v);
FN_FUNC_FN(log1p)

/**
 * Compute the exponent of the value.
 *
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) logb(float);
FN_FUNC_FN(logb)

/**
 * Compute (a * b) + c
 *
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) mad(float a, float b, float c);
FN_FUNC_FN_FN_FN(mad)

/**
 * Return the integral and fractional components of a number.
 * Supports 1,2,3,4 components
 *
 * @param x Source value
 * @param iret iret[0] will be set to the integral portion of the number.
 * @return The floating point portion of the value.
 */
extern float __attribute__((pure, overloadable)) modf(float x, float *iret);
FN_FUNC_FN_PFN(modf);

extern float __attribute__((const, overloadable)) nan(uint);

/**
 * Return the next floating point number from x towards y.
 *
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) nextafter(float x, float y);
FN_FUNC_FN_FN(nextafter)

/**
 * Return (v ^ p).
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) pown(float v, int p);
FN_FUNC_FN_IN(pown)

/**
 * Return (v ^ p).
 * @param v must be greater than 0.
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) powr(float v, float p);
FN_FUNC_FN_FN(powr)

/**
 * Return round x/y to the nearest integer then compute the remander.
 *
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) remainder(float x, float y);
FN_FUNC_FN_FN(remainder)

// document once we know the precision of bionic
extern float __attribute__((pure, overloadable)) remquo(float, float, int *);
FN_FUNC_FN_FN_PIN(remquo)

/**
 * Round to the nearest integral value.
 *
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) rint(float);
FN_FUNC_FN(rint)

/**
 * Compute the Nth root of a value.
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) rootn(float v, int n);
FN_FUNC_FN_IN(rootn)

/**
 * Round to the nearest integral value.  Half values are rounded away from zero.
 *
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) round(float);
FN_FUNC_FN(round)

/**
 * Return the square root of a value.
 *
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) sqrt(float);
FN_FUNC_FN(sqrt)

/**
 * Return (1 / sqrt(value)).
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) rsqrt(float v);
FN_FUNC_FN(rsqrt)

/**
 * Return the sine of a value specified in radians.
 *
 * @param v The incoming value in radians
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) sin(float v);
FN_FUNC_FN(sin)

/**
 * Return the sine and cosine of a value.
 *
 * @return sine
 * @param v The incoming value in radians
 * @param *cosptr cosptr[0] will be set to the cosine value.
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((pure, overloadable)) sincos(float v, float *cosptr);
FN_FUNC_FN_PFN(sincos);

/**
 * Return the hyperbolic sine of a value specified in radians.
 *
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) sinh(float);
FN_FUNC_FN(sinh)

/**
 * Return the sin(v * PI).
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) sinpi(float v);
FN_FUNC_FN(sinpi)

/**
 * Return the tangent of a value.
 *
 * Supports 1,2,3,4 components
 * @param v The incoming value in radians
 */
extern float __attribute__((const, overloadable)) tan(float v);
FN_FUNC_FN(tan)

/**
 * Return the hyperbolic tangent of a value.
 *
 * Supports 1,2,3,4 components
 * @param v The incoming value in radians
 */
extern float __attribute__((const, overloadable)) tanh(float);
FN_FUNC_FN(tanh)

/**
 * Return tan(v * PI)
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) tanpi(float v);
FN_FUNC_FN(tanpi)

/**
 * Compute the gamma function of a value.
 *
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) tgamma(float);
FN_FUNC_FN(tgamma)

/**
 * Round to integral using truncation.
 *
 * Supports 1,2,3,4 components
 */
extern float __attribute__((const, overloadable)) trunc(float);
FN_FUNC_FN(trunc)

#ifdef DOXYGEN

#define XN_FUNC_YN(typeout, fnc, typein)                                \
extern typeout __attribute__((overloadable)) fnc(typein v);

#define XN_FUNC_XN_XN_BODY(type, fnc, body)         \
_RS_RUNTIME type __attribute__((overloadable))      \
        fnc(type v1, type v2);

#else

#define XN_FUNC_YN(typeout, fnc, typein)                                      \
extern typeout __attribute__((const, overloadable)) fnc(typein v);            \
_RS_RUNTIME typeout##2 __attribute__((const, overloadable)) fnc(typein##2 v); \
_RS_RUNTIME typeout##3 __attribute__((const, overloadable)) fnc(typein##3 v); \
_RS_RUNTIME typeout##4 __attribute__((const, overloadable)) fnc(typein##4 v);

#define XN_FUNC_XN_XN_BODY(type, fnc, body)              \
_RS_RUNTIME type __attribute__((const, overloadable))    \
        fnc(type v1, type v2);                           \
_RS_RUNTIME type##2 __attribute__((const, overloadable)) \
        fnc(type##2 v1, type##2 v2);                     \
_RS_RUNTIME type##3 __attribute__((const, overloadable)) \
        fnc(type##3 v1, type##3 v2);                     \
_RS_RUNTIME type##4 __attribute__((const, overloadable)) \
        fnc(type##4 v1, type##4 v2);

#endif  // DOXYGEN

#define UIN_FUNC_IN(fnc)          \
XN_FUNC_YN(uchar, fnc, char)      \
XN_FUNC_YN(ushort, fnc, short)    \
XN_FUNC_YN(uint, fnc, int)

#define IN_FUNC_IN(fnc)           \
XN_FUNC_YN(uchar, fnc, uchar)     \
XN_FUNC_YN(char, fnc, char)       \
XN_FUNC_YN(ushort, fnc, ushort)   \
XN_FUNC_YN(short, fnc, short)     \
XN_FUNC_YN(uint, fnc, uint)       \
XN_FUNC_YN(int, fnc, int)

#define IN_FUNC_IN_IN_BODY(fnc, body)   \
XN_FUNC_XN_XN_BODY(uchar, fnc, body)    \
XN_FUNC_XN_XN_BODY(char, fnc, body)     \
XN_FUNC_XN_XN_BODY(ushort, fnc, body)   \
XN_FUNC_XN_XN_BODY(short, fnc, body)    \
XN_FUNC_XN_XN_BODY(uint, fnc, body)     \
XN_FUNC_XN_XN_BODY(int, fnc, body)      \
XN_FUNC_XN_XN_BODY(float, fnc, body)

/**
 * \fn uchar abs(char)
 * Return the absolute value of a value.
 *
 * Supports 1,2,3,4 components of char, short, int.
 */
UIN_FUNC_IN(abs)

/**
 * Return the number of leading 0-bits in a value.
 *
 * Supports 1,2,3,4 components of uchar, char, ushort, short, uint, int.
 */
IN_FUNC_IN(clz)

/**
 * Return the minimum of two values.
 *
 * Supports 1,2,3,4 components of uchar, char, ushort, short, uint, int, float.
 */
IN_FUNC_IN_IN_BODY(min, (v1 < v2 ? v1 : v2))
FN_FUNC_FN_F(min)

/**
 * Return the maximum of two values.
 *
 * Supports 1,2,3,4 components of uchar, char, ushort, short, uint, int, float.
 */
IN_FUNC_IN_IN_BODY(max, (v1 > v2 ? v1 : v2))
FN_FUNC_FN_F(max)

/**
 *  Clamp a value to a specified high and low bound.
 *
 * @param amount value to be clamped.  Supports 1,2,3,4 components
 * @param low Lower bound, must be scalar or matching vector.
 * @param high High bound, must match type of low
 */

#if !defined(RS_VERSION) || (RS_VERSION < 19)
_RS_RUNTIME float __attribute__((const, overloadable)) clamp(float amount, float low, float high);
FN_FUNC_FN_FN_FN(clamp)
FN_FUNC_FN_F_F(clamp)
#else
#define _CLAMP(T)                                                                   \
extern T __attribute__((overloadable)) clamp(T amount, T low, T high);              \
extern T##2 __attribute__((overloadable)) clamp(T##2 amount, T##2 low, T##2 high);  \
extern T##3 __attribute__((overloadable)) clamp(T##3 amount, T##3 low, T##3 high);  \
extern T##4 __attribute__((overloadable)) clamp(T##4 amount, T##4 low, T##4 high);  \
extern T##2 __attribute__((overloadable)) clamp(T##2 amount, T low, T high);        \
extern T##3 __attribute__((overloadable)) clamp(T##3 amount, T low, T high);        \
extern T##4 __attribute__((overloadable)) clamp(T##4 amount, T low, T high)

_CLAMP(float);
_CLAMP(double);
_CLAMP(char);
_CLAMP(uchar);
_CLAMP(short);
_CLAMP(ushort);
_CLAMP(int);
_CLAMP(uint);
_CLAMP(long);
_CLAMP(ulong);

#undef _CLAMP
#endif

/**
 * Convert from radians to degrees.
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) degrees(float radians);
FN_FUNC_FN(degrees)

/**
 * return start + ((stop - start) * amount);
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) mix(float start, float stop, float amount);
FN_FUNC_FN_FN_FN(mix)
FN_FUNC_FN_FN_F(mix)

/**
 * Convert from degrees to radians.
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) radians(float degrees);
FN_FUNC_FN(radians)

/**
 * if (v < edge)
 *     return 0.f;
 * else
 *     return 1.f;
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) step(float edge, float v);
FN_FUNC_FN_FN(step)
FN_FUNC_FN_F(step)

// not implemented
extern float __attribute__((const, overloadable)) smoothstep(float, float, float);
extern float2 __attribute__((const, overloadable)) smoothstep(float2, float2, float2);
extern float3 __attribute__((const, overloadable)) smoothstep(float3, float3, float3);
extern float4 __attribute__((const, overloadable)) smoothstep(float4, float4, float4);
extern float2 __attribute__((const, overloadable)) smoothstep(float, float, float2);
extern float3 __attribute__((const, overloadable)) smoothstep(float, float, float3);
extern float4 __attribute__((const, overloadable)) smoothstep(float, float, float4);

/**
 * Return the sign of a value.
 *
 * if (v < 0) return -1.f;
 * else if (v > 0) return 1.f;
 * else return 0.f;
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) sign(float v);
FN_FUNC_FN(sign)

/**
 * Compute the cross product of two vectors.
 *
 * Supports 3,4 components
 */
_RS_RUNTIME float3 __attribute__((const, overloadable)) cross(float3 lhs, float3 rhs);
_RS_RUNTIME float4 __attribute__((const, overloadable)) cross(float4 lhs, float4 rhs);

/**
 * Compute the dot product of two vectors.
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) dot(float lhs, float rhs);
F_FUNC_FN_FN(dot)

/**
 * Compute the length of a vector.
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) length(float v);
F_FUNC_FN(length)

/**
 * Compute the distance between two points.
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) distance(float lhs, float rhs);
F_FUNC_FN_FN(distance)

/**
 * Normalize a vector.
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) normalize(float v);
FN_FUNC_FN(normalize)


// New approx API functions
#if (defined(RS_VERSION) && (RS_VERSION >= 17))

/**
 * Return the approximate reciprocal of a value.
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) half_recip(float);
FN_FUNC_FN(half_recip)

/**
 * Return the approximate square root of a value.
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) half_sqrt(float);
FN_FUNC_FN(half_sqrt)

/**
 * Return the approximate value of (1 / sqrt(value)).
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) half_rsqrt(float v);
FN_FUNC_FN(half_rsqrt)

/**
 * Compute the approximate length of a vector.
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) fast_length(float v);
F_FUNC_FN(fast_length)

/**
 * Compute the approximate distance between two points.
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) fast_distance(float lhs, float rhs);
F_FUNC_FN_FN(fast_distance)

/**
 * Approximately normalize a vector.
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) fast_normalize(float v);
F_FUNC_FN(fast_normalize)

#endif  // (defined(RS_VERSION) && (RS_VERSION >= 17))



#if (defined(RS_VERSION) && (RS_VERSION >= 18))
// Fast native math functions.


/**
 * Fast approximate exp2
 * valid for inputs -125.f to 125.f
 * Max 8192 ulps of error
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) native_exp2(float v);
FN_FUNC_FN(native_exp2)

/**
 * Fast approximate exp
 * valid for inputs -86.f to 86.f
 * Max 8192 ulps of error
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) native_exp(float v);
FN_FUNC_FN(native_exp)

/**
 * Fast approximate exp10
 * valid for inputs -37.f to 37.f
 * Max 8192 ulps of error
 *
 * Supports 1,2,3,4 components
 */
_RS_RUNTIME float __attribute__((const, overloadable)) native_exp10(float v);
FN_FUNC_FN(native_exp10)


_RS_RUNTIME float __attribute__((const, overloadable)) native_log2(float v);
FN_FUNC_FN(native_log2)

_RS_RUNTIME float __attribute__((const, overloadable)) native_log(float v);
FN_FUNC_FN(native_log)

_RS_RUNTIME float __attribute__((const, overloadable)) native_log10(float v);
FN_FUNC_FN(native_log10)


_RS_RUNTIME float __attribute__((const, overloadable)) native_powr(float v, float y);
FN_FUNC_FN_FN(native_powr)


#endif  // (defined(RS_VERSION) && (RS_VERSION >= 18))


#undef CVT_FUNC
#undef CVT_FUNC_2
#undef FN_FUNC_FN
#undef F_FUNC_FN
#undef IN_FUNC_FN
#undef FN_FUNC_FN_FN
#undef F_FUNC_FN_FN
#undef FN_FUNC_FN_F
#undef FN_FUNC_FN_IN
#undef FN_FUNC_FN_I
#undef FN_FUNC_FN_PFN
#undef FN_FUNC_FN_PIN
#undef FN_FUNC_FN_FN_FN
#undef FN_FUNC_FN_FN_F
#undef FN_FUNC_FN_F_F
#undef FN_FUNC_FN_FN_PIN
#undef XN_FUNC_YN
#undef UIN_FUNC_IN
#undef IN_FUNC_IN
#undef XN_FUNC_XN_XN_BODY
#undef IN_FUNC_IN_IN_BODY

#endif
