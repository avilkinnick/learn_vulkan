#include <volk.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

VkLayerProperties* vk_instance_layer_properties = NULL;
VkExtensionProperties* vk_instance_extension_properties = NULL;
VkInstance vk_instance = VK_NULL_HANDLE;

static void cleanup(void)
{
    if (vk_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(vk_instance, NULL);
        vk_instance = VK_NULL_HANDLE;
    }

    free(vk_instance_extension_properties);
    vk_instance_extension_properties = NULL;

    free(vk_instance_layer_properties);
    vk_instance_layer_properties = NULL;
}

static void indent(const int n)
{
    for (int i = 0; i < n; ++i)
    {
        printf("    ");
    }
}

static void print_vk_version(const uint32_t version)
{
    printf("%u.%u.%u.%u", VK_API_VERSION_VARIANT(version),
                          VK_API_VERSION_MAJOR(version),
                          VK_API_VERSION_MINOR(version),
                          VK_API_VERSION_PATCH(version));
}

static VkBool32 vk_debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
)
{
    (void)pUserData;

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        printf("[VERBOSE]");
    }

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        printf("[INFO]");
    }

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        printf("[WARNING]");
    }

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        printf("[ERROR]");
    }

    if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
    {
        printf("[GENERAL]");
    }

    if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
    {
        printf("[VALIDATION]");
    }

    if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
    {
        printf("[PERFORMANCE]");
    }

    if (messageTypes &
        VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT)
    {
        printf("[DEVICE_ADDRESS_BINDING]");
    }

    printf(" %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

int main(void)
{
    atexit(cleanup);

    if (volkInitialize() != VK_SUCCESS)
    {
        fputs("Failed to initialize volk\n", stderr);

        return EXIT_FAILURE;
    }

    uint32_t vk_instance_version;
    vkEnumerateInstanceVersion(&vk_instance_version);

    printf("vk_instance_version: ");
    print_vk_version(vk_instance_version);
    printf("\n");

    uint32_t vk_instance_layer_property_count;
    vkEnumerateInstanceLayerProperties(&vk_instance_layer_property_count, NULL);

    if (vk_instance_layer_property_count > 0)
    {
        vk_instance_layer_properties = malloc(vk_instance_layer_property_count *
            sizeof(VkLayerProperties));

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
            const VkLayerProperties* const layer_properties =
                &vk_instance_layer_properties[i];

            const char* const layer_name = layer_properties->layerName;

            indent(1);
            printf("%s:\n", layer_name);

            indent(2);
            printf("Spec version: ");
            print_vk_version(layer_properties->specVersion);
            printf("\n");

            indent(2);
            printf("Implementation version: %u\n",
                layer_properties->implementationVersion);

            indent(2);
            printf("Description: %s\n", layer_properties->description);

            uint32_t extension_property_count;
            vkEnumerateInstanceExtensionProperties(layer_name,
                &extension_property_count, NULL);

            if (extension_property_count > 0)
            {
                vk_instance_extension_properties = malloc(
                    extension_property_count * sizeof(VkExtensionProperties));

                if (vk_instance_extension_properties == NULL)
                {
                    fputs("Failed to allocate memory for "
                        "vk_instance_extension_properties\n", stderr);

                    return EXIT_FAILURE;
                }

                vkEnumerateInstanceExtensionProperties(layer_name,
                    &extension_property_count,
                    vk_instance_extension_properties);

                indent(2);
                printf("vk_instance_extension_properties[%u]:\n",
                    extension_property_count);

                for (uint32_t j = 0; j < extension_property_count; ++j)
                {
                    const VkExtensionProperties* const extension_properties =
                        &vk_instance_extension_properties[j];

                    indent(3);
                    printf("%s v.%u\n", extension_properties->extensionName,
                        extension_properties->specVersion);
                }

                free(vk_instance_extension_properties);
                vk_instance_extension_properties = NULL;
            }
        }

        free(vk_instance_layer_properties);
        vk_instance_layer_properties = NULL;
    }

    uint32_t vk_instance_extension_property_count;
    vkEnumerateInstanceExtensionProperties(NULL,
        &vk_instance_extension_property_count, NULL);

    if (vk_instance_extension_property_count > 0)
    {
        vk_instance_extension_properties = malloc(
            vk_instance_extension_property_count *
            sizeof(VkExtensionProperties));

        if (vk_instance_extension_properties == NULL)
        {
            fputs("Failed to allocate memory for "
                "vk_instance_extension_properties\n", stderr);

            return EXIT_FAILURE;
        }

        vkEnumerateInstanceExtensionProperties(NULL,
            &vk_instance_extension_property_count,
            vk_instance_extension_properties);

        printf("vk_instance_extension_properties[%u]:\n",
            vk_instance_extension_property_count);

        for (uint32_t i = 0; i < vk_instance_extension_property_count; ++i)
        {
            const VkExtensionProperties* const properties =
                &vk_instance_extension_properties[i];

            indent(1);
            printf("%s v.%u\n", properties->extensionName,
                properties->specVersion);
        }

        free(vk_instance_extension_properties);
        vk_instance_extension_properties = NULL;
    }

    const VkDebugUtilsMessengerCreateInfoEXT vk_debug_messenger_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = NULL,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
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
        "VK_KHR_surface",
        "VK_EXT_debug_utils"
    };

    const VkInstanceCreateInfo vk_instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &vk_debug_messenger_create_info,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &vk_application_info,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = 2,
        .ppEnabledExtensionNames = vk_enabled_instance_extension_names
    };

    if (vkCreateInstance(&vk_instance_create_info, NULL, &vk_instance) !=
        VK_SUCCESS)
    {
        fputs("Failed to create Vulkan instance\n", stderr);

        return EXIT_FAILURE;
    }

    volkLoadInstance(vk_instance);

    return EXIT_SUCCESS;
}
