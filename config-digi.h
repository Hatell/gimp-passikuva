/* GIMP - The GNU Image Manipulation Program - Passikuva
 * vi: et sw=2 fileencoding=utf8
 * Copyright (C) 2013 Pispalan Insinööritoimisto Oy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define PLUG_IN_PROC   "plug-in-passikuva-digi"
#define PLUG_IN_BINARY "passikuva-digi"
#define PLUG_IN_ROLE   "gimp-passikuva-digi"

#define DIGIPASSIKUVA_LEVEYS 500
#define DIGIPASSIKUVA_KORKEUS 653

static void
query (void)
{
  static const GimpParamDef args[] =
  {
    { GIMP_PDB_INT32,    "run-mode",  "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { GIMP_PDB_IMAGE,    "image",      "Input image (unused)"        },
    { GIMP_PDB_DRAWABLE, "org-drawable",   "Input drawable"              },
  };

  static const GimpParamDef return_vals[] =
  {
    { GIMP_PDB_IMAGE, "new-image", "Output image (-1 if new-image == FALSE)" },
    { GIMP_PDB_LAYER, "new-layer", "Output layer (-1 if new-image == FALSE)" }
  };

  gimp_install_procedure (PLUG_IN_PROC,
                          "Passikuva digi",
                          "Passikuva, digikuvaa",
                          "Pispalan Insinööritoimisto Oy - Harry Karvonen",
                          "Pispalan Insinööritoimisto Oy - Harry Karvonen",
                          "2013",
                          "1 Digikuva",
                          "RGB*, GRAY*, INDEXED*",
                          GIMP_PLUGIN,
                          G_N_ELEMENTS (args),
                          G_N_ELEMENTS (return_vals),
                          args, return_vals);

  gimp_plugin_menu_register (PLUG_IN_PROC, "<Image>/Passikuva");
}
