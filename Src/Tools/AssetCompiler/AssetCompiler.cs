using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.IO;
using System.Reflection;
using System.Windows.Forms;
using ToolsCommonNET;

namespace AssetCompiler
{
    class AssetCompiler
    {
        static int Main(string[] args)
        {
            prevConsoleColor = Console.ForegroundColor;
            prevConsoleTitle = Console.Title;

            // Parse arguments

            ArgSet argSet;
            try
            {
                argSet = new ArgSet(args);
            }
            catch (Exception e)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                System.Console.Error.WriteLine("Failed to parse arguments, reason:\n{0}", e.Message);
                Quit();
                return 1;
            }

            // Check "no wait" parameter

            if (argSet.HasArg("noWait"))
                noWait = true;

            // Check "wait on error" parameter

            if (argSet.HasArg("waitOnError"))
                waitOnError = true;

            // Check "build" parameter

            if (argSet.HasArg("noBuild"))
                build = false;

            // Check "rebuild" parameter

            if (argSet.HasArg("rebuild"))
                rebuild = true;

            // Open log file

            string logFileName;
            if (!argSet.GetArg("log", out logFileName))
                logFileName = "asset_compiler.log";
            Logger.OpenLogFile(logFileName);

            // Get config and source files

            string targetsFile;
            if (!argSet.GetArg("targetsFile", out targetsFile))
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Logger.LogE("Missing 'targetsFile' argument.");
                Quit();
                return 1;
            }
            string fullTargetsFile = Path.GetFullPath(targetsFile);

            if (!argSet.GetArg("targets", out targetsString))
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Logger.LogE("Missing 'targets' argument.");
                Quit();
                return 1;
            }

            string source;
            if (!argSet.GetArg("source", out source))
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Logger.LogE("Missing 'source' argument.");
                Quit();
                return 1;
            }

            // Check whether to enable asset watcher

            if (argSet.HasArg("enableAssetWatcher"))
                assetWatcher = new AssetWatcher();

            // Determine bin & root dirs

            Assembly a = Assembly.GetEntryAssembly();
            binDir = Path.GetDirectoryName(a.Location).Replace("\\", "/");
            rootDir = Path.GetDirectoryName(Path.GetFullPath(source)).Replace("\\", "/");

            // Change root directory

            Directory.SetCurrentDirectory(rootDir);

            // Start work

            targets = targetsString.Split(';');
            string rootAssetGroupPath = Path.GetFileName(source).Replace("\\", "/");

            if (build)
            {
                Console.Title = "UberEngine Asset Compiler (hold ESC+F3 to quit)";
                Logger.LogI("Starting compilation of '{0}' for '{1}' targets...", Path.GetFullPath(rootAssetGroupPath), targetsString);
            }

            try
            {
                AssetTargetSet.Startup(fullTargetsFile);

                // If we're in "asset watcher" mode only, then we need to build dependency map

                foreach (string target in targets)
                    CompileAssetGroup(target, rootAssetGroupPath, true);
            }
            catch (Exception e)
            {
                wasError = true;
                Logger.LogE(e.Message);
                Quit();
                return 1;
            }

            if (build)
            {
                // Report build result

                Logger.LogI("");
                if (quit)
                    Logger.LogI("COMPILATION ABORTED !");
                else
                    Logger.LogI("FINISHED SUCCESSFULLY !");
                Logger.LogI("");
                Logger.LogI("Compiled: {0}", numCompiledAssets);
                if (rebuild)
                    Logger.LogI("This was full rebuild.");
                else
                    Logger.LogI("Up-to-date: {0}", numSkippedAssets);
            }

            // Run asset watcher

            if (assetWatcher != null)
                assetWatcher.Run(source, rootSrcDir, targetsString, targets);

            // Quit

            Quit();
            return 0;
        }

        static void Quit()
        {
            Logger.CloseLogFile();
            if (!noWait || (wasError && waitOnError))
            {
                Console.Out.WriteLine();
                Console.Out.WriteLine("Press any key to quit...");
                Console.In.Read();
            }

            Console.Title = prevConsoleTitle;
            Console.ForegroundColor = prevConsoleColor;
        }

        static void CompileAssetGroup(string target, string assetGroupFilePath, bool isRootGroup)
        {
/*
            Disabled as it was resulting in too much spam especially when there's nothing to be rebuilt.

            if (!isRootGroup)
              Logger.LogI("COMPILING ASSET GROUP '{0}' ...", assetGroupFilePath);
*/
            XmlDocument doc = new XmlDocument();
            try
            {
                doc.Load(assetGroupFilePath);
            }
            catch (Exception e)
            {
                throw new Exception(string.Format("Failed to load asset group file '{0}', reason:\n{1}", assetGroupFilePath, e.Message));
            }

            XmlElement assetsElement = doc.ChildNodes[1] as XmlElement;

            if (rootSrcDir == null)
            {
                rootSrcDir = assetsElement.Attributes["rootSrcDir"].Value;
                rootExpDir = assetsElement.Attributes["rootExpDir"].Value;
                rootDepDir = assetsElement.Attributes["rootDepDir"].Value;

                AssetTargetSet.SetRootSrcDir(rootSrcDir);
            }
            else
            {
                if (assetsElement.HasAttribute("rootSrcDir") ||
                    assetsElement.HasAttribute("rootExpDir") ||
                    assetsElement.HasAttribute("rootDepDir"))
                    Logger.LogI("[WRN] Child asset group {0} has unnecessary root dir(s) specified - they'll be ignored", assetGroupFilePath);
            }

            foreach (XmlNode assetNode in assetsElement.ChildNodes)
            {
                if (Utils.IsKeyPressed(Keys.F3) && Utils.IsKeyPressed(Keys.Escape))
                {
                    Logger.LogI("Aborting on user request...");
                    quit = true;
                }
                if (quit)
                    return;

                if (assetNode.Name == "asset")
                {
                    XmlElement assetElement = assetNode as XmlElement;
                    XmlAttribute nameAttr = assetElement.Attributes["name"];
                    string name = nameAttr.Value;

                    try
                    {
                        CompileAsset(target, name);
                    }
                    catch (Exception e)
                    {
                        throw new Exception(string.Format("Compilation of asset '{0}' for target '{1}' failed, reason:\n{2}", name, target, e.Message));
                    }
                }
                else if (assetNode.Name == "assetGroup")
                {
                    XmlElement assetGroupElement = assetNode as XmlElement;
                    XmlAttribute nameAttr = assetGroupElement.Attributes["name"];
                    string name = nameAttr.Value;

                    string childAssetGroupFilePath = rootSrcDir + "/" + name;

                    try
                    {
                        CompileAssetGroup(target, childAssetGroupFilePath, false);
                    }
                    catch (Exception e)
                    {
                        throw new Exception(string.Format("Compilation of child asset group '{0}' for target '{1}' failed, reason:\n{2}", name, target, e.Message));
                    }
                }
            }
        }

        public static void CompileAsset(string target, string name)
        {
            string configName;
            string assetType;
            AssetTargetSet.AssetInfo assetInfo = AssetTargetSet.GetAssetInfo(out assetType, out configName, name, target);
            if (assetInfo == null)
                throw new Exception(string.Format("Failed to determine asset compiler and config for asset '{0}' for '{1}' target.", name, target));
            if (configName == "none")
                return;

            if (!build && assetWatcher != null)
            {
                CheckUpToDate(name, configName); // This will build dependency map for us
                numSkippedAssets++;
                return;
            }

            if (!rebuild && CheckUpToDate(name, configName))
            {
                numSkippedAssets++;
                return;
            }

            Logger.LogI("COMPILING '{0}' WITH '{1}'...", name, assetInfo.compilerName);

            string compilerPath = binDir + "/" + assetInfo.compilerName;
            if (!System.IO.File.Exists(compilerPath))
                throw new Exception(string.Format("Asset compiler '{0}' (path = '{1}') not found.", assetInfo.compilerName, compilerPath));

            string args =
                "-asset \"" + name +
                "\" -config \"" + configName +
                "\" -root-src \"" + rootSrcDir +
                "\" -root-exp \"" + rootExpDir +
                "\" -root-dep \"" + rootDepDir +
                "\"";
            Logger.LogI("Running: {0} {1}", compilerPath, args);

            System.Diagnostics.Process proc = new System.Diagnostics.Process();
            try
            {
                proc.EnableRaisingEvents = false;
                proc.StartInfo.FileName = compilerPath;
                proc.StartInfo.Arguments = args;
                proc.StartInfo.UseShellExecute = false;
                proc.StartInfo.RedirectStandardOutput = true;
                proc.StartInfo.RedirectStandardError = true;
                proc.StartInfo.CreateNoWindow = true;
                proc.Start();

                Logger.StartOutThreads(proc);
                proc.WaitForExit();
            }
            catch (Exception e)
            {
                Logger.CloseOutThreads();
                throw new Exception(string.Format("Asset compiler process startup failure, reason:\n{0}", e.Message));
            }

            Logger.CloseOutThreads();

            if (proc.ExitCode != 0)
                throw new Exception(string.Format("Asset compiler '{0}' failed (return code: {1})\narguments: {2}\nprocess stderr:\n{3}process stdout:\n{4}",
                    assetInfo.compilerName, proc.ExitCode, proc.StartInfo.Arguments, proc.StandardError.ReadToEnd(), proc.StandardOutput.ReadToEnd()));
            else
            {
                numCompiledAssets++;
                Logger.LogI(proc.StandardOutput.ReadToEnd());
                Logger.LogE(proc.StandardError.ReadToEnd());
            }
        }

        static bool CheckUpToDate(string assetName, string config)
        {
            XmlDocument doc = new XmlDocument();
            try
            {
                doc.Load(rootDepDir + "/" + config + "/" + assetName + ".deps.xml");

                // Process all dependendies
                XmlNode dependenciesNode = doc.ChildNodes[1];
                foreach (XmlNode dependencyNode in dependenciesNode.ChildNodes)
                {
                    XmlElement dependencyElement = dependencyNode as XmlElement;

                    string path = dependencyElement.Attributes["path"].Value;
                    string dateAsString = dependencyElement.Attributes["date"].Value;
                    DateTime date = DateTime.Parse(dateAsString);

                    // Get current file's modification date & time
                    DateTime currDateFull = FileTime.GetLastWriteTime(path);
                    DateTime currDate = new DateTime(currDateFull.Year, currDateFull.Month, currDateFull.Day, currDateFull.Hour, currDateFull.Minute, currDateFull.Second);
                    if (date != currDate)
                        return false;

                    // Add entry to dependency map

                    if (assetWatcher != null)
                        assetWatcher.AddDependencyEntry(path, assetName);
                }
            }
            catch (Exception)
            {
                return false;
            }

            return true;
        }

        // Asset compiler configuration

        static string targetsString = null;
        static string[] targets = null;

        static string binDir = null;
        static string rootDir = null;

        public static string rootDepDir = null;
        public static string rootExpDir = null;
        public static string rootSrcDir = null;

        static bool build = true;
        static bool rebuild = false;
        static bool noWait = false;
        static bool waitOnError = false;

        // Compilation state

        static bool quit = false;
        static bool wasError = false;

        static int numSkippedAssets = 0;
        static int numCompiledAssets = 0;

        // Asset watcher

        static AssetWatcher assetWatcher = null;

        // Initial console state (needed so that we can restore it when done; hence AssetCompiler may be run from another process with console being shared)

        static ConsoleColor prevConsoleColor;
        static string prevConsoleTitle;
    }
}
