#include "rwg.h"

using namespace rwg;

int object::next_id=1;

object::object(){
    next  =NULL;
    prev  =NULL;
    type  =RWG_OBJ_DEFAULT;
    give_new_id();
    clear();

    object_count++;
}

object::~object() {
    destroy();

    object_count--;
}

void object::clear(void) {
    broken = true;
}

void object::tidy(void) {

}

bool object::create(void) {
    if (broken==false) return false;

    if (id==-1) {
        // Probably maximum id limit exceeded:
        return false;
    }

    broken=false;
    return true;
}

bool object::destroy(void) {
    if (broken) return false;
    clear();
    return true;
}

void object::update(void) {

}


