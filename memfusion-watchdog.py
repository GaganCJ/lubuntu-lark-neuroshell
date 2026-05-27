#!/usr/bin/env python3
import os
import subprocess
import time

def setup_zram_loop():
    try:
        with open('/proc/swaps', 'r') as f:
            if '/dev/zram' in f.read():
                print("[MemFusion Watchdog] zRam module already tracking.")
                return
    except Exception:
        pass

    print("[MemFusion Watchdog] Instantiating zRam pipeline (Algorithm: zstd)...")
    subprocess.run(["modprobe", "zram"], check=True)
    
    with open('/sys/block/zram0/comp_algorithm', 'w') as f:
        f.write('zstd')
        
    # Dynamically scale zRam buffer limit (using 2G default baseline)
    with open('/sys/block/zram0/disksize', 'w') as f:
        f.write('2G')
        
    subprocess.run(["mkswap", "/dev/zram0"], check=True)
    subprocess.run(["swapon", "-p", "100", "/dev/zram0"], check=True)
    
    print("[MemFusion Watchdog] Dynamically elevating tracking properties (sysctl vm.swappiness=150)...")
    subprocess.run(["sysctl", "vm.swappiness=150"], check=True)

if __name__ == '__main__':
    if os.geteuid() != 0:
        print("Memfusion watchdog operations halted: Root tracking permissions required.")
        exit(1)
        
    setup_zram_loop()
    
    try:
        while True:
            time.sleep(30)
    except KeyboardInterrupt:
        print("MemFusion Watchdog successfully terminated.")