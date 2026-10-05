# 3D models: uSonicTimer rev 1d

This folder is where project-local 3D models go. Footprints reference them as
`${KIPRJMOD}/3dmodels/<file>`. **It holds no model files right now.** None of the
third-party models the board used could be redistributed in this public
CERN-OHL-S-2.0 repository (details below).

All 3D bodies the board shows now come from the stock KiCad 10 3D library
(`${KICAD10_3DMODEL_DIR}`, installed with KiCad). That library is licensed under
CC-BY-SA 4.0 with the KiCad library exception, so it can be used in any design.

## Third-party models removed from the board and the project footprints (2026-10-04)

| Part | Footprint | Model referenced before | Source and licence | Decision |
|---|---|---|---|---|
| Q1-Q5 | `SOT-23-3` | `/Users/.../KiCad/lib/3dmodels/km3dpacks/User Library-SOT23-3.step` (absolute path) | Downloaded by Kevin from Dassault's 3D ContentCentral (3dcontentcentral.com); the "User Library-…" name is its export naming. SolidWorks 2022 STEP, 2023-02-13. User-uploaded under Dassault's terms. | **Not committed** (no redistribution licence). Replaced by the stock `Package_TO_SOT_SMD.3dshapes/SOT-23-3.step` (same package, same pad layout). |
| ENC1 | `RotaryEncoder_Alps_EC11E-Switch_Vertical_H20mm` | `${MYSPECLMOD}/EC11 Rotary Encoder Dode Switch-15mm .STEP` (offset 7.5 -2.5 0.1, rot 0 0 90) | GrabCAD model "EC11 Rotary Encoder Dode Switch-15mm" by user *xindela* (SolidWorks 2016, 2021-10-20). GrabCAD terms allow only non-commercial, internal use, under a non-sublicensable licence. | **Not committed** (can't be relicensed or redistributed). Removed. ENC1 has no 3D body, and KiCad ships no stock EC11 model. |
| ENC1 (hidden) | same | `${MYSPECLMOD}/User Library-EC11B15244.STEP` | Downloaded by Kevin from 3D ContentCentral ("User Library-…" export naming). User-uploaded under Dassault's terms. File is no longer on Kevin's machines. | Removed |
| ENC1 (hidden) | same | `${KICAD6_3DMODEL_DIR}/Rotary_Encoder.3dshapes/...EC11E-Switch_Vertical_H20mm.wrl` | KiCad stock path, but KiCad 10 ships no such model. | Removed (dangling) |
| SW1 | `SW_Tactile_SPST_NO_Straight_CK_PTS636Sx25SMTRLFS` | `${MYSPECLMOD}/g73-6x3-5-mm-smd-button-1/...PTS636Sx25SMTRLFS.STEP` | File isn't on Kevin's machines. Probably 3D ContentCentral like the others; the folder name also looks like a GrabCAD download ("g73-6x3-5-mm-smd-button-1"). | Removed. The stock KiCad model for this exact C&K part was already on the footprint (hidden); it is now shown. |
| J1 | `ScrewContact_ALLELEC_2,5_5-G-5,00_1x05_P5.00mm_Horizontal_copy_copy` | `${MYSPECLMOD}/pcb-terminal-blocks-1/BR1102V.stp` and `BR1103V.stp` (scaled 0.5) | Files aren't on Kevin's machines. Probably 3D ContentCentral like the others; the folder name also looks like a GrabCAD download ("pcb-terminal-blocks-1"). | Removed. J1 has no 3D body. |

### Where the excluded models came from

Kevin downloaded most of the excluded models from **3D ContentCentral**
(3dcontentcentral.com, run by Dassault Systèmes; the site appears to be gone now):
- Q1-Q5 `User Library-SOT23-3`
- ENC1 `User Library-EC11B15244`
- probably the SW1 and J1 models too

The `User Library-…` file names match 3D ContentCentral's export naming. These models
were uploaded by users under Dassault's terms, which give no licence to redistribute
them, so they are not included in this CERN-OHL-S repository. Keep them in a local
library instead. The EC11 encoder model (`EC11 Rotary Encoder Dode Switch-15mm`) is
attributed to GrabCAD user *xindela*, as above.

`${MYSPECLMOD}` is not defined in Kevin's KiCad 9/10 configuration. So none of the
`MYSPECLMOD` models resolved on any machine before this change either.

To see the original bodies locally, Kevin can add them back in his own copy, but must not
commit them. Use `${KIPRJMOD}/3dmodels/<file>`, add the file names to a local
`.git/info/exclude`, and use the offsets listed above. Models with a licence that allows
redistribution (for example the manufacturer's own STEP with redistribution terms, or a
model Kevin draws himself) can be committed here, with a row added to this table.

## Symbol and footprint libraries (project-local, `${KIPRJMOD}`)

| Library | Contents | Sources and licences |
|---|---|---|
| `uSonicTimer1d.kicad_sym` (`sym-lib-table`, nickname `uSonicTimer1d`) | All 19 symbols the schematic uses | KiCad stock symbols, CC-BY-SA 4.0 with the KiCad library exception. `EOMZ-240D25R` and `MMBT3904LT1G` come from the Digi-Key KiCad Library, CC-BY-SA 4.0 with a design-use exception (https://github.com/Digi-Key/digikey-kicad-library). `Nokia_5110_LCD` is Kevin's own (km-kicad-library). |
| `uSonicTimer1d.pretty` (`fp-lib-table`, nickname `uSonicTimer1d`) | All 17 footprints on the board | Mostly KiCad stock footprints, CC-BY-SA 4.0 with the exception. `SOT-23-3` is the Digi-Key KiCad Library version, CC-BY-SA 4.0 with the exception. `ICSP_5_POGO`, `Nokia_5110-3310_LCD_uSonic1d` and `kibuzzard-63EC72D8` (KiBuzzard silkscreen label) are Kevin's own. `ScrewContact_ALLELEC_...` comes from Kevin's personal `Connector_ScrewTerminal.pretty`; it follows the KiCad terminal-block generator format and its origin isn't otherwise documented. |

Only the stock KiCad libraries (for example `Package_TO_SOT_THT`, used in the DS18B20
footprint field) and the stock 3D library are taken from the KiCad installation.
