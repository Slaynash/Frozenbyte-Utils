using System;
using System.Buffers.Binary;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Text;
using ZstdNet;

namespace FrozenbyteFBEUtils
{
    public class Program
    {
        public const string starbasepath = @"D:\SteamLibrary\steamapps\common\Starbase\";
        public static readonly string appdata = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
        public static bool dontSaveFiles = false;

        public static void Main(string[] args)
        {
            Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;
            Thread.CurrentThread.CurrentUICulture = CultureInfo.InvariantCulture;

            //string sourcefile = Path.Combine(starbasepath, @"cs_extract\data\mission\default\default.fbe");
            //string sourcefile = Path.Combine(starbasepath, @"cs_extract\data\mission\station_origin_01\station_origin_01.fbe");
            //string sourcefile = Path.Combine(starbasepath, @"D:\shadwen_editor\data\mission\editor_tutorial\01_getting_started\01_getting_started.fbe.layer_custom2.fbel");
            //string sourcefile = Path.Combine(starbasepath, @"D:\shadwen_editor\data\mission\testmap4\testmap4.fbe");
            //Editor4ExtractMap(@"D:\shadwen_editor\data\mission", "anna_01_castle_wall_city_walls");
            //Editor4ExtractMap(@"D:\shadwen_editor\data\mission", "testmap5");
            //Editor5ExtractMap(Path.Combine(starbasepath, @"cs_extract\data\mission"), "station_origin_01");
            //Editor5ExtractMap(Path.Combine(starbasepath, @"cs_extract\data\mission"), "mainmenu");
            //Editor4ExtractMap(@"D:\shadwen_editor\data\mission", "testmap6");

            //Editor5ExtractLayerPacked(Path.Combine(appdata, @"Starbase\ssc\autosave\ship_blueprints\ship_50.fbe.decrypted"), Path.Combine(appdata, @"Starbase\ssc\autosave\ship_blueprints\ship_50.fbe.extracted"));
            //Editor5ExtractLayerPacked(Path.Combine(appdata, @"Starbase\ssc\user_box_layout_ship.lua"), Path.Combine(appdata, @"Starbase\ssc\user_box_layout_ship.lua.extracted"));
            ExtractPackedZStd(Path.Combine(appdata, @"Starbase\blueprints\2681\19457\1.fbe.decrypted"), Path.Combine(appdata, @"Starbase\blueprints\2681\19457\1.fbe.extracted"));
        }

        private static void Editor5ExtractMap(string missionFolder, string folderName)
        {
            string targetFolder = Path.Combine(missionFolder, folderName + "_extracted");
            string sourceFolder = Path.Combine(missionFolder, folderName);
            if (!sourceFolder.EndsWith('/') && !sourceFolder.EndsWith('\\'))
                sourceFolder += "\\";

            if (!dontSaveFiles)
            {
                if (Directory.Exists(targetFolder))
                    Directory.Delete(targetFolder, true);
                Directory.CreateDirectory(targetFolder);
            }

            foreach (string file in Directory.GetFiles(sourceFolder, "*.fbe"))
            {
                string filename = file.Substring(sourceFolder.Length);
                Editor5ExtractLayerPacked(file, Path.Combine(targetFolder, filename));
            }
        }


        private static void Editor4ExtractMap(string missionFolder, string folderName)
        {
            string targetFolder = Path.Combine(missionFolder, folderName + "_extracted");
            string sourceFolder = Path.Combine(missionFolder, folderName);
            if (!sourceFolder.EndsWith('/') && !sourceFolder.EndsWith('\\'))
                sourceFolder += "\\";

            if (!dontSaveFiles)
            {
                if (Directory.Exists(targetFolder))
                    Directory.Delete(targetFolder, true);
                Directory.CreateDirectory(targetFolder);
            }

            foreach (string file in Directory.GetFiles(sourceFolder, "*.fbe*").Where(f => f.EndsWith(".fbe") || f.EndsWith(".fbel")))
            {
                string filename = file.Substring(sourceFolder.Length);
                Editor4ExtractLayer(file, Path.Combine(targetFolder, filename));
            }
        }


        private static void ExtractPackedZStd(string sourceFile, string targetFile)
        {
            byte[] data = File.ReadAllBytes(sourceFile);
            ExtractPackedZStd(sourceFile, targetFile, ref data);
        }
        private static void ExtractPackedZStd(string sourceFile, string targetFile, ref byte[] data)
        {
            // Frozenbyte Editor file Z
            if (data.Length > "PACKED_ZSTD".Length &&
                data[0] == 'P' &&
                data[1] == 'A' &&
                data[2] == 'C' &&
                data[3] == 'K' &&
                data[4] == 'E' &&
                data[5] == 'D' &&
                data[6] == '_' &&
                data[7] == 'Z' &&
                data[8] == 'S' &&
                data[9] == 'T' &&
                data[10] == 'D'
            )
            {
                // Copy our compressed data to a tmp buffer
                byte[] datatmp = new byte[data.Length - "PACKED_ZSTD".Length - 4];
                Array.Copy(data, 15, datatmp, 0, data.Length - 15);
                // Realloc the passed buffer to the decompressed size
                data = new byte[BitConverter.ToUInt32(data, "PACKED_ZSTD".Length)];
                // Decompress our tmp buffer to our passed buffer
                using Decompressor decompressor = new Decompressor();
                int uncompressedSizeActual = decompressor.Unwrap(datatmp, data, 0, false);
                if (data.Length != uncompressedSizeActual)
                    throw new IOException("Uncompressed expected and actual size mismatch");

                if (!dontSaveFiles)
                {
                    byte[] filedata = new byte[uncompressedSizeActual + "PACKED_DATA".Length];
                    Encoding.UTF8.GetBytes("PACKED_DATA", 0, "PACKED_DATA".Length, filedata, 0);
                    Array.Copy(data, 0, filedata, "PACKED_DATA".Length, uncompressedSizeActual);

                    File.WriteAllBytes(targetFile, filedata);
                }
            }
            else
            {
                if (!dontSaveFiles)
                    File.Copy(sourceFile, targetFile, true);
            }
        }

        private static void Editor4ExtractLayer(string sourceFile, string targetFile)
        {
            byte[] data = File.ReadAllBytes(sourceFile);

            if (sourceFile.EndsWith(".fbe"))
                Console.WriteLine("Layer: Default");
            else
            {
                string layername = sourceFile.Substring(sourceFile.IndexOf(".fbe.") + 5);
                if (layername.StartsWith("layer_"))
                    Console.WriteLine("Layer: " + layername.Substring(6, layername.Length - 6 - 5));
                else
                    Console.WriteLine("Layer: " + layername.Substring(0, layername.Length - 5));
            }

            ExtractPackedZStd(sourceFile, targetFile, ref data);

            DebugLogFBE(data);
        }

        private static void Editor5ExtractLayerPacked(string sourceFile, string targetFile)
        {
            byte[] data = File.ReadAllBytes(sourceFile);
            Console.WriteLine(sourceFile);

            ExtractPackedZStd(sourceFile, targetFile, ref data);

            DebugLogFBE(data);
        }


        private static void DebugLogFBE(byte[] data)
        {
            /*
            
            "dST\u0\u1"
                [4]  str_count
                szstring[str_count] strings
            
            [4]  0x1C
            time32_t save_time
            [4]  0x0
            "RsNa"
            [4]  0x3
            [4]  res_entry_count
            [4]  res_entries_total_length
            res_entry[res_entry_count] res_entries
            "TyNa"
            [4]  0x1
            [4]  type_entry_count
            type_entry[type_entry_count] type_entries

            */

            int eofOffset = data.Length;
            int offset = 0;

            int stringTableSize = 0;
            string[] stringTable = new string[0];

            int resTableSize = 0;
            FBE4ResEntry[] resTable = new FBE4ResEntry[0];

            int typeTableSize = 0;
            FBE4TypeEntry[] typeTable = new FBE4TypeEntry[0];

            DateTime localSaveTime;

            Console.WriteLine($"{BitConverter.ToInt32(data, offset):X8}");

            while (offset != eofOffset)
            {

                if (offset + 4 <= eofOffset && BitConverter.ToInt32(data, offset) == 0x00545364) // dST\x0
                {
                    offset += 4;

                    if (data[offset] != 0x01)
                        throw new InvalidDataException($"Invalid byte 0x{data[offset]:X2}. Only 0x01 is supported.");
                    offset++;

                    stringTableSize = BitConverter.ToInt32(data, offset);
                    offset += 4;

                    stringTable = new string[stringTableSize];
                    for (int i = 0; i < stringTableSize; ++i)
                        stringTable[i] = ReadString(data, ref offset);

                    Console.WriteLine("    String Table:");
                    for (int i = 0; i < stringTableSize; ++i)
                        Console.WriteLine($"        [{i:X4}] {stringTable[i]}");
                }

                if (offset + 4 <= eofOffset && BitConverter.ToInt32(data, offset) == 0x0000001C)
                {
                    offset += 4;

                    int saveTimeInt = ReadInt(data, ref offset);
                    localSaveTime = DateTimeOffset.FromUnixTimeSeconds(saveTimeInt).LocalDateTime;
                    Console.WriteLine("    Save Time (local): " + localSaveTime);

                    EnsureNextInt(data, ref offset, 0x00000000);
                    EnsureNextInt(data, ref offset, 0x614E7352); // RsNa
                    EnsureNextInt(data, ref offset, 0x00000003);
                
                    resTableSize = ReadInt(data, ref offset);
                    int resTableByteSize = ReadInt(data, ref offset);
                    if ((resTableSize == 0) != (resTableByteSize == 0))
                        throw new IOException($"Unexpected ressource count/length: expected 0/0, got {resTableSize}/{resTableByteSize}");
                    if (resTableByteSize != 0)
                    {
                        if (resTableByteSize / resTableSize != 24)
                            throw new IOException($"Unexpected ressource entry size: expected 24, got {resTableByteSize / resTableSize}");
                        resTable = new FBE4ResEntry[resTableSize];

                        for (int i = 0; i < resTableSize; ++i)
                        {
                            resTable[i].guid = new FBE4Guid(new ReadOnlySpan<byte>(data, offset, 16));
                            resTable[i].nameIndex = BitConverter.ToUInt32(data, offset + 16);
                            resTable[i].nameIndex2 = BitConverter.ToUInt32(data, offset + 20);
                            if (resTable[i].nameIndex != resTable[i].nameIndex2)
                                throw new IOException($"Unexpected mismatching name indices: {resTable[i].nameIndex:X8}/{resTable[i].nameIndex2:X8}");
                            offset += 24;
                        }
                    }

                    Console.WriteLine("    Ressource Table:");
                    for (int i = 0; i < resTableSize; ++i)
                        Console.WriteLine($"        [{i:X4}] {resTable[i].guid} {stringTable[resTable[i].nameIndex]}");

                    EnsureNextInt(data, ref offset, 0x614E7954); // RsNa
                    EnsureNextInt(data, ref offset, 0x00000001);

                    typeTableSize = ReadInt(data, ref offset);
                    typeTable = new FBE4TypeEntry[typeTableSize];

                    for (int i = 0; i < typeTableSize; ++i)
                    {
                        typeTable[i].guid = new FBE4Guid(new ReadOnlySpan<byte>(data, offset, 16));
                        typeTable[i].nameIndex = BitConverter.ToUInt32(data, offset + 16);
                        offset += 20;
                    }

                    Console.WriteLine("    Type Table:");
                    for (int i = 0; i < typeTableSize; ++i)
                    {
                        if (typeTable[i].guid.type != 0x55)
                            throw new ArgumentException($"Unexpected guid type 0x{typeTable[i].guid.type:X2} while enumerating type table (expected: 0x55)");
                        if (typeTable[i].guid.c != 0x00 || typeTable[i].guid.b != 0x00000000)
                            Console.WriteLine($"        [{i:X4}] [{typeTable[i].guid.identifier:X8}] {{0x{typeTable[i].guid.c:X2}{typeTable[i].guid.b:X8}}} {stringTable[typeTable[i].nameIndex]}");
                        else
                            Console.WriteLine($"        [{i:X4}] [{typeTable[i].guid.identifier:X8}] {{            }} {stringTable[typeTable[i].nameIndex]}");
                    }
                }

                if (offset + 3 < eofOffset && data[offset + 0] == 0x00 && data[offset + 1] == 0x00 && data[offset + 2] == 0x00)
                {
                    offset += 3;

                    int componentCount = ReadInt(data, ref offset);
                    Console.WriteLine("    Nodes:");

                    for (int i = 0; i < componentCount; ++i)
                        Editor4ParseComponent(data, ref offset, stringTable, i, firstComponent: i == 0);
                }

                break;
            }

            int unknownDataLength = eofOffset - offset;

            if (unknownDataLength != 0)
                Console.WriteLine($"    {{{unknownDataLength} unknown bytes}}: " /*+ BitConverter.ToString(data, offset, unknownDataLength)*/);


        }

        private static FBE4Component Editor4ParseComponent(byte[] data, ref int offset, string[] stringTable, int index, bool subComponent = false, bool firstComponent = false, int logdepth = 0)
        {
            // format:
            //  Guid : Type
            //  u8  ?: ??? (only when sub-component, only encountered '0x00' so far)
            //  Guid : Id
            //  u32 ?: ??? (only first? component)
            //  u32  : DataSize
            //  u8   : ??? (somehow equivalent to DataSize?)
            //  u8   : SubComponentCount
            //  C[]  : SubComponents (array of sub-Components)
            //  u8   : PropertyCount
            //  P[]  : Properties (array of Property)
            //  u32  : ChildrenCount
            //  C[]  : Children (array of Component)


            FBE4Guid typeId = new FBE4Guid(new ReadOnlySpan<byte>(data, offset, 16));
            offset += 16;

            if (subComponent)
            {
                byte unkchild0 = data[offset++];
                if (unkchild0 != 0x00)
                    throw new NotImplementedException($"Child component inter-byte has not been implemented yet (offset: 0x{(offset-1):X2})");
            }

            FBE4Guid instOrResId = new FBE4Guid(new ReadOnlySpan<byte>(data, offset, 16));
            offset += 16;

            int componentDataLength;
            if (firstComponent)
            {
                int unknownSize = ReadInt(data, ref offset);
                componentDataLength = ReadInt(data, ref offset);
                if (unknownSize - 4 != componentDataLength)
                    throw new NotImplementedException($"1st component sizes don't have a difference of 4: 0x{unknownSize:X8}, 0x{componentDataLength:X8}");
            }
            else
            {
                componentDataLength = ReadInt(data, ref offset);
                int possibleSameSize = BitConverter.ToInt32(data, offset);
                if (componentDataLength - 4 == possibleSameSize)
                    throw new NotImplementedException($"Non-1st component have 2 sizes fields rather than one");
            }

            Console.WriteLine(new string(' ', 8 + logdepth * 8) + $"[{index:X2}] {typeId} {instOrResId} (size: {componentDataLength})");

            int startOffset = offset;

            byte unkByte = data[offset++];
            if (unkByte != 0x00)
                throw new NotImplementedException($"Component 1st byte data has not been implemented yet (offset: 0x{(offset-1):X2})");


            byte subCount = data[offset++];
            FBE4Component[] subComponents = new FBE4Component[subCount];
            if (subCount > 0)
                Console.WriteLine(new string(' ', 12 + logdepth * 8) + $"Sub-Components ({subCount}):");
            else
                Console.WriteLine(new string(' ', 12 + logdepth * 8) + $"Sub-Components (0)");
            for (int i = 0; i < subCount; ++i)
                subComponents[i] = Editor4ParseComponent(data, ref offset, stringTable, i, logdepth: logdepth + 1, subComponent: true);


            byte propertycount = data[offset++];
            FBE4Property[] properties = new FBE4Property[propertycount];
            if (propertycount > 0)
                Console.WriteLine(new string(' ', 12 + logdepth * 8) + $"Properties ({propertycount}):");
            else
                Console.WriteLine(new string(' ', 12 + logdepth * 8) + $"Properties (0)");
            for (int i = 0; i < propertycount; ++i)
            {
                properties[i] = Editor4ParseProperty(data, ref offset);
                Console.WriteLine(new string(' ', 16 + logdepth * 8) + $"[{i:X2}] {properties[i].ToString(stringTable)}");
            }


            int childrenCount = ReadInt(data, ref offset);
            FBE4Component[] children = new FBE4Component[childrenCount];
            if (childrenCount > 0)
                Console.WriteLine(new string(' ', 12 + logdepth * 8) + $"Children ({childrenCount}):");
            else
                Console.WriteLine(new string(' ', 12 + logdepth * 8) + $"Children (0)");
            for (int i = 0; i < childrenCount; ++i)
                children[i] = Editor4ParseComponent(data, ref offset, stringTable, i, logdepth: logdepth + 1);





            int unknownByteCount = (startOffset + componentDataLength) - offset;

            if (unknownByteCount != 0)
            {
                Console.WriteLine(new string(' ', 12 + logdepth * 8) + $"{{{unknownByteCount} unknown bytes}}: " /*+ BitConverter.ToString(data, offset, unknownByteCount)*/);
                offset += unknownByteCount;
            }

            return new FBE4Component()
            {
                typeId = typeId,
                instOrResId = instOrResId,
                subComponents = subComponents,
                properties = properties,
                children = children,
                unknownByteCount = unknownByteCount
            };
        }

        private static FBE4Property Editor4ParseProperty(byte[] data, ref int offset)
        {
            FBE4Property value;

            int nameStringIndex = ReadInt(data, ref offset);

            byte type = data[offset++];

            switch (type)
            {
                case 0x01: // int8 (?)
                    if (data[offset++] != 0x01)
                        throw new IOException($"Unexpected size for int8 data: expected 0x01, got 0x{data[offset - 1]:X2}");
                    value = new FBE4Property(nameStringIndex, 0x01, data[offset++]);
                    break;

                case 0x05: // int16 (?)
                    if (data[offset++] != 0x02)
                        throw new IOException($"Unexpected size for int16 data: expected 0x02, got 0x{data[offset - 1]:X2}");
                    value = new FBE4Property(nameStringIndex, 0x05, ReadShort(data, ref offset));
                    break;

                case 0x06: // int32 (?)
                    if (data[offset++] != 0x04)
                        throw new IOException($"Unexpected size for int32 data: expected 0x04, got 0x{data[offset - 1]:X2}");
                    value = new FBE4Property(nameStringIndex, 0x06, ReadInt(data, ref offset));
                    break;

                case 0x08: // single
                    if (data[offset++] != 0x04)
                        throw new IOException($"Unexpected size for single data: expected 0x04, got 0x{data[offset - 1]:X2}");
                    value = new FBE4Property(nameStringIndex, 0x06, ReadSingle(data, ref offset));
                    break;

                case 0x0A: // Guid
                case 0x0B: // Guid - Shadwen/Starbase: 16/8
                    if (data[offset++] != 0x10)
                        throw new IOException($"Unexpected size for Guid data: expected 0x10, got 0x{data[offset - 1]:X2}");
                    value = new FBE4Property(nameStringIndex, 0x0B, new FBE4Guid(new ReadOnlySpan<byte>(data, offset, 16)));
                    offset += 16;
                    break;

                case 0x0C: // VC2
                    if (data[offset++] != 0x08)
                        throw new IOException($"Unexpected size for VC2 data: expected 0x08, got 0x{data[offset - 1]:X2}");
                    value = new FBE4Property(nameStringIndex, 0x0D, new VC2(new ReadOnlySpan<byte>(data, offset, 8)));
                    offset += 8;
                    break;

                case 0x0D: // VC3
                    if (data[offset++] != 0x0C)
                        throw new IOException($"Unexpected size for VC3 data: expected 0x0C, got 0x{data[offset - 1]:X2}");
                    value = new FBE4Property(nameStringIndex, 0x0D, new VC3(new ReadOnlySpan<byte>(data, offset, 12)));
                    offset += 12;
                    break;

                case 0x0F: // QUAT
                    if (data[offset++] != 0x10)
                        throw new IOException($"Unexpected size for QUAT data: expected 0x10, got 0x{data[offset - 1]:X2}");
                    value = new FBE4Property(nameStringIndex, 0x10, new QUAT(new ReadOnlySpan<byte>(data, offset, 16)));
                    offset += 16;
                    break;

                case 0x10: // COL
                    if (data[offset++] != 0x03)
                        throw new IOException($"Unexpected size for COL data: expected 0x03, got 0x{data[offset - 1]:X2}");
                    value = new FBE4Property(nameStringIndex, 0x03, new COL(new ReadOnlySpan<byte>(data, offset, 3)));
                    offset += 3;
                    break;

                // If the size is >0x20 (based on Starbase code), then it contains additional data

                case 0x11: // LayerIndex
                    if (BitConverter.ToInt16(data, offset) != 0x0081)
                        throw new IOException($"Unexpected size for Layer data: expected 0x0081, got 0x{BitConverter.ToInt16(data, offset):X4}");
                    offset += 2;
                    value = new FBE4Property(nameStringIndex, 0x11, data[offset++]);
                    break;

                case 0x12: // StringIndex
                    if (BitConverter.ToInt16(data, offset) != 0x0084)
                        throw new IOException($"Unexpected size for StringIndex data: expected 0x0084, got 0x{BitConverter.ToInt16(data, offset):X4}");
                    offset += 2;
                    value = new FBE4Property(nameStringIndex, 0x12, ReadInt(data, ref offset));
                    break;

                case 0x15: // ???
                    if (BitConverter.ToInt16(data, offset) != 0x0084)
                        throw new IOException($"Unexpected info for ??? data: expected 0x0084, got 0x{BitConverter.ToInt16(data, offset):X4}");
                    offset += 2;
                    if (ReadInt(data, ref offset) != 0x00000000)
                        throw new IOException($"unimplemented type 0x15 has data");

                    value = new FBE4Property(nameStringIndex, 0x15, "NULL");
                    break;

                case 0x17: // Array: Guid
                {
                    short length = (short)(ReadShort(data, ref offset) - 0x80);
                    int elementCount = ReadInt(data, ref offset);
                    // if (elementCount * 16 + 4 != length)
                    //     throw new IOException($"length and size mismatch for Guid[] data");
                    FBE4Guid[] elements = new FBE4Guid[elementCount];
                    for (int i = 0; i < elementCount; i++)
                    {
                        elements[i] = new FBE4Guid(new ReadOnlySpan<byte>(data, offset, 16));
                        offset += 16;
                    }
                    value = new FBE4Property(nameStringIndex, 0x17, elements);
                    break;
                }

                case 0x18: // Array: UV
                {
                    short length = (short)(ReadShort(data, ref offset) - 0x80);
                    int elementCount = ReadInt(data, ref offset);
                    // if (elementCount * 8 + 4 != length)
                    //    throw new IOException($"length and size mismatch for UV[] data");
                    UV[] elements = new UV[elementCount];
                    for (int i = 0; i < elementCount; i++)
                    {
                        elements[i] = new UV(new ReadOnlySpan<byte>(data, offset, 8));
                        offset += 8;
                    }
                    value = new FBE4Property(nameStringIndex, 0x18, elements);
                    break;
                }

                case 0x19: // Array: VC3
                {
                    short length = (short)(ReadShort(data, ref offset) - 0x80);
                    int elementCount = ReadInt(data, ref offset);
                    // if (elementCount * 12 + 4 != length)
                    //     throw new IOException($"length and size mismatch for VC3[] data");
                    VC3[] elements = new VC3[elementCount];
                    for (int i = 0; i < elementCount; i++)
                    {
                        elements[i] = new VC3(new ReadOnlySpan<byte>(data, offset, 12));
                        offset += 12;
                    }
                    value = new FBE4Property(nameStringIndex, 0x19, elements);
                    break;
                }

                case 0x1B: // Array: QUAT
                {
                    short length = (short)(ReadShort(data, ref offset) - 0x80);
                    int elementCount = ReadInt(data, ref offset);
                    // if (elementCount * 16 + 4 != length)
                    //    throw new IOException($"length and size mismatch for QUAT[] data");
                    QUAT[] elements = new QUAT[elementCount];
                    for (int i = 0; i < elementCount; i++)
                    {
                        elements[i] = new QUAT(new ReadOnlySpan<byte>(data, offset, 16));
                        offset += 16;
                    }
                    value = new FBE4Property(nameStringIndex, 0x1B, elements);
                    break;
                }

                case 0x1D: // Array: int32
                {
                    short length = (short)(ReadShort(data, ref offset) - 0x80);
                    int elementCount = ReadInt(data, ref offset);
                    // if (elementCount * 4 + 4 != length)
                    //    throw new IOException($"length and size mismatch for UV[] data");
                    int[] elements = new int[elementCount];
                    for (int i = 0; i < elementCount; i++)
                        elements[i] = ReadInt(data, ref offset);
                    value = new FBE4Property(nameStringIndex, 0x18, elements);
                    break;
                }

                default:
                    throw new NotImplementedException($"Property type 0x{type:X2} (name index: {nameStringIndex:X}) has not been implemented. Next 3 bytes: {BitConverter.ToString(data, offset, 3)}");
            }

            return value;
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

        private static byte ReadByte(byte[] buffer, ref int offset)
        {
            if (offset + 4 > buffer.Length)
                throw new IOException($"Not enough data to read int");

            byte ret = buffer[offset];
            offset += 1;
            return ret;
        }

        private static short ReadShort(byte[] buffer, ref int offset)
        {
            if (offset + 2 > buffer.Length)
                throw new IOException($"Not enough data to read int");

            short ret = BitConverter.ToInt16(buffer, offset);
            offset += 2;
            return ret;
        }

        private static int ReadInt(byte[] buffer, ref int offset)
        {
            if (offset + 4 > buffer.Length)
                throw new IOException($"Not enough data to read int");

            int ret = BitConverter.ToInt32(buffer, offset);
            offset += 4;
            return ret;
        }

        private static float ReadSingle(byte[] buffer, ref int offset)
        {
            if (offset + 4 > buffer.Length)
                throw new IOException($"Not enough data to read int");

            float ret = BitConverter.ToSingle(buffer, offset);
            offset += 4;
            return ret;
        }


        private static void EnsureNextInt(byte[] buffer, ref int offset, int expected)
        {
            int read = ReadInt(buffer, ref offset);
            if (read != expected)
                throw new IOException($"Unexpected int while reading data: expected {expected:X8}, got {read:X8}.");
        }
    }
}