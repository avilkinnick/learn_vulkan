#include <volk.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void print_vk_version(const uint32_t version)
{
    printf("%u.%u.%u.%u", VK_API_VERSION_VARIANT(version),
                          VK_API_VERSION_MAJOR(version),
                          VK_API_VERSION_MINOR(version),
                          VK_API_VERSION_PATCH(version));
}

int main(void)
{
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

    return EXIT_SUCCESS;
}
