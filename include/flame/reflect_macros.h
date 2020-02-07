#pragma once

#define BP_IN_BASE_LINE enum { in_base = __LINE__ }
#define BP_OUT_BASE_LINE enum { out_base = __LINE__ }
#define BP_IN(type, name) type name##$i; inline BP::Slot* name##_s() { return n->input(__LINE__ - in_base - 1); }
#define BP_OUT(type, name) type name##$o; inline BP::Slot* name##_s() { return n->output(__LINE__ - out_base - 1); }
#define BP_INd(type, name, decoration) type name##$i##decoration; inline BP::Slot* name##_s() { return n->input(__LINE__ - in_base - 1); }
#define BP_OUTd(type, name, decoration) type name##$o##decoration; inline BP::Slot* name##_s() { return n->output(__LINE__ - out_base - 1); }
