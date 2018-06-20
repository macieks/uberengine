using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace ToolsCommonNET
{
    public class Utils
    {
        public static bool Contains(string[] table, string value)
        {
            foreach (string elem in table)
                if (elem == value)
                    return true;
            return false;
        }

        public static string NormalizePath(string path)
        {
            path = path.Replace("\\\\", "\\");
            path = path.Replace("/", "\\");
            path = path.ToLower();
            return path;
        }

        [DllImport("user32.dll")]
        static extern ushort GetKeyState(short nVirtKey);

        const ushort keyDownBit = 0x80;

        public static bool IsKeyPressed(Keys key)
        {
            return ((GetKeyState((short)key) & keyDownBit) == keyDownBit);
        }
    }
}
