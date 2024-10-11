using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LOCCounter
{
    internal class DirectoryAnalyzer
    {
        public static List<string> s_IgnoredDirectories = new List<string>();
        public static List<string> s_IncludedExtensions = new List<string>();
        public static int GetLOCCountFromDirectory(string directory)
        {
            int loc = 0;
            string[] files = Directory.GetFiles(directory);
            foreach (var file in files)
            {
                foreach (var ext in s_IncludedExtensions)
                {
                    if (file.EndsWith(ext))
                    {
                        int linesFromFile = File.ReadLines(file).Count();
                        Console.ForegroundColor = ConsoleColor.White;
                        Console.Write("{0}: {1} lines.\r\n", file, linesFromFile);
                        loc += linesFromFile;
                        break;
                    }
                }
            }
            string[] directories = Directory.GetDirectories(directory);
            foreach (var dir in directories)
            {
                bool skip = false;
                foreach (var ign in s_IgnoredDirectories)
                {
                    if (dir.EndsWith(ign))
                    {
                        skip = true;
                        break;
                    }
                }
                if (skip == false)
                {
                    loc += GetLOCCountFromDirectory(dir);
                }
            }
            if (loc > 0)
            {
                Console.ForegroundColor = ConsoleColor.Yellow;
                Console.WriteLine("[[Dir: {0}]]: {1} lines.", directory, loc);
            }
            return loc;
        }

        public static string? FindDirectoryWithFile(string startDirectory, string fileName)
        {
            string filePath = Path.Combine(startDirectory, fileName);

            if (File.Exists(filePath))
            {
                return startDirectory; // File found, return the directory
            }

            // If the directory has no parent, stop
            DirectoryInfo? parentDirectory = Directory.GetParent(startDirectory);
            if (parentDirectory == null)
            {
                return null; // No more parent directories to check
            }

            // Recursively search in the parent directory
            return FindDirectoryWithFile(parentDirectory.FullName, fileName);
        }
    }
}
