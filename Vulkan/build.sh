check_vulkan_sdk() 
{
	if [[ -z "${VULKAN_SDK}" ]]; then
		echo "Missing environment variable VULKAN_SDK"
		echo "Please download the Vulkan SDK from https://vulkan.lunarg.com/sdk/home#linux"
		echo "Extract it and run the following before running this script:"
		echo "source <extracted dir>/setup-env.sh"
		exit 1
	fi
}
