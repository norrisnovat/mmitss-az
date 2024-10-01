import os
import json
import re

# Function to find and replace IP addresses in a JSON file
def update_ips_in_json(file_path, new_subnet):
    with open(file_path, 'r') as file:
        content = json.load(file)

    # Function to recursively find IP addresses and their keys
    def find_ips(obj, parent_key=''):
        ips = []
        if isinstance(obj, dict):
            for key, value in obj.items():
                full_key = f"{parent_key}.{key}" if parent_key else key
                ips.extend(find_ips(value, full_key))
        elif isinstance(obj, list):
            for index, item in enumerate(obj):
                full_key = f"{parent_key}[{index}]" if parent_key else f"[{index}]"
                ips.extend(find_ips(item, full_key))
        elif isinstance(obj, str):
            if re.match(r'(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})', obj):
                ips.append((parent_key, obj))
        return ips

    # Get a list of IP addresses and their corresponding keys
    ips = find_ips(content)

    if not ips:
        print(f"No IP addresses found in {file_path}. Skipping this file.")
        return

    print(f"\nIP addresses found in {file_path}:")
    for i, (key, ip) in enumerate(ips):
        print(f"{i+1}. {ip} (Key: {key})")

    # Prompt the user to select which IP addresses to change
    indices = input("\nEnter the numbers of the IP addresses you want to change (e.g., 1, 2, 3): ").split(',')
    indices = [int(index.strip()) - 1 for index in indices]

    # Update only the selected IP addresses to match the new subnet
    for index in indices:
        key, old_ip = ips[index]
        new_ip = re.sub(r'(\d{1,3}\.\d{1,3}\.\d{1,3}\.)\d{1,3}', new_subnet + old_ip.split('.')[-1], old_ip)
        
        print(f"Changing {old_ip} to {new_ip}...")
        
        # Navigate through the JSON structure to the correct element
        keys = re.split(r'\.|\[|\]', key)
        keys = [k for k in keys if k]  # Remove any empty strings from the list
        ref = content
        for k in keys[:-1]:
            if k.isdigit():
                ref = ref[int(k)]
            else:
                ref = ref[k]
        ref[keys[-1]] = new_ip

    # Write the updated content back to the file
    with open(file_path, 'w') as file:
        json.dump(content, file, indent=4)
    print(f"Updated IP addresses in {file_path}.")

# Function to find and replace IP addresses in supervisord.conf
def update_ips_in_supervisord(file_path, new_subnet):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    ips = []
    for i, line in enumerate(lines):
        match = re.search(r'(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})', line)
        if match:
            ips.append((i, match.group(0)))

    if not ips:
        print(f"No IP addresses found in {file_path}. Skipping this file.")
        return

    print(f"\nIP addresses found in {file_path}:")
    for i, (line_num, ip) in enumerate(ips):
        print(f"{i+1}. {ip} (Line: {line_num + 1})")

    # Prompt the user to select which IP addresses to change
    indices = input("\nEnter the numbers of the IP addresses you want to change (e.g., 1, 2, 3): ").split(',')
    indices = [int(index.strip()) - 1 for index in indices]

    # Update only the selected IP addresses to match the new subnet
    for index in indices:
        line_num, old_ip = ips[index]
        new_ip = re.sub(r'(\d{1,3}\.\d{1,3}\.\d{1,3}\.)\d{1,3}', new_subnet + old_ip.split('.')[-1], old_ip)
        print(f"Changing {old_ip} to {new_ip}...")
        lines[line_num] = lines[line_num].replace(old_ip, new_ip)

    # Write the updated content back to the file
    with open(file_path, 'w') as file:
        file.writelines(lines)
    print(f"Updated IP addresses in {file_path}.")

def main():
    # Get the directory of the current script
    directory = os.path.dirname(os.path.abspath(__file__))
    
    # Loop through all files in the directory
    for filename in os.listdir(directory):
        if filename.endswith(".json"):
            file_path = os.path.join(directory, filename)
            new_subnet = input(f"Enter the new subnet for {filename} (e.g., 192.168.12.): ")
            update_ips_in_json(file_path, new_subnet)
        elif filename.endswith(".conf"):
            file_path = os.path.join(directory, filename)
            new_subnet = input(f"Enter the new subnet for {filename} (e.g., 192.168.12.): ")
            update_ips_in_supervisord(file_path, new_subnet)

if __name__ == "__main__":
    main()
