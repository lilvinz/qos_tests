/*
 * FactoryTest.h
 *
 *  Created on: 07.11.2011
 *      Author: cba
 */

#ifndef FACTORYTEST_H_
#define FACTORYTEST_H_

// Factory test is invoked by magic uart keyword if defined, otherwise a LOW RTS pin
#define CHECKSTARTMAGIC_UART    1

#define FACTORYTEST_MAGIC_REQUEST  0xfa12b8f8
#define FACTORYTEST_MAGIC_RESPONSE 0xb52ab27b



#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


void FactoryTest_CTTM(void);

#endif /* FACTORYTEST_H_ */
