#include <stdlib.h>

#include "rwg.h"

using namespace rwg;

landmark::landmark() {
    type  =RWG_OBJ_LANDMARK;
    clear();
}

landmark::~landmark() {
    destroy();
}

void landmark::clear(void) {
    object::clear();
}

bool landmark::create(void) {
    if (object::create()==false) return false;

    return true;
}

bool landmark::destroy(void) {
    if (object::destroy()==false) return false;
    clear();
    return true;
}

void landmark::update(void) {
    object::update();
}
