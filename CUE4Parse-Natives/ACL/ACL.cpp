#include "includes/ACLDecompress.h"

// Forward declaration
template <bool bUseBindPose>
void ProcessClips(const acl::compressed_tracks& tracks, FTransform* inRefPoses, FTrackToSkeletonMap* inTrackToSkeletonMap, FTransform* outAtom);

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
DLLEXPORT void nReadACLClipData(const acl::compressed_tracks& tracks, FTransform* inRefPoses, FTrackToSkeletonMap* inTrackToSkeletonMap, FTransform* outAtom)
{
    // For clip format, we use the same decompression logic as tracks
    // The main difference is in the memory layout and metadata handling
    if (tracks.get_duration() != 0)
    {
        ProcessClips<true>(tracks, inRefPoses, inTrackToSkeletonMap, outAtom);
    }
    else
    {
        ProcessClips<false>(tracks, inRefPoses, inTrackToSkeletonMap, outAtom);
    }
}


template <bool bUseBindPose>
void ProcessTracks(const acl::compressed_tracks& tracks, FTransform* inRefPoses, FTrackToSkeletonMap* inTrackToSkeletonMap, FTransform* outAtom)
{
    uint32_t numSamples = tracks.get_num_samples_per_track();
    float sampleRate = tracks.get_sample_rate();
    float duration = tracks.get_duration();

    DecompContextDefault context;
    context.initialize(tracks);

    FCUE4ParseOutputWriter<bUseBindPose> writer(inRefPoses, inTrackToSkeletonMap, outAtom, numSamples);
    for (uint32_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const float sample_time = rtm::scalar_min(float(sampleIndex) / sampleRate, duration);
        context.seek(sample_time, acl::SampleRoundingPolicy::Nearest);
        writer.SampleIndex = sampleIndex;
        context.decompress_tracks(writer);
    }
}

template <bool bUseBindPose>
void ProcessClips(const acl::compressed_tracks& tracks, FTransform* inRefPoses, FTrackToSkeletonMap* inTrackToSkeletonMap, FTransform* outAtom)
{
    uint32_t numSamples = tracks.get_num_samples_per_track();
    float sampleRate = tracks.get_sample_rate();
    float duration = tracks.get_duration();

    DecompContextDefault context;
    context.initialize(tracks);

    FCUE4ParseOutputWriter<bUseBindPose> writer(inRefPoses, inTrackToSkeletonMap, outAtom, numSamples);
    for (uint32_t sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const float sample_time = rtm::scalar_min(float(sampleIndex) / sampleRate, duration);
        context.seek(sample_time, acl::SampleRoundingPolicy::Nearest);
        writer.SampleIndex = sampleIndex;
        context.decompress_pose(writer);
    }
}
