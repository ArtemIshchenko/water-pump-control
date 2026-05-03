#include "EdgeDetector.h"

EdgeDetector::EdgeDetector(bool initValue)
    : _prev(initValue), _current(initValue) {}

void EdgeDetector::update(bool value) {
    _current = value;
    rose     = value && !_prev;
    fell     = !value && _prev;
    _prev    = value;
}

bool EdgeDetector::value() const {
    return _current;
}
