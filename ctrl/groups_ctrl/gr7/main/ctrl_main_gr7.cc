/*! 
 * \author Group 7
 * \file controller_main_gr7.cc
 * \brief Initialization, loop and finilization of the controller written in C (but compiled as C++)
 */


//blabla testzone blabla
//test.......

#include "ctrl_main_gr7.h"
#include "namespace_ctrl.h"
#include "init_pos_gr7.h"
#include "odometry_gr7.h"
#include "opp_pos_gr7.h"
#include "speed_regulation_gr7.h"
#include "calibration_gr7.h"
#include "triangulation_gr7.h"
#include "strategy_gr7.h"

NAMESPACE_INIT(ctrlGr7);


/*! \brief initialize controller operations (called once)
 * 
 * \param[in] cvs controller main structure
 */
void controller_init(CtrlStruct *cvs)
{
	// variables declaration
	double t;
	CtrlIn *inputs;
	
	inputs = cvs->inputs;
	t = inputs->t;

	// robot ID
	cvs->robot_id = inputs->robot_id;

	// robot team
	switch (inputs->robot_id)
	{
		case ROBOT_B: cvs->team_id = TEAM_A; break;
		case ROBOT_R: cvs->team_id = TEAM_A; break;
		case ROBOT_Y: cvs->team_id = TEAM_B; break;
		case ROBOT_W: cvs->team_id = TEAM_B; break;
	
		default:
			printf("Error: unknown robot ID: %d !\n", inputs->robot_id);
			exit(EXIT_FAILURE);
	}

	// number of opponents
	cvs->nb_opp = inputs->nb_opponents;

	// robot initial position
	set_init_position(cvs->robot_id, cvs->rob_pos);
	cvs->rob_pos->last_t = t;

	// speed regulation
	cvs->sp_reg->last_t = t;
	cvs->main_state = CALIB_STATE;
}

/*! \brief controller loop (called every time-step)
 * 
 * \param[in] cvs controller main structure
 */
void controller_loop(CtrlStruct *cvs)
{
	// variables declaration
	double t;
	CtrlIn *inputs;
	CtrlOut *outputs;

	// variables initialization
	inputs  = cvs->inputs;
	outputs = cvs->outputs;

	// time
	t = inputs->t;

	// update the robot odometry
	update_odometry(cvs);
	
	// triangulation
	triangulation(cvs);
	
	// opponents position
	// only do if tower had time to turn 1 time!
	if (t>-14){
		opponents_tower(cvs);
	}

	// tower control
	outputs->tower_command = 20;
	if (1) {
		if (t>-5) {
			speed_regulation(cvs, -5.0, +5.0);

		}
		else {
			speed_regulation(cvs, 0, 0);
		}
		return;
	}
	
	switch (cvs->main_state)
	{
		// calibration
		case CALIB_STATE:
			//printf("calib state\n");
			calibration(cvs);
			break;

		// wait before match beginning
		case WAIT_INIT_STATE:
			speed_regulation(cvs, 0.0, 0.0);

			if (t > 0.0)
			{
				cvs->main_state = RUN_STATE;
				cvs->strat->main_state = GAME_STATE_A;
			}
			break;

		// during game
		case RUN_STATE:
			main_strategy(cvs);

			if (t > 89.0) // 1 second safety
			{
				cvs->main_state = STOP_END_STATE;
			}
			break;

		// stop at the end of the game
		case STOP_END_STATE:
			speed_regulation(cvs, 0.0, 0.0);

			outputs->flag_release = 1;
			break;

		case NB_MAIN_STATES:
			printf("Error: state NB_MAIN_STATES should not be reached !\n");
			exit(EXIT_FAILURE);
			break;
	
		default:
			printf("Error:unknown state : %d !\n", cvs->main_state);
			exit(EXIT_FAILURE);
	}
}

/*! \brief last controller operations (called once)
 * 
 * \param[in] cvs controller main structure
 */
void controller_finish(CtrlStruct *cvs)
{

}

NAMESPACE_CLOSE();
