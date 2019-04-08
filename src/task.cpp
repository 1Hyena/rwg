#include <stdlib.h>

#include "rwg.h"

using namespace rwg;

task::task() {
    type  =RWG_OBJ_TASK;
    clear();
}

task::~task() {
    destroy();
}

void task::clear(void) {
    object::clear();

    job="";
    arg_int=0;
    arg_double=0.0;
    arg_double2=0.0;
    done=0.0;
}

bool task::create(void) {
    if (object::create()==false) return false;

    return true;
}

bool task::destroy(void) {
    if (object::destroy()==false) return false;
    clear();
    return true;
}

void task::update(void) {
    if (!job.compare("set_smooth")) {
        maps["target"]->set_smooth(arg_int);
    }
    else if (!job.compare("scramble")) {
        if ( (done=maps["target"]->iscramble()) < 1.0) {
            return;
        }
    }
    else if (!job.compare("generate")) {
        if ( (done=maps["target"]->igenerate(maps["noise"],arg_int)) < 1.0) {
            return;
        }
    }
    else if (!job.compare("mix")) {
        if ( (done=maps["target"]->imix(maps["relief"],maps["rocks"],arg_double, arg_double2)) < 1.0) {
            return;
        }
    }
    else if (!job.compare("multiply")) {
        if ( (done=maps["target"]->imultiply(arg_double)) < 1.0) {
            return;
        }
    }
    else if (!job.compare("map_multiply")) {
        if ( (done=maps["target"]->imap_multiply(maps["argument"])) < 1.0) {
            return;
        }
    }
    else if (!job.compare("add")) {
        if ( (done=maps["target"]->iadd(arg_double)) < 1.0) {
            return;
        }
    }
    destroy();
    done=1.0;
    return;
}
