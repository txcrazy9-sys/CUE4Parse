using System;
using System.Runtime.InteropServices;
using CUE4Parse.UE4.Objects.Core.Math;
using CUE4Parse.UE4.Assets.Exports.Animation;

namespace CUE4Parse.ACL
{
    public static class ACLNative
    {
        public const string LIB_NAME = "CUE4Parse-Natives";

        [DllImport(LIB_NAME)]
        public static extern IntPtr nAllocate(int size, int alignment = 16);

        [DllImport(LIB_NAME)]
        public static extern void nDeallocate(IntPtr ptr, int size);

        [DllImport(LIB_NAME)]
        public static extern IntPtr nCompressedTracks_IsValid(IntPtr handle, bool checkHash);

        [DllImport(LIB_NAME)]
        public static extern void nTracksHeader_SetDefaultScale(IntPtr handle, uint scale);

        [DllImport(LIB_NAME)]
        public static extern void nReadACLData(IntPtr tracks, IntPtr refPoses, IntPtr trackToSkeletonMap, IntPtr outAtom);

        [DllImport(LIB_NAME)]
        public static extern void nReadCurveACLData(IntPtr tracks, IntPtr outFloatKeys);

        [DllImport(LIB_NAME)]
        public static extern unsafe void nReadACLClipData(IntPtr compressedClip, FTransform* inRefPoses, FTrackToSkeletonMap* inTrackToSkeletonMap, FTransform* outAtom);

        [DllImport(LIB_NAME)]
        public static extern void nGetClipHeader(IntPtr compressedClip, out ushort numBones, out uint numSamples, out float sampleRate);

        // pure c# way:
        //var rawPtr = Marshal.AllocHGlobal(size + 8);
        //var aligned = new IntPtr(16 * (((long) rawPtr + 15) / 16));
    }
}