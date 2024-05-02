using System;
using System.Diagnostics;
using System.Text;

namespace FrozenbyteFDBUtils
{
    public class Program
    {
        public const string starbasepath = @"D:\SteamLibrary\steamapps\common\Starbase\";
        public static readonly byte[] PACKED_DATA_bytes = Encoding.UTF8.GetBytes("PACKED_DATA");

        public static void Main(string[] args)
        {
#if FALSE
            string filename = @"C:\Users\hugof\AppData\Roaming\Starbase\blueprints\2681\19064\1.fbe";
            byte[] filedata = File.ReadAllBytes(filename);
            ulong nonce = BitConverter.ToUInt64(filedata, 4);
            byte[] data = new byte[filedata.Length - 12];
            Array.Copy(filedata, 12, data, 0, data.Length);
            FileUtils.Decrypt(nonce, data);
            File.WriteAllBytes(filename.Substring(0, filename.Length - 4) + "_decrypteddata.fbe", data);
#endif

#if TRUE
            Console.WriteLine(Environment.CurrentDirectory);
            FDBFile fdb = new FDBFile(starbasepath + "fdb.bin");
            Console.WriteLine($"fdb contains {fdb.data.Length} entries.");

            //FDBFileInfo infos = fdb.GetFileInfos("data/script/init.lub");
            //if (infos.datalength == -1)
            //    Console.WriteLine($"{infos.targetfile} => {infos.sourcefile} (key: 0x{infos.nonce:X16}) {BitConverter.ToString(infos.unknowndata)}");
            //else
            //    Console.WriteLine($"{infos.targetfile} => {infos.sourcefile} @ {infos.dataoffset:X} (0x{infos.datalength:X} bytes) (key: 0x{infos.nonce:X16}) {BitConverter.ToString(infos.unknowndata)}");

            //infos = fdb.GetFileInfos("data/mission/spaceship_ssc_stackoverflow_starfire/spaceship_ssc_stackoverflow_starfire.fbe");
            //if (infos.datalength == -1)
            //    Console.WriteLine($"{infos.targetfile} => {infos.sourcefile} (key: 0x{infos.nonce:X16}) {BitConverter.ToString(infos.unknowndata)}");
            //else
            //    Console.WriteLine($"{infos.targetfile} => {infos.sourcefile} @ {infos.dataoffset:X} (0x{infos.datalength:X} bytes) (key: 0x{infos.nonce:X16}) {BitConverter.ToString(infos.unknowndata)}");

            int i = -1;
            foreach (FDBFileInfo info in fdb.data)
            {
                i++;

                //if (!info.targetfile.EndsWith(".fbe") && !info.targetfile.EndsWith(".fbel") /* && !info.targetfile.EndsWith(".lub") && !info.targetfile.EndsWith(".lua") */)
                //    continue;

                //if (!info.targetfile.EndsWith("station_origin_01.fbe"))
                //    continue;

                Console.WriteLine($"[{i} / {fdb.data.Length}] {info.targetfile}");

                string targetFile = Path.Combine(Program.starbasepath, "cs_extract", info.targetfile);
                Directory.CreateDirectory(Path.GetDirectoryName(targetFile));

                if (File.Exists(targetFile) && !info.targetfile.EndsWith(".lub"))
                    continue;

                // Filter, avoids having the full 70GB+ of files decompressed (remove at your own disk space risk)
                if (!info.targetfile.EndsWith(".lua") && !info.targetfile.EndsWith(".lub") && !info.targetfile.EndsWith(".bin") && !info.targetfile.EndsWith(".txt"))
                {
                    using FileStream fs = File.Create(targetFile);
                    continue;
                }

                byte[] filedata = info.LoadData(decompress: false);

                /* if decompressed
                if (info.targetfile.EndsWith(".fbe"))
                {
                    // write as loadable file
                    byte[] filedatatmp = new byte[PACKED_DATA_bytes.Length + filedata.Length];
                    Array.Copy(PACKED_DATA_bytes, filedatatmp, PACKED_DATA_bytes.Length);
                    Array.Copy(filedata, 0, filedatatmp, PACKED_DATA_bytes.Length, filedata.Length);
                    filedata = filedatatmp;
                }
                */

                File.WriteAllBytes(targetFile, filedata);

                if (info.targetfile.EndsWith(".lub"))
                {
                    using (Process unluac = new Process())
                    {
                        unluac.StartInfo.RedirectStandardInput = true;
                        unluac.StartInfo.RedirectStandardOutput = true;
                        unluac.StartInfo.CreateNoWindow = true;
                        unluac.StartInfo.UseShellExecute = false;
                        unluac.StartInfo.FileName = "cmd.exe";
                        unluac.StartInfo.Arguments = $"/c java -jar unluac_1_2_3_511.jar -o \"{targetFile + ".decompiled.lua"}\" \"{targetFile}\"";
                        unluac.Start();

                        unluac.WaitForExit();
                    }
                }

                if (info.targetfile.EndsWith(".fbe"))
                    GC.Collect();
            }


            //File.WriteAllBytes(@"D:\SteamLibrary\steamapps\common\Starbase\fdb.bin.csdecrypted", fdb.data);

            
#endif
        }
    }
}