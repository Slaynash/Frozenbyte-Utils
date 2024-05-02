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
    public struct VC2
    {
        public float x, y;

        public VC2(ReadOnlySpan<byte> span)
        {
            x = BinaryPrimitives.ReadSingleLittleEndian(span[0..]);
            y = BinaryPrimitives.ReadSingleLittleEndian(span[4..]);
        }

        public override string ToString()
        {
            return $"VC3({x},{y})";
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    [Serializable]
    public struct VC3
    {
        public float x, y, z;

        public VC3(ReadOnlySpan<byte> span)
        {
            x = BinaryPrimitives.ReadSingleLittleEndian(span[0..]);
            y = BinaryPrimitives.ReadSingleLittleEndian(span[4..]);
            z = BinaryPrimitives.ReadSingleLittleEndian(span[8..]);
        }

        public override string ToString()
        {
            return $"VC3({x},{y},{z})";
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    [Serializable]
    public struct QUAT
    {
        public float x, y, z, w;

        public QUAT(ReadOnlySpan<byte> span)
        {
            x = BinaryPrimitives.ReadSingleLittleEndian(span[0..]);
            y = BinaryPrimitives.ReadSingleLittleEndian(span[4..]);
            z = BinaryPrimitives.ReadSingleLittleEndian(span[8..]);
            w = BinaryPrimitives.ReadSingleLittleEndian(span[12..]);
        }

        public override string ToString()
        {
            return $"QUAT({x},{y},{z},{w})";
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    [Serializable]
    public struct COL
    {
        public byte r, g, b;

        public COL(ReadOnlySpan<byte> span)
        {
            r = span[0];
            g = span[1];
            b = span[2];
        }

        public override string ToString()
        {
            return $"COL({r},{g},{b})";
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    [Serializable]
    public struct UV
    {
        public float u, v;

        public UV(ReadOnlySpan<byte> span)
        {
            u = BinaryPrimitives.ReadSingleLittleEndian(span[0..]);
            v = BinaryPrimitives.ReadSingleLittleEndian(span[4..]);
        }

        public override string ToString()
        {
            return $"UV({u},{v})";
        }
    }
}
