import os
import re
import shutil
import subprocess

SRC_DIR = r"D:\inès\Bureau\PokemonCorner\Radicalred\rr-cfru"
DEST_DIR = r"D:\inès\Bureau\PokemonCorner\Radicalred\Custom-EV-IV-Display-Screen"

SRC_GBA = os.path.join(SRC_DIR, "test.gba")
DEST_GBA = os.path.join(DEST_DIR, "CFRU.gba")
DEST_TEST_GBA = os.path.join(DEST_DIR, "test.gba")

OFFSETS_INI = os.path.join(SRC_DIR, "offsets.ini")
CONFIG_MK = os.path.join(DEST_DIR, "config.mk")
CFRU_LD = os.path.join(DEST_DIR, "CFRU.ld")

def parse_offsets(path):
    """
    Parses offsets.ini for the two keys:
      - CompressedMonToMon
      - CalculateMonStatsNew
    Accepts 'key: HEX' or 'key = HEX', case-insensitive, HEX w/o 0x.
    """
    if not os.path.isfile(path):
        raise FileNotFoundError(f"offsets.ini not found at: {path}")

    with open(path, "r", encoding="utf-8", errors="ignore") as f:
        text = f.read()

    def grab(key):
        m = re.search(rf"^{re.escape(key)}\s*[:=]\s*([0-9A-Fa-f]+)\s*$", text, re.MULTILINE)
        if not m:
            raise ValueError(f"Key '{key}' not found in offsets.ini")
        return int(m.group(1), 16)

    cmtm = grab("CompressedMonToMon")
    calc = grab("CalculateMonStatsNew")
    return cmtm, calc

def set_or_append_config_mk(path, cmtm_plus1):
    """
    Ensures/updates:
      COMPRESSED_MON_TO_MON_PTR ?= 0x{cmtm_plus1:08X}
    If the var exists (with '=' or '?='), replace its line; otherwise append.
    """
    new_line = f"COMPRESSED_MON_TO_MON_PTR ?= 0x{cmtm_plus1:08X}\n"
    lines = []
    found = False

    if os.path.exists(path):
        with open(path, "r", encoding="utf-8", errors="ignore") as f:
            lines = f.readlines()

        for i, line in enumerate(lines):
            if re.match(r"^\s*COMPRESSED_MON_TO_MON_PTR\s*\??=", line):
                lines[i] = new_line
                found = True
                break

    if not found:
        lines.append(new_line)

    with open(path, "w", encoding="utf-8", errors="ignore") as f:
        f.writelines(lines)

def set_cfru_ld_calc_symbol(path, calc_value):
    """
    Sets the line to:
      CalculateMonStatsNew = 0x{calc_value:08X} | 1;
    Replace if present; otherwise append at end.
    """
    new_line = f"CalculateMonStatsNew = 0x{calc_value:08X} | 1;\n"
    if os.path.exists(path):
        with open(path, "r", encoding="utf-8", errors="ignore") as f:
            content = f.read()

        # Replace any existing assignment form (with optional '| 1')
        new_content, n = re.subn(
            r"^CalculateMonStatsNew\s*=\s*0x[0-9A-Fa-f]+\s*(?:\|\s*1)?\s*;\s*$",
            new_line.strip(),
            content,
            flags=re.MULTILINE
        )
        if n == 0:
            # Not found—append with a trailing newline if needed
            if not content.endswith("\n"):
                content += "\n"
            new_content = content + new_line
    else:
        new_content = new_line

    with open(path, "w", encoding="utf-8", errors="ignore") as f:
        f.write(new_content)

def main():
    # Ensure destination exists
    os.makedirs(DEST_DIR, exist_ok=True)

    # Clean conflicting files in destination
    for f in (DEST_GBA, DEST_TEST_GBA):
        if os.path.exists(f):
            try:
                print(f"Removing old file: {f}")
                os.remove(f)
            except PermissionError:
                print(f"ERROR: Can't remove {f} (in use?). Close apps using it and retry.")
                return

    # Copy (do not move) test.gba -> CFRU.gba
    if not os.path.isfile(SRC_GBA):
        print(f"ERROR: Source ROM not found:\n  {SRC_GBA}")
        return

    print(f"Copying {SRC_GBA} -> {DEST_GBA}")
    try:
        shutil.copy2(SRC_GBA, DEST_GBA)
    except PermissionError:
        print("ERROR: Copy failed (file may be locked by OneDrive or another app).")
        return

    # Parse offsets.ini and apply edits to config.mk and CFRU.ld
    try:
        compressed_mon_to_mon, calculate_mon_stats_new = parse_offsets(OFFSETS_INI)
        # +1 (thumb pointer) for COMPRESSED_MON_TO_MON_PTR
        cmtm_plus1 = (compressed_mon_to_mon + 1) & 0xFFFFFFFF
        print(f"CompressedMonToMon: 0x{compressed_mon_to_mon:08X} -> COMPRESSED_MON_TO_MON_PTR: 0x{cmtm_plus1:08X}")
        print(f"CalculateMonStatsNew: 0x{calculate_mon_stats_new:08X} -> CFRU.ld symbol with '| 1'")
        set_or_append_config_mk(CONFIG_MK, cmtm_plus1)
        set_cfru_ld_calc_symbol(CFRU_LD, calculate_mon_stats_new)
    except (FileNotFoundError, ValueError) as e:
        print(f"ERROR processing offsets.ini: {e}")
        return

    # Run `make` in DEST_DIR
    print("Running make...")
    try:
        subprocess.run(["make"], cwd=DEST_DIR, check=True)
    except FileNotFoundError:
        print("ERROR: `make` not found. Install GNU Make and ensure it’s in your PATH.")
        return
    except subprocess.CalledProcessError as e:
        print(f"`make` failed with exit code {e.returncode}.")
        return

    print("Done!")

if __name__ == "__main__":
    main()