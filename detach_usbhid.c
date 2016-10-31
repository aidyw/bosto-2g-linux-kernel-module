//#include <iostream>
#include <libusb.h>
#include <stdio.h>


int main() {
	int res                      = 0;  /* return codes from libusb functions */
	libusb_device_handle* handle = 0;  /* handle for USB device */
	int kernelDriverDetached     = 0;  /* Set to 1 if kernel driver detached */
	int r; //for return values

	r = libusb_init(0); //initialize a library session


	 /* Get the first device with the matching Vendor ID and Product ID. If
	   * intending to allow multiple devices, you
	   * might need to use libusb_get_device_list() instead. Refer to the libusb
	   * documentation for details. */
	  handle = libusb_open_device_with_vid_pid(0, 0x0b57, 0x9016);
	  if (!handle)
	  {
	    printf("Unable to open device.\n");
	    return 1;
	  }

	  /* Check whether a kernel driver is attached to interface #0. If so, we'll
	   * need to detach it.
	   */
	  if (libusb_kernel_driver_active(handle, 0))
	  {
	    res = libusb_detach_kernel_driver(handle, 0);
	    if (res == 0)
	    {
	      kernelDriverDetached = 1;
	      printf("Driver Module detached from bosto device.\n");
	    }
	    else
	    {
	    	printf("Error detaching kernel driver.\n");
	      return 1;
	    }
	  }

		libusb_exit(0); //close the session
		return 0;
}



