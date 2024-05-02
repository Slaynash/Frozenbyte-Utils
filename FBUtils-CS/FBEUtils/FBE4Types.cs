using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace FrozenbyteFBEUtils
{

    [StructLayout(LayoutKind.Sequential)]
    [Serializable]
    public struct FBE4Guid
    {
        public uint identifier;

        public uint b; // ???
        public byte c; // ???

        // public byte d; // ??? - 02, sometimes 01
        // public ushort e; // ??? - always 0x0077 (?)
        // public byte f; // ??? - always 0x00 (?)
        public byte type;
        // public ushort h; // ??? - always 0x0000 (?)

        public FBE4Guid(ReadOnlySpan<byte> span)
        {
            identifier = BinaryPrimitives.ReadUInt32LittleEndian(span);

            b = BinaryPrimitives.ReadUInt32LittleEndian(span.Slice(4));
            c = span[8];


            type = span[13];



            ushort e = BinaryPrimitives.ReadUInt16LittleEndian(span.Slice(10));
            // if (e != 0x0077)
            //    throw new IOException($"Unexpected bytes [10;11]: expected 0x0077, got 0x{e:X4}");

            byte f = span[12];
            // if ((type == 0x33 && f != 0xA7) || (type != 0x33 && f != 0x00))
            //    throw new IOException($"Unexpected byte 12: expected {(type == 0x33 ? "0xA7" : "0x00")}, got 0x{f:X2}");

            if (type != 0x33 && type != 0x44 && type != 0x55) // Instance / Resource / Type
                throw new IOException($"Unknown guid type {type:X2}");

            byte d = span[9];
            // if (/*(type == 0x44 && d != 0x01) || */(type != 0x44 && d != 0x02))
            //    throw new IOException($"Unexpected byte 9: expected {(type == 0x44 ? "0x01" : "0x02")}, got 0x{d:X2}");

            ushort h = BinaryPrimitives.ReadUInt16LittleEndian(span.Slice(14));
            if (h != 0x0000)
                throw new IOException($"Unexpected byte [14;15]: expected 0x0000, got 0x{h:X4}");


            if (type == 0x44 && (b != 0x00000000 || c != 0x00))
                throw new IOException($"Invalid data: expected 0x0000000000, got {c:X2}{b:X8}");
            if (type == 0x33 && (b == 0x00000000 && c == 0x00))
                throw new IOException($"Invalid data: expected 0x0000000000, got {c:X2}{b:X8}");
        }

        public override string ToString()
        {
            string guidType = type switch
            {
                0x33 => "Instance",
                0x44 => "Resource",
                0x55 => "Type",
                _ => $"0x{type:X2}"
            };

            if (type == 0x44)
                return $"[{identifier:X8}]";
            else
                return $"[id: {identifier:X8}, data: 0x{c:X2}{b:X8}, type: {guidType}]";
        }

        public string ToString(Dictionary<FBE4Guid, int> typeNameTable, string[] stringTable)
        {
            if (typeNameTable.TryGetValue(this, out int stringIndex))
                return stringTable[stringIndex];

            string guidType = type switch
            {
                0x33 => "Instance",
                0x44 => "Resource",
                0x55 => "Type",
                _ => $"0x{type:X2}"
            };

            if (type == 0x44)
                return $"[{identifier:X8}]";
            else
                return $"[id: {identifier:X8}, data: 0x{c:X2}{b:X8}, type: {guidType}]";
        }
    }


    public struct FBE4Component
    {
        public FBE4Guid typeId;
        public FBE4Guid instOrResId;
        public FBE4Component[] subComponents;
        public FBE4Property[] properties;
        public FBE4Component[] children;
        public int unknownByteCount;
    }

    public struct FBE4Property
    {
        public int nameStringIndex;
        public byte type;
        public object value;

        public FBE4Property(int nameStringIndex, byte type, object value)
        {
            this.nameStringIndex = nameStringIndex;
            this.type = type;
            this.value = value;
        }

        public string ToString(string[] stringTable)
        {
            string name = stringTable[nameStringIndex];
            string valueString = type switch
            {
                0x12 => stringTable[(int)value],
                _ => value.ToString() ?? "<NULL>"
            };

            return $"{name}: {valueString}";
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    [Serializable]
    public struct FBE4ResEntry
    {
        public FBE4Guid guid;
        public uint nameIndex;
        public uint nameIndex2;
    }

    [StructLayout(LayoutKind.Sequential)]
    [Serializable]
    public struct FBE4TypeEntry
    {
        public FBE4Guid guid;
        public uint nameIndex;
    }
}
