using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Diagnostics;

namespace ToolsCommonNET
{
    public class Logger
    {
        // Helper for asynchronous console output & error handling

        static Thread s_stdOutThread;
        static OutputThread s_stdOut;
        static Thread s_stdErrThread;
        static OutputThread s_stdErr;

        public delegate void LogFunc(string format, params object[] args);

        public class OutputThread
        {
            System.IO.StreamReader reader;
            LogFunc logFunc;
            volatile bool quit = false;

            public OutputThread(System.IO.StreamReader reader, LogFunc logFunc)
            {
                this.reader = reader;
                this.logFunc = logFunc;
            }

            public void Work()
            {
                while (!quit)
                {
                    Thread.Sleep(100);

                    string str;
                    while ((str = reader.ReadLine()) != null)
                        logFunc(str);
                }
            }

            public void Quit()
            {
                quit = true;
            }
        }

        public static void StartOutThreads(System.Diagnostics.Process proc)
        {
            s_stdOut = new OutputThread(proc.StandardOutput, LogI_NoPrefix);
            s_stdOutThread = new Thread(new ThreadStart(s_stdOut.Work));
            s_stdOutThread.Start();

            s_stdErr = new OutputThread(proc.StandardError, LogE_NoPrefix);
            s_stdErrThread = new Thread(new ThreadStart(s_stdErr.Work));
            s_stdErrThread.Start();
        }

        public static void CloseOutThreads()
        {
            if (s_stdOutThread != null)
            {
                s_stdOut.Quit();
                s_stdOutThread.Join();
                s_stdOutThread = null;
            }

            if (s_stdErrThread != null)
            {
                s_stdErr.Quit();
                s_stdErrThread.Join();
                s_stdErrThread = null;
            }
        }

        public static void OpenLogFile(string fileName)
        {
            logFileName = fileName;
            logFileWriter = null;
            try
            {
                logFileWriter = new System.IO.StreamWriter(fileName);
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("Failed to create log file {0}, reason: {1}", fileName, e.Message);
            }
        }

        public static void CloseLogFile()
        {
            if (logFileWriter != null)
            {
                Logger.LogI("Logs saved to {0}", System.IO.Path.GetFullPath(logFileName));

                logFileWriter.Close();
                logFileWriter = null;
            }
        }

        public static void LogI(string format, params object[] args)
        {
            Log(false, true, ConsoleColor.White, Console.Out, format, args);
        }

        public static void LogI_NoPrefix(string format, params object[] args)
        {
            Log(false, false, ConsoleColor.White, Console.Out, format, args);
        }

        public static void LogE(string format, params object[] args)
        {
            Log(true, true, ConsoleColor.Red, Console.Error, format, args);
        }

        static void LogE_NoPrefix(string format, params object[] args)
        {
            Log(true, false, ConsoleColor.Red, Console.Error, format, args);
        }

        public static void Log(bool isError, bool includePrefix, ConsoleColor color, System.IO.TextWriter tw, string format, params object[] args)
        {
            if (format.Length == 0)
                return;

            if (includePrefix)
            {
                StackTrace stackTrace = new StackTrace();
                StackFrame stackFrame = stackTrace.GetFrame(2);

                string fileName = stackFrame != null ? (stackFrame.GetMethod().Module.Name + ":" + stackFrame.GetMethod().Name) : "<unknown>";
                int fileLine = stackFrame != null ? stackFrame.GetFileLineNumber() : 0;
                DateTime dt = DateTime.Now;
                string prefix = "" + dt.Hour + ":" + dt.Minute + ":" + dt.Second + " [" + (isError ? "ERR" : "INF") + ":" + fileName + ":" + fileLine.ToString() + "] ";

                format = prefix + format;
            }

            logFileMutex.WaitOne();

            ConsoleColor prevColor = Console.ForegroundColor;
            Console.ForegroundColor = color;
            tw.WriteLine(format, args);
            tw.Flush();
            Console.ForegroundColor = prevColor;

            if (logFileWriter != null)
                logFileWriter.WriteLine((isError ? "STDERR" : "STDOUT") + ": " + format, args);

            logFileMutex.ReleaseMutex();
        }

        static string logFileName = "";
        static System.Threading.Mutex logFileMutex = new System.Threading.Mutex();
        static System.IO.TextWriter logFileWriter = null;
    }
}
