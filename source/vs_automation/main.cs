using System;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using EnvDTE;

namespace vs_automation
{
    public enum OpType
    {
        AttachDebugger,
        DetachDebugger
    }

    public static class Automator
    {
        [DllImport("ole32.dll")]
        private static extern int CreateBindCtx(uint reserved, out IBindCtx ppbc);

        private static DTE dte = null;

        private static DTE GetDTE(int processId)
        {
            string progId = "!VisualStudio.DTE.17.0:" + processId.ToString();
            object runningObject = null;

            IBindCtx bindCtx = null;
            IRunningObjectTable rot = null;
            IEnumMoniker enumMonikers = null;

            try
            {
                Marshal.ThrowExceptionForHR(CreateBindCtx(reserved: 0, ppbc: out bindCtx));
                bindCtx.GetRunningObjectTable(out rot);
                rot.EnumRunning(out enumMonikers);

                IMoniker[] moniker = new IMoniker[1];
                IntPtr numberFetched = IntPtr.Zero;
                while (enumMonikers.Next(1, moniker, numberFetched) == 0)
                {
                    IMoniker runningObjectMoniker = moniker[0];

                    string name = null;

                    try
                    {
                        if (runningObjectMoniker != null)
                        {
                            runningObjectMoniker.GetDisplayName(bindCtx, null, out name);
                        }
                    }
                    catch (UnauthorizedAccessException)
                    {
                        // Do nothing, there is something in the ROT that we do not have access to.
                    }

                    Console.WriteLine(name);

                    if (!string.IsNullOrEmpty(name) && string.Equals(name, progId, StringComparison.Ordinal))
                    {
                        Marshal.ThrowExceptionForHR(rot.GetObject(runningObjectMoniker, out runningObject));
                        break;
                    }
                }
            }
            finally
            {
                if (enumMonikers != null)
                {
                    Marshal.ReleaseComObject(enumMonikers);
                }

                if (rot != null)
                {
                    Marshal.ReleaseComObject(rot);
                }

                if (bindCtx != null)
                {
                    Marshal.ReleaseComObject(bindCtx);
                }
            }

            return (DTE)runningObject;
        }

        public static void Execute(int pid, OpType op)
        {
			if (dte == null)
            {
                foreach (var devenv in System.Diagnostics.Process.GetProcessesByName("devenv"))
				{
                    dte = GetDTE(devenv.Id);
                    if (dte != null)
                        break;
                }
            }
            if (dte != null)
            {
                try
                {
                    switch (op)
                    {
                        case OpType.AttachDebugger:
                            foreach (var p in dte.Debugger.LocalProcesses.OfType<EnvDTE.Process>())
                            {
                                if (p.ProcessID == pid)
                                {
                                    p.Attach();
                                    break;
                                }
                            }
                            break;
                        case OpType.DetachDebugger:
                            foreach (var p in dte.Debugger.DebuggedProcesses.OfType<EnvDTE.Process>())
                            {
                                if (p.ProcessID == pid)
                                {
                                    p.Detach(false);
                                    break;
                                }
                            }
                            break;
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.ToString());
                }
            }
        }
    }
}
