#include "includes/ACLDecompress.h"

// Forward declaration
template <bool bUseBindPose>
void ProcessClips(const acl::CompressedClip& clip, FTransform* inRefPoses, FTrackToSkeletonMap* inTrackToSkeletonMap, FTransform* outAtom);

template <bool bUseBindPose>
void ProcessTracks(const acl::compressed_tracks& tracks, FTransform* inRefPoses, FTrackToSkeletonMap* inTrackToSkeletonMap, FTransform* outAtom);

// ACL allocator
DLLEXPORT void* nAllocate(size_t size, size_t alignment) { return ACLAllocatorImpl.allocate(size, alignment); }
DLLEXPORT void nDeallocate(void* ptr, size_t size) { ACLAllocatorImpl.deallocate(ptr, size); }

// ACL compressed tracks
DLLEXPORT const char* nCompressedTracks_IsValid(acl::compressed_tracks* tracks, bool checkHash) { return tracks->is_valid(checkHash).c_str(); }
DLLEXPORT void nTracksHeader_SetDefaultScale(acl::acl_impl::tracks_header* header, uint32_t defaultScale) {
    // set_default_scale not available in ACL 1.3.5, this is a no-op
    // ACL 1.3.5 tracks_header doesn't have misc_packed field or set_default_scale method
}

DLLEXPORT void nReadACLData(const acl::compressed_tracks& tracks, FTransform* inRefPoses, FTrackToSkeletonMap* inTrackToSkeletonMap, FTransform* outAtom)
{
    if (tracks.get_duration() != 0)
    {
        ProcessTracks<true>(tracks, inRefPoses, inTrackToSkeletonMap, outAtom);
    }
    else
    {
        ProcessTracks<false>(tracks, inRefPoses, inTrackToSkeletonMap, outAtom);
    }
}

DLLEXPORT void nReadCurveACLData(const acl::compressed_tracks& tracks, float* outFloatKeys)
{
    uint32_t numSamples = tracks.get_num_samples_per_track();
    float sampleRate = tracks.get_sample_rate();
    float duration = tracks.get_duration();

    DecompContextDefault context;
    context.initialize(tracks);

    FCUE4ParseCurveWriter writer(outFloatKeys, numSamples);
    for (uint32_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const float sample_time = rtm::scalar_min(float(sampleIndex) / sampleRate, duration);
        context.seek(sample_time, acl::SampleRoundingPolicy::Nearest);
        writer.SampleIndex = sampleIndex;
        context.decompress_tracks(writer);
    }
}

// For clip format decompression - simplified implementation
DLLEXPORT void nReadACLClipData(const acl::CompressedClip& clip, FTransform* inRefPoses, FTrackToSkeletonMap* inTrackToSkeletonMap, FTransform* outAtom)
{
    // For clip format, we use the same decompression logic as tracks
    // The main difference is in the memory layout and metadata handling
    if (clip.get_size() != 0)
    {
        ProcessClips<true>(clip, inRefPoses, inTrackToSkeletonMap, outAtom);
    }
    else
    {
        ProcessClips<false>(clip, inRefPoses, inTrackToSkeletonMap, outAtom);
    }
}

// Get clip header information
DLLEXPORT void nGetClipHeader(const acl::CompressedClip& clip, uint16_t* numBones, uint32_t* numSamples, float* sampleRate)
{
    const acl::ClipHeader& header = acl::get_clip_header(clip);
    *numBones = header.num_bones;
    *numSamples = header.num_samples;
    *sampleRate = header.sample_rate;
}

template <bool bUseBindPose>
void ProcessTracks(const acl::compressed_tracks& tracks, FTransform* inRefPoses, FTrackToSkeletonMap* inTrackToSkeletonMap, FTransform* outAtom)
{
    uint32_t numSamples = tracks.get_num_samples_per_track();
    float sampleRate = tracks.get_sample_rate();
    float duration = tracks.get_duration();

    DecompContextDefault context;
    context.initialize(tracks);

    // Use FCUE4ParseTrackWriter for compressed_tracks
    FCUE4ParseTrackWriter<bUseBindPose> writer(inRefPoses, inTrackToSkeletonMap, outAtom, numSamples);
    for (uint32_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const float sample_time = rtm::scalar_min(float(sampleIndex) / sampleRate, duration);
        context.seek(sample_time, acl::SampleRoundingPolicy::Nearest);
        writer.SampleIndex = sampleIndex;
        context.decompress_tracks(writer);
    }
}

template <bool bUseBindPose>
void ProcessClips(const acl::CompressedClip& clip, FTransform* inRefPoses, FTrackToSkeletonMap* inTrackToSkeletonMap, FTransform* outAtom)
{
    CompressedClipContext context;
    context.initialize(clip);
    
    // Use get_clip_header() function instead of direct construction
    const acl::ClipHeader& header = acl::get_clip_header(clip);
    uint32_t numSamples = header.num_samples;
    float sampleRate = header.sample_rate;
    float duration = sampleRate > 0 ? float(numSamples) / sampleRate : 0.0f;

    // Use FCUE4ParseOutputWriter for compressed clips
    FCUE4ParseOutputWriter<bUseBindPose> writer(inRefPoses, inTrackToSkeletonMap, outAtom, numSamples);
    for (uint32_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const float sample_time = rtm::scalar_min(float(sampleIndex) / sampleRate, duration);
        context.seek(sample_time, acl::SampleRoundingPolicy::Nearest);
        writer.SampleIndex = sampleIndex;
        context.decompress_pose(writer);
    }
}
