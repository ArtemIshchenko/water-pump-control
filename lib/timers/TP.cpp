#include "TP.h"

TP::TP(unsigned long pt) : _pt(pt) {}

void TP::setPT(unsigned long pt) {
    _pt = pt;
}

void TP::update(bool in) {
    unsigned long now = millis();

    if (in && !_in) {
        _startTime = now;
        _q = true;
    }

    if (_q) {
        _et = now - _startTime;
        if (_et >= _pt) {
            _q = false;
            _et = _pt;
        }
    }

    _in = in;
}

bool TP::Q() const { return _q; }
unsigned long TP::ET() const { return _et; }

void TP::reset() {
    _q = false;
    _et = 0;
}
