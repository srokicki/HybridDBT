/*
 * optimizeBasicBlock.h
 *
 *  Created on: 16 nov. 2016
 *      Author: simon
 */

#ifndef INCLUDES_TRANSFORMATION_OPTIMIZEBASICBLOCK_H_
#define INCLUDES_TRANSFORMATION_OPTIMIZEBASICBLOCK_H_

#include <isa/irISA.h>
void optimizeBasicBlock(IRBlock *block, DBTPlateform *platform, IRApplication *application, uint32 placeCode);



#endif /* INCLUDES_TRANSFORMATION_OPTIMIZEBASICBLOCK_H_ */
