#include <stdlib.h>

#include "rwg.h"

using namespace rwg;

lake::lake() {
    type  =RWG_OBJ_LAKE;
    clear();
}

lake::~lake() {
    destroy();
}

void lake::clear(void) {
    object::clear();
}

bool lake::create(void) {
    if (object::create()==false) return false;

    return true;
}

bool lake::destroy(void) {
    if (object::destroy()==false) return false;
    clear();
    return true;
}

void lake::update(void) {
    object::update();
}
