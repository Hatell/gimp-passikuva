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
gdouble kuvan_korkeus = 47.0;

MAIN ()

static void run (
  const gchar      *name,
  gint              nparams,
  const GimpParam  *param,
  gint             *nreturn_vals,
  GimpParam       **return_vals
) {
  static GimpParam  values[3];
  gint32            new_layer = -1;

#if 0
  g_print("passikuva.run(%s, %d, ...)\n", name, nparams);
#endif

  run_mode = param[0].data.d_int32;

  switch(run_mode) {
    case GIMP_RUN_INTERACTIVE:
      if(!passikuva_dialog()) {
        *nreturn_vals = 0;
        return;
      }
      break;

    case GIMP_RUN_NONINTERACTIVE:
    case GIMP_RUN_WITH_LAST_VALS:
      break;

    default:
      break;
  }

  gimp_progress_init("Passikuva");

  values[1].type = GIMP_PDB_IMAGE;
  values[1].data.d_image = passikuva(
    param[1].data.d_image,
    param[2].data.d_drawable,
    &new_layer
  );

  if (run_mode != GIMP_RUN_NONINTERACTIVE) {
    gimp_display_new(values[1].data.d_image);
  }

  values[2].type = GIMP_PDB_LAYER;
  values[2].data.d_layer = new_layer;

  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = GIMP_PDB_SUCCESS;

  *nreturn_vals = 3;
  *return_vals = values;
}

static gint32 passikuva (
  gint32  image_id,
  gint32  drawable_id,
  gint32 *layer_id
) {
  GimpDrawable      *org_drawable = 0;
  GimpDrawable      *new_layer = 0;
  GimpDrawable      *tmp_layer = 0;
  GimpImageBaseType  image_type   = GIMP_RGB;
  gint32             new_image_id = 0;
  gint32 tmp_layer_id = 0;
  gint               org_width = 0;
  gint               org_height = 0;
  gint dst_height = 653;
  gint dst_width = 0;
  gint               gap = 0;
  gint               new_width = 0;
  gint               new_height = 0;
  gint               i = 0, j = 0;
  gint               x1 = 0, y1 = 0, x2 = 0, y2 = 0;
  gint c = 0;
  gdouble            kerroin = 0;

#if 0
  g_print("passikuva.passikuva(%d, %d, %d)\n", image_id, drawable_id, *layer_id);

  g_print(" - gimp_drawable_mask_bounds(%d, ->", drawable_id);
#endif
  gimp_drawable_mask_bounds (
    drawable_id,
    &x1, &y1,
    &x2, &y2
  );

#if 0
  g_print("%d, ->%d, ->%d, ->%d)\n", x1, y1, x2, y2);
#endif

  org_drawable = gimp_drawable_get(drawable_id);

  org_width = x2 - x1;
  org_height = y2 - y1;

  dst_width = ((org_width * dst_height) / org_height) + 0.5;

  kerroin = ((50.5 - kuvan_korkeus) / 2.0) / kuvan_korkeus;

  if (dst_height >= dst_width) {
    gap = dst_height * kerroin;
  } else {
    gap = dst_width * kerroin;
  }

  new_width = tvals.images_x * (dst_width + gap * 2);
  new_height = tvals.images_y * (dst_height + gap * 2);

  if (new_width <= 1.5 * new_height) {
    new_width = 1.5 * new_height;
  } else {
    new_height = 1.5 * new_width;
  }

#if 0
  g_print("new width = %d, heigth = %d\n", new_width, new_height);
#endif

  /*  create  a new image  */
  switch (gimp_drawable_type(drawable_id)) {
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

  tmp_layer_id = gimp_layer_new(
    new_image_id, "tmp",
    org_width, org_height,
    gimp_drawable_type(drawable_id),
    100, GIMP_NORMAL_MODE
  );

  if (tmp_layer_id == -1) {
    return -1;
  }

  gimp_image_add_layer(new_image_id, tmp_layer_id, -1);

  tmp_layer = gimp_drawable_get(tmp_layer_id);

  copy_area(
    org_drawable,
    tmp_layer,
    x1,
    y1,
    0,
    0,
    org_width,
    org_height
  );

  gimp_layer_scale(tmp_layer_id, dst_width, dst_height, FALSE);

  gimp_drawable_detach(tmp_layer);
  gimp_drawable_detach(org_drawable);

  *layer_id = gimp_layer_new(
    new_image_id, "Passikuva",
    new_width, new_height,
    gimp_drawable_type(drawable_id),
    100, GIMP_NORMAL_MODE
  );

  if (*layer_id == -1) {
    return -1;
  }

  gimp_image_add_layer(new_image_id, *layer_id, -1);

  new_layer = gimp_drawable_get(*layer_id);
  tmp_layer = gimp_drawable_get(tmp_layer_id);

  gimp_drawable_fill(new_layer->drawable_id, GIMP_BACKGROUND_FILL);
  /*  tile...  */
  for (i = 0; i < tvals.images_y; i ++) {
    for (j = 0; j < tvals.images_x; j++) {
      copy_area(
        tmp_layer,
        new_layer,
        0,
        0,
        gap + j * (dst_width + gap * 2), // x
        gap + i * (dst_height + gap * 2), // y
        dst_width,
        dst_height
      );

      gimp_progress_update(c / (tvals.images_x * tvals.images_y));

      c++;
    }
  }

#if 0
  g_print("passikuva.gimp_progress_update(1.0)");
#endif

  gimp_progress_update (1.0);
  gimp_drawable_detach(new_layer);
  gimp_drawable_detach(tmp_layer);

  gimp_image_remove_layer(new_image_id, tmp_layer_id);
  gimp_image_set_resolution(new_image_id, 356.108, 356.108);

  /*  copy the colormap, if necessary  */
  if (image_type == GIMP_INDEXED) {
    guchar *cmap;
    gint    ncols;

    cmap = gimp_image_get_colormap (image_id, &ncols);
    gimp_image_set_colormap (new_image_id, cmap, ncols);
    g_free (cmap);
  }

  gimp_image_undo_enable(new_image_id);

  return new_image_id;
}

static void copy_area(
  GimpDrawable *src_layer,
  GimpDrawable *dst_layer,
  gint src_x,
  gint src_y,
  gint dst_x,
  gint dst_y,
  gint sd_width,
  gint sd_height
) {
  GimpPixelRgn src_rgn;
  GimpPixelRgn dest_rgn;
  gpointer           pr = 0;

#if 0
  g_print("SRC x: %d, y: %d, w: %d, h: %d\n",
    src_x,
    src_y,
    sd_width,
    sd_height
  );
#endif

  gimp_pixel_rgn_init(
    &src_rgn, src_layer,
    src_x, src_y,
    sd_width, sd_height,
    FALSE, FALSE
  );

#if 0
  g_print("DST x: %d, y: %d, w: %d, h: %d\n",
    dst_x,
    dst_y,
    sd_width,
    sd_height
  );
#endif

  gimp_pixel_rgn_init(
    &dest_rgn, dst_layer,
    dst_x,
    dst_y,
    sd_width, sd_height,
    TRUE, FALSE
  );

  pr = gimp_pixel_rgns_register(2, &src_rgn, &dest_rgn);
  do {
    gint k;

    for (k = 0; k < src_rgn.h; k++) {
      memcpy (dest_rgn.data + k * dest_rgn.rowstride,
          src_rgn.data + k * src_rgn.rowstride,
          src_rgn.w * src_rgn.bpp);
    }
  } while ((pr = gimp_pixel_rgns_process(pr)));
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

  gimp_ui_init("passikuva", FALSE);

  dialog = gimp_dialog_new(
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

  spinbutton_adj = gtk_adjustment_new (kuvan_korkeus, 1, 51, 1, 5, 5);
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
