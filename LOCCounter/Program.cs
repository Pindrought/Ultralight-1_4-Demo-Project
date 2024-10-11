//This is just a program I put together to analyze the line of code breakdown in the project.
//It isn't doing anything for the demo.

using LOCCounter;

string executablePath = System.Reflection.Assembly.GetExecutingAssembly().Location;

//Need to determine sln directory by checking every directory with this executable and moving up
string slnName = "Ultralight 1_4 Demo Project.sln";
string? slnDirectory = DirectoryAnalyzer.FindDirectoryWithFile(executablePath, slnName);
if (slnDirectory == null)
{
    Console.ForegroundColor = ConsoleColor.Red;
    Console.WriteLine("Failed to identify solution directory to analyze.");
}
else
{
    try
    {
        List<string> arr = new List<string>();
        DirectoryAnalyzer.s_IncludedExtensions.AddRange([".cpp", ".h", ".inl"]);
        DirectoryAnalyzer.s_IgnoredDirectories.AddRange(["ThirdParty", ".vs", ".git", "LOCCounter", "Release", "Debug"]);

        int loc = DirectoryAnalyzer.GetLOCCountFromDirectory(slnDirectory);
        Console.ForegroundColor = ConsoleColor.Green;
        Console.WriteLine("Total lines: {0}.", loc);

    }
    catch (Exception ex)
    {
        Console.WriteLine(ex.ToString());
    }
}

Console.ReadLine();