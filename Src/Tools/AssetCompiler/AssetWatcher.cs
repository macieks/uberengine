using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.IO;
using System.Threading;
using System.Windows.Forms;
using ToolsCommonNET;

namespace AssetCompiler
{
    class AssetWatcher
    {
        FileSystemWatcher fileWatcher;

        string targetsString;
        string[] targets;

        // Dependency file to asset names mapping
        Dictionary<string, Dictionary<string, bool /* dummy */>> depsToAssetNames = new Dictionary<string, Dictionary<string, bool>>();

        void Startup(string configString, string[] configs, string rootSrcDir, Dictionary<string, Dictionary<string, bool>> depsToAssetNames)
        {
            this.targetsString = configString;
            this.targets = configs;
            this.depsToAssetNames = depsToAssetNames;

            fileWatcher = new FileSystemWatcher();
            try
            {
                fileWatcher.Path = System.IO.Path.GetFullPath(rootSrcDir);
            }
            catch (Exception)
            {
                fileWatcher.Path = System.IO.Directory.GetCurrentDirectory();
            }
            fileWatcher.IncludeSubdirectories = true;
            fileWatcher.Filter = "*.*";

            fileWatcher.Created += new FileSystemEventHandler(OnCreated);
            fileWatcher.Changed += new FileSystemEventHandler(OnChanged);
            fileWatcher.Renamed += new RenamedEventHandler(OnRenamed);
            fileWatcher.EnableRaisingEvents = true;
        }

        void Shutdown()
        {
            fileWatcher.Dispose();
            fileWatcher = null;
        }

        public void Run(string source, string dir, string configString, string[] configs)
        {
            Console.Title = "UberEngine Asset Watcher (hold ESC+F3 to quit)";
            Logger.LogI("Starting asset watcher...\nAssets: '{0}'\nWatched path: '{1}'\nConfiguration: '{2}'", source, Path.GetFullPath(dir), configString);

            Startup(configString, configs, dir, depsToAssetNames);

            while (!(Utils.IsKeyPressed(Keys.F3) && Utils.IsKeyPressed(Keys.Escape)))
                Thread.Sleep(100);

            Shutdown();

            Logger.LogI("Closed asset watcher...");
        }

        public void AddDependencyEntry(string dependencyName, string assetName)
        {
            dependencyName = Utils.NormalizePath(dependencyName);

            Dictionary<string, bool> depAssetNames;
            if (depsToAssetNames.TryGetValue(dependencyName, out depAssetNames) == false)
            {
                depAssetNames = new Dictionary<string, bool>();
                depsToAssetNames.Add(dependencyName, depAssetNames);
            }

            if (depAssetNames.ContainsKey(assetName) == false)
                depAssetNames.Add(assetName, true);
        }

        void OnCreated(object sender, FileSystemEventArgs e)
        {
            OnFileChange(e.FullPath);
        }

        void OnChanged(object sender, FileSystemEventArgs e)
        {
            OnFileChange(e.FullPath);
        }

        void OnRenamed(object sender, RenamedEventArgs e)
        {
            OnFileChange(e.FullPath);
        }

        void OnFileChange(string path)
        {
            path = Utils.NormalizePath(path);

            Dictionary<string, bool> assetNames;
            if (depsToAssetNames.TryGetValue(path, out assetNames))
            {
                Logger.LogI("Detected change under {0} - re-exporting {1} assets", path, assetNames.Count);

                foreach (string assetName in assetNames.Keys)
                    foreach (string target in targets)
                        AssetCompiler.CompileAsset(target, assetName);

                Logger.LogI("Re-exporting of {0} assets done!", assetNames.Count);
            }
        }
    }
}
