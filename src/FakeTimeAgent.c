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

typedef jlong (JNICALL *call__jlong) (JNIEnv*, jclass);

static jvmtiEnv *jvmti = NULL;
static GlobalAgentData *gdata;
static call__jlong originalMethodCurrentTimeInMillis = NULL;

// Read the offset in seconds from a text file and convert it to a long in milliseconds.
long readOffsetFromFile(const char* filePath) {
  FILE* pFile = fopen(filePath, "r");

  if (pFile == NULL) {
    return 0L;
  }

  // Find the size of the file
  fseek(pFile, 0, SEEK_END);
  long lSize = ftell(pFile);
  rewind(pFile);

  // Allocate a buffer for the file contents, adding room for NULL-termination.
  char* buffer = (char*) malloc(sizeof(char) * (lSize + 1));
  fread(buffer, sizeof(char), lSize, pFile);
  fclose(pFile);

  buffer[lSize] = '\0'; // Make sure the string is null terminated

  // Convert the offset string to a long and multiply by 1000 to get milliseconds
  long offset = atol(buffer) * 1000;

  free(buffer);

  return offset;
}

JNIEXPORT jlong JNICALL newCurrentTimeInMillis(JNIEnv* env, jclass jc) {
  jclass systemClass = (*env)->FindClass(env, "java/lang/System");
  jmethodID getPropertyMethodId = (*env)->GetStaticMethodID(env, systemClass, "getProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");

  if (!gdata->vm_is_started) {
    return originalMethodCurrentTimeInMillis(env, jc);
  }

  jstring offsetPropertyName = (*env)->NewStringUTF(env, "faketime.offset.seconds");
  jstring offsetPropertyDefault = (*env)->NewStringUTF(env, "0");
  jstring filePathPropertyName = (*env)->NewStringUTF(env, "faketime.file.path");
  //jstring fileCacheDurationPropertyName = (*env)->NewStringUTF(env, "faketime.file.cache_duration");
  //jstring fileCacheDurationPropertyDefault = (*env)->NewStringUTF(env, "10");

  jstring offsetValue = (*env)->CallStaticObjectMethod(env, systemClass, getPropertyMethodId, offsetPropertyName, offsetPropertyDefault);
  jstring filePath = (*env)->CallStaticObjectMethod(env, systemClass, getPropertyMethodId, filePathPropertyName, NULL);
  //jstring fileCacheDuration = (*env)->CallStaticObjectMethod(env, systemClass, getPropertyMethodId, fileCacheDurationPropertyName, fileCacheDurationPropertyDefault);
  if ((*env)->ExceptionCheck(env)) return 0;

  jlong offset;
  if (filePath != NULL) {
    // If filePath is defined, read the offset from that file (in milliseconds)
    const char* filepathStr = (*env)->GetStringUTFChars(env, filePath, NULL);
    offset = readOffsetFromFile(filepathStr);
    (*env)->ReleaseStringUTFChars(env, filePath, filepathStr);
  } else {
    // if not, use the offset from system property "faketime.offset.seconds"
    const char *offsetStr = (*env)->GetStringUTFChars(env, offsetValue, NULL);
    offset = atol(offsetStr) * 1000;
    (*env)->ReleaseStringUTFChars(env, offsetValue, offsetStr);
  }

  jlong realTime = originalMethodCurrentTimeInMillis(env, jc);
  jlong timeWithOffset = realTime + offset;

  return timeWithOffset;
}

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

void JNICALL callbackVMInit(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread) {
  gdata->vm_is_started = JNI_TRUE;
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved)
{
  static GlobalAgentData data;
  jvmtiEventCallbacks callbacks;

  jint res = (*jvm)->GetEnv(jvm, (void **) &jvmti, JVMTI_VERSION_1_0);
  if (res != JNI_OK || jvmti == NULL) {
    fprintf(stderr, "ERROR: Unable to access JVMTI Version 1");
  }

  (void)memset((void*)&data, 0, sizeof(data));
  gdata = &data;
  gdata->jvmti = jvmti;

  jvmtiCapabilities capa;
  (void)memset(&capa, 0, sizeof(jvmtiCapabilities));
  capa.can_get_owned_monitor_info = 1;
  capa.can_generate_native_method_bind_events = 1;
  check_jvmti_error(jvmti, (*jvmti)->AddCapabilities(jvmti, &capa), "Unable to get necessary JVMTI capabilities.");

  (void)memset(&callbacks, 0, sizeof(callbacks));
  callbacks.NativeMethodBind = &callbackNativeMethodBind;
  callbacks.VMInit = &callbackVMInit;
  check_jvmti_error(jvmti, (*jvmti)->SetEventCallbacks(jvmti, &callbacks, (jint)sizeof(callbacks)), "Cannot set jvmti callbacks");

  check_jvmti_error(jvmti, (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_NATIVE_METHOD_BIND, (jthread)NULL), "Cannot set event notification");
  check_jvmti_error(jvmti, (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, (jthread)NULL), "Cannot set event notification");

  check_jvmti_error(jvmti, (*jvmti)->CreateRawMonitor(jvmti, "agent data", &(gdata->lock)), "Cannot create raw monitor");

  return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm)
{
}
