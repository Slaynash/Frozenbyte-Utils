using System.Text;

namespace FrozenbyteFDBUtils
{
    internal class FDBFile
    {
        internal byte[] rawdata;
        internal FDBFileInfo[] data;
        private Dictionary<string, FDBFileInfo> datamap;

        // FDB File Format:
        //   uint32        entry count
        //   FDBFileData[] entries
        //   <EOF>

        // FDBFileData Format:
        //   NT char[]  original file name
        //   NT char[]  target file name
        //   byte[5]    <unknown>
        //   uint32     ? offset or -1
        //   uint64     ? nonce

        public FDBFileInfo GetFileInfos(string name)
            => datamap.GetValueOrDefault(name);

        public FDBFile(string path)
        {
            byte[] filerawdata = File.ReadAllBytes(path);
            rawdata = new byte[filerawdata.Length - 12];
            Array.Copy(filerawdata, 12, rawdata, 0, rawdata.Length);

            FileUtils.Decrypt(BitConverter.ToUInt64(filerawdata, 4), rawdata);

            int expectedSize = BitConverter.ToInt32(rawdata, 0);

            int offset = 4;

            List<FDBFileInfo> datatmp = new List<FDBFileInfo>();
            datamap = new Dictionary<string, FDBFileInfo>();

            while (offset < rawdata.Length)
            {
                byte[] unknowndata = new byte[5];

                string targetfile = ReadString(rawdata, ref offset);
                string sourcefile = ReadString(rawdata, ref offset);
                Array.Copy(rawdata, offset, unknowndata, 0, 5); offset += 5;
                int dataoffset = BitConverter.ToInt32(rawdata, offset); offset += 4;
                int datalength = BitConverter.ToInt32(rawdata, offset); offset += 4;
                ulong nonce = BitConverter.ToUInt64(rawdata, offset); offset += 8;

                FDBFileInfo fileinfo = new FDBFileInfo(sourcefile, targetfile, unknowndata, dataoffset, datalength, nonce);
                datatmp.Add(fileinfo);
                datamap.Add(targetfile, fileinfo);
            }

            data = datatmp.ToArray();

            if (data.Length != expectedSize)
                Console.WriteLine($"Warning: File data size and expected size mismatch: expected {expectedSize}, got {data.Length}.");
        }

        private static string ReadString(byte[] buffer, ref int offset)
        {
            int endoffset = Array.IndexOf(buffer, (byte)0x00, offset);
            if (endoffset == -1)
                throw new IOException($"String never ends (starting index: {offset:x}");

            string ret = Encoding.UTF8.GetString(buffer, offset, endoffset - offset);

            offset = endoffset + 1;

            return ret;
        }
    }
}
