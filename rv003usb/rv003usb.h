#ifndef _RV003USB_H
#define _RV003USB_H


#define USB_GPIO_BASE GPIOD_BASE

// Packet Type + 8 + CRC + Buffer
#define USB_BUFFER_SIZE 12

#define USB_DMASK ((1<<(USB_DM)) | 1<<(USB_DP))

#include "usb_config.h"


#ifdef  REALLY_TINY_COMP_FLASH
#define MY_ADDRESS_OFFSET_BYTES 4
#define LAST_SE0_OFFSET         12
#define DELTA_SE0_OFFSET        16
#define SE0_WINDUP_OFFSET       20
#define ENDP_OFFSET             28
#define SETUP_REQUEST_OFFSET    8
#else
#define MY_ADDRESS_OFFSET_BYTES 1
#define LAST_SE0_OFFSET         4
#define DELTA_SE0_OFFSET        8
#define SE0_WINDUP_OFFSET       12
#endif

#ifndef __ASSEMBLER__

#define EMPTY_SEND_BUFFER (uint8_t*)1

// This can be undone once support for fast-c.lbu / c.sbu is made available.
#ifdef  REALLY_TINY_COMP_FLASH
#define TURBO8TYPE uint32_t
#define TURBO16TYPE uint32_t
#else
#define TURBO8TYPE uint8_t
#define TURBO16TYPE uint16_t
#endif


struct usb_endpoint
{
	TURBO8TYPE count_in;	// ack count
	TURBO8TYPE count_out;	// For future: When receiving data. // XXX TODO: Can this be merged?
	TURBO8TYPE opaque;     // For user.
	TURBO8TYPE toggle_in;   // DATA0 or DATA1?
	TURBO8TYPE toggle_out;  //Out PC->US
	TURBO8TYPE is_descriptor;
	TURBO8TYPE max_len;
#ifdef REALLY_TINY_COMP_FLASH
	TURBO8TYPE reserved1;
#endif
};


struct rv003usb_internal
{
	TURBO8TYPE current_endpoint; // Can this be combined with setup_request?
	TURBO8TYPE my_address; // Will be 0 until set up.
	TURBO8TYPE setup_request;
	TURBO8TYPE reserved;
	uint32_t last_se0_cyccount;
	int32_t delta_se0_cyccount;
	uint32_t se0_windup;
	// 5 bytes + 6 * ENDPOINTS

	struct usb_endpoint eps[ENDPOINTS];
};

//Detailed analysis of some useful stuff and performance tweaking: http://naberius.de/2015/05/14/esp8266-gpio-output-performance/
//Reverse engineerd boot room can be helpful, too: http://cholla.mmto.org/esp8266/bootrom/boot.txt
//USB Protocol read from wikipedia: https://en.wikipedia.org/wiki/USB
// Neat stuff: http://www.usbmadesimple.co.uk/ums_3.htm
// Neat stuff: http://www.beyondlogic.org/usbnutshell/usb1.shtml

struct usb_urb
{
	uint16_t wRequestTypeLSBRequestMSB;
	uint32_t lValueLSBIndexMSB;
	uint16_t wLength;
} __attribute__((packed));


extern struct rv003usb_internal rv003usb_internal_data;
extern uint32_t * always0;

// If you are using the .c functionality, be sure to #define USE_RV003_C 1
// usb_hande_interrupt_in is OBLIGATED to call usb_send_data or usb_send_nak.
void usb_hande_interrupt_in( struct usb_endpoint * e, uint8_t * scratchpad, uint32_t sendtok );

// NOTE: Tricky: When making outbound OUT messages, this will be called at the end.
void usb_handle_control_in( struct usb_endpoint * e, uint8_t * scratchpad, uint32_t sendtok );

void usb_handle_control_out_start( struct usb_endpoint * e, int reqLen, uint32_t lValueLSBIndexMSB );
void usb_handle_control_out( struct usb_endpoint * e, uint8_t * data, int len );
void usb_handle_control_in_start( struct usb_endpoint * e, int reqLen, uint32_t lValueLSBIndexMSB );


// Note: This checks addr & endp to make sure they are valid.
void usb_pid_handle_setup( uint32_t addr, uint8_t * data, uint32_t endp, uint32_t unused, struct rv003usb_internal * ist );
void usb_pid_handle_in( uint32_t addr, uint8_t * data, uint32_t endp, uint32_t unused, struct rv003usb_internal * ist );
void usb_pid_handle_out( uint32_t addr, uint8_t * data, uint32_t endp, uint32_t unused, struct rv003usb_internal * ist );
void usb_pid_handle_data( uint32_t this_token, uint8_t * data, uint32_t which_data, uint32_t length, struct rv003usb_internal * ist );
void usb_pid_handle_ack( uint32_t dummy, uint8_t * data, uint32_t dummy2, uint32_t dummy3, struct rv003usb_internal * ist  );

//poly_function = 0 to include CRC.
//poly_function = 2 to exclude CRC.
//This function is provided in assembly
void usb_send_data( uint8_t * data, uint32_t length, uint32_t poly_function, uint32_t token );
void usb_send_nak( uint32_t token );

void usb_setup();

#endif

#endif

