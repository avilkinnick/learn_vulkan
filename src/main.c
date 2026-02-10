#include <volk.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

VkLayerProperties* vk_instance_layer_properties = NULL;
VkExtensionProperties* vk_instance_extension_properties = NULL;

static void cleanup(void)
{
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

    return EXIT_SUCCESS;
}
