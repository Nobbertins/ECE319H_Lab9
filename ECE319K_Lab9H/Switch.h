/*
 * Switch.h
 *
 *  Created on: Nov 5, 2023
 *      Author: jonat
 */

#ifndef SWITCH_H_
#define SWITCH_H_

// initialize your switches
void Buttons_Init(void);

// return current state of switches
bool readShootButton(void);
bool readReloadButton(void);


#endif /* SWITCH_H_ */
