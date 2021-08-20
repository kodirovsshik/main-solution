
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
#include <Windows.h>
#endif


#include <ksn/window.hpp>
#include <ksn/logger.hpp>
#include <ksn/debug_utils.hpp>
#include <ksn/crc.hpp>
#include <ksn/time.hpp>
#include <ksn/math_vec.hpp>
#include <ksn/math_matrix.hpp>

#pragma comment(lib, "libksn_window")
#pragma comment(lib, "libksn_time")
#pragma comment(lib, "libksn_debug_utils")
#pragma comment(lib, "libksn_crc")


#include <vulkan/vulkan.h>

#pragma comment(lib, "vulkan-1.lib")


#include <vector>
#include <unordered_set>
#include <filesystem>
#include <thread>
#include <mutex>
#include <semaphore>
#include <array>


#pragma warning(disable : 26812 4996 )



ksn::file_logger logger;



class exception_with_code
	: public std::runtime_error
{
private:
	int m_code = 0;

public:
	exception_with_code(int code, const char* what = "") noexcept
		: std::runtime_error(what), m_code(code)
	{
	}

	int err() const noexcept { return this->m_code; };
};

#define assert_throw(true_expr, msg, code) if (!(true_expr)) { throw exception_with_code(code, msg); } else ksn::nop()



#define USE_DEBUG_MESSENGER_FOR_RELEASE 1

const std::vector<const char*> vk_validation_layers_data = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> vk_extension_data = 
{ 
#ifdef _WIN32
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
	VK_KHR_SURFACE_EXTENSION_NAME,
#if _KSN_IS_DEBUG_BUILD || USE_DEBUG_MESSENGER_FOR_RELEASE
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};
const std::vector<const char*> vk_device_extension_data =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class main_app_t
{

	static constexpr uint16_t WIDTH = 600, HEIGHT = 600;
	static constexpr uint32_t framerate_limit = 1;


private:
	ksn::window_t window;

	VkInstance vk_instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT vk_debug_messenger = VK_NULL_HANDLE;
	VkPhysicalDevice vk_device_physical = VK_NULL_HANDLE;
	VkDevice vk_device = VK_NULL_HANDLE;
	VkQueue q_graphics = VK_NULL_HANDLE;
	VkQueue q_present = VK_NULL_HANDLE;
	VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
	VkSwapchainKHR vk_swap_chain = VK_NULL_HANDLE;
	std::vector<VkImage> vk_swap_chain_images;
	std::vector<VkImageView> vk_swap_chain_image_views;
	VkFormat vk_swap_chain_format = VkFormat::VK_FORMAT_MAX_ENUM;
	VkExtent2D vk_swapchain_extent{};
	VkPipelineLayout vk_pipeline_layout = VK_NULL_HANDLE;
	VkRenderPass vk_render_pass = VK_NULL_HANDLE;
	VkPipeline vk_graphics_pipeline = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> vk_framebuffers;
	VkCommandPool vk_graphics_command_pool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> vk_command_buffers;
	std::vector<VkSemaphore> vk_semaphores_image_acquired;
	std::vector<VkSemaphore> vk_semaphores_rendering_done;
	std::vector<VkFence> vk_frame_fences;
	std::vector<VkFence> vk_image_fences;
	//std::mutex main_loop_mutex;
	std::binary_semaphore main_loop_semaphore;
	VkBuffer vk_vertex_buffer = VK_NULL_HANDLE;
	VkDeviceMemory vertex_buffer_memory = VK_NULL_HANDLE;

	//Main loop data
	ksn::time fps_update_period = ksn::time::from_msec(250);
	ksn::stopwatch fps_stopwatch;
	size_t fps_counter = 0;

	struct window_data_t
	{
		main_app_t* app;
	};

	std::aligned_storage_t<sizeof(window_data_t), alignof(window_data_t)> window_data_storage;

	uint32_t simultaneous_frames = 2;
	bool framebuffer_resize_pending = false;

	static constexpr bool feature_vk_validation_layers = _KSN_IS_DEBUG_BUILD;

#define vkCreateDebugUtilsMessengerEXT vk_load<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT")
#define vkDestroyDebugUtilsMessengerEXT vk_load<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT")



	struct vertex2_colored_t
	{
		ksn::vec2f pos;
		ksn::vec3f color;

		static void populate_attribute_description(std::array<VkVertexInputAttributeDescription, 2>& data)
		{
			data[0].binding = 0;
			data[0].location = 0;
			data[0].format = VK_FORMAT_R32G32_SFLOAT;
			data[0].offset = offsetof(vertex2_colored_t, pos);

			data[1].binding = 0;
			data[1].location = 1;
			data[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			data[1].offset = offsetof(vertex2_colored_t, color);
		}

		static void populate_binding_description(VkVertexInputBindingDescription* data)
		{
			data->binding = 0;
			data->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			data->stride = sizeof(vertex2_colored_t);
		}
	};

	struct colored_2d_triangle_t
	{
		vertex2_colored_t vertices[3];
	};



	colored_2d_triangle_t my_triangle = 
	{{
		{{0.0, -0.6}, {1,0,0}},
		{{0.692820323, 0.6}, {0,1,0}},
		{{-0.692820323, 0.6}, {0,0,1}},
	}};
	


public:
	void run()
	{
		this->init();
		this->main_loop();
		this->cleanup();
	}

	void cleanup()
	{
		this->_vulkan_cleanup_swap_chain();

		
		if (this->vertex_buffer_memory)
			vkFreeMemory(this->vk_device, this->vertex_buffer_memory, nullptr);
		this->vertex_buffer_memory = VK_NULL_HANDLE;


		if (this->vk_vertex_buffer)
			vkDestroyBuffer(this->vk_device, this->vk_vertex_buffer, nullptr);
		this->vk_vertex_buffer = VK_NULL_HANDLE;

		
		for (auto fence : this->vk_frame_fences)
			vkDestroyFence(this->vk_device, fence, nullptr);
		this->vk_frame_fences.clear();


		for (auto semaphore : this->vk_semaphores_image_acquired)
			vkDestroySemaphore(this->vk_device, semaphore, nullptr);
		this->vk_semaphores_image_acquired.clear();


		for (auto semaphore : this->vk_semaphores_rendering_done)
			vkDestroySemaphore(this->vk_device, semaphore, nullptr);
		this->vk_semaphores_rendering_done.clear();


		if (this->vk_graphics_command_pool)
			vkDestroyCommandPool(this->vk_device, this->vk_graphics_command_pool, nullptr);
		this->vk_graphics_command_pool = VK_NULL_HANDLE;


		if (this->vk_surface)
			vkDestroySurfaceKHR(this->vk_instance, this->vk_surface, nullptr);
		this->vk_surface = VK_NULL_HANDLE;


		if (this->vk_device)
			vkDestroyDevice(this->vk_device, nullptr);
		this->vk_device = VK_NULL_HANDLE;

		
		if (this->vk_debug_messenger)
			vkDestroyDebugUtilsMessengerEXT(this->vk_instance, this->vk_debug_messenger, nullptr);
		this->vk_debug_messenger = VK_NULL_HANDLE;

		if (this->vk_instance) vkDestroyInstance(this->vk_instance, nullptr);
			this->vk_instance = VK_NULL_HANDLE;
	}

	main_app_t()
		: main_loop_semaphore(1)
	{
	}
	~main_app_t()
	{
		this->cleanup();
	}


private:
	
	template<class f_t>
	f_t vk_load(const char* fname)
	{
		f_t func = (f_t)vkGetInstanceProcAddr(this->vk_instance, fname);

		using namespace std::string_literals;
		std::string error = "vkGetInstanceProcAddr has failed for \""s + fname + '\"';
		assert_throw(func, error.data(), VK_ERROR_EXTENSION_NOT_PRESENT);

		return func;
	}


	static VkResult err_checker(VkResult err, const char* what)
	{
		assert_throw(err == VK_SUCCESS, what, err);
		return err;
	}

	void init()
	{
		this->_init_window();
		this->_init_vulkan();

		this->window.show();
	}

	void _init_window()
	{
		ksn::window_style_t style = 0;
		style |= ksn::window_style::border;
		style |= ksn::window_style::caption;
		style |= ksn::window_style::close_button;
		style |= ksn::window_style::hidden;
		style |= ksn::window_style::resize;
		
		ksn::window_open_result_t window_result;
		window_result = this->window.open(WIDTH, HEIGHT, L"Vulkanありますか？", style);
		assert_throw(window_result == ksn::window_open_result::ok, "Failed to open a window", window_result);

		//this->window.set_repeat_resize(true); //TODO
		this->window.set_framerate(framerate_limit);
		this->window.arbitrary_data_set_pointer(&this->window_data_storage, sizeof(this->window_data_storage));
		this->window.set_size_min_height(1);

		window_data_t* p_window_data = (window_data_t*)this->window.arbitrary_data_get_pointer();
		p_window_data->app = this;
	}

	static bool _validation_layers_supported() noexcept
	{
		uint32_t count = 0;
		vkEnumerateInstanceLayerProperties(&count, nullptr);

		std::vector<VkLayerProperties> props(count);
		vkEnumerateInstanceLayerProperties(&count, props.data());

		for (const char* requested : vk_validation_layers_data)
		{
			bool present = false;
			for (const auto& current : props)
			{
				if (strcmp(requested, current.layerName) == 0)
				{
					present = true;
					break;
				}
			}
			if (!present)
				return false;
		}
		return true;
	}

	static bool _extensions_supported() noexcept
	{
		uint32_t count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

		std::vector<VkExtensionProperties> props(count);
		vkEnumerateInstanceExtensionProperties(nullptr, &count, props.data());

		for (const char* requested : vk_extension_data)
		{
			bool present = false;
			for (const auto& current : props)
			{
				if (strcmp(requested, current.extensionName) == 0)
				{
					present = true;
					break;
				}
			}
			if (!present)
				return false;
		}
		return true;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL _vulkan_debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT* data,
		void*)
	{
		if (!_KSN_IS_DEBUG_BUILD && severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			return VK_FALSE;

		if (strstr(data->pMessage, "Physical devices will not be sorted") != nullptr)
			return VK_FALSE; 

		logger.log("%sVulkan debug callback called: %i %i\nMessage: %s\n\n", 
			(severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) ? "[INFO] " : "",
			(int)severity, (int)type, data->pMessage);

		if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			__debugbreak();

		if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			return VK_TRUE;

		return VK_FALSE;
	}

	void _vulkan_create_debug_messenger()
	{
		if (!_KSN_IS_DEBUG_BUILD && !USE_DEBUG_MESSENGER_FOR_RELEASE)
			return;

		VkDebugUtilsMessengerCreateInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		info.messageType = 
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		info.pfnUserCallback = _vulkan_debug_callback;

		err_checker(vkCreateDebugUtilsMessengerEXT(this->vk_instance, &info, nullptr, &this->vk_debug_messenger), "");

		//err_checker(vk_load_and_call<PFN_vkCreateDebugUtilsMessengerEXT>
			//("vkCreateDebugUtilsMessengerEXT", this->vk_instance, &info, nullptr, &this->vk_debug_messenger), "Failed to create a vulkan debug messenger");
	}

	void _vulkan_create_instance()
	{
		//uint32_t temp;
		//vkEnumerateInstanceExtensionProperties(nullptr, &temp, nullptr);
		//std::vector<VkExtensionProperties> exts(temp);
		//vkEnumerateInstanceExtensionProperties(nullptr, &temp, exts.data());

		assert_throw(!feature_vk_validation_layers || _validation_layers_supported(), "Unsupported Vulkan layer(s) requested", -1);
		assert_throw(_extensions_supported(), "Unsupported Vulkan extensions requested", -1);

		VkApplicationInfo info{};
		info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		info.apiVersion = VK_API_VERSION_1_2;
		info.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
		info.pApplicationName = "Vulkan arimasu ka?";
		info.pNext = nullptr;

		VkInstanceCreateInfo info2{};
		info2.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info2.pApplicationInfo = &info;
		info2.enabledExtensionCount = (uint32_t)vk_extension_data.size();
		info2.ppEnabledExtensionNames = vk_extension_data.data();
		info2.enabledLayerCount = (uint32_t)vk_validation_layers_data.size() * (int)feature_vk_validation_layers;
		if (feature_vk_validation_layers) 
			info2.ppEnabledLayerNames = vk_validation_layers_data.data();

		err_checker(vkCreateInstance(&info2, nullptr, &this->vk_instance), "Failed to create a VkInstance");
	}

	void _vulkan_init_device()
	{
		uint32_t count = 0;
		vkEnumeratePhysicalDevices(this->vk_instance, &count, nullptr);

		assert_throw(count != 0, "No Vulkan-compatible devices found", -1);

		std::vector<VkPhysicalDevice> devices(count);
		vkEnumeratePhysicalDevices(this->vk_instance, &count, devices.data());

		devices.resize(std::remove_if(devices.begin(), devices.end(), [&](VkPhysicalDevice dev) { return _check_gpu_unsituable(dev); }) - devices.begin());
		assert_throw(devices.size() != 0, "No Vulkan-compatible situable devices found", -1);

		
		this->vk_device_physical = devices.front();
	}

	struct queue_families
	{
		std::optional<uint32_t> graphics, compute, transfer, present;

		operator bool() noexcept
		{
			return this->graphics.has_value() && this->present.has_value();
		}
	};

	queue_families _get_queue_families(VkPhysicalDevice dev)
	{
		queue_families data;

		uint32_t count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, nullptr);

		std::vector<VkQueueFamilyProperties> props(count);
		vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, props.data());

		int i = 0;
		for (const auto& entry : props)
		{
			VkBool32 present_support = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, this->vk_surface, &present_support);
			if (!present_support)
			{
				data.present.reset();
				continue;
			}

			data.present = i;

			if (entry.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				data.graphics = i;
			if (entry.queueFlags & VK_QUEUE_COMPUTE_BIT)
				data.compute = i;
			if (entry.queueFlags & VK_QUEUE_TRANSFER_BIT)
				data.transfer = i;

			if (data)
				break;

			++i;
		}

		return data;
	}

	bool _check_gpu_unsituable(VkPhysicalDevice dev)
	{
		return !_check_gpu_situable(dev);
	}
	bool _check_gpu_situable(VkPhysicalDevice dev)
	{
		queue_families qfs = _get_queue_families(dev);

		if (!qfs)
			return false;

		uint32_t ext_count = 0;
		vkEnumerateDeviceExtensionProperties(dev, nullptr, &ext_count, nullptr);

		std::vector<VkExtensionProperties> extensions(ext_count);
		vkEnumerateDeviceExtensionProperties(dev, nullptr, &ext_count, extensions.data());

		for (const char* required : vk_device_extension_data)
		{
			bool present = false;
			for (const auto& entry : extensions)
			{
				if (strcmp(required, entry.extensionName) == 0)
				{
					present = true;
					break;
				}
			}
			if (!present)
				return false;
		}

		VkSurfaceCapabilitiesKHR swapchain_capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, this->vk_surface, &swapchain_capabilities);

		if (swapchain_capabilities.minImageCount > 2 || swapchain_capabilities.maxImageCount < 3)
			return false;


		uint32_t nformats = 0, npresent_modes = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(dev, this->vk_surface, &nformats, nullptr);
		vkGetPhysicalDeviceSurfacePresentModesKHR(dev, this->vk_surface, &npresent_modes, nullptr);

		if (nformats == 0 || npresent_modes == 0)
			return false;

		//VkPhysicalDeviceProperties props;
		//VkPhysicalDeviceFeatures features;

		//vkGetPhysicalDeviceProperties(dev, &props);
		//vkGetPhysicalDeviceFeatures(dev, &features);

		return true;
	}

	void _vulkan_create_surface()
	{
#ifdef _WIN32
		VkWin32SurfaceCreateInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		info.hwnd = this->window.window_native_handle();
		info.hinstance = GetModuleHandle(nullptr);
		
		err_checker(vkCreateWin32SurfaceKHR(this->vk_instance, &info, nullptr, &this->vk_surface), "Failed to create a window surface");
#else
		static_assert(false);
#endif
	}

	void _vulkan_setup_logical_device_and_queues()
	{
		queue_families q_families_indexes = _get_queue_families(this->vk_device_physical);

		const std::unordered_set<uint32_t> q_indexes = { q_families_indexes.graphics.value(), q_families_indexes.present.value() };
		std::vector<VkDeviceQueueCreateInfo> q_infos;

		q_infos.reserve(q_indexes.size());

		float priority = 1.f;
		for (uint32_t index : q_indexes)
		{
			VkDeviceQueueCreateInfo q_info{};
			q_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			q_info.queueFamilyIndex = index;
			q_info.queueCount = 1;
			q_info.pQueuePriorities = &priority;
			q_infos.push_back(q_info);
		}

		VkPhysicalDeviceFeatures features{};

		VkDeviceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		info.pQueueCreateInfos = q_infos.data();
		info.queueCreateInfoCount = (uint32_t)q_indexes.size();
		info.pEnabledFeatures = &features;
		info.enabledExtensionCount = (uint32_t)vk_device_extension_data.size();
		info.ppEnabledExtensionNames = vk_device_extension_data.data();

		info.enabledLayerCount = (uint32_t)vk_validation_layers_data.size() * (int)feature_vk_validation_layers;
		if (feature_vk_validation_layers)
			info.ppEnabledLayerNames = vk_validation_layers_data.data();

		err_checker(vkCreateDevice(this->vk_device_physical, &info, nullptr, &this->vk_device), "Failed to create a logical Vulkan device");

		vkGetDeviceQueue(this->vk_device, q_families_indexes.graphics.value(), 0, &this->q_graphics);
		vkGetDeviceQueue(this->vk_device, q_families_indexes.present.value(), 0, &this->q_present);
	}

	void _vulkan_setup_swap_chain()
	{
		//VkResult result;

		uint32_t nformats = 0, npresent_modes = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(this->vk_device_physical, this->vk_surface, &nformats, nullptr);
		vkGetPhysicalDeviceSurfacePresentModesKHR(this->vk_device_physical, this->vk_surface, &npresent_modes, nullptr);

		std::vector<VkSurfaceFormatKHR> surface_formats(nformats);
		std::vector<VkPresentModeKHR> surface_present_modes(npresent_modes);

		vkGetPhysicalDeviceSurfaceFormatsKHR(this->vk_device_physical, this->vk_surface, &nformats, surface_formats.data());
		vkGetPhysicalDeviceSurfacePresentModesKHR(this->vk_device_physical, this->vk_surface, &npresent_modes, surface_present_modes.data());

		VkSurfaceFormatKHR surface_format = surface_formats[0];
		VkPresentModeKHR surface_present_mode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& entry : surface_formats)
		{
			if (entry.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
				entry.format == VK_FORMAT_B8G8R8A8_SRGB)
			{
				surface_format = entry;
				break;
			}
		}

		for (const auto& entry : surface_present_modes)
		{
			if (entry == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				surface_present_mode = entry;
				break;
			}
		}

		VkSurfaceCapabilitiesKHR surface_capabilities;
		err_checker(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->vk_device_physical, this->vk_surface, &surface_capabilities), "Failed to obtain draw surface capabilities");

		auto [win_width, win_height] = this->window.get_client_size();

		VkExtent2D swap_extent = surface_capabilities.currentExtent;

		if (swap_extent.width == UINT32_MAX)
			swap_extent.width = std::clamp((uint32_t)win_width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
		if (swap_extent.height == UINT32_MAX)
			swap_extent.height = std::clamp((uint32_t)win_height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);

		uint32_t swap_chain_image_count = surface_capabilities.maxImageCount ?
			std::max(this->simultaneous_frames, surface_capabilities.minImageCount) :
			std::clamp(this->simultaneous_frames, surface_capabilities.minImageCount, surface_capabilities.maxImageCount);

		if constexpr (true)
		{
			queue_families q_families_indices = this->_get_queue_families(this->vk_device_physical);
			uint32_t q_indices[] = { q_families_indices.graphics.value(), q_families_indices.present.value() };

			VkSwapchainCreateInfoKHR info{};
			info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			info.imageArrayLayers = 1;
			info.imageColorSpace = surface_format.colorSpace;
			info.imageFormat = surface_format.format;
			info.imageExtent = swap_extent;
			info.minImageCount = swap_chain_image_count;
			info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			info.surface = this->vk_surface;

			if (q_families_indices.graphics != q_families_indices.present)
			{
				info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				info.queueFamilyIndexCount = (uint32_t)ksn::countof(q_indices);
				info.pQueueFamilyIndices = q_indices;
			}
			else
				info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

			info.preTransform = surface_capabilities.currentTransform;
			info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			info.clipped = true;


			err_checker(vkCreateSwapchainKHR(this->vk_device, &info, nullptr, &this->vk_swap_chain), "Failed to create a swapchain");

			this->vk_swapchain_extent = swap_extent;
			this->vk_swap_chain_format = surface_format.format;
		}

		vkGetSwapchainImagesKHR(this->vk_device, this->vk_swap_chain, &swap_chain_image_count, nullptr);
		this->vk_swap_chain_images.resize(swap_chain_image_count);
		this->vk_swap_chain_image_views.resize(swap_chain_image_count);
		vkGetSwapchainImagesKHR(this->vk_device, this->vk_swap_chain, &swap_chain_image_count, this->vk_swap_chain_images.data());

		if constexpr (true)
		{
			VkImageViewCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.format = this->vk_swap_chain_format;
			info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			info.subresourceRange.baseArrayLayer = 0;
			info.subresourceRange.baseMipLevel = 0;
			info.subresourceRange.layerCount = 1;
			info.subresourceRange.levelCount = 1;

			for (size_t i = 0; i < swap_chain_image_count; ++i)
			{
				info.image = this->vk_swap_chain_images[i];
				err_checker(vkCreateImageView(this->vk_device, &info, nullptr, &this->vk_swap_chain_image_views[i]), "Failed to create a swap chain image view");
			}
		}
	}

	static std::vector<uint8_t> read_file_bin(const char* name, uint64_t expected_crc)
	{
		size_t size = std::filesystem::file_size(name);
		std::vector<uint8_t> data(size);

		FILE* fd = fopen(name, "rb");
		assert_throw(fd, name, errno);

		fread(data.data(), sizeof(char), size, fd);
		fclose(fd);

		if (ksn::crc64_ecma(data.data(), size) != expected_crc)
			return {};

		return data;
	}

	VkShaderModule _vulkan_create_shader_module(const std::vector<uint8_t> code)
	{
		VkShaderModuleCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.pCode = (uint32_t*)code.data();
		info.codeSize = code.size();

		VkShaderModule shader_module;
		err_checker(vkCreateShaderModule(this->vk_device, &info, nullptr, &shader_module), "Failed to create shader module");
		return shader_module;
	}

	void _vulkan_create_primary_graphics_pipeline()
	{
		auto shader_vertex_code = read_file_bin("resources/shader_vertex.spr", 0x57663803E8BE689A);
		auto shader_fragment_code = read_file_bin("resources/shader_fragment.spr", 0x2A91F8E96103D230);

		assert_throw(shader_vertex_code.size() > 0 && shader_fragment_code.size() > 0, "Failed to create graphics pipeline: failed to read shader data", -1);

		struct _local_sentry_t 
		{
			VkDevice device = VK_NULL_HANDLE;
			VkShaderModule watchee = VK_NULL_HANDLE;
			~_local_sentry_t()
			{ 
				if (this->watchee) 
					vkDestroyShaderModule(this->device, this->watchee, nullptr);
			}
		};

		VkShaderModule shader_vertex = this->_vulkan_create_shader_module(shader_vertex_code);
		_local_sentry_t sentry_vertex{ this->vk_device, shader_vertex };

		VkShaderModule shader_fragment = this->_vulkan_create_shader_module(shader_fragment_code);
		_local_sentry_t sentry_fragment{ this->vk_device, shader_fragment };

		VkPipelineShaderStageCreateInfo shader_stages[2] = { {}, {} };

		VkPipelineShaderStageCreateInfo& shader_vertex_info = shader_stages[0];
		shader_vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_vertex_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		shader_vertex_info.module = shader_vertex;
		shader_vertex_info.pName = "main";

		VkPipelineShaderStageCreateInfo& shader_fragment_info = shader_stages[1];
		shader_fragment_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_fragment_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_fragment_info.module = shader_fragment;
		shader_fragment_info.pName = "main";

		VkVertexInputBindingDescription vertex_input_binding_descriprion;
		vertex2_colored_t::populate_binding_description(&vertex_input_binding_descriprion);

		std::array<VkVertexInputAttributeDescription, 2> vertex_input_attribute_descriprion;
		vertex2_colored_t::populate_attribute_description(vertex_input_attribute_descriprion);

		VkPipelineVertexInputStateCreateInfo vertex_input_info{};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexBindingDescriptionCount = 1;
		vertex_input_info.pVertexBindingDescriptions = &vertex_input_binding_descriprion;
		vertex_input_info.vertexAttributeDescriptionCount = (uint32_t)ksn::countof(vertex_input_attribute_descriprion);
		vertex_input_info.pVertexAttributeDescriptions = vertex_input_attribute_descriprion.data();

		VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
		input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_info.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.width = (float)this->vk_swapchain_extent.width;
		viewport.height = (float)this->vk_swapchain_extent.height;
		viewport.maxDepth = 1;

		VkRect2D scissor{};
		scissor.extent = this->vk_swapchain_extent;

		VkPipelineViewportStateCreateInfo viewport_info{};
		viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_info.viewportCount = 1;
		viewport_info.scissorCount = 1;
		viewport_info.pViewports = &viewport;
		viewport_info.pScissors = &scissor;
		
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

		VkPipelineMultisampleStateCreateInfo multisampling_info{};
		multisampling_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState blend_attachment_info{};
		blend_attachment_info.blendEnable = VK_FALSE;
		blend_attachment_info.colorWriteMask =
			VK_COLOR_COMPONENT_A_BIT | 
			VK_COLOR_COMPONENT_R_BIT | 
			VK_COLOR_COMPONENT_G_BIT | 
			VK_COLOR_COMPONENT_B_BIT;

		VkPipelineColorBlendStateCreateInfo blend_info{};
		blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blend_info.logicOpEnable = VK_FALSE;
		blend_info.attachmentCount = 1;
		blend_info.pAttachments = &blend_attachment_info;

		VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };
		VkPipelineDynamicStateCreateInfo dynamic_state_info{};
		dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_info.dynamicStateCount = (uint32_t)ksn::countof(dynamic_states);
		dynamic_state_info.pDynamicStates = dynamic_states;

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		err_checker(vkCreatePipelineLayout(this->vk_device, &pipeline_layout_info, nullptr, &this->vk_pipeline_layout), "Failed to create pipeline layout");
		


		VkGraphicsPipelineCreateInfo pipeline_info{};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = (uint32_t)ksn::countof(shader_stages);
		pipeline_info.pStages = shader_stages;
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &input_assembly_info;
		pipeline_info.pViewportState = &viewport_info;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pMultisampleState = &multisampling_info;
		pipeline_info.pColorBlendState = &blend_info;
		pipeline_info.layout = this->vk_pipeline_layout;
		pipeline_info.renderPass = this->vk_render_pass;

		err_checker(vkCreateGraphicsPipelines(this->vk_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &this->vk_graphics_pipeline), "Faield to create graphics pipeline");
	}

	void _vulkan_create_rednerpass()
	{
		VkAttachmentDescription color_buffer{};
		color_buffer.format = this->vk_swap_chain_format;
		color_buffer.samples = VK_SAMPLE_COUNT_1_BIT;
		color_buffer.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_buffer.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_buffer.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_buffer.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_buffer_ref{};
		color_buffer_ref.attachment = 0;
		color_buffer_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_buffer_ref;

		VkSubpassDependency subpass_color_attachment_dep{};
		subpass_color_attachment_dep.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpass_color_attachment_dep.dstSubpass = 0;
		subpass_color_attachment_dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_color_attachment_dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_color_attachment_dep.srcAccessMask = 0;
		subpass_color_attachment_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo render_pass_info{};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = 1;
		render_pass_info.pAttachments = &color_buffer;
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &subpass_color_attachment_dep;

		err_checker(vkCreateRenderPass(this->vk_device, &render_pass_info, nullptr, &this->vk_render_pass), "Failed to create render pass");
	}

	void _vulkan_create_framebuffers()
	{
		this->vk_framebuffers.resize(this->vk_swap_chain_images.size());

		VkFramebufferCreateInfo fbinfo{};
		fbinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbinfo.renderPass = this->vk_render_pass;
		fbinfo.attachmentCount = 1;
		fbinfo.width = this->vk_swapchain_extent.width;
		fbinfo.height = this->vk_swapchain_extent.height;
		fbinfo.layers = 1;

		for (size_t i = 0; i < this->vk_swap_chain_images.size(); ++i)
		{
			fbinfo.pAttachments = &this->vk_swap_chain_image_views[i];
			err_checker(vkCreateFramebuffer(this->vk_device, &fbinfo, nullptr, &this->vk_framebuffers[i]), "Failed to create framebuffer");
		}
	}

	void _vulkan_create_command_pool()
	{
		auto indices = this->_get_queue_families(this->vk_device_physical);

		VkCommandPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.queueFamilyIndex = indices.graphics.value();
		info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		err_checker(vkCreateCommandPool(this->vk_device, &info, nullptr, &this->vk_graphics_command_pool), "Failed to create graphics command pool");
	}

	void _vulkan_create_command_buffers()
	{
		this->vk_command_buffers.resize(this->vk_framebuffers.size());

		VkCommandBufferAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.commandPool = this->vk_graphics_command_pool;
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		info.commandBufferCount = (uint32_t)this->vk_command_buffers.size();

		err_checker(vkAllocateCommandBuffers(this->vk_device, &info, this->vk_command_buffers.data()), "Failed to create command buffers");

		VkCommandBufferBeginInfo buffer_begin_info{};
		buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkClearValue clear = { 0.f, 0.f, 0.f, 1.f };

		VkRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = this->vk_render_pass;
		render_pass_begin_info.renderArea.extent = this->vk_swapchain_extent;
		render_pass_begin_info.renderArea.offset = { 0, 0 };
		render_pass_begin_info.clearValueCount = 1;
		render_pass_begin_info.pClearValues = &clear;


		for (size_t i = 0; i < this->vk_command_buffers.size(); ++i)
		{
			err_checker(vkBeginCommandBuffer(this->vk_command_buffers[i], &buffer_begin_info), "vkBeginCommandBuffer has failed");

			render_pass_begin_info.framebuffer = this->vk_framebuffers[i];
			vkCmdBeginRenderPass(this->vk_command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(this->vk_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->vk_graphics_pipeline);

			VkDeviceSize vertex_buffer_offset = 0;
			vkCmdBindVertexBuffers(this->vk_command_buffers[i], 0, 1, &this->vk_vertex_buffer, &vertex_buffer_offset);

			vkCmdDraw(this->vk_command_buffers[i], (uint32_t)ksn::countof(this->my_triangle.vertices), 1, 0, 0);

			vkCmdEndRenderPass(this->vk_command_buffers[i]);

			err_checker(vkEndCommandBuffer(this->vk_command_buffers[i]), "vkEndCommandBuffer has failed");
		}
	}

	void _vulkan_cleanup_swap_chain()
	{
		if (this->vk_device)
			vkDeviceWaitIdle(this->vk_device);


		if (this->vk_graphics_command_pool && this->vk_command_buffers.size())
			vkFreeCommandBuffers(this->vk_device, this->vk_graphics_command_pool, (uint32_t)this->vk_command_buffers.size(), this->vk_command_buffers.data());
		this->vk_command_buffers.clear();


		for (auto entry : this->vk_framebuffers)
			vkDestroyFramebuffer(this->vk_device, entry, nullptr);
		this->vk_framebuffers.clear();


		if (this->vk_graphics_pipeline)
			vkDestroyPipeline(this->vk_device, this->vk_graphics_pipeline, nullptr);
		this->vk_graphics_pipeline = VK_NULL_HANDLE;


		if (this->vk_render_pass)
			vkDestroyRenderPass(this->vk_device, this->vk_render_pass, nullptr);
		this->vk_render_pass = VK_NULL_HANDLE;


		if (this->vk_pipeline_layout)
			vkDestroyPipelineLayout(this->vk_device, this->vk_pipeline_layout, nullptr);
		this->vk_pipeline_layout = VK_NULL_HANDLE;


		for (auto view : this->vk_swap_chain_image_views)
			vkDestroyImageView(this->vk_device, view, nullptr);
		this->vk_swap_chain_image_views.clear();


		if (this->vk_swap_chain)
			vkDestroySwapchainKHR(this->vk_device, this->vk_swap_chain, nullptr);
		this->vk_swap_chain = VK_NULL_HANDLE;
	}

	void _vulkan_recreate_swap_chain()
	{
		this->_vulkan_cleanup_swap_chain();

		this->_vulkan_setup_swap_chain();
		this->_vulkan_create_rednerpass();
		this->_vulkan_create_primary_graphics_pipeline();
		this->_vulkan_create_framebuffers();
		this->_vulkan_create_command_buffers();
	}

	void _vulkan_create_vertex_buffers()
	{
		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = sizeof(my_triangle.vertices);
		info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		err_checker(vkCreateBuffer(this->vk_device, &info, nullptr, &this->vk_vertex_buffer), "Failed to create vertex buffer");

		VkMemoryRequirements mem;
		vkGetBufferMemoryRequirements(this->vk_device, this->vk_vertex_buffer, &mem);
		
		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem.size;
		alloc_info.memoryTypeIndex = this->_find_memory_type(
			mem.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		
		err_checker(vkAllocateMemory(this->vk_device, &alloc_info, nullptr, &this->vertex_buffer_memory), "Failed to allocate videomemory");
		vkBindBufferMemory(this->vk_device, this->vk_vertex_buffer, this->vertex_buffer_memory, 0);

		this->_vulkan_update_vertex_buffers();
	}

	void _vulkan_update_vertex_buffers()
	{
		void* buffer_data;
		vkMapMemory(this->vk_device, this->vertex_buffer_memory, 0, sizeof(this->my_triangle.vertices), 0, &buffer_data);
		memcpy(buffer_data, &this->my_triangle.vertices, sizeof(this->my_triangle.vertices));
		vkUnmapMemory(this->vk_device, this->vertex_buffer_memory);
	}

	uint32_t _find_memory_type(uint32_t filter, VkMemoryPropertyFlags props)
	{
		static VkPhysicalDevice _device = this->vk_device_physical;
		static VkPhysicalDeviceMemoryProperties mem_properties{};
		static int __init = [&]
		{
			vkGetPhysicalDeviceMemoryProperties(_device, &mem_properties);
			return 0;
		}();

		for (uint32_t bit = 0; bit < mem_properties.memoryTypeCount; ++bit)
		{
			if ((filter & (1 << bit)) && (mem_properties.memoryTypes[bit].propertyFlags & props) == props)
				return bit;
		}

		throw exception_with_code(-1, "Failed to find situable memory type");
	}

	void vulkan_start()
	{
		throw;
		VkCommandBufferBeginInfo binfo{};
		binfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkClearValue clear_color = { 0.f, 0.f, 0.f, 1.f };

		VkRenderPassBeginInfo rpinfo{};
		rpinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpinfo.renderPass = this->vk_render_pass;
		rpinfo.renderArea.offset = { 0, 0 };
		rpinfo.renderArea.extent = this->vk_swapchain_extent;
		rpinfo.clearValueCount = 1;
		rpinfo.pClearValues = &clear_color;

		size_t i = 0;
		for (auto buffer : this->vk_command_buffers)
		{
			err_checker(vkBeginCommandBuffer(buffer, &binfo), "Failed to begin a command buffer");

			rpinfo.framebuffer = this->vk_framebuffers[i];
			vkCmdBeginRenderPass(buffer, &rpinfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->vk_graphics_pipeline);

			++i;
		}
	}

	void vulkan_draw()
	{
		throw;
		for (auto buffer : this->vk_command_buffers)
		{
			vkCmdDraw(buffer, 3, 1, 0, 0);
		}
	}

	void vulkan_end()
	{
		throw;
		for (auto buffer : this->vk_command_buffers)
		{
			vkCmdEndRenderPass(buffer);
			err_checker(vkEndCommandBuffer(buffer), "vkEndCommandBuffer has failed");
		}
	}

	void display()
	{
		uint32_t image_index = -1;
		static uint32_t current_frame = 0;
		
		vkWaitForFences(this->vk_device, 1, &this->vk_frame_fences[current_frame], VK_FALSE, UINT64_MAX);

		if (this->framebuffer_resize_pending)
		{
			this->framebuffer_resize_pending = false;
			this->_vulkan_recreate_swap_chain();
		}

		VkResult vk_result;
		while (true)
		{
			vk_result = vkAcquireNextImageKHR(this->vk_device, this->vk_swap_chain, UINT64_MAX, this->vk_semaphores_image_acquired[current_frame], VK_NULL_HANDLE, &image_index);
			if (vk_result != VK_ERROR_OUT_OF_DATE_KHR)
				break;
			this->_vulkan_recreate_swap_chain();
		};

		if (vk_result != VK_SUBOPTIMAL_KHR)
			err_checker(vk_result, "vkAcquireNextImageKHR has failed, failed to acquire next render buffer");
		

		if (this->vk_image_fences[image_index])
			vkWaitForFences(this->vk_device, 1, &this->vk_image_fences[image_index], VK_FALSE, UINT64_MAX);

		this->vk_image_fences[image_index] = this->vk_frame_fences[current_frame];

		VkSemaphore semaphores_to_wait_for[] = { this->vk_semaphores_image_acquired[current_frame] };
		VkSemaphore semaphores_to_signal_on[] = { this->vk_semaphores_rendering_done[current_frame] };
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		static_assert(ksn::countof(semaphores_to_wait_for) == ksn::countof(wait_stages), u8"ඞ??");

		

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = (uint32_t)ksn::countof(semaphores_to_wait_for);
		submit_info.pWaitSemaphores = semaphores_to_wait_for;
		submit_info.signalSemaphoreCount = (uint32_t)ksn::countof(semaphores_to_signal_on);
		submit_info.pSignalSemaphores = semaphores_to_signal_on;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &this->vk_command_buffers[image_index];
		submit_info.pWaitDstStageMask = wait_stages;

		vkResetFences(this->vk_device, 1, &this->vk_frame_fences[current_frame]);
		err_checker(vkQueueSubmit(this->q_graphics, 1, &submit_info, this->vk_frame_fences[current_frame]), "Failed to sumbit commands to graphics queue");

		VkSemaphore presentation_wait_semaphores[] = { this->vk_semaphores_rendering_done[current_frame] };
		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pImageIndices = &image_index;
		present_info.pSwapchains = &this->vk_swap_chain;
		present_info.pWaitSemaphores = presentation_wait_semaphores;
		present_info.swapchainCount = 1;
		present_info.waitSemaphoreCount = (uint32_t)ksn::countof(presentation_wait_semaphores);
		
		vk_result = vkQueuePresentKHR(this->q_present, &present_info);

		if (vk_result == VK_ERROR_OUT_OF_DATE_KHR || vk_result == VK_SUBOPTIMAL_KHR)
			this->_vulkan_recreate_swap_chain();
		else
			err_checker(vk_result, "Presentation error");
		
		current_frame = (current_frame + 1) % this->simultaneous_frames;
	}

	void _vulkan_create_sync_objects()
	{
		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		this->vk_semaphores_image_acquired.resize(this->simultaneous_frames);
		this->vk_semaphores_rendering_done.resize(this->simultaneous_frames);
		this->vk_frame_fences.resize(this->simultaneous_frames);
		this->vk_image_fences.resize(this->vk_swap_chain_images.size(), VK_NULL_HANDLE);

		for (size_t i = 0; i < this->simultaneous_frames; ++i)
		{
			err_checker(vkCreateSemaphore(this->vk_device, &semaphore_info, nullptr, &this->vk_semaphores_image_acquired[i]), "Failed to create VkSemaphore object");
			err_checker(vkCreateSemaphore(this->vk_device, &semaphore_info, nullptr, &this->vk_semaphores_rendering_done[i]), "Failed to create VkSemaphore object");
			err_checker(vkCreateFence(this->vk_device, &fence_info, nullptr, &this->vk_frame_fences[i]), "Failed to create VkFence object");
		}
	}

	void _init_vulkan()
	{
		this->_vulkan_create_instance();
		this->_vulkan_create_debug_messenger();
		this->_vulkan_create_surface();
		this->_vulkan_init_device();
		this->_vulkan_setup_logical_device_and_queues();
		this->_vulkan_create_command_pool();
		this->_vulkan_create_vertex_buffers();
		this->_vulkan_recreate_swap_chain();
		this->_vulkan_create_sync_objects();
	}

	void _main_loop_update()
	{
		//Update
		//...


		//Aux
		if (this->fps_stopwatch.current() > this->fps_update_period)
		{
			char title[32];
			snprintf(title, ksn::countof(title), "%zu FPS", this->fps_counter);

			this->fps_counter = 0;
			this->fps_stopwatch.restart();
		}
		else
			++this->fps_counter;
	}

	bool _main_loop_render()
	{
		//Actual rendering
		//...


		//Display
		try
		{
			this->display();
		}
		catch (const exception_with_code& excp)
		{
			if (excp.err() == VK_ERROR_SURFACE_LOST_KHR)
				return false;
			throw;
		}

		return true;
	}

	bool _main_loop_poll()
	{
		ksn::event_t ev;

		//printf("polling\n");
		while (this->window.poll_event(ev))
		{
			//printf("polled %s\n", ev.to_string());

			switch (ev.type)
			{
			case ksn::event_type_t::close:
				this->window.close();
				break;

			case ksn::event_type_t::keyboard_press:
				switch (ev.keyboard_button_data.button)
				{
				case ksn::keyboard_button_t::esc:
					this->window.close();
					break;
				}
				break;

			case ksn::event_type_t::resize:
				//this->framebuffer_resize_pending = true;
				break;
			}
		}
		//printf("pollingn't\n");

		return this->window.is_open();
	}

	static void _resizemove_handle(const ksn::resizemove_data_t* data)
	{
		window_data_t* window_data = static_cast<window_data_t*>(data->window->arbitrary_data_get_pointer());
		main_app_t* app = window_data->app;

		app->framebuffer_resize_pending = true;

		ksn::stopwatch sw;
		sw.start();

		app->main_loop_semaphore.acquire();
		app->_main_loop_update();
		//auto dt2 = sw.restart();
		app->_main_loop_render();
		//auto dt4 = sw.restart();
		app->main_loop_semaphore.release();

		//printf("%g %g\n", dt2.as_float_sec() * 1e3f, dt4.as_float_sec() * 1e3f);
	}

	static void _main_loop_worker(std::stop_token stop_token, main_app_t* app)
	{
		app->_main_loop_render();
		printf("0");
		while (!stop_token.stop_requested())
		{
			printf("A");
			app->window.tick();

			printf("B");
			app->main_loop_semaphore.acquire();
			printf("C");
			app->_main_loop_update();
			printf("D");
			app->_main_loop_render();
			printf("E");
			app->main_loop_semaphore.release();
			printf("F");
		}
	}

	void main_loop()
	{
		std::jthread main_loop_worker_thread(_main_loop_worker, this);

		this->window.set_resizemode_handle(_resizemove_handle);

		while (true)
		{
			bool keep_going = this->_main_loop_poll();

			if (!keep_going)
			{
				break;
			}
		}

		main_loop_worker_thread.request_stop();
	}

};

int main()
{

	try
	{
		assert_throw(logger.add_file(stderr) == ksn::file_logger::add_ok, "Failed to add stderr to the logger", -1);
		assert_throw(logger.add_file("log.txt", "w") == ksn::file_logger::add_ok, "Failed to add log.txt to the logger", -1);

		main_app_t app;
		app.run();
	}
	catch (const exception_with_code& excp)
	{
		logger.log("Unhandled exception: %s\nCode = %i\n", excp.what(), excp.err());

		const char* p_err_descriprion = nullptr;

		switch (excp.err())
		{
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			p_err_descriprion = "Incompatible Vulkan driver found\nMaybe update your videocard drivers?\n";
			break;
			
		default:
			break;
		}

		if (p_err_descriprion)
			logger.log(p_err_descriprion);

		return excp.err();
	}
	catch (const std::exception& excp)
	{
		logger.log("Unhandled exception: %s", excp.what());
		return -1;
	}
	
	return 0;
}
