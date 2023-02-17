#include <string>

using namespace vs_automation;

int main(int argc, char** args)
{
	if (argc != 3)
		return 0;

	int pid = std::stoi(args[1]);
	std::string cmd(args[2]);
	if (cmd == "attach_debugger")
		Automator::Execute(pid, OpType::AttachDebugger);
	else if (cmd == "detach_debugger")
		Automator::Execute(pid, OpType::DetachDebugger);

	return 0;
}
