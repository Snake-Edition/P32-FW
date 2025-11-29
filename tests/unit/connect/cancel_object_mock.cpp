#include <feature/cancel_object/cancel_object.hpp>

using namespace buddy;

CancelObject &buddy::cancel_object() {
    static CancelObject instance;
    return instance;
}

bool CancelObject::is_object_cancelled(ObjectID) const {
    return false;
}

CancelObject::ObjectID CancelObject::object_count() const {
    return 0;
}
