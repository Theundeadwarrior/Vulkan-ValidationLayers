/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "generated/vk_extension_helper.h"

#include <algorithm>
#include <chrono>
#include <mutex>
#include <thread>

TEST_F(VkPositiveLayerTest, ShaderRelaxedBlockLayout) {
    // This is a positive test, no errors expected
    // Verifies the ability to relax block layout rules with a shader that requires them to be relaxed
    TEST_DESCRIPTION("Create a shader that requires relaxed block layout.");

    AddRequiredExtensions(VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader requiring relaxed layout.
    // Without relaxed layout, we would expect a message like:
    // "Structure id 2 decorated as Block for variable in Uniform storage class
    // must follow standard uniform buffer layout rules: member 1 at offset 4 is not aligned to 16"

    const char *spv_source = R"(
                  OpCapability Shader
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint Vertex %main "main"
                  OpSource GLSL 450
                  OpMemberDecorate %S 0 Offset 0
                  OpMemberDecorate %S 1 Offset 4
                  OpDecorate %S Block
                  OpDecorate %B DescriptorSet 0
                  OpDecorate %B Binding 0
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
       %v3float = OpTypeVector %float 3
             %S = OpTypeStruct %float %v3float
%_ptr_Uniform_S = OpTypePointer Uniform %S
             %B = OpVariable %_ptr_Uniform_S Uniform
          %main = OpFunction %void None %3
             %5 = OpLabel
                  OpReturn
                  OpFunctionEnd
        )";
    VkShaderObj vs(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
}

TEST_F(VkPositiveLayerTest, ShaderUboStd430Layout) {
    // This is a positive test, no errors expected
    // Verifies the ability to scalar block layout rules with a shader that requires them to be relaxed
    TEST_DESCRIPTION("Create a shader that requires UBO std430 layout.");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2 =
        (PFN_vkGetPhysicalDeviceFeatures2)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");

    auto uniform_buffer_standard_layout_features = LvlInitStruct<VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR>(NULL);
    uniform_buffer_standard_layout_features.uniformBufferStandardLayout = VK_TRUE;
    auto query_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&uniform_buffer_standard_layout_features);
    vkGetPhysicalDeviceFeatures2(gpu(), &query_features2);

    auto set_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&uniform_buffer_standard_layout_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &set_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader requiring std430 in a uniform buffer.
    // Without uniform buffer standard layout, we would expect a message like:
    // "Structure id 3 decorated as Block for variable in Uniform storage class
    // must follow standard uniform buffer layout rules: member 0 is an array
    // with stride 4 not satisfying alignment to 16"

    const char *spv_source = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main"
               OpSource GLSL 460
               OpDecorate %_arr_float_uint_8 ArrayStride 4
               OpMemberDecorate %foo 0 Offset 0
               OpDecorate %foo Block
               OpDecorate %b DescriptorSet 0
               OpDecorate %b Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_8 = OpConstant %uint 8
%_arr_float_uint_8 = OpTypeArray %float %uint_8
        %foo = OpTypeStruct %_arr_float_uint_8
%_ptr_Uniform_foo = OpTypePointer Uniform %foo
          %b = OpVariable %_ptr_Uniform_foo Uniform
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, spv_source, "main", nullptr, SPV_ENV_VULKAN_1_0);
}

TEST_F(VkPositiveLayerTest, ShaderScalarBlockLayout) {
    // This is a positive test, no errors expected
    // Verifies the ability to scalar block layout rules with a shader that requires them to be relaxed
    TEST_DESCRIPTION("Create a shader that requires scalar block layout.");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2 =
        (PFN_vkGetPhysicalDeviceFeatures2)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");

    auto scalar_block_features = LvlInitStruct<VkPhysicalDeviceScalarBlockLayoutFeaturesEXT>(NULL);
    auto query_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&scalar_block_features);
    vkGetPhysicalDeviceFeatures2(gpu(), &query_features2);

    if (scalar_block_features.scalarBlockLayout != VK_TRUE) {
        GTEST_SKIP() << "scalarBlockLayout feature not supported";
    }

    auto set_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&scalar_block_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &set_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader requiring scalar layout.
    // Without scalar layout, we would expect a message like:
    // "Structure id 2 decorated as Block for variable in Uniform storage class
    // must follow standard uniform buffer layout rules: member 1 at offset 4 is not aligned to 16"

    const char *spv_source = R"(
                  OpCapability Shader
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint Vertex %main "main"
                  OpSource GLSL 450
                  OpMemberDecorate %S 0 Offset 0
                  OpMemberDecorate %S 1 Offset 4
                  OpMemberDecorate %S 2 Offset 8
                  OpDecorate %S Block
                  OpDecorate %B DescriptorSet 0
                  OpDecorate %B Binding 0
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
       %v3float = OpTypeVector %float 3
             %S = OpTypeStruct %float %float %v3float
%_ptr_Uniform_S = OpTypePointer Uniform %S
             %B = OpVariable %_ptr_Uniform_S Uniform
          %main = OpFunction %void None %3
             %5 = OpLabel
                  OpReturn
                  OpFunctionEnd
        )";

    VkShaderObj vs(this, spv_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
}

TEST_F(VkPositiveLayerTest, ComputeSharedMemoryLimitWorkgroupMemoryExplicitLayout) {
    TEST_DESCRIPTION(
        "Validate compute shader shared memory does not exceed maxComputeSharedMemorySize when using "
        "VK_KHR_workgroup_memory_explicit_layout");
    // More background: When workgroupMemoryExplicitLayout is enabled and there are 2 or more structs, the
    // maxComputeSharedMemorySize is the MAX of the structs since they share the same WorkGroup memory. Test makes sure validation
    // is not doing an ADD and correctly doing a MAX operation in this case.

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // need at least SPIR-V 1.4 for SPV_KHR_workgroup_memory_explicit_layout
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto explicit_layout_features = LvlInitStruct<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(explicit_layout_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    if (!explicit_layout_features.workgroupMemoryExplicitLayout) {
        GTEST_SKIP() << "workgroupMemoryExplicitLayout feature not supported.";
    }

    const uint32_t max_shared_memory_size = m_device->phy().properties().limits.maxComputeSharedMemorySize;
    const uint32_t max_shared_vec4 = max_shared_memory_size / 16;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        #extension GL_EXT_shared_memory_block : enable

        // Both structs by themselves are 16 bytes less than the max
        shared X {
            vec4 x1[)glsl";
    csSource << (max_shared_vec4 - 1);
    csSource << R"glsl(];
            vec4 x2;
        };

        void main() {
            x2.x = 0.0f; // prevent dead-code elimination
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2));
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(VkPositiveLayerTest, ComputeSharedMemoryLimitWorkgroupMemoryExplicitLayoutSpec) {
    TEST_DESCRIPTION(
        "Same test as ComputeSharedMemoryLimitWorkgroupMemoryExplicitLayout but making sure the path when using spec constants "
        "works");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // need at least SPIR-V 1.4 for SPV_KHR_workgroup_memory_explicit_layout
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto explicit_layout_features = LvlInitStruct<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR>();
    GetPhysicalDeviceFeatures2(explicit_layout_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &explicit_layout_features));

    if (!explicit_layout_features.workgroupMemoryExplicitLayout) {
        GTEST_SKIP() << "workgroupMemoryExplicitLayout feature not supported.";
    }

    const uint32_t max_shared_memory_size = m_device->phy().properties().limits.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;
    const uint32_t max_shared_vec4 = max_shared_memory_size / 16;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        #extension GL_EXT_shared_memory_block : enable

        // will be over the max if the spec constant uses default value
        layout(constant_id = 0) const uint value = )glsl";
    csSource << (max_shared_vec4 + 16);
    csSource << R"glsl(;

        // Both structs by themselves are 16 bytes less than the max
        shared X {
            vec4 x1[value];
            vec4 x2;
        };

        shared Y {
            int y1[)glsl";
    csSource << (max_shared_ints - 4);
    csSource << R"glsl(];
            int y2;
        };

        void main() {
            x2.x = 0.0f; // prevent dead-code elimination
            y2 = 0;
        }
    )glsl";

    uint32_t data = max_shared_vec4 - 16;

    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL,
                                   &specialization_info));
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(VkPositiveLayerTest, ComputeSharedMemoryAtLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory is valid at the exact maxComputeSharedMemorySize");

    ASSERT_NO_FATAL_FAILURE(Init());

    const uint32_t max_shared_memory_size = m_device->phy().properties().limits.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        shared int a[)glsl";
    csSource << (max_shared_ints);
    csSource << R"glsl(];
        void main(){}
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(VkPositiveLayerTest, ComputeSharedMemoryBooleanAtLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory is valid at the exact maxComputeSharedMemorySize using Booleans");

    ASSERT_NO_FATAL_FAILURE(Init());

    const uint32_t max_shared_memory_size = m_device->phy().properties().limits.maxComputeSharedMemorySize;
    // "Boolean values considered as 32-bit integer values for the purpose of this calculation."
    const uint32_t max_shared_bools = max_shared_memory_size / 4;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        shared bool a[)glsl";
    csSource << (max_shared_bools);
    csSource << R"glsl(];
        void main(){}
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT));
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(VkPositiveLayerTest, MeshSharedMemoryAtLimit) {
    TEST_DESCRIPTION("Validate mesh shader shared memory is valid at the exact maxMeshSharedMemorySize");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.meshShader) {
        GTEST_SKIP() << "Mesh shader not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &mesh_shader_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required.";
    }

    auto mesh_shader_properties = LvlInitStruct<VkPhysicalDeviceMeshShaderPropertiesEXT>();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxMeshSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream mesh_source;
    mesh_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        shared int a[)glsl";
    mesh_source << (max_shared_ints);
    mesh_source << R"glsl(];
        void main(){}
    )glsl";

    VkShaderObj mesh(this, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.fs_->GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(VkPositiveLayerTest, TaskSharedMemoryAtLimit) {
    TEST_DESCRIPTION("Validate Task shader shared memory is valid at the exact maxTaskSharedMemorySize");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.meshShader || !mesh_shader_features.taskShader) {
        GTEST_SKIP() << "Mesh and Task shader not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &mesh_shader_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required.";
    }

    auto mesh_shader_properties = LvlInitStruct<VkPhysicalDeviceMeshShaderPropertiesEXT>();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxMeshSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream task_source;
    task_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        shared int a[)glsl";
    task_source << (max_shared_ints);
    task_source << R"glsl(];
        void main(){}
    )glsl";

    VkShaderObj task(this, task_source.str().c_str(), VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj mesh(this, bindStateMeshShaderText, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task.GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(VkPositiveLayerTest, ComputeWorkGroupSizePrecedenceOverLocalSize) {
    // "If an object is decorated with the WorkgroupSize decoration, this takes precedence over any LocalSize or LocalSizeId
    // execution mode."
    TEST_DESCRIPTION("Make sure work WorkgroupSize decoration is used over LocalSize");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t x_size_limit = m_device->props.limits.maxComputeWorkGroupSize[0];
    uint32_t y_size_limit = m_device->props.limits.maxComputeWorkGroupSize[1];
    uint32_t z_size_limit = m_device->props.limits.maxComputeWorkGroupSize[2];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize )";
    spv_source << std::to_string(x_size_limit + 1) << " " << std::to_string(y_size_limit + 1) << " "
               << std::to_string(z_size_limit + 1);
    spv_source << R"(
               OpSource GLSL 450
               OpName %main "main"
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(
            new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit);
}

TEST_F(VkPositiveLayerTest, ComputeWorkGroupSizeSpecConstantUnder) {
    TEST_DESCRIPTION("Make sure spec constants get applied to to be under maxComputeWorkGroupSize");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t x_size_limit = m_device->props.limits.maxComputeWorkGroupSize[0];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %7 SpecId 0
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
          %7 = OpSpecConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpSpecConstantComposite %v3uint %7 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    uint32_t data = 1;

    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                         SPV_SOURCE_ASM, &specialization_info));
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit);
}

TEST_F(VkPositiveLayerTest, ComputeWorkGroupSizeLocalSizeId) {
    TEST_DESCRIPTION("Validate LocalSizeId doesn't triggers maxComputeWorkGroupSize limit");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto features13 = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    features13.maintenance4 = VK_TRUE;  // required to be supported in 1.3
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features13));

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %uint_2 %uint_1 %uint_1
               OpSource GLSL 450
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(
            new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM));
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit);
}

TEST_F(VkPositiveLayerTest, ComputeWorkGroupSizeLocalSizeIdSpecConstant) {
    TEST_DESCRIPTION("Validate LocalSizeId doesn't triggers maxComputeWorkGroupSize limit with spec constants");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto features13 = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    features13.maintenance4 = VK_TRUE;  // required to be supported in 1.3
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features13));

    uint32_t x_size_limit = m_device->props.limits.maxComputeWorkGroupSize[0];

    // layout(local_size_x_id = 18, local_size_z_id = 19) in;
    // layout(local_size_x = 32) in;
    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %spec_x %uint_1 %spec_z
               OpSource GLSL 450
               OpDecorate %spec_x SpecId 18
               OpDecorate %spec_z SpecId 19
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %spec_x = OpSpecConstant %uint 32
     %uint_1 = OpConstant %uint 1
     %spec_z = OpSpecConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    uint32_t data = x_size_limit - 1;

    VkSpecializationMapEntry entry;
    entry.constantID = 18;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3,
                                         SPV_SOURCE_ASM, &specialization_info));
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit);
}

TEST_F(VkPositiveLayerTest, ComputeWorkGroupSizePrecedenceOverLocalSizeId) {
    // "If an object is decorated with the WorkgroupSize decoration, this takes precedence over any LocalSize or LocalSizeId
    // execution mode."
    TEST_DESCRIPTION("Make sure work WorkgroupSize decoration is used over LocalSizeId");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto features13 = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    features13.maintenance4 = VK_TRUE;  // required to be supported in 1.3
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features13));

    uint32_t x_size_limit = m_device->props.limits.maxComputeWorkGroupSize[0];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %spec_x %uint_1 %uint_1
               OpSource GLSL 450
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
               OpDecorate %spec_x SpecId 18
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %spec_x = OpSpecConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(
            new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM));
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit);
}

TEST_F(VkPositiveLayerTest, ComputeSharedMemorySpecConstantOp) {
    TEST_DESCRIPTION("Validate compute shader shared memory");

    ASSERT_NO_FATAL_FAILURE(Init());

    const uint32_t max_shared_memory_size = m_device->phy().properties().limits.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    if (max_shared_ints < 16 * 7) {
        GTEST_SKIP() << "Supported compute shader shared memory size is too small";
    }

    char const *cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

        layout(constant_id = 0) const uint Condition = 0;
        layout(constant_id = 1) const uint SharedSize = 16;

        #define enableSharedMemoryOpt (Condition == 1 || Condition == 2 || Condition == 3)
        shared uint arr[enableSharedMemoryOpt ? SharedSize : 1][enableSharedMemoryOpt ? 7 : 1];

        void main() {}
    )glsl";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT));
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(VkPositiveLayerTest, ShaderNonSemanticInfo) {
    // This is a positive test, no errors expected
    // Verifies the ability to use non-semantic extended instruction sets when the extension is enabled
    TEST_DESCRIPTION("Create a shader that uses SPV_KHR_non_semantic_info.");
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // compute shader using a non-semantic extended instruction set.

    const char *spv_source = R"(
                   OpCapability Shader
                   OpExtension "SPV_KHR_non_semantic_info"
   %non_semantic = OpExtInstImport "NonSemantic.Validation.Test"
                   OpMemoryModel Logical GLSL450
                   OpEntryPoint GLCompute %main "main"
                   OpExecutionMode %main LocalSize 1 1 1
           %void = OpTypeVoid
              %1 = OpExtInst %void %non_semantic 55 %void
           %func = OpTypeFunction %void
           %main = OpFunction %void None %func
              %2 = OpLabel
                   OpReturn
                   OpFunctionEnd
        )";

    VkShaderObj cs(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
}

TEST_F(VkPositiveLayerTest, SpirvGroupDecorations) {
    TEST_DESCRIPTION("Test shader validation support for group decorations.");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const std::string spv_source = R"(
              OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 430
               OpName %main "main"
               OpName %gl_GlobalInvocationID "gl_GlobalInvocationID"
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
               OpDecorate %_runtimearr_float ArrayStride 4
               OpDecorate %4 BufferBlock
               OpDecorate %5 Offset 0
          %4 = OpDecorationGroup
          %5 = OpDecorationGroup
               OpGroupDecorate %4 %_struct_6 %_struct_7 %_struct_8 %_struct_9 %_struct_10 %_struct_11
               OpGroupMemberDecorate %5 %_struct_6 0 %_struct_7 0 %_struct_8 0 %_struct_9 0 %_struct_10 0 %_struct_11 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %13 NonWritable
               OpDecorate %13 Restrict
         %14 = OpDecorationGroup
         %12 = OpDecorationGroup
         %13 = OpDecorationGroup
               OpGroupDecorate %12 %15
               OpGroupDecorate %12 %15
               OpGroupDecorate %12 %15
               OpDecorate %15 DescriptorSet 0
               OpDecorate %15 Binding 5
               OpGroupDecorate %14 %16
               OpDecorate %16 DescriptorSet 0
               OpDecorate %16 Binding 0
               OpGroupDecorate %12 %17
               OpDecorate %17 Binding 1
               OpGroupDecorate %13 %18 %19
               OpDecorate %18 Binding 2
               OpDecorate %19 Binding 3
               OpGroupDecorate %14 %20
               OpGroupDecorate %12 %20
               OpGroupDecorate %13 %20
               OpDecorate %20 Binding 4
       %bool = OpTypeBool
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %v3uint = OpTypeVector %uint 3
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%_ptr_Uniform_int = OpTypePointer Uniform %int
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_runtimearr_int = OpTypeRuntimeArray %int
%_runtimearr_float = OpTypeRuntimeArray %float
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
      %int_0 = OpConstant %int 0
  %_struct_6 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_6 = OpTypePointer Uniform %_struct_6
         %15 = OpVariable %_ptr_Uniform__struct_6 Uniform
  %_struct_7 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
         %16 = OpVariable %_ptr_Uniform__struct_7 Uniform
  %_struct_8 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_8 = OpTypePointer Uniform %_struct_8
         %17 = OpVariable %_ptr_Uniform__struct_8 Uniform
  %_struct_9 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_9 = OpTypePointer Uniform %_struct_9
         %18 = OpVariable %_ptr_Uniform__struct_9 Uniform
 %_struct_10 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_10 = OpTypePointer Uniform %_struct_10
         %19 = OpVariable %_ptr_Uniform__struct_10 Uniform
 %_struct_11 = OpTypeStruct %_runtimearr_float
%_ptr_Uniform__struct_11 = OpTypePointer Uniform %_struct_11
         %20 = OpVariable %_ptr_Uniform__struct_11 Uniform
       %main = OpFunction %void None %23
         %40 = OpLabel
         %41 = OpLoad %v3uint %gl_GlobalInvocationID
         %42 = OpCompositeExtract %uint %41 0
         %43 = OpAccessChain %_ptr_Uniform_float %16 %int_0 %42
         %44 = OpAccessChain %_ptr_Uniform_float %17 %int_0 %42
         %45 = OpAccessChain %_ptr_Uniform_float %18 %int_0 %42
         %46 = OpAccessChain %_ptr_Uniform_float %19 %int_0 %42
         %47 = OpAccessChain %_ptr_Uniform_float %20 %int_0 %42
         %48 = OpAccessChain %_ptr_Uniform_float %15 %int_0 %42
         %49 = OpLoad %float %43
         %50 = OpLoad %float %44
         %51 = OpLoad %float %45
         %52 = OpLoad %float %46
         %53 = OpLoad %float %47
         %54 = OpFAdd %float %49 %50
         %55 = OpFAdd %float %54 %51
         %56 = OpFAdd %float %55 %52
         %57 = OpFAdd %float %56 %53
               OpStore %48 %57
               OpReturn
               OpFunctionEnd
)";

    // CreateDescriptorSetLayout
    VkDescriptorSetLayoutBinding dslb[6] = {};
    size_t dslb_size = size(dslb);
    for (size_t i = 0; i < dslb_size; i++) {
        dslb[i].binding = i;
        dslb[i].descriptorCount = 1;
        dslb[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        dslb[i].pImmutableSamplers = NULL;
        dslb[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_ALL;
    }
    if (m_device->props.limits.maxPerStageDescriptorStorageBuffers < dslb_size) {
        GTEST_SKIP() << "Needed storage buffer bindings (" << dslb_size << ") exceeds this devices limit of "
                     << m_device->props.limits.maxPerStageDescriptorStorageBuffers;
    }

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dsl_bindings_.resize(dslb_size);
    memcpy(pipe.dsl_bindings_.data(), dslb, dslb_size * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_.reset(new VkShaderObj(this, spv_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM));
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(VkPositiveLayerTest, CreatePipelineCheckShaderCapabilityExtension1of2) {
    // This is a positive test, no errors expected
    // Verifies the ability to deal with a shader that declares a non-unique SPIRV capability ID
    TEST_DESCRIPTION("Create a shader in which uses a non-unique capability ID extension, 1 of 2");

    AddRequiredExtensions(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // These tests require that the device support multiViewport
    if (!m_device->phy().features().multiViewport) {
        GTEST_SKIP() << "Device does not support multiViewport, test skipped.";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader using viewport array capability
    char const *vsSource = R"glsl(
        #version 450
        #extension GL_ARB_shader_viewport_layer_array : enable
        void main() {
            gl_ViewportIndex = 1;
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(VkPositiveLayerTest, CreatePipelineCheckShaderCapabilityExtension2of2) {
    // This is a positive test, no errors expected
    // Verifies the ability to deal with a shader that declares a non-unique SPIRV capability ID
    TEST_DESCRIPTION("Create a shader in which uses a non-unique capability ID extension, 2 of 2");

    // Need to use SPV_EXT_shader_viewport_index_layer
    AddRequiredExtensions(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // These tests require that the device support multiViewport
    if (!m_device->phy().features().multiViewport) {
        GTEST_SKIP() << "Device does not support multiViewport, test skipped.";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Vertex shader using viewport array capability
    char const *vsSource = R"glsl(
        #version 450
        #extension GL_ARB_shader_viewport_layer_array : enable
        void main() {
            gl_ViewportIndex = 1;
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(VkPositiveLayerTest, CreatePipelineFragmentOutputNotWrittenButMasked) {
    TEST_DESCRIPTION(
        "Test that no error is produced when the fragment shader fails to declare an output, but the corresponding attachment's "
        "write mask is 0.");

    ASSERT_NO_FATAL_FAILURE(Init());

    char const *fsSource = R"glsl(
        #version 450
        void main() {}
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

    /* set up CB 0, not written, but also masked */
    pipe.AddDefaultColorAttachment(0);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendDummy();
    descriptorSet.CreateVKDescriptorSet(m_commandBuffer);

    pipe.CreateVKPipeline(descriptorSet.GetPipelineLayout(), renderPass());
}

TEST_F(VkPositiveLayerTest, PointSizeWriteInFunction) {
    TEST_DESCRIPTION("Create a pipeline using TOPOLOGY_POINT_LIST and write PointSize in vertex shader function.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Create VS declaring PointSize and write to it in a function call.
    VkShaderObj vs(this, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj ps(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
    {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), ps.GetStageCreateInfo()};
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
    }
}

TEST_F(VkPositiveLayerTest, PointSizeGeomShaderSuccess) {
    TEST_DESCRIPTION(
        "Create a pipeline using TOPOLOGY_POINT_LIST, set PointSize vertex shader, and write in the final geometry stage.");

    ASSERT_NO_FATAL_FAILURE(Init());

    if ((!m_device->phy().features().geometryShader) || (!m_device->phy().features().shaderTessellationAndGeometryPointSize)) {
        GTEST_SKIP() << "Device does not support the required geometry shader features";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Create VS declaring PointSize and writing to it
    VkShaderObj vs(this, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, bindStateGeomPointSizeShaderText, VK_SHADER_STAGE_GEOMETRY_BIT);
    VkShaderObj ps(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), ps.GetStageCreateInfo()};
    // Set Input Assembly to TOPOLOGY POINT LIST
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(VkPositiveLayerTest, PointSizeGeomShaderDontEmit) {
    TEST_DESCRIPTION("If vertex is not emitted, don't need Point Size in Geometry shader");

    ASSERT_NO_FATAL_FAILURE(Init());

    if ((!m_device->phy().features().geometryShader) || (!m_device->phy().features().shaderTessellationAndGeometryPointSize)) {
        GTEST_SKIP() << "Device does not support the required geometry shader features";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Never calls OpEmitVertex
    static char const *gsSource = R"glsl(
        #version 450
        layout (points) in;
        layout (points) out;
        layout (max_vertices = 1) out;
        void main() {
           gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertPointSizeShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(VkPositiveLayerTest, LoosePointSizeWrite) {
    TEST_DESCRIPTION("Create a pipeline using TOPOLOGY_POINT_LIST and write PointSize outside of a structure.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    const char *LoosePointSizeWrite = R"(
                                       OpCapability Shader
                                  %1 = OpExtInstImport "GLSL.std.450"
                                       OpMemoryModel Logical GLSL450
                                       OpEntryPoint Vertex %main "main" %glposition %glpointsize %gl_VertexIndex
                                       OpSource GLSL 450
                                       OpName %main "main"
                                       OpName %vertices "vertices"
                                       OpName %glposition "glposition"
                                       OpName %glpointsize "glpointsize"
                                       OpName %gl_VertexIndex "gl_VertexIndex"
                                       OpDecorate %glposition BuiltIn Position
                                       OpDecorate %glpointsize BuiltIn PointSize
                                       OpDecorate %gl_VertexIndex BuiltIn VertexIndex
                               %void = OpTypeVoid
                                  %3 = OpTypeFunction %void
                              %float = OpTypeFloat 32
                            %v2float = OpTypeVector %float 2
                               %uint = OpTypeInt 32 0
                             %uint_3 = OpConstant %uint 3
                %_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
   %_ptr_Private__arr_v2float_uint_3 = OpTypePointer Private %_arr_v2float_uint_3
                           %vertices = OpVariable %_ptr_Private__arr_v2float_uint_3 Private
                                %int = OpTypeInt 32 1
                              %int_0 = OpConstant %int 0
                           %float_n1 = OpConstant %float -1
                                 %16 = OpConstantComposite %v2float %float_n1 %float_n1
               %_ptr_Private_v2float = OpTypePointer Private %v2float
                              %int_1 = OpConstant %int 1
                            %float_1 = OpConstant %float 1
                                 %21 = OpConstantComposite %v2float %float_1 %float_n1
                              %int_2 = OpConstant %int 2
                            %float_0 = OpConstant %float 0
                                 %25 = OpConstantComposite %v2float %float_0 %float_1
                            %v4float = OpTypeVector %float 4
            %_ptr_Output_gl_Position = OpTypePointer Output %v4float
                         %glposition = OpVariable %_ptr_Output_gl_Position Output
           %_ptr_Output_gl_PointSize = OpTypePointer Output %float
                        %glpointsize = OpVariable %_ptr_Output_gl_PointSize Output
                     %_ptr_Input_int = OpTypePointer Input %int
                     %gl_VertexIndex = OpVariable %_ptr_Input_int Input
                              %int_3 = OpConstant %int 3
                %_ptr_Output_v4float = OpTypePointer Output %v4float
                  %_ptr_Output_float = OpTypePointer Output %float
                               %main = OpFunction %void None %3
                                  %5 = OpLabel
                                 %18 = OpAccessChain %_ptr_Private_v2float %vertices %int_0
                                       OpStore %18 %16
                                 %22 = OpAccessChain %_ptr_Private_v2float %vertices %int_1
                                       OpStore %22 %21
                                 %26 = OpAccessChain %_ptr_Private_v2float %vertices %int_2
                                       OpStore %26 %25
                                 %33 = OpLoad %int %gl_VertexIndex
                                 %35 = OpSMod %int %33 %int_3
                                 %36 = OpAccessChain %_ptr_Private_v2float %vertices %35
                                 %37 = OpLoad %v2float %36
                                 %38 = OpCompositeExtract %float %37 0
                                 %39 = OpCompositeExtract %float %37 1
                                 %40 = OpCompositeConstruct %v4float %38 %39 %float_0 %float_1
                                 %42 = OpAccessChain %_ptr_Output_v4float %glposition
                                       OpStore %42 %40
                                       OpStore %glpointsize %float_1
                                       OpReturn
                                       OpFunctionEnd
        )";

    // Create VS declaring PointSize and write to it in a function call.
    VkShaderObj vs(this, LoosePointSizeWrite, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj ps(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), ps.GetStageCreateInfo()};
        // Set Input Assembly to TOPOLOGY POINT LIST
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
    }
}

TEST_F(VkPositiveLayerTest, ShaderDrawParametersWithoutFeature) {
    TEST_DESCRIPTION("Use VK_KHR_shader_draw_parameters in 1.0 before shaderDrawParameters feature was added");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        GTEST_SKIP() << "requires Vulkan 1.0 exactly";
    }

    char const *vsSource = R"glsl(
        #version 460
        void main(){
           gl_Position = vec4(float(gl_BaseVertex));
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);

    if (VK_SUCCESS == vs.InitFromGLSLTry()) {
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit);
    }
}

TEST_F(VkPositiveLayerTest, ShaderDrawParametersWithoutFeature11) {
    TEST_DESCRIPTION("Use VK_KHR_shader_draw_parameters in 1.1 using the extension");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 460
        void main(){
           gl_Position = vec4(float(gl_BaseVertex));
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

    // make sure using SPIR-V 1.3 as extension is core and not needed in Vulkan then
    if (VK_SUCCESS == vs.InitFromGLSLTry(false)) {
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit);
    }
}

TEST_F(VkPositiveLayerTest, ShaderDrawParametersWithFeature) {
    TEST_DESCRIPTION("Use VK_KHR_shader_draw_parameters in 1.2 with feature bit enabled");

    // use 1.2 to get the feature bit in VkPhysicalDeviceVulkan11Features
    SetTargetApiVersion(VK_API_VERSION_1_2);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto features11 = LvlInitStruct<VkPhysicalDeviceVulkan11Features>();
    features11.shaderDrawParameters = VK_TRUE;
    auto features2 = GetPhysicalDeviceFeatures2(features11);

    GetPhysicalDeviceFeatures2(features2);

    if (features11.shaderDrawParameters != VK_TRUE) {
        printf("shaderDrawParameters not supported, skipping test\n");
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 460
        void main(){
           gl_Position = vec4(float(gl_BaseVertex));
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_GLSL_TRY);

    // make sure using SPIR-V 1.3 as extension is core and not needed in Vulkan then
    if (VK_SUCCESS == vs.InitFromGLSLTry(false)) {
        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit | kWarningBit);
    }
}

TEST_F(VkPositiveLayerTest, ValidateComputeShaderSharedMemory) {
    TEST_DESCRIPTION("Validate compute shader shared memory does not exceed maxComputeSharedMemorySize");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Make sure compute pipeline has a compute shader stage set
    char const *csSource = R"glsl(
        #version 450
        shared uint a;
        shared float b;
        shared vec2 c;
        shared mat3 d;
        shared mat4 e[3];
        struct A {
            int f;
            float g;
            uint h;
        };
        shared A f;
        void main(){
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT));
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(VkPositiveLayerTest, TestShaderInputAndOutputComponents) {
    TEST_DESCRIPTION("Test shader layout in and out with different components.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
                #version 450

                layout(location = 0, component = 0) out vec2 rg;
                layout(location = 0, component = 2) out float b;

                layout(location = 1, component = 0) out float r;
                layout(location = 1, component = 1) out vec3 gba;

                layout(location = 2) out vec4 out_color_0;
                layout(location = 3) out vec4 out_color_1;

                layout(location = 4, component = 0) out float x;
                layout(location = 4, component = 1) out vec2 yz;
                layout(location = 4, component = 3) out float w;

                layout(location = 5, component = 0) out vec3 stp;
                layout(location = 5, component = 3) out float q;

                layout(location = 6, component = 0) out vec2 cd;
                layout(location = 6, component = 2) out float e;
                layout(location = 6, component = 3) out float f;

                layout(location = 7, component = 0) out float ar1;
                layout(location = 7, component = 1) out float ar2[2];
                layout(location = 7, component = 3) out float ar3;

                void main() {
	                    vec2 xy = vec2((gl_VertexIndex >> 1u) & 1u, gl_VertexIndex & 1u);
                        gl_Position = vec4(xy, 0.0f, 1.0f);
                        out_color_0 = vec4(1.0f, 0.0f, 1.0f, 0.0f);
                        out_color_1 = vec4(0.0f, 1.0f, 0.0f, 1.0f);
                        rg = vec2(0.25f, 0.75f);
                        b = 0.5f;
                        r = 0.75f;
                        gba = vec3(1.0f);
                        x = 1.0f;
                        yz = vec2(0.25f);
                        w = 0.5f;
                        stp = vec3(1.0f);
                        q = 0.1f;
                        ar1 = 1.0f;
                        ar2[0] = 0.5f;
                        ar2[1] = 0.75f;
                        ar3 = 1.0f;
                }
            )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fsSource = R"glsl(
                #version 450

                layout(location = 0, component = 0) in float r;
                layout(location = 0, component = 1) in vec2 gb;

                layout(location = 1, component = 0) in float r1;
                layout(location = 1, component = 1) in float g1;
                layout(location = 1, component = 2) in float b1;
                layout(location = 1, component = 3) in float a1;

                layout(location = 2) in InputBlock {
                    layout(location = 3, component = 3) float one_alpha;
                    layout(location = 2, component = 3) float zero_alpha;
                    layout(location = 3, component = 2) float one_blue;
                    layout(location = 2, component = 2) float zero_blue;
                    layout(location = 3, component = 1) float one_green;
                    layout(location = 2, component = 1) float zero_green;
                    layout(location = 3, component = 0) float one_red;
                    layout(location = 2, component = 0) float zero_red;
                } inBlock;

                layout(location = 4, component = 0) in vec2 xy;
                layout(location = 4, component = 2) in vec2 zw;

                layout(location = 5, component = 0) in vec4 st;

                layout(location = 6, component = 0) in vec4 cdef;

                layout(location = 7, component = 0) in float ar1;
                layout(location = 7, component = 1) in float ar2;
                layout(location = 8, component = 1) in float ar3;
                layout(location = 7, component = 3) in float ar4;

                layout (location = 0) out vec4 color;

                void main() {
                    color = vec4(r, gb, 1.0f) *
                            vec4(r1, g1, 1.0f, a1) *
                            vec4(inBlock.zero_red, inBlock.zero_green, inBlock.zero_blue, inBlock.zero_alpha) *
                            vec4(inBlock.one_red, inBlock.one_green, inBlock.one_blue, inBlock.one_alpha) *
                            vec4(xy, zw) * st * cdef * vec4(ar1, ar2, ar3, ar4);
                }
            )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit | kErrorBit);
}

TEST_F(VkPositiveLayerTest, TestShaderInputAndOutputStructComponents) {
    TEST_DESCRIPTION("Test shader interface with structs.");

    ASSERT_NO_FATAL_FAILURE(Init());

    // There is a crash inside the driver on S10
    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "This test should not run on Galaxy S10";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
                #version 450

                struct R {
                    vec4 rgba;
                };

                layout(location = 0) out R color[3];

                void main() {
                    color[0].rgba = vec4(1.0f);
                    color[1].rgba = vec4(0.5f);
                    color[2].rgba = vec4(0.75f);
                }
            )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fsSource = R"glsl(
                #version 450

                struct R {
                    vec4 rgba;
                };

                layout(location = 0) in R inColor[3];

                layout (location = 0) out vec4 color;

                void main() {
                    color = inColor[0].rgba * inColor[1].rgba * inColor[2].rgba;
                }
            )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit | kErrorBit);
}

TEST_F(VkPositiveLayerTest, ShaderPointSizeStructMemeberWritten) {
    TEST_DESCRIPTION("Write built-in PointSize within a struct");

    SetTargetApiVersion(VK_API_VERSION_1_1); // At least 1.1 is required for maintenance4
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " required but not supported";
    }
    auto maint4features = LvlInitStruct<VkPhysicalDeviceMaintenance4FeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(maint4features);
    if (!maint4features.maintenance4) {
        GTEST_SKIP() << "VkPhysicalDeviceMaintenance4FeaturesKHR::maintenance4 is required but not enabled.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const std::string vs_src = R"asm(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %14 %25 %47 %52
               OpSource GLSL 450
               OpMemberDecorate %12 0 BuiltIn Position
               OpMemberDecorate %12 1 BuiltIn PointSize
               OpMemberDecorate %12 2 BuiltIn ClipDistance
               OpMemberDecorate %12 3 BuiltIn CullDistance
               OpDecorate %12 Block
               OpMemberDecorate %18 0 ColMajor
               OpMemberDecorate %18 0 Offset 0
               OpMemberDecorate %18 0 MatrixStride 16
               OpMemberDecorate %18 1 Offset 64
               OpMemberDecorate %18 2 Offset 80
               OpDecorate %18 Block
               OpDecorate %25 Location 0
               OpDecorate %47 Location 1
               OpDecorate %52 Location 0
          %3 = OpTypeVoid
          %4 = OpTypeFunction %3
          %7 = OpTypeFloat 32
          %8 = OpTypeVector %7 4
          %9 = OpTypeInt 32 0
         %10 = OpConstant %9 1
         %11 = OpTypeArray %7 %10
         %12 = OpTypeStruct %8 %7 %11 %11
         %13 = OpTypePointer Output %12
         %14 = OpVariable %13 Output
         %15 = OpTypeInt 32 1
         %16 = OpConstant %15 0
         %17 = OpTypeMatrix %8 4
         %18 = OpTypeStruct %17 %7 %8
         %19 = OpTypePointer PushConstant %18
         %20 = OpVariable %19 PushConstant
         %21 = OpTypePointer PushConstant %17
         %24 = OpTypePointer Input %8
         %25 = OpVariable %24 Input
         %28 = OpTypePointer Output %8
         %30 = OpConstant %7 0.5
         %31 = OpConstant %9 2
         %32 = OpTypePointer Output %7
         %36 = OpConstant %9 3
         %46 = OpConstant %15 1
         %47 = OpVariable %24 Input
         %48 = OpTypePointer Input %7
         %52 = OpVariable %28 Output
         %53 = OpTypeVector %7 3
         %56 = OpConstant %7 1
          %main = OpFunction %3 None %4
          %6 = OpLabel

               ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
               ; For the following, only the _first_ index of the access chain
               ; should be used for output validation, as subsequent indices refer
               ; to individual components within the output variable of interest.
               ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
         %22 = OpAccessChain %21 %20 %16
         %23 = OpLoad %17 %22
         %26 = OpLoad %8 %25
         %27 = OpMatrixTimesVector %8 %23 %26
         %29 = OpAccessChain %28 %14 %16
               OpStore %29 %27
         %33 = OpAccessChain %32 %14 %16 %31
         %34 = OpLoad %7 %33
         %35 = OpFMul %7 %30 %34
         %37 = OpAccessChain %32 %14 %16 %36
         %38 = OpLoad %7 %37
         %39 = OpFMul %7 %30 %38
         %40 = OpFAdd %7 %35 %39
         %41 = OpAccessChain %32 %14 %16 %31
               OpStore %41 %40
         %42 = OpAccessChain %32 %14 %16 %10
         %43 = OpLoad %7 %42
         %44 = OpFNegate %7 %43
         %45 = OpAccessChain %32 %14 %16 %10
               OpStore %45 %44
         %49 = OpAccessChain %48 %47 %36
         %50 = OpLoad %7 %49
         %51 = OpAccessChain %32 %14 %46
               OpStore %51 %50

         %54 = OpLoad %8 %47
         %55 = OpVectorShuffle %53 %54 %54 0 1 2
         %57 = OpCompositeExtract %7 %55 0
         %58 = OpCompositeExtract %7 %55 1
         %59 = OpCompositeExtract %7 %55 2
         %60 = OpCompositeConstruct %8 %57 %58 %59 %56
               OpStore %52 %60
               OpReturn
               OpFunctionEnd
    )asm";
    auto vs = VkShaderObj::CreateFromASM(*this, VK_SHADER_STAGE_VERTEX_BIT, vs_src, "main");

    if (vs) {
        VkPushConstantRange push_constant_ranges[1]{{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * (16 + 4 + 1)}};

        VkPipelineLayoutCreateInfo const pipeline_layout_info{
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, push_constant_ranges};

        VkVertexInputBindingDescription input_binding[2] = {
            {0, 16, VK_VERTEX_INPUT_RATE_VERTEX},
            {1, 16, VK_VERTEX_INPUT_RATE_VERTEX},
        };
        VkVertexInputAttributeDescription input_attribs[2] = {
            {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
            {1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
        };

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.shader_stages_ = {vs->GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
        pipe.pipeline_layout_ci_ = pipeline_layout_info;
        pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        pipe.vi_ci_.pVertexBindingDescriptions = input_binding;
        pipe.vi_ci_.vertexBindingDescriptionCount = 2;
        pipe.vi_ci_.pVertexAttributeDescriptions = input_attribs;
        pipe.vi_ci_.vertexAttributeDescriptionCount = 2;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
    } else {
        printf("Error creating shader from assembly\n");
    }
}

TEST_F(VkPositiveLayerTest, Std430SpirvOptFlags10) {
    TEST_DESCRIPTION("Reproduces issue 3442 where spirv-opt fails to set layout flags options using Vulkan 1.0");
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/3442

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto uniform_buffer_standard_layout_features = LvlInitStruct<VkPhysicalDeviceUniformBufferStandardLayoutFeatures>();
    auto scalar_block_layout_features =
        LvlInitStruct<VkPhysicalDeviceScalarBlockLayoutFeatures>(&uniform_buffer_standard_layout_features);
    auto features2 = GetPhysicalDeviceFeatures2(scalar_block_layout_features);

    if (scalar_block_layout_features.scalarBlockLayout == VK_FALSE ||
        uniform_buffer_standard_layout_features.uniformBufferStandardLayout == VK_FALSE) {
        GTEST_SKIP() << "scalarBlockLayout and uniformBufferStandardLayout are not supported Skipping";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);

    const char *fragment_source = R"glsl(
#version 450
#extension GL_ARB_separate_shader_objects:enable
#extension GL_EXT_samplerless_texture_functions:require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

layout(std430, set=0,binding=0)uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
    vec4 lightPositions[1];
    int SliceCutoffs[6];
}ubo;

// this specialization constant triggers the validation layer to recompile the shader
// which causes the error related to the above uniform
layout(constant_id = 0) const float spec = 10.0f;

layout(location=0) out vec4 frag_color;
void main() {
    frag_color = vec4(ubo.lightPositions[0]) * spec;
}
    )glsl";

    // Force a random value to replace the default to trigger shader val logic to replace it
    float data = 2.0f;
    VkSpecializationMapEntry entry = {0, 0, sizeof(float)};
    VkSpecializationInfo specialization_info = {1, &entry, sizeof(float), &data};
    const VkShaderObj fs(this, fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                         &specialization_info);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(VkPositiveLayerTest, Std430SpirvOptFlags12) {
    TEST_DESCRIPTION("Reproduces issue 3442 where spirv-opt fails to set layout flags options using Vulkan 1.2");
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/3442

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    auto features2 = GetPhysicalDeviceFeatures2(features12);
    if (features12.scalarBlockLayout == VK_FALSE || features12.uniformBufferStandardLayout == VK_FALSE) {
        GTEST_SKIP() << "scalarBlockLayout and uniformBufferStandardLayout are not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);

    const char *fragment_source = R"glsl(
#version 450
#extension GL_ARB_separate_shader_objects:enable
#extension GL_EXT_samplerless_texture_functions:require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

layout(std430, set=0,binding=0)uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
    vec4 lightPositions[1];
    int SliceCutoffs[6];
}ubo;

// this specialization constant triggers the validation layer to recompile the shader
// which causes the error related to the above uniform
layout(constant_id = 0) const float spec = 10.0f;

layout(location=0) out vec4 frag_color;
void main() {
    frag_color = vec4(ubo.lightPositions[0]) * spec;
}
    )glsl";

    // Force a random value to replace the default to trigger shader val logic to replace it
    float data = 2.0f;
    VkSpecializationMapEntry entry = {0, 0, sizeof(float)};
    VkSpecializationInfo specialization_info = {1, &entry, sizeof(float), &data};
    const VkShaderObj fs(this, fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL,
                         &specialization_info);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}

TEST_F(VkPositiveLayerTest, SpecializationWordBoundryOffset) {
    TEST_DESCRIPTION("Make sure a specialization constant entry can stide over a word boundry");

    // require to make enable logic simpler
    AddRequiredExtensions(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto float16int8_features = LvlInitStruct<VkPhysicalDeviceFloat16Int8FeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(float16int8_features);
    if (float16int8_features.shaderInt8 == VK_FALSE) {
        GTEST_SKIP() << "shaderInt8 feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD, need real device to produce output to check";
    }

    // glslang currenlty turned the GLSL to
    //      %19 = OpSpecConstantOp %uint UConvert %a
    // which causes issue (to be fixed outside scope of this test)
    // but move the UConvert to inside the function as
    //      %19 = OpUConvert %uint %a
    //
    // #version 450
    // #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
    // layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
    // // All spec constants will write zero by default
    // layout (constant_id = 0) const uint8_t a = uint8_t(0);
    // layout (constant_id = 1) const uint b = 0;
    // layout (constant_id = 3) const uint c = 0;
    // layout (constant_id = 4) const uint d = 0;
    // layout (constant_id = 5) const uint8_t e = uint8_t(0);
    //
    // layout(set = 0, binding = 0) buffer ssbo {
    //     uint data[5];
    // };
    //
    // void main() {
    //     data[0] = 0; // clear full word
    //     data[0] = uint(a);
    //     data[1] = b;
    //     data[2] = c;
    //     data[3] = d;
    //     data[4] = 0; // clear full word
    //     data[4] = uint(e);
    // }
    std::string cs_src = R"(
               OpCapability Shader
               OpCapability Int8
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types_int8"
               OpDecorate %_arr_uint_uint_5 ArrayStride 4
               OpMemberDecorate %ssbo 0 Offset 0
               OpDecorate %ssbo BufferBlock
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
               OpDecorate %a SpecId 0
               OpDecorate %b SpecId 1
               OpDecorate %c SpecId 3
               OpDecorate %d SpecId 4
               OpDecorate %e SpecId 5
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_5 = OpConstant %uint 5
%_arr_uint_uint_5 = OpTypeArray %uint %uint_5
       %ssbo = OpTypeStruct %_arr_uint_uint_5
%_ptr_Uniform_ssbo = OpTypePointer Uniform %ssbo
          %_ = OpVariable %_ptr_Uniform_ssbo Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %uint_0 = OpConstant %uint 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
      %uchar = OpTypeInt 8 0
          %a = OpSpecConstant %uchar 0
      %int_1 = OpConstant %int 1
          %b = OpSpecConstant %uint 0
      %int_2 = OpConstant %int 2
          %c = OpSpecConstant %uint 0
      %int_3 = OpConstant %int 3
          %d = OpSpecConstant %uint 0
      %int_4 = OpConstant %int 4
          %e = OpSpecConstant %uchar 0
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpUConvert %uint %a
         %33 = OpUConvert %uint %e
         %16 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_0
               OpStore %16 %uint_0
         %20 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_0
               OpStore %20 %19
         %23 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_1
               OpStore %23 %b
         %26 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_2
               OpStore %26 %c
         %29 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_3
               OpStore %29 %d
         %31 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_4
               OpStore %31 %uint_0
         %34 = OpAccessChain %_ptr_Uniform_uint %_ %int_0 %int_4
               OpStore %34 %33
               OpReturn
               OpFunctionEnd
    )";

    // Use strange combinations of size and offsets around word boundry
    VkSpecializationMapEntry entries[5] = {
        {0, 1, 1},  // OpTypeInt 8
        {1, 1, 4},  // OpTypeInt 32
        {3, 2, 4},  // OpTypeInt 32
        {4, 3, 4},  // OpTypeInt 32
        {5, 3, 1},  // OpTypeInt 8
    };

    uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    VkSpecializationInfo specialization_info = {
        5,
        entries,
        sizeof(uint8_t) * 8,
        reinterpret_cast<void *>(data),
    };

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_.reset(new VkShaderObj(this, cs_src.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM,
                                   &specialization_info));
    pipe.InitState();
    pipe.CreateComputePipeline();

    // Submit shader to see SSBO output
    VkBufferObj buffer;
    auto bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bci.size = 1024;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer.init(*m_device, bci, mem_props);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 0, 1024, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    // Make sure spec constants were updated correctly
    void *pData;
    ASSERT_VK_SUCCESS(vk::MapMemory(m_device->device(), buffer.memory().handle(), 0, VK_WHOLE_SIZE, 0, &pData));
    uint32_t *ssbo_data = reinterpret_cast<uint32_t *>(pData);
    ASSERT_EQ(ssbo_data[0], 0x02);
    ASSERT_EQ(ssbo_data[1], 0x05040302);
    ASSERT_EQ(ssbo_data[2], 0x06050403);
    ASSERT_EQ(ssbo_data[3], 0x07060504);
    ASSERT_EQ(ssbo_data[4], 0x04);
    vk::UnmapMemory(m_device->device(), buffer.memory().handle());
}

TEST_F(VkPositiveLayerTest, Spirv16Vulkan13) {
    TEST_DESCRIPTION("Create a shader using 1.3 spirv environment");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(Init());

    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_3);
}

TEST_F(VkPositiveLayerTest, PositiveShaderModuleIdentifier) {
    TEST_DESCRIPTION("Create a pipeline using a shader module identifier");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto shader_cache_control_features = LvlInitStruct<VkPhysicalDevicePipelineCreationCacheControlFeatures>();
    auto shader_module_id_features =
        LvlInitStruct<VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT>(&shader_cache_control_features);
    auto features2 = GetPhysicalDeviceFeatures2(shader_module_id_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    auto sm_id_create_info = LvlInitStruct<VkPipelineShaderStageModuleIdentifierCreateInfoEXT>();
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);

    auto get_identifier = LvlInitStruct<VkShaderModuleIdentifierEXT>();
    vk::GetShaderModuleIdentifierEXT(device(), vs.handle(), &get_identifier);
    sm_id_create_info.identifierSize = get_identifier.identifierSize;
    sm_id_create_info.pIdentifier = get_identifier.identifier;

    auto stage_ci = LvlInitStruct<VkPipelineShaderStageCreateInfo>(&sm_id_create_info);
    stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.stageCount = 1;
    pipe.gp_ci_.pStages = &stage_ci;
    pipe.gp_ci_.flags = VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(VkPositiveLayerTest, OpTypeArraySpecConstant) {
    TEST_DESCRIPTION("Make sure spec constants for a OpTypeArray doesn't assert");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %storageBuffer 0 Offset 0
               OpDecorate %storageBuffer BufferBlock
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
               OpDecorate %sc SpecId 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
%storageBuffer = OpTypeStruct %int
%_ptr_Uniform_storageBuffer = OpTypePointer Uniform %storageBuffer
          %_ = OpVariable %_ptr_Uniform_storageBuffer Uniform
      %int_0 = OpConstant %int 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
         %sc = OpSpecConstant %uint 10
%_arr_int_sc = OpTypeArray %int %sc
%_ptr_Workgroup__arr_int_sc = OpTypePointer Workgroup %_arr_int_sc
  %wg_normal = OpVariable %_ptr_Workgroup__arr_int_sc Workgroup
      %int_3 = OpConstant %int 3
%_ptr_Workgroup_int = OpTypePointer Workgroup %int
         %xx = OpSpecConstant %uint 1
         %yy = OpSpecConstant %uint 1
         %zz = OpSpecConstant %uint 1
%gl_WorkGroupSize = OpSpecConstantComposite %v3uint %xx %yy %zz
         %57 = OpSpecConstantOp %uint CompositeExtract %gl_WorkGroupSize 2
         %58 = OpSpecConstantOp %uint CompositeExtract %gl_WorkGroupSize 1
         %59 = OpSpecConstantOp %uint IMul %57 %58
         %60 = OpSpecConstantOp %uint CompositeExtract %gl_WorkGroupSize 0
         %61 = OpSpecConstantOp %uint IMul %59 %60
%_arr_int_21 = OpTypeArray %int %61
%_ptr_Workgroup__arr_int_21 = OpTypePointer Workgroup %_arr_int_21
      %wg_op = OpVariable %_ptr_Workgroup__arr_int_21 Workgroup
%_ptr_Function__arr_int_sc = OpTypePointer Function %_arr_int_sc
%_ptr_Function_int = OpTypePointer Function %int
         %34 = OpSpecConstantOp %uint IAdd %sc %uint_1
%_arr_int_34 = OpTypeArray %int %34
%_ptr_Function__arr_int_34 = OpTypePointer Function %_arr_int_34
%_ptr_Uniform_int = OpTypePointer Uniform %int
       %main = OpFunction %void None %3
          %5 = OpLabel
%func_normal = OpVariable %_ptr_Function__arr_int_sc Function
    %func_op = OpVariable %_ptr_Function__arr_int_34 Function
         %18 = OpAccessChain %_ptr_Workgroup_int %wg_normal %int_3
         %19 = OpLoad %int %18
         %25 = OpAccessChain %_ptr_Workgroup_int %wg_op %int_3
         %26 = OpLoad %int %25
         %27 = OpIAdd %int %19 %26
         %31 = OpAccessChain %_ptr_Function_int %func_normal %int_3
         %32 = OpLoad %int %31
         %33 = OpIAdd %int %27 %32
         %38 = OpAccessChain %_ptr_Function_int %func_op %int_3
         %39 = OpLoad %int %38
         %40 = OpIAdd %int %33 %39
         %42 = OpAccessChain %_ptr_Uniform_int %_ %int_0
               OpStore %42 %40
               OpReturn
               OpFunctionEnd
    )";

    uint32_t data = 5;

    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    // Use default value for spec constant
    const auto set_info_nospec = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                         SPV_SOURCE_ASM, nullptr));
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info_nospec, kErrorBit | kWarningBit);

    // Use spec constant to update value
    const auto set_info_spec = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                         SPV_SOURCE_ASM, &specialization_info));
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info_spec, kErrorBit | kWarningBit);
}

TEST_F(VkPositiveLayerTest, OpTypeStructRuntimeArray) {
    TEST_DESCRIPTION("Make sure variables with a OpTypeStruct can handle a runtime array inside");

    ASSERT_NO_FATAL_FAILURE(Init());

    // %float = OpTypeFloat 32
    // %ra = OpTypeRuntimeArray %float
    // %struct = OpTypeStruct %ra
    char const *cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer sb {
            float values[];
        };
        void main(){
            values[gl_LocalInvocationIndex] = gl_LocalInvocationIndex;
        }
    )glsl";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_.reset(new VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT));
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(VkPositiveLayerTest, StorageImageWriteMoreComponent) {
    TEST_DESCRIPTION("Test writing to image with less components.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    VkPhysicalDeviceFeatures available_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&available_features));
    if (!available_features.shaderStorageImageExtendedFormats) {
        GTEST_SKIP() << "shaderStorageImageExtendedFormats is not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(&available_features));

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Rg32ui) uniform uimage2D storageImage;
    // imageStore(storageImage, ivec2(1, 1), uvec3(1, 1, 1));
    //
    // Rg32ui == 2-component but writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpCapability StorageImageExtendedFormats
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint 2D 0 0 0 2 Rg32ui
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %coord %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    const VkFormat format = VK_FORMAT_R32G32_UINT;  // Rg32ui
    if (!ImageFormatAndFeaturesSupported(gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL);

    VkDescriptorImageInfo image_info = {};
    image_info.imageView = image.targetView(format);
    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &ds.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();
}

TEST_F(VkPositiveLayerTest, StorageImageUnknownWriteMoreComponent) {
    TEST_DESCRIPTION("Test writing to image with less components for Unknown for OpTypeImage.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    VkPhysicalDeviceFeatures available_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&available_features));
    if (!available_features.shaderStorageImageExtendedFormats) {
        GTEST_SKIP() << "shaderStorageImageExtendedFormats is not supported";
    } else if (!available_features.shaderStorageImageWriteWithoutFormat) {
        GTEST_SKIP() << "shaderStorageImageWriteWithoutFormat is not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(&available_features));

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Unknown) readonly uniform uimage2D storageImage;
    // imageStore(storageImage, ivec2(1, 1), uvec3(1, 1, 1));
    //
    // Unknown will become a 2-component but writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpCapability StorageImageExtendedFormats
               OpCapability StorageImageWriteWithoutFormat
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
               OpDecorate %var NonReadable
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint 2D 0 0 0 2 Unknown
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %coord %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    const VkFormat format = VK_FORMAT_R32G32_UINT;
    if (!ImageFormatAndFeaturesSupported(gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL);

    VkDescriptorImageInfo image_info = {};
    image_info.imageView = image.targetView(format);
    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &ds.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();
}

TEST_F(VkPositiveLayerTest, StorageImageWriteSpecConstantMoreComponent) {
    TEST_DESCRIPTION("Test writing to image with less components with Texel being a spec constant.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    VkPhysicalDeviceFeatures available_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&available_features));
    if (!available_features.shaderStorageImageExtendedFormats) {
        GTEST_SKIP() << "shaderStorageImageExtendedFormats is not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(&available_features));

    // not valid GLSL, but would look like:
    // layout (constant_id = 0) const uint sc = 1;
    // layout(set = 0, binding = 0, Rg32ui) uniform uimage2D storageImage;
    // imageStore(storageImage, ivec2(1, 1), uvec3(1, sc, sc + 1));
    //
    // Rg32ui == 2-component but writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpCapability StorageImageExtendedFormats
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint 2D 0 0 0 2 Rg32ui
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
      %coord = OpConstantComposite %v2int %int_1 %int_1
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
         %sc = OpSpecConstant %uint 1
      %sc_p1 = OpSpecConstantOp %uint IAdd %sc %uint_1
    %texelU3 = OpSpecConstantComposite %v3uint %uint_1 %sc %sc_p1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %coord %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    const VkFormat format = VK_FORMAT_R32G32_UINT;  // Rg32ui
    if (!ImageFormatAndFeaturesSupported(gpu(), format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage image";
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, format, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL);

    VkDescriptorImageInfo image_info = {};
    image_info.imageView = image.targetView(format);
    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

    uint32_t data = 2;
    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);
    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(
        new VkShaderObj(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM, &specialization_info));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &ds.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();
}

TEST_F(VkPositiveLayerTest, StorageTexelBufferWriteMoreComponent) {
    TEST_DESCRIPTION("Test writing to image with less components.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    VkPhysicalDeviceFeatures available_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&available_features));
    if (!available_features.shaderStorageImageExtendedFormats) {
        GTEST_SKIP() << "shaderStorageImageExtendedFormats is not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(&available_features));

    // not valid GLSL, but would look like:
    // layout(set = 0, binding = 0, Rg32ui) uniform uimageBuffer storageTexelBuffer;
    // imageStore(storageTexelBuffer, 1, uvec3(1, 1, 1));
    //
    // Rg32ui == 2-component but writing 3 texels to it
    const char *source = R"(
               OpCapability Shader
               OpCapability ImageBuffer
               OpCapability StorageImageExtendedFormats
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %image = OpTypeImage %uint Buffer 0 0 0 2 Rg32ui
        %ptr = OpTypePointer UniformConstant %image
        %var = OpVariable %ptr UniformConstant
     %v3uint = OpTypeVector %uint 3
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
    %texelU3 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %image %var
               OpImageWrite %load %int_1 %texelU3 ZeroExtend
               OpReturn
               OpFunctionEnd
        )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    const VkFormat format = VK_FORMAT_R32G32_UINT;  // Rg32ui
    if (!BufferFormatAndFeaturesSupported(gpu(), format, VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        GTEST_SKIP() << "Format doesn't support storage texel buffer";
    }

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    VkBufferViewCreateInfo buff_view_ci = LvlInitStruct<VkBufferViewCreateInfo>();
    buff_view_ci.buffer = buffer.handle();
    buff_view_ci.format = format;
    buff_view_ci.range = VK_WHOLE_SIZE;
    vk_testing::BufferView buffer_view(*m_device, buff_view_ci);

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    descriptor_write.pTexelBufferView = &buffer_view.handle();
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(this, source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&ds.layout_});
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &ds.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();
}

TEST_F(VkPositiveLayerTest, UnnormalizedCoordinatesNotSampled) {
    TEST_DESCRIPTION("If a samper is unnormalizedCoordinates, using COMBINED_IMAGE_SAMPLER, but texelFetch, don't throw error");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // This generates OpImage*Dref* instruction on R8G8B8A8_UNORM format.
    // Verify that it is allowed on this implementation if
    // VK_KHR_format_feature_flags2 is available.
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        auto fmt_props_3 = LvlInitStruct<VkFormatProperties3KHR>();
        auto fmt_props = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);

        vk::GetPhysicalDeviceFormatProperties2(gpu(), VK_FORMAT_R8G8B8A8_UNORM, &fmt_props);

        if (!(fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT_KHR)) {
            GTEST_SKIP() << "R8G8B8A8_UNORM does not support OpImage*Dref* operations";
        }
    }

    VkShaderObj vs(this, bindStateMinimalShaderText, VK_SHADER_STAGE_VERTEX_BIT);

    const char *fsSource = R"(
               OpCapability Shader
               OpCapability ImageBuffer
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %var DescriptorSet 0
               OpDecorate %var Binding 0
       %void = OpTypeVoid
       %func = OpTypeFunction %void
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
    %v4float = OpTypeVector %float 4
      %v3int = OpTypeVector %int 3
 %image_type = OpTypeImage %float 3D 0 0 0 1 Unknown
%sampled_image = OpTypeSampledImage %image_type
        %ptr = OpTypePointer UniformConstant %sampled_image
        %var = OpVariable %ptr UniformConstant
      %int_1 = OpConstant %int 1
      %cords = OpConstantComposite %v3int %int_1 %int_1 %int_1
       %main = OpFunction %void None %func
      %label = OpLabel
       %load = OpLoad %sampled_image %var
      %image = OpImage %image_type %load
      %fetch = OpImageFetch %v4float %image %cords
               OpReturn
               OpFunctionEnd
        )";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitInfo();
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_VK_SUCCESS(g_pipe.CreateGraphicsPipeline());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    VkImageObj image_3d(m_device);
    image_3d.Init(image_ci);
    ASSERT_TRUE(image_3d.initialized());

    // If the sampler is unnormalizedCoordinates, the imageview type shouldn't be 3D, CUBE, 1D_ARRAY, 2D_ARRAY, CUBE_ARRAY.
    // This causes DesiredFailure.
    VkImageView view = image_3d.targetView(format, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0,
                                           VK_REMAINING_ARRAY_LAYERS, VK_IMAGE_VIEW_TYPE_3D);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    vk_testing::Sampler sampler(*m_device, sampler_ci);

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}
