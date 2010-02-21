using Gdk;
using Gtk;

class ShotCanvas : Gtk.DrawingArea {
	public ShotCanvas(Gdk.Pixbuf pixbuf) {
		this.pixbuf = pixbuf;
		this.pixbuf_scaled = null;

		expose_event.connect(draw);
	}

	public void save(string filename) throws GLib.Error {
		pixbuf.save(filename, "png", null);
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
											Gdk.InterpType.BILINEAR);
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
		return true;
	}

	// Source image.
	private Gdk.Pixbuf pixbuf;
	// Image scaled to current available space.
	private Gdk.Pixbuf? pixbuf_scaled;
}

class SnapzWin : Gtk.Window {
	private ShotCanvas canvas;

	public SnapzWin(Gdk.Pixbuf shot) {
		title = "Snapz";

		var hbox = new Gtk.HBox(false, 12);
		hbox.border_width = 12;

		canvas = new ShotCanvas(shot);
		hbox.pack_start(canvas, true, true, 0);

		var bbox = new Gtk.VButtonBox();
		bbox.layout_style = Gtk.ButtonBoxStyle.START;

		var save_button = new Gtk.Button.with_mnemonic("_Save");
		save_button.clicked.connect(save);
		bbox.add(save_button);

		hbox.pack_start(bbox, false, false, 0);

		add(hbox);
		hbox.show_all();

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
