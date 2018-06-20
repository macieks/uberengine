using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.IO;
using System.Reflection;
using ToolsCommonNET;

namespace AssetPackageBuilder
{
    class AssetPackageBuilder
    {
        static int Main(string[] args)
        {
            // Parse arguments

            ArgSet argSet;
            try
            {
                argSet = new ArgSet(args);
            }
            catch (Exception e)
            {
                System.Console.Error.WriteLine("Failed to parse arguments, reason:\n{0}", e.Message);
                return 1;
            }

            // Open log file

            string logFileName;
            if (!argSet.GetArg("log", out logFileName))
                logFileName = "asset_package_builder.log";
            Logger.OpenLogFile(logFileName);

            // Get config and source files

            string targetsFile;
            if (!argSet.GetArg("targetsFile", out targetsFile))
            {
                Logger.LogE("Missing 'targetsFile' argument.");
                Quit();
                return 1;
            }
            string fullTargetsFile = Path.GetFullPath(targetsFile);

            if (!argSet.GetArg("target", out target))
            {
                Logger.LogE("Missing 'target' argument.");
                Quit();
                return 1;
            }

            if (!argSet.GetArg("platform", out platform))
            {
                Logger.LogE("Missing 'platform' argument.");
                Quit();
                return 1;
            }

            if (!argSet.GetArg("assetPackage", out assetPackage))
            {
                Logger.LogE("Missing 'asset_package' argument.");
                Quit();
                return 1;
            }

            // Get optional no-wait parameter

            noWait = argSet.HasArg("noWait");

            // Determine root dirs

            Assembly a = Assembly.GetEntryAssembly();
            binDir = Path.GetDirectoryName(a.Location).Replace("\\", "/");
            rootDir = Path.GetDirectoryName(Path.GetFullPath(assetPackage)).Replace("\\", "/");

            // Change root directory

            Directory.SetCurrentDirectory(rootDir);

            // Start work

            Console.Title = "UberEngine Asset Package Builder";
            Logger.LogI("Starting building package '{0}' for '{1}' target...", Path.GetFullPath(assetPackage), target);

            // Load targets

            Logger.LogI("Loading targets from '{0}'...", targetsFile);
            try
            {
                AssetTargetSet.Startup(targetsFile);
            }
            catch (Exception e)
            {
                Logger.LogE("Failed to load targets - does file '{0}' exist and is correct?, reason: {1}", targetsFile, e.Message);
                Quit();
                return 0;
            }

            // Create temp directory

            tmpDir = "tmp_666";
            Logger.LogI("(Re)creating temporary directory '{0}'...", tmpDir);
            try
            {
                Directory.Delete(tmpDir, true);
            }
            catch (Exception) { }
            try
            {
                Directory.CreateDirectory(tmpDir);
            }
            catch (Exception e)
            {
                Logger.LogE("Failed to (re)create temp directory - is directory '{0}' writable?, reason: {1}", tmpDir, e.Message);
                Quit();
                return 0;
            }

            // Build package

            bool success = BuildPackage();

            // Delete temp directory

            try
            {
                // Remove "read-only" flag
                (new DirectoryInfo(tmpDir)).Attributes &= ~FileAttributes.ReadOnly;
                // Delete recursively
                Directory.Delete(tmpDir, true);
            }
            catch (Exception) { }

            // Return

            Quit();
            return success ? 0 : 1;
        }

        static void Quit()
        {
            Logger.CloseLogFile();
            if (!noWait)
            {
                Console.Out.WriteLine();
                Console.Out.WriteLine("Press any key to quit...");
                Console.In.Read();
            }
        }

        static bool BuildPackage()
        {
            // Copy assets / files to temporary directory

            Logger.LogI("Copying assets to temporary directory '{0}'...", tmpDir);

            XmlDocument doc = new XmlDocument();
            try
            {
                doc.Load(assetPackage);
            }
            catch (Exception e)
            {
                Logger.LogE(string.Format("Failed to load asset group file '{0}', reason:\n{1}", assetPackage, e.Message));
                return false;
            }

            XmlElement assetPackageNode = doc.ChildNodes[1] as XmlElement;

            rootSrcDir = assetPackageNode.Attributes["rootSrcDir"].Value;
            rootExpDir = assetPackageNode.Attributes["rootExpDir"].Value;

            AssetTargetSet.SetRootSrcDir(rootSrcDir);

            foreach (XmlNode node in assetPackageNode.ChildNodes)
            {
                XmlElement element = node as XmlElement;
                if (element == null)
                    continue;

                if (element.Name == "asset")
                {
                    if (!CopyAsset(element.Attributes["name"].Value))
                        return false;
                }
                else if (element.Name == "file")
                {
                    if (!CopyFile(element.Attributes["src"].Value))
                        return false;
                }
            }

            // Compress assets into final package

            string binAssetPackageName = Path.GetFileNameWithoutExtension(Path.GetFileNameWithoutExtension(assetPackage)) + "-" + target + ".afs";

            Logger.LogI("Compressing to AFS file '{0}'...", binAssetPackageName);

            string afsPackerPath = binDir + "/AFSPacker.exe";

            string args = platform + " \"" + tmpDir + "\" \"" + binAssetPackageName + "\"";
            Logger.LogI("Running: {0} {1}", afsPackerPath, args);

            System.Diagnostics.Process proc = new System.Diagnostics.Process();
            try
            {
                proc.EnableRaisingEvents = false;
                proc.StartInfo.FileName = afsPackerPath;
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
                throw new Exception(string.Format("AFS packer process startup failure, reason:\n{0}", e.Message));
            }

            Logger.CloseOutThreads();

            if (proc.ExitCode != 0)
            {
                Logger.LogE(string.Format("AFS packer '{0}' failed (return code: {1})\narguments: {2}\nprocess stderr:\n{3}process stdout:\n{4}",
                    afsPackerPath, proc.ExitCode, proc.StartInfo.Arguments, proc.StandardError.ReadToEnd(), proc.StandardOutput.ReadToEnd()));
                return false;
            }
            else
            {
                Logger.LogI(proc.StandardOutput.ReadToEnd());
                Logger.LogE(proc.StandardError.ReadToEnd());
            }

            return true;
        }

        static bool CopyAsset(string name)
        {
            string assetType;
            string configName;
            AssetTargetSet.AssetInfo assetInfo = AssetTargetSet.GetAssetInfo(out assetType, out configName, name, target);
            if (assetInfo == null)
                return false;

            string src = rootExpDir + "/" + configName + "/" + name + ".asset";
            string dst = tmpDir + "/" + name + ".asset";

            try
            {
                Directory.CreateDirectory(Path.GetDirectoryName(dst));
                File.Copy(src, dst, true);
            }
            catch (Exception)
            {
                Logger.LogE("Failed to copy asset: '{0}' -> '{1}'", src, dst);
                return false;
            }
            return true;
        }

        static bool CopyFile(string srcFileName)
        {
            string src = rootSrcDir + "/" + srcFileName;
            string dst = tmpDir + "/" + src;

            try
            {
                Directory.CreateDirectory(Path.GetDirectoryName(dst));
                File.Copy(src, dst, true);
            }
            catch (Exception)
            {
                Logger.LogE("Failed to copy file: '{0}' -> '{1}'", src, dst);
                return false;
            }
            return true;
        }

        static string assetPackage;

        static string target;
        static string platform;

        static string rootDir;
        static string binDir;

        static string rootSrcDir;
        static string rootExpDir;

        static string tmpDir;

        static bool noWait;
    }
}
