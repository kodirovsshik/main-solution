
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
#include <Windows.h>
#endif


#include <ksn/window.hpp>
#include <ksn/logger.hpp>
#include <ksn/debug_utils.hpp>

#pragma comment(lib, "libksn_window")
#pragma comment(lib, "libksn_time")
#pragma comment(lib, "libksn_debug_utils")


#include <vulkan/vulkan.h>

#pragma comment(lib, "vulkan-1.lib")


#include <vector>
#include <unordered_set>


#pragma warning(disable : 26812 4996 )



ksn::file_logger logger;



class exception_with_code
	: public std::exception
{
private:
	int m_code = 0;

public:
	exception_with_code(int code, const char* what = "") noexcept
		: std::exception(what), m_code(code)
	{
	}

	int err() const noexcept { return this->m_code; };
};

#define assert_throw(true_expr, msg, code) if ((true_expr) == false) { throw exception_with_code(code, msg); } else ksn::nop()



const std::vector<const char*> vk_validation_layers_data = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> vk_extension_data = 
{ 
#ifdef _WIN32
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
	VK_KHR_SURFACE_EXTENSION_NAME,
#if _KSN_IS_DEBUG_BUILD
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
};
const std::vector<const char*> vk_device_extension_data =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class main_app_t
{

	static constexpr uint16_t WIDTH = 800, HEIGHT = 600;


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

	static constexpr bool feature_vk_validation_layers = _KSN_IS_DEBUG_BUILD;


public:
	void run()
	{
		this->init();
		this->main_loop();
		this->cleanup();
	}

	void cleanup()
	{
		for (auto view : this->vk_swap_chain_image_views)
			vkDestroyImageView(this->vk_device, view, nullptr);
		this->vk_swap_chain_image_views.resize(0);

		if (this->vk_swap_chain) vkDestroySwapchainKHR(this->vk_device, this->vk_swap_chain, nullptr);
		this->vk_swap_chain = VK_NULL_HANDLE;

		if (this->vk_surface) vkDestroySurfaceKHR(this->vk_instance, this->vk_surface, nullptr);
		this->vk_surface = VK_NULL_HANDLE;

		if (this->vk_device) vkDestroyDevice(this->vk_device, nullptr);
		this->vk_device = VK_NULL_HANDLE;

		if (this->vk_debug_messenger) vk_load_and_call<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT", this->vk_instance, this->vk_debug_messenger, nullptr);
		this->vk_debug_messenger = VK_NULL_HANDLE;

		if (this->vk_instance) vkDestroyInstance(this->vk_instance, nullptr);
		this->vk_instance = VK_NULL_HANDLE;
	}

	main_app_t()
	{
	}
	~main_app_t()
	{
		this->cleanup();
	}


private:
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
		
		ksn::window_open_result_t window_result;
		window_result = this->window.open(WIDTH, HEIGHT, L"Vulkanありますか？", style);
		assert_throw(window_result == ksn::window_open_result::ok, "Failed to open a window", window_result);

		this->window.set_framerate(60);
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

		if (severity != VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			ksn::nop();

		if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			return VK_TRUE;

		return VK_FALSE;
	}

	template<class f_t, class... args_t>
	auto vk_load_and_call(const char* fname, args_t&& ...args)
	{
		f_t func = (f_t)vkGetInstanceProcAddr(this->vk_instance, fname);
		if (func)
			return func(std::forward<args_t>(args)...);
		else
			return (decltype(func(std::forward<args_t>(args)...)))VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void _vulkan_create_debug_messenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | 
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		info.messageType = 
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		info.pfnUserCallback = _vulkan_debug_callback;

		err_checker(vk_load_and_call<PFN_vkCreateDebugUtilsMessengerEXT>
			("vkCreateDebugUtilsMessengerEXT", this->vk_instance, &info, nullptr, &this->vk_debug_messenger), "Failed to create a vulkan debug messenger");
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
		info.engineVersion = VK_MAKE_VERSION(1, 2, 0);
		info.pApplicationName = "Vulkan arimasu ka?";
		info.pEngineName = "";
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

	void _vulkan_setup_swap_chain(bool tripple_buffering = true)
	{
		uint32_t nformats = 0, npresent_modes = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(this->vk_device_physical, this->vk_surface, &nformats, nullptr);
		vkGetPhysicalDeviceSurfacePresentModesKHR(this->vk_device_physical, this->vk_surface, &npresent_modes, nullptr);

		std::vector<VkSurfaceFormatKHR> surface_formats(nformats);
		std::vector<VkPresentModeKHR> surface_present_modes(npresent_modes);

		vkGetPhysicalDeviceSurfaceFormatsKHR(this->vk_device_physical, this->vk_surface, &nformats, surface_formats.data());
		vkGetPhysicalDeviceSurfacePresentModesKHR(this->vk_device_physical, this->vk_surface, &npresent_modes, surface_present_modes.data());

		VkSurfaceFormatKHR surface_format = surface_formats[0];
		//VkPresentModeKHR surface_present_mode = surface_present_modes[0];
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
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->vk_device_physical, this->vk_surface, &surface_capabilities);

		auto [win_width, win_height] = this->window.get_client_size();

		VkExtent2D swap_extent = surface_capabilities.currentExtent;

		if (swap_extent.width == UINT32_MAX)
			swap_extent.width = std::clamp((uint32_t)win_width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
		if (swap_extent.height == UINT32_MAX)
			swap_extent.height = std::clamp((uint32_t)win_height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);


		uint32_t swap_chain_image_count_wanted = tripple_buffering ? 3 : 2;

		uint32_t swap_chain_image_count = surface_capabilities.maxImageCount ?
			std::max(swap_chain_image_count_wanted, surface_capabilities.minImageCount) :
			std::clamp(swap_chain_image_count_wanted, surface_capabilities.minImageCount, surface_capabilities.maxImageCount);

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
				info.queueFamilyIndexCount = ksn::countof(q_indices);
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

	void _init_vulkan()
	{
		this->_vulkan_create_instance();
		this->_vulkan_create_debug_messenger();
		this->_vulkan_create_surface();
		this->_vulkan_init_device();
		this->_vulkan_setup_logical_device_and_queues();
		this->_vulkan_setup_swap_chain();
	}

	void main_loop()
	{

		while (true)
		{
			//Render, poll, update

			//Render
			//...
			this->window.tick();


			//Poll

			ksn::event_t ev;
			while (this->window.poll_event(ev))
			{
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
				}
			}

			if (!this->window.is_open())
				break;

			//Update
			//...
		}

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
		return excp.err();
	}
	catch (const std::exception& excp)
	{
		logger.log("Unhandled exception: %s", excp.what());
		return -1;
	}
	
	return 0;
}
