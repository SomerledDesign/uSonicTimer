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
| `3dmodels/` | Project 3D models folder (currently only a README: sources, licences and excluded models) |
| `Gerber/` | Fabrication outputs: Gerbers, drill files, job file and `uSonicTimer1d_revD_gerbers.zip` for ordering |
| `bom/ibom.html` | Interactive BOM (open in a browser) |
| `Renders/` | Top and bottom board renders |
| `[TODO]/` | Changes planned for the next board revision |

The library tables use `${KIPRJMOD}`, so the project opens from any location. Every
symbol and footprint is in the project libraries. Every 3D model comes from the stock
KiCad 10 3D library, so nothing is needed outside this folder and a standard KiCad 10
install. The encoder (ENC1) and the screw terminal (J1) have no 3D body: their
third-party models can't be redistributed. Sources and licences are listed in
[`3dmodels/README.md`](3dmodels/README.md).

Licensed under the CERN Open Hardware Licence Version 2, Strongly Reciprocal. See
[`LICENSE-HARDWARE`](../../LICENSE-HARDWARE).
SPDX-License-Identifier: CERN-OHL-S-2.0
