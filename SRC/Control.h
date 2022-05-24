
#ifndef CONTROL_H_INCLUDED
#define CONTROL_H_INCLUDED

/******************************************************/
/* Common defines */

#define MAX_CONTROLLER_COUNT	2	/* Max number of controllers we have to support. */
#define MAX_CONTROLLER_BYTES	34	/* Size of the biggest packet we have to support according to the tech ref. */


/******************************************************/
/* Controller types */

#define CONTROLLER_DATA_PAD			1
#define CONTROLLER_DATA_MOUSE		2
#define CONTROLLER_DATA_NEGCON		3
#define CONTROLLER_DATA_ANALOG		8
#define CONTROLLER_DATA_MULTITAP	16
#define CONTROLLER_DATA_GUNCON		3

#define CONTROLLER_TYPE_UNKNOWN		0
#define CONTROLLER_TYPE_MOUSE		1
#define CONTROLLER_TYPE_NEGCON		2
#define CONTROLLER_TYPE_PAD			4
#define CONTROLLER_TYPE_ANALOG		5
#define CONTROLLER_TYPE_GUNCON		6
#define CONTROLLER_TYPE_DUALSHOCK	7
#define CONTROLLER_TYPE_MULTITAP	8

typedef unsigned short PadData;

/******************************************************/
/* Standard gamepad packet data */

#define PAD_None		0xffff

#define PAD_L2			0x0100
#define PAD_R2			0x0200
#define PAD_L1			0x0400
#define PAD_R1			0x0800

#define PAD_Triangle	0x1000
#define PAD_Circle		0x2000
#define PAD_Cross		0x4000
#define PAD_Square		0x8000

#define PAD_Up			0x0010
#define PAD_Right		0x0020
#define PAD_Down		0x0040
#define PAD_Left		0x0080

#define PAD_Select		0x0001
#define PAD_Start		0x0008

/* Determines if the given pad button is pressed. */
#define IsPadButtonPressed(packet, button)		(!((packet)->data.pad & (button)))
/* Determines if any pad button is pressed. */
#define IsAnyPadButtonPressed(packet)			((packet)->data.pad != PAD_None)
/* Determines if no pad button is pressed at all. */
#define IsNoPadButtonPressed(packet)			((packet)->data.pad == PAD_None)

/******************************************************/
/* Mouse packet data */

typedef struct
{
	unsigned short buttons;
    signed char x_offset;
    signed char y_offset;
} MouseData;

#define MOUSE_LEFT		0x0800		/* left mouse button*/
#define MOUSE_RIGHT		0x0400		/* right mouse button */
#define MOUSE_NOKEY		0xfcff		/* no buttons pressed */

/* Gets the x offset of the mouse. */
#define GetMouseXOffset(packet)					((packet)->data.mouse.x_offset)
/* Gets the y offset of the mouse. */
#define GetMouseYOffset(packet)					((packet)->data.mouse.y_offset)
/* Determines whether the mouse is moving. */
#define IsMouseMoving(packet)					((packet)->data.mouse.x_offset || (packet)->data.mouse.y_offset)
/* Determines whether a mouse button is pressed. */
#define IsMouseButtonPressed(packet, button)	(!((packet)->data.mouse.buttons & (button)))
/* Determines whether any mouse button is pressed. */
#define IsAnyMouseButtonPressed(packet)			((packet)->data.mouse.buttons != MOUSE_NOKEY)
/* Determines whether no mouse button is pressed at all. */
#define IsNoMouseButtonPressed(packet)			((packet)->data.mouse.buttons == MOUSE_NOKEY)

/******************************************************/
/* Negcon controller packet data */
typedef struct
{
	unsigned short digital_buttons;               /* Bit mask of plain keys. */
	char central_twist;                           /* Analogue twisting thing.*/
	char button_i;                                /* The I analogue button.  */
	char button_ii;                               /* The II analogue button. */
	char top_left;                                /* The analogue tl button. */
} NegconData;

/* TODO: Add NegCon support macros etc. */

/******************************************************/
/* Namco Gun packet data */
typedef struct
{
	unsigned short digital_buttons;              /* Bit mask of plain keys. */
    unsigned char guncon_x;
    unsigned char guncon_screen_div;
    unsigned char guncon_y;
    unsigned char pad;
} GunConData;

/* TODO: Add GunCon support macros etc. */

/******************************************************/
/* Analog Joystick packet data */
typedef struct
{
     unsigned short	digital_buttons;               /* Bit mask of plain keys. */
     unsigned char	right_x;
     unsigned char	right_y;
     unsigned char	left_x;
     unsigned char	left_y;
} AnalogjoyData;

#define GetRightAnalogStickX(packet)			( (packet)->data.analog.right_x)
#define GetRightAnalogStickY(packet)			( (packet)->data.analog.right_y)
#define GetLeftAnalogStickX(packet)				( (packet)->data.analog.left_x)
#define GetLeftAnalogStickY(packet)				( (packet)->data.analog.left_y)

/******************************************************/
/* Multitap controller packet data*/

typedef struct
{
    unsigned char status;
    unsigned char data_format;

	/* Controller data union. */
    union
    {
        PadData		pad;				/* Plain pad.       */
        NegconData	negcon;				/* Namco controller.*/
    } data;
} TapControllerData;

typedef struct
{
    TapControllerData controllers[4];	/* Just 4 controller packets. */
} MultiTapData;

#define TAP_CONTROLLER_1			0
#define TAP_CONTROLLER_2			1
#define TAP_CONTROLLER_3			2
#define TAP_CONTROLLER_4			3
#define MAX_TAP_CONTROLLER_COUNT	4

/* Gets the controller data for a controller of the tap (index 0 - 3). */
#define GetTapControllerData(packet, controller_index)           (&((packet)->data.tap.ctrllers[(controller_index)]))
/* Determines whether a pad button for the first controller in the multi tap is pressed. */
#define IsMultiTap1PadButtonPressed(packet, button)   (!((packet)->data.tap.controllers[0].data.pad & (button)))
/* Determines whether a pad button for the second controller in the multi tap is pressed. */
#define IsMultiTap2PadButtonPressed(packet, button)   (!((packet)->data.tap.controllers[1].data.pad & (button)))
/* Determines whether a pad button for the third controller in the multi tap is pressed. */
#define IsMultiTap3PadButtonPressed(packet, button)   (!((packet)->data.tap.controllers[2].data.pad & (button)))
/* Determines whether a pad button for the fourth controller in the multi tap is pressed. */
#define IsMultiTap4PadButtonPressed(packet, button)   (!((packet)->data.tap.controllers[3].data.pad & (button)))

/******************************************************/
/* Controller packet */

/* Pad status codes used for first byte in ControllerPacket struct. */
#define PAD_STATUS_OK		0x00		/* Pad is connected, data transmission was successful */
#define PAD_STATUS_ERROR	0xff	/* Pad is either disconnected, broken or bad transmission */

typedef struct
{
	unsigned char status;		/* 0xff = no pad, bad pad, bad transmission */
	unsigned char data_format;	/* Top 4 bits = type of controller */
                                /* Bottom 4 == shorts of data written */

    /* Controller data union. */
	union
	{
		PadData       pad;							/* Plain pad.							*/
		MouseData     mouse;						/* Mouse.								*/
		NegconData    negcon;						/* Namco controller.					*/
		AnalogjoyData analog;						/* Anlog Joystick						*/
		MultiTapData  tap;							/* 4-way multi-tap.						*/
		GunConData    guncon;						/* Namco Gun							*/
		unsigned char bytes[MAX_CONTROLLER_BYTES];	/* Ensure struct size is big enough.	*/
	} data;
} ControllerPacket;

/* Returns true if the packet is valid. */
#define ControllerPacketIsValid(packet)		((packet)->status != PAD_STATUS_ERROR)
/* Returns the controller type (see CONTROLLER_TYPE_ defines above). */
#define GetControllerType(packet)			(((packet)->data_format & 0xf0) >> 4)
/* Returns the amount of bytes received of the controller packet. */
#define GetControllerPacketDataSize(packet)	(((packet)->data_format & 0x0f) << 1)
/* Returns the start address of the controller packet. */
#define GetControllerDataAddress(packet)	(&((packet)->data_format) + 1)

#endif
