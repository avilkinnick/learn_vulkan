#include <volk.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PRINT_VK_DEBUG_MESSAGE_SEVERITY_BIT(severity, bit)             \
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_##bit##_BIT_EXT)    \
    {                                                                  \
        fputs("["#bit"]", stdout);                                     \
    }

#define PRINT_VK_DEBUG_MESSAGE_TYPE_BIT(types, bit)             \
    if (types & VK_DEBUG_UTILS_MESSAGE_TYPE_##bit##_BIT_EXT)    \
    {                                                           \
        fputs("["#bit"]", stdout);                              \
    }

#define PRINT_VK_QUEUE_FLAG_BIT(flags, bit)    \
    if (flags & VK_QUEUE_##bit##_BIT)          \
    {                                          \
        fputs(#bit" | ", stdout);              \
    }

#define PRINT_VK_QUEUE_FLAG_BIT_EXT(flags, bit, ext)    \
    if (flags & VK_QUEUE_##bit##_BIT_##ext)             \
    {                                                   \
        fputs(#bit"_"#ext" | ", stdout);                \
    }

VkLayerProperties* vk_instance_layer_properties = NULL;
VkInstance vk_instance = NULL;
VkDebugUtilsMessengerEXT vk_debug_messenger = NULL;
VkPhysicalDevice* vk_physical_devices = NULL;
VkPhysicalDevice vk_physical_device = NULL;
VkPhysicalDeviceGroupProperties* vk_physical_device_group_properties = NULL;

static void cleanup(void)
{
    free(vk_physical_device_group_properties);
    vk_physical_device_group_properties = NULL;

    free(vk_physical_devices);
    vk_physical_devices = NULL;

    if (vk_debug_messenger != NULL)
    {
        vkDestroyDebugUtilsMessengerEXT(vk_instance, vk_debug_messenger, NULL);
        vk_debug_messenger = NULL;
    }

    if (vk_instance != NULL)
    {
        vkDestroyInstance(vk_instance, NULL);
        vk_instance = NULL;
    }

    free(vk_instance_layer_properties);
    vk_instance_layer_properties = NULL;
}

static void print_vk_version(const uint32_t version)
{
    printf("%u.%u.%u.%u", VK_API_VERSION_VARIANT(version),
                          VK_API_VERSION_MAJOR(version),
                          VK_API_VERSION_MINOR(version),
                          VK_API_VERSION_PATCH(version));
}

static void indent(const int n)
{
    for (int i = 0; i < n; ++i)
    {
        fputs("    ", stdout);
    }
}

static VkBool32 vk_debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
)
{
    (void)pUserData;

    PRINT_VK_DEBUG_MESSAGE_SEVERITY_BIT(messageSeverity, VERBOSE);
    PRINT_VK_DEBUG_MESSAGE_SEVERITY_BIT(messageSeverity, INFO);
    PRINT_VK_DEBUG_MESSAGE_SEVERITY_BIT(messageSeverity, WARNING);
    PRINT_VK_DEBUG_MESSAGE_SEVERITY_BIT(messageSeverity, ERROR);

    PRINT_VK_DEBUG_MESSAGE_TYPE_BIT(messageTypes, GENERAL);
    PRINT_VK_DEBUG_MESSAGE_TYPE_BIT(messageTypes, VALIDATION);
    PRINT_VK_DEBUG_MESSAGE_TYPE_BIT(messageTypes, PERFORMANCE);
    PRINT_VK_DEBUG_MESSAGE_TYPE_BIT(messageTypes, DEVICE_ADDRESS_BINDING);

    printf(" %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

typedef enum ExtensionType
{
    EXTENSION_TYPE_INSTANCE,
    EXTENSION_TYPE_DEVICE
} ExtensionType;

static void print_vk_extensions(
    const ExtensionType type,
    VkPhysicalDevice physical_device,
    const char* const layer_name,
    const int indentation
)
{
    uint32_t extension_property_count = 0;
    const char* header = NULL;

    switch (type)
    {
        case EXTENSION_TYPE_INSTANCE:
        {
            vkEnumerateInstanceExtensionProperties(layer_name,
                &extension_property_count, NULL);

            header = "Vulkan instance extensions";
            break;
        }
        case EXTENSION_TYPE_DEVICE:
        {
            vkEnumerateDeviceExtensionProperties(physical_device, layer_name,
                &extension_property_count, NULL);

            header = "Vulkan device extensions";
            break;
        }
        default:
        {
            fputs("Wrong extension type\n", stderr);
            exit(EXIT_FAILURE);
        }
    }

    if (extension_property_count == 0)
    {
        return;
    }

    VkExtensionProperties* extension_properties = malloc(
        sizeof(VkExtensionProperties) * extension_property_count);

    if (extension_properties == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for %s\n", header);
        exit(EXIT_FAILURE);
    }

    switch (type)
    {
        case EXTENSION_TYPE_INSTANCE:
        {
            vkEnumerateInstanceExtensionProperties(layer_name,
                &extension_property_count, extension_properties);

            break;
        }
        case EXTENSION_TYPE_DEVICE:
        {
            vkEnumerateDeviceExtensionProperties(physical_device, layer_name,
                &extension_property_count, extension_properties);

            break;
        }
    }

    indent(indentation);
    printf("%s[%u]:\n", header, extension_property_count);

    for (uint32_t i = 0; i < extension_property_count; ++i)
    {
        const VkExtensionProperties* const properties =
            &extension_properties[i];

        indent(indentation + 1);
        printf("%s v.%u\n", properties->extensionName, properties->specVersion);
    }

    free(extension_properties);
    extension_properties = NULL;
}

int main(void)
{
    atexit(cleanup);

    if (volkInitialize() != VK_SUCCESS)
    {
        fputs("Failed to initialize volk\n", stderr);
        return EXIT_FAILURE;
    }

    uint32_t vk_instance_version = 0;
    vkEnumerateInstanceVersion(&vk_instance_version);

    fputs("vk_instance_version: ", stdout);
    print_vk_version(vk_instance_version);
    putc('\n', stdout);

    uint32_t vk_instance_layer_property_count = 0;
    vkEnumerateInstanceLayerProperties(&vk_instance_layer_property_count, NULL);

    if (vk_instance_layer_property_count > 0)
    {
        vk_instance_layer_properties = malloc(sizeof(VkLayerProperties) *
            vk_instance_layer_property_count);

        if (vk_instance_layer_properties == NULL)
        {
            fputs("Failed to allocate memory for "
                "vk_instance_layer_properties\n", stderr);

            return EXIT_FAILURE;
        }

        vkEnumerateInstanceLayerProperties(&vk_instance_layer_property_count,
            vk_instance_layer_properties);

        printf("vk_instance_layer_properties[%u]:\n",
            vk_instance_layer_property_count);

        for (uint32_t i = 0; i < vk_instance_layer_property_count; ++i)
        {
            const VkLayerProperties* const properties =
                &vk_instance_layer_properties[i];

            indent(1);
            printf("%s:\n", properties->layerName);

            indent(2);
            fputs("Spec version: ", stdout);
            print_vk_version(properties->specVersion);
            putc('\n', stdout);

            indent(2);
            printf("Implementation version: %u\n",
                properties->implementationVersion);

            indent(2);
            printf("Description: %s\n", properties->description);

            print_vk_extensions(EXTENSION_TYPE_INSTANCE, NULL,
                properties->layerName, 2);
        }
    }

    print_vk_extensions(EXTENSION_TYPE_INSTANCE, NULL, NULL, 0);

    const VkDebugUtilsMessengerCreateInfoEXT vk_debug_messenger_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = NULL,
        .flags = 0,
        .messageSeverity = // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
        .pfnUserCallback = vk_debug_messenger_callback,
        .pUserData = NULL
    };

    const VkApplicationInfo vk_application_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = NULL,
        .applicationVersion = 0,
        .pEngineName = NULL,
        .engineVersion = 0,
        .apiVersion = vk_instance_version
    };

    const char* const vk_enabled_instance_extension_names[] = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME
    };

    const VkInstanceCreateInfo vk_instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &vk_debug_messenger_create_info,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &vk_application_info,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = sizeof vk_enabled_instance_extension_names /
            sizeof(const char*),
        .ppEnabledExtensionNames = vk_enabled_instance_extension_names
    };

    if (vkCreateInstance(&vk_instance_create_info, NULL, &vk_instance) !=
        VK_SUCCESS)
    {
        fputs("Failed to create Vulkan instance\n", stderr);
        return EXIT_FAILURE;
    }

    volkLoadInstance(vk_instance);

    if (vkCreateDebugUtilsMessengerEXT(vk_instance,
        &vk_debug_messenger_create_info, NULL, &vk_debug_messenger) !=
        VK_SUCCESS)
    {
        fputs("Failed to create Vulkan debug messenger\n", stderr);
        return EXIT_FAILURE;
    }

    uint32_t vk_physical_device_count = 0;
    vkEnumeratePhysicalDevices(vk_instance, &vk_physical_device_count, NULL);

    if (vk_physical_device_count == 0)
    {
        fputs("Failed to find physical devices\n", stderr);
        return EXIT_FAILURE;
    }

    vk_physical_devices = malloc(sizeof(VkPhysicalDevice) *
        vk_physical_device_count);

    if (vk_physical_devices == NULL)
    {
        fputs("Failed to allocate memory for vk_physical_devices\n", stderr);
        return EXIT_FAILURE;
    }

    vkEnumeratePhysicalDevices(vk_instance, &vk_physical_device_count,
        vk_physical_devices);

    printf("vk_physical_devices[%u]:\n", vk_physical_device_count);

    for (uint32_t i = 0; i < vk_physical_device_count; ++i)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(vk_physical_devices[i], &properties);

        indent(1);
        printf("%s:\n", properties.deviceName);

        indent(2);
        fputs("API version: ", stdout);
        print_vk_version(properties.apiVersion);
        putc('\n', stdout);

        indent(2);
        printf("Driver version: %u\n", properties.driverVersion);

        indent(2);
        printf("Vendor ID: 0x%X\n", properties.vendorID);

        indent(2);
        printf("Device ID: %u\n", properties.deviceID);

        indent(2);
        fputs("Device type: ", stdout);
        switch (properties.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            {
                fputs("OTHER", stdout);
                break;
            }
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            {
                fputs("INTEGRATED_GPU", stdout);
                break;
            }
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            {
                fputs("DISCRETE_GPU", stdout);
                break;
            }
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            {
                fputs("VIRTUAL_GPU", stdout);
                break;
            }
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
            {
                fputs("CPU", stdout);
                break;
            }
            default:
            {
                fputs("WRONG", stdout);
                break;
            }
        }
        putc('\n', stdout);
    }

    vk_physical_device = vk_physical_devices[0];

    uint32_t vk_physical_device_group_count = 0;
    vkEnumeratePhysicalDeviceGroups(vk_instance,
        &vk_physical_device_group_count, NULL);

    if (vk_physical_device_group_count > 0)
    {
        vk_physical_device_group_properties = malloc(
            sizeof(VkPhysicalDeviceGroupProperties) *
            vk_physical_device_group_count);

        if (vk_physical_device_group_properties == NULL)
        {
            fputs("Failed to allocate memory for "
                "vk_physical_device_group_properties\n", stderr);

            return EXIT_FAILURE;
        }

        vkEnumeratePhysicalDeviceGroups(vk_instance,
            &vk_physical_device_group_count,
            vk_physical_device_group_properties);

        printf("vk_physical_device_group_properties[%u]:\n",
            vk_physical_device_group_count);

        for (uint32_t i = 0; i < vk_physical_device_group_count; ++i)
        {
            const VkPhysicalDeviceGroupProperties* const group_properties =
                &vk_physical_device_group_properties[i];

            const uint32_t physical_device_count =
                group_properties->physicalDeviceCount;

            const VkPhysicalDevice* const physical_devices =
                group_properties->physicalDevices;

            indent(1);
            printf("Physical devices[%u]:\n", physical_device_count);

            for (uint32_t j = 0; j < physical_device_count; ++j)
            {
                VkPhysicalDeviceProperties device_properties;
                vkGetPhysicalDeviceProperties(physical_devices[j],
                    &device_properties);

                indent(2);
                puts(device_properties.deviceName);
            }
        }

        free(vk_physical_device_group_properties);
        vk_physical_device_group_properties = NULL;
    }

    if (vk_instance_layer_properties != NULL)
    {
        puts("Vulkan layer extensions:");

        for (uint32_t i = 0; i < vk_instance_layer_property_count; ++i)
        {
            print_vk_extensions(EXTENSION_TYPE_DEVICE, vk_physical_device,
                vk_instance_layer_properties[i].layerName, 1);
        }

        free(vk_instance_layer_properties);
        vk_instance_layer_properties = NULL;
    }

    print_vk_extensions(EXTENSION_TYPE_DEVICE, vk_physical_device, NULL, 0);

    uint32_t vk_queue_family_property_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device,
        &vk_queue_family_property_count, NULL);

    if (vk_queue_family_property_count == 0)
    {
        fputs("Failed to get Vulkan queue family property count\n", stderr);
        return EXIT_FAILURE;
    }

    VkQueueFamilyProperties* vk_queue_family_properties = malloc(
        sizeof(VkQueueFamilyProperties) * vk_queue_family_property_count);

    if (vk_queue_family_properties == NULL)
    {
        fputs("Failed to allocate memory for "
            "vk_queue_family_properties\n", stderr);

        return EXIT_FAILURE;
    }

    vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device,
        &vk_queue_family_property_count, vk_queue_family_properties);

    printf("Queue family properties[%u]:\n", vk_queue_family_property_count);

    for (uint32_t i = 0; i < vk_queue_family_property_count; ++i)
    {
        const VkQueueFamilyProperties* const properties =
            &vk_queue_family_properties[i];

        const VkQueueFlags flags = properties->queueFlags;

        indent(1);

        PRINT_VK_QUEUE_FLAG_BIT(flags, GRAPHICS);
        PRINT_VK_QUEUE_FLAG_BIT(flags, COMPUTE);
        PRINT_VK_QUEUE_FLAG_BIT(flags, TRANSFER);
        PRINT_VK_QUEUE_FLAG_BIT(flags, SPARSE_BINDING);
        PRINT_VK_QUEUE_FLAG_BIT(flags, PROTECTED);

        PRINT_VK_QUEUE_FLAG_BIT_EXT(flags, VIDEO_DECODE, KHR);
        PRINT_VK_QUEUE_FLAG_BIT_EXT(flags, VIDEO_ENCODE, KHR);
        PRINT_VK_QUEUE_FLAG_BIT_EXT(flags, OPTICAL_FLOW, NV);
        PRINT_VK_QUEUE_FLAG_BIT_EXT(flags, DATA_GRAPH, ARM);

        printf("%u\n", properties->queueCount);
    }

    free(vk_queue_family_properties);
    vk_queue_family_properties = NULL;

    return EXIT_SUCCESS;
}
