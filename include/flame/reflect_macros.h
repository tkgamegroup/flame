#pragma once

#define RB(n, ...) struct n {
#define RE };
#define BASE0 enum { base0 = __LINE__ }
#define BASE1 enum { base1 = __LINE__ }
#define BPS(cond, name) BPS##cond(name)
#define BPSn(name)
#define BPSi(name) inline BP::Slot* name##_s() { return n->input(__LINE__ - base0 - 1); }
#define BPSo(name) inline BP::Slot* name##_s() { return n->output(__LINE__ - base1 - 1); }
#define RV(type, name, io, ...) type name; BPS(io, name) // io can be i, o or n, means input, output or none of them
#define RF(name) name

#define BP_IN_BASE_LINE enum { in_base = __LINE__ }
#define BP_OUT_BASE_LINE enum { out_base = __LINE__ }
#define BP_IN(type, name) type name##$i; inline BP::Slot* name##_s() { return n->input(__LINE__ - in_base - 1); }
#define BP_OUT(type, name) type name##$o; inline BP::Slot* name##_s() { return n->output(__LINE__ - out_base - 1); }
#define BP_INd(type, name, decoration) type name##$i##decoration; inline BP::Slot* name##_s() { return n->input(__LINE__ - in_base - 1); }
#define BP_OUTd(type, name, decoration) type name##$o##decoration; inline BP::Slot* name##_s() { return n->output(__LINE__ - out_base - 1); }
