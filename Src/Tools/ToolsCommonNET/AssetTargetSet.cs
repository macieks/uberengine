using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;

namespace ToolsCommonNET
{
    public class AssetTargetSet
    {
        public static void SetRootSrcDir(string rootSrcDir)
        {
            AssetTargetSet.rootSrcDir = rootSrcDir;
        }

        public static void Startup(string targetsPath)
        {
            XmlDocument doc = new XmlDocument();
            try
            {
                doc.Load(targetsPath);
            }
            catch (Exception e)
            {
                throw new Exception(string.Format("Failed to load asset targets file '{0}', reason:\n{1}", targetsPath, e.Message));
            }

            try
            {
                XmlElement targetsElement = doc.ChildNodes[1] as XmlElement;
                if (targetsElement.Name != "targets")
                    throw new Exception("Expected 'targets' Xml element.");

                foreach (XmlNode targetNode in targetsElement.ChildNodes)
                {
                    XmlElement targetElement = targetNode as XmlElement;
                    if (targetElement.Name != "target")
                        continue;

                    string targetName = targetElement.Attributes["name"].Value;
                    Target target = new Target();

                    // Copy configs from parent first

                    string parentName = targetElement.HasAttribute("parent") ? targetElement.Attributes["parent"].Value : null;
                    if (parentName != null && parentName.Length > 0)
                    {
                        Target parent;
                        if (!targets.TryGetValue(parentName, out parent))
                            throw new Exception(string.Format("Target '{0}' references non-existant '{1}' parent target", targetName, parentName));

                        foreach (string assetType in parent.assetTypeToConfig.Keys)
                            target.assetTypeToConfig.Add(assetType, parent.assetTypeToConfig[assetType]);
                    }

                    // Load configs

                    foreach (XmlNode configNode in targetElement.ChildNodes)
                    {
                        XmlElement configElement = configNode as XmlElement;
                        if (configElement.Name != "config")
                            continue;

                        string configName = configElement.Attributes["configName"].Value;
                        string assetType = configElement.Attributes["assetType"].Value;

                        Config config;
                        if (!configs.TryGetValue(configName, out config))
                        {
                            config = new Config();
                            config.name = configName;
                            configs.Add(configName, config);
                        }

                        target.assetTypeToConfig.Add(assetType, config);
                    }

                    // Add target

                    targets.Add(targetName, target);
                }
            }
            catch (Exception e)
            {
                throw new Exception(string.Format("Failed to parse asset targets file '{0}', reason:\n{1}", targetsPath, e.Message));
            }
        }

        public static bool GetAssetConfig(out Config config, string assetType, string targetName)
        {
            config = null;
            Target target;
            if (!targets.TryGetValue(targetName, out target))
                return false;
            if (!target.assetTypeToConfig.TryGetValue(assetType, out config))
                return false;
            return true;
        }

        public static AssetInfo GetAssetInfo(out string assetType, out string configName, string assetName, string targetName)
        {
            // Get existing asset info

            Config config = null;
            AssetInfo assetInfo = null;
            if (assetTypes.TryGetValue(assetName, out assetType))
            {
                if (!GetAssetConfig(out config, assetType, targetName))
                    throw new Exception(string.Format("Failed to deduce asset '{0}' config, reason: asset type '{1}' has no defined configs for target '{2}'.", assetName, assetType, targetName));

                if (config.assetInfos.TryGetValue(assetName, out assetInfo))
                {
                    configName = config.name;
                    return assetInfo;
                }
            }

            // Load asset info

            XmlDocument doc = new XmlDocument();
            string assetPath = rootSrcDir + "/" + assetName + ".asset.xml";
            try
            {
                doc.Load(assetPath);
            }
            catch (Exception e)
            {
                throw new Exception(string.Format("Failed to load asset description Xml file (path = '{0}'), reason:\n{1}", assetPath, e.Message));
            }

            XmlElement assetDescElement = doc.ChildNodes[1] as XmlElement;

            // Load parent first

            XmlAttribute parentAttr = assetDescElement.Attributes["parent"];
            string parentConfigName;
            AssetInfo parentAssetInfo = (parentAttr.Value.Length != 0) ? GetAssetInfo(out assetType, out parentConfigName, parentAttr.Value, targetName) : null;

            // Determine asset type first

            if (assetType == null)
            {
                if (assetDescElement.HasAttribute("type") == false)
                    throw new Exception(string.Format("Failed to determine type of '{0}' asset", assetName));
                assetType = assetDescElement.Attributes["type"].Value;
            }

            assetTypes.Add(assetName, assetType);

            // Get config

            if (!GetAssetConfig(out config, assetType, targetName))
                throw new Exception(string.Format("Failed to deduce asset '{0}' config, reason: asset type '{1}' has no defined configs for target '{2}'.", assetName, assetType, targetName));
            configName = config.name;

            // Get compiler

            string compiler_all = null;
            string compiler_config = null;

            assetInfo = new AssetInfo();

            foreach (XmlNode paramGroupNode in assetDescElement.ChildNodes)
            {
                if (paramGroupNode.Name != "paramGroup")
                    continue;

                XmlElement paramGroupElement = paramGroupNode as XmlElement;
                XmlAttribute paramGroupNameAttr = paramGroupElement.Attributes["name"];

                if (paramGroupNameAttr.Value == "all")
                    compiler_all = GetElemValue(paramGroupElement, "compiler");
                else if (configName == paramGroupNameAttr.Value)
                {
                    compiler_config = GetElemValue(paramGroupElement, "compiler");
                    if (compiler_config != null)
                        break;
                }
            }

            if (compiler_config != null)
                assetInfo.compilerName = compiler_config; // Set compiler from "all" config group
            else if (compiler_all != null)
                assetInfo.compilerName = compiler_all; // Set compiler from matching config group
            else if (parentAssetInfo != null)
                assetInfo.compilerName = parentAssetInfo.compilerName; // Set compiler from parent

            config.assetInfos[assetName] = assetInfo;
            return assetInfo;
        }

        static string GetElemValue(XmlNode node, string name)
        {
            foreach (XmlNode child in node.ChildNodes)
            {
                if (child.Name != "param")
                    continue;

                XmlElement childElement = child as XmlElement;
                XmlAttribute childNameAttr = childElement.Attributes["name"];
                if (childNameAttr.Value == name)
                {
                    if (childElement.HasAttribute("value"))
                        return childElement.Attributes["value"].Value;
                    return null;
                }
            }
            return null;
        }

        static string rootSrcDir;

        static Dictionary<string, string> assetTypes = new Dictionary<string, string>();

        public class AssetInfo
        {
            public string compilerName;
        };
        public class Config
        {
            public string name;
            public Dictionary<string, AssetInfo> assetInfos = new Dictionary<string, AssetInfo>();
        }
        static Dictionary<string, Config> configs = new Dictionary<string, Config>();

        class Target
        {
            public Dictionary<string, Config> assetTypeToConfig = new Dictionary<string, Config>();
        }
        static Dictionary<string, Target> targets = new Dictionary<string,Target>();
    }
}
