/****************************************************************************//**
  \file vcpDescriptors.c

  \brief Virtual communication port descriptors.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    11/09/08 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
                   Includes section
******************************************************************************/
#include <vcpDescriptors.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
// Descriptor's type definitions.
#define DESCRIPTOR_TYPE_DEVICE           0x01
#define DESCRIPTOR_TYPE_CONFIGURATION    0x02
#define DESCRIPTOR_TYPE_STRING           0x03
#define DESCRIPTOR_TYPE_INTERFACE        0x04
#define DESCRIPTOR_TYPE_CS_INTERFACE     0x24
#define DESCRIPTOR_TYPE_ENDPOINT         0x05
#define DESCRIPTOR_TYPE_DEVICE_QUALIFIER 0x06
#define DESCRIPTOR_TYPE_OTHER_SPEED_CFG  0x07
#define DESCRIPTOR_TYPE_INTERFACE_POWER  0x08

// String definitions' indexes.
#define MANUFACTURER_STRING_INDEX     0
#define PRODUCT_STRING_INDEX          0
#define SERIAL_NUMBER_STRING_INDEX    0
#define CONFIGURATION_STRING_INDEX    0
#define FIRST_INTERFACE_STRING_INDEX  0
#define SECOND_INTERFACE_STRING_INDEX 0

#define VENDOR_ID       0x03EB    // Atmel
#define PRODUCT_ID      0x6119    // COM port from application notes for AT91
#define DEVICE_RELEASE  0x0001
#define USB_BUS_RELEASE 0x0200

// Number of possible configurations for the device.
#define NUMBER_OF_CONFIGURATIONS     0x01

// Class specification parameters of communication device.
#define DEVICE_CLASS       0x02
#define DEVICE_SUBCLASS    0x00
#define DEVICE_PROTOCOL    0x00

// Endpoint definitions' sizes.
#if defined(AT91SAM7X256)
  #define SZ_CONTROL_ENDPOINT      0x08      // endpoint 0 is control pipe
#elif defined(AT90USB1287) || defined(AT91SAM3S4C)
  #define SZ_CONTROL_ENDPOINT      0x40      // endpoint 0 is control pipe
#endif
#define SZ_ACM_INT_ENDPOINT      0x0008    // endpoint 3 is interrupt pipe for abstraction control model
#define SZ_CDC_BULKIN_ENDPOINT   BULK_SIZE // endpoint 2 is bulk pipe for input communication data
#define SZ_CDC_BULKOUT_ENDPOINT  BULK_SIZE // endpoint 1 is bulk pipe for output communication data

// Configuration descriptor parameters.
// Common size all descriptors in the configuration
#define COMMON_CFG_SIZE          (sizeof(ConfigurationDescriptor_t) + sizeof(InterfaceDescriptor_t) + sizeof(HeaderFunctionalDescriptor_t) + \
                                  sizeof(CallManagmentFunctionalDescriptor_t) + sizeof(AbstractControlManagmentDescriptor_t) + \
                                  sizeof(UnionFunctionalDescriptor_t) + sizeof(HAL_UsbEndPointDescptr_t) + sizeof(InterfaceDescriptor_t) + \
                                  sizeof(HAL_UsbEndPointDescptr_t) + sizeof(HAL_UsbEndPointDescptr_t))
#define NUMBER_OF_INTERFACES      0x02
#define CFG_SELECTING_VALUE       0x01
#define CFG_CHARACTERISTICS       0x80  // D7 is reserved and must be set to one for historical reasons.
#define MAXIMUM_POWER_CONSUMPTION 0x32  // Step is 2 mA.

// Parameters for interfaces descriptors
#define NUMBER_OF_FIRST_INTERFACE          0x00
#define NUMBER_OF_SECOND_INTERFACE         0x01
#define ALTERNATIVE_SETTING                0x00
#define NUMBER_USING_ENDPOINTS_FIRST_IFC   0x01
#define NUMBER_USING_ENDPOINTS_SECOND_IFC  0x02
#define FIRST_IFC_CLASS        0x02
#define FIRST_IFC_SUBCLASS     0x02
#define FIRST_IFC_PROTOCOL     0x00
#define SECOND_IFC_CLASS       0x0A
#define SECOND_IFC_SUBCLASS    0x00
#define SECOND_IFC_PROTOCOL    0x00

// Header descriptor parameters.
#define HEADER_SUBTYPE        0x00
#define CDC_CLASS_DEFINITION  0x0110

// call management functional descriptor parameters.
#define CALL_MNGMT_SUBTYPE       0x01
#define CALL_MNGMT_CAPABILITIES  0x01  // See cdc specification.
#define NUMBER_OF_CALL_MNGMT     0x00

// abstract control management functional descriptor parameters.
#define ACM_SUBTYPE              0x02
#define ACM_CAPABILITIES         0x02  // see cdc specification

// Union functional descriptor parameters.
#define UNION_SUBTYPE            0x06
#define MASTER_IFC_TYPE          0x00
#define SLAVE0_IFC_TYPE          0x01

// Endpoints descriptor parameters.
#define ADDRESS_ITNTERRUPT_PIPE   0x83
#define ADDRESS_BULKIN_PIPE       0x82
#define ADDRESS_BULKOUT_PIPE      0x01
#define INTERRUPT_TYPE_PIPE       0x03
#define BULK_TYPE_PIPE            0x02
#define INTERRUPT_PIPE_POLLING    0x0A  // step is 1 ms
#define BULK_MAX_SPEED            0x00

/******************************************************************************
                   Constants section
******************************************************************************/
const DeviceDescriptor_t deviceDescrptr = {
  sizeof(DeviceDescriptor_t), // Size of this descriptor in bytes
  DESCRIPTOR_TYPE_DEVICE,     // Descriptor type
  USB_BUS_RELEASE,            // USB specification release number in BCD format
  DEVICE_CLASS,               // Device class code
  DEVICE_SUBCLASS,            // Device subclass code
  DEVICE_PROTOCOL,            // Device protocol code
  SZ_CONTROL_ENDPOINT,        // Maximum packet size of endpoint 0 (in bytes)
  VENDOR_ID,                  // Vendor ID
  PRODUCT_ID,                 // Product ID
  DEVICE_RELEASE,             // Device release number in BCD format
  MANUFACTURER_STRING_INDEX,  // Index of the manufacturer string descriptor
  PRODUCT_STRING_INDEX,       // Index of the product string descriptor
  SERIAL_NUMBER_STRING_INDEX, // Index of the serial number string descriptor
  NUMBER_OF_CONFIGURATIONS    // Number of possible configurations for the device
};

const ConfigurationFrameResponse_t vcpConfigDescriptor = {
  { // configuration
    sizeof(ConfigurationDescriptor_t),  // Size of the descriptor in bytes
    DESCRIPTOR_TYPE_CONFIGURATION,      // Descriptor type
    COMMON_CFG_SIZE,                    // Length of all descriptors returned along with this configuration descriptor
    NUMBER_OF_INTERFACES,               // Number of interfaces in this configuration
    CFG_SELECTING_VALUE,                // Value for selecting this configuration
    CONFIGURATION_STRING_INDEX,         // Index of the configuration string descriptor
    CFG_CHARACTERISTICS,                // Configuration characteristics
    MAXIMUM_POWER_CONSUMPTION           // Maximum power consumption of the device when in this configuration
  },
  { // interface 1
    sizeof(InterfaceDescriptor_t),      // Size of the descriptor in bytes
    DESCRIPTOR_TYPE_INTERFACE,          // Descriptor type
    NUMBER_OF_FIRST_INTERFACE,          // Number of the interface in its configuration
    ALTERNATIVE_SETTING,                // Value to select this alternate interface setting
    NUMBER_USING_ENDPOINTS_FIRST_IFC,   // Number of endpoints used by the interface (excluding endpoint 0)
    FIRST_IFC_CLASS,                    // Interface class code
    FIRST_IFC_SUBCLASS,                 // Interface subclass code
    FIRST_IFC_PROTOCOL,                 // Interface protocol code
    FIRST_INTERFACE_STRING_INDEX        // Index of the interface string descriptor
  },
  { // header functional descriptor
    sizeof(HeaderFunctionalDescriptor_t),  // Size of the descriptor in bytes
    DESCRIPTOR_TYPE_CS_INTERFACE,          // Descriptor type
    HEADER_SUBTYPE,                        // Header functional descriptor subtype
    CDC_CLASS_DEFINITION                   // USB Class Definitions for Communication
  },
  { // Call Management Functional Descriptor
    sizeof(CallManagmentFunctionalDescriptor_t), // Size of the descriptor in bytes
    DESCRIPTOR_TYPE_CS_INTERFACE,                // Descriptor type
    CALL_MNGMT_SUBTYPE,                          // bDescriptor subtype: Call Management Func
    CALL_MNGMT_CAPABILITIES,                     // bmCapabilities: D1 + D0
    NUMBER_OF_CALL_MNGMT                         // bDataInterface: Data Class Interface 1
  },
  { // Abstract Control Management Functional Descriptor
    sizeof(AbstractControlManagmentDescriptor_t),  // Size of the descriptor in bytes
    DESCRIPTOR_TYPE_CS_INTERFACE,                  // Descriptor type
    ACM_SUBTYPE,                                   // Abstract Control Management Functional descriptor subtype
    ACM_CAPABILITIES                               // bmCapabilities: see cdc specification (support command type)
  },
  { // Union Functional Descriptor
    sizeof(UnionFunctionalDescriptor_t),  // Size of the descriptor in bytes
    DESCRIPTOR_TYPE_CS_INTERFACE,         // Descriptor type
    UNION_SUBTYPE,                        // Union Functional descriptor subtype
    MASTER_IFC_TYPE,                      // bMasterInterface: CDC Interface
    SLAVE0_IFC_TYPE                       // bSlaveInterface0: Data Class Interface
  },
  { // endpoint 3
    sizeof(HAL_UsbEndPointDescptr_t),   // Size of the descriptor in bytes
    DESCRIPTOR_TYPE_ENDPOINT,           // Descriptor type
    ADDRESS_ITNTERRUPT_PIPE,            // Address and direction of the endpoint
    INTERRUPT_TYPE_PIPE,                // Endpoint type and additional characteristics (for isochronous endpoints)
    SZ_ACM_INT_ENDPOINT,                // Maximum packet size (in bytes) of the endpoint
    INTERRUPT_PIPE_POLLING              // Polling rate of the endpoint
  },
  { // interface 2
    sizeof(InterfaceDescriptor_t),      // Size of the descriptor in bytes
    DESCRIPTOR_TYPE_INTERFACE,          // Descriptor type
    NUMBER_OF_SECOND_INTERFACE,         // Number of the interface in its configuration
    ALTERNATIVE_SETTING,                // Value to select this alternate interface setting
    NUMBER_USING_ENDPOINTS_SECOND_IFC,  // Number of endpoints used by the interface (excluding endpoint 0)
    SECOND_IFC_CLASS,                   // Interface class code
    SECOND_IFC_SUBCLASS,                // Interface subclass code
    SECOND_IFC_PROTOCOL,                // Interface protocol code
    SECOND_INTERFACE_STRING_INDEX       // Index of the interface string descriptor
  },
  {{ // endpoint 1
    sizeof(HAL_UsbEndPointDescptr_t),   // Size of the descriptor in bytes
    DESCRIPTOR_TYPE_ENDPOINT,           // Descriptor type
    ADDRESS_BULKOUT_PIPE,               // Address and direction of the endpoint
    BULK_TYPE_PIPE,                     // Endpoint type and additional characteristics (for isochronous endpoints)
    SZ_CDC_BULKOUT_ENDPOINT,            // Maximum packet size (in bytes) of the endpoint
    BULK_MAX_SPEED                      // Polling rate of the endpoint
  },
  { // endpoint 2
    sizeof(HAL_UsbEndPointDescptr_t),   // Size of the descriptor in bytes
    DESCRIPTOR_TYPE_ENDPOINT,           // Descriptor type
    ADDRESS_BULKIN_PIPE,                // Address and direction of the endpoint
    BULK_TYPE_PIPE,                     // Endpoint type and additional characteristics (for isochronous endpoints)
    SZ_CDC_BULKIN_ENDPOINT,             // Maximum packet size (in bytes) of the endpoint
    BULK_MAX_SPEED                      // Polling rate of the endpoint
  }}
};

// eof vcpDescriptors.c
