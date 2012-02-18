#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "jvmti.h"

/* ------------------------------------------------------------------- */
/* Some constant maximum sizes */

#define MAX_TOKEN_LENGTH        16
#define MAX_THREAD_NAME_LENGTH  512
#define MAX_METHOD_NAME_LENGTH  1024

static jvmtiEnv *jvmti = NULL;
static jvmtiCapabilities capa;

/* Global agent data structure */

typedef struct {
  /* JVMTI Environment */
  jvmtiEnv      *jvmti;
  jboolean       vm_is_started;
  /* Data access Lock */
  jrawMonitorID  lock;
} GlobalAgentData;

static GlobalAgentData *gdata;

typedef jlong (JNICALL *call__jlong) (JNIEnv*, jclass);
call__jlong originalMethodCurrentTimeInMillis = NULL;

static jlong combined_size;
static int num_class_refs;
static int num_field_refs;
static int num_array_refs;
static int num_classloader_refs;
static int num_signer_refs;
static int num_protection_domain_refs;
static int num_interface_refs; 
static int num_static_field_refs;
static int num_constant_pool_refs;


/* Every JVMTI interface returns an error code, which should be checked
 *   to avoid any cascading errors down the line.
 *   The interface GetErrorName() returns the actual enumeration constant
 *   name, making the error messages much easier to understand.
 */
static void check_jvmti_error(jvmtiEnv *jvmti, jvmtiError errnum, const char *str)
{
  if ( errnum != JVMTI_ERROR_NONE ) {
    char       *errnum_str;

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

void noprintf(char *format, ...) {
  return;
  va_list argp;
  va_start(argp, format);
  vprintf(format, argp);
  va_end(argp);
}

void describe(jvmtiError err) {
  jvmtiError err0;
  char *descr;
  err0 = (*jvmti)->GetErrorName(jvmti, err, &descr);
  if (err0 == JVMTI_ERROR_NONE) {
    printf(descr);
  } else {
    printf("error [%d]", err);
  }
}

/* Get a name for a jthread */
static void get_thread_name(jvmtiEnv *jvmti, jthread thread, char *tname, int maxlen)
{
  jvmtiThreadInfo info;
  jvmtiError      error;

  /* Make sure the stack variables are garbage free */
  (void)memset(&info,0, sizeof(info));

  /* Assume the name is unknown for now */
  (void)strcpy(tname, "Unknown");

  /* Get the thread information, which includes the name */
  error = (*jvmti)->GetThreadInfo(jvmti, thread, &info);
  check_jvmti_error(jvmti, error, "Cannot get thread info");

  noprintf("Hello!\n");
  /* The thread might not have a name, be careful here. */
  if ( info.name != NULL ) {
    int len;

    /* Copy the thread name into tname if it will fit */
    len = (int)strlen(info.name);
    if ( len < maxlen ) {
      (void)strcpy(tname, info.name);
    }

    /* Every string allocated by JVMTI needs to be freed */
    /*
       jni_env->DeleteWeakGlobalRef(info.thread_group);
       jni_env->DeleteWeakGlobalRef(info.context_class_loader);
       jvmti->Deallocate((unsigned char *)(info.name));
       */

    error = (*jvmti)->Deallocate(jvmti, (unsigned char *)info.name);
    if (error != JVMTI_ERROR_NONE) {
      noprintf("(get_thread_name) Error expected: %d, got: %d\n", JVMTI_ERROR_NONE, error);
      describe(error);
      noprintf("\n");
    }

  }
}


// VM init callback
static void JNICALL callbackVMInit(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread)
{
  enter_critical_section(jvmti); {

    char  tname[MAX_THREAD_NAME_LENGTH];
    jvmtiError error;

    /* The VM has started. */
    noprintf("Got VM init event.\n");
    get_thread_name(jvmti_env , thread, tname, sizeof(tname));
    noprintf("callbackVMInit:  %s thread\n", tname);

    check_jvmti_error(jvmti_env, (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_EXCEPTION, (jthread)NULL), "Cannot set");

  } exit_critical_section(jvmti);

}

JNIEXPORT jlong JNICALL newCurrentTimeInMillis(JNIEnv* env, jclass jc) {
  jlong realTime = originalMethodCurrentTimeInMillis(env, jc);
  return realTime + 400000000000;
}

void JNICALL callbackNativeMethodBind(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method, void* address, void** new_address_ptr) {
  char *methodName = NULL;
  char *declaringClassName = NULL;
  jclass declaring_class;
  jvmtiError err;

  err = (*jvmti)->GetMethodName(jvmti, method, &methodName, NULL, NULL);
  if (err == JVMTI_ERROR_NONE && strcmp("currentTimeMillis", methodName) == 0) {
    err = (*jvmti)->GetMethodDeclaringClass(jvmti, method, &declaring_class);
    err = (*jvmti)->GetClassSignature(jvmti, declaring_class, &declaringClassName, NULL);
    if (err == JVMTI_ERROR_NONE) {
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

void JNICALL callbackClassFileLoadHook(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jclass class_being_redefined, jobject loader,
  const char* name, jobject protection_domain, jint class_data_len, const unsigned char* class_data, jint* new_class_data_len,
  unsigned char** new_class_data) {
  printf("Load!\n");
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved)
{
  static GlobalAgentData data;
  jvmtiError error;
  jint res;
  jvmtiEventCallbacks callbacks;


  /* Setup initial global agent data area
   *   Use of static/extern data should be handled carefully here.
   *   We need to make sure that we are able to cleanup after ourselves
   *     so anything allocated in this library needs to be freed in
   *     the Agent_OnUnload() function.
   */
  (void)memset((void*)&data, 0, sizeof(data));
  gdata = &data;

  /*  We need to first get the jvmtiEnv* or JVMTI environment */

  res = (*jvm)->GetEnv(jvm, (void **) &jvmti, JVMTI_VERSION_1_0);

  if (res != JNI_OK || jvmti == NULL) {
    /* This means that the VM was unable to obtain this version of the
     *   JVMTI interface, this is a fatal error.
     */
    noprintf("ERROR: Unable to access JVMTI Version 1 (0x%x),"
        " is your J2SE a 1.5 or newer version?"
        " JNIEnv's GetEnv() returned %d\n",
        JVMTI_VERSION_1, res);

  }

  /* Here we save the jvmtiEnv* for Agent_OnUnload(). */
  gdata->jvmti = jvmti;


  (void)memset(&capa, 0, sizeof(jvmtiCapabilities));
  capa.can_signal_thread = 1;
  capa.can_get_owned_monitor_info = 1;
  capa.can_generate_method_entry_events = 1;
  capa.can_generate_native_method_bind_events = 1;
  capa.can_generate_exception_events = 1;
  capa.can_generate_vm_object_alloc_events = 1;
  capa.can_generate_all_class_hook_events = 1;
  capa.can_tag_objects = 1;	

  error = (*jvmti)->AddCapabilities(jvmti, &capa);
  check_jvmti_error(jvmti, error, "Unable to get necessary JVMTI capabilities.");


  (void)memset(&callbacks, 0, sizeof(callbacks));
  callbacks.VMInit = &callbackVMInit; /* JVMTI_EVENT_VM_INIT */
  // callbacks.VMDeath = &callbackVMDeath; /* JVMTI_EVENT_VM_DEATH */
  // callbacks.Exception = &callbackException;/* JVMTI_EVENT_EXCEPTION */
  // callbacks.VMObjectAlloc = &callbackVMObjectAlloc;/* JVMTI_EVENT_VM_OBJECT_ALLOC */
  callbacks.NativeMethodBind = &callbackNativeMethodBind;

  error = (*jvmti)->SetEventCallbacks(jvmti, &callbacks, (jint)sizeof(callbacks));
  check_jvmti_error(jvmti, error, "Cannot set jvmti callbacks");

  /* At first the only initial events we are interested in are VM
   *   initialization, VM death, and Class File Loads.
   *   Once the VM is initialized we will request more events.
   */
  check_jvmti_error(jvmti, (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, (jthread)NULL), "Hmm");
  check_jvmti_error(jvmti, (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_VM_DEATH, (jthread)NULL), "Nooo");
  check_jvmti_error(jvmti, (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_VM_OBJECT_ALLOC, (jthread)NULL), "Ouch!");
  check_jvmti_error(jvmti, (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, (jthread)NULL), "Cannot set event notification");
  check_jvmti_error(jvmti, (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_NATIVE_METHOD_BIND, (jthread)NULL), "Cannot set event notification");

  /* Here we create a raw monitor for our use in this agent to
   *   protect critical sections of code.
   */
  error = (*jvmti)->CreateRawMonitor(jvmti, "agent data", &(gdata->lock));
  check_jvmti_error(jvmti, error, "Cannot create raw monitor");

  /* We return JNI_OK to signify success */
  return JNI_OK;


}


/* Agent_OnUnload: This is called immediately before the shared library is
 *   unloaded. This is the last code executed.
 */
  JNIEXPORT void JNICALL
Agent_OnUnload(JavaVM *vm)
{
  /* Make sure all malloc/calloc/strdup space is freed */

}
