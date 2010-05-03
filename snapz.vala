// Snapz -- simple screenshot app.
// Copyright (c) 2010 Evan Martin.  All rights reserved.

using Gdk;
using Gtk;
using Json;
using Soup;

Gdk.Pixbuf* pixbuf_from_surface(Cairo.ImageSurface surface) {
    // Gdk.Pixbuf wants 24-bit RGB, but the Cairo RGB24 format
    // still produces 32-bit pixels where one byte is unused.  So
    // we must swizzle here.
    uchar* rgb32 = surface.get_data();
    var width = surface.get_width();
    var height = surface.get_height();
    var stride = surface.get_stride();
    uchar* rgb24 = malloc(width * height * 3);
    uchar* dst = rgb24;
    for (var y = 0; y < height; ++y) {
        uint32* src = (uint32*) (rgb32 + (y * stride));
        for (var x = 0; x < width; ++x) {
            *dst++ = src[x] >> 16;
            *dst++ = src[x] >> 8;
            *dst++ = src[x];
        }
    }

    return new Gdk.Pixbuf.from_data((uchar[])rgb24, Gdk.Colorspace.RGB, false,
                                    8, width, height, width*3,
                                    (PixbufDestroyNotify)g_free);
}

// Clamp x such that min <= x < max.
int clamp(int x, int min, int max) {
    if (x < min)
        return min;
    if (x >= max)
        return max - 1;
    return x;
}

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
        canvas.realize.connect(() => {
                canvas.window.set_cursor(
                        new Gdk.Cursor(Gdk.CursorType.CROSSHAIR));
            });

        canvas.button_press_event.connect((event) => {
                sel_start = canvas_to_image(Gdk.Point() {
                        x = (int)event.x, y = (int)event.y
                    });
                clamp_point(ref sel_start);
                sel_end = sel_start;
                selection_changed();
                return true;
            });
        canvas.motion_notify_event.connect((event) => {
                sel_end = canvas_to_image(Gdk.Point() {
                        x = (int)event.x, y = (int)event.y
                    });
                clamp_point(ref sel_end);
                selection_changed();
                return true;
            });
        canvas.button_release_event.connect((event) => {
                sel_end = canvas_to_image(Gdk.Point() {
                        x = (int)event.x, y = (int)event.y
                    });
                clamp_point(ref sel_end);
                if (sel_end.x == sel_start.x && sel_end.y == sel_start.y)
                    sel_start.x = -1;
                else {
                    if (sel_end.x < sel_start.x) {
                        var t = sel_start.x;
                        sel_start.x = sel_end.x;
                        sel_end.x = t;
                    }
                    if (sel_end.y < sel_start.y) {
                        var t = sel_start.y;
                        sel_start.y = sel_end.y;
                        sel_end.y = t;
                    }
                }
                selection_changed();
                return true;
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
        return sel_start.x != -1;
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

    // Highlight the current selection by darkening the rest of the image.
    public void highlight() {
        var width = pixbuf.get_width();
        var height = pixbuf.get_height();

        var surface = new Cairo.ImageSurface(Cairo.Format.RGB24, width, height);
        var ctx = new Cairo.Context(surface);
        Gdk.cairo_set_source_pixbuf(ctx, pixbuf, 0, 0);
        ctx.paint();

        draw_mask(ctx, 0.2f, width, height,
                  sel_start.x, sel_start.y, sel_end.x, sel_end.y);

        pixbuf = pixbuf_from_surface(surface);
        pixbuf_scaled = null;
        sel_start.x = -1;
        selection_changed();
    }

    // Clamp a point to the bounds of the image.
    private void clamp_point(ref Gdk.Point p) {
        p.x = clamp(p.x, 0, pixbuf.get_width());
        p.y = clamp(p.y, 0, pixbuf.get_height());
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

    // Darken all of the image except for the designated rectangle.
    private void draw_mask(Cairo.Context ctx, float alpha,
                           int width, int height,
                           float x1, float y1, float x2, float y2) {
        var mask = new Cairo.Surface.similar(ctx.get_target(),
                                             Cairo.Content.ALPHA,
                                             width, height);
        var mask_ctx = new Cairo.Context(mask);
        mask_ctx.set_source_rgba(0, 0, 0, alpha);
        mask_ctx.paint();
        mask_ctx.set_operator(Cairo.Operator.CLEAR);
        mask_ctx.rectangle(x1, y1, x2-x1, y2-y1);
        mask_ctx.fill();
        ctx.set_source_surface(mask, 0, 0);
        ctx.paint();
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
            draw_mask(ctx, 0.1f,
                      canvas.allocation.width, canvas.allocation.height,
                      start.x, start.y, end.x, end.y);

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


// Read a file to a buffer.  Man, this is ugly.
void read_file(string path, out uint8[] content) {
    content = new uint8[16 << 10];
    size_t total_len = 0;
    size_t len = 0;
    var file = FileStream.open(path, "rb");
    do {
        uint8 buf[4096];
        len = file.read(buf);
        if (len > 0) {
            if (total_len + len > content.length)
                content.resize(content.length * 2);
            Memory.copy((uint8*)content + total_len, buf, len);
            total_len += len;
        }
    } while (len > 0);
    content.resize((int)total_len);
}

errordomain ImgurError {
    FAIL
}
string imgur_parse_response(string response) throws ImgurError {
    var parser = new Json.Parser();
    try {
        if (!parser.load_from_data(response))
            throw new ImgurError.FAIL("Couldn't parse response");

        var root = parser.get_root().get_object();
        if (!root.has_member("rsp"))
            throw new ImgurError.FAIL("No 'rsp' element in response");
        var rsp = root.get_object_member("rsp");

        if (!rsp.has_member("stat"))
            throw new ImgurError.FAIL("No 'stat' element in response");
        var stat = rsp.get_string_member("stat");

        if (stat == "ok") {
            var image = rsp.get_object_member("image");
            if (!rsp.has_member("image"))
                throw new ImgurError.FAIL("No 'image' element in response");
            if (!image.has_member("original_image"))
                throw new ImgurError.FAIL("No 'original_image' element in " +
                                          "response");
            var url = image.get_string_member("original_image");
            return url;
        }

        if (!rsp.has_member("error_msg"))
            throw new ImgurError.FAIL("No 'error_msg' in response");
        throw new ImgurError.FAIL(rsp.get_string_member("error_msg"));
    } catch (ImgurError e) {
        throw e;
    } catch (GLib.Error e) {
        throw new ImgurError.FAIL("parser error: " + e.message);
    }
}

// The main window, containing the canvas and buttons to operate on it.
class SnapzWin : Gtk.Window {
    public SnapzWin(Gdk.Pixbuf shot) {
        title = "Snapz";
        set_default_size(shot.get_width() / 2, shot.get_height() / 2);

        var vbox = new Gtk.VBox(false, 12);
        vbox.border_width = 12;

        this.canvas = new ShotCanvas(shot);
        vbox.pack_start(canvas, true, true, 0);

        var bbox = new Gtk.HButtonBox();
        bbox.layout_style = Gtk.ButtonBoxStyle.START;

        var crop_button = new Gtk.Button.with_mnemonic("_Crop");
        crop_button.clicked.connect(() => {
                canvas.crop();
            });
        bbox.pack_start(crop_button, false, false, 0);

        var highlight_button = new Gtk.Button.with_mnemonic("_Highlight");
        highlight_button.clicked.connect(() => {
                canvas.highlight();
            });
        bbox.pack_start(highlight_button, false, false, 0);

        this.selection_buttons = { crop_button, highlight_button };
        foreach (var button in selection_buttons) {
            button.set_sensitive(false);
        }
        canvas.selection_changed.connect(() => {
                foreach (var button in selection_buttons)
                    button.set_sensitive(canvas.has_selection());
            });

        var imgur_button = new Gtk.Button.with_mnemonic("_imgur");
        imgur_button.clicked.connect(imgur_upload);
        bbox.pack_end(imgur_button, false, false, 0);
        bbox.set_child_secondary(imgur_button, true);

        var save_button = new Gtk.Button.with_mnemonic("_Save");
        save_button.clicked.connect(save);
        bbox.pack_end(save_button, false, false, 0);
        bbox.set_child_secondary(save_button, true);

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

    private void imgur_upload() {
        uint8[] content = null;

        // TODO: use Gdk.Pixbuf.save_to_buffer once I get a valac with
        // the fixed bindings.
        var path = "/tmp/imgur-upload.png";
        try {
            canvas.save(path);
            read_file(path, out content);
        } catch (GLib.Error e) {
            // XXX alert
            return;
        } finally {
            FileUtils.unlink(path);
        }

        var session = new Soup.SessionSync();
        var message = Soup.form_request_new(
            "POST",
            "http://imgur.com/api/upload.json",
            "key", "83b56669cd32cbd97fd604ecc8418037",
            "image", Base64.encode(content),
            null);

        try {
            var status = session.send_message(message);
            if (status != 200)
                throw new ImgurError.FAIL("HTTP status " + status.to_string());
            var response = message.response_body.flatten().data;
            var url = imgur_parse_response(response);
            var dlg = new Gtk.MessageDialog.with_markup(
                this,
                Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
                Gtk.MessageType.INFO,
                Gtk.ButtonsType.OK,
                "<tt>%s</tt>", url);
            dlg.set_title("Upload Success");
            dlg.response.connect(() => {
                    dlg.destroy();
                });
            dlg.run();
        } catch (ImgurError e) {
            var dlg = new Gtk.MessageDialog(
                this,
                Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
                Gtk.MessageType.ERROR,
                Gtk.ButtonsType.OK,
                "%s", e.message);
            dlg.set_title("Upload Error");
            dlg.response.connect(() => {
                    dlg.destroy();
                });
            dlg.run();
        }
    }

    // The canvas widget.
    private ShotCanvas canvas;

    // Buttons that become insensitive when there is no selection.
    private Gtk.Button[] selection_buttons;
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
