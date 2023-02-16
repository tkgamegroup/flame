#include "vs_automation.h"

#include <string>

#using <debug/vs_automation.dll>

using namespace vs_automation;

void vs_automate(const char* _cmd)
{
	std::string cmd(_cmd);
	if (cmd == "attach_debugger")
		Automator::Execute(OpType::AttachDebugger);
	else if (cmd == "detach_debugger")
		Automator::Execute(OpType::DetachDebugger);
}
