#include "TOF.h"

TOF::TOF(unsigned long pt) : _pt(pt) {}

void TOF::setPT(unsigned long pt) {
    _pt = pt;
}

void TOF::update(bool in) {
    unsigned long now = millis();

    if (in) {
        _q = true;
        _timing = false;
        _et = 0;
    } else {
        if (_in) {
            _startTime = now;
            _timing = true;
        }

        if (_timing) {
            _et = now - _startTime;
            if (_et >= _pt) {
                _q = false;
                _timing = false;
                _et = _pt;
            }
        }
    }

    _in = in;
}

bool TOF::Q() const { return _q; }
unsigned long TOF::ET() const { return _et; }

void TOF::reset() {
    _q = false;
    _timing = false;
    _et = 0;
}
