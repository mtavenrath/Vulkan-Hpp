// Copyright(c) 2018, NVIDIA CORPORATION. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// VulkanHpp Samples : DispatchLoaderDynamic
//                     Compile test on DispatchLoaderDynamic functions

#include "vulkan/vulkan.hpp"
#include <iostream>
#include <map>

static char const* AppName = "DispatchLoaderDynamic";
static char const* EngineName = "Vulkan.hpp";

namespace vk {
  class DynamicLoader {
  public:
    DynamicLoader() : m_success(false) {
      m_library = LoadLibrary("vulkan-1.dll");
      m_success = (m_library != 0);
    }

    ~DynamicLoader()
    {
      if (m_library)
      {
        FreeLibrary(m_library);
      }
    }

    template <typename T>
    T getProcAddress(const char* function) const
    {
      return (T)GetProcAddress(m_library, function);
    }

    bool success() const { return m_success; }
      
  private:
    bool m_success;
    HMODULE m_library;
  };
}

int main(int /*argc*/, char ** /*argv*/)
{
  try
  {
    // empty DispatchLoaderDynamic, used for init calls later on
    vk::DispatchLoaderDynamic dld0;

    HMODULE vulkanDll = LoadLibrary("vulkan-1.dll");
    vk::DynamicLoader dl;

    if (dl.)
    {
      // create a dispatcher, based on vkInstance/vkGetInstanceProcAddr only
      PFN_vkCreateInstance vkCreateInstance = PFN_vkCreateInstance(GetProcAddress(vulkanDll, "vkCreateInstance"));
      assert(vkCreateInstance);

      VkInstanceCreateInfo vkInstanceCreateInfo = {};
      vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      VkInstance vkInstanceC;
      VkResult vkResult = vkCreateInstance(&vkInstanceCreateInfo, nullptr, &vkInstanceC);
      assert(vkResult == VK_SUCCESS);

      vk::Instance vkInstance(vkInstanceC);

      PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = PFN_vkGetInstanceProcAddr(GetProcAddress(vulkanDll, "vkGetInstanceProcAddr"));
      assert(vkGetInstanceProcAddr);

      vk::DispatchLoaderDynamic dld1(vkInstance, vkGetInstanceProcAddr);

      // compare to the empty dispatcher, and init the empty dispatcher the same way
      assert(memcmp(&dld0, &dld1, sizeof(vk::DispatchLoaderDynamic)) != 0);
      dld0.init(vkInstance, vkGetInstanceProcAddr);
      assert(memcmp(&dld0, &dld1, sizeof(vk::DispatchLoaderDynamic)) == 0);

      // create a dispatcher, based on additional vkDevice/vkGetDeviceProcAddr
      std::vector<vk::PhysicalDevice> physicalDevices = vkInstance.enumeratePhysicalDevices(dld1);

      assert(!physicalDevices.empty());

      VkDeviceCreateInfo vkDeviceCreateInfo = {};
      vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      vk::Device device = physicalDevices[0].createDevice(vkDeviceCreateInfo, nullptr, dld1);
      assert(vkResult == VK_SUCCESS);

      PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkInstance.getProcAddr("vkGetDeviceProcAddr", dld1);

      vk::DispatchLoaderDynamic dld2(vkInstance, vkGetInstanceProcAddr, device, vkGetDeviceProcAddr);

      // compare to "simpler" dispatcher and make them equal
      assert(memcmp(&dld0, &dld2, sizeof(vk::DispatchLoaderDynamic)) != 0);
      dld0.init(vkInstance, vkGetInstanceProcAddr, device, vkGetDeviceProcAddr);
      assert(memcmp(&dld0, &dld2, sizeof(vk::DispatchLoaderDynamic)) == 0);

      FreeLibrary(vulkanDll);
    }
  }
  catch (vk::SystemError err)
  {
    std::cout << "vk::SystemError: " << err.what() << std::endl;
    exit(-1);
  }
  catch (...)
  {
    std::cout << "unknown error\n";
    exit(-1);
  }

  return 0;
}
