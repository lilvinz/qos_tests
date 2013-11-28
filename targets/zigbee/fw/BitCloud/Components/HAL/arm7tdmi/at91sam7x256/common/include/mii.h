/**************************************************************************//**
\file  mii.h

\brief Desription of registers of MII interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    11/02/08 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/* Generic MII registers. */
#ifdef PLATFORM_SAM7X_EK_RF2XX
  #define MII_BMCR            0x00        /* Basic mode control register */
  #define MII_BMSR            0x01        /* Basic mode status register  */
  #define MII_PHYSID1         0x02        /* PHYS ID 1                   */
  #define MII_PHYSID2         0x03        /* PHYS ID 2                   */
  #define MII_ADVERTISE       0x04        /* Advertisement control reg   */
  #define MII_LPA             0x05        /* Link partner ability reg    */
  #define MII_EXPANSION       0x06        /* Expansion register          */
  #define MII_DCOUNTER        0x12        /* Disconnect counter          */
  #define MII_FCSCOUNTER      0x13        /* False carrier counter       */
  #define MII_NWAYTEST        0x14        /* N-way auto-neg test reg     */
  #define MII_RERRCOUNTER     0x15        /* Receive error counter       */
  #define MII_SREVISION       0x16        /* Silicon revision            */
  #define MII_RESV1           0x17        /* Reserved...                 */
  #define MII_LBRERROR        0x18        /* Lpback, rx, bypass error    */
  #define MII_PHYADDR         0x19        /* PHY address                 */
  #define MII_RESV2           0x1a        /* Reserved...                 */
  #define MII_TPISTATUS       0x1b        /* TPI status for 10mbps       */
  #define MII_NCONFIG         0x1c        /* Network interface config    */
#else
  #define MII_BMCR            0x00        /* Basic mode control register */
  #define MII_BMSR            0x01        /* Basic mode status register  */
  #define MII_PHYSID1         0x02        /* PHYS ID 1                   */
  #define MII_PHYSID2         0x03        /* PHYS ID 2                   */
  #define MII_ADVERTISE       0x04        /* Advertisement control reg   */
  #define MII_LPA             0x05        /* Link partner ability reg    */
  #define MII_EXPANSION       0x06        /* Expansion register          */
  #define MII_SREVISION       0x10        /* Silicon revision            */
  #define MII_MODECONTROL     0x11        /* Mode Control/Status Register*/
  #define MII_SPECMODE        0x12        /* Special Modes Register      */
  #define MII_SYMERRCOUNT     0x1A        /* Symbol error countrer       */
  #define MII_SPECCONTRIND    0x1B        /* Special Contr/Stat Indication*/
  #define MII_INTSRCFLAGS     0x1D        /* Intrpt source flags         */
  #define MII_INTMASK         0x1E        /* Intrpt mask                 */
  #define MII_PHYSPECCONTR    0x1F        /* PHY special control/status  */
#endif

/* Basic mode control register. */
#define BMCR_RESV               0x007f  /* Unused...                   */
#define BMCR_CTST               0x0080  /* Collision test              */
#define BMCR_FULLDPLX           0x0100  /* Full duplex                 */
#define BMCR_ANRESTART          0x0200  /* Auto negotiation restart    */
#define BMCR_ISOLATE            0x0400  /* Disconnect PHY from MII     */
#define BMCR_PDOWN              0x0800  /* Powerdown the LN8700(DM9161)*/
#define BMCR_ANENABLE           0x1000  /* Enable auto negotiation     */
#define BMCR_SPEED100           0x2000  /* Select 100Mbps              */
#define BMCR_LOOPBACK           0x4000  /* TXD loopback bits           */
#define BMCR_RESET              0x8000  /* Reset the LN8700(DM9161)    */

/* Basic mode status register. */
#define BMSR_ERCAP              0x0001  /* Ext-reg capability          */
#define BMSR_JCD                0x0002  /* Jabber detected             */
#define BMSR_LSTATUS            0x0004  /* Link status                 */
#define BMSR_ANEGCAPABLE        0x0008  /* Able to do auto-negotiation */
#define BMSR_RFAULT             0x0010  /* Remote fault detected       */
#define BMSR_ANEGCOMPLETE       0x0020  /* Auto-negotiation complete   */
#define BMSR_RESV               0x07c0  /* Unused...                   */
#define BMSR_10HALF             0x0800  /* Can do 10mbps, half-duplex  */
#define BMSR_10FULL             0x1000  /* Can do 10mbps, full-duplex  */
#define BMSR_100HALF            0x2000  /* Can do 100mbps, half-duplex */
#define BMSR_100FULL            0x4000  /* Can do 100mbps, full-duplex */
#define BMSR_100BASE4           0x8000  /* Can do 100mbps, 4k packets  */

/* Advertisement control register. */
#ifdef PLATFORM_SAM7X_EK_RF2XX
  #define ADVERTISE_SLCT          0x001f  /* Selector bits               */
  #define ADVERTISE_CSMA          0x0001  /* Only selector supported     */
  #define ADVERTISE_10HALF        0x0020  /* Try for 10mbps half-duplex  */
  #define ADVERTISE_10FULL        0x0040  /* Try for 10mbps full-duplex  */
  #define ADVERTISE_100HALF       0x0080  /* Try for 100mbps half-duplex */
  #define ADVERTISE_100FULL       0x0100  /* Try for 100mbps full-duplex */
  #define ADVERTISE_100BASE4      0x0200  /* Try for 100mbps 4k packets  */
  #define ADVERTISE_RESV          0x1c00  /* Unused...                   */
  #define ADVERTISE_RFAULT        0x2000  /* Say we can detect faults    */
  #define ADVERTISE_LPACK         0x4000  /* Ack link partners response  */
  #define ADVERTISE_NPAGE         0x8000  /* Next page bit               */
#else
  #define ADVERTISE_SLCT          0x001f  /* Selector bits               */
  #define ADVERTISE_CSMA          0x0001  /* Only selector supported     */
  #define ADVERTISE_10HALF        0x0020  /* Try for 10mbps half-duplex  */
  #define ADVERTISE_10FULL        0x0040  /* Try for 10mbps full-duplex  */
  #define ADVERTISE_100HALF       0x0080  /* Try for 100mbps half-duplex */
  #define ADVERTISE_100FULL       0x0100  /* Try for 100mbps full-duplex */
  #define ADVERTISE_100BASE4      0x0200  /* Try for 100mbps 4k packets  */
  #define ADVERTISE_PAUSE         0x0c00  /* Pause Operation             */
  #define ADVERTISE_RESV          0x1000  /* Unused...                   */
  #define ADVERTISE_RFAULT        0x2000  /* Say we can detect faults    */
  #define ADVERTISE_RESVD         0x4000  /* Unused...                   */
  #define ADVERTISE_NPAGE         0x8000  /* Next page bit               */
#endif

#define ADVERTISE_FULL (ADVERTISE_100FULL | ADVERTISE_10FULL | \
                        ADVERTISE_CSMA)
#define ADVERTISE_ALL (ADVERTISE_10HALF | ADVERTISE_10FULL | \
                       ADVERTISE_100HALF | ADVERTISE_100FULL)

/* Link partner ability register. */
#ifdef PLATFORM_SAM7X_EK_RF2XX
  #define LPA_SLCT                0x001f  /* Same as advertise selector  */
  #define LPA_10HALF              0x0020  /* Can do 10mbps half-duplex   */
  #define LPA_10FULL              0x0040  /* Can do 10mbps full-duplex   */
  #define LPA_100HALF             0x0080  /* Can do 100mbps half-duplex  */
  #define LPA_100FULL             0x0100  /* Can do 100mbps full-duplex  */
  #define LPA_100BASE4            0x0200  /* Can do 100mbps 4k packets   */
  #define LPA_RESV                0x1c00  /* Unused...                   */
  #define LPA_RFAULT              0x2000  /* Link partner faulted        */
  #define LPA_LPACK               0x4000  /* Link partner acked us       */
  #define LPA_NPAGE               0x8000  /* Next page bit               */
#else
  #define LPA_SLCT                0x001f  /* Same as advertise selector  */
  #define LPA_10HALF              0x0020  /* Can do 10mbps half-duplex   */
  #define LPA_10FULL              0x0040  /* Can do 10mbps full-duplex   */
  #define LPA_100HALF             0x0080  /* Can do 100mbps half-duplex  */
  #define LPA_100FULL             0x0100  /* Can do 100mbps full-duplex  */
  #define LPA_100BASE4            0x0200  /* Can do 100mbps 4k packets   */
  #define LPA_PAUSE               0x0400  /* Pause operation support     */
  #define LPA_RESV                0x1800  /* Unused...                   */
  #define LPA_RFAULT              0x2000  /* Link partner faulted        */
  #define LPA_LPACK               0x4000  /* Link partner acked us       */
  #define LPA_NPAGE               0x8000  /* Next page bit               */
#endif

#define LPA_DUPLEX              (LPA_10FULL | LPA_100FULL)
#define LPA_100                 (LPA_100FULL | LPA_100HALF | LPA_100BASE4)

/* Expansion register for auto-negotiation. */
#define EXPANSION_NWAY          0x0001  /* Can do N-way auto-nego      */
#define EXPANSION_LCWP          0x0002  /* Got new RX page code word   */
#define EXPANSION_ENABLENPAGE   0x0004  /* This enables npage words    */
#define EXPANSION_NPCAPABLE     0x0008  /* Link partner supports npage */
#define EXPANSION_MFAULTS       0x0010  /* Multiple faults detected    */
#define EXPANSION_RESV          0xffe0  /* Unused...                   */

/* N-way test register. */
#define NWAYTEST_RESV1          0x00ff  /* Unused...                   */
#define NWAYTEST_LOOPBACK       0x0100  /* Enable loopback for N-way   */
#define NWAYTEST_RESV2          0xfe00  /* Unused...                   */

#define SPEED_10                10
#define SPEED_100               100

/* Duplex, half or full. */
#define DUPLEX_HALF             0x00
#define DUPLEX_FULL             0x01

/* PHY ID */
#define MII_DM9161_ID     0x0181b8a0
// with manufacturer's revision number 0x0007c0c3
#define MII_LN8700_ID     0x0007c0c0

#ifdef PLATFORM_SAM7X_EK_RF2XX
  #define AT91C_PHY_ADDR        31
#else
  #define AT91C_PHY_ADDR        16
#endif

// eof mii.h
