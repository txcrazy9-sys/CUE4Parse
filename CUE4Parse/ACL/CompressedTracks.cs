using System;
using System.Runtime.InteropServices;
using static CUE4Parse.ACL.ACLNative;

namespace CUE4Parse.ACL
{
    public class CompressedTracks
    {
        public IntPtr Handle { get; private set; }
        private readonly int _bufferLength;

        public CompressedTracks(byte[] buffer)
        {
            _bufferLength = buffer.Length;
            Handle = nAllocate(_bufferLength);
            Marshal.Copy(buffer, 0, Handle, buffer.Length);

            // For clip format (0xac10ac10), skip validation to avoid tag mismatch
            if (IsClipFormat(buffer))
            {
                return; // Skip validation for clip format
            }

            var error = IsValid(false);
            if (error != null)
            {
                nDeallocate(Handle, _bufferLength);
                Handle = IntPtr.Zero;
                throw new ACLException(error);
            }
        }

        public CompressedTracks(IntPtr existing)
        {
            _bufferLength = -1;
            Handle = existing;
        }

        ~CompressedTracks()
        {
            if (_bufferLength >= 0 && Handle != IntPtr.Zero)
            {
                nDeallocate(Handle, _bufferLength);
                Handle = IntPtr.Zero;
            }
        }

        public string? IsValid(bool checkHash)
        {
            var error = Marshal.PtrToStringAnsi(nCompressedTracks_IsValid(Handle, checkHash))!;
            return error.Length > 0 ? error : null;
        }

        /// <summary>
        /// Check if the buffer contains clip format (0xac10ac10) rather than tracks format (0xac11ac11)
        /// </summary>
        private static bool IsClipFormat(byte[] buffer)
        {
            if (buffer == null || buffer.Length < 12)
                return false;

            // Read the tag from byte offset 8 (where CompressedClip stores its tag)
            uint tag = BitConverter.ToUInt32(buffer, 8);
            return tag == 0xac10ac10; // Clip format tag
        }

        public TracksHeader GetTracksHeader() => Marshal.PtrToStructure<TracksHeader>(Handle + Marshal.SizeOf<RawBufferHeader>());
        
        public ClipHeader GetClipHeader()
        {
            nGetClipHeader(Handle, out ushort numBones, out uint numSamples, out float sampleRate);
            return new ClipHeader
            {
                NumBones = numBones,
                NumSamples = numSamples,
                SampleRate = sampleRate
            };
        }
        
        public void SetDefaultScale(uint scale) => nTracksHeader_SetDefaultScale(Handle + Marshal.SizeOf<RawBufferHeader>(), scale);

        [DllImport(LIB_NAME)]
        private static extern IntPtr nCompressedTracks_IsValid(IntPtr handle, bool checkHash);

        [DllImport(LIB_NAME)]
        private static extern IntPtr nTracksHeader_SetDefaultScale(IntPtr handle, uint scale);
    }
}
