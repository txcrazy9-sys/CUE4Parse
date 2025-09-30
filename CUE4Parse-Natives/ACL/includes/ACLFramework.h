#pragma once
#include <acl/math/quat_32.h>
#include <acl/core/ansi_allocator.h>
#include <acl/core/compressed_clip.h>
#include <acl/core/compressed_tracks.h>
#include <acl/algorithm/uniformly_sampled/decoder.h>
#include <acl/decompression/decompress.h>
#include "Structs.h"
#include "Framework.h"

acl::ANSIAllocator ACLAllocatorImpl;
using DecompContextDefault = acl::decompression_context<acl::default_decompression_settings>;
using CompressedClipContext = acl::uniformly_sampled::DecompressionContext<acl::uniformly_sampled::DefaultDecompressionSettings>;
