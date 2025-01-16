
/***************************************************************************************

!!!

***************************************************************************************

  TrajAlgo.h  
  Created by: Saeid Soleimaniamiri
  Saxton Lab 
  Leidos

  ???

  ???

***************************************************************************************/

#include <math.h>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <vector>
#include "TrajConfig.h"

using namespace std;

namespace traj_algo
{


/**
 * \brief Struct representing trajectory smoothing algorithm parameters using distance and acceleration
 */
enum TSCase {
  CASE_1 = 1,
  CASE_2 = 2,
  CASE_3 = 3,
  CASE_4 = 4,
  CASE_5 = 5,
  CASE_6 = 6,
  CASE_7 = 7,
  CASE_8 = 8,
  STOPPING=9,
  UNAVAILABLE = 10,
  EMERGENCY_STOPPING=11,
  DEGRADED_TSCASE=12  // when not performing trajectory smoothing, but making through GREEN
};

struct TrajectoryParams
{
  double t0_ = 0;
  double v0_ = 0;
  double x0_ = 0;

  double a1_ = 0;
  double t1_ = 0;
  double v1_ = 0;
  double x1_ = 0;

  double a2_ = 0;
  double t2_ = 0;
  double v2_ = 0;
  double x2_ = 0;

  double a3_ = 0;
  double t3_ = 0;
  double v3_ = 0;
  double x3_ = 0;

  bool is_algorithm_successful = true;
  TSCase case_num;
  double modified_departure_speed = -1.0;  // modified departure speed if algorithm failed
  double modified_remaining_time = -1.0;  // modified departure time if algorithm failed

  double accel_next = 0.0;

};

struct BoundaryDistances
{
  double dx1 = 0.0;
  double dx2 = 0.0;
  double dx3 = 0.0;
  double dx4 = 0.0;
  double dx5 = 0.0;
};

/**
 * @brief Enum describing the possible states of the LCIStrategic Strategic Plugin
 * 
 */   
enum class TransitState{
  UNAVAILABLE=0, // State representing that there are no applicable intersections in range, so the plugin cannot plan
  APPROACHING=1, // State representing that the vehicle is approaching an intersection
  WAITING=2, // State representing that the vehicle is stopped and waiting at a light
  DEPARTING=3 // State representing that the vehicle is traversing the intersection
};

struct vehicle
{
  double sim_timestep_length = 0.0;
  double sim_timestep = 0.0;
  long veh_id = 0;
  double min_gap = 0.0;
  double reaction_time = 0.0;
  double speed_max = 0.0;
  double speed_current = 0.0;
  double speed_departure = 0.0;
  double accel_current = 0.0;
  double accel_max = 0.0;
  double decel_max = -10.0;
  double distance_to_signal = 0.0;
  long signal_state = 6;
  long signal_state_next = 6;
  double signal_remaining_time = 0.0;
  double signal_cycle_length = 0.0;
  double signal_start_time = 0.0;

  TransitState transit_state_cur = TransitState::UNAVAILABLE;
  TransitState transit_state_prev = TransitState::UNAVAILABLE;
  bool can_depart = false;

  double accel_desired_suggestion = 0.0;
  double accel_desired_vissim = 0.0;
  double speed_desired_suggestion = 0.0;
  double speed_desired_vissim = 0.0;
};


class TrajAlgo 
{


  public:

    void plan_maneuvers(vehicle& veh_info);


    
  private:

    TrajConfig config;

    // TS planning related variables
    double max_comfort_accel_ = config.vehicle_accel_limit * config.vehicle_accel_limit_multiplier;
    double max_comfort_decel_ = -1 * config.vehicle_decel_limit * config.vehicle_decel_limit_multiplier;
    double max_comfort_decel_norm_ = config.vehicle_decel_limit * config.vehicle_decel_limit_multiplier;
    double emergency_decel_norm_ = 2 * config.vehicle_decel_limit * config.vehicle_decel_limit_multiplier;
    double epsilon_ = 0.001; //Small constant to compare (double) 0.0 with
    double accel_epsilon_ = 0.0001; //Small constant to compare (double) 0.0 with


    void update_transit_state(vehicle& veh_info);


    void planWhenUNAVAILABLE(vehicle& veh_info);


    void planWhenAPPROACHING(vehicle& veh_info);


    void planWhenWAITING(vehicle& veh_info);


    void planWhenDEPARTING(vehicle& veh_info);


    /**
     * 
     */
    double get_earliest_entry_time(double remaining_distance, double free_flow_speed, double current_speed, double departure_speed, double max_accel, double max_decel) const;


    /**
       */

    double get_nearest_green_entry_time(const double& current_time, const double& earliest_entry_time, vehicle& veh_info, double minimum_required_green_time = 0.0) const;
    

    /**
     * 
     */
    double get_distance_to_accel_or_decel_once(double current_speed, double departure_speed, double max_accel, double max_decel) const;


    /**
     * 
     */
    double get_distance_to_accel_or_decel_twice(double free_flow_speed, double current_speed, double departure_speed, double max_accel, double max_decel) const;


    /**
     * 
     */
    double get_inflection_speed_value(double x, double x1, double x2, double free_flow_speed, double current_speed, double departure_speed, double max_accel, double max_decel) const;


    // /**
    //  * \brief Helper method to use basic kinematics to compute an estimated stopping distance from from the inputs
    //  *
    //  * \param v The initial velocity in m/s
    //  * \param a The deceleration in m/s^2
    //  *
    //  * \return The estimated stopping distance in meters
    //  */
    // double estimate_distance_to_stop(double v, double a) const;


    // /**
    //  * \brief Helper method to use basic kinematics to compute an estimated stopping time from from the inputs
    //  *
    //  * \param d The distance to travel in meters
    //  * \param v The initial velocity in m/s
    //  *
    //  * \return The estimated stopping time in seconds
    //  */
    // double estimate_time_to_stop(double d, double v) const;


    /**
     * \brief Get boundary distances used to compare against current state in order to create trajectory smoothing parameters
     *
     * \param v0 starting_speed
     * \param v1 departure_speed
     * \param v_max speed_limit
     * \param v_min minimum speed
     * \param a_max max_comfort_acceleration limit
     * \param a_min max_comfort_deceleration limit
     * \return boundary distances used to generate trajectory smoothing segments
     */
    BoundaryDistances get_delta_x(double v0, double v1, double v_max, double v_min, double a_max, double a_min);


    /**
     * \brief Get all possible trajectory smoothing parameters for each segments. Later this will be used to generate a single trajectory
     * \param t current time in seconds
     * \param v0 starting_speed
     * \param v1 departure_speed
     * \param v_max speed_limit
     * \param v_min minimum speed
     * \param a_max max_comfort_acceleration limit
     * \param a_min max_comfort_deceleration limit
     * \param x0 current_downtrack
     * \param x_end traffic_light_down_track
     * \param dx distance_remaining_to_traffic_light
     * \param boundary_distances boundary_distances to compare against current state
     * \return all possible trajectory smoothing parameters to later generate single trajectory
     */
    vector<TrajectoryParams> get_boundary_traj_params(double t, double v0, double v1, double v_max, double v_min, double a_max, double a_min, double x0, double x_end, double dx, BoundaryDistances boundary_distances);


    TrajectoryParams boundary_accel_or_decel_incomplete_upper(double t, double v0, double v1, double a_max, double a_min, double x0, double x_end, double dx);
    TrajectoryParams boundary_accel_nocruise_notmaxspeed_decel(double t, double v0, double v1, double a_max, double a_min, double x0, double x_end, double dx);
    TrajectoryParams boundary_accel_cruise_maxspeed_decel(double t, double v0, double v1, double v_max, double a_max, double a_min, double x0, double x_end, double dx);
    TrajectoryParams boundary_accel_nocruise_maxspeed_decel(double t, double v0, double v1, double v_max, double a_max, double a_min, double x0, double x_end, double dx);
    TrajectoryParams boundary_accel_or_decel_complete_upper(double t, double v0, double v1, double x0, double x_end, double dx);
    TrajectoryParams boundary_decel_nocruise_notminspeed_accel(double t, double v0, double v1, double v_min, double a_max, double a_min, double x0, double x_end, double dx);
    TrajectoryParams boundary_decel_nocruise_minspeed_accel_incomplete(double t, double v0, double v_min, double a_max, double a_min, double x0, double x_end, double dx);
    TrajectoryParams boundary_decel_nocruise_minspeed_accel_complete(double t, double v0, double v1, double v_max, double v_min, double a_max, double a_min, double x0, double x_end, double dx);
    TrajectoryParams boundary_decel_cruise_minspeed_accel(double t, double v0, double v1, double v_min, double a_max, double a_min, double x0, double x_end, double dx);
    TrajectoryParams boundary_decel_cruise_minspeed(double t, double v0, double v_min, double a_min, double x0, double x_end, double dx);
    TrajectoryParams boundary_decel_incomplete_lower(double t, double v0, double a_min, double x0, double x_end, double dx);
    TrajectoryParams boundary_decel_cruise_minspeed_decel(double t, double v0, double v_min, double a_min, double x0, double x_end, double dx);


    /**
     * \brief Trajectory Smoothing Algorithm 8 cases.
     */
    TrajectoryParams get_ts_case(double delta_t, double t, double et, double v0, double v1, double v_max, double v_min, double a_max, double a_min, double x0, double x_end, double dx, BoundaryDistances boundary_distances, std::vector<TrajectoryParams> params);


    TrajectoryParams ts_case1(double t, double et, double v0, double v1, double v_max, double a_max, double a_min, double x0, double x_end, double dx);
    TrajectoryParams ts_case2(double t, double et, double v0, double v1, double a_max, double a_min, double x0, double x_end, double dx);
    TrajectoryParams ts_case3(double t, double et, double v0, double v1, double a_max, double a_min, double x0, double x_end, double dx);
    TrajectoryParams ts_case4(double t, double et, double v0, double v1, double v_min, double a_max, double a_min, double x0, double x_end, double dx);
    TrajectoryParams ts_case5(double t, double et, double v0, double a_max, double a_min, double x0, double x_end, double dx);
    TrajectoryParams ts_case6(double t, double et, double v0, double v_min, double a_min, double x0, double x_end, double dx, double dx3, TrajectoryParams traj6);
    TrajectoryParams ts_case7(double t, double et, double v0, double v_min, double a_min, double x0, double x_end, double dx);
    TrajectoryParams ts_case8(double dx, double dx5, TrajectoryParams traj8);




    

};



}










