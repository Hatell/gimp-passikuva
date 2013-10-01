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

GimpRunMode run_mode;
gdouble       kuvan_korkeus = 47.0;

MAIN ()

static void run (
  const gchar      *name,
  gint              nparams,
  const GimpParam  *param,
  gint             *nreturn_vals,
  GimpParam       **return_vals
) {
  static GimpParam  values[3];
  GimpPDBStatusType status    = GIMP_PDB_SUCCESS;
  gint32            new_layer = -1;

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 3;
  *return_vals  = values;

  values[0].type          = GIMP_PDB_STATUS;
  values[0].data.d_status = status;
  values[1].type          = GIMP_PDB_IMAGE;
  values[2].type          = GIMP_PDB_LAYER;

  switch (run_mode) {
    case GIMP_RUN_INTERACTIVE:
      if (!passikuva_dialog()) {
        return;
      }
      break;

    case GIMP_RUN_NONINTERACTIVE:
      break;

    case GIMP_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      //gimp_get_data (PLUG_IN_PROC, &tvals);
      break;

    default:
      break;
  }

  if (status == GIMP_PDB_SUCCESS) {
    gimp_progress_init ("Passikuva");

    values[1].data.d_image = passikuva (param[1].data.d_image,
                                   param[2].data.d_drawable,
                                   &new_layer);
    values[2].data.d_layer = new_layer;

    /*  Store data  */
    //if (run_mode == GIMP_RUN_INTERACTIVE)
    //  gimp_set_data (PLUG_IN_PROC, &tvals, sizeof (PassikuvaVals));

    if (run_mode != GIMP_RUN_NONINTERACTIVE) {
      gimp_display_new (values[1].data.d_image);
    }
  }

  values[0].data.d_status = status;
}

static gint32 passikuva (
  gint32  image_id,
  gint32  drawable_id,
  gint32 *layer_id
) {
  GimpPixelRgn       src_rgn;
  GimpPixelRgn       dest_rgn;
  GimpDrawable      *org_drawable = 0;
  GimpDrawable      *new_layer = 0;
  GimpImageBaseType  image_type   = GIMP_RGB;
  gint32             new_image_id = 0;
  gint               org_width = 0;
  gint               org_height = 0;
  gint               gap = 0;
  gint               new_width = 0;
  gint               new_height = 0;
  gint               i = 0, j = 0;
  gint               x1 = 0, y1 = 0, x2 = 0, y2 = 0;
  gint               progress = 0;
  gint               max_progress = 0;
  gpointer           pr = 0;
  gdouble            kerroin = 0;

  gimp_drawable_mask_bounds (
    drawable_id,
    &x1, &y1,
    &x2, &y2
  );

  org_drawable = gimp_drawable_get(drawable_id);

  org_width = x2 - x1;
  org_height = y2 - y1;

  kerroin = ((50.5 - kuvan_korkeus) / 2.0) / kuvan_korkeus;

  if (org_height >= org_width) {
    gap = org_height * kerroin;
  } else {
    gap = org_width * kerroin;
  }

  new_width = tvals.images_x * (org_width + gap * 2);
  new_height = tvals.images_y * (org_height + gap * 2);
  if (new_width <= 1.5 * new_height) {
    new_width = 1.5 * new_height;
  } else {
    new_height = 1.5 * new_width;
  }

  //g_print("new width = %d, heigth = %d\n", new_width, new_height);

  /*  create  a new image  */
  switch (gimp_drawable_type (drawable_id)) {
    case GIMP_RGB_IMAGE:
    case GIMP_RGBA_IMAGE:
      image_type = GIMP_RGB;
      break;

    case GIMP_GRAY_IMAGE:
    case GIMP_GRAYA_IMAGE:
      image_type = GIMP_GRAY;
      break;

    case GIMP_INDEXED_IMAGE:
    case GIMP_INDEXEDA_IMAGE:
      image_type = GIMP_INDEXED;
      break;
  }

  // Luodaan lopullinen kuva
  new_image_id = gimp_image_new(
    new_width,
    new_height,
    image_type
  );
  gimp_image_undo_disable(new_image_id);

  *layer_id = gimp_layer_new(
    new_image_id, "Background",
    new_width, new_height,
    image_type,
    100, GIMP_NORMAL_MODE
  );

  if (*layer_id == -1) {
    return -1;
  }

  gimp_image_insert_layer(new_image_id, *layer_id, -1, 0);
  new_layer = gimp_drawable_get(*layer_id);

  /*  progress  */
  progress = 0;
  max_progress = new_width * new_height;

  gimp_drawable_fill(new_layer->drawable_id, GIMP_BACKGROUND_FILL);
/*
  g_print("SRC x: %d, y: %d, w: %d, h: %d: %d\n",
    x1,
    y1,
    org_width,
    org_height
  );
*/

  /*  tile...  */
  for (i = 0; i < tvals.images_y; i ++) {
    for (j = 0; j < tvals.images_x; j++) {
      gint c;

      gimp_pixel_rgn_init (
        &src_rgn, org_drawable,
        x1, y1,
        org_width, org_height,
        FALSE, FALSE
      );

/*
      g_print("DST x: %d, y: %d, w: %d, h: %d: %d\n",
        tvals.gap + j * (org_width + tvals.gap * 2), // x
        tvals.gap + i * (org_height + tvals.gap * 2), // y
        org_width,
        org_height
      );
*/

      gimp_pixel_rgn_init (
        &dest_rgn, new_layer,
        gap + j * (org_width + gap * 2), // x
        gap + i * (org_height + gap * 2), // y
        org_width, org_height,
        TRUE, FALSE
      );

      for (pr = gimp_pixel_rgns_register (2, &src_rgn, &dest_rgn), c = 0;
           pr != NULL;
           pr = gimp_pixel_rgns_process (pr), c++) {
        gint k;

        for (k = 0; k < src_rgn.h; k++) {
          memcpy (dest_rgn.data + k * dest_rgn.rowstride,
              src_rgn.data + k * src_rgn.rowstride,
              src_rgn.w * src_rgn.bpp);
        }

        progress += src_rgn.w * src_rgn.h;

        if (c % 16 == 0) {
          gimp_progress_update ((gdouble) progress /
              (gdouble) max_progress);
        }
      }
    }
  }

  gimp_progress_update (1.0);

  gimp_drawable_detach(org_drawable);
  gimp_drawable_detach(new_layer);

  gimp_image_set_resolution(new_image_id, 300, 300);

  /*  copy the colormap, if necessary  */
  if (image_type == GIMP_INDEXED) {
    guchar *cmap;
    gint    ncols;

    cmap = gimp_image_get_colormap (image_id, &ncols);
    gimp_image_set_colormap (new_image_id, cmap, ncols);
    g_free (cmap);
  }

  gimp_image_undo_enable (new_image_id);

  return new_image_id;
}


static gboolean passikuva_dialog(void)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *main_hbox;
  GtkWidget *frame;
  GtkWidget *radius_label;
  GtkWidget *alignment;
  GtkWidget *spinbutton;
  GtkObject *spinbutton_adj;
  GtkWidget *frame_label;
  gboolean   run;

  gimp_ui_init ("passikuva", FALSE);

  dialog = gimp_dialog_new (
    "Passikuva",
    "passikuva",
    NULL, 0,
    NULL, 0, //gimp_standard_help_func, "plug-in-passikuva",
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_OK,     GTK_RESPONSE_OK,
    NULL
  );

  main_vbox = gtk_vbox_new (FALSE, 6);
  //gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), main_vbox);
  gtk_container_add (GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), main_vbox);
  gtk_widget_show (main_vbox);

  frame = gtk_frame_new (NULL);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 6);

  alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment);
  gtk_container_add (GTK_CONTAINER (frame), alignment);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 6, 6, 6, 6);

  main_hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (main_hbox);
  gtk_container_add (GTK_CONTAINER (alignment), main_hbox);

  radius_label = gtk_label_new_with_mnemonic ("_Kuvan korkeus (mm):");
  gtk_widget_show (radius_label);
  gtk_box_pack_start (GTK_BOX (main_hbox), radius_label, FALSE, FALSE, 6);
  gtk_label_set_justify (GTK_LABEL (radius_label), GTK_JUSTIFY_RIGHT);

  spinbutton_adj = gtk_adjustment_new (47, 1, 51, 1, 5, 5);
  spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 1, 2);
  gtk_widget_show (spinbutton);
  gtk_box_pack_start (GTK_BOX (main_hbox), spinbutton, FALSE, FALSE, 6);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton), TRUE);

  frame_label = gtk_label_new ("Aseta kuvan korkeus");
  gtk_widget_show (frame_label);
  gtk_frame_set_label_widget (GTK_FRAME (frame), frame_label);
  gtk_label_set_use_markup (GTK_LABEL (frame_label), TRUE);

  g_signal_connect (spinbutton_adj, "value_changed",
      G_CALLBACK (gimp_double_adjustment_update),
      &kuvan_korkeus);
  gtk_widget_show (dialog);

  run = (gimp_dialog_run (GIMP_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}
