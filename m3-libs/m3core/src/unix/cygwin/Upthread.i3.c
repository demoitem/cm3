/* $Id$ */

#include <pthread.h>
#include <stdio.h>

#define SIZE(a) (((sizeof(a) + sizeof(int) - 1)) / sizeof(int))

int main()
{
    unsigned i;
    const static struct
    {
        const char* Format;
        unsigned Value;
    } Data[] =
{
"(* Copyright (C) 1994, Digital Equipment Corporation.         *)", 0,
"(* All rights reserved.                                       *)", 0,
"(* See the file COPYRIGHT for a full description.             *)", 0,
"", 0,
"(* $Id$ *)", 0,
"", 0,
"(* This file was generated from " __FILE__ ". Do not edit it. *)", 0,
"", 0,
"INTERFACE Upthread;", 0,
"", 0,
"FROM Ctypes IMPORT int, unsigned_int, void_star;", 0,
"FROM Utypes IMPORT size_t;", 0,
"FROM Usignal IMPORT sigset_t;", 0,
"FROM Utime IMPORT struct_timespec;", 0,
"", 0,
"(* <bits/local_lim.h> *)", 0,
"", 0,
"CONST", 0,
"  PTHREAD_KEYS_MAX = 16_%x;", PTHREAD_KEYS_MAX,
"", 0,
"(* <bits/pthreadtypes.h> *)", 0,
"", 0,
"CONST", 0,
"  SIZEOF_PTHREAD_MUTEX_T  = 16_%x;", SIZE(pthread_mutex_t),
"  SIZEOF_PTHREAD_COND_T   = 16_%x;", SIZE(pthread_mutexattr_t),
"  SIZEOF_PTHREAD_RWLOCK_T = 16_%x;", SIZE(pthread_rwlock_t),
"  SIZEOF_PTHREAD_ONCE_T   = 16_%x;", SIZE(pthread_once_t),
"", 0,
"TYPE", 0,
"  pthread_t = void_star;", 0,
"  pthread_attr_t = RECORD", 0,
"    opaque: ARRAY[0..16_%x - 1] OF unsigned_int;", SIZE(pthread_attr_t),
"  END;", 0,
"", 0,
"TYPE", 0,
"  pthread_mutex_t = RECORD", 0,
"    opaque: ARRAY[0..SIZEOF_PTHREAD_MUTEX_T - 1] OF unsigned_int;", 0,
"  END;", 0,
"  pthread_mutexattr_t = RECORD", 0,
"    opaque: ARRAY[0..16_%x - 1] OF unsigned_int;", SIZE(pthread_mutexattr_t),
"  END;", 0,
"", 0,
"TYPE", 0,
"  pthread_cond_t = RECORD", 0,
"    opaque: ARRAY [0..SIZEOF_PTHREAD_COND_T - 1] OF unsigned_int;", 0,
"  END;", 0,
"  pthread_condattr_t = RECORD", 0,
"    opaque: ARRAY [0..16_%x - 1] OF unsigned_int;", SIZE(pthread_condattr_t),
"  END;", 0,
"", 0,
"(* Keys for thread-specific data *)", 0,
"TYPE", 0,
"  pthread_key_t = unsigned_int;", 0,
"", 0,
"(* Once-only execution *)", 0,
"TYPE", 0,
"  pthread_once_t = RECORD", 0,
"    opaque: ARRAY [0..SIZEOF_PTHREAD_ONCE_T - 1] OF unsigned_int;", 0,
"  END;", 0,
"", 0,
"TYPE", 0,
"  pthread_rwlock_t = RECORD", 0,
"    opaque: ARRAY [0..SIZEOF_PTHREAD_RWLOCK_T - 1] OF unsigned_int;", 0,
"  END;", 0,
"  pthread_rwlockattr_t = RECORD", 0,
"    opaque: ARRAY [0..16_%x - 1] OF unsigned_int;", SIZE(pthread_rwlockattr_t),
"  END;", 0,
"", 0,
"(* <bits/sched.h> *)", 0,
"TYPE", 0,
"  struct_sched_param = RECORD", 0,
"    sched_priority: int;", 0,
"  END;", 0,
"", 0,
"(* <pthread.h> *)", 0,
"", 0,
"(* Mutex initializers.  *)", 0,
"CONST", 0,
"  PTHREAD_MUTEX_INITIALIZER =", 0,
"    pthread_mutex_t { ARRAY [0..SIZEOF_PTHREAD_MUTEX_T - 1] OF unsigned_int {16_%x, .. } };", (unsigned) (size_t) PTHREAD_MUTEX_INITIALIZER,
"", 0,
"(* Read-write lock initializers.  *)", 0,
"CONST", 0,
"  PTHREAD_RWLOCK_INITIALIZER =", 0,
"    pthread_rwlock_t { ARRAY [0..SIZEOF_PTHREAD_RWLOCK_T - 1] OF unsigned_int {16_%x, .. } };", (unsigned) (size_t) PTHREAD_RWLOCK_INITIALIZER,
"", 0,
"(* Conditional variable handling.  *)", 0,
"CONST", 0,
"  PTHREAD_COND_INITIALIZER =", 0,
"    pthread_cond_t { ARRAY [0..SIZEOF_PTHREAD_COND_T - 1] OF unsigned_int {16_%x, .. } };", (unsigned) (size_t) PTHREAD_COND_INITIALIZER,
"", 0,
"(* Single execution handling.  *)", 0,
"CONST", 0,
"  PTHREAD_ONCE_INIT = ", 0,
"    pthread_once_t { ARRAY [0..SIZEOF_PTHREAD_ONCE_T - 1] OF unsigned_int {16_%x, .. } };", (unsigned) (size_t) PTHREAD_MUTEX_INITIALIZER,
"", 0,
"TYPE start_routine_t = PROCEDURE(arg: ADDRESS): ADDRESS;", 0,
"<*EXTERNAL pthread_create*>", 0,
"PROCEDURE create (VAR pthread: pthread_t;", 0,
"                  READONLY attr: pthread_attr_t;", 0,
"                  start_routine: start_routine_t;", 0,
"                  arg: ADDRESS): int;", 0,
"", 0,
"<*EXTERNAL pthread_exit*>", 0,
"PROCEDURE exit (value_ptr: ADDRESS);", 0,
"", 0,
"<*EXTERNAL pthread_join*>", 0,
"PROCEDURE join (thread: pthread_t; VAR value_ptr: ADDRESS): int;", 0,
"", 0,
"<*EXTERNAL pthread_detach*>", 0,
"PROCEDURE detach (thread: pthread_t): int;", 0,
"", 0,
"(* Obtain the identifier of the current thread.  *)", 0,
"<*EXTERNAL pthread_self*>", 0,
"PROCEDURE self (): pthread_t;", 0,
"", 0,
"(* Compare two thread identifiers.  *)", 0,
"<*EXTERNAL pthread_equal*>", 0,
"PROCEDURE equal (t1, t2: pthread_t): int;", 0,
"", 0,
"(* Thread attribute handling.  *)", 0,
"<*EXTERNAL pthread_attr_init*>", 0,
"PROCEDURE attr_init (VAR attr: pthread_attr_t): int;", 0,
"<*EXTERNAL pthread_attr_destroy*>", 0,
"PROCEDURE attr_destroy (VAR attr: pthread_attr_t): int;", 0,
"<*EXTERNAL pthread_attr_getdetachstate*>", 0,
"PROCEDURE attr_getdetachstate (READONLY attr: pthread_attr_t;", 0,
"                               VAR detachstate: int): int;", 0,
"<*EXTERNAL pthread_attr_setdetachstate*>", 0,
"PROCEDURE attr_setdetachstate (VAR attr: pthread_attr_t;", 0,
"                               detachstate: int): int;", 0,
"<*EXTERNAL pthread_attr_getguardsize*>", 0,
"PROCEDURE attr_getguardsize (READONLY attr: pthread_attr_t;", 0,
"                             VAR guardsize: size_t): int;", 0,
"<*EXTERNAL pthread_attr_setguardsize*>", 0,
"PROCEDURE attr_setguardsize (VAR attr: pthread_attr_t; guardsize: size_t): int;", 0,
"<*EXTERNAL pthread_attr_getschedparam*>", 0,
"PROCEDURE attr_getschedparam (READONLY attr: pthread_attr_t;", 0,
"                              VAR param: struct_sched_param): int;", 0,
"<*EXTERNAL pthread_attr_setschedparam*>", 0,
"PROCEDURE attr_setschedparam (VAR attr: pthread_attr_t;", 0,
"                              READONLY param: struct_sched_param): int;", 0,
"<*EXTERNAL pthread_attr_getschedpolicy*>", 0,
"PROCEDURE attr_getschedpolicy (READONLY attr: pthread_attr_t;", 0,
"                               VAR policy: int): int;", 0,
"<*EXTERNAL pthread_attr_setschedpolicy*>", 0,
"PROCEDURE attr_setschedpolicy (VAR attr: pthread_attr_t; policy: int): int;", 0,
"<*EXTERNAL pthread_attr_getinheritsched*>", 0,
"PROCEDURE attr_getinheritsched (READONLY attr: pthread_attr_t;", 0,
"                                VAR inheritsched: int): int;", 0,
"<*EXTERNAL pthread_attr_setinheritsched*>", 0,
"PROCEDURE attr_setinheritsched (VAR attr: pthread_attr_t;", 0,
"                                inheritsched: int): int;", 0,
"<*EXTERNAL pthread_attr_getscope*>", 0,
"PROCEDURE attr_getscope (READONLY attr: pthread_attr_t;", 0,
"                         VAR contentionscope: int): int;", 0,
"<*EXTERNAL pthread_attr_setscope*>", 0,
"PROCEDURE attr_setscope (VAR attr: pthread_attr_t; contentionscope:int): int;", 0,
"<*EXTERNAL pthread_attr_getstackaddr*>", 0,
"PROCEDURE attr_getstackaddr (READONLY attr: pthread_attr_t;", 0,
"                             VAR stackaddr: ADDRESS): int;", 0,
"<*EXTERNAL pthread_attr_setstackaddr*>", 0,
"PROCEDURE attr_setstackaddr (VAR attr: pthread_attr_t;", 0,
"                             stackaddr: ADDRESS): int;", 0,
"<*EXTERNAL pthread_attr_getstacksize*>", 0,
"PROCEDURE attr_getstacksize (READONLY attr: pthread_attr_t;", 0,
"                             VAR stacksize: size_t): int;", 0,
"<*EXTERNAL pthread_attr_setstacksize*>", 0,
"PROCEDURE attr_setstacksize (VAR attr: pthread_attr_t; stacksize: size_t): int;", 0,
"<*EXTERNAL pthread_attr_getstack*>", 0,
"PROCEDURE attr_getstack (READONLY attr: pthread_attr_t;", 0,
"                         VAR stackaddr: ADDRESS; VAR stacksize: size_t): int;", 0,
"<*EXTERNAL pthread_attr_setstack*>", 0,
"PROCEDURE attr_setstack (VAR attr: pthread_attr_t;", 0,
"                         stackaddr: ADDRESS; stacksize: size_t): int;", 0,
"", 0,
"(* Functions for scheduling control.  *)", 0,
"<*EXTERNAL pthread_setschedparam*>", 0,
"PROCEDURE setschedparam (thread: pthread_t; policy: int;", 0,
"                         READONLY param: struct_sched_param): int;", 0,
"<*EXTERNAL pthread_getschedparam*>", 0,
"PROCEDURE getschedparam (thread: pthread_t;", 0,
"                         VAR policy: int;", 0,
"                         VAR param: struct_sched_param): int;", 0,
"<*EXTERNAL pthread_setschedprio*>", 0,
"PROCEDURE setschedprio (thread: pthread_t; prio: int): int;", 0,
"", 0,
"<*EXTERNAL pthread_getconcurrency*>", 0,
"PROCEDURE getconcurrency (): int;", 0,
"<*EXTERNAL pthread_setconcurrency*>", 0,
"PROCEDURE setconcurrency (concurrency: int): int;", 0,
"", 0,
"<*EXTERNAL pthread_yield*>", 0,
"PROCEDURE yield (): int;", 0,
"", 0,
"(* Functions for handling initialization.  *)", 0,
"TYPE init_routine_t = PROCEDURE();", 0,
"<*EXTERNAL pthread_once*>", 0,
"PROCEDURE once (VAR once_control: pthread_once_t;", 0,
"                init_routine: init_routine_t): int;", 0,
"", 0,
"(* Functions for handling cancellation. *)", 0,
"<*EXTERNAL pthread_setcancelstate*>", 0,
"PROCEDURE setcancelstate (state: int; VAR oldstate: int): int;", 0,
"<*EXTERNAL pthread_setcanceltype*>", 0,
"PROCEDURE setcanceltype (type: int; VAR oldtype: int): int;", 0,
"<*EXTERNAL pthread_cancel*>", 0,
"PROCEDURE cancel (thread: pthread_t): int;", 0,
"<*EXTERNAL pthread_testcancel*>", 0,
"PROCEDURE testcancel ();", 0,
"", 0,
"(* Mutex handling.  *)", 0,
"<*EXTERNAL pthread_mutex_init*>", 0,
"PROCEDURE mutex_init (VAR mutex: pthread_mutex_t;", 0,
"                      attr: UNTRACED REF pthread_mutexattr_t): int;", 0,
"<*EXTERNAL pthread_mutex_destroy*>", 0,
"PROCEDURE mutex_destroy (VAR mutex: pthread_mutex_t): int;", 0,
"<*EXTERNAL pthread_mutex_trylock*>", 0,
"PROCEDURE mutex_trylock (VAR mutex: pthread_mutex_t): int;", 0,
"<*EXTERNAL pthread_mutex_lock*>", 0,
"PROCEDURE mutex_lock (VAR mutex: pthread_mutex_t): int;", 0,
"<*EXTERNAL pthread_mutex_unlock*>", 0,
"PROCEDURE mutex_unlock (VAR mutex: pthread_mutex_t): int;", 0,
"", 0,
"(* Functions for handling mutex attributes.  *)", 0,
"<*EXTERNAL pthread_mutexattr_init*>", 0,
"PROCEDURE mutexattr_init (VAR attr: pthread_mutexattr_t): int;", 0,
"<*EXTERNAL pthread_mutexattr_destroy*>", 0,
"PROCEDURE mutexattr_destroy (VAR attr: pthread_mutexattr_t): int;", 0,
"<*EXTERNAL pthread_mutexattr_getpshared*>", 0,
"PROCEDURE mutexattr_getpshared (READONLY attr: pthread_mutexattr_t;", 0,
"                                VAR pshared: int): int;", 0,
"<*EXTERNAL pthread_mutexattr_setpshared*>", 0,
"PROCEDURE mutexattr_setpshared (VAR attr: pthread_mutexattr_t;", 0,
"                                pshared: int): int;", 0,
"<*EXTERNAL pthread_mutexattr_gettype*>", 0,
"PROCEDURE mutexattr_gettype (READONLY attr: pthread_mutexattr_t;", 0,
"                             VAR type: int): int;", 0,
"<*EXTERNAL pthread_mutexattr_settype*>", 0,
"PROCEDURE mutexattr_settype (VAR attr: pthread_mutexattr_t; type: int): int;", 0,
"<*EXTERNAL pthread_mutex_getprioceiling*>", 0,
"PROCEDURE mutex_getprioceiling(READONLY mutex: pthread_mutex_t;", 0,
"                               VAR prioceiling: int): int;", 0,
"<*EXTERNAL pthread_mutexattr_setprioceiling*>", 0,
"PROCEDURE mutexattr_setprioceiling (VAR attr: pthread_mutexattr_t;", 0,
"                                    prioceiling: int): int;", 0,
"<*EXTERNAL pthread_mutexattr_getprotocol*>", 0,
"PROCEDURE mutexattr_getprotocol (READONLY attr: pthread_mutexattr_t;", 0,
"                                 VAR protocol: int): int;", 0,
"<*EXTERNAL pthread_mutexattr_setprotocol*>", 0,
"PROCEDURE mutexattr_setprotocol (VAR attr: pthread_mutexattr_t;", 0,
"                                 protocol: int): int;", 0,
"", 0,
"(* Functions for handling read-write locks.  *)", 0,
"<*EXTERNAL pthread_rwlock_init*>", 0,
"PROCEDURE rwlock_init (VAR rwlock: pthread_rwlock_t;", 0,
"                       READONLY attr: pthread_rwlockattr_t): int;", 0,
"<*EXTERNAL pthread_rwlock_destroy*>", 0,
"PROCEDURE rwlock_destroy (VAR rwlock: pthread_rwlock_t): int;", 0,
"<*EXTERNAL pthread_rwlock_rdlock*>", 0,
"PROCEDURE rwlock_rdlock (VAR rwlock: pthread_rwlock_t): int;", 0,
"<*EXTERNAL pthread_rwlock_tryrdlock*>", 0,
"PROCEDURE wrlock_tryrdlock (VAR rwlock: pthread_rwlock_t): int;", 0,
"<*EXTERNAL pthread_rwlock_wrlock*>", 0,
"PROCEDURE rwlock_wrlock (VAR rwlock: pthread_rwlock_t): int;", 0,
"<*EXTERNAL pthread_rwlock_trywrlock*>", 0,
"PROCEDURE rwlock_trywrlock (VAR rwlock: pthread_rwlock_t): int;", 0,
"<*EXTERNAL pthread_rwlock_unlock*>", 0,
"PROCEDURE rwlock_unlock (VAR rwlock: pthread_rwlock_t): int;", 0,
"", 0,
"(* Functions for handling read-write lock attributes.  *)", 0,
"<*EXTERNAL pthread_rwlockattr_init*>", 0,
"PROCEDURE rwlockattr_init (VAR attr: pthread_rwlockattr_t): int;", 0,
"<*EXTERNAL pthread_rwlockattr_destroy*>", 0,
"PROCEDURE rwlockattr_destroy (VAR attr: pthread_rwlockattr_t): int;", 0,
"<*EXTERNAL pthread_rwlockattr_getpshared*>", 0,
"PROCEDURE rwlockattr_getpshared (READONLY attr: pthread_rwlockattr_t;", 0,
"                                 VAR pshared: int): int;", 0,
"<*EXTERNAL pthread_rwlockattr_setpshared*>", 0,
"PROCEDURE rwlockattr_setpshared (VAR attr: pthread_rwlockattr_t;", 0,
"                                 pshared: int): int;", 0,
"", 0,
"(* Functions for handling conditional variables.  *)", 0,
"<*EXTERNAL pthread_cond_init*>", 0,
"PROCEDURE cond_init (VAR cond: pthread_cond_t;", 0,
"                     attr: UNTRACED REF pthread_condattr_t): int;", 0,
"<*EXTERNAL pthread_cond_destroy*>", 0,
"PROCEDURE cond_destroy (VAR cond: pthread_cond_t): int;", 0,
"<*EXTERNAL pthread_cond_signal*>", 0,
"PROCEDURE cond_signal (VAR cond: pthread_cond_t): int;", 0,
"<*EXTERNAL pthread_cond_broadcast*>", 0,
"PROCEDURE cond_broadcast (VAR cond: pthread_cond_t): int;", 0,
"<*EXTERNAL pthread_cond_wait*>", 0,
"PROCEDURE cond_wait (VAR cond: pthread_cond_t;", 0,
"                     VAR mutex: pthread_mutex_t): int;", 0,
"<*EXTERNAL pthread_cond_timedwait*>", 0,
"PROCEDURE cond_timedwait (VAR cond: pthread_cond_t;", 0,
"                          VAR mutex: pthread_mutex_t;", 0,
"                          READONLY abstime: struct_timespec): int;", 0,
"", 0,
"(* Functions for handling condition variable attributes.  *)", 0,
"<*EXTERNAL pthread_condattr_init*>", 0,
"PROCEDURE condattr_init (VAR attr: pthread_condattr_t): int;", 0,
"<*EXTERNAL pthread_condattr_destroy*>", 0,
"PROCEDURE condattr_destroy (VAR attr: pthread_condattr_t): int;", 0,
"<*EXTERNAL pthread_condattr_getpshared*>", 0,
"PROCEDURE condattr_getpshared (READONLY attr: pthread_condattr_t;", 0,
"                               VAR pshared: int): int;", 0,
"<*EXTERNAL pthread_condattr_setpshared*>", 0,
"PROCEDURE condattr_setpshared (VAR attr: pthread_condattr_t;", 0,
"                               pshared: int): int;", 0,
"", 0,
"(* Functions for handling thread-specific data.  *)", 0,
"TYPE destructor_t = PROCEDURE(arg: ADDRESS);", 0,
"<*EXTERNAL pthread_key_create*>", 0,
"PROCEDURE key_create (VAR key: pthread_key_t;", 0,
"                      destructor: destructor_t): int;", 0,
"<*EXTERNAL pthread_key_delete*>", 0,
"PROCEDURE key_delete (key: pthread_key_t): int;", 0,
"<*EXTERNAL pthread_getspecific*>", 0,
"PROCEDURE getspecific (key: pthread_key_t): ADDRESS;", 0,
"<*EXTERNAL pthread_setspecific*>", 0,
"PROCEDURE setspecific (key: pthread_key_t; value: ADDRESS): int;", 0,
"", 0,
"(* <bits/sigthread.h> *)", 0,
"", 0,
"(* Functions for handling signals. *)", 0,
"<*EXTERNAL pthread_sigmask*>", 0,
"PROCEDURE sigmask (how: int; READONLY set: sigset_t; VAR oset: sigset_t): int;", 0,
"<*EXTERNAL pthread_kill*>", 0,
"PROCEDURE kill (thread: pthread_t; sig: int): int;", 0,
"", 0,
"END Upthread.", 0,
};
    for (i = 0 ; i != sizeof(Data)/sizeof(Data[0]) ; ++i)
    {
        printf(Data[i].Format, Data[i].Value);
        printf("\n");
    }
    return 0;
}
