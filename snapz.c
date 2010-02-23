/* Snapz -- simple screenshot app.*/
/* Copyright (c) 2010 Evan Martin.  All rights reserved.*/

#include <glib.h>
#include <glib-object.h>
#include <cairo.h>
#include <gdk-pixbuf/gdk-pixdata.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>


#define TYPE_SHOT_CANVAS (shot_canvas_get_type ())
#define SHOT_CANVAS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_SHOT_CANVAS, ShotCanvas))
#define SHOT_CANVAS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_SHOT_CANVAS, ShotCanvasClass))
#define IS_SHOT_CANVAS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_SHOT_CANVAS))
#define IS_SHOT_CANVAS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_SHOT_CANVAS))
#define SHOT_CANVAS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_SHOT_CANVAS, ShotCanvasClass))

typedef struct _ShotCanvas ShotCanvas;
typedef struct _ShotCanvasClass ShotCanvasClass;
typedef struct _ShotCanvasPrivate ShotCanvasPrivate;
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _gdk_cursor_unref0(var) ((var == NULL) ? NULL : (var = (gdk_cursor_unref (var), NULL)))
#define _cairo_surface_destroy0(var) ((var == NULL) ? NULL : (var = (cairo_surface_destroy (var), NULL)))
#define _cairo_destroy0(var) ((var == NULL) ? NULL : (var = (cairo_destroy (var), NULL)))

#define TYPE_SNAPZ_WIN (snapz_win_get_type ())
#define SNAPZ_WIN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_SNAPZ_WIN, SnapzWin))
#define SNAPZ_WIN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_SNAPZ_WIN, SnapzWinClass))
#define IS_SNAPZ_WIN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_SNAPZ_WIN))
#define IS_SNAPZ_WIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_SNAPZ_WIN))
#define SNAPZ_WIN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_SNAPZ_WIN, SnapzWinClass))

typedef struct _SnapzWin SnapzWin;
typedef struct _SnapzWinClass SnapzWinClass;
typedef struct _SnapzWinPrivate SnapzWinPrivate;
#define _g_free0(var) (var = (g_free (var), NULL))
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))

struct _ShotCanvas {
	GtkAspectFrame parent_instance;
	ShotCanvasPrivate * priv;
};

struct _ShotCanvasClass {
	GtkAspectFrameClass parent_class;
};

struct _ShotCanvasPrivate {
	GdkPixbuf* pixbuf;
	GdkPixbuf* pixbuf_scaled;
	GtkDrawingArea* canvas;
	GdkPoint sel_start;
	GdkPoint sel_end;
};

struct _SnapzWin {
	GtkWindow parent_instance;
	SnapzWinPrivate * priv;
};

struct _SnapzWinClass {
	GtkWindowClass parent_class;
};

struct _SnapzWinPrivate {
	ShotCanvas* canvas;
	GtkButton** selection_buttons;
	gint selection_buttons_length1;
	gint selection_buttons_size;
};


static gpointer shot_canvas_parent_class = NULL;
static gpointer snapz_win_parent_class = NULL;

GdkPixbuf* pixbuf_from_surface (cairo_surface_t* surface);
gint clamp (gint x, gint min, gint max);
GType shot_canvas_get_type (void);
#define SHOT_CANVAS_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_SHOT_CANVAS, ShotCanvasPrivate))
enum  {
	SHOT_CANVAS_DUMMY_PROPERTY
};
static void _lambda0_ (ShotCanvas* self);
static void __lambda0__gtk_widget_realize (GtkWidget* _sender, gpointer self);
static void shot_canvas_canvas_to_image (ShotCanvas* self, const GdkPoint* p, GdkPoint* result);
static void shot_canvas_clamp_point (ShotCanvas* self, GdkPoint* p);
static gboolean _lambda1_ (const GdkEventButton* event, ShotCanvas* self);
static gboolean __lambda1__gtk_widget_button_press_event (GtkWidget* _sender, const GdkEventButton* event, gpointer self);
static gboolean _lambda2_ (const GdkEventMotion* event, ShotCanvas* self);
static gboolean __lambda2__gtk_widget_motion_notify_event (GtkWidget* _sender, const GdkEventMotion* event, gpointer self);
static gboolean _lambda3_ (const GdkEventButton* event, ShotCanvas* self);
static gboolean __lambda3__gtk_widget_button_release_event (GtkWidget* _sender, const GdkEventButton* event, gpointer self);
static gboolean shot_canvas_draw (ShotCanvas* self, const GdkEventExpose* expose);
static gboolean _shot_canvas_draw_gtk_widget_expose_event (GtkWidget* _sender, const GdkEventExpose* event, gpointer self);
static void _lambda4_ (ShotCanvas* self);
static void __lambda4__shot_canvas_selection_changed (ShotCanvas* _sender, gpointer self);
ShotCanvas* shot_canvas_new (GdkPixbuf* pixbuf);
ShotCanvas* shot_canvas_construct (GType object_type, GdkPixbuf* pixbuf);
void shot_canvas_save (ShotCanvas* self, const char* filename, GError** error);
gboolean shot_canvas_has_selection (ShotCanvas* self);
void shot_canvas_crop (ShotCanvas* self);
static void shot_canvas_draw_mask (ShotCanvas* self, cairo_t* ctx, float alpha, gint width, gint height, float x1, float y1, float x2, float y2);
void shot_canvas_highlight (ShotCanvas* self);
static void shot_canvas_scale_point (ShotCanvas* self, const GdkPoint* p, float mult, GdkPoint* result);
static void shot_canvas_image_to_canvas (ShotCanvas* self, const GdkPoint* p, GdkPoint* result);
static void shot_canvas_rescale_if_necessary (ShotCanvas* self);
static void shot_canvas_finalize (GObject* obj);
GType snapz_win_get_type (void);
#define SNAPZ_WIN_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_SNAPZ_WIN, SnapzWinPrivate))
enum  {
	SNAPZ_WIN_DUMMY_PROPERTY
};
static void _lambda5_ (SnapzWin* self);
static void __lambda5__gtk_button_clicked (GtkButton* _sender, gpointer self);
static void _lambda6_ (SnapzWin* self);
static void __lambda6__gtk_button_clicked (GtkButton* _sender, gpointer self);
static void _lambda7_ (SnapzWin* self);
static void __lambda7__shot_canvas_selection_changed (ShotCanvas* _sender, gpointer self);
static void snapz_win_save (SnapzWin* self);
static void _snapz_win_save_gtk_button_clicked (GtkButton* _sender, gpointer self);
static void _gtk_main_quit_gtk_object_destroy (GtkObject* _sender, gpointer self);
SnapzWin* snapz_win_new (GdkPixbuf* shot);
SnapzWin* snapz_win_construct (GType object_type, GdkPixbuf* shot);
static void snapz_win_finalize (GObject* obj);
GdkPixbuf* screenshot (void);
gint _main (char** args, int args_length1);
static void _vala_array_destroy (gpointer array, gint array_length, GDestroyNotify destroy_func);
static void _vala_array_free (gpointer array, gint array_length, GDestroyNotify destroy_func);



#line 8 "snapz.vala"
GdkPixbuf* pixbuf_from_surface (cairo_surface_t* surface) {
#line 140 "snapz.c"
	GdkPixbuf* result;
	guchar* rgb32;
	gint width;
	gint height;
	gint stride;
	guchar* rgb24;
	guchar* dst;
#line 8 "snapz.vala"
	g_return_val_if_fail (surface != NULL, NULL);
#line 150 "snapz.c"
	rgb32 = cairo_image_surface_get_data (surface);
	width = cairo_image_surface_get_width (surface);
	height = cairo_image_surface_get_height (surface);
	stride = cairo_image_surface_get_stride (surface);
	rgb24 = g_malloc ((gulong) ((width * height) * 3));
	dst = rgb24;
	{
		gint y;
		y = 0;
		{
			gboolean _tmp0_;
			_tmp0_ = TRUE;
#line 18 "snapz.vala"
			while (TRUE) {
#line 165 "snapz.c"
				guint32* src;
#line 18 "snapz.vala"
				if (!_tmp0_) {
#line 18 "snapz.vala"
					y = y + 1;
#line 171 "snapz.c"
				}
#line 18 "snapz.vala"
				_tmp0_ = FALSE;
#line 18 "snapz.vala"
				if (!(y < height)) {
#line 18 "snapz.vala"
					break;
#line 179 "snapz.c"
				}
				src = (guint32*) (rgb32 + (y * stride));
				{
					gint x;
					x = 0;
					{
						gboolean _tmp1_;
						_tmp1_ = TRUE;
#line 20 "snapz.vala"
						while (TRUE) {
#line 20 "snapz.vala"
							if (!_tmp1_) {
#line 20 "snapz.vala"
								x = x + 1;
#line 194 "snapz.c"
							}
#line 20 "snapz.vala"
							_tmp1_ = FALSE;
#line 20 "snapz.vala"
							if (!(x < width)) {
#line 20 "snapz.vala"
								break;
#line 202 "snapz.c"
							}
#line 21 "snapz.vala"
							*(dst++) = (guchar) (src[x] >> 16);
#line 22 "snapz.vala"
							*(dst++) = (guchar) (src[x] >> 8);
#line 23 "snapz.vala"
							*(dst++) = (guchar) src[x];
#line 210 "snapz.c"
						}
					}
				}
			}
		}
	}
	result = gdk_pixbuf_new_from_data ((guchar*) rgb24, GDK_COLORSPACE_RGB, FALSE, 8, width, height, width * 3, (GdkPixbufDestroyNotify) g_free, NULL);
	return result;
}


#line 33 "snapz.vala"
gint clamp (gint x, gint min, gint max) {
#line 224 "snapz.c"
	gint result;
#line 34 "snapz.vala"
	if (x < min) {
#line 228 "snapz.c"
		result = min;
		return result;
	}
#line 36 "snapz.vala"
	if (x >= max) {
#line 234 "snapz.c"
		result = max - 1;
		return result;
	}
	result = x;
	return result;
}


static gpointer _g_object_ref0 (gpointer self) {
	return self ? g_object_ref (self) : NULL;
}


static void _lambda0_ (ShotCanvas* self) {
	GdkCursor* _tmp0_;
#line 60 "snapz.vala"
	gdk_window_set_cursor (((GtkWidget*) self->priv->canvas)->window, _tmp0_ = gdk_cursor_new (GDK_CROSSHAIR));
#line 252 "snapz.c"
	_gdk_cursor_unref0 (_tmp0_);
}


static void __lambda0__gtk_widget_realize (GtkWidget* _sender, gpointer self) {
	_lambda0_ (self);
}


static gboolean _lambda1_ (const GdkEventButton* event, ShotCanvas* self) {
	gboolean result;
	GdkPoint _tmp2_ = {0};
	GdkPoint _tmp1_;
	GdkPoint _tmp0_ = {0};
#line 65 "snapz.vala"
	self->priv->sel_start = (shot_canvas_canvas_to_image (self, (_tmp1_ = (memset (&_tmp0_, 0, sizeof (GdkPoint)), _tmp0_.x = (gint) (*event).x, _tmp0_.y = (gint) (*event).y, _tmp0_), &_tmp1_), &_tmp2_), _tmp2_);
#line 68 "snapz.vala"
	shot_canvas_clamp_point (self, &self->priv->sel_start);
#line 69 "snapz.vala"
	self->priv->sel_end = self->priv->sel_start;
#line 70 "snapz.vala"
	g_signal_emit_by_name (self, "selection-changed");
#line 275 "snapz.c"
}


static gboolean __lambda1__gtk_widget_button_press_event (GtkWidget* _sender, const GdkEventButton* event, gpointer self) {
	return _lambda1_ (event, self);
}


static gboolean _lambda2_ (const GdkEventMotion* event, ShotCanvas* self) {
	gboolean result;
	GdkPoint _tmp2_ = {0};
	GdkPoint _tmp1_;
	GdkPoint _tmp0_ = {0};
#line 73 "snapz.vala"
	self->priv->sel_end = (shot_canvas_canvas_to_image (self, (_tmp1_ = (memset (&_tmp0_, 0, sizeof (GdkPoint)), _tmp0_.x = (gint) (*event).x, _tmp0_.y = (gint) (*event).y, _tmp0_), &_tmp1_), &_tmp2_), _tmp2_);
#line 76 "snapz.vala"
	shot_canvas_clamp_point (self, &self->priv->sel_end);
#line 77 "snapz.vala"
	g_signal_emit_by_name (self, "selection-changed");
#line 295 "snapz.c"
}


static gboolean __lambda2__gtk_widget_motion_notify_event (GtkWidget* _sender, const GdkEventMotion* event, gpointer self) {
	return _lambda2_ (event, self);
}


static gboolean _lambda3_ (const GdkEventButton* event, ShotCanvas* self) {
	gboolean result;
	GdkPoint _tmp2_ = {0};
	GdkPoint _tmp1_;
	GdkPoint _tmp0_ = {0};
	gboolean _tmp3_;
#line 80 "snapz.vala"
	self->priv->sel_end = (shot_canvas_canvas_to_image (self, (_tmp1_ = (memset (&_tmp0_, 0, sizeof (GdkPoint)), _tmp0_.x = (gint) (*event).x, _tmp0_.y = (gint) (*event).y, _tmp0_), &_tmp1_), &_tmp2_), _tmp2_);
#line 83 "snapz.vala"
	shot_canvas_clamp_point (self, &self->priv->sel_end);
#line 314 "snapz.c"
	_tmp3_ = FALSE;
#line 84 "snapz.vala"
	if (self->priv->sel_end.x == self->priv->sel_start.x) {
#line 84 "snapz.vala"
		_tmp3_ = self->priv->sel_end.y == self->priv->sel_start.y;
#line 320 "snapz.c"
	} else {
#line 84 "snapz.vala"
		_tmp3_ = FALSE;
#line 324 "snapz.c"
	}
#line 84 "snapz.vala"
	if (_tmp3_) {
#line 85 "snapz.vala"
		self->priv->sel_start.x = -1;
#line 330 "snapz.c"
	} else {
#line 87 "snapz.vala"
		if (self->priv->sel_end.x < self->priv->sel_start.x) {
#line 334 "snapz.c"
			gint t;
			t = self->priv->sel_start.x;
#line 89 "snapz.vala"
			self->priv->sel_start.x = self->priv->sel_end.x;
#line 90 "snapz.vala"
			self->priv->sel_end.x = t;
#line 341 "snapz.c"
		}
#line 92 "snapz.vala"
		if (self->priv->sel_end.y < self->priv->sel_start.y) {
#line 345 "snapz.c"
			gint t;
			t = self->priv->sel_start.y;
#line 94 "snapz.vala"
			self->priv->sel_start.y = self->priv->sel_end.y;
#line 95 "snapz.vala"
			self->priv->sel_end.y = t;
#line 352 "snapz.c"
		}
	}
#line 98 "snapz.vala"
	g_signal_emit_by_name (self, "selection-changed");
#line 357 "snapz.c"
}


static gboolean __lambda3__gtk_widget_button_release_event (GtkWidget* _sender, const GdkEventButton* event, gpointer self) {
	return _lambda3_ (event, self);
}


#line 207 "snapz.vala"
static gboolean _shot_canvas_draw_gtk_widget_expose_event (GtkWidget* _sender, const GdkEventExpose* event, gpointer self) {
#line 368 "snapz.c"
	return shot_canvas_draw (self, event);
}


static void _lambda4_ (ShotCanvas* self) {
#line 102 "snapz.vala"
	gtk_widget_queue_draw ((GtkWidget*) self);
#line 376 "snapz.c"
}


static void __lambda4__shot_canvas_selection_changed (ShotCanvas* _sender, gpointer self) {
	_lambda4_ (self);
}


#line 46 "snapz.vala"
ShotCanvas* shot_canvas_construct (GType object_type, GdkPixbuf* pixbuf) {
#line 387 "snapz.c"
	ShotCanvas * self;
	GdkPixbuf* _tmp0_;
	GdkPixbuf* _tmp1_;
	GtkDrawingArea* _tmp2_;
#line 46 "snapz.vala"
	g_return_val_if_fail (pixbuf != NULL, NULL);
#line 394 "snapz.c"
	self = g_object_newv (object_type, 0, NULL);
#line 47 "snapz.vala"
	g_object_set ((GtkAspectFrame*) self, "obey-child", FALSE, NULL);
#line 48 "snapz.vala"
	gtk_frame_set_shadow_type ((GtkFrame*) self, GTK_SHADOW_NONE);
#line 50 "snapz.vala"
	self->priv->pixbuf = (_tmp0_ = _g_object_ref0 (pixbuf), _g_object_unref0 (self->priv->pixbuf), _tmp0_);
#line 51 "snapz.vala"
	self->priv->pixbuf_scaled = (_tmp1_ = NULL, _g_object_unref0 (self->priv->pixbuf_scaled), _tmp1_);
#line 52 "snapz.vala"
	g_object_set ((GtkAspectFrame*) self, "ratio", gdk_pixbuf_get_width (pixbuf) / ((float) gdk_pixbuf_get_height (pixbuf)), NULL);
#line 53 "snapz.vala"
	self->priv->sel_start.x = -1;
#line 55 "snapz.vala"
	self->priv->canvas = (_tmp2_ = g_object_ref_sink ((GtkDrawingArea*) gtk_drawing_area_new ()), _g_object_unref0 (self->priv->canvas), _tmp2_);
#line 56 "snapz.vala"
	gtk_widget_add_events ((GtkWidget*) self->priv->canvas, (gint) ((GDK_BUTTON_PRESS_MASK | GDK_BUTTON1_MOTION_MASK) | GDK_BUTTON_RELEASE_MASK));
#line 59 "snapz.vala"
	g_signal_connect_object ((GtkWidget*) self->priv->canvas, "realize", (GCallback) __lambda0__gtk_widget_realize, self, 0);
#line 64 "snapz.vala"
	g_signal_connect_object ((GtkWidget*) self->priv->canvas, "button-press-event", (GCallback) __lambda1__gtk_widget_button_press_event, self, 0);
#line 72 "snapz.vala"
	g_signal_connect_object ((GtkWidget*) self->priv->canvas, "motion-notify-event", (GCallback) __lambda2__gtk_widget_motion_notify_event, self, 0);
#line 79 "snapz.vala"
	g_signal_connect_object ((GtkWidget*) self->priv->canvas, "button-release-event", (GCallback) __lambda3__gtk_widget_button_release_event, self, 0);
#line 101 "snapz.vala"
	g_signal_connect_object ((GtkWidget*) self->priv->canvas, "expose-event", (GCallback) _shot_canvas_draw_gtk_widget_expose_event, self, 0);
#line 102 "snapz.vala"
	g_signal_connect_object (self, "selection-changed", (GCallback) __lambda4__shot_canvas_selection_changed, self, 0);
#line 103 "snapz.vala"
	gtk_container_add ((GtkContainer*) self, (GtkWidget*) self->priv->canvas);
#line 426 "snapz.c"
	return self;
}


#line 46 "snapz.vala"
ShotCanvas* shot_canvas_new (GdkPixbuf* pixbuf) {
#line 46 "snapz.vala"
	return shot_canvas_construct (TYPE_SHOT_CANVAS, pixbuf);
#line 435 "snapz.c"
}


#line 107 "snapz.vala"
void shot_canvas_save (ShotCanvas* self, const char* filename, GError** error) {
#line 441 "snapz.c"
	GError * _inner_error_;
#line 107 "snapz.vala"
	g_return_if_fail (self != NULL);
#line 107 "snapz.vala"
	g_return_if_fail (filename != NULL);
#line 447 "snapz.c"
	_inner_error_ = NULL;
#line 108 "snapz.vala"
	gdk_pixbuf_save (self->priv->pixbuf, filename, "png", &_inner_error_, NULL, NULL);
#line 451 "snapz.c"
	if (_inner_error_ != NULL) {
		g_propagate_error (error, _inner_error_);
		return;
	}
}


#line 112 "snapz.vala"
gboolean shot_canvas_has_selection (ShotCanvas* self) {
#line 461 "snapz.c"
	gboolean result;
#line 112 "snapz.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 465 "snapz.c"
	result = self->priv->sel_start.x != (-1);
	return result;
}


#line 120 "snapz.vala"
void shot_canvas_crop (ShotCanvas* self) {
#line 473 "snapz.c"
	GdkPixbuf* _tmp0_;
	GdkPixbuf* _tmp1_;
#line 120 "snapz.vala"
	g_return_if_fail (self != NULL);
#line 121 "snapz.vala"
	self->priv->pixbuf = (_tmp0_ = gdk_pixbuf_new_subpixbuf (self->priv->pixbuf, self->priv->sel_start.x, self->priv->sel_start.y, self->priv->sel_end.x - self->priv->sel_start.x, self->priv->sel_end.y - self->priv->sel_start.y), _g_object_unref0 (self->priv->pixbuf), _tmp0_);
#line 124 "snapz.vala"
	self->priv->pixbuf_scaled = (_tmp1_ = NULL, _g_object_unref0 (self->priv->pixbuf_scaled), _tmp1_);
#line 125 "snapz.vala"
	self->priv->sel_start.x = -1;
#line 126 "snapz.vala"
	g_signal_emit_by_name (self, "selection-changed");
#line 127 "snapz.vala"
	g_object_set ((GtkAspectFrame*) self, "ratio", gdk_pixbuf_get_width (self->priv->pixbuf) / ((float) gdk_pixbuf_get_height (self->priv->pixbuf)), NULL);
#line 488 "snapz.c"
}


#line 131 "snapz.vala"
void shot_canvas_highlight (ShotCanvas* self) {
#line 494 "snapz.c"
	gint width;
	gint height;
	cairo_surface_t* surface;
	cairo_t* ctx;
	GdkPixbuf* _tmp0_;
	GdkPixbuf* _tmp1_;
#line 131 "snapz.vala"
	g_return_if_fail (self != NULL);
#line 503 "snapz.c"
	width = gdk_pixbuf_get_width (self->priv->pixbuf);
	height = gdk_pixbuf_get_height (self->priv->pixbuf);
	surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, width, height);
	ctx = cairo_create (surface);
#line 137 "snapz.vala"
	gdk_cairo_set_source_pixbuf (ctx, self->priv->pixbuf, (double) 0, (double) 0);
#line 138 "snapz.vala"
	cairo_paint (ctx);
#line 140 "snapz.vala"
	shot_canvas_draw_mask (self, ctx, 0.2f, width, height, (float) self->priv->sel_start.x, (float) self->priv->sel_start.y, (float) self->priv->sel_end.x, (float) self->priv->sel_end.y);
#line 143 "snapz.vala"
	self->priv->pixbuf = (_tmp0_ = pixbuf_from_surface (surface), _g_object_unref0 (self->priv->pixbuf), _tmp0_);
#line 144 "snapz.vala"
	self->priv->pixbuf_scaled = (_tmp1_ = NULL, _g_object_unref0 (self->priv->pixbuf_scaled), _tmp1_);
#line 145 "snapz.vala"
	self->priv->sel_start.x = -1;
#line 146 "snapz.vala"
	g_signal_emit_by_name (self, "selection-changed");
#line 522 "snapz.c"
	_cairo_surface_destroy0 (surface);
	_cairo_destroy0 (ctx);
}


#line 150 "snapz.vala"
static void shot_canvas_clamp_point (ShotCanvas* self, GdkPoint* p) {
#line 150 "snapz.vala"
	g_return_if_fail (self != NULL);
#line 151 "snapz.vala"
	(*p).x = clamp ((*p).x, 0, gdk_pixbuf_get_width (self->priv->pixbuf));
#line 152 "snapz.vala"
	(*p).y = clamp ((*p).y, 0, gdk_pixbuf_get_height (self->priv->pixbuf));
#line 536 "snapz.c"
}


#line 156 "snapz.vala"
static void shot_canvas_scale_point (ShotCanvas* self, const GdkPoint* p, float mult, GdkPoint* result) {
#line 542 "snapz.c"
	GdkPoint _tmp0_ = {0};
#line 156 "snapz.vala"
	g_return_if_fail (self != NULL);
#line 546 "snapz.c"
	*result = (memset (&_tmp0_, 0, sizeof (GdkPoint)), _tmp0_.x = (gint) round ((double) ((*p).x * mult)), _tmp0_.y = (gint) round ((double) ((*p).y * mult)), _tmp0_);
	return;
}


#line 164 "snapz.vala"
static void shot_canvas_canvas_to_image (ShotCanvas* self, const GdkPoint* p, GdkPoint* result) {
#line 554 "snapz.c"
	GdkPoint _tmp0_ = {0};
#line 164 "snapz.vala"
	g_return_if_fail (self != NULL);
#line 558 "snapz.c"
	*result = (shot_canvas_scale_point (self, &(*p), gdk_pixbuf_get_width (self->priv->pixbuf) / ((float) ((GtkWidget*) self->priv->canvas)->allocation.width), &_tmp0_), _tmp0_);
	return;
}


#line 169 "snapz.vala"
static void shot_canvas_image_to_canvas (ShotCanvas* self, const GdkPoint* p, GdkPoint* result) {
#line 566 "snapz.c"
	GdkPoint _tmp0_ = {0};
#line 169 "snapz.vala"
	g_return_if_fail (self != NULL);
#line 570 "snapz.c"
	*result = (shot_canvas_scale_point (self, &(*p), ((GtkWidget*) self->priv->canvas)->allocation.width / ((float) gdk_pixbuf_get_width (self->priv->pixbuf)), &_tmp0_), _tmp0_);
	return;
}


#line 175 "snapz.vala"
static void shot_canvas_rescale_if_necessary (ShotCanvas* self) {
#line 578 "snapz.c"
	gint width;
	gint height;
	gboolean _tmp0_;
	gboolean _tmp1_;
	GdkPixbuf* _tmp2_;
#line 175 "snapz.vala"
	g_return_if_fail (self != NULL);
#line 586 "snapz.c"
	width = ((GtkWidget*) self->priv->canvas)->allocation.width;
	height = ((GtkWidget*) self->priv->canvas)->allocation.height;
	_tmp0_ = FALSE;
	_tmp1_ = FALSE;
#line 179 "snapz.vala"
	if (self->priv->pixbuf_scaled != NULL) {
#line 180 "snapz.vala"
		_tmp1_ = gdk_pixbuf_get_width (self->priv->pixbuf_scaled) == width;
#line 595 "snapz.c"
	} else {
#line 179 "snapz.vala"
		_tmp1_ = FALSE;
#line 599 "snapz.c"
	}
#line 179 "snapz.vala"
	if (_tmp1_) {
#line 181 "snapz.vala"
		_tmp0_ = gdk_pixbuf_get_height (self->priv->pixbuf_scaled) == height;
#line 605 "snapz.c"
	} else {
#line 179 "snapz.vala"
		_tmp0_ = FALSE;
#line 609 "snapz.c"
	}
#line 179 "snapz.vala"
	if (_tmp0_) {
#line 613 "snapz.c"
		return;
	}
#line 185 "snapz.vala"
	self->priv->pixbuf_scaled = (_tmp2_ = gdk_pixbuf_scale_simple (self->priv->pixbuf, width, height, GDK_INTERP_TILES), _g_object_unref0 (self->priv->pixbuf_scaled), _tmp2_);
#line 618 "snapz.c"
}


#line 190 "snapz.vala"
static void shot_canvas_draw_mask (ShotCanvas* self, cairo_t* ctx, float alpha, gint width, gint height, float x1, float y1, float x2, float y2) {
#line 624 "snapz.c"
	cairo_surface_t* mask;
	cairo_t* mask_ctx;
#line 190 "snapz.vala"
	g_return_if_fail (self != NULL);
#line 190 "snapz.vala"
	g_return_if_fail (ctx != NULL);
#line 631 "snapz.c"
	mask = cairo_surface_create_similar (cairo_get_target (ctx), CAIRO_CONTENT_ALPHA, width, height);
	mask_ctx = cairo_create (mask);
#line 197 "snapz.vala"
	cairo_set_source_rgba (mask_ctx, (double) 0, (double) 0, (double) 0, (double) alpha);
#line 198 "snapz.vala"
	cairo_paint (mask_ctx);
#line 199 "snapz.vala"
	cairo_set_operator (mask_ctx, CAIRO_OPERATOR_CLEAR);
#line 200 "snapz.vala"
	cairo_rectangle (mask_ctx, (double) x1, (double) y1, (double) (x2 - x1), (double) (y2 - y1));
#line 201 "snapz.vala"
	cairo_fill (mask_ctx);
#line 202 "snapz.vala"
	cairo_set_source_surface (ctx, mask, (double) 0, (double) 0);
#line 203 "snapz.vala"
	cairo_paint (ctx);
#line 648 "snapz.c"
	_cairo_surface_destroy0 (mask);
	_cairo_destroy0 (mask_ctx);
}


#line 207 "snapz.vala"
static gboolean shot_canvas_draw (ShotCanvas* self, const GdkEventExpose* expose) {
#line 656 "snapz.c"
	gboolean result;
	gboolean _tmp0_;
	cairo_t* ctx;
#line 207 "snapz.vala"
	g_return_val_if_fail (self != NULL, FALSE);
#line 662 "snapz.c"
	_tmp0_ = FALSE;
#line 208 "snapz.vala"
	if (((GtkWidget*) self->priv->canvas)->allocation.width == 1) {
#line 208 "snapz.vala"
		_tmp0_ = TRUE;
#line 668 "snapz.c"
	} else {
#line 208 "snapz.vala"
		_tmp0_ = ((GtkWidget*) self->priv->canvas)->allocation.height == 1;
#line 672 "snapz.c"
	}
#line 208 "snapz.vala"
	if (_tmp0_) {
#line 676 "snapz.c"
		result = FALSE;
		return result;
	}
#line 214 "snapz.vala"
	shot_canvas_rescale_if_necessary (self);
#line 682 "snapz.c"
	ctx = gdk_cairo_create ((GdkDrawable*) ((GtkWidget*) self->priv->canvas)->window);
#line 218 "snapz.vala"
	gdk_cairo_set_source_pixbuf (ctx, self->priv->pixbuf_scaled, (double) 0, (double) 0);
#line 219 "snapz.vala"
	cairo_paint (ctx);
#line 221 "snapz.vala"
	if (shot_canvas_has_selection (self)) {
#line 690 "snapz.c"
		GdkPoint _tmp1_ = {0};
		GdkPoint start;
		GdkPoint _tmp2_ = {0};
		GdkPoint end;
		gint _tmp4__length1;
		double* _tmp4_;
		double* _tmp3_;
		start = (shot_canvas_image_to_canvas (self, &self->priv->sel_start, &_tmp1_), _tmp1_);
		end = (shot_canvas_image_to_canvas (self, &self->priv->sel_end, &_tmp2_), _tmp2_);
#line 226 "snapz.vala"
		shot_canvas_draw_mask (self, ctx, 0.1f, ((GtkWidget*) self->priv->canvas)->allocation.width, ((GtkWidget*) self->priv->canvas)->allocation.height, (float) start.x, (float) start.y, (float) end.x, (float) end.y);
#line 231 "snapz.vala"
		cairo_translate (ctx, 0.5, 0.5);
#line 232 "snapz.vala"
		cairo_set_line_width (ctx, (double) 1);
#line 234 "snapz.vala"
		cairo_set_source_rgb (ctx, (double) 0, (double) 0, (double) 0);
#line 235 "snapz.vala"
		cairo_rectangle (ctx, (double) start.x, (double) start.y, (double) (end.x - start.x), (double) (end.y - start.y));
#line 237 "snapz.vala"
		cairo_stroke (ctx);
#line 239 "snapz.vala"
		cairo_set_source_rgb (ctx, (double) 1, (double) 1, (double) 1);
#line 240 "snapz.vala"
		_tmp3_ = NULL;
#line 240 "snapz.vala"
		cairo_set_dash (ctx, (_tmp4_ = (_tmp3_ = g_new0 (double, 2), _tmp3_[0] = (double) 5, _tmp3_[1] = (double) 5, _tmp3_), _tmp4__length1 = 2, _tmp4_), 2, (double) 0);
#line 718 "snapz.c"
		_tmp4_ = (g_free (_tmp4_), NULL);
#line 241 "snapz.vala"
		cairo_rectangle (ctx, (double) start.x, (double) start.y, (double) (end.x - start.x), (double) (end.y - start.y));
#line 243 "snapz.vala"
		cairo_stroke (ctx);
#line 724 "snapz.c"
	}
	result = TRUE;
	_cairo_destroy0 (ctx);
	return result;
}


static void shot_canvas_class_init (ShotCanvasClass * klass) {
	shot_canvas_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (ShotCanvasPrivate));
	G_OBJECT_CLASS (klass)->finalize = shot_canvas_finalize;
	g_signal_new ("selection_changed", TYPE_SHOT_CANVAS, G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void shot_canvas_instance_init (ShotCanvas * self) {
	self->priv = SHOT_CANVAS_GET_PRIVATE (self);
}


static void shot_canvas_finalize (GObject* obj) {
	ShotCanvas * self;
	self = SHOT_CANVAS (obj);
	_g_object_unref0 (self->priv->pixbuf);
	_g_object_unref0 (self->priv->pixbuf_scaled);
	_g_object_unref0 (self->priv->canvas);
	G_OBJECT_CLASS (shot_canvas_parent_class)->finalize (obj);
}


GType shot_canvas_get_type (void) {
	static GType shot_canvas_type_id = 0;
	if (shot_canvas_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (ShotCanvasClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) shot_canvas_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (ShotCanvas), 0, (GInstanceInitFunc) shot_canvas_instance_init, NULL };
		shot_canvas_type_id = g_type_register_static (GTK_TYPE_ASPECT_FRAME, "ShotCanvas", &g_define_type_info, 0);
	}
	return shot_canvas_type_id;
}


static void _lambda5_ (SnapzWin* self) {
#line 280 "snapz.vala"
	shot_canvas_crop (self->priv->canvas);
#line 768 "snapz.c"
}


static void __lambda5__gtk_button_clicked (GtkButton* _sender, gpointer self) {
	_lambda5_ (self);
}


static void _lambda6_ (SnapzWin* self) {
#line 286 "snapz.vala"
	shot_canvas_highlight (self->priv->canvas);
#line 780 "snapz.c"
}


static void __lambda6__gtk_button_clicked (GtkButton* _sender, gpointer self) {
	_lambda6_ (self);
}


static void _lambda7_ (SnapzWin* self) {
	{
		GtkButton** button_collection;
		int button_collection_length1;
		int button_it;
#line 295 "snapz.vala"
		button_collection = self->priv->selection_buttons;
#line 796 "snapz.c"
		button_collection_length1 = self->priv->selection_buttons_length1;
		for (button_it = 0; button_it < self->priv->selection_buttons_length1; button_it = button_it + 1) {
			GtkButton* button;
			button = _g_object_ref0 (button_collection[button_it]);
			{
#line 296 "snapz.vala"
				gtk_widget_set_sensitive ((GtkWidget*) button, shot_canvas_has_selection (self->priv->canvas));
#line 804 "snapz.c"
				_g_object_unref0 (button);
			}
		}
	}
}


static void __lambda7__shot_canvas_selection_changed (ShotCanvas* _sender, gpointer self) {
	_lambda7_ (self);
}


#line 312 "snapz.vala"
static void _snapz_win_save_gtk_button_clicked (GtkButton* _sender, gpointer self) {
#line 819 "snapz.c"
	snapz_win_save (self);
}


#line 7481 "gtk+-2.0.vapi"
static void _gtk_main_quit_gtk_object_destroy (GtkObject* _sender, gpointer self) {
#line 826 "snapz.c"
	gtk_main_quit ();
}


#line 265 "snapz.vala"
SnapzWin* snapz_win_construct (GType object_type, GdkPixbuf* shot) {
#line 833 "snapz.c"
	SnapzWin * self;
	GtkVBox* vbox;
	ShotCanvas* _tmp0_;
	GtkHButtonBox* bbox;
	GtkButton* crop_button;
	GtkButton* highlight_button;
	GtkButton** _tmp2_;
	GtkButton** _tmp1_;
	GtkButton* save_button;
#line 265 "snapz.vala"
	g_return_val_if_fail (shot != NULL, NULL);
#line 845 "snapz.c"
	self = g_object_newv (object_type, 0, NULL);
#line 266 "snapz.vala"
	gtk_window_set_title ((GtkWindow*) self, "Snapz");
#line 267 "snapz.vala"
	gtk_window_set_default_size ((GtkWindow*) self, gdk_pixbuf_get_width (shot) / 2, gdk_pixbuf_get_height (shot) / 2);
#line 851 "snapz.c"
	vbox = g_object_ref_sink ((GtkVBox*) gtk_vbox_new (FALSE, 12));
#line 270 "snapz.vala"
	gtk_container_set_border_width ((GtkContainer*) vbox, (guint) 12);
#line 272 "snapz.vala"
	self->priv->canvas = (_tmp0_ = g_object_ref_sink (shot_canvas_new (shot)), _g_object_unref0 (self->priv->canvas), _tmp0_);
#line 273 "snapz.vala"
	gtk_box_pack_start ((GtkBox*) vbox, (GtkWidget*) self->priv->canvas, TRUE, TRUE, (guint) 0);
#line 859 "snapz.c"
	bbox = g_object_ref_sink ((GtkHButtonBox*) gtk_hbutton_box_new ());
#line 276 "snapz.vala"
	g_object_set ((GtkButtonBox*) bbox, "layout-style", GTK_BUTTONBOX_START, NULL);
#line 863 "snapz.c"
	crop_button = g_object_ref_sink ((GtkButton*) gtk_button_new_with_mnemonic ("_Crop"));
#line 279 "snapz.vala"
	g_signal_connect_object (crop_button, "clicked", (GCallback) __lambda5__gtk_button_clicked, self, 0);
#line 282 "snapz.vala"
	gtk_box_pack_start ((GtkBox*) bbox, (GtkWidget*) crop_button, FALSE, FALSE, (guint) 0);
#line 869 "snapz.c"
	highlight_button = g_object_ref_sink ((GtkButton*) gtk_button_new_with_mnemonic ("_Highlight"));
#line 285 "snapz.vala"
	g_signal_connect_object (highlight_button, "clicked", (GCallback) __lambda6__gtk_button_clicked, self, 0);
#line 288 "snapz.vala"
	gtk_box_pack_start ((GtkBox*) bbox, (GtkWidget*) highlight_button, FALSE, FALSE, (guint) 0);
#line 290 "snapz.vala"
	_tmp1_ = NULL;
#line 290 "snapz.vala"
	self->priv->selection_buttons = (_tmp2_ = (_tmp1_ = g_new0 (GtkButton*, 2 + 1), _tmp1_[0] = _g_object_ref0 (crop_button), _tmp1_[1] = _g_object_ref0 (highlight_button), _tmp1_), self->priv->selection_buttons = (_vala_array_free (self->priv->selection_buttons, self->priv->selection_buttons_length1, (GDestroyNotify) g_object_unref), NULL), self->priv->selection_buttons_length1 = 2, self->priv->selection_buttons_size = self->priv->selection_buttons_length1, _tmp2_);
#line 879 "snapz.c"
	{
		GtkButton** button_collection;
		int button_collection_length1;
		int button_it;
#line 291 "snapz.vala"
		button_collection = self->priv->selection_buttons;
#line 886 "snapz.c"
		button_collection_length1 = self->priv->selection_buttons_length1;
		for (button_it = 0; button_it < self->priv->selection_buttons_length1; button_it = button_it + 1) {
			GtkButton* button;
			button = _g_object_ref0 (button_collection[button_it]);
			{
#line 292 "snapz.vala"
				gtk_widget_set_sensitive ((GtkWidget*) button, FALSE);
#line 894 "snapz.c"
				_g_object_unref0 (button);
			}
		}
	}
#line 294 "snapz.vala"
	g_signal_connect_object (self->priv->canvas, "selection-changed", (GCallback) __lambda7__shot_canvas_selection_changed, self, 0);
#line 901 "snapz.c"
	save_button = g_object_ref_sink ((GtkButton*) gtk_button_new_with_mnemonic ("_Save"));
#line 300 "snapz.vala"
	g_signal_connect_object (save_button, "clicked", (GCallback) _snapz_win_save_gtk_button_clicked, self, 0);
#line 301 "snapz.vala"
	gtk_box_pack_end ((GtkBox*) bbox, (GtkWidget*) save_button, FALSE, FALSE, (guint) 0);
#line 302 "snapz.vala"
	gtk_button_box_set_child_secondary ((GtkButtonBox*) bbox, (GtkWidget*) save_button, TRUE);
#line 304 "snapz.vala"
	gtk_box_pack_start ((GtkBox*) vbox, (GtkWidget*) bbox, FALSE, FALSE, (guint) 0);
#line 306 "snapz.vala"
	gtk_container_add ((GtkContainer*) self, (GtkWidget*) vbox);
#line 307 "snapz.vala"
	gtk_widget_show_all ((GtkWidget*) vbox);
#line 309 "snapz.vala"
	g_signal_connect ((GtkObject*) self, "destroy", (GCallback) _gtk_main_quit_gtk_object_destroy, NULL);
#line 917 "snapz.c"
	_g_object_unref0 (vbox);
	_g_object_unref0 (bbox);
	_g_object_unref0 (crop_button);
	_g_object_unref0 (highlight_button);
	_g_object_unref0 (save_button);
	return self;
}


#line 265 "snapz.vala"
SnapzWin* snapz_win_new (GdkPixbuf* shot) {
#line 265 "snapz.vala"
	return snapz_win_construct (TYPE_SNAPZ_WIN, shot);
#line 931 "snapz.c"
}


#line 312 "snapz.vala"
static void snapz_win_save (SnapzWin* self) {
#line 937 "snapz.c"
	GError * _inner_error_;
	GtkFileChooserDialog* dialog;
	gint response;
#line 312 "snapz.vala"
	g_return_if_fail (self != NULL);
#line 943 "snapz.c"
	_inner_error_ = NULL;
	dialog = g_object_ref_sink ((GtkFileChooserDialog*) gtk_file_chooser_dialog_new (NULL, (GtkWindow*) self, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL, NULL));
#line 318 "snapz.vala"
	gtk_file_chooser_set_current_name ((GtkFileChooser*) dialog, "Screenshot.png");
#line 948 "snapz.c"
	response = gtk_dialog_run ((GtkDialog*) dialog);
#line 320 "snapz.vala"
	if (response == GTK_RESPONSE_OK) {
#line 952 "snapz.c"
		{
			char* _tmp0_;
#line 322 "snapz.vala"
			shot_canvas_save (self->priv->canvas, _tmp0_ = gtk_file_chooser_get_filename ((GtkFileChooser*) dialog), &_inner_error_);
#line 957 "snapz.c"
			if (_inner_error_ != NULL) {
				goto __catch0_g_error;
				goto __finally0;
			}
			_g_free0 (_tmp0_);
		}
		goto __finally0;
		__catch0_g_error:
		{
			GError * e;
			e = _inner_error_;
			_inner_error_ = NULL;
			{
				_g_error_free0 (e);
			}
		}
		__finally0:
		if (_inner_error_ != NULL) {
			_g_object_unref0 (dialog);
			g_critical ("file %s: line %d: uncaught error: %s", __FILE__, __LINE__, _inner_error_->message);
			g_clear_error (&_inner_error_);
			return;
		}
	}
#line 327 "snapz.vala"
	gtk_object_destroy ((GtkObject*) dialog);
#line 984 "snapz.c"
	_g_object_unref0 (dialog);
}


static void snapz_win_class_init (SnapzWinClass * klass) {
	snapz_win_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (SnapzWinPrivate));
	G_OBJECT_CLASS (klass)->finalize = snapz_win_finalize;
}


static void snapz_win_instance_init (SnapzWin * self) {
	self->priv = SNAPZ_WIN_GET_PRIVATE (self);
}


static void snapz_win_finalize (GObject* obj) {
	SnapzWin * self;
	self = SNAPZ_WIN (obj);
	_g_object_unref0 (self->priv->canvas);
	self->priv->selection_buttons = (_vala_array_free (self->priv->selection_buttons, self->priv->selection_buttons_length1, (GDestroyNotify) g_object_unref), NULL);
	G_OBJECT_CLASS (snapz_win_parent_class)->finalize (obj);
}


GType snapz_win_get_type (void) {
	static GType snapz_win_type_id = 0;
	if (snapz_win_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (SnapzWinClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) snapz_win_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (SnapzWin), 0, (GInstanceInitFunc) snapz_win_instance_init, NULL };
		snapz_win_type_id = g_type_register_static (GTK_TYPE_WINDOW, "SnapzWin", &g_define_type_info, 0);
	}
	return snapz_win_type_id;
}


#line 338 "snapz.vala"
GdkPixbuf* screenshot (void) {
#line 1022 "snapz.c"
	GdkPixbuf* result;
	GdkScreen* screen;
	screen = _g_object_ref0 (gdk_screen_get_default ());
	result = _g_object_ref0 (gdk_pixbuf_get_from_drawable (NULL, (GdkDrawable*) gdk_screen_get_root_window (screen), NULL, 0, 0, 0, 0, gdk_screen_get_width (screen), gdk_screen_get_height (screen)));
	_g_object_unref0 (screen);
	return result;
}


#line 345 "snapz.vala"
gint _main (char** args, int args_length1) {
#line 1034 "snapz.c"
	gint result;
	GdkPixbuf* shot;
	SnapzWin* win;
#line 346 "snapz.vala"
	gtk_init (&args_length1, &args);
#line 1040 "snapz.c"
	shot = screenshot ();
	win = g_object_ref_sink (snapz_win_new (shot));
#line 350 "snapz.vala"
	gtk_widget_show ((GtkWidget*) win);
#line 351 "snapz.vala"
	gtk_main ();
#line 1047 "snapz.c"
	result = 0;
	_g_object_unref0 (shot);
	_g_object_unref0 (win);
	return result;
}


#line 345 "snapz.vala"
int main (int argc, char ** argv) {
#line 345 "snapz.vala"
	g_type_init ();
#line 345 "snapz.vala"
	return _main (argv, argc);
#line 1061 "snapz.c"
}


static void _vala_array_destroy (gpointer array, gint array_length, GDestroyNotify destroy_func) {
	if ((array != NULL) && (destroy_func != NULL)) {
		int i;
		for (i = 0; i < array_length; i = i + 1) {
			if (((gpointer*) array)[i] != NULL) {
				destroy_func (((gpointer*) array)[i]);
			}
		}
	}
}


static void _vala_array_free (gpointer array, gint array_length, GDestroyNotify destroy_func) {
	_vala_array_destroy (array, array_length, destroy_func);
	g_free (array);
}




