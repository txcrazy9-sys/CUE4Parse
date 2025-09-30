using System.Runtime.CompilerServices;

namespace CUE4Parse.ACL
{
    public struct RawBufferHeader
    {
        public uint Size;
        public uint Hash;
    }

    public struct TracksHeader
    {
        public uint Tag;
        public ushort Version;
        public byte AlgorithmType;
        public byte TrackType;
        public uint NumTracks;
        public uint NumSamples;
        public float SampleRate;
        public uint MiscPacked;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool GetHasScale() => (MiscPacked & 1) != 0;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public int GetDefaultScale() => (int) (MiscPacked >> 1) & 1;
    }

    public struct ClipHeader
    {
        public ushort NumBones;
        public ushort NumSegments;
        public byte RotationFormat;
        public byte TranslationFormat;
        public byte ScaleFormat;
        public byte ClipRangeReduction;
        public byte SegmentRangeReduction;
        public byte HasScale;
        public byte DefaultScale;
        public byte Padding;
        public uint NumSamples;
        public float SampleRate;
        // Note: We only expose the basic fields needed for header information
        // The offset fields are not needed for basic header access
    }
}
