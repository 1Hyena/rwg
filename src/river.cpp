#include <stdlib.h>

#include "rwg.h"

using namespace rwg;

river::river() {
    type  =RWG_OBJ_RIVER;
    clear();
}

river::~river() {
    destroy();
}

void river::clear(void) {
    object::clear();

    trail.clear();
}

bool river::create(void) {
    if (object::create()==false) return false;

    return true;
}

bool river::destroy(void) {
    if (object::destroy()==false) return false;
    clear();
    return true;
}

void river::update(void) {
    object::update();
}
