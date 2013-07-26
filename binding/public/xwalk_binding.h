// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_BINDING_PUBLIC_XWALK_BINDING_H_
#define XWALK_BINDING_PUBLIC_XWALK_BINDING_H_

#include "npruntime.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XWALK_BINDING_VERSION 1

typedef void*        (*XWB_MemAllocProcPtr)(uint32_t size);
typedef void         (*XWB_MemFreeProcPtr)(void* ptr);
typedef uint32_t     (*XWB_MemFlushProcPtr)(uint32_t size);
typedef NPIdentifier (*XWB_GetStringIdentifierProcPtr)(const NPUTF8* name);
typedef void         (*XWB_GetStringIdentifiersProcPtr)(
                         const NPUTF8** names,
                         int32_t nameCount,
                         NPIdentifier* identifiers);
typedef NPIdentifier (*XWB_GetIntIdentifierProcPtr)(int32_t intid);
typedef bool         (*XWB_IdentifierIsStringProcPtr)(NPIdentifier identifier);
typedef NPUTF8*      (*XWB_UTF8FromIdentifierProcPtr)(NPIdentifier identifier);
typedef int32_t      (*XWB_IntFromIdentifierProcPtr)(NPIdentifier identifier);
typedef NPObject*    (*XWB_CreateObjectProcPtr)(NPP npp, NPClass* aClass);
typedef NPObject*    (*XWB_RetainObjectProcPtr)(NPObject* obj);
typedef void         (*XWB_ReleaseObjectProcPtr)(NPObject* obj);
typedef bool         (*XWB_InvokeProcPtr)(NPP npp,
                                          NPObject* obj,
                                          NPIdentifier methodName,
                                          const NPVariant* args,
                                          uint32_t argCount,
                                          NPVariant* result);
typedef bool         (*XWB_InvokeDefaultProcPtr)(NPP npp,
                                                 NPObject* obj,
                                                 const NPVariant* args,
                                                 uint32_t argCount,
                                                 NPVariant* result);
typedef bool         (*XWB_EvaluateProcPtr)(NPP npp,
                                            NPObject* obj,
                                            NPString* script,
                                            NPVariant* result);
typedef bool         (*XWB_GetPropertyProcPtr)(NPP npp,
                                               NPObject* obj,
                                               NPIdentifier propertyName,
                                               NPVariant* result);
typedef bool         (*XWB_SetPropertyProcPtr)(NPP npp,
                                               NPObject* obj,
                                               NPIdentifier propertyName,
                                               const NPVariant* value);
typedef bool         (*XWB_RemovePropertyProcPtr)(NPP npp,
                                                  NPObject* obj,
                                                  NPIdentifier propertyName);
typedef bool         (*XWB_HasPropertyProcPtr)(NPP npp,
                                               NPObject* obj,
                                               NPIdentifier propertyName);
typedef bool         (*XWB_HasMethodProcPtr)(NPP npp,
                                             NPObject* obj,
                                             NPIdentifier propertyName);
typedef void         (*XWB_ReleaseVariantValueProcPtr)(NPVariant* variant);
typedef void         (*XWB_SetExceptionProcPtr)(NPObject* obj,
                                                const NPVariant *value);
typedef bool         (*XWB_EnumerateProcPtr)(NPP npp,
                                             NPObject* obj,
                                             NPIdentifier **identifier,
                                             uint32_t* count);
typedef void         (*XWB_BindingThreadAsyncCallProcPtr)(
                         NPP instance,
                         void (*func)(void *data),
                         void* userData);
typedef bool         (*XWB_ConstructProcPtr)(NPP npp,
                                             NPObject* obj,
                                             const NPVariant* args,
                                             uint32_t argCount,
                                             NPVariant* result);
typedef uint32_t     (*XWB_ScheduleTimerPtr)(
                         NPP instance,
                         uint32_t interval,
                         bool repeat,
                         void (*timerFunc)(NPP npp, uint32_t timerID));
typedef void         (*XWB_UnscheduleTimerPtr)(NPP instance, uint32_t timerID);

typedef struct _XWalkBindingFunctions {
  uint16_t size;
  uint16_t version;
  XWB_MemAllocProcPtr memalloc;
  XWB_MemFreeProcPtr memfree;
  XWB_MemFlushProcPtr memflush;
  XWB_GetStringIdentifierProcPtr getstringidentifier;
  XWB_GetStringIdentifiersProcPtr getstringidentifiers;
  XWB_GetIntIdentifierProcPtr getintidentifier;
  XWB_IdentifierIsStringProcPtr identifierisstring;
  XWB_UTF8FromIdentifierProcPtr utf8fromidentifier;
  XWB_IntFromIdentifierProcPtr intfromidentifier;
  XWB_CreateObjectProcPtr createobject;
  XWB_RetainObjectProcPtr retainobject;
  XWB_ReleaseObjectProcPtr releaseobject;
  XWB_InvokeProcPtr invoke;
  XWB_InvokeDefaultProcPtr invokeDefault;
  XWB_EvaluateProcPtr evaluate;
  XWB_GetPropertyProcPtr getproperty;
  XWB_SetPropertyProcPtr setproperty;
  XWB_RemovePropertyProcPtr removeproperty;
  XWB_HasPropertyProcPtr hasproperty;
  XWB_HasMethodProcPtr hasmethod;
  XWB_ReleaseVariantValueProcPtr releasevariantvalue;
  XWB_SetExceptionProcPtr setexception;
  XWB_EnumerateProcPtr enumerate;
  XWB_BindingThreadAsyncCallProcPtr bindingthreadasynccall;
  XWB_ConstructProcPtr construct;
  XWB_ScheduleTimerPtr scheduletimer;
  XWB_UnscheduleTimerPtr unscheduletimer;
} XWalkBindingFunctions;

typedef struct _XWalkBindingFactory {
  const char* feature;
  const char* binding;
  NPObject* (*construct)(NPObject* root);
} XWalkBindingFactory;

typedef bool (*XWalkBinding_InitializeFunc)(const XWalkBindingFunctions* funcs);
typedef const XWalkBindingFactory* (*XWalkBinding_GetFactoryFunc)(void);
typedef void (*XWalkBinding_ShutdownFunc)(void);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // XWALK_BINDING_PUBLIC_XWALK_BINDING_H_
