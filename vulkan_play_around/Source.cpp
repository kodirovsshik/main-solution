
#pragma warning(disable : 28251 26812 26495)

#include <ksn/window.hpp>
#include <ksn/math_vec.hpp>

#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <vector>
#include <algorithm>
#include <optional>
#include <execution>


#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#pragma comment(lib, "vulkan-1.lib")
#endif

#include <vulkan/vulkan.hpp>

#pragma comment(lib, "libksn_window.lib")
#pragma comment(lib, "libksn_time.lib")




#define USE_DEBUG_MESSENGER_FOR_RELEASE 0

const std::array vk_layers = {
	"VK_LAYER_KHRONOS_validation",
};
const std::array vk_extension_data =
{
#ifdef _WIN32
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
	VK_KHR_SURFACE_EXTENSION_NAME,
#if _KSN_IS_DEBUG_BUILD || USE_DEBUG_MESSENGER_FOR_RELEASE
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};


const std::array vk_device_extensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	//VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
	//VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME,
	//VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME,
	//VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME,
	//VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
};




void _check(VkResult status, const char* file, int line)
{
	if (status == VK_SUCCESS)
		return;

	std::stringstream ss;
	ss << file << ':' << line << ": Vulkan call has failed: " << (int)status;
	throw std::exception(ss.str().data());
}

#define check(expr) _check((VkResult)(expr), __FILE__, __LINE__)



class main_app_t
{
	struct queue_family_indices
	{
		std::optional<uint32_t> arr[4];

		static constexpr size_t graphics_index = 0;
		static constexpr size_t compute_index = 1;
		static constexpr size_t transfer_index = 2;
		static constexpr size_t present_index = 3;

		static constexpr size_t families_count = 4;


		operator bool() const noexcept
		{
			for (size_t i = 1; i < 4; ++i)
				if (!this->arr[i].has_value())
					return false;
			return true;
		}
	};


	void _init_open_window()
	{
		static constexpr ksn::window_style_t style =
			ksn::window_style::border |
			ksn::window_style::caption |
			ksn::window_style::close_min_max |
			ksn::window_style::hidden |
			0;

		auto result = this->window.open(this->window_size[0], this->window_size[1], this->window_title.data(), style);

		if (result != ksn::window_open_result::ok)
		{
			std::stringstream ss;
			ss << "Failed to open the window: Code " << (int)result;
			throw std::exception(ss.str().data());
		}
	}
	void _init_create_surface()
	{
#ifdef _WIN32
		vk::Win32SurfaceCreateInfoKHR create_info{};
		create_info.hwnd = this->window.window_native_handle();
		create_info.hinstance = GetModuleHandle(NULL);

		this->vk_surface = this->vk_instance.createWin32SurfaceKHR(create_info);
#endif
	}

	ksn::window_t window;
	ksn::vec<2, uint16_t> window_size = { 800, 600 };
	std::u8string window_title = u8"Sus window ですか ඞ?";
	
	vk::Instance vk_instance;
	vk::PhysicalDevice vk_physical_device;
	vk::Device vk_device;
	vk::SurfaceKHR vk_surface;
	vk::ShaderModule vk_test_shader;
	vk::PhysicalDeviceMemoryProperties vk_device_memory_properties;


	vk::Queue vk_compute_queue;
	vk::CommandPool vk_compute_command_pool;
	vk::CommandBuffer vk_compute_command_buffer;

	vk::Queue vk_transfer_queue;
	vk::CommandPool vk_transfer_command_pool;
	vk::CommandBuffer vk_transfer_command_buffer;

	vk::Queue vk_present_queue;
	vk::CommandPool vk_present_command_pool;
	vk::CommandBuffer vk_present_command_buffer;

	vk::Pipeline vk_compute_pipeline;
	vk::PipelineLayout vk_compute_pipeline_layout;
	vk::DescriptorSetLayout vk_compute_pipeline_descriptor_set_layout;

	std::vector<vk::DeviceMemory> m_vk_allocations;

	void _init_create_instance()
	{
		vk::InstanceCreateInfo create_info{};
		create_info.ppEnabledLayerNames = vk_layers.data();
		create_info.enabledLayerCount = (uint32_t)vk_layers.size();
		create_info.enabledExtensionCount = (uint32_t)vk_extension_data.size();
		create_info.ppEnabledExtensionNames = vk_extension_data.data();

		vk::ApplicationInfo application_info{};
		application_info.apiVersion = VK_API_VERSION_1_2;

		create_info.pApplicationInfo = &application_info;

		this->vk_instance = vk::createInstance(create_info);
	}
	static bool _check_device_fitness_extensions(const vk::PhysicalDevice& device)
	{
		auto extensions = device.enumerateDeviceExtensionProperties();
		std::vector<std::string_view> extension_names{ extensions.size() };

		std::transform(std::execution::par_unseq, extensions.begin(), extensions.end(), extension_names.begin(), []
		(const vk::ExtensionProperties& extension) -> std::string_view
		{
			return extension.extensionName;
		});

		std::sort(std::execution::par_unseq, extension_names.begin(), extension_names.end(), []
		(const std::string_view& a, const std::string_view& b) -> bool
		{
			return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
		});

		auto extension_present_checker = [&]
		(const char* required_extension) -> bool
		{
			return std::binary_search(extension_names.begin(), extension_names.end(), std::string_view(required_extension));
		};

		bool all_extensions_supported = true;
		if (!std::all_of(std::execution::par_unseq, vk_device_extensions.begin(), vk_device_extensions.end(), extension_present_checker))
		{
			return false;
		}

		return true;
	}
	bool _check_device_fitness_queues(const vk::PhysicalDevice& dev, queue_family_indices& indices)
	{
		const auto queues = dev.getQueueFamilyProperties();

		uint32_t i = 0;
		for (const auto& family : queues)
		{
			if (indices)
				break;

			static constexpr vk::QueueFlags families_bits[4] = 
			{ 
				(vk::QueueFlags)VK_QUEUE_GRAPHICS_BIT , 
				(vk::QueueFlags)VK_QUEUE_COMPUTE_BIT , 
				(vk::QueueFlags)VK_QUEUE_TRANSFER_BIT, 
				(vk::QueueFlags)0
			};

			for (size_t j = 0; j < queue_family_indices::families_count; ++j)
			{
				if (indices.arr[j].has_value())
					continue;

				bool condition;
				if (families_bits[i] == (vk::QueueFlags)0)
					condition = dev.getSurfaceSupportKHR(i, this->vk_surface);
				else
					condition = (bool)(family.queueFlags & families_bits[i]);

				if (condition)
					indices.arr[j] = i;
			}
		}

		return (bool)indices;
	}
	bool _is_device_supported(const vk::PhysicalDevice& dev, queue_family_indices& indices)
	{
		//uint32_t nformats = 0, npresent_modes = 0;
		//vkGetPhysicalDeviceSurfaceFormatsKHR(dev, this->vk_surface, &nformats, nullptr);
		//vkGetPhysicalDeviceSurfacePresentModesKHR(dev, this->vk_surface, &npresent_modes, nullptr);

		//if (nformats == 0 || npresent_modes == 0)
		//	return false;
		
		return 
			_check_device_fitness_extensions(dev) &&
			_check_device_fitness_queues(dev, indices) &&
			true;
	}

	void _init_physical_device(queue_family_indices& indices)
	{
		auto devices = this->vk_instance.enumeratePhysicalDevices();
		vk::PhysicalDevice* p_device = nullptr;
		
		for (auto& device : devices)
		{
			if (!_is_device_supported(device, indices))
				continue;
			p_device = &device;
			break;
		}

		if (p_device == nullptr)
			throw std::exception("Failed to find suitabe device");

		this->vk_physical_device = std::move(*p_device);
		this->vk_device_memory_properties = this->vk_physical_device.getMemoryProperties();
	}
	void _init_device(const queue_family_indices& indices)
	{
		vk::DeviceCreateInfo device_create_info{};
		device_create_info.enabledExtensionCount = (uint32_t)vk_device_extensions.size();
		device_create_info.ppEnabledExtensionNames = vk_device_extensions.data();
		
		std::vector<uint32_t> queue_families_indices;
		for (auto& idx : indices.arr)
		{
			if (!idx.has_value())
				continue;
			if (std::ranges::find(queue_families_indices, idx.value()) == queue_families_indices.end())
				queue_families_indices.push_back(idx.value());
		}
		
		const float priority = 1;
		size_t i = 0;
		std::vector<vk::DeviceQueueCreateInfo> queue_families_infos(queue_families_indices.size());
		for (auto& info : queue_families_infos)
		{
			info.queueFamilyIndex = queue_families_indices[i];
			info.queueCount = 1;
			info.pQueuePriorities = &priority;
			++i;
		}

		queue_families_indices.clear();
		queue_families_indices.shrink_to_fit();

		device_create_info.pQueueCreateInfos = queue_families_infos.data();
		device_create_info.queueCreateInfoCount = (uint32_t)queue_families_infos.size();
		this->vk_device = this->vk_physical_device.createDevice(device_create_info);
	}
	void _init_queues(const queue_family_indices& indices)
	{
		this->vk_compute_queue = this->vk_device.getQueue(indices.arr[indices.compute_index].value(), 0);
		this->vk_transfer_queue = this->vk_device.getQueue(indices.arr[indices.transfer_index].value(), 0);
		this->vk_present_queue = this->vk_device.getQueue(indices.arr[indices.present_index].value(), 0);
	}
	void _init_create_command_pool(const queue_family_indices& indices)
	{
		vk::CommandPoolCreateInfo create_info{};

		create_info.queueFamilyIndex = indices.arr[indices.compute_index].value();
		this->vk_compute_command_pool = this->vk_device.createCommandPool(create_info);

		create_info.queueFamilyIndex = indices.arr[indices.transfer_index].value();
		this->vk_transfer_command_pool = this->vk_device.createCommandPool(create_info);

		create_info.queueFamilyIndex = indices.arr[indices.present_index].value();
		this->vk_present_command_pool = this->vk_device.createCommandPool(create_info);
	}
	void _init_create_command_buffer()
	{
		vk::CommandBufferAllocateInfo allocate_info{};
		allocate_info.commandBufferCount = 1;

		allocate_info.commandPool = this->vk_compute_command_pool;
		allocate_info.level = vk::CommandBufferLevel::ePrimary;
		check(this->vk_device.allocateCommandBuffers(&allocate_info, &this->vk_compute_command_buffer));

		allocate_info.commandPool = this->vk_transfer_command_pool;
		allocate_info.level = vk::CommandBufferLevel::ePrimary;
		check(this->vk_device.allocateCommandBuffers(&allocate_info, &this->vk_transfer_command_buffer));

		allocate_info.commandPool = this->vk_present_command_pool;
		allocate_info.level = vk::CommandBufferLevel::ePrimary;
		check(this->vk_device.allocateCommandBuffers(&allocate_info, &this->vk_present_command_buffer));
	}
	void _init_create_compute_pipeline()
	{
		vk::PushConstantRange push_consts[1]{};
		push_consts[0].offset = 0;
		push_consts[0].size = 8;
		push_consts[0].stageFlags = vk::ShaderStageFlagBits::eCompute;

		vk::DescriptorSetLayoutBinding bindings[1]{};
		bindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
		bindings[0].binding = 0;
		bindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
		bindings[0].descriptorCount = 1;
		
		vk::DescriptorSetLayoutCreateInfo descriptor_set_info{};
		descriptor_set_info.bindingCount = (uint32_t)std::size(bindings);
		descriptor_set_info.pBindings = bindings;
		this->vk_compute_pipeline_descriptor_set_layout = this->vk_device.createDescriptorSetLayout(descriptor_set_info);

		vk::PipelineLayoutCreateInfo layout_info{};
		layout_info.pPushConstantRanges = push_consts;
		layout_info.pushConstantRangeCount = (uint32_t)std::size(push_consts);
		layout_info.pSetLayouts = &this->vk_compute_pipeline_descriptor_set_layout;
		layout_info.setLayoutCount = 1;
		this->vk_compute_pipeline_layout = this->vk_device.createPipelineLayout(layout_info);

		vk::ComputePipelineCreateInfo create_info{};
		create_info.flags = vk::PipelineCreateFlagBits::eDispatchBase;
		create_info.stage.stage = vk::ShaderStageFlagBits::eCompute;
		create_info.stage.module = this->vk_test_shader;
		create_info.stage.pName = "iota";
		create_info.layout = this->vk_compute_pipeline_layout;

		auto create_result = this->vk_device.createComputePipeline(nullptr, create_info);
		check(create_result.result);
		this->vk_compute_pipeline = std::move(create_result.value);
	}
	void init()
	{
		this->_init_create_instance();

		this->_init_open_window();
		this->_init_create_surface();

		queue_family_indices indices;
		this->_init_physical_device(indices);
		this->_init_device(indices);

		this->_init_queues(indices);
		this->_init_create_command_pool(indices);
		this->_init_create_command_buffer();

		this->load_shader();

		this->_init_create_compute_pipeline();
	}

	void load_shader()
	{
		const static std::string name = "D:\\1\\a.spv";
		std::ifstream shader(name, std::ios::binary | std::ios::in);
		if (!shader.is_open())
			throw std::exception(("Failed to open " + name).data());

		shader.seekg(0, std::ios::_Seekend);
		auto file_size = shader.tellg();

		if (file_size % 4)
			throw std::exception("Invalid shader size");

		shader.seekg(0, std::ios::_Seekbeg);

		std::vector<char> shader_code(file_size, 0);
		shader.read(shader_code.data(), file_size);

		shader.close();

		vk::ShaderModuleCreateInfo create_info;
		create_info.codeSize = file_size;
		create_info.pCode = (uint32_t*)shader_code.data();
		this->vk_test_shader = this->vk_device.createShaderModule(create_info);
	}

	
	vk::DeviceMemory allocate_memory(size_t size, vk::MemoryPropertyFlags mask, vk::MemoryPropertyFlags flags, uint32_t memory_types_mask)
	{
		vk::MemoryAllocateInfo alloc_info{};
		alloc_info.allocationSize = size;

		for (size_t i = 0; i < (size_t)this->vk_device_memory_properties.memoryTypeCount; ++i)
		{
			if (memory_types_mask & (1 << i)) {}
			else continue;

			if ((this->vk_device_memory_properties.memoryTypes[i].propertyFlags & mask) == flags)
			{
				alloc_info.memoryTypeIndex = (uint32_t)i;
				auto memory = this->vk_device.allocateMemory(alloc_info);
				if (memory)
					return memory;
			}
		}

		return vk::DeviceMemory();
	}
	vk::DeviceMemory allocate_local_memory(size_t size, uint32_t memory_types_mask = -1)
	{
		return this->allocate_memory(size, 
			vk::MemoryPropertyFlagBits::eDeviceLocal, 
			vk::MemoryPropertyFlagBits::eDeviceLocal, 
			memory_types_mask);
	}
	vk::DeviceMemory allocate_global_memory(size_t size, uint32_t memory_types_mask = -1)
	{
		return this->allocate_memory(size, 
			vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible, 
			vk::MemoryPropertyFlagBits::eHostVisible, 
			memory_types_mask);
	}
	vk::Buffer allocate_global_buffer(size_t size)
	{
		vk::BufferCreateInfo create_info{};
		create_info.sharingMode = vk::SharingMode::eExclusive;
		create_info.usage = vk::BufferUsageFlagBits::eStorageBuffer;
		create_info.size = size;
		vk::Buffer buffer = this->vk_device.createBuffer(create_info);

		vk::MemoryRequirements reqs = this->vk_device.getBufferMemoryRequirements(buffer);
		vk::DeviceMemory mem = this->allocate_global_memory(reqs.size, reqs.memoryTypeBits);
		if (!mem)
		{
			this->vk_device.destroyBuffer(buffer);
			check((bool)mem);
		}

		this->m_vk_allocations.push_back(mem);
		this->vk_device.bindBufferMemory(buffer, mem, 0);
		return buffer;
	}
	void payload()
	{
		static constexpr int32_t push[] = { 0, -1 };
		
		static constexpr size_t n = 16;

		using intarr = int32_t[n];
		using intarrptr = intarr*;

		intarr arr;

		void* _device_arr;
		auto &device_arr = *(intarrptr*)&_device_arr;


		std::iota(arr, arr + n, 1);
		vk::CommandBufferBeginInfo begin_info{};
		vk::FenceCreateInfo fence_create_info{};
		vk::SubmitInfo submit_info{};
		submit_info.commandBufferCount = 1;

		//vk::Fence transfer_fence = this->vk_device.createFence(fence_create_info);
		vk::Fence compute_fence = this->vk_device.createFence(fence_create_info);


		vk::Buffer buff = this->allocate_global_buffer(sizeof(arr));

		check(this->vk_device.mapMemory(this->m_vk_allocations[0], 0, sizeof(arr), {}, &_device_arr));
		memcpy(device_arr, arr, sizeof(arr));
		this->vk_device.unmapMemory(this->m_vk_allocations[0]);
		check(this->vk_device.mapMemory(this->m_vk_allocations[0], 0, sizeof(arr), {}, &_device_arr));
		//memcpy(device_arr, arr, sizeof(arr));
		this->vk_device.unmapMemory(this->m_vk_allocations[0]);


		//submit_info.pCommandBuffers = &this->vk_transfer_command_buffer;
		//check(this->vk_transfer_queue.submit(1, &submit_info, transfer_fence));



		vk::DescriptorSet sets[1];

		vk::DescriptorPoolSize pool_size{};
		pool_size.descriptorCount = 1;
		pool_size.type = vk::DescriptorType::eStorageBuffer;
		
		vk::DescriptorPoolCreateInfo sets_pool_info{};
		sets_pool_info.maxSets = 1;
		sets_pool_info.poolSizeCount = 1;
		sets_pool_info.pPoolSizes = &pool_size;

		auto sets_pool = this->vk_device.createDescriptorPool(sets_pool_info);

		vk::DescriptorSetAllocateInfo sets_allocate_info{};
		sets_allocate_info.descriptorSetCount = (uint32_t)std::size(sets);
		sets_allocate_info.pSetLayouts = &this->vk_compute_pipeline_descriptor_set_layout;
		sets_allocate_info.descriptorPool = sets_pool;

		check(this->vk_device.allocateDescriptorSets(&sets_allocate_info, sets));

		vk::DescriptorBufferInfo buffer_info{};
		buffer_info.buffer = buff;
		buffer_info.offset = 0;
		buffer_info.range = sizeof(arr);

		vk::WriteDescriptorSet buffer_descriptor{};
		buffer_descriptor.descriptorCount = 1;
		buffer_descriptor.descriptorType = vk::DescriptorType::eStorageBuffer;
		buffer_descriptor.dstArrayElement = 0;
		buffer_descriptor.dstBinding = 0;
		buffer_descriptor.dstSet = sets[0];
		buffer_descriptor.pBufferInfo = &buffer_info;

		this->vk_device.updateDescriptorSets(1, &buffer_descriptor, 0, nullptr);

		this->vk_compute_command_buffer.begin(begin_info);
		this->vk_compute_command_buffer.bindPipeline(vk::PipelineBindPoint::eCompute, this->vk_compute_pipeline);
		this->vk_compute_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, this->vk_compute_pipeline_layout, 0, 1, sets, 0, nullptr);
		this->vk_compute_command_buffer.pushConstants(this->vk_compute_pipeline_layout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(push), push);
		this->vk_compute_command_buffer.dispatch(n, 1, 1);
		this->vk_compute_command_buffer.end();

		submit_info.pCommandBuffers = &this->vk_compute_command_buffer;
		check(this->vk_compute_queue.submit(1, &submit_info, compute_fence));


		check(this->vk_device.waitForFences(1, &compute_fence, VK_FALSE, -1));
		
		check(this->vk_device.mapMemory(this->m_vk_allocations[0], 0, sizeof(arr), {}, &_device_arr));
		this->vk_device.unmapMemory(this->m_vk_allocations[0]);

		this->vk_device.destroyDescriptorPool(sets_pool);
		this->vk_device.destroyBuffer(buff);
	}
public:
	void run()
	{
		this->init();
		this->payload();
	}
};

int main()
{
	main_app_t app;

	try
	{
		app.run();
		return 0;
	}
	catch (const std::exception& excp)
	{
		fprintf(stderr, "Exception caught:\n%s\n", excp.what());
		return 1;
	}
}
