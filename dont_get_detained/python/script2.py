import json
import os
import math
from datetime import datetime

DATA_FILE = "attendance_data.json"

def clear_screen():
    os.system('cls' if os.name == 'nt' else 'clear')

class Tracker:
    def __init__(self):
        self.data = self.load_data()

    def load_data(self):
        if os.path.exists(DATA_FILE):
            with open(DATA_FILE, 'r') as f:
                return json.load(f)
        return None

    def save_data(self):
        with open(DATA_FILE, 'w') as f:
            json.dump(self.data, f, indent=4)

    def setup_wizard(self):
        print("--- Initial Semester Setup ---")
        total_weeks = int(input("How many weeks in this semester? "))
        sub_count = int(input("How many subjects do you have? "))
        
        subjects = {}
        for _ in range(sub_count):
            name = input("\nEnter Subject Name: ")
            print(f"For {name}, which components exist? (y/n)")
            
            # Sub-dictionary for components
            components = {}
            for comp in ['Lecture', 'Tutorial', 'Practical']:
                if input(f"  Does it have {comp}? ").lower() == 'y':
                    per_week = int(input(f"    How many {comp}s per week? "))
                    components[comp] = {
                        "per_week": per_week,
                        "total_expected": per_week * total_weeks,
                        "absent_count": 0
                    }
            subjects[name] = components

        self.data = {"subjects": subjects, "total_weeks": total_weeks}
        self.save_data()
        print("\nSetup Complete! Data saved to attendance_data.json")

    def show_status(self):
        clear_screen()
        print(f"{'Subject':<20} | {'Type':<10} | {'Missed':<8} | {'Buffer Left'}")
        print("-" * 60)
        
        for sub_name, components in self.data['subjects'].items():
            for comp_name, stats in components.items():
                # 25% Rule: Floor(Total * 0.25) - Current Absents
                max_allowed = math.floor(stats['total_expected'] * 0.25)
                buffer = max_allowed - stats['absent_count']
                
                # Color coding logic (Optional but helpful)
                status = "OK" if buffer > 0 else "DANGER"
                
                print(f"{sub_name[:18]:<20} | {comp_name:<10} | {stats['absent_count']:<8} | {buffer} ({status})")
        
        input("\nPress Enter to return to menu...")

    def mark_absent(self):
        clear_screen()
        subs = list(self.data['subjects'].keys())
        print("Select Subject to mark ABSENT:")
        for i, name in enumerate(subs):
            print(f"{i+1}. {name}")
        
        choice = int(input("> ")) - 1
        sub_name = subs[choice]
        
        comps = list(self.data['subjects'][sub_name].keys())
        print(f"\nWhich component of {sub_name} did you miss?")
        for i, c in enumerate(comps):
            print(f"{i+1}. {c}")
        
        c_choice = int(input("> ")) - 1
        comp_name = comps[c_choice]
        
        self.data['subjects'][sub_name][comp_name]['absent_count'] += 1
        self.save_data()
        print(f"\nUpdated! You have now missed {self.data['subjects'][sub_name][comp_name]['absent_count']} classes.")
        input("Press Enter...")

def main():
    tracker = Tracker()
    
    # If no data exists, run setup
    if not tracker.data:
        tracker.setup_wizard()
        tracker = Tracker() # Reload with new data

    while True:
        clear_screen()
        print("=== ATTENDANCE MANAGER ===")
        print("1. View Attendance & Buffer")
        print("2. Mark an Absence")
        print("3. Reset Semester (Caution!)")
        print("4. Exit")
        
        choice = input("\nChoose option: ")
        
        if choice == '1':
            tracker.show_status()
        elif choice == '2':
            tracker.mark_absent()
        elif choice == '3':
            if input("Delete all data? (y/n): ") == 'y':
                os.remove(DATA_FILE)
                break
        elif choice == '4':
            break

if __name__ == "__main__":
    main()
