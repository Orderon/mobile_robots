#include "triangulation_gr7.h"
#include "useful_gr7.h"
#include "init_pos_gr7.h"
#include <math.h>

NAMESPACE_INIT(ctrlGr7);

/*! \brief set the fixed beacons positions, depending on the team
 * 
 * \param[in] team_id ID of the team ('TEAM_A' or 'TEAM_B')
 * \param[out] x_beac_1 first beacon x position [m]
 * \param[out] y_beac_1 first beacon y position [m]
 * \param[out] x_beac_2 second beacon x position [m]
 * \param[out] y_beac_2 second beacon y position [m]
 * \param[out] x_beac_3 third beacon x position [m]
 * \param[out] y_beac_3 third beacon y position [m]
 *
 * This function can be adapted, depending on the map.
 */
void fixed_beacon_positions(int team_id, double *x_beac_1, double *y_beac_1,
	double *x_beac_2, double *y_beac_2, double *x_beac_3, double *y_beac_3)
{
	switch (team_id)
	{
		case TEAM_A:
			*x_beac_1 = 0.0;
			*y_beac_1 = 0.0;

			*x_beac_2 = 0.0;
			*y_beac_2 = 0.0;

			*x_beac_3 = 0.0;
			*y_beac_3 = 0.0;
			break;

		case TEAM_B:
			*x_beac_1 = 0.0;
			*y_beac_1 = 0.0;

			*x_beac_2 = 0.0;
			*y_beac_2 = 0.0;

			*x_beac_3 = 0.0;
			*y_beac_3 = 0.0;
			break;
	
		default:
			printf("Error unknown team ID (%d) !\n", team_id);
			exit(EXIT_FAILURE);
	}
}

/*! \brief get the index of the best angle prediction
 * 
 * \param[in] alpha_predicted angle to reach [rad]
 * \param[in] alpha_a angle computed for A [rad]
 * \param[in] alpha_b angle computed for B [rad]
 * \param[in] alpha_c angle computed for C [rad]
 * \return best index (0, 1, or 2)
 */
int index_predicted(double alpha_predicted, double alpha_a, double alpha_b, double alpha_c)
{
	double pred_err_a, pred_err_b, pred_err_c;

	pred_err_a = fabs(limit_angle(alpha_a - alpha_predicted));
	pred_err_b = fabs(limit_angle(alpha_b - alpha_predicted));
	pred_err_c = fabs(limit_angle(alpha_c - alpha_predicted));

	return (pred_err_a < pred_err_b) ? ((pred_err_a < pred_err_c) ? 0 : 2) : ((pred_err_b < pred_err_c) ? 1 : 2);
}

/*! \brief triangulation main algorithm
 * 
 * \param[in] cvs controller main structure
 *
 * computation found here: http://www.telecom.ulg.ac.be/triangulation/
 */
void triangulation(CtrlStruct *cvs)
{
	// variables declaration
	RobotPosition *pos_tri, *rob_pos;
	CtrlIn *inputs;

	int alpha_1_index, alpha_2_index, alpha_3_index;
	int rise_index_1, rise_index_2, rise_index_3;
	int fall_index_1, fall_index_2, fall_index_3;

	double alpha_a, alpha_b, alpha_c;
	double alpha_1, alpha_2, alpha_3;
	double alpha_1_predicted, alpha_2_predicted, alpha_3_predicted;
	double x_beac_1, y_beac_1, x_beac_2, y_beac_2, x_beac_3, y_beac_3;

	// variables initialization
	pos_tri = cvs->triang_pos;
	rob_pos = cvs->rob_pos;
	inputs  = cvs->inputs;

	// safety
	if ((inputs->rising_index_fixed < 0) || (inputs->falling_index_fixed < 0))
	{
		return;
	}

	// known positions of the beacons
	fixed_beacon_positions(cvs->team_id, &x_beac_1, &y_beac_1, &x_beac_2, &y_beac_2, &x_beac_3, &y_beac_3);	

	// indexes fot the angles detection
	rise_index_1 = inputs->rising_index_fixed;
	rise_index_2 = (rise_index_1 - 1 < 0) ? NB_STORE_EDGE-1 : rise_index_1 - 1;
	rise_index_3 = (rise_index_2 - 1 < 0) ? NB_STORE_EDGE-1 : rise_index_2 - 1;

	fall_index_1 = inputs->falling_index_fixed;
	fall_index_2 = (fall_index_1 - 1 < 0) ? NB_STORE_EDGE-1 : fall_index_1 - 1;
	fall_index_3 = (fall_index_2 - 1 < 0) ? NB_STORE_EDGE-1 : fall_index_2 - 1;

	// beacons angles measured with the laser (to compute)
	alpha_a = 0.0;
	alpha_b = 0.0;
	alpha_c = 0.0;

	// beacons angles predicted thanks to odometry measurements (to compute)
	alpha_1_predicted = 0.0;
	alpha_2_predicted = 0.0;
	alpha_3_predicted = 0.0;

	// indexes of each beacon
	alpha_1_index = index_predicted(alpha_1_predicted, alpha_a, alpha_b, alpha_c);
	alpha_2_index = index_predicted(alpha_2_predicted, alpha_a, alpha_b, alpha_c);
	alpha_3_index = index_predicted(alpha_3_predicted, alpha_a, alpha_b, alpha_c);

	// safety
	if ((alpha_1_index == alpha_2_index) || (alpha_1_index == alpha_3_index) || (alpha_2_index == alpha_3_index))
	{
		return;
	}

	// angle of the first beacon
	switch (alpha_1_index)
	{
		case 0: alpha_1 = alpha_a; break;
		case 1: alpha_1 = alpha_b; break;
		case 2: alpha_1 = alpha_c; break;
	
		default:
			printf("Error: unknown index %d !\n", alpha_1_index);
			exit(EXIT_FAILURE);
	}

	// angle of the second beacon
	switch (alpha_2_index)
	{
		case 0: alpha_2 = alpha_a; break;
		case 1: alpha_2 = alpha_b; break;
		case 2: alpha_2 = alpha_c; break;
	
		default:
			printf("Error: unknown index %d !\n", alpha_2_index);
			exit(EXIT_FAILURE);
	}

	// angle of the third beacon
	switch (alpha_3_index)
	{
		case 0: alpha_3 = alpha_a; break;
		case 1: alpha_3 = alpha_b; break;
		case 2: alpha_3 = alpha_c; break;
	
		default:
			printf("Error: unknown index %d !\n", alpha_3_index);
			exit(EXIT_FAILURE);
	}
	

	// ----- triangulation computation start ----- //

	// robot position
	pos_tri->x = 0.0;
	pos_tri->y = 0.0;

	// robot orientation
	pos_tri->theta = 0.0;

	// ----- triangulation computation end ----- //
}

NAMESPACE_CLOSE();
