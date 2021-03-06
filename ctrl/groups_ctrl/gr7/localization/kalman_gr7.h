/*! 
 * \author Group 7
 * \file kalman_gr7.h
 * \brief localization sensors fusion with Kalman
 */

#ifndef _KALMAN_GR7_H_
#define _KALMAN_GR7_H_

#include "CtrlStruct_gr7.h"
#include "init_pos_gr7.h"

NAMESPACE_INIT(ctrlGr7);

/// Kalman main structure
struct KalmanStruc
{
	int dummy_variable; ///< put your own variable, this is just an example without purpose
};

void kalman(CtrlStruct *cvs);

NAMESPACE_CLOSE();

#endif
