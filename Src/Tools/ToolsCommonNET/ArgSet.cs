using System;
using System.Collections.Generic;
using System.Text;

namespace ToolsCommonNET
{
    public class ArgSet
    {
        public ArgSet(string[] args)
        {
            int i = 0;
            while (i < args.Length)
            {
                string name = args[i];
                if (name[0] != '-')
                    throw new Exception(string.Format("Unexpected parameter {0}", name));

                i++;
                List<string> values = new List<string>();
                while (i < args.Length && args[i][0] != '-')
                {
                    values.Add(args[i]);
                    i++;
                }

                argsMap[name.Substring(1)] = values;
            }
        }

        public bool GetArg(string name, out string value)
        {
            List<string> values;
            if (!argsMap.TryGetValue(name, out values))
            {
                value = null;
                return false;
            }
            value = values[0];
            return true;
        }

        public bool HasArg(string name)
        {
            return argsMap.ContainsKey(name);
        }

        public bool GetArgs(string name, out List<string> values)
        {
            return argsMap.TryGetValue(name, out values);
        }

        Dictionary<string, List<string>> argsMap = new Dictionary<string, List<string>>();
    }
}
