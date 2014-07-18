/*
 * Copyright (C) 2011 The Android Open Source Project
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

/** @file rs_debug.rsh
 *  \brief Utility debugging routines
 *
 *  Routines intended to be used during application developement.  These should
 *  not be used in shipping applications.  All print a string and value pair to
 *  the standard log.
 *
 */

#ifndef __RS_DEBUG_RSH__
#define __RS_DEBUG_RSH__


/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, float);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, float, float);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, float, float, float);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, float, float, float, float);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, float2);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, float3);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, float4);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, double);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, const rs_matrix4x4 *);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, const rs_matrix3x3 *);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, const rs_matrix2x2 *);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, int);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, uint);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, long);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, unsigned long);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, long long);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, unsigned long long);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, const void *);

#if (defined(RS_VERSION) && (RS_VERSION >= 17))
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, char);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, char2);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, char3);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, char4);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, unsigned char);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, uchar2);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, uchar3);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, uchar4);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, short);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, short2);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, short3);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, short4);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, unsigned short);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, ushort2);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, ushort3);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, ushort4);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, int2);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, int3);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, int4);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, uint2);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, uint3);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, uint4);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, long2);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, long3);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, long4);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, ulong2);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, ulong3);
/**
 * Debug function.  Prints a string and value to the log.
 */
extern void __attribute__((overloadable))
    rsDebug(const char *, ulong4);
#endif  // (defined(RS_VERSION) && (RS_VERSION >= 17))

#define RS_DEBUG(a) rsDebug(#a, a)
#define RS_DEBUG_MARKER rsDebug(__FILE__, __LINE__)

#endif
