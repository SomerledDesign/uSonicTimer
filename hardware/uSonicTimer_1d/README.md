# uSonicTimer controller board, rev 1d

KiCad 10 project for the uSonicTimer controller PCB (schematic rev D). The firmware
pin map in the top-level [`README`](../../README.md) matches this board.

| Path | Contents |
|---|---|
| `uSonicTimer1d.kicad_pro` | KiCad project (open this) |
| `uSonicTimer1d.kicad_sch` | Schematic |
| `uSonicTimer1d.kicad_pcb` | PCB layout |
| `uSonicTimer1d.kicad_dru` | Custom design rules |
| `uSonicTimer1d.kicad_sym`, `sym-lib-table` | Project symbol library |
| `uSonicTimer1d.pretty/`, `fp-lib-table` | Project footprint library |
| `Gerber/` | Fabrication outputs: Gerbers, drill files, job file and `uSonicTimer1d_revD_gerbers.zip` for ordering |
| `bom/ibom.html` | Interactive BOM (open in a browser) |
| `Renders/` | Top and bottom board renders |
| `[TODO]/` | Changes planned for the next board revision |

The library tables use `${KIPRJMOD}`, so the project opens from any location. A few
3D models (STEP) come from a local library outside this repository; KiCad will show
those parts without a 3D body, which does not affect the schematic, PCB or fab outputs.

Licensed under the CERN Open Hardware Licence Version 2, Strongly Reciprocal. See
[`LICENSE-HARDWARE`](../../LICENSE-HARDWARE).
SPDX-License-Identifier: CERN-OHL-S-2.0
