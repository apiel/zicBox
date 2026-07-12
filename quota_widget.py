#!/usr/bin/env python3
import tkinter as tk
import subprocess
import re
import threading

class QuotaWidget:
    def __init__(self, root):
        self.root = root
        self.root.title("Antigravity Quota")
        self.root.attributes("-topmost", True)
        
        # Geometry: wide and short
        self.root.geometry("450x35")
        
        # Frame
        self.frame = tk.Frame(root, bg="#1e1e2e")
        self.frame.pack(fill=tk.BOTH, expand=True)
        
        # Quota Labels
        self.label_weekly = tk.Label(self.frame, text="Weekly: Fetching...", fg="#cdd6f4", bg="#1e1e2e", font=("Arial", 10))
        self.label_weekly.pack(side=tk.LEFT, padx=10)
        
        self.separator = tk.Label(self.frame, text="|", fg="#585b70", bg="#1e1e2e", font=("Arial", 10))
        self.separator.pack(side=tk.LEFT)
        
        self.label_five_hour = tk.Label(self.frame, text="5-Hour: Fetching...", fg="#cdd6f4", bg="#1e1e2e", font=("Arial", 10))
        self.label_five_hour.pack(side=tk.LEFT, padx=10)
        
        # Refresh Button
        self.btn_refresh = tk.Button(self.frame, text="↻", command=self.start_update, fg="#a6e3a1", bg="#313244", activebackground="#45475a", activeforeground="#a6e3a1", font=("Arial", 10, "bold"), bd=0, highlightthickness=0, padx=5)
        self.btn_refresh.pack(side=tk.RIGHT, padx=10)
        
        # Start initial update
        self.start_update()
        
        # Auto refresh every 5 minutes (300,000 ms)
        self.schedule_auto_refresh()

    def schedule_auto_refresh(self):
        self.root.after(300000, self.schedule_auto_refresh)
        self.start_update()

    def start_update(self):
        self.btn_refresh.config(state=tk.DISABLED, text="...")
        self.label_weekly.config(text="Weekly: Updating...")
        self.label_five_hour.config(text="5-Hour: Updating...")
        # Run in thread to prevent UI freezing
        threading.Thread(target=self.update_quota, daemon=True).start()

    def update_quota(self):
        weekly_text = "Weekly: Error"
        five_hour_text = "5-Hour: Error"
        
        try:
            # Run echo "/quota" | agy
            result = subprocess.run(
                "echo '/quota' | agy",
                shell=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                timeout=15
            )
            output = result.stdout
            
            # Regex match
            weekly_match = re.search(r"Weekly Gemini Quota:\s*(.*)", output)
            five_hour_match = re.search(r"5-Hour Gemini Quota:\s*(.*)", output)
            
            if weekly_match:
                weekly_text = f"Weekly: {weekly_match.group(1).strip()}"
            if five_hour_match:
                five_hour_text = f"5-Hour: {five_hour_match.group(1).strip()}"
                
        except Exception as e:
            weekly_text = f"Weekly: Error ({str(e)})"
            
        # Update UI in main thread
        self.root.after(0, self.apply_update, weekly_text, five_hour_text)

    def apply_update(self, weekly, five_hour):
        self.label_weekly.config(text=weekly)
        self.label_five_hour.config(text=five_hour)
        self.btn_refresh.config(state=tk.NORMAL, text="↻")

if __name__ == "__main__":
    root = tk.Tk()
    app = QuotaWidget(root)
    root.mainloop()
