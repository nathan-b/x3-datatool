# Composite Test Archives

This directory contains test archives for testing the datadir compositing functionality.

## Archive Structure

### 1.cat / 1.dat (6 files)
Base archive with initial versions of all files:
- `models/ship.mdl` - "Model v1"
- `models/station.mdl` - "Model v1"
- `scripts/init.lua` - "Script v1"
- `scripts/main.lua` - "Script v1"
- `textures/cockpit.tex` - "Texture v1"
- `textures/hull.tex` - "Texture v1"

### 2.cat / 2.dat (6 files)
Overrides 4 files from archive 1, adds 2 new files:

**Overrides:**
- `models/ship.mdl` - "Model v2 UPDATED"
- `models/station.mdl` - "Model v2 UPDATED"
- `scripts/init.lua` - "Script v2 UPDATED"
- `textures/hull.tex` - "Texture v2 UPDATED"

**New files:**
- `sounds/engine.wav` - "Sound v2 NEW"
- `sounds/weapons.wav` - "Sound v2 NEW"

### 10.cat / 10.dat (3 files)
All 3 files override files from earlier archives:

**Overrides:**
- `models/ship.mdl` - "Model v10 FINAL"
- `textures/hull.tex` - "Texture v10 FINAL"
- `sounds/engine.wav` - "Sound v10 FINAL"

## Expected Composite Result

When all three archives are loaded (1, 2, 10), the final filesystem should contain:

| File | Source Archive | Content |
|------|----------------|---------|
| `models/ship.mdl` | 10 | "Model v10 FINAL" |
| `models/station.mdl` | 2 | "Model v2 UPDATED" |
| `scripts/init.lua` | 2 | "Script v2 UPDATED" |
| `scripts/main.lua` | 1 | "Script v1" |
| `textures/cockpit.tex` | 1 | "Texture v1" |
| `textures/hull.tex` | 10 | "Texture v10 FINAL" |
| `sounds/engine.wav` | 10 | "Sound v10 FINAL" |
| `sounds/weapons.wav` | 2 | "Sound v2 NEW" |

**Total: 8 unique files**

## Source Directories

The `src1/`, `src2/`, and `src10/` directories contain the original uncompressed files used to build the archives. These are kept for reference and rebuilding if needed.
