#include <stdio.h>
#include <stdlib.h>
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
static void
check_jvmti_error(jvmtiEnv *jvmti, jvmtiError errnum, const char *str)
{
    if ( errnum != JVMTI_ERROR_NONE ) {
        char       *errnum_str;

        errnum_str = NULL;
        (void)(*jvmti)->GetErrorName(jvmti, errnum, &errnum_str);

        printf("ERROR: JVMTI: %d(%s): %s\n", errnum, (errnum_str==NULL?"Unknown":errnum_str), (str==NULL?"":str));
    }
}


/* Enter a critical section by doing a JVMTI Raw Monitor Enter */
static void
enter_critical_section(jvmtiEnv *jvmti)
{
    jvmtiError error;

    error = (*jvmti)->RawMonitorEnter(jvmti, gdata->lock);
    check_jvmti_error(jvmti, error, "Cannot enter with raw monitor");
}

/* Exit a critical section by doing a JVMTI Raw Monitor Exit */
static void
exit_critical_section(jvmtiEnv *jvmti)
{
    jvmtiError error;

    error = (*jvmti)->RawMonitorExit(jvmti, gdata->lock);
    check_jvmti_error(jvmti, error, "Cannot exit with raw monitor");
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


// Exception callback
 static void JNICALL callbackException(jvmtiEnv *jvmti_env, JNIEnv* env, jthread thr, jmethodID method, jlocation location, jobject exception, jmethodID catch_method, jlocation catch_location) {
enter_critical_section(jvmti); {

      jvmtiError err, err1, err2, error;
      jvmtiThreadInfo info, info1;
      jvmtiThreadGroupInfo groupInfo;
      jint num_monitors;
      jobject *arr_monitors;

      jvmtiFrameInfo frames[5];
      jint count;
      jint flag = 0;
      jint thr_st_ptr;
      jint thr_count;
      jthread *thr_ptr;
     
      jvmtiError err3;
      char *name;
      char *sig;
      char *gsig;

      err3  = (*jvmti)->GetMethodName(jvmti, method, &name, &sig, &gsig);
      if (err3  != JVMTI_ERROR_NONE) {
          printf("GetMethodName:%d\n", err);
          return;
      }

      printf("Got Exception from Method:%s%s\n", name, sig);


      err = (*jvmti)->GetThreadInfo(jvmti, thr, &info);
      if (err != JVMTI_ERROR_NONE) {
          printf("(GetThreadInfo) Error expected: %d, got: %d\n", JVMTI_ERROR_NONE, err);
	  describe(err);
                 jvmtiPhase phase;
                 jvmtiError phaseStat;
		 phaseStat = (*jvmti)->GetPhase(jvmti,&phase);
                 printf("    current phase is %d\n", phase);
          printf("\n");

      }


      if (err == JVMTI_ERROR_NONE) {
  	err1 = (*jvmti)->GetThreadGroupInfo(jvmti,info.thread_group, &groupInfo);
                 if (err1 != JVMTI_ERROR_NONE)
                 {
          		printf("(GetThreadGroupInfo) Error expected: %d, got: %d\n", JVMTI_ERROR_NONE, err);
	  		describe(err);
          		printf("\n");
                 }
             }

      if ((err == JVMTI_ERROR_NONE ) && (err1 == JVMTI_ERROR_NONE ) )
             {
                 printf("Current Thread is : %s and it belongs to Thread Group :  %s\n", info.name, groupInfo.name);
             }


     err = (*jvmti)->GetOwnedMonitorInfo(jvmti, thr, &num_monitors, &arr_monitors);
      if (err != JVMTI_ERROR_NONE) {
          printf("(GetThreadInfo) Error expected: %d, got: %d\n", JVMTI_ERROR_NONE, err);
	  describe(err);
       	  printf("\n");

      }

     printf("Number of Monitors Owned by this thread : %d\n", num_monitors);

     /* Get Thread Status */
     err = (*jvmti)->GetThreadState(jvmti, thr, &thr_st_ptr);
     if (err != JVMTI_ERROR_NONE) {
          printf("(GetThreadInfo) Error expected: %d, got: %d\n", JVMTI_ERROR_NONE, err);
          describe(err);
          printf("\n");

      }

     if (err == JVMTI_ERROR_NONE) {
	printf("Thread Status\n");
	printf("==============\n");
	if  ( thr_st_ptr & JVMTI_THREAD_STATE_ALIVE) {
	    printf("Thread %s is Alive\n", info.name);
		flag = 1;
	}

	if  ( thr_st_ptr & JVMTI_THREAD_STATE_TERMINATED) {

	   printf("Thread %s has been Terminated\n", info.name);
		flag = 1;
	}

	if ( thr_st_ptr & JVMTI_THREAD_STATE_RUNNABLE ) {

	   printf("Thread %s is Runnable\n", info.name);
		flag = 1;
	}

	if ( thr_st_ptr & JVMTI_THREAD_STATE_WAITING ) {
	   printf("Thread %s waiting\n", info.name);
		flag = 1;
	}

	if ( thr_st_ptr & JVMTI_THREAD_STATE_WAITING_INDEFINITELY ) {
	   printf("Thread %s waiting indefinitely\n", info.name);
		flag = 1;
	}

	if ( thr_st_ptr & JVMTI_THREAD_STATE_WAITING_WITH_TIMEOUT ) {    
	   printf("Thread %s waiting with Timeout\n", info.name);
		flag = 1;
	}

	if ( thr_st_ptr & JVMTI_THREAD_STATE_SLEEPING ) {
	   printf("Thread %s Sleeping \n", info.name);
		flag = 1;
	}
	
	if ( thr_st_ptr & JVMTI_THREAD_STATE_WAITING_FOR_NOTIFICATION ) {
	   printf("Thread %s Waiting for Notification \n", info.name);
		flag = 1;
	}
		    
	if ( thr_st_ptr & JVMTI_THREAD_STATE_IN_OBJECT_WAIT ) {
	   printf("Thread %s is in Object Wait \n", info.name);
		flag = 1;
	}
	if ( thr_st_ptr & JVMTI_THREAD_STATE_PARKED ) {
	   printf("Thread %s is Parked \n", info.name);
		flag = 1;
	}
	if ( thr_st_ptr & JVMTI_THREAD_STATE_BLOCKED_ON_MONITOR_ENTER ) { 
	   printf("Thread %s is blocked on monitor enter \n", info.name);
		flag = 1;
	}
	if ( thr_st_ptr & JVMTI_THREAD_STATE_SUSPENDED ) { 
	   printf("Thread %s is Suspended \n", info.name);
		flag = 1;
	}
	if ( thr_st_ptr & JVMTI_THREAD_STATE_INTERRUPTED ) {
	   printf("Thread %s is Interrupted \n", info.name);
		flag = 1;
	}
	if ( thr_st_ptr & JVMTI_THREAD_STATE_IN_NATIVE ) {

	   printf("Thread %s is in Native \n", info.name);
		flag = 1;
	}
	 if ( flag != 1 )  {
    	    printf("Illegal value  %d for Thread State\n", thr_st_ptr);
	}

	}

     /* Get All Threads */
     err = (*jvmti)->GetAllThreads(jvmti, &thr_count, &thr_ptr);
     if (err != JVMTI_ERROR_NONE) {
          printf("(GetThreadInfo) Error expected: %d, got: %d\n", JVMTI_ERROR_NONE, err);
          describe(err);
          printf("\n");

      }
      if (err == JVMTI_ERROR_NONE && thr_count >= 1) {
	int i = 0;
	printf("Thread Count: %d\n", thr_count);
	
	for ( i=0; i < thr_count; i++) {

	    /* Make sure the stack variables are garbage free */
    	    (void)memset(&info1,0, sizeof(info1));

	    err1 = (*jvmti)->GetThreadInfo(jvmti, thr_ptr[i], &info1);
            if (err1 != JVMTI_ERROR_NONE) {
          	printf("(GetThreadInfo) Error expected: %d, got: %d\n", JVMTI_ERROR_NONE, err1);
          	describe(err1);
          	printf("\n");
            }

            printf("Running Thread#%d: %s, Priority: %d, context class loader:%s\n", i+1,info1.name, info1.priority,(info1.context_class_loader == NULL ? ": NULL" : "Not Null"));


	    /* Every string allocated by JVMTI needs to be freed */

	    err2 = (*jvmti)->Deallocate(jvmti, (unsigned)info1.name);
            if (err2 != JVMTI_ERROR_NONE) {
                printf("(GetThreadInfo) Error expected: %d, got: %d\n", JVMTI_ERROR_NONE, err2);
                describe(err2);
                printf("\n");
            }
	


	}
      }


     /* Get Stack Trace */
     err = (*jvmti)->GetStackTrace(jvmti, thr, 0, 5, &frames, &count);
     if (err != JVMTI_ERROR_NONE) {
          printf("(GetThreadInfo) Error expected: %d, got: %d\n", JVMTI_ERROR_NONE, err);
          describe(err);
          printf("\n");

      }
	printf("Number of records filled: %d\n", count);
     if (err == JVMTI_ERROR_NONE && count >=1) {
	
	char *methodName;
        methodName = "yet_to_call()";
   	char *declaringClassName;
        jclass declaring_class;

	int i=0;

	printf("Exception Stack Trace\n");
	printf("=====================\n");
     	printf("Stack Trace Depth: %d\n", count); 

        for ( i=0; i < count; i++) {
		err = (*jvmti)->GetMethodName(jvmti, frames[i].method, &methodName, NULL, NULL);
  	 	if (err == JVMTI_ERROR_NONE) {

               		 err = (*jvmti)->GetMethodDeclaringClass(jvmti, frames[i].method, &declaring_class);
                         err = (*jvmti)->GetClassSignature(jvmti, declaring_class, &declaringClassName, NULL);
                         if (err == JVMTI_ERROR_NONE) {
	                         printf("at method %s() in class %s\n", methodName, declaringClassName);
                         }
                }

	}
        printf("\n");

        err = (*jvmti)->Deallocate(jvmti, (unsigned)methodName);
        err = (*jvmti)->Deallocate(jvmti, (unsigned)declaringClassName);
    }
 } exit_critical_section(jvmti);

 }
// VM Death callback
     static void JNICALL callbackVMDeath(jvmtiEnv *jvmti_env, JNIEnv* jni_env)
     {
	 enter_critical_section(jvmti); {

		printf("Got VM Death event\n");

	} exit_critical_section(jvmti);

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

    /* The thread might not have a name, be careful here. */
    if ( info.name != NULL ) {
        int len;

        /* Copy the thread name into tname if it will fit */
        len = (int)strlen(info.name);
        if ( len < maxlen ) {
            (void)strcpy(tname, info.name);
        }

        /* Every string allocated by JVMTI needs to be freed */
	    error = (*jvmti)->Deallocate(jvmti, (unsigned)info.name);
            if (error != JVMTI_ERROR_NONE) {
                printf("(get_thread_name) Error expected: %d, got: %d\n", JVMTI_ERROR_NONE, error);
                describe(error);
                printf("\n");
            }

    }
}


// VM init callback
     static void JNICALL callbackVMInit(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread)
     {
	enter_critical_section(jvmti); {

	char  tname[MAX_THREAD_NAME_LENGTH];
        static jvmtiEvent events[] = { JVMTI_EVENT_THREAD_START, JVMTI_EVENT_THREAD_END }; 
        int        i;
	jvmtiFrameInfo frames[5];
	jvmtiError err, err1;
        jvmtiError error;
	jint count;

        /* The VM has started. */
        printf("Got VM init event\n");
        get_thread_name(jvmti_env , thread, tname, sizeof(tname));
        printf("callbackVMInit:  %s thread\n", tname);


            error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_EXCEPTION, (jthread)NULL);
            check_jvmti_error(jvmti_env, error, "Cannot set event notification");



     } exit_critical_section(jvmti);

     }


/* JVMTI callback function. */
static jvmtiIterationControl JNICALL
reference_object(jvmtiObjectReferenceKind reference_kind,
                jlong class_tag, jlong size, jlong* tag_ptr,
                jlong referrer_tag, jint referrer_index, void *user_data)
{

	combined_size = combined_size + size;

	switch (reference_kind) {

		case JVMTI_REFERENCE_CLASS:
			num_class_refs = num_class_refs + 1;
			break;
                case JVMTI_REFERENCE_FIELD:
			num_field_refs = num_field_refs + 1;
                        break;
                case JVMTI_REFERENCE_ARRAY_ELEMENT:
			num_array_refs = num_array_refs + 1;
                        break;
                case JVMTI_REFERENCE_CLASS_LOADER:
			num_classloader_refs = num_classloader_refs + 1;
                        break;
                case JVMTI_REFERENCE_SIGNERS:
			num_signer_refs = num_signer_refs + 1;
                        break;
                case JVMTI_REFERENCE_PROTECTION_DOMAIN:
			num_protection_domain_refs = num_protection_domain_refs + 1;
                        break;
                case JVMTI_REFERENCE_INTERFACE:
			num_interface_refs = num_interface_refs + 1;
                        break;
                case JVMTI_REFERENCE_STATIC_FIELD:
			num_static_field_refs = num_static_field_refs + 1;
                        break;
                case JVMTI_REFERENCE_CONSTANT_POOL:
			num_constant_pool_refs = num_constant_pool_refs + 1;
                        break;
                default:
                        break;
	}


    return JVMTI_ITERATION_CONTINUE;
}


static void JNICALL callbackVMObjectAlloc(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jobject object, jclass object_klass, jlong size) {


	char *methodName;
	char *className;
	char *declaringClassName;
	jclass declaring_class;
	jvmtiError err;


	if (size > 50) {

		err = (*jvmti)->GetClassSignature(jvmti, object_klass, &className, NULL);

		if (className != NULL) {
			printf("\ntype %s object allocated with size %d\n", className, (jint)size);
		}

		//print stack trace
		jvmtiFrameInfo frames[5];
		jint count;
		int i;

		err = (*jvmti)->GetStackTrace(jvmti, NULL, 0, 5, &frames, &count);
		if (err == JVMTI_ERROR_NONE && count >= 1) {

			for (i = 0; i < count; i++) {
				err = (*jvmti)->GetMethodName(jvmti, frames[i].method, &methodName, NULL, NULL);
				if (err == JVMTI_ERROR_NONE) {

					err = (*jvmti)->GetMethodDeclaringClass(jvmti, frames[i].method, &declaring_class);
					err = (*jvmti)->GetClassSignature(jvmti, declaring_class, &declaringClassName, NULL); 
					if (err == JVMTI_ERROR_NONE) {
						printf("at method %s in class %s\n", methodName, declaringClassName);
					}
				}
			}
		}

		//reset counters
		combined_size  = 0;
		num_class_refs = 0;
		num_field_refs = 0;
		num_array_refs = 0;
		num_classloader_refs = 0;
		num_signer_refs = 0;
		num_protection_domain_refs = 0;
		num_interface_refs = 0;
		num_static_field_refs = 0;
		num_constant_pool_refs = 0;

	        err = (*jvmti)->IterateOverObjectsReachableFromObject(jvmti, object, &reference_object, NULL);
		if ( err != JVMTI_ERROR_NONE ) {
		        printf("Cannot iterate over reachable objects\n");
    		}

		printf("\nThis object has references to objects of combined size %d\n", (jint)combined_size);
		printf("This includes %d classes, %d fields, %d arrays, %d classloaders, %d signers arrays,\n", num_class_refs, num_field_refs, num_array_refs, num_classloader_refs, num_signer_refs);
		printf("%d protection domains, %d interfaces, %d static fields, and %d constant pools.\n\n", num_protection_domain_refs, num_interface_refs, num_static_field_refs, num_constant_pool_refs);

		err = (*jvmti)->Deallocate(jvmti, (unsigned)className);
		err = (*jvmti)->Deallocate(jvmti, (unsigned)methodName);
		err = (*jvmti)->Deallocate(jvmti, (unsigned)declaringClassName);
	}
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
        printf("ERROR: Unable to access JVMTI Version 1 (0x%x),"
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
    capa.can_generate_exception_events = 1;
    capa.can_generate_vm_object_alloc_events = 1;
    capa.can_tag_objects = 1;	

    error = (*jvmti)->AddCapabilities(jvmti, &capa);
    check_jvmti_error(jvmti, error, "Unable to get necessary JVMTI capabilities.");


    (void)memset(&callbacks, 0, sizeof(callbacks));
    callbacks.VMInit = &callbackVMInit; /* JVMTI_EVENT_VM_INIT */
    callbacks.VMDeath = &callbackVMDeath; /* JVMTI_EVENT_VM_DEATH */
    callbacks.Exception = &callbackException;/* JVMTI_EVENT_EXCEPTION */
    callbacks.VMObjectAlloc = &callbackVMObjectAlloc;/* JVMTI_EVENT_VM_OBJECT_ALLOC */


    error = (*jvmti)->SetEventCallbacks(jvmti, &callbacks, (jint)sizeof(callbacks));
    check_jvmti_error(jvmti, error, "Cannot set jvmti callbacks");

    /* At first the only initial events we are interested in are VM
     *   initialization, VM death, and Class File Loads.
     *   Once the VM is initialized we will request more events.
    */
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE,
                          JVMTI_EVENT_VM_INIT, (jthread)NULL);
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE,
                          JVMTI_EVENT_VM_DEATH, (jthread)NULL);
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_VM_OBJECT_ALLOC, (jthread)NULL);
    check_jvmti_error(jvmti, error, "Cannot set event notification");


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

