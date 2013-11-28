/****************************************************************************//**
  \file vcpDescriptors.h

  \brief Declaration of descriptors structures.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    05/09/08 A. Khromykh - Created
*******************************************************************************/
#ifndef _VCPDESCRIPTORS_H
#define _VCPDESCRIPTORS_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <types.h>
#include <usb.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define INTERFACE_ENDPOINTS_NUMBER     2  // VCP
#define BULK_SIZE   0x0040

/******************************************************************************
                   Types section
******************************************************************************/
// USB standard device descriptor structure.
BEGIN_PACK
typedef struct PACK
{
  uint8_t  bLength;            // Size of this descriptor in bytes
  uint8_t  bDescriptorType;    // Descriptor type
  uint16_t bcdUSB;             // USB specification release number in BCD format
  uint8_t  bDeviceClass;       // Device class code
  uint8_t  bDeviceSubClass;    // Device subclass code
  uint8_t  bDeviceProtocol;    // Device protocol code
  uint8_t  bMaxPacketSize0;    // Maximum packet size of endpoint 0 (in bytes)
  uint16_t idVendor;           // Vendor ID
  uint16_t idProduct;          // Product ID
  uint16_t bcdDevice;          // Device release number in BCD format
  uint8_t  iManufacturer;      // Index of the manufacturer string descriptor
  uint8_t  iProduct;           // Index of the product string descriptor
  uint8_t  iSerialNumber;      // Index of the serial number string descriptor
  uint8_t  bNumConfigurations; // Number of possible configurations for the device
} DeviceDescriptor_t;

// USB standard configuration descriptor structure.
typedef struct PACK
{
  uint8_t  bLength;             // Size of the descriptor in bytes
  uint8_t  bDescriptorType;     // Descriptor type
  uint16_t wTotalLength;        // Length of all descriptors returned along with this configuration descriptor
  uint8_t  bNumInterfaces;      // Number of interfaces in this configuration
  uint8_t  bConfigurationValue; // Value for selecting this configuration
  uint8_t  iConfiguration;      // Index of the configuration string descriptor
  uint8_t  bmAttributes;        // Configuration characteristics
  uint8_t  bMaxPower;           // Maximum power consumption of the device when in this configuration
} ConfigurationDescriptor_t;

// USB standard interface descriptor structure.
typedef struct PACK
{
  uint8_t bLength;            // Size of the descriptor in bytes
  uint8_t bDescriptorType;    // Descriptor type
  uint8_t bInterfaceNumber;   // Number of the interface in its configuration
  uint8_t bAlternateSetting;  // Value to select this alternate interface setting
  uint8_t bNumEndpoints;      // Number of endpoints used by the interface (excluding endpoint 0)
  uint8_t bInterfaceClass;    // Interface class code
  uint8_t bInterfaceSubClass; // Interface subclass code
  uint8_t bInterfaceProtocol; // Interface protocol code
  uint8_t iInterface;         // Index of the interface string descriptor
} InterfaceDescriptor_t;

// USB header functional descriptor.
typedef struct PACK
{
  uint8_t  bFunctionalLength;  // Size of the descriptor in bytes
  uint8_t  bDescriptorType;    // Descriptor type
  uint8_t  bDescriptorSubtype; // Header functional descriptor subtype
  uint16_t bcdCDC;             // USB Class Definitions for Communication
                               // Devices Specification release number in binary-coded decimal.
} HeaderFunctionalDescriptor_t;

// USB Abstract Control Management Functional Descriptor.
typedef struct PACK
{
  uint8_t bFunctionalLength;  // Size of the descriptor in bytes
  uint8_t bDescriptorType;    // Descriptor type
  uint8_t bDescriptorSubtype; // ACM descriptor subtype
  uint8_t bmCapabilities;     // The capabilities that this configuration supports
} AbstractControlManagmentDescriptor_t;

// USB Union Functional Descriptor.
typedef struct PACK
{
  uint8_t bFunctionalLength;  // Size of the descriptor in bytes
  uint8_t bDescriptorType;    // Descriptor type
  uint8_t bDescriptorSubtype; // Union functional descriptor subtype
  uint8_t bMasterInterface;   // The interface number of the Communication or Data Class interface, designated as the master or controlling interface for the union
  uint8_t bSlaveInterface0;   // Interface number of first slave or associated interface in the union
} UnionFunctionalDescriptor_t;

// USB Call Management Functional Descriptor.
typedef struct PACK
{
  uint8_t bFunctionalLength;  // Size of the descriptor in bytes
  uint8_t bDescriptorType;    // Descriptor type
  uint8_t bDescriptorSubtype; // Union functional descriptor subtype
  uint8_t bmCapabilities;     // The capabilities that this configuration supports
  uint8_t bDataInterface;     // Interface number of Data Class interface optionally used for call management
} CallManagmentFunctionalDescriptor_t;

// USB frame for interface request.
typedef struct PACK
{
  ConfigurationDescriptor_t             config;
  InterfaceDescriptor_t                 interface1;
  HeaderFunctionalDescriptor_t          headerFunctional;
  CallManagmentFunctionalDescriptor_t   cmFunctionak;
  AbstractControlManagmentDescriptor_t  acmFunctional;
  UnionFunctionalDescriptor_t           unionFunctional;
  HAL_UsbEndPointDescptr_t              endpointIfc1;
  InterfaceDescriptor_t                 interface2;
  HAL_UsbEndPointDescptr_t              endpointIfc2[INTERFACE_ENDPOINTS_NUMBER];
} ConfigurationFrameResponse_t;
END_PACK

#endif /* _VCPDESCRIPTORS_H */
