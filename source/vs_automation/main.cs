using System;
using System.IO;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using EnvDTE;

namespace vs_automation
{
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
                    Marshal.ReleaseComObject(enumMonikers);
                if (rot != null)
                    Marshal.ReleaseComObject(rot);
                if (bindCtx != null)
                    Marshal.ReleaseComObject(bindCtx);
            }

            return (DTE)runningObject;
        }
        private static DTE FindDTE(string title)
        {
            foreach (var devenv in System.Diagnostics.Process.GetProcessesByName("devenv"))
            {
                if (devenv.MainWindowTitle.StartsWith(title))
                    return GetDTE(devenv.Id);
            }
            return null;
        }

        public static void Main(string[] args)
        {
            string ide_title = "";
            string cmd = "";
            int cmd_parms_i = -1;
            for (int i = 0; i < args.Length; i++)
            {
                if (args[i] == "-p")
                {
                    ide_title = args[i + 1];
                    i++;
                }
                else if (args[i] == "-c")
                {
                    cmd = args[i + 1];
                    i++;
                    cmd_parms_i = i;
                    break;
                }
            }

            if (args.Length >= 2)
            {
                if (ide_title.Length == 0)
                    ide_title = "flame";
                dte = FindDTE(ide_title);
                if (dte == null)
                    return;

                try
                {
                    switch (cmd)
                    {
                        case "attach_debugger":
                            {
                                int pid = cmd_parms_i != -1 ? int.Parse(args[cmd_parms_i]) : 0;
                                foreach (var p in dte.Debugger.LocalProcesses.OfType<EnvDTE.Process>())
                                {
                                    if (p.ProcessID == pid)
                                    {
                                        p.Attach();
                                        break;
                                    }
                                }
                            }
                            break;
                        case "detach_debugger":
                            {
                                int pid = cmd_parms_i != -1 ? int.Parse(args[cmd_parms_i]) : 0;
                                foreach (var p in dte.Debugger.DebuggedProcesses.OfType<EnvDTE.Process>())
                                {
                                    if (p.ProcessID == pid)
                                    {
                                        p.Detach(false);
                                        break;
                                    }
                                }
                            }
                            break;
                        case "open_file":
                            {
                                string path = cmd_parms_i != -1 ? args[cmd_parms_i] : "";
                                if (path.Length > 0 && File.Exists(path))
                                    dte.Documents.Open(path);
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
