/**************************************************************************//**
\file  halUsb.c

\brief Implementation of usb hardware-dependent module.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    08/07/08 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#ifdef USB_TRACE
  #include <trace.h>
#endif
#include <halUsb.h>
#include <usb.h>
#if defined(FREE_RTOS)
  #include <FreeRTOS.h>
  #include <task.h>
#endif

/******************************************************************************
                   Defines section
******************************************************************************/
/* UDP_RXDATA - Bit mask for both banks of the UDP_CSR register. */
#define UDP_RXDATA              (AT91C_UDP_RX_DATA_BK0 | AT91C_UDP_RX_DATA_BK1)
/* Initial data */
#define INITIAL_DATA             0xFFFFFFFF
/* End point mask for interrupt registers */
#define EP_INT_MASK             (AT91C_UDP_EPINT0 | AT91C_UDP_EPINT1 | AT91C_UDP_EPINT2 | \
                                 AT91C_UDP_EPINT3 | AT91C_UDP_EPINT4 | AT91C_UDP_EPINT5)

/******************************************************************************
                   Static inline functions section
******************************************************************************/
/**************************************************************************//**
\brief Clears the specified bit(s) in the UDP_CSR register.
******************************************************************************/
static inline void halClearCSR(uint8_t endpoint, uint32_t flags)
{
  volatile uint32_t reg;

  reg = AT91C_BASE_UDP->UDP_CSR[endpoint];
  reg &= ~(flags);
  reg |= (AT91C_UDP_RX_DATA_BK0 | AT91C_UDP_RX_DATA_BK1);
  AT91C_BASE_UDP->UDP_CSR[endpoint] = reg;
  while ((AT91C_BASE_UDP->UDP_CSR[endpoint] & (flags)) == ((uint32_t)flags));
}

/**************************************************************************//**
\brief Sets the specified bit(s) in the UDP_CSR register.
******************************************************************************/
static inline void halSetCSR(uint8_t endpoint, uint32_t flags)
{
  volatile uint32_t reg;

  reg = AT91C_BASE_UDP->UDP_CSR[endpoint];
  reg |= (flags);
  reg |= (AT91C_UDP_RX_DATA_BK0 | AT91C_UDP_RX_DATA_BK1);
  AT91C_BASE_UDP->UDP_CSR[endpoint] = reg;
  while ((AT91C_BASE_UDP->UDP_CSR[endpoint] & (flags)) != ((uint32_t)flags));
}

/******************************************************************************
                   Static prototypes section
******************************************************************************/
#if defined(FREE_RTOS)
  static void usbHandler(void) __attribute__((naked));
#endif
static void halClearRxFlag(uint8_t eptnum);
static void halWritePayload(uint8_t eptnum);
static void halReadPayload(uint8_t eptnum, uint16_t size);
static void halReadRequest(uint8_t *request, uint8_t eptnum);
static uint8_t halIsTransferFinished(uint8_t eptnum);

/******************************************************************************
                   Global variables section
******************************************************************************/
// Holds the internal state for each endpoint of the UDP
UsbEndpoint_t endpoints[USB_END_POINTS_NUMBER];
// Device current state
volatile uint8_t deviceState;
// Previous device current state
volatile uint8_t previousDeviceState;
// user's request handler
void (* sendReqToUpper)(uint8_t *req) = NULL;
// pointer to request memory. Memory allocate by user.
uint8_t *requestMemory = NULL;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
usb interrupt handler
******************************************************************************/
static void usbHandler(void)
{
#if defined(FREE_RTOS)
  portSAVE_CONTEXT();
#endif

  uint32_t status;

  // Get interrupt status
  // Some interrupts may get masked depending on the device state
  status = AT91C_BASE_UDP->UDP_ISR;
  status &= AT91C_BASE_UDP->UDP_IMR;

  // Return immediately if there is no interrupt to service
  if (!status)
    return;

  if (status & AT91C_UDP_ENDBUSRES)
  {
    // Acknowledge end of bus reset interrupt and the others
    AT91C_BASE_UDP->UDP_ICR = AT91C_UDP_ENDBUSRES | AT91C_UDP_WAKEUP | AT91C_UDP_RXRSM | AT91C_UDP_RXSUSP;
    AT91C_BASE_UDP->UDP_IER = AT91C_UDP_RXSUSP;
    halInterruptBusReset();
  }
  // Suspend
  // This interrupt is always treated last
  else if (status & AT91C_UDP_RXSUSP)
  {
    // Acknowledge interrupt
    AT91C_BASE_UDP->UDP_ICR = AT91C_UDP_RXSUSP;
    // Don't do anything if the device is already suspended
    if (deviceState >= DEVICE_ADDRESS)
    {
      // The device enters the Suspended state
      // Disable interrupt
      AT91C_BASE_UDP->UDP_IDR = AT91C_UDP_RXSUSP;
      // Enable wake up
      AT91C_BASE_UDP->UDP_IER = AT91C_UDP_WAKEUP | AT91C_UDP_RXRSM;
      // Switch to the Suspended state
      previousDeviceState = deviceState;
      deviceState = DEVICE_SUSPENDED;
      // disable transceiver
      halDisableTransceiver();
      halDisableUsbClock();
      halDisablePeripheralClock();
      halInterruptSuspend();
    }
  }
  // Resume
  else if (status & (AT91C_UDP_WAKEUP | AT91C_UDP_RXRSM))
  {
    // The device enters its previous state
    halEnablePeripheralClock();
    halEnableUsbClock();
    halEnableTransceiver();
    // Don't do anything if the device was not suspended
    if (DEVICE_SUSPENDED == deviceState)
    {
      // Enable the transceiver if the device was past the Default state
      deviceState = previousDeviceState;
    }
    // Clear and disable resume interrupts
    AT91C_BASE_UDP->UDP_ICR = AT91C_UDP_WAKEUP | AT91C_UDP_RXRSM;
    AT91C_BASE_UDP->UDP_IDR = AT91C_UDP_WAKEUP | AT91C_UDP_RXRSM;
    // Enable suspend interrupt
    AT91C_BASE_UDP->UDP_IER = AT91C_UDP_RXSUSP;
    halInterruptResume();
  }
  // Endpoint interrupts
  else
  {
    if (status & EP_INT_MASK)
    {
      AT91C_BASE_UDP->UDP_IDR = status & EP_INT_MASK;
      halInterruptEndPoints();
    }
  }

#if defined(FREE_RTOS)
  /* End the interrupt in the AIC. */
  AT91C_BASE_AIC->AIC_EOICR = 0;

  portRESTORE_CONTEXT();
#endif
}

/******************************************************************************
tuning and setting AIC for UDP
******************************************************************************/
void halUsbInterrupt(void)
{
  /* Clear interrupt */
  AT91C_BASE_UDP->UDP_ICR = AT91C_UDP_WAKEUP | AT91C_UDP_RXSUSP | AT91C_UDP_RXRSM;

  /* AIC initialization for USB */
  /* Disable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IDCR = (1 << AT91C_ID_UDP);
  /* Save the interrupt handler routine pointer and the interrupt priority */
  AT91C_BASE_AIC->AIC_SVR[AT91C_ID_UDP] = (uint32_t)usbHandler;
  /* Store the Source Mode Register */
  AT91C_BASE_AIC->AIC_SMR[AT91C_ID_UDP] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | AT91C_AIC_PRIOR_LOWEST;
  /* Clear the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_ICCR = 1 << AT91C_ID_UDP;
  /* Enable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IECR = 1 << AT91C_ID_UDP;
}

/******************************************************************************
Handles a completed transfer on the given endpoint, invoking the configured
callback if any.

Parameters:
  eptnum - Number of the endpoint for which the transfer has completed.
  status - Result of the USB transfer.
******************************************************************************/
void halEndOfTransfer(uint8_t eptnum, uint8_t status)
{
  UsbEndpoint_t *endpoint = &(endpoints[eptnum]);
  UsbTransfer_t *transfer = &(endpoint->transfer);

  // Check that endpoint was sending or receiving data
  if ((UDP_ENDPOINT_RECEIVING == endpoint->state) || (UDP_ENDPOINT_SENDING == endpoint->state))
  {
    #ifdef USB_TRACE
      trace_LOG(trace_INFO, "EoT ");
    #endif
    // Endpoint returns in Idle state
    endpoint->state = UDP_ENDPOINT_IDLE;
    // Invoke callback is present
    if (NULL != transfer->callback)
    {
      ((TransferCallback_t) transfer->callback)(transfer->argument, status,
                                              transfer->transferred, transfer->remaining + transfer->buffered);
    }
  }
}

/******************************************************************************
Clears the correct reception flag (bank 0 or bank 1) of an endpoint.

Parameters:
  eptnum - Number of endpoint.
******************************************************************************/
static void halClearRxFlag(uint8_t eptnum)
{
  UsbEndpoint_t *endpoint = &(endpoints[eptnum]);

  // Clear flag and change banks
  if (endpoint->bank == 0)
  {
    ATOMIC_SECTION_ENTER
    AT91C_BASE_UDP->UDP_CSR[eptnum] &= ~AT91C_UDP_RX_DATA_BK0;
    while ((AT91C_BASE_UDP->UDP_CSR[eptnum] & AT91C_UDP_RX_DATA_BK0) == AT91C_UDP_RX_DATA_BK0);
    ATOMIC_SECTION_LEAVE

    // Swap bank if in dual-fifo mode
    if (UDP_ENDPOINTS_BANKS(eptnum) > 1)
      endpoint->bank = 1;
  }
  else
  {
    ATOMIC_SECTION_ENTER
    AT91C_BASE_UDP->UDP_CSR[eptnum] &= ~AT91C_UDP_RX_DATA_BK1;
    while ((AT91C_BASE_UDP->UDP_CSR[eptnum] & AT91C_UDP_RX_DATA_BK1) == AT91C_UDP_RX_DATA_BK1);
    ATOMIC_SECTION_LEAVE
    endpoint->bank = 0;
  }
}

/******************************************************************************
Writes a data payload into the current FIFO buffer of the UDP.

Parameters:
  eptnum - Number of the endpoint which is sending data.
******************************************************************************/
static void halWritePayload(uint8_t eptnum)
{
  UsbEndpoint_t *endpoint = &(endpoints[eptnum]);
  UsbTransfer_t *transfer = &(endpoint->transfer);
  uint16_t size;

  // Get the number of bytes to send
  size = endpoint->size;
  if (size > transfer->remaining)
  {
    size = transfer->remaining;
  }

  // Update transfer descriptor information
  transfer->buffered += size;
  transfer->remaining -= size;

  // Write packet in the FIFO buffer
  while (size > 0)
  {
    AT91C_BASE_UDP->UDP_FDR[eptnum] = *(transfer->data);
    transfer->data++;
    size--;
  }
}

/******************************************************************************
Reads a data payload from the current FIFO buffer of an endpoint.

Parameters:
  eptnum - Endpoint number.
  size - Size of the data to read.
******************************************************************************/
static void halReadPayload(uint8_t eptnum, uint16_t size)
{
  UsbEndpoint_t *endpoint = &(endpoints[eptnum]);
  UsbTransfer_t *transfer = &(endpoint->transfer);

  // Check that the requested size is not bigger than the remaining transfer
  if (size > transfer->remaining)
  {
    transfer->buffered += size - transfer->remaining;
    size = transfer->remaining;
  }

  // Update transfer descriptor information
  transfer->remaining -= size;
  transfer->transferred += size;

  // Retrieve packet
  while (size > 0)
  {
    *(transfer->data) = (uint8_t)AT91C_BASE_UDP->UDP_FDR[eptnum];
    transfer->data++;
    size--;
  }
}

/******************************************************************************
Reads a SETUP request from the FIFO buffer of Control endpoint and stores it
into the global request variable.

Parameters:
  request - global variable for request reading
  eptnum - endpoint number
******************************************************************************/
static void halReadRequest(uint8_t *request, uint8_t eptnum)
{
  uint16_t i;

  // Copy packet
  for (i = 0; i < 8; i++)
  {
    *request = (uint8_t)AT91C_BASE_UDP->UDP_FDR[eptnum];
    request++;
  }
}

/******************************************************************************
Resets all the endpoints of the UDP peripheral.
******************************************************************************/
void halResetEndpoints(void)
{
  UsbEndpoint_t *endpoint;
  UsbTransfer_t *transfer;
  uint8_t eptnum;

  // Reset the transfer descriptor of every endpoint
  for (eptnum = 0; eptnum < USB_END_POINTS_NUMBER; eptnum++)
  {
    endpoint = &(endpoints[eptnum]);
    transfer = &(endpoint->transfer);
    // Reset endpoint transfer descriptor
    transfer->data = 0;
    transfer->transferred = INITIAL_DATA;
    transfer->buffered = INITIAL_DATA;
    transfer->remaining = INITIAL_DATA;
    transfer->callback = NULL;
    transfer->argument = 0;
    // Reset endpoint state
    endpoint->bank = 0;
    endpoint->state = UDP_ENDPOINT_DISABLED;
  }
}

/******************************************************************************
Disables all endpoints of the UDP peripheral except control endpoint 0.
******************************************************************************/
void halDisableEndpoints(void)
{
  uint8_t eptnum;

  // Disable each endpoint, terminating any pending transfer
  for (eptnum = 1; eptnum < USB_END_POINTS_NUMBER; eptnum++)
  {
    halEndOfTransfer(eptnum, STATUS_ABORTED);
    endpoints[eptnum].state = UDP_ENDPOINT_DISABLED;
  }
}

/******************************************************************************
Checks if an ongoing transfer on an endpoint has been completed.

Parameters:
 eptnum - Endpoint number.

Returns:
  1 - if the current transfer on the given endpoint is complete;
  0 - otherwise
******************************************************************************/
static uint8_t halIsTransferFinished(uint8_t eptnum)
{
  UsbEndpoint_t *endpoint = &(endpoints[eptnum]);
  UsbTransfer_t *transfer = &(endpoint->transfer);

  // Check if it is a Control endpoint
  // Control endpoint must always finish their transfer with a zero-length packet
  if ((AT91C_BASE_UDP->UDP_CSR[eptnum] & AT91C_UDP_EPTYPE) == AT91C_UDP_EPTYPE_CTRL)
    return (transfer->buffered < endpoint->size);
  else // Other endpoints only need to transfer all the data
    return (transfer->buffered <= endpoint->size) && (transfer->remaining == 0);
}

/******************************************************************************
End of bus reset state handler
******************************************************************************/
void halEndOfBusResetHandler(void)
{
  // The device enters the Default state
  deviceState = DEVICE_DEFAULT;
  halEnableTransceiver();
  halResetEndpoints();
  halDisableEndpoints();
  HAL_ConfigureEndpoint(NULL);
}

/******************************************************************************
Endpoint interrupt handler. Manages IN, OUT & SETUP transaction, as well
as the STALL condition.

Parameters:
  eptnum - Number of the endpoint to handle interrupt for.
******************************************************************************/
void halEndpointHandler(uint8_t eptnum)
{
  UsbEndpoint_t *endpoint = &(endpoints[eptnum]);
  UsbTransfer_t *transfer = &(endpoint->transfer);
  uint32_t status = AT91C_BASE_UDP->UDP_CSR[eptnum];

  #ifdef USB_TRACE
    trace_LOG(trace_INFO, "Ept%d ", eptnum);
  #endif

  // Handle interrupts
  // IN packet sent
  if (status & AT91C_UDP_TXCOMP)
  {
    #ifdef USB_TRACE
      trace_LOG(trace_INFO, "Wr ");
    #endif

    // Check that endpoint was in Sending state
    if (UDP_ENDPOINT_SENDING == endpoint->state)
    {
      if (halIsTransferFinished(eptnum))
      { // transfer is finished
        #ifdef USB_TRACE
          trace_LOG(trace_INFO, "%d ", (int)transfer->buffered);
        #endif
        transfer->transferred += transfer->buffered;
        transfer->buffered = 0;

        halClearCSR(eptnum, AT91C_UDP_TXCOMP);
        halEndOfTransfer(eptnum, STATUS_SUCCESS);

        // Enable interrupt if this is a control endpoint
        if ((status & AT91C_UDP_EPTYPE) == AT91C_UDP_EPTYPE_CTRL)
          halEnableEndPointInterrupt(eptnum);
      }
      else
      {
        // Transfer remaining data
        #ifdef USB_TRACE
          trace_LOG(trace_INFO, "%d ", endpoint->size);
        #endif
        transfer->transferred += endpoint->size;
        transfer->buffered -= endpoint->size;
        // Send next packet
        if (UDP_ENDPOINTS_BANKS(eptnum) == 1)
        {
          // No double buffering
          halWritePayload(eptnum);
          halSetCSR(eptnum, AT91C_UDP_TXPKTRDY);
          halClearCSR(eptnum, AT91C_UDP_TXCOMP);
        }
        else
        {
          // Double buffering
          halSetCSR(eptnum, AT91C_UDP_TXPKTRDY);
          halClearCSR(eptnum, AT91C_UDP_TXCOMP);
          halWritePayload(eptnum);
        }
        halEnableEndPointInterrupt(eptnum);
      }
    }
    else
    {
      // Acknowledge interrupt
      halClearCSR(eptnum, AT91C_UDP_TXCOMP);
      halEnableEndPointInterrupt(eptnum);
    }
  }

  // OUT packet received
  if (status & UDP_RXDATA)
  {
    #ifdef USB_TRACE
      trace_LOG(trace_INFO, "Rd ");
    #endif
    // Check that the endpoint is in Receiving state
    if (UDP_ENDPOINT_RECEIVING != endpoint->state)
    {
      // Check if an ACK has been received on a Control endpoint
      if (((status & AT91C_UDP_EPTYPE) == AT91C_UDP_EPTYPE_CTRL)
       && ((status & AT91C_UDP_RXBYTECNT) == 0))
      {
        // Acknowledge the data and finish the current transfer
        #ifdef USB_TRACE
          trace_LOG(trace_INFO, "Ack ");
        #endif
        halClearRxFlag(eptnum);
        halEndOfTransfer(eptnum, STATUS_SUCCESS);
        halEnableEndPointInterrupt(eptnum);
      }
      // Check if the data has been STALLed
      else if (status & AT91C_UDP_FORCESTALL)
      {
        // Discard STALLed data
        #ifdef USB_TRACE
          trace_LOG(trace_INFO, "Disc ");
        #endif
        halClearRxFlag(eptnum);
        halEnableEndPointInterrupt(eptnum);
      }
      // NAK the data
      else
      {
        #ifdef USB_TRACE
          trace_LOG(trace_INFO, "Nak ");
        #endif
        halDisableEndPointInterrupt(eptnum);
      }
    }
    // Endpoint is in Read state
    else
    {
      // Retrieve data and store it into the current transfer buffer
      uint16_t size = (uint16_t)(status >> 16);
      #ifdef USB_TRACE
        trace_LOG(trace_INFO, "%d ", size);
      #endif
      halReadPayload(eptnum, size);
      halClearRxFlag(eptnum);
      // Check if the transfer is finished
      if ((transfer->remaining == 0) || (size < endpoint->size))
      {
        // Enable interrupt if this is a control endpoint
        if ((status & AT91C_UDP_EPTYPE) == AT91C_UDP_EPTYPE_CTRL)
          halEnableEndPointInterrupt(eptnum);
        halEndOfTransfer(eptnum, STATUS_SUCCESS);
      }
    }
  }

  // SETUP packet received
  if (status & AT91C_UDP_RXSETUP)
  {
    #ifdef USB_TRACE
      trace_LOG(trace_INFO, "Stp ");
    #endif
    // If a transfer was pending, complete it
    // Handles the case where during the status phase of a control write
    // transfer, the host receives the device ZLP and ack it, but the ack
    // is not received by the device
    if ((UDP_ENDPOINT_RECEIVING == endpoint->state) || (UDP_ENDPOINT_SENDING == endpoint->state))
    {
      halEndOfTransfer(eptnum, STATUS_SUCCESS);
    }
    halReadRequest(requestMemory, eptnum);

    // Set the DIR bit before clearing RXSETUP in Control IN sequence
    // Transfer direction is located in bit D7 of the bmRequestType field
    if (requestMemory[0] & 0x80)
    {
      halSetCSR(eptnum, AT91C_UDP_DIR);
    }
    halClearCSR(eptnum, AT91C_UDP_RXSETUP);
    halEnableEndPointInterrupt(eptnum);

    // Forward the request to the upper layer
    if (sendReqToUpper)
      sendReqToUpper(requestMemory);
  }

  // STALL sent
  if (status & AT91C_UDP_STALLSENT)
  {
    #ifdef USB_TRACE
      trace_LOG(trace_INFO, "Sta ");
    #endif
    // If the endpoint is not halted, clear the STALL condition
    halClearCSR(eptnum, AT91C_UDP_STALLSENT);
    if (endpoint->state != UDP_ENDPOINT_HALTED)
    {
      halClearCSR(eptnum, AT91C_UDP_FORCESTALL);
    }
    halEnableEndPointInterrupt(eptnum);
  }
}

/******************************************************************************
Sets UDP endpoint parameters

Parameters:
   eptnum - endpoint number
   type - endpoint type
   direction - transaction direction
******************************************************************************/
void halConfigureEndpoint(uint8_t eptnum, uint8_t type, HAL_UsbEndPointDirect_t direction)
{
  // Reset Endpoint Fifos
  AT91C_BASE_UDP->UDP_RSTEP |= (1 << eptnum);
  AT91C_BASE_UDP->UDP_RSTEP &= ~(1 << eptnum);

  // Configure endpoint
  halSetCSR(eptnum, AT91C_UDP_EPEDS | (type << 8) | (direction << 10));
  if (EP_CONTROL == type)
    halEnableEndPointInterrupt(eptnum);

  #ifdef USB_TRACE
    trace_LOG(trace_INFO, "CfgEpt%d ", eptnum);
  #endif
}

/******************************************************************************
Loads data to current FIFO and starts sending

Parameters:
  eptnum - Endpoint number.
  remaining - data remaining
******************************************************************************/
void halStartUsbWrite(uint8_t eptnum, uint32_t remaining)
{
  while(AT91C_UDP_TXPKTRDY == (AT91C_BASE_UDP->UDP_CSR[eptnum] & AT91C_UDP_TXPKTRDY));

  halWritePayload(eptnum);
  ATOMIC_SECTION_ENTER
  halSetCSR(eptnum, AT91C_UDP_TXPKTRDY);
  ATOMIC_SECTION_LEAVE

  // If double buffering is enabled and there is data remaining,
  // prepare another packet
  if ((UDP_ENDPOINTS_BANKS(eptnum) > 1) && (remaining > 0))
  {
    halWritePayload(eptnum);
  }
}

/******************************************************************************
Sets the HALT feature on the given endpoint.

Parameters:
  eptnum - Endpoint number.
******************************************************************************/
void halEndpointHaltState(uint8_t eptnum)
{
  // Put endpoint into Halt state
  halSetCSR(eptnum, AT91C_UDP_FORCESTALL);
  endpoints[eptnum].state = UDP_ENDPOINT_HALTED;
}

/******************************************************************************
Clears the Halt feature on the given endpoint.

Parameters:
  eptnum - Endpoint number.
******************************************************************************/
void halEndpointUnHaltState(uint8_t eptnum)
{
  // Clear FORCESTALL flag
  halClearCSR(eptnum, AT91C_UDP_FORCESTALL);

  // Reset Endpoint Fifos, beware this is a 2 steps operation
  AT91C_BASE_UDP->UDP_RSTEP |= 1 << eptnum;
  AT91C_BASE_UDP->UDP_RSTEP &= ~(1 << eptnum);
}

/******************************************************************************
Sends STALL packet

Parameters:
  eptnum - Endpoint number.
******************************************************************************/
void halSendStallToHost(uint8_t eptnum)
{
  halSetCSR(eptnum, AT91C_UDP_FORCESTALL);
}

/******************************************************************************
Starts a remote wake-up procedure.
******************************************************************************/
/*void HAL_RemoteWakeUp(void)
{
  halEnablePeripheralClock();
  halEnableUsbClock();
  halEnableTransceiver();

  trace_LOG(trace_INFO, "RWUp ");

  // Activates a remote wakeup (edge on ESR), then clear ESR
  AT91C_BASE_UDP->UDP_GLBSTATE |= AT91C_UDP_ESR;
  AT91C_BASE_UDP->UDP_GLBSTATE &= ~AT91C_UDP_ESR;
}*/

/******************************************************************************
Sets the device address to the given value.

Parameters:
  address - New device address.
******************************************************************************/
void halSetUsbAddress(uint8_t address)
{
  // Set address
  AT91C_BASE_UDP->UDP_FADDR = AT91C_UDP_FEN | address;

  // If the address is non-zero, the device enters the Address state
  if (address)
    AT91C_BASE_UDP->UDP_GLBSTATE = AT91C_UDP_FADDEN;
  // If the address is 0, the device returns to the Default state
  else
    AT91C_BASE_UDP->UDP_GLBSTATE = 0;
}

/******************************************************************************
Sets the current device configuration.

Parameters:
  cfgnum - Configuration number to set.
******************************************************************************/
void halSetUsbConfigState(uint8_t cfgnum)
{
  // Configured state
  if (cfgnum)
    AT91C_BASE_UDP->UDP_GLBSTATE |= AT91C_UDP_CONFG;
  // If the configuration number is zero, the device goes back to the Address state
  else
    AT91C_BASE_UDP->UDP_GLBSTATE = AT91C_UDP_FADDEN;
}

// eof halUsb.c
