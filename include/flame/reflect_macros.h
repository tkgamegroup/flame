#pragma once

#define R(name) name
#define BASE0 enum { base0 = __LINE__ }
#define BASE1 enum { base1 = __LINE__ }
#define BPS(cond, name) BPS##cond(name)
#define BPSn(name)
#define BPSi(name) inline BP::Slot* name##_s() { return n->input(__LINE__ - base0 - 1); }
#define BPSo(name) inline BP::Slot* name##_s() { return n->output(__LINE__ - base1 - 1); }
#define RV(type, name, io, ...) type name; BPS(io, name) // io can be i, o or n, means input, output or none of them
#define RF(name) name
