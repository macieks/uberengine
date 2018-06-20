using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ue_gen_xml_for_png
{
    class Program
    {
        static void Main(string[] args)
        {
            string[] paths = System.IO.Directory.GetFiles("./");
            foreach (string path in paths)
            {
                string extension = System.IO.Path.GetExtension(path);
                if (extension != ".png")
                    continue;

                string name = System.IO.Path.GetFileNameWithoutExtension(path);
                string content =
                    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" +
                    "<asset parent=\"puzzled_rabbit/texture\">\n" +
                      "\t<paramGroup name=\"all\">\n" +
                        "\t\t<param name=\"source\" value=\"puzzled_rabbit/font/" + name + ".png\"/>\n" +
                      "\t</paramGroup>\n" +
                    "</asset>";
                name = name.Substring(0, name.Length - 3).ToLower(); // get rid of '@2x'
                System.IO.File.WriteAllText(name + ".asset.xml", content);
            }
        }
    }
}
