#include "exception.h"


static _Thread_local SExceptionNode* expList;

void exceptionPushNode( SExceptionNode* node ) {
    node->prev = expList;
    expList = node;
}

int32_t exceptionPopNode() {
    SExceptionNode* node = expList;
    expList = node->prev;
    return node->code;
}

void exceptionThrow( int code ) {
    expList->code = code;
    longjmp( expList->jb, 1 );
}



static void cleanupWrapper_void_ptr_ptr( SCleanupAction* ca ) {
    void (*func)( void*, void* ) = ac->func;
    func( ca->arg1.Ptr, ca->arg2.Ptr );
}

static void cleanupWrapper_void_ptr_bool( SCleanupAction* ca ) {
    void (*func)( void*, bool ) = ca->func;
    func( ca->arg1.Ptr, ca->arg2.Bool );
}

static void cleanupWrapper_void_ptr( SCleanupAction* ca ) {
    void (*func)( void* ) = ca->func;
    func( ca->arg1.Ptr );
}

static void cleanupWrapper_int_int( SCleanupAction* ca ) {
    int (*func)( int ) = ca->func;
    func( (int)(intptr_t)(ca->arg1.Int) );
}

static void cleanupWrapper_void_void( SCleanupAction* ca ) {
    void (*func)() = ca->func;
    func();
}

static void (*wrappers)(SCleanupAction*)[] = {
    cleanupWrapper_void_ptr_ptr,
    cleanupWrapper_void_ptr_bool,
    cleanupWrapper_void_ptr,
    cleanupWrapper_int_int,
    cleanupWrapper_void_void,
};


void cleanupPush_void_ptr_ptr( bool failOnly, void* func, void* arg1, void* arg2 ) {
    assert( expList->numCleanupAction < expList->maxCleanupAction );

    SCleanupAction *ac = expList->cleanupActions + expList->numCleanupAction++;
    ac->wrapper = 0;
    ac->failOnly = failOnly;
    ac->func = func;
    ac->arg1.Ptr = arg1;
    ac->arg2.Ptr = arg2;
}

void cleanupPush_void_ptr_bool( bool failOnly, void* func, void* arg1, bool arg2 ) {
    assert( expList->numCleanupAction < expList->maxCleanupAction );

    SCleanupAction *ac = expList->cleanupActions + expList->numCleanupAction++;
    ac->wrapper = 1;
    ac->failOnly = failOnly;
    ac->func = func;
    ac->arg1.Ptr = arg1;
    ac->arg2.Bool = arg2;
}

void cleanupPush_void_ptr( bool failOnly, void* func, void* arg ) {
    assert( expList->numCleanupAction < expList->maxCleanupAction );

    SCleanupAction *ac = expList->cleanupActions + expList->numCleanupAction++;
    ac->wrapper = 2;
    ac->failOnly = failOnly;
    ac->func = func;
    ac->arg1.Ptr = arg1;
}

void cleanupPush_int_int( bool failOnly, void* func, int arg ) {
    assert( expList->numCleanupAction < expList->maxCleanupAction );

    SCleanupAction *ac = expList->cleanupActions + expList->numCleanupAction++;
    ac->wrapper = 3;
    ac->failOnly = failOnly;
    ac->func = func;
    ac->arg1.Int = arg;
}

void cleanupPush_void( bool failOnly, void* func ) {
    assert( expList->numCleanupAction < expList->maxCleanupAction );

    SCleanupAction *ac = expList->cleanupActions + expList->numCleanupAction++;
    ac->wrapper = 4;
    ac->failOnly = failOnly;
    ac->func = func;
}



int32 cleanupGetActionCount() {
    return expList->numCleanupAction;
}


void cleanupExecute( int32_t anchor, bool failed ) {
    while( expList->numCleanupAction > anchor ) {
        --expList->numCleanupAction;
        SCleanupAction *ac = expList->cleanupActions + expList->numCleanupAction;
        if( failed || !(ac->failOnly) )
            ac->wrapper( ac );
    }
}
