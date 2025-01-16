#pragma once
/*
 * 
 */

#include <string>
#include <iostream>
using namespace std;

namespace traj_algo
{
//! @brief Struct to store the configuration settings for the LCIStrategicPlugin class
struct TrajConfig
{
  //! The maximum allowable vehicle deceleration limit in m/s
  double vehicle_decel_limit = 2.0;

  //! A multiplier to apply to the maximum allowable vehicle deceleration limit so we plan under our capabilities
  double vehicle_decel_limit_multiplier = 0.75;

  //! The maximum allowable vehicle acceleration limit in m/s
  double vehicle_accel_limit = 2.0;

  //! A multiplier to apply to the maximum allowable vehicle acceleration limit so we plan under our capabilities
  double vehicle_accel_limit_multiplier = 0.75;

  //! The minimum distance in meters that the vehicle can be at before requiring a transition to the APPROACH state
  double min_approach_distance = 30.0;

  //! Downtrack distance until nearest intersection where the Trajectory Smoothing algorithm should activate
  double trajectory_smoothing_activation_distance = 200.0;

  //! A buffer infront of the stopping location which will still be considered a valid stop. Units in meters
  double stopping_location_buffer = 3.0;

  //! A buffer in seconds around the green phase which will reduce the phase length such that vehicle still considers it non-green
  double green_light_time_buffer = 0.0;

  //! Minimum allowable speed TS algorithm in m/s
  double algo_minimum_speed = 2.2352;

  //! Stopping speed in m/s
  double algo_stopping_speed = 0.1;

  //! Double: Safety multiplier of planned allowable vehicle deceleration to use when stopping. This new deceleration makes vehicle decelerate earlier distance.
  //!      NOTE: Stacks on vehicle_decel_limit_multiplier and stopping uses max_decel; this distance is only used for calculating earlier downtrack
  double deceleration_fraction = 0.8;

  //! Double: Desired distance to stop buffer in meters
  double desired_distance_to_stop_buffer = 10.0;

  //! Double: Vehicle reaction time to a received schedule in seconds (approximate value, only used for communication with the schedule)
  double reaction_time = 2.0;

  //! Double: Minimum inter-vehicle gap
  double min_gap = 10.0;

};
}  // namespace localizer