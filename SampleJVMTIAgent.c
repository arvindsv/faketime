#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "jvmti.h"

typedef struct {
  /* JVMTI Environment */
  jvmtiEnv      *jvmti;
  jboolean       vm_is_started;
  /* Data access Lock */
  jrawMonitorID  lock;
} GlobalAgentData;

static jvmtiEnv *jvmti = NULL;
static jvmtiCapabilities capa;
static GlobalAgentData *gdata;

typedef jlong (JNICALL *call__jlong) (JNIEnv*, jclass);
call__jlong originalMethodCurrentTimeInMillis = NULL;

static void check_jvmti_error(jvmtiEnv *jvmti, jvmtiError errnum, const char *str)
{
  if (errnum != JVMTI_ERROR_NONE) {
    char *errnum_str;

    errnum_str = NULL;
    (void)(*jvmti)->GetErrorName(jvmti, errnum, &errnum_str);

    printf("ERROR: JVMTI: %d(%s): %s\n", errnum, (errnum_str==NULL?"Unknown":errnum_str), (str==NULL?"":str));
  }
}

/* Enter a critical section by doing a JVMTI Raw Monitor Enter */
static void enter_critical_section(jvmtiEnv *jvmti)
{
  jvmtiError error;

  error = (*jvmti)->RawMonitorEnter(jvmti, gdata->lock);
  check_jvmti_error(jvmti, error, "Cannot enter with raw monitor");
}

/* Exit a critical section by doing a JVMTI Raw Monitor Exit */
static void exit_critical_section(jvmtiEnv *jvmti)
{
  jvmtiError error;

  error = (*jvmti)->RawMonitorExit(jvmti, gdata->lock);
  check_jvmti_error(jvmti, error, "Cannot exit with raw monitor");
}

JNIEXPORT jlong JNICALL newCurrentTimeInMillis(JNIEnv* env, jclass jc) {
  jlong realTime = originalMethodCurrentTimeInMillis(env, jc);
  return realTime + 400000000000;
}

void JNICALL callbackNativeMethodBind(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method, void* address, void** new_address_ptr) {
  enter_critical_section(jvmti); {
    char *methodName = NULL;
    char *declaringClassName = NULL;
    jclass declaring_class;
    jvmtiError err;

    err = (*jvmti)->GetMethodName(jvmti, method, &methodName, NULL, NULL);
    if (err == JVMTI_ERROR_NONE && strcmp("currentTimeMillis", methodName) == 0) {
      err = (*jvmti)->GetMethodDeclaringClass(jvmti, method, &declaring_class);
      err = (*jvmti)->GetClassSignature(jvmti, declaring_class, &declaringClassName, NULL);
      if (err == JVMTI_ERROR_NONE && strcmp("Ljava/lang/System;", declaringClassName) == 0) {
        printf("at method %s in class %s (new: %p, old: %p).\n", methodName, declaringClassName, *new_address_ptr, address);
        if (originalMethodCurrentTimeInMillis == NULL) {
          originalMethodCurrentTimeInMillis = address;
          *new_address_ptr = (void *) &newCurrentTimeInMillis;
        }
      }
      check_jvmti_error(jvmti_env, (*jvmti)->Deallocate(jvmti, (unsigned char *)declaringClassName), "Nope");
    }

    check_jvmti_error(jvmti_env, (*jvmti)->Deallocate(jvmti, (unsigned char *)methodName), "Err");
  }
  exit_critical_section(jvmti);
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved)
{
  static GlobalAgentData data;
  jvmtiEventCallbacks callbacks;

  jint res = (*jvm)->GetEnv(jvm, (void **) &jvmti, JVMTI_VERSION_1_0);
  if (res != JNI_OK || jvmti == NULL) {
    printf("ERROR: Unable to access JVMTI Version 1");
  }

  (void)memset((void*)&data, 0, sizeof(data));
  gdata = &data;
  gdata->jvmti = jvmti;

  (void)memset(&capa, 0, sizeof(jvmtiCapabilities));
  capa.can_get_owned_monitor_info = 1;
  capa.can_generate_native_method_bind_events = 1;
  capa.can_generate_all_class_hook_events = 1;
  capa.can_tag_objects = 1;	
  check_jvmti_error(jvmti, (*jvmti)->AddCapabilities(jvmti, &capa), "Unable to get necessary JVMTI capabilities.");

  (void)memset(&callbacks, 0, sizeof(callbacks));
  callbacks.NativeMethodBind = &callbackNativeMethodBind;
  check_jvmti_error(jvmti, (*jvmti)->SetEventCallbacks(jvmti, &callbacks, (jint)sizeof(callbacks)), "Cannot set jvmti callbacks");

  check_jvmti_error(jvmti, (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_NATIVE_METHOD_BIND, (jthread)NULL), "Cannot set event notification");

  check_jvmti_error(jvmti, (*jvmti)->CreateRawMonitor(jvmti, "agent data", &(gdata->lock)), "Cannot create raw monitor");

  return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm)
{
}
