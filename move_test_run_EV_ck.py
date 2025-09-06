import os
import shutil
import subprocess

def main():
    # Paths
    src_file = r"C:\Users\EBAfa\OneDrive\Desktop\GBA\RadicalRed_BattleTower\Complete-Fire-Red-Upgrade\test.gba"
    dest_dir = r"C:\Users\EBAfa\OneDrive\Desktop\GBA\RadicalRed\UpdatedEV-IV-Display\Custom-EV-IV-Display-Screen-NEW"
    dest_file = os.path.join(dest_dir, "CFRU.gba")
    old_test_file = os.path.join(dest_dir, "test.gba")

    # Ensure destination directory exists
    os.makedirs(dest_dir, exist_ok=True)

    # 0. Remove existing files if they exist
    for f in (dest_file, old_test_file):
        if os.path.exists(f):
            print(f"Removing old file: {f}")
            try:
                os.remove(f)
            except PermissionError:
                print(f"ERROR: Can't remove {f} (in use?). Close any app using it and try again.")
                return

    # 1. Copy file and rename it
    if not os.path.isfile(src_file):
        print(f"ERROR: Source file not found:\n  {src_file}")
        return

    print(f"Copying {src_file} to {dest_file}...")
    try:
        shutil.copy2(src_file, dest_file)
    except PermissionError:
        print("ERROR: Copy failed (file may be locked by OneDrive or another app).")
        return

    # 2. Change working directory to destination folder
    os.chdir(dest_dir)

    # 3. Run `make`
    print("Running make...")
    try:
        subprocess.run(["make"], check=True)
    except FileNotFoundError:
        print("ERROR: `make` not found. Make sure it’s installed and available in your PATH.")
        return

    print("Done!")

if __name__ == "__main__":
    main()