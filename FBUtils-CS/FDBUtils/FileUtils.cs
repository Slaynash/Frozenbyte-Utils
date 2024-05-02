using CSChaCha8;

namespace FrozenbyteFDBUtils
{
    internal class FileUtils
    {

        private static readonly uint[] key = new uint[] {
            0xAA00B54B      , 0xDC7A80D4      , 0x46ECCABE      , 0xCF6BFF50,
			0x56B03217      , 0x12308085      , 0xC556B032      , 0x9CDDC7E1,
			0x20F83EC5      , 0xCEB1AC2D      , 0xFF5DF51C      , 0xEF8980D9
        };

        internal static void Decrypt(ulong nonce, byte[] data)
        {
            if (data.Length <= 0)
                return;

            using (ChaCha8 chacha8 = new ChaCha8(key, new uint[] { (uint)nonce, (uint)(nonce >> 32) }, 0))
                chacha8.DecryptBytesInline(data);
        }
    }
}
