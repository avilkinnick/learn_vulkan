#include <volk.h>

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    if (volkInitialize() != VK_SUCCESS)
    {
        fputs("Failed to initialize volk\n", stderr);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
