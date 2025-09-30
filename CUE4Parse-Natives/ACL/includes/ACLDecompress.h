#pragma once
#include "ACLFramework.h"

inline rtm::vector4f RTM_SIMD_CALL UEVector3ToACL(const FVector& Input) { return rtm::vector_set(Input.X, Input.Y, Input.Z); }
inline rtm::quatf RTM_SIMD_CALL UEQuatToACL(const FQuat& Input) { return acl::quat_set(Input.X, Input.Y, Input.Z, Input.W); }

// Writer for compressed_tracks (inherits from track_writer)
template<bool bUseBindPose>
struct FCUE4ParseTrackWriter final : public acl::track_writer
{
    FTransform* RefPoses;
    FTrackToSkeletonMap* TrackToBoneMapping;

    FACLTransform* Atoms;
    uint32_t NumSamples;
    uint32_t SampleIndex;

    FCUE4ParseTrackWriter(FTransform* inRefPoses, FTrackToSkeletonMap* inTrackToSkeletonMap, FTransform* inAtoms, uint32_t inNumSamples)
        : RefPoses(inRefPoses)
        , TrackToBoneMapping(inTrackToSkeletonMap)
        , Atoms(static_cast<FACLTransform*>(inAtoms))
        , NumSamples(inNumSamples)
    {}

    RTM_FORCE_INLINE void RTM_SIMD_CALL write_float1(uint32_t track_index, rtm::scalarf_arg0 value)
    {
        // Could be used for scalar tracks
        Atoms[track_index * NumSamples + SampleIndex].SetScale3DRaw(rtm::vector_set(rtm::scalar_cast(value)));
    }

    RTM_FORCE_INLINE void RTM_SIMD_CALL write_float2(uint32_t track_index, rtm::vector4f_arg0 value)
    {
        // Could be used for 2D vector tracks
        Atoms[track_index * NumSamples + SampleIndex].SetTranslationRaw(value);
    }

    RTM_FORCE_INLINE void RTM_SIMD_CALL write_float3(uint32_t track_index, rtm::vector4f_arg0 value)
    {
        // Could be used for translation/scale
        Atoms[track_index * NumSamples + SampleIndex].SetTranslationRaw(value);
    }

    RTM_FORCE_INLINE void RTM_SIMD_CALL write_float4(uint32_t track_index, rtm::vector4f_arg0 value)
    {
        // Could be used for quaternion rotation
        rtm::quatf quat = acl::vector_to_quat(value);
        Atoms[track_index * NumSamples + SampleIndex].SetRotationRaw(quat);
    }

    RTM_FORCE_INLINE void RTM_SIMD_CALL write_vector4(uint32_t track_index, rtm::vector4f_arg0 value)
    {
        // Generic 4D vector
        Atoms[track_index * NumSamples + SampleIndex].SetTranslationRaw(value);
    }

    // Helper methods for getting default values from bind pose
    RTM_FORCE_INLINE rtm::quatf RTM_SIMD_CALL get_variable_default_rotation(uint32_t trackIndex) const
    {
        return UEQuatToACL(RefPoses[TrackToBoneMapping[trackIndex].BoneTreeIndex].Rotation);
    }

    RTM_FORCE_INLINE rtm::vector4f RTM_SIMD_CALL get_variable_default_translation(uint32_t trackIndex) const
    {
        return UEVector3ToACL(RefPoses[TrackToBoneMapping[trackIndex].BoneTreeIndex].Translation);
    }

    RTM_FORCE_INLINE rtm::vector4f RTM_SIMD_CALL get_variable_default_scale(uint32_t trackIndex) const
    {
        return UEVector3ToACL(RefPoses[TrackToBoneMapping[trackIndex].BoneTreeIndex].Scale3D);
    }
};

// Writer for compressed clips (inherits from OutputWriter)
template<bool bUseBindPose>
struct FCUE4ParseOutputWriter final : public acl::OutputWriter
{
    FTransform* RefPoses;
    FTrackToSkeletonMap* TrackToBoneMapping;

    FACLTransform* Atoms;
    uint32_t NumSamples;
    uint32_t SampleIndex;

    FCUE4ParseOutputWriter(FTransform* inRefPoses, FTrackToSkeletonMap* inTrackToSkeletonMap, FTransform* inAtoms, uint32_t inNumSamples)
        : RefPoses(inRefPoses)
        , TrackToBoneMapping(inTrackToSkeletonMap)
        , Atoms(static_cast<FACLTransform*>(inAtoms))
        , NumSamples(inNumSamples)
    {}

    // Required OutputWriter interface methods
    RTM_FORCE_INLINE constexpr bool skip_all_bone_rotations() const { return false; }
    RTM_FORCE_INLINE constexpr bool skip_all_bone_translations() const { return false; }
    RTM_FORCE_INLINE constexpr bool skip_all_bone_scales() const { return false; }
    
    RTM_FORCE_INLINE constexpr bool skip_bone_rotation(uint32_t /*bone_index*/) const { return false; }
    RTM_FORCE_INLINE constexpr bool skip_bone_translation(uint32_t /*bone_index*/) const { return false; }
    RTM_FORCE_INLINE constexpr bool skip_bone_scale(uint32_t /*bone_index*/) const { return false; }

    RTM_FORCE_INLINE void RTM_SIMD_CALL write_bone_rotation(uint32_t bone_index, rtm::quatf_arg0 rotation)
    {
        if constexpr (bUseBindPose)
        {
            Atoms[bone_index * NumSamples + SampleIndex].SetRotationRaw(rotation);
        }
        else
        {
            // Use default rotation from bind pose
            rtm::quatf default_rotation = get_variable_default_rotation(bone_index);
            Atoms[bone_index * NumSamples + SampleIndex].SetRotationRaw(default_rotation);
        }
    }

    RTM_FORCE_INLINE void RTM_SIMD_CALL write_bone_translation(uint32_t bone_index, rtm::vector4f_arg0 translation)
    {
        if constexpr (bUseBindPose)
        {
            Atoms[bone_index * NumSamples + SampleIndex].SetTranslationRaw(translation);
        }
        else
        {
            // Use default translation from bind pose
            rtm::vector4f default_translation = get_variable_default_translation(bone_index);
            Atoms[bone_index * NumSamples + SampleIndex].SetTranslationRaw(default_translation);
        }
    }

    RTM_FORCE_INLINE void RTM_SIMD_CALL write_bone_scale(uint32_t bone_index, rtm::vector4f_arg0 scale)
    {
        if constexpr (bUseBindPose)
        {
            Atoms[bone_index * NumSamples + SampleIndex].SetScale3DRaw(scale);
        }
        else
        {
            // Use default scale from bind pose
            rtm::vector4f default_scale = get_variable_default_scale(bone_index);
            Atoms[bone_index * NumSamples + SampleIndex].SetScale3DRaw(default_scale);
        }
    }

    // Helper methods for getting default values from bind pose
    RTM_FORCE_INLINE rtm::quatf RTM_SIMD_CALL get_variable_default_rotation(uint32_t trackIndex) const
    {
        return UEQuatToACL(RefPoses[TrackToBoneMapping[trackIndex].BoneTreeIndex].Rotation);
    }

    RTM_FORCE_INLINE rtm::vector4f RTM_SIMD_CALL get_variable_default_translation(uint32_t trackIndex) const
    {
        return UEVector3ToACL(RefPoses[TrackToBoneMapping[trackIndex].BoneTreeIndex].Translation);
    }

    RTM_FORCE_INLINE rtm::vector4f RTM_SIMD_CALL get_variable_default_scale(uint32_t trackIndex) const
    {
        return UEVector3ToACL(RefPoses[TrackToBoneMapping[trackIndex].BoneTreeIndex].Scale3D);
    }
};

struct FCUE4ParseCurveWriter final : public acl::track_writer
{
	float* Floats;
    uint32_t NumSamples;
    uint32_t SampleIndex;

    FCUE4ParseCurveWriter(float* inFloats, uint32_t inNumSamples)
        : Floats(inFloats)
        , NumSamples(inNumSamples)
    {}

	RTM_FORCE_INLINE void RTM_SIMD_CALL write_float1(uint32_t trackIndex, rtm::scalarf_arg0 floatValue)
    {
        Floats[trackIndex * NumSamples + SampleIndex] = rtm::scalar_cast(floatValue);
    }

    // Implement other track_writer methods for completeness
    RTM_FORCE_INLINE void RTM_SIMD_CALL write_float2(uint32_t /*track_index*/, rtm::vector4f_arg0 /*value*/) {}
    RTM_FORCE_INLINE void RTM_SIMD_CALL write_float3(uint32_t /*track_index*/, rtm::vector4f_arg0 /*value*/) {}
    RTM_FORCE_INLINE void RTM_SIMD_CALL write_float4(uint32_t /*track_index*/, rtm::vector4f_arg0 /*value*/) {}
    RTM_FORCE_INLINE void RTM_SIMD_CALL write_vector4(uint32_t /*track_index*/, rtm::vector4f_arg0 /*value*/) {}
};
