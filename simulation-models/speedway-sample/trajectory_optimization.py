import win32com.client
import time
import os

# Connect to VISSIM via COM interface
Vissim = win32com.client.Dispatch("Vissim.Vissim")

# Load VISSIM model
filename = r"C:\Users\novatbarn\OneDrive - Leidos-LeidosCorpUS\Desktop\QW\MMITSS_VISSIM_FILES - TSP\ISIG_Actuated_Simulation\va_isig_v1.7_actuated_v2.inpx"
layout = r"C:\Users\novatbarn\OneDrive - Leidos-LeidosCorpUS\Desktop\QW\MMITSS_VISSIM_FILES - TSP\ISIG_Actuated_Simulation\va_isig_v1.7_actuated_v2.layx"
Vissim.LoadNet(filename)
Vissim.LoadLayout(layout)

# Check if layout exists before loading it
if os.path.exists(layout):
    try:
        Vissim.LoadNet(filename)
        Vissim.LoadLayout(layout)
    except Exception as e:
        print(f"Error loading layout: {e}")
        print("Proceeding without the layout file.")
else:
    print(f"Layout file not found: {layout}")
    Vissim.LoadNet(filename)


# Define simulation parameters
simulation_time = 3600  # Total simulation time in seconds (1 hour)
simulation_step = 1  # Time step in seconds (1-second intervals)

# Function to get traffic signal status and time until next green and red
def get_signal_timing(signal_controller_id, signal_group_id):
    signal_controller = Vissim.Net.SignalControllers.ItemByKey(signal_controller_id)
    signal_group = signal_controller.SGs.ItemByKey(signal_group_id)
    
    # Get current cycle time and signal cycle second (current position in cycle)
    cycle_time = signal_controller.AttValue("CycTm")  # Total cycle time in seconds
    cycle_second = signal_group.AttValue("CycSec")  # Current time in the cycle
    
    # Time until the next green and red
    green_start = signal_group.AttValue("GreenStart")
    green_end = signal_group.AttValue("GreenEnd")
    
    if green_start >= cycle_second:
        time_until_next_green = green_start - cycle_second
    else:
        time_until_next_green = cycle_time - cycle_second + green_start
    
    if green_end >= cycle_second:
        time_until_next_red = green_end - cycle_second
    else:
        time_until_next_red = cycle_time - cycle_second + green_end
    
    return time_until_next_green, time_until_next_red

# Function to calculate the distance from the vehicle to the next signal
def calculate_distance_to_signal(vehicle, signal_head):
    vehicle_pos = vehicle.AttValue("Pos")  # Current vehicle position on the link
    signal_pos = signal_head.AttValue("Pos")  # Signal position on the link
    distance_to_signal = max(0, signal_pos - vehicle_pos)  # Distance to the signal
    return distance_to_signal

# Function to calculate the optimal speed for arriving at the green signal
def calculate_optimal_speed(vehicle, signal_controller_id, signal_group_id, signal_head):
    time_until_next_green, time_until_next_red = get_signal_timing(signal_controller_id, signal_group_id)
    
    # Calculate the distance from the vehicle to the signal
    distance_to_signal = calculate_distance_to_signal(vehicle, signal_head)
    
    # Compute the optimal speed to reach the next green
    if time_until_next_green > 0 and distance_to_signal > 0:
        optimal_speed = (distance_to_signal / time_until_next_green) * 3.6  # Convert m/s to km/h
        return min(optimal_speed, vehicle.AttValue("OrgDesSpeed"))  # Cap the speed at the vehicle's original desired speed
    
    return 10  # Default speed if calculation is not feasible (e.g., red light)

# Function to adjust speed for CVs and CAVs based on signal status
def adjust_vehicle_speed(vehicle, signal_controller_id, signal_group_id, signal_head):
    time_until_next_green, time_until_next_red = get_signal_timing(signal_controller_id, signal_group_id)
    
    if time_until_next_green > 0:  # If the signal will turn green soon
        optimal_speed = calculate_optimal_speed(vehicle, signal_controller_id, signal_group_id, signal_head)
        vehicle.SetAttValue("DesSpeed", optimal_speed)  # Set the vehicle speed to the computed optimal value

# List of signal IDs and their corresponding signal heads (adjust accordingly)
signal_ids = [8, 9, 10, 11, 12, 13]

# Vehicle type for Connected Vehicles (CVs) - Replace with your actual vehicle type ID
cv_vehicle_type_id = 101  # Example: 101 is the CV type ID

# Main simulation loop
for i in range(simulation_time):
    # Run one step of the simulation (1-second interval)
    Vissim.Simulation.RunSingleStep()
    
    # Loop through all vehicles in the simulation
    all_vehicles = Vissim.Net.Vehicles.GetAll()
    for vehicle in all_vehicles:
        if vehicle.AttValue("VehType") == cv_vehicle_type_id:  # Filter CV vehicles only
            for signal_controller_id in signal_ids:
                signal_group_id = 1  # Example signal group ID, adjust if needed
                signal_head = Vissim.Net.SignalHeads.ItemByKey(signal_controller_id)  # Reference to the signal head
                
                # Adjust speed based on signal timing and the vehicle's position
                adjust_vehicle_speed(vehicle, signal_controller_id, signal_group_id, signal_head)
    
    # Optional: Add a delay to see the simulation progress in real-time
    time.sleep(simulation_step)


# Stop the simulation after completing the loop
Vissim.Simulation.Stop()
