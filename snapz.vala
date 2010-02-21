// Snapz -- simple screenshot app.
// Copyright (c) 2010 Evan Martin.  All rights reserved.

using Gdk;
using Gtk;

// ShotCanvas is the main widget in the window, letting you select
// regions of and crop the image.  It's rooted in an AspectFrame
// so it can maintain the correct aspect ratio regardless of the
// window size.
class ShotCanvas : Gtk.AspectFrame {
    public ShotCanvas(Gdk.Pixbuf pixbuf) {
        this.obey_child = false;
        this.shadow_type = Gtk.ShadowType.NONE;

        this.pixbuf = pixbuf;
        this.pixbuf_scaled = null;
        this.ratio = pixbuf.get_width() / (float)pixbuf.get_height();
        this.sel_start.x = -1;

        canvas = new Gtk.DrawingArea();
        canvas.add_events(Gdk.EventMask.BUTTON_PRESS_MASK |
                          Gdk.EventMask.BUTTON1_MOTION_MASK |
                          Gdk.EventMask.BUTTON_RELEASE_MASK);

        canvas.button_press_event.connect((event) => {
                sel_start = canvas_to_image(Gdk.Point() {
                        x = (int)event.x, y = (int)event.y
                    });
                sel_end = sel_start;
                selection_changed();
            });
        canvas.motion_notify_event.connect((event) => {
                sel_end = canvas_to_image(Gdk.Point() {
                        x = (int)event.x, y = (int)event.y
                    });
                selection_changed();
            });
        canvas.button_release_event.connect((event) => {
                sel_end = canvas_to_image(Gdk.Point() {
                        x = (int)event.x, y = (int)event.y
                    });
                if (sel_end.x == sel_start.x && sel_end.y == sel_start.y)
                    sel_start.x = -1;
                selection_changed();
            });

        canvas.expose_event.connect(draw);
        selection_changed.connect(() => { queue_draw(); });
        add(canvas);
    }

    // Save the image to a file.
    public void save(string filename) throws GLib.Error {
        pixbuf.save(filename, "png", null);
    }

    // Return true if there is a region currently selected.
    public bool has_selection() {
        return sel_start.x != -1 &&
            (sel_start.x != sel_end.x ||
             sel_start.y != sel_end.y);
    }

    // A signal that's fired whenever the selection changes.
    public signal void selection_changed();

    // Crop the image to the current selection.
    public void crop() {
        pixbuf = new Gdk.Pixbuf.subpixbuf(pixbuf, sel_start.x, sel_start.y,
                                          sel_end.x - sel_start.x,
                                          sel_end.y - sel_start.y);
        pixbuf_scaled = null;
        sel_start.x = -1;
        selection_changed();
        this.ratio = pixbuf.get_width() / (float)pixbuf.get_height();
    }


    // Multiply the x/y coords of point |p| by |mult|.
    private Gdk.Point scale_point(Gdk.Point p, float mult) {
        return Gdk.Point() {
            x = (int)Math.round(p.x * mult),
            y = (int)Math.round(p.y * mult)
        };
    }

    // Convert from canvas (scaled-down, on screen) space to image space.
    private Gdk.Point canvas_to_image(Gdk.Point p) {
        return scale_point(p,
                           pixbuf.get_width() / (float)canvas.allocation.width);
    }
    // Convert from image (underlying hi-res image) space to canvas space.
    private Gdk.Point image_to_canvas(Gdk.Point p) {
        return scale_point(p,
                           canvas.allocation.width / (float)pixbuf.get_width());
    }

    // Recompute |pixbuf_scaled|, the scaled version of |pixbuf|, if needed.
    private void rescale_if_necessary() {
        var width = canvas.allocation.width;
        var height = canvas.allocation.height;

        if (pixbuf_scaled != null &&
            pixbuf_scaled.get_width() == width &&
            pixbuf_scaled.get_height() == height) {
            return;
        }

        pixbuf_scaled = pixbuf.scale_simple(width, height,
                                            Gdk.InterpType.TILES);
    }

    // Redraw the the current contents.
    private bool draw(Gdk.EventExpose expose) {
        if (canvas.allocation.width == 1 || canvas.allocation.height == 1) {
            // On startup we get a 1x1 expose which makes the scaling
            // computation pathological.
            return false;
        }

        rescale_if_necessary();

        var ctx = Gdk.cairo_create(canvas.window);
        // Start by drawing the image into the context.
        Gdk.cairo_set_source_pixbuf(ctx, pixbuf_scaled, 0, 0);
        ctx.paint();

        if (has_selection()) {
            var start = image_to_canvas(sel_start);
            var end = image_to_canvas(sel_end);

            // Cut out the selected region from a mask and apply it.
            var mask = new Cairo.Surface.similar(ctx.get_target(),
                                                 Cairo.Content.ALPHA,
                                                 canvas.allocation.width,
                                                 canvas.allocation.height);
            var mask_ctx = new Cairo.Context(mask);
            mask_ctx.set_source_rgba(0, 0, 0, 0.1);
            mask_ctx.paint();
            mask_ctx.translate(0.5, 0.5);
            mask_ctx.set_operator(Cairo.Operator.CLEAR);
            mask_ctx.rectangle(start.x, start.y,
                               end.x - start.x, end.y - start.y);
            mask_ctx.fill();
            ctx.set_source_surface(mask, 0, 0);
            ctx.paint();

            // Draw the dotted line around the selection.
            ctx.translate(0.5, 0.5);
            ctx.set_line_width(1);
            // First draw a black background for the line.
            ctx.set_source_rgb(0, 0, 0);
            ctx.rectangle(start.x, start.y,
                          end.x - start.x, end.y - start.y);
            ctx.stroke();
            // Then draw white dashes.
            ctx.set_source_rgb(1, 1, 1);
            ctx.set_dash({5, 5}, 0);
            ctx.rectangle(start.x, start.y,
                          end.x - start.x, end.y - start.y);
            ctx.stroke();
        }

        return true;
    }

    // Source image.
    private Gdk.Pixbuf pixbuf;
    // Image scaled to current available space.
    private Gdk.Pixbuf? pixbuf_scaled;

    // The actual widget we draw on.
    private Gtk.DrawingArea canvas;

    // Current selection, if any.
    // sel_start.x == -1 indicates no selection.
    private Gdk.Point sel_start;
    private Gdk.Point sel_end;
}

// The main window, containing the canvas and buttons to operate on it.
class SnapzWin : Gtk.Window {
    public SnapzWin(Gdk.Pixbuf shot) {
        title = "Snapz";

        var vbox = new Gtk.VBox(false, 12);
        vbox.border_width = 12;

        this.canvas = new ShotCanvas(shot);
        vbox.pack_start(canvas, true, true, 0);

        var bbox = new Gtk.HButtonBox();
        bbox.layout_style = Gtk.ButtonBoxStyle.EDGE;

        this.crop_button = new Gtk.Button.with_mnemonic("_Crop");
        crop_button.set_sensitive(false);
        canvas.selection_changed.connect(() => {
                crop_button.set_sensitive(canvas.has_selection());
            });
        crop_button.clicked.connect(() => {
                canvas.crop();
            });
        bbox.add(crop_button);

        var save_button = new Gtk.Button.with_mnemonic("_Save");
        save_button.clicked.connect(save);
        bbox.pack_end(save_button, false, false, 0);

        vbox.pack_start(bbox, false, false, 0);

        add(vbox);
        vbox.show_all();

        destroy.connect(Gtk.main_quit);
    }

    private void save() {
        var dialog = new Gtk.FileChooserDialog(
            null, this, Gtk.FileChooserAction.SAVE,
            Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
            Gtk.STOCK_OK, Gtk.ResponseType.OK,
            null);
        dialog.set_current_name("Screenshot.png");
        var response = dialog.run();
        if (response == Gtk.ResponseType.OK) {
            try {
                canvas.save(dialog.get_filename());
            } catch (GLib.Error e) {
                // XXX alert
            }
        }
        dialog.destroy();
    }

    // The canvas widget.
    private ShotCanvas canvas;

    // This must be a member for the selection_changed closure to work(!).
    private Gtk.Button crop_button;
}

// Take a screenshot and return the pixbuf.
Gdk.Pixbuf screenshot() {
    var screen = Gdk.Screen.get_default();
    return Gdk.pixbuf_get_from_drawable(
        null, screen.get_root_window(), null, 0, 0,
        0, 0, screen.get_width(), screen.get_height());
}

int main(string[] args) {
    Gtk.init(ref args);

    var shot = screenshot();
    var win = new SnapzWin(shot);
    win.show();
    Gtk.main();

    return 0;
}
