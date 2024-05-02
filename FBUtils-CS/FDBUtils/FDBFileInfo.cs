using ZstdNet;

namespace FrozenbyteFDBUtils
{
    internal class FDBFileInfo
    {
        public string sourcefile;
        public string targetfile;
        public byte[] unknowndata;
        public int dataoffset;
        public int datalength;
        public ulong nonce;

        public FDBFileInfo(string sourcefile, string targetfile, byte[] unknowndata, int dataoffset, int datalength, ulong nonce)
        {
            this.sourcefile = sourcefile;
            this.targetfile = targetfile;
            this.unknowndata = unknowndata;
            this.dataoffset = dataoffset;
            this.datalength = datalength;
            this.nonce = nonce;
        }

        public byte[] LoadData(bool decompress = true)
        {
            if (dataoffset > 0 && datalength <= 0)
                throw new NotSupportedException($"Partial external file not supported (file: {sourcefile}, offset: {dataoffset}, length: {datalength})");

            string sourcefilepath = Path.Combine(Program.starbasepath, sourcefile);
            byte[] data;

            if (datalength != -1)
            {
                using BinaryReader br = new BinaryReader(new FileStream(sourcefilepath, FileMode.Open, FileAccess.Read));
                br.BaseStream.Position = dataoffset;
                data = br.ReadBytes(datalength);
            }
            else
                data = File.ReadAllBytes(sourcefilepath);

            FileUtils.Decrypt(nonce, data);

            if (data.Length >= 4 && BitConverter.ToUInt32(data, 0) == 0xFD2FB528) // Zstd-compressed file
            {
                using Decompressor decompressor = new Decompressor();
                data = decompressor.Unwrap(data).ToArray();
            }

            if (!decompress)
                return data;

            // Frozenbyte Editor file
            if (data.Length > "PACKED_ZSTD".Length &&
                data[ 0] == 'P' &&
                data[ 1] == 'A' &&
                data[ 2] == 'C' &&
                data[ 3] == 'K' &&
                data[ 4] == 'E' &&
                data[ 5] == 'D' &&
                data[ 6] == '_' &&
                data[ 7] == 'Z' &&
                data[ 8] == 'S' &&
                data[ 9] == 'T' &&
                data[10] == 'D'
            ) {
                byte[] datatmp = new byte[data.Length - 15];
                Array.Copy(data, 15, datatmp, 0, data.Length - 15);
                data = new byte[BitConverter.ToUInt32(data, "PACKED_ZSTD".Length)];
                using Decompressor decompressor = new Decompressor();
                int uncompressedSizeActual = decompressor.Unwrap(datatmp, data, 0, false);
                if (data.Length != uncompressedSizeActual)
                    throw new IOException("Uncompressed expected and actual size mismatch");
            }

            return data;
        }
    }
}
