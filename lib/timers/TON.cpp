#include "TON.h"

TON::TON(unsigned long pt) : _pt(pt) {}

void TON::setPT(unsigned long pt) {
    _pt = pt;
}

void TON::update(bool in) {
    unsigned long now = millis();

    if (in) {
        if (!_in) {
            _startTime = now;
            _timing = true;
        }

        if (_timing) {
            _et = now - _startTime;
            if (_et >= _pt) {
                _q = true;
                _timing = false;
                _et = _pt;
            }
        }
    } else {
        reset();
    }

    _in = in;
}

bool TON::Q() const { return _q; }
unsigned long TON::ET() const { return _et; }

void TON::reset() {
    _q = false;
    _timing = false;
    _et = 0;
}
