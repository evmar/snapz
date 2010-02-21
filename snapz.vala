using Gdk;
using Gtk;

class ShotCanvas : Gtk.DrawingArea {
	public ShotCanvas(Gdk.Pixbuf pixbuf) {
		add_events(Gdk.EventMask.BUTTON_PRESS_MASK |
				   Gdk.EventMask.BUTTON1_MOTION_MASK |
				   Gdk.EventMask.BUTTON_RELEASE_MASK);

		this.pixbuf = pixbuf;
		this.pixbuf_scaled = null;
		this.sel_start.x = -1;

		this.button_press_event.connect((event) => {
				sel_start = canvas_to_image(Gdk.Point() {
						x = (int)event.x, y = (int)event.y
					});
				sel_end = sel_start;
				selection_changed();
			});
		this.motion_notify_event.connect((event) => {
				sel_end = canvas_to_image(Gdk.Point() {
						x = (int)event.x, y = (int)event.y
					});
				selection_changed();
			});
		this.button_release_event.connect((event) => {
				sel_end = canvas_to_image(Gdk.Point() {
						x = (int)event.x, y = (int)event.y
					});
				if (sel_end.x == sel_start.x && sel_end.y == sel_start.y)
					sel_start.x = -1;
				selection_changed();
			});

		expose_event.connect(draw);
		selection_changed.connect(() => { queue_draw(); });
	}

	public void save(string filename) throws GLib.Error {
		pixbuf.save(filename, "png", null);
	}

	public bool has_selection() {
		return sel_start.x != -1;
	}

	public signal void selection_changed();


	public void crop() {
		pixbuf = new Gdk.Pixbuf.subpixbuf(pixbuf, sel_start.x, sel_start.y,
										  sel_end.x - sel_start.x,
										  sel_end.y - sel_start.y);
		pixbuf_scaled = null;
		sel_start.x = -1;
		selection_changed();
	}


	private Gdk.Point scale_point(Gdk.Point p, float mult) {
		return Gdk.Point() {
			x = (int)(p.x * mult),
			y = (int)(p.y * mult)
		};
	}

	private Gdk.Point canvas_to_image(Gdk.Point p) {
		return scale_point(p, pixbuf.get_width() / (float)allocation.width);
	}
	private Gdk.Point image_to_canvas(Gdk.Point p) {
		return scale_point(p, allocation.width / (float)pixbuf.get_width());
	}

	private void rescale_if_necessary() {
		var aspect = pixbuf.get_width() / (float)pixbuf.get_height();
		var width = allocation.width;
		var height = allocation.height;

		if (height * aspect > width)  // too tall to fit; clip height
			height = (int) (width / aspect);
		else
			width = (int) (height * aspect);

		if (pixbuf_scaled != null &&
			pixbuf_scaled.get_width() == width &&
			pixbuf_scaled.get_height() == height) {
			return;
		}

		//stdout.printf("w %d h %d\n", width, height);
		pixbuf_scaled = pixbuf.scale_simple(width, height,
											Gdk.InterpType.TILES);
	}

	private bool draw(Gdk.EventExpose expose) {
		if (allocation.width == 1 || allocation.height == 1) {
			// On startup we get a 1x1 expose which makes the scaling
			// computation pathological.
			return false;
		}

		rescale_if_necessary();

		var ctx = Gdk.cairo_create(window);
		Gdk.cairo_set_source_pixbuf(ctx, pixbuf_scaled, 0, 0);
		ctx.paint();

		if (has_selection()) {
			var start = image_to_canvas(sel_start);
			var end = image_to_canvas(sel_end);

			ctx.translate(0.5, 0.5);
			ctx.set_line_width(1);

			ctx.set_source_rgb(0, 0, 0);
			ctx.rectangle(start.x, start.y,
						  end.x - start.x, end.y - start.y);
			ctx.stroke();

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

	// Current selection, if any.
	private Gdk.Point sel_start;
	private Gdk.Point sel_end;
}

class SnapzWin : Gtk.Window {
	private ShotCanvas canvas;
	private Gtk.Button crop_button;

	public SnapzWin(Gdk.Pixbuf shot) {
		title = "Snapz";

		var vbox = new Gtk.VBox(false, 12);
		vbox.border_width = 12;

		canvas = new ShotCanvas(shot);
		vbox.pack_start(canvas, true, true, 0);

		var bbox = new Gtk.HButtonBox();
		bbox.layout_style = Gtk.ButtonBoxStyle.EDGE;

		crop_button = new Gtk.Button.with_mnemonic("_Crop");
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
}

Gdk.Pixbuf screenshot() {
	var screen = Gdk.Screen.get_default();
	var shot = Gdk.pixbuf_get_from_drawable(
		null, screen.get_root_window(), null, 0, 0,
		0, 0, screen.get_width(), screen.get_height());
	return shot;
}

int main(string[] args) {
	Gtk.init(ref args);

	var shot = screenshot();
	var win = new SnapzWin(shot);
	win.show();
	Gtk.main();

	return 0;
}
