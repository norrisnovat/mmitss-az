

#include "TrajAlgo.h"



namespace traj_algo
{


void TrajAlgo::update_transit_state(vehicle& veh_info)
{
  
  veh_info.transit_state_prev = veh_info.transit_state_cur;


  if (veh_info.distance_to_signal < 0.0 || veh_info.distance_to_signal > config.trajectory_smoothing_activation_distance)
  {
    veh_info.transit_state_cur = TransitState::UNAVAILABLE;
    return;
  }

  if (veh_info.transit_state_prev == TransitState::UNAVAILABLE 
      && veh_info.distance_to_signal >= 0.0 
      && veh_info.distance_to_signal <= config.trajectory_smoothing_activation_distance)
  {
    veh_info.transit_state_cur = TransitState::APPROACHING;
    return;
  }

  if (veh_info.transit_state_prev == TransitState::APPROACHING 
      && veh_info.distance_to_signal <= config.desired_distance_to_stop_buffer)
  {
    if (veh_info.speed_current <= 0.1)
    {
      veh_info.transit_state_cur = TransitState::WAITING;
      return;
    }
    if (veh_info.signal_state == 3 && veh_info.can_depart == true)
    {
      veh_info.transit_state_cur = TransitState::DEPARTING;
      return;
    }
  }
  if (veh_info.transit_state_prev == TransitState::WAITING && veh_info.signal_state == 3)
  {
      veh_info.transit_state_cur = TransitState::DEPARTING;
      return;
  }

}


void TrajAlgo::plan_maneuvers(vehicle& veh_info){

  update_transit_state(veh_info);

  switch (veh_info.transit_state_cur)
  {
  case TransitState::UNAVAILABLE:
    planWhenUNAVAILABLE(veh_info);
    break;
  case TransitState::APPROACHING:
    planWhenAPPROACHING(veh_info);
    break;
  case TransitState::WAITING:
    planWhenWAITING(veh_info);
    break;
  case TransitState::DEPARTING:
    planWhenDEPARTING(veh_info);
    break;  

  default:
    break;
  }

}


void TrajAlgo::planWhenUNAVAILABLE(vehicle& veh_info)
{
  veh_info.accel_desired_suggestion = veh_info.accel_desired_vissim;
  veh_info.speed_desired_suggestion = veh_info.speed_desired_vissim;
}


void TrajAlgo::planWhenWAITING(vehicle& veh_info)
{
  veh_info.accel_desired_suggestion = veh_info.accel_desired_vissim;
  veh_info.speed_desired_suggestion = veh_info.speed_desired_vissim;
}


void TrajAlgo::planWhenDEPARTING(vehicle& veh_info)
{
  veh_info.accel_desired_suggestion = veh_info.accel_desired_vissim;
  veh_info.speed_desired_suggestion = veh_info.speed_desired_vissim;
}


void TrajAlgo::planWhenAPPROACHING(vehicle& veh_info)
{
  
  if (veh_info.speed_current <= config.algo_stopping_speed)
  {
    veh_info.accel_desired_suggestion = veh_info.accel_desired_vissim;
    return;
  }

  // double remaining_distance = veh_info.distance_to_signal;
  // double free_flow_speed = veh_info.speed_max;
  // double current_speed = veh_info.speed_current;
  // double departure_speed = veh_info.speed_max;
  double max_accel = min(veh_info.accel_max, max_comfort_accel_);
  double max_decel = max(veh_info.decel_max, max_comfort_decel_);
  double max_decel_norm = -max_decel;
  double emergency_decel_norm = 2 * max_decel_norm;
  double downtrack_vehicle = 0.0;
  double downtrack_intersection = downtrack_vehicle + veh_info.distance_to_signal;
  
  double min_speed = max(config.algo_stopping_speed, min(veh_info.speed_current, config.algo_minimum_speed));
  

  double earliest_entry_time = veh_info.sim_timestep + get_earliest_entry_time(veh_info.distance_to_signal, veh_info.speed_max, veh_info.speed_current, veh_info.speed_departure, max_accel, max_decel);

  double minimum_required_green_time = 0.0;

  double scheduled_entry_time = get_nearest_green_entry_time(veh_info.sim_timestep, earliest_entry_time, veh_info, minimum_required_green_time);

  
  auto boundary_distances = get_delta_x(veh_info.speed_current, veh_info.speed_max, veh_info.speed_max, config.algo_minimum_speed, max_accel, max_decel);
  auto boundary_traj_params = get_boundary_traj_params(veh_info.sim_timestep, veh_info.speed_current, veh_info.speed_departure, veh_info.speed_max, min_speed, max_accel, max_decel, downtrack_vehicle, downtrack_intersection, veh_info.distance_to_signal, boundary_distances);
  TrajectoryParams ts_params = get_ts_case(veh_info.sim_timestep_length, veh_info.sim_timestep, scheduled_entry_time, veh_info.speed_current, veh_info.speed_departure, veh_info.speed_max, min_speed, max_accel, max_decel, downtrack_vehicle, downtrack_intersection, veh_info.distance_to_signal, boundary_distances, boundary_traj_params);
  
  veh_info.accel_desired_suggestion = max(veh_info.decel_max, min(ts_params.accel_next, veh_info.accel_max));
  veh_info.accel_desired_suggestion = min(veh_info.accel_desired_suggestion, veh_info.accel_desired_vissim);
  veh_info.speed_desired_suggestion = veh_info.speed_current + (veh_info.accel_desired_suggestion * veh_info.sim_timestep_length);

  /*
  double emergency_distance_to_stop = pow(veh_info.speed_current, 2)/(2 * emergency_decel_norm) + config.stopping_location_buffer / 2; 
  double safe_distance_to_stop = pow(veh_info.speed_current, 2)/(2 * max_decel_norm) + config.stopping_location_buffer / 2; 
  double desired_distance_to_stop = pow(veh_info.speed_current, 2)/(2 * max_decel_norm * config_.deceleration_fraction) + config_.desired_distance_to_stop_buffer;

  emergency_distance_to_stop = max(emergency_distance_to_stop, config.stopping_location_buffer);
  safe_distance_to_stop = max(safe_distance_to_stop, config.stopping_location_buffer);
  desired_distance_to_stop = max(desired_distance_to_stop, config.stopping_location_buffer);

  // Basic RED signal violation check
  if (veh_info.distance_to_signal <= emergency_distance_to_stop || last_case_num_ == TSCase::EMERGENCY_STOPPING)
  {

  }
  */
}




double TrajAlgo::get_earliest_entry_time(double remaining_distance, double free_flow_speed, double current_speed, double departure_speed, double max_accel, double max_decel) const
{
  double x = remaining_distance;
  double x1 = get_distance_to_accel_or_decel_twice(free_flow_speed, current_speed, departure_speed, max_accel, max_decel);
  double x2 = get_distance_to_accel_or_decel_once(current_speed, departure_speed, max_accel, max_decel);
  double v_hat = get_inflection_speed_value(x, x1, x2, free_flow_speed, current_speed, departure_speed, max_accel, max_decel);

  // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "x: " << x << ", x2: " << x2 << ", x1: " << x1 << ", v_hat: " << v_hat);
  if (v_hat <= config.algo_minimum_speed - epsilon_ || isnan(v_hat))
  {
    // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "Detected that v_hat is smaller than allowed!!!: " << v_hat);
    v_hat = config.algo_minimum_speed;
  }

  if (v_hat >= free_flow_speed + epsilon_)
  {
    // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "Detected that v_hat is Bigger than allowed!!!: " << v_hat);
    v_hat = free_flow_speed;
  }

  double t_accel = 0.0;
  if ( x < x2 && current_speed > departure_speed)
  {
    t_accel = 0.0;
  }
  else
  {
    // t_accel = max((v_hat - current_speed) / max_accel, 0.0) * 1e9;
    t_accel = max((v_hat - current_speed) / max_accel, 0.0);
  }

  double t_decel = 0.0;
  if ( x < x2 && current_speed < departure_speed)
  {
    t_decel = 0.0;
  }
  else
  {
    if (x < x2)
    {
      t_decel = max((v_hat - current_speed) / max_decel, 0.0);
      // t_decel = max((v_hat - current_speed) / max_decel, 0.0) * 1e9;

    }
    else
    {
      t_decel = max((departure_speed - v_hat) / max_decel, 0.0);
      // t_decel = max((departure_speed - v_hat) / max_decel, 0.0) * 1e9;
    }
  }

  double t_cruise = 0.0;
  if (x1 <= x)
  {
    // t_cruise = max((x - x1)/v_hat, 0.0) * 1e9;
    t_cruise = max((x - x1)/v_hat, 0.0);
  }
  else
  {
    t_cruise = 0.0;
  }

  // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "t_accel: " <<  t_accel.seconds() << ", t_cruise: " << t_cruise.seconds() << ", t_decel: " << t_decel.seconds());
  return t_accel + t_cruise + t_decel;

}


double TrajAlgo::get_nearest_green_entry_time(const double& current_time, const double& earliest_entry_time, vehicle& veh_info, double minimum_required_green_time) const
{
  
  if (veh_info.signal_state == 3)
  {
    if (earliest_entry_time - current_time < veh_info.signal_remaining_time)
    {
      return earliest_entry_time;
    }
    else{
      return max(earliest_entry_time, veh_info.signal_start_time + veh_info.signal_cycle_length + 2);      
    }
  }
  else if (veh_info.signal_state == 2)
  {
    return max(earliest_entry_time, veh_info.signal_start_time + veh_info.signal_cycle_length - 5);
  }
  else if (veh_info.signal_state == 1)
  {
    if (earliest_entry_time - current_time < veh_info.signal_remaining_time)
    {
      return current_time + veh_info.signal_remaining_time;
    }
    else
    // else if (earliest_entry_time - current_time < veh_info.signal_start_time + veh_info.signal_cycle_length)
    {
      return earliest_entry_time;
    }
  }
  else{
    return earliest_entry_time;
  }

  /*
  double g = minimum_required_green_time;         // provided by considering min headways of vehicles in front
  double t = current_time;                        // time variable
  double eet = earliest_entry_time;               // earliest entry time
  
  // check if the signal even has a green signal
  bool has_green_signal = false;
  for (auto pair : signal->recorded_time_stamps)
  {
    if (isStateAllowedGreen(pair.second) && t <= pair.first )
    {
      has_green_signal = true;
      break;
    }
  }

  if (!has_green_signal)
  {
    return std::nullopt;
  }

  auto curr_pair = signal->predictState(t);
  if (!curr_pair)
    throw std::invalid_argument("Traffic signal with id:" + std::to_string(signal->id()) + ", does not have any recorded time stamps!");

  boost::posix_time::time_duration theta =  curr_pair.get().first - t;   // remaining time left in this state
  auto p = curr_pair.get().second;
  while ( 0.0 < g.total_milliseconds() || (!isStateAllowedGreen(p))) //green
  {
    if (isStateAllowedGreen(p))
    {
      if (g < theta)
      {
        t = t + g;
        theta = theta - g;
        g = boost::posix_time::seconds(0);
      }
      else
      {
        t = t + theta;
        g = g - theta;
        curr_pair = signal->predictState(t + boost::posix_time::milliseconds(20)); // select next phase
        p = curr_pair.get().second;
        theta = curr_pair.get().first - t;
      }
    }
    else
    {
      t = t + theta;
      curr_pair = signal->predictState(t + boost::posix_time::milliseconds(20)); // select next phase
      p = curr_pair.get().second;
      theta = curr_pair.get().first - t;
    }
  }

  if (t <= eet)
  {
    double cycle_duration = signal->fixed_cycle_duration.total_milliseconds()/1000.0;
    if (cycle_duration < 0.001) //if it is a dynamic traffic signal not fixed
      cycle_duration = lanelet::time::toSec(signal->recorded_time_stamps.back().first) - lanelet::time::toSec(signal->recorded_start_time_stamps.front());

    t = t + lanelet::time::durationFromSec(std::floor((eet - t).total_milliseconds()/1000.0/cycle_duration) * cycle_duration); //fancy logic was needed to compile
    curr_pair = signal->predictState(t + boost::posix_time::milliseconds(20)); // select next phase
    p = curr_pair.get().second;
    theta = curr_pair.get().first - t;
    while ( t < eet || !isStateAllowedGreen(p))
    {
      if ( (isStateAllowedGreen(p)) && eet - t < theta)
      {
        t = eet;
        theta = theta - (eet - t);
      }
      else
      {
        t = t + theta;
        curr_pair = signal->predictState(t + boost::posix_time::milliseconds(20)); // select next phase
        p = curr_pair.get().second;
        theta = curr_pair.get().first - t;
      }

      if (t != lanelet::time::timeFromSec(lanelet::time::INFINITY_END_TIME_FOR_NOT_ENOUGH_STATES))
        continue;

      // if no green signal left in the signal states, return
      return std::nullopt;
    }
  }
  // return rclcpp::Time(lanelet::time::toSec(t) * 1e9);
  return eet;
  */

}


double TrajAlgo::get_distance_to_accel_or_decel_once (double current_speed, double departure_speed, double max_accel, double max_decel) const
{
  if (current_speed <= departure_speed + epsilon_)
  {
    return (std::pow(departure_speed, 2) - std::pow(current_speed, 2))/(2 * max_accel);
  }
  else
  {
    return (std::pow(departure_speed, 2) - std::pow(current_speed, 2))/(2 * max_decel);
  }
}


double TrajAlgo::get_distance_to_accel_or_decel_twice(double free_flow_speed, double current_speed, double departure_speed, double max_accel, double max_decel) const
{
  // (v_e^2 - v^2)/ 2a_a + (v_d^2 - v_e^2)/ 2a_d
  return (std::pow(free_flow_speed, 2) - std::pow(current_speed, 2))/(2 * max_accel) + (std::pow(departure_speed, 2) - std::pow(free_flow_speed, 2))/(2* max_decel);
}


double TrajAlgo::get_inflection_speed_value(double x, double x1, double x2, double free_flow_speed, double current_speed, double departure_speed, double max_accel, double max_decel) const
{
  if (x >= x1)
  {
    return free_flow_speed;
  }
  else if (x1 > x && x > x2)
  {
    return std::sqrt(((2 * x * max_accel * max_decel) + (max_decel * std::pow(current_speed, 2)) - (max_accel * (std::pow(departure_speed, 2))))/(max_decel - max_accel));
  }
  else if (x2 >= x)
  {
    if (current_speed <= departure_speed)
    {
      return std::sqrt(2 * x * max_accel + std::pow(current_speed, 2));
    }
    else
    {
      return std::sqrt(2 * x * max_decel + std::pow(current_speed, 2));
    }
  }

}


// double TrajAlgo::estimate_distance_to_stop(double v, double a) const
// {
//   return (v * v) / (2.0 * a);
// }


// double TrajAlgo::estimate_time_to_stop(double d, double v) const
// {
//   return 2.0 * d / v;
// };



BoundaryDistances TrajAlgo::get_delta_x(double v0, double v1, double v_max, double v_min, double a_max, double a_min)
{
  BoundaryDistances distances;

  distances.dx1 = ((pow(v_max, 2) - pow(v0, 2)) / (2 * a_max)) + ((pow(v1, 2) - pow(v_max, 2)) / (2 * a_min));
  if (v1 > v0)
    distances.dx2 = ((pow(v1, 2) - pow(v0, 2)) / (2 * a_max));
  else
    distances.dx2 = ((pow(v1, 2) - pow(v0, 2)) / (2 * a_min));

  distances.dx3 = ((pow(v_min, 2) - pow(v0, 2)) / (2 * a_min)) + ((pow(v1, 2) - pow(v_min, 2)) / (2 * a_max));
  distances.dx4 = ((pow(v_min, 2) - pow(v0, 2)) / (2 * a_min));
  distances.dx5 = - pow(v0, 2) / (2 * a_min);

  return distances;
}


vector<TrajectoryParams> TrajAlgo::get_boundary_traj_params(double t, double v0, double v1, double v_max, double v_min, double a_max, double a_min, double x0, double x_end, double dx, BoundaryDistances boundary_distances)
{
  double dx1 = boundary_distances.dx1;
  double dx2 = boundary_distances.dx2;
  double dx3 = boundary_distances.dx3;
  double dx4 = boundary_distances.dx4;
  double dx5 = boundary_distances.dx5;
  TrajectoryParams traj1, traj2, traj3, traj4, traj5, traj6, traj7, traj8;

  // t1, t2, t3
  if (dx < dx2)
  {
    traj1 = boundary_accel_or_decel_incomplete_upper(t, v0, v1, a_max, a_min, x0, x_end, dx);
    traj2 = traj1;
    traj3 = traj1;
  }
  else if (dx < dx1)
  {
    traj1 = boundary_accel_nocruise_notmaxspeed_decel(t, v0, v1, a_max, a_min, x0, x_end, dx);
    traj2 = traj1;
    traj3 = boundary_accel_or_decel_complete_upper(t, v0, v1, x0, x_end, dx);
  }
  else
  {
    traj1 = boundary_accel_cruise_maxspeed_decel(t, v0, v1, v_max, a_max, a_min, x0, x_end, dx);
    traj2 = boundary_accel_nocruise_maxspeed_decel(t, v0, v1, v_max, a_max, a_min, x0, x_end, dx);
    traj3 = boundary_accel_or_decel_complete_upper(t, v0, v1, x0, x_end, dx);
  }
  // t4, t5, t6, t7
  if (dx < dx4)
  {
    traj4 = traj1;
    traj5 = traj1;
    traj6 = boundary_decel_incomplete_lower(t, v0, a_min, x0, x_end, dx);
    traj7 = traj6;
  }
  else if (dx < dx3)
  {
    if (dx < dx2)
    {
      traj4 = traj1;
    }
    else
    {
      traj4 = boundary_decel_nocruise_notminspeed_accel(t, v0, v1, v_min, a_max, a_min, x0, x_end, dx);
    }
    traj5 = traj4;
    traj6 = boundary_decel_nocruise_minspeed_accel_incomplete(t, v0, v_min, a_max, a_min, x0, x_end, dx);
    traj7 = boundary_decel_cruise_minspeed(t, v0, v_min, a_min, x0, x_end, dx);

  }
  else
  {
    traj4 = boundary_decel_nocruise_minspeed_accel_complete(t, v0, v1, v_max, v_min, a_max, a_min, x0, x_end, dx);
    traj5 = boundary_decel_cruise_minspeed_accel(t, v0, v1, v_min, a_max, a_min, x0, x_end, dx);
    traj6 = traj5;
    traj7 = boundary_decel_cruise_minspeed(t, v0, v_min, a_min, x0, x_end, dx);
  }
  // t8
  if (dx < dx4)
  {
    traj8 = traj6;
  }
  else if (dx < dx5)
  {
    traj8 = boundary_decel_incomplete_lower(t, v0, a_min, x0, x_end, dx);
  }
  else
  {
    traj8 = boundary_decel_cruise_minspeed_decel(t, v0, v_min, a_min, x0, x_end, dx);
  }

  return {traj1, traj2, traj3, traj4, traj5, traj6, traj7, traj8};
}


TrajectoryParams TrajAlgo::boundary_accel_or_decel_incomplete_upper(double t, double v0, double v1, double a_max, double a_min, double x0, double x_end, double dx)
{
  double t_end;

  if (v0 <= v1 + epsilon_)
    t_end = t + (sqrt(pow(v0, 2) + (2 * a_max * dx)) - v0)/a_max;
  else
    t_end = t + (sqrt(pow(v0, 2) + (2 * a_min * dx)) - v0) / a_min;

  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  traj.t1_ = t_end;

  if (v0 <= v1 + epsilon_)
  {
    traj.a1_ = a_max;
    traj.v1_ = sqrt(pow(v0, 2) + (2 * a_max * dx));
  }
  else
  {
    traj.a1_ = a_min;
    traj.v1_ = sqrt(pow(v0, 2) + (2 * a_min * dx));
  }

  traj.x1_ = x_end;

  traj.t2_ = traj.t1_;
  traj.a2_ = 0;
  traj.v2_ = traj.v1_;
  traj.x2_ = traj.x1_;

  traj.t3_ = traj.t1_;
  traj.a3_ = 0;
  traj.v3_ = traj.v1_;
  traj.x3_ = traj.x1_;

  return traj;
}


TrajectoryParams TrajAlgo::boundary_accel_nocruise_notmaxspeed_decel(double t, double v0, double v1, double a_max, double a_min, double x0, double x_end, double dx)
{

  double v_hat = sqrt(((2 * dx * a_max * a_min) + (a_min * pow(v0, 2)) - (a_max * pow(v1, 2))) / (a_min - a_max));
  double t_end = t + ((v_hat * (a_min - a_max)) - (v0 * a_min) + (v1 * a_max)) / (a_max * a_min);

  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  traj.v1_ = v_hat;
  traj.a1_ = a_max;
  traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);

  traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));

  traj.t2_ = t_end;
  traj.a2_ = a_min;
  traj.v2_ = v1;
  traj.x2_ = x_end;

  traj.t3_ = traj.t2_;
  traj.a3_ = 0;
  traj.v3_ = traj.v2_;
  traj.x3_ = traj.x2_;

  return traj;

}


TrajectoryParams TrajAlgo::boundary_accel_cruise_maxspeed_decel(double t, double v0, double v1, double v_max, double a_max, double a_min, double x0, double x_end, double dx)
{
  double t_end = t + (dx / v_max) + (pow(v_max - v0, 2) / (2 * a_max * v_max)) - (pow(v1 - v_max, 2) / (2 * a_min * v_max));

  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  traj.v1_ = v_max;
  traj.a1_ = a_max;
  traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);
  traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));

  traj.v2_ = v_max;
  traj.a2_ = 0;
  traj.t2_ = t_end - ((v1 - v_max) / a_min);
  traj.x2_ = x_end - ((pow(v1, 2) - pow(v_max, 2)) / (2 * a_min));

  traj.t3_ = t_end;
  traj.a3_ = a_min;
  traj.v3_ = v1;
  traj.x3_ = x_end;

  return traj;
}


TrajectoryParams TrajAlgo::boundary_accel_nocruise_maxspeed_decel(double t, double v0, double v1, double v_max, double a_max, double a_min, double x0, double x_end, double dx)
{
  double nom = (v_max - v0) + ((a_max / a_min) * (v1 - v_max));
  double den = (pow(v_max, 2) - pow(v0, 2)) + ((a_max / a_min) * (pow(v1, 2) - pow(v_max, 2)));

  if (den <= epsilon_ && den >= -epsilon_)
    throw std::invalid_argument("boundary_accel_nocruise_maxspeed_decel: Received den near zero..." + std::to_string(den));

  double t_end = t + (2 * dx * nom / den);

  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  double dt = t_end - t;
  double tc = 0;

  traj.v1_ = v_max;

  if (dt - tc <= epsilon_ && dt - tc >= -epsilon_)
    throw std::invalid_argument("boundary_accel_nocruise_maxspeed_decel: Received dt - tc near zero..." + std::to_string(dt - tc));

  traj.a1_ = (((1 - (a_max / a_min)) * v_max) + ((a_max / a_min) * v1) - v0) / (dt - tc);

  if (traj.a1_ <= accel_epsilon_ && traj.a1_ >= -accel_epsilon_)
  {
    // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "boundary_accel_nocruise_maxspeed_decel: Received traj.a1_ near zero...");
    traj.t1_ = traj.t0_ + (dt * (a_max / (a_min + a_max)));
    traj.x1_ = traj.x0_ + (v_max * (traj.t1_ - traj.t0_));
  }
  else
  {
    traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);
    traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));
  }

  traj.t2_ = t_end;
  traj.a2_ = ((((a_min / a_max) - 1) * v_max) + v1 - ((a_min / a_max) * v0)) / (dt - tc);
  traj.v2_ = v1;
  traj.x2_ = x_end;

  traj.t3_ = traj.t2_;
  traj.a3_ = 0;
  traj.v3_ = traj.v2_;
  traj.x3_ = traj.x2_;

  return traj;
}


TrajectoryParams TrajAlgo::boundary_accel_or_decel_complete_upper(double t, double v0, double v1, double x0, double x_end, double dx)
{
  if (v0 + v1 <= epsilon_ && v0 + v1 >= -epsilon_)
    throw std::invalid_argument("boundary_accel_or_decel_complete_upper: Received v0 + v1 near zero..." + std::to_string(v0 + v1));

  double t_end = t + ((2 * dx) / (v0 + v1));

  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  traj.t1_ = t_end;

  if (dx <= epsilon_ && dx >= -epsilon_)
    throw std::invalid_argument("boundary_accel_or_decel_complete_upper: Received dx near zero..." + std::to_string(dx));

  traj.a1_ = (pow(v1, 2) - pow(v0, 2)) / (2 * dx);
  traj.v1_ = v1;
  traj.x1_ = x_end;

  traj.t2_ = traj.t1_;
  traj.a2_ = 0;
  traj.v2_ = traj.v1_;
  traj.x2_ = traj.x1_;

  traj.t3_ = traj.t1_;
  traj.a3_ = 0;
  traj.v3_ = traj.v1_;
  traj.x3_ = traj.x1_;

  return traj;
}


TrajectoryParams TrajAlgo::boundary_decel_nocruise_notminspeed_accel(double t, double v0, double v1, double v_min, double a_max, double a_min, double x0, double x_end, double dx)
{
  double v_hat = sqrt(((2 * dx * a_max * a_min) + (a_max * pow(v0, 2)) - (a_min * pow(v1, 2))) / (a_max - a_min));
  double t_end = t + ((v_hat * (a_max - a_min)) - (v0 * a_max) + (v1 * a_min)) / (a_max * a_min);

  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  traj.v1_ = v_hat;
  traj.a1_ = a_min;
  traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);
  traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));

  traj.t2_ = t_end;
  traj.a2_ = a_max;
  traj.v2_ = v1;
  traj.x2_ = x_end;

  traj.t3_ = traj.t2_;
  traj.a3_ = 0;
  traj.v3_ = traj.v2_;
  traj.x3_ = traj.x2_;

  return traj;
}


TrajectoryParams TrajAlgo::boundary_decel_nocruise_minspeed_accel_incomplete(double t, double v0, double v_min, double a_max, double a_min, double x0, double x_end, double dx)
{
  double sqr = sqrt((2 * a_max * dx) - ((pow(v_min, 2) - pow(v0, 2)) * (a_max / a_min)) + pow(v_min, 2));

  double t_end = t + ((sqr - v_min) / a_max) + ((v_min - v0) / a_min);

  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  traj.v1_ = v_min;
  traj.a1_ = a_min;
  traj.t1_ = traj.t0_ + (traj.v1_ - traj.v0_) / a_min;
  traj.x1_ = traj.x0_ + (pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * a_min);

  traj.t2_ = t_end;
  traj.a2_ = a_max;
  traj.v2_ = (traj.a2_ * (traj.t2_ - traj.t1_)) + traj.v1_;
  traj.x2_ = x_end;

  traj.t3_ = traj.t2_;
  traj.a3_ = 0;
  traj.v3_ = traj.v2_;
  traj.x3_ = traj.x2_;

  return traj;
}


TrajectoryParams TrajAlgo::boundary_decel_nocruise_minspeed_accel_complete(double t, double v0, double v1, double v_max, double v_min, double a_max, double a_min, double x0, double x_end, double dx)
{
  double nom = (v1 - v_min) + ((a_max / a_min) * (v_min - v0));
  double den = (pow(v1, 2) - pow(v_min, 2)) + ((a_max / a_min) * (pow(v_min, 2) - pow(v0, 2)));

  if (den <= epsilon_ && den >= -epsilon_)
    throw std::invalid_argument("boundary_decel_nocruise_minspeed_accel_complete: Received den near zero..." + std::to_string(den));

  double t_end = t + (2 * dx * nom / den);
  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  double dt = t_end - t;
  double tc = 0;

  traj.v1_ = v_min;

  if (dt - tc <= epsilon_ && dt - tc >= -epsilon_)
    throw std::invalid_argument("boundary_decel_nocruise_minspeed_accel_complete: Received dt - tc near zero..." + std::to_string(dt - tc));

  traj.a1_ = (((1 - (a_min / a_max)) * v_min) + ((a_min / a_max) * v1) - v0) / (dt - tc);
  traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);
  traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));

  traj.t2_ = t_end;
  traj.a2_ = ((((a_max / a_min) - 1) * v_min) + v1 - ((a_max / a_min) * v0)) / (dt - tc);
  traj.v2_ = v1;
  traj.x2_ = x_end;

  traj.t3_ = traj.t2_;
  traj.a3_ = 0;
  traj.v3_ = traj.v2_;
  traj.x3_ = traj.x2_;

  return traj;
}


TrajectoryParams TrajAlgo::boundary_decel_cruise_minspeed_accel(double t, double v0, double v1, double v_min, double a_max, double a_min, double x0, double x_end, double dx)
{
  double t_end = t + (dx / v_min) + ((pow(v_min - v0, 2)) / (2 * a_min * v_min)) - ((pow(v1 - v_min, 2)) / (2 * a_max * v_min));

  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  traj.v1_ = v_min;
  traj.a1_ = a_min;
  traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);
  traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));

  traj.v2_ = v_min;
  traj.a2_ = 0;
  traj.t2_ = t_end - ((v1 - v_min) / a_max);
  traj.x2_ = x_end - ((pow(v1, 2) - pow(v_min, 2)) / (2 * a_max));

  traj.t3_ = t_end;
  traj.a3_ = a_max;
  traj.v3_ = v1;
  traj.x3_ = x_end;

  return traj;
}


TrajectoryParams TrajAlgo::boundary_decel_cruise_minspeed(double t, double v0, double v_min, double a_min, double x0, double x_end, double dx)
{
  double t_end = t + (dx / v_min) + (pow(v_min - v0, 2) / (2 * a_min * v_min));

  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  traj.v1_ = v_min;
  traj.a1_ = a_min;
  traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);
  traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));

  traj.v2_ = v_min;
  traj.a2_ = 0;
  traj.t2_ = t_end;
  traj.x2_ = x_end;

  traj.t3_ = traj.t2_;
  traj.a3_ = traj.a2_;
  traj.v3_ = traj.v2_;
  traj.x3_ = traj.x2_;

  return traj;
}


TrajectoryParams TrajAlgo::boundary_decel_incomplete_lower(double t, double v0, double a_min, double x0, double x_end, double dx)
{
  double t_end = t + (sqrt(pow(v0, 2) + (2 * a_min * dx)) - v0) / a_min;

  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  traj.t1_ = t_end;
  traj.v1_ = sqrt(pow(v0, 2) + (2 * a_min * dx));
  traj.a1_ = a_min;
  traj.x1_ = x_end;

  traj.t2_ = traj.t1_;
  traj.a2_ = 0;
  traj.v2_ = traj.v1_;
  traj.x2_ = traj.x1_;

  traj.t3_ = traj.t1_;
  traj.a3_ = 0;
  traj.v3_ = traj.v1_;
  traj.x3_ = traj.x1_;

  return traj;
}


TrajectoryParams TrajAlgo::boundary_decel_cruise_minspeed_decel(double t, double v0, double v_min, double a_min, double x0, double x_end, double dx)
{
  double t_end = t + (dx / v_min) + (v0 * (v0 - (2 * v_min)) / (2 * a_min * v_min));

  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  traj.v1_ = v_min;
  traj.a1_ = a_min;
  traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);
  traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));

  traj.a2_ = 0;
  traj.v2_ = v_min;
  traj.t2_ = t_end - ((0 - traj.v2_) / a_min);
  traj.x2_ = x_end - ((0 - pow(traj.v2_, 2)) / (2 * a_min));

  traj.t3_ = t_end;
  traj.a3_ = a_min;
  traj.v3_ = 0;
  traj.x3_ = x_end;

  return traj;
}




TrajectoryParams TrajAlgo::get_ts_case(double delta_t, double t, double et, double v0, double v1, double v_max, double v_min, double a_max, double a_min, double x0, double x_end, double dx, BoundaryDistances boundary_distances, std::vector<TrajectoryParams> params)
{
  double dx1 = boundary_distances.dx1;
  double dx2 = boundary_distances.dx2;
  double dx3 = boundary_distances.dx3;
  double dx4 = boundary_distances.dx4;
  double dx5 = boundary_distances.dx5;

  if (params.size() != 8)
  {
    throw std::invalid_argument("Not enough trajectory paras given! Given size: " + std::to_string(params.size()));
  }

  TrajectoryParams traj1 = params[0];
  TrajectoryParams traj2 = params[1];
  TrajectoryParams traj3 = params[2];
  TrajectoryParams traj4 = params[3];
  TrajectoryParams traj5 = params[4];
  TrajectoryParams traj6 = params[5];
  TrajectoryParams traj7 = params[6];
  TrajectoryParams traj8 = params[7];
  TrajectoryParams veh_traj;
  veh_traj.is_algorithm_successful = true;

  if (et < traj1.t3_)
  {
    veh_traj = ts_case1(t, traj1.t3_, v0, v1, v_max, a_max, a_min, x0, x_end, dx);
    veh_traj.case_num = CASE_1;
  }
  else if (traj1.t3_ <= et && et < traj2.t3_)
  {
    // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "CALCULATED: case 1");
    veh_traj = ts_case1(t, et, v0, v1, v_max, a_max, a_min, x0, x_end, dx);
    veh_traj.case_num = CASE_1;
  }
  else if (traj2.t3_ <= et && et < traj3.t3_)
  {
    // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "CALCULATED: case 2");
    veh_traj = ts_case2(t, et, v0, v1, a_max, a_min, x0, x_end, dx);
    veh_traj.case_num = CASE_2;
  }
  else if (traj3.t3_ <= et && et < traj4.t3_)
  {
    // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "CALCULATED: case 3");
    veh_traj = ts_case3(t, et, v0, v1, a_max, a_min, x0, x_end, dx);
    veh_traj.case_num = CASE_3;
  }
  else if (traj4.t3_ <= et && et < traj5.t3_)
  {
    // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "CALCULATED: case 4");
    veh_traj = ts_case4(t, et, v0, v1, v_min, a_max, a_min, x0, x_end, dx);
    veh_traj.case_num = CASE_4;
  }
  else if (traj5.t3_ <= et && et < traj6.t3_)
  {
    // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "CALCULATED: case 5");
    veh_traj = ts_case5(t, et, v0, a_max, a_min, x0, x_end, dx);
    veh_traj.case_num = CASE_5;
  }
  else if (traj6.t3_ <= et && et < traj7.t3_)
  {
    // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "CALCULATED: case 6");
    veh_traj = ts_case6(t, et, v0, v_min, a_min, x0, x_end, dx, dx3, traj6);
    veh_traj.case_num = CASE_6;
  }
  else if (traj7.t3_ <= et && et <= traj8.t3_)
  {
    // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "CALCULATED: case 7");
    veh_traj = ts_case7(t, et, v0, v_min, a_min, x0, x_end, dx);
    veh_traj.case_num = CASE_7;
  }
  else
  {
    // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "CALCULATED: case 8");
    veh_traj = ts_case8(dx, dx5, traj8);
    veh_traj.case_num = CASE_8;
  }

  if (delta_t <= (veh_traj.t1_ - veh_traj.t0_))
  {
    veh_traj.accel_next = veh_traj.a1_;
  }
  else if (delta_t <= (veh_traj.t2_ - veh_traj.t0_))
  {
    double remaining_time = delta_t - (veh_traj.t1_ - veh_traj.t0_);
    double next_speed = veh_traj.v1_ + (remaining_time * veh_traj.a2_);
    double accel_next = (next_speed - veh_traj.v0_) / delta_t;
    veh_traj.accel_next = accel_next;
  }
  else if (delta_t <= (veh_traj.t3_ - veh_traj.t0_))
  {
    double remaining_time = delta_t - (veh_traj.t2_ - veh_traj.t0_);
    double next_speed = veh_traj.v2_ + (remaining_time * veh_traj.a3_);
    double accel_next = (next_speed - veh_traj.v0_) / delta_t;
    veh_traj.accel_next = accel_next;
  }
  else
  {
    veh_traj.accel_next = (veh_traj.v3_ - veh_traj.v0_) / (veh_traj.t3_ - veh_traj.t0_);
  }


  return veh_traj;
}


TrajectoryParams TrajAlgo::ts_case1(double t, double et, double v0, double v1, double v_max, double a_max, double a_min, double x0, double x_end, double dx)
{
  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  double dt = et - t;
  double nom1 = 2 * dx * (((1 - (a_max / a_min)) * v_max) + ((a_max / a_min) * v1) - v0);
  double nom2 = dt * (((1 - (a_max / a_min)) * pow(v_max, 2)) + ((a_max / a_min) * pow(v1, 2)) - pow(v0, 2));
  double den = pow(v_max - v0, 2) - ((a_max / a_min) * pow(v_max - v1, 2));

  if (den <= epsilon_ && den >= -epsilon_)
    throw std::invalid_argument("CASE1: Received den near zero..." + std::to_string(den));

  double tc = (nom1 - nom2) / den;

  traj.v1_ = v_max;

  if (dt - tc <= epsilon_ && dt - tc >= -epsilon_)
    throw std::invalid_argument("CASE1: Received dt - tc near zero..." + std::to_string(dt - tc));

  traj.a1_ = (((1 - (a_max / a_min)) * v_max) + ((a_max / a_min) * v1) - v0) / (dt - tc);

  if (traj.a1_ <= accel_epsilon_ && traj.a1_ >= -accel_epsilon_)
  {
    //RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "CASE1: Received traj.a1_ near zero...");
    traj.t1_ = traj.t0_ + ((dt - tc) * (a_max / (a_min + a_max)));
    traj.x1_ = traj.x0_ + (v_max * (traj.t1_ - traj.t0_));
  }
  else
  {
    traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);
    traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));
  }
  traj.a2_ = 0;
  traj.v2_ = v_max;
  traj.t2_ = traj.t1_ + tc;
  traj.x2_ = traj.x1_ + (v_max * tc);

  traj.t3_ = et;
  traj.a3_ = traj.a1_ * (a_min / a_max);
  traj.v3_ = v1;
  traj.x3_ = x_end;





  return traj;

}


TrajectoryParams TrajAlgo::ts_case2(double t, double et, double v0, double v1, double a_max, double a_min, double x0, double x_end, double dx)
{
  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  double dt = et - t;

  if (dt <= epsilon_ && dt >= -epsilon_)
    throw std::invalid_argument("CASE2: Received dt near zero..." + std::to_string(dt));

  double sqr1 = pow(1 - (a_max / a_min), 2) * pow(dx / dt, 2);
  double sqr2 = (1 - (a_max / a_min)) * (((a_max / a_min) * v1 * (v1 - (2 * dx / dt))) + (v0 * ((2 * dx / dt) - v0)));
  double v_hat = (dx / dt) + (sqrt(sqr1 - sqr2) / (1 - (a_max / a_min)));

  traj.v1_ = v_hat;
  traj.a1_ = (((1 - (a_max / a_min)) * v_hat) + ((a_max / a_min) * v1) - v0) / dt;

  if (traj.a1_ <= accel_epsilon_ && traj.a1_ >= -accel_epsilon_)
  {
    //RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "CASE2: Received traj.a1_ near zero...");
    traj.t1_ = traj.t0_ + (dt * (a_max / (a_min + a_max)));
    traj.x1_ = traj.x0_ + (v_hat * (traj.t1_ - traj.t0_));
  }
  else
  {
    traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);
    traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));
  }

  traj.v2_ = v1;
  traj.a2_ = traj.a1_ * a_min / a_max;

  if (traj.a2_ <= accel_epsilon_ && traj.a2_ >= -accel_epsilon_)
  {
    //RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "CASE2: Received traj.a2_ near zero...");
    traj.t2_ = traj.t1_ + (dt * (a_min / (a_min + a_max)));
    traj.x2_ = traj.x1_ + (v_hat * (traj.t2_ - traj.t1_));
  }
  else
  {
    traj.t2_ = traj.t1_ + ((traj.v2_ - traj.v1_) / traj.a2_);
    traj.x2_ = x_end;
  }

  traj.t3_ = traj.t2_;
  traj.a3_ = 0;
  traj.v3_ = traj.v2_;
  traj.x3_ = traj.x2_;

  return traj;
}


TrajectoryParams TrajAlgo::ts_case3(double t, double et, double v0, double v1, double a_max, double a_min, double x0, double x_end, double dx)
{
  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  double dt = et - t;

  if (dt <= epsilon_ && dt >= -epsilon_)
    throw std::invalid_argument("CASE3: Received dt near zero..." + std::to_string(dt));

  double sqr1 = pow((a_max / a_min) - 1, 2) * pow(dx / dt, 2);
  double sqr2 = ((a_max / a_min) - 1) * ((v1 * (v1 - (2 * dx / dt))) + ((a_max / a_min) * v0 * ((2 * dx / dt) - v0)));
  double v_hat = (dx / dt) + (sqrt(sqr1 - sqr2) / ((a_max / a_min) - 1));

  traj.v1_ = v_hat;
  traj.a1_ = (((1 - (a_min / a_max)) * v_hat) + ((a_min / a_max) * v1) - v0) / dt;

  if (traj.a1_ <= accel_epsilon_ && traj.a1_ >= -accel_epsilon_)
  {
    // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "CASE3: Received traj.a1_ near zero...");
    traj.t1_ = traj.t0_ + (dt * (a_max / (a_min + a_max)));
    traj.x1_ = traj.x0_ + (v_hat * (traj.t1_ - traj.t0_));
  }
  else
  {
    traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);
    traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));
  }

  traj.v2_ = v1;
  traj.a2_ = traj.a1_ * a_max / a_min;
  traj.t2_ = traj.t1_ + ((traj.v2_ - traj.v1_) / traj.a2_);
  traj.x2_ = x_end;

  traj.t3_ = traj.t2_;
  traj.a3_ = 0;
  traj.v3_ = traj.v2_;
  traj.x3_ = traj.x2_;

  return traj;
}


TrajectoryParams TrajAlgo::ts_case4(double t, double et, double v0, double v1, double v_min, double a_max, double a_min, double x0, double x_end, double dx)
{
  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  double dt = et - t;
  double nom1 = 2 * dx * ((((a_max / a_min) - 1) * v_min) + v1 - ((a_max / a_min) * v0));
  double nom2 = dt * ((((a_max / a_min) - 1) * pow(v_min, 2)) + pow(v1, 2) - ((a_max / a_min) * pow(v0, 2)));
  double den = ((a_max / a_min) * pow(v_min - v0, 2)) - pow(v_min - v1, 2);

  if (den <= epsilon_ && den >= -epsilon_)
    throw std::invalid_argument("CASE4: Received den near zero..." + std::to_string(den));

  double tc = (nom1 - nom2) / den;

  traj.v1_ = v_min;

  if (dt - tc <= epsilon_ && dt - tc >= -epsilon_)
    throw std::invalid_argument("CASE4: Received dt - tc near zero..." + std::to_string(dt - tc));

  traj.a1_ = (((1 - (a_min / a_max)) * v_min) + ((a_min / a_max) * v1) - v0) / (dt - tc);

  if (traj.a1_ <= accel_epsilon_ && traj.a1_ >= -accel_epsilon_)
  {
    // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "CASE4: Received traj.a1_ near zero...");
    traj.t1_ = traj.t0_ + ((dt - tc) * (a_min / (a_min + a_max)));
    traj.x1_ = traj.x0_ + (v_min * (traj.t1_ - traj.t0_));
  }
  else
  {
    traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);
    traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));
  }

  traj.v2_ = v_min;
  traj.a2_ = 0;
  traj.t2_ = traj.t1_ + tc;
  traj.x2_ = traj.x1_ + (v_min * tc);

  traj.t3_ = et;
  traj.a3_ = traj.a1_ * a_max / a_min;
  traj.v3_ = v1;
  traj.x3_ = x_end;

  return traj;
}


TrajectoryParams TrajAlgo::ts_case5(double t, double et, double v0, double a_max, double a_min, double x0, double x_end, double dx)
{
  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  double dt = et - t;
  double sqr = ((a_max / a_min) - 1) * ((2 * a_min * (dx - (v0 * dt))) - pow(a_min * dt, 2));
  double v_hat = (v0 + (a_min * dt)) - (sqrt(sqr) / ((a_max / a_min) - 1));
  double v_p = ((1 - (a_max / a_min)) * v_hat) + ((a_max / a_min) * v0) + (a_max * dt);

  traj.v1_ = v_hat;
  traj.a1_ = a_min;

  traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);
  traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));

  traj.v2_ = v_p;
  traj.a2_ = a_max;
  traj.t2_ = traj.t1_ + ((traj.v2_ - traj.v1_) / traj.a2_);
  traj.x2_ = x_end;

  traj.t3_ = traj.t2_;
  traj.a3_ = 0;
  traj.v3_ = traj.v2_;
  traj.x3_ = traj.x2_;

  return traj;
}


TrajectoryParams TrajAlgo::ts_case6(double t, double et, double v0, double v_min, double a_min, double x0, double x_end, double dx, double dx3, TrajectoryParams traj6)
{
  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  double dt = et - t;

  traj.v1_ = v_min;
  traj.a1_ = a_min;
  traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);
  traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));

  double tc;

  if (dx <= dx3)
    tc = 0;
  else
    tc = traj6.t2_ - traj6.t1_;

  traj.v2_ = v_min;
  traj.a2_ = 0;
  traj.t2_ = traj.t1_ + tc;
  traj.x2_ = traj.x1_ + (v_min * tc);

  double dt_p = dt - (traj.t1_ - traj.t0_) - tc;

  if (dt_p <= epsilon_ && dt_p >= -epsilon_)
    throw std::invalid_argument("CASE6: Received dt_p near zero..." + std::to_string(dt_p));

  double v_p = ((2 * a_min * (dx - (v_min * tc))) - (pow(v_min, 2) - pow(v0, 2)) - (v_min * dt_p * a_min)) / (dt_p * a_min);

  traj.v3_ = v_p;
  traj.a3_ = (v_p - v_min) / dt_p;
  traj.t3_ = et;
  traj.x3_ = x_end;

  return traj;
}


TrajectoryParams TrajAlgo::ts_case7(double t, double et, double v0, double v_min, double a_min, double x0, double x_end, double dx)
{
  TrajectoryParams traj;

  traj.t0_ = t;
  traj.v0_ = v0;
  traj.x0_ = x0;

  traj.v1_ = v_min;
  traj.a1_ = a_min;
  traj.t1_ = traj.t0_ + ((traj.v1_ - traj.v0_) / traj.a1_);
  traj.x1_ = traj.x0_ + ((pow(traj.v1_, 2) - pow(traj.v0_, 2)) / (2 * traj.a1_));

  double dt = et - t;
  double v_p = v_min - sqrt(pow(v_min - v0, 2) - (2 * a_min * ((v_min * dt) - dx)));
  double dt_p = (v_p - v_min) / a_min;

  if (dt_p <= epsilon_ && dt_p >= -epsilon_)
    throw std::invalid_argument("CASE7: Received dt_p near zero..." + std::to_string(dt_p));

  double tc = dt - ((v_p - v0) / a_min);

  traj.v2_ = v_min;
  traj.a2_ = 0;
  traj.t2_ = traj.t1_ + tc;
  traj.x2_ = traj.x1_ + (v_min * tc);

  traj.v3_ = v_p;
  traj.a3_ = (v_p - v_min) / dt_p;
  traj.t3_ = et;
  traj.x3_ = x_end;

  return traj;
}


TrajectoryParams TrajAlgo::ts_case8(double dx, double dx5, TrajectoryParams traj8)
{
  TrajectoryParams traj = traj8;
  if (dx < dx5)
  {
    traj.is_algorithm_successful = false;
    // RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "CASE8: Not within safe stopping distance originally planned!");
  }
  return traj;
}






/*


double TrajAlgo::get_trajectory_smoothing_activation_distance(double time_remaining_at_free_flow, double full_cycle_duration, double current_speed, double speed_limit, double departure_speed, double max_accel, double max_decel) const
{
  // TSMO USE CASE 2: Figure 7. Trajectory smoothing solution Case 2. Subsituted a+ as max_accel and solved for inflection_speed
  double accel_ratio = max_accel / max_decel;
  double remaining_time = time_remaining_at_free_flow - full_cycle_duration / 2;
  double inflection_speed = (max_accel * remaining_time - accel_ratio * departure_speed + current_speed)/ (1 - accel_ratio);
  RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "ENTER TRAJ CALC: time_remaining_at_free_flow: " << time_remaining_at_free_flow << ", full_cycle_duration: " << full_cycle_duration << ", inflection_speed: " << inflection_speed);

  if (remaining_time < 0)
    return -1;

  if (inflection_speed > 0 && inflection_speed <= speed_limit + epsilon_ && inflection_speed >= departure_speed - epsilon_)
  {
    // kinematic equation to find distance of acceleration + deceleration
    // (vf^2 - vi^2)/2a = d
    double d = (std::pow(inflection_speed, 2) - std::pow (current_speed, 2)) / (2 * max_accel) +  (std::pow(departure_speed, 2) - std::pow(inflection_speed, 2)) / (2 * max_decel);
    RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "calculated distance WITHOUT cruising: " << d);
    return d;
  }
  else //there must be cruising
  {
    // acceleration and deceleration parts must reach maximum speed
    // kinematic equation: t = (vf - vi)/ a where vf = 0
    double decel_time = (current_speed - speed_limit) / max_decel;
    double accel_time = (speed_limit - current_speed) / max_accel;
    double cruising_time = remaining_time - decel_time - accel_time;
    RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "decel_time: " << decel_time << ", accel_time: " << accel_time << ", cruising_time: " << cruising_time);
    double d = (std::pow(speed_limit, 2) - std::pow (current_speed, 2)) / (2 * max_accel) +  (std::pow(departure_speed, 2) - std::pow(speed_limit, 2)) / (2 * max_decel) + cruising_time * speed_limit;
    RCLCPP_DEBUG_STREAM(rclcpp::get_logger("lci_strategic_plugin"), "calculated distance with cruising: " <<  d << ", accel_seg: " << (std::pow(speed_limit, 2) - std::pow (current_speed, 2)) / (2 * max_accel) <<
                      ", cruising: " << + cruising_time * speed_limit << ", decel_seg:" << (std::pow(departure_speed, 2) - std::pow(speed_limit, 2)) / (2 * max_decel));
    return d;
  }
}




*/

}
